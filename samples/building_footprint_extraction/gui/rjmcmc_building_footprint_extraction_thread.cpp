#include "rjmcmc_building_footprint_extraction_thread.hpp"
#include "chart_frame.hpp"

struct layer_visitor {
public:
  layer_visitor( Layer::ptrLayerType& layer) : m_layer(layer) {}

  void energy(double e) { m_energy=e; }
  typedef void result_type;
  void operator()(const Rectangle_2& r) const {
    std::vector<double> vectx, vecty;
    for (unsigned int i=0; i<4;++i)
    {
      vectx.push_back(CGAL::to_double(r[i].x()));
      vecty.push_back(CGAL::to_double(r[i].y()));
    }
/* // test clipping
    Iso_Rectangle_2 bbox(100,100,500,500);
    for (unsigned int i=0; i<4;++i)
    {
      Segment_2 s(r.segment(i));
      if(!CGAL::clip(bbox,s)) continue;
      vectx.push_back(CGAL::to_double(s.source().x()));
      vecty.push_back(CGAL::to_double(s.source().y()));
      vectx.push_back(CGAL::to_double(s.target().x()));
      vecty.push_back(CGAL::to_double(s.target().y()));
    }
*/
    m_layer->AddPolygon(vectx, vecty );
    std::ostringstream oss;
    oss << m_energy;
    double x = CGAL::to_double(r.center().x());
    double y = CGAL::to_double(r.center().y());
    m_layer->AddText(x, y, oss.str() , wxColour(255,0,0) );
  }
  void operator()(const Circle_2& c) const {
    double x = CGAL::to_double(c.center().x());
    double y = CGAL::to_double(c.center().y());
    double r = CGAL::to_double(c.radius());
    m_layer->AddCircle(x,y,r);
    std::ostringstream oss;
    oss << m_energy;
    m_layer->AddText(x, y, oss.str() , wxColour(255,0,0) );
  }
private:
  Layer::ptrLayerType& m_layer;
  double m_energy;
};

Layer::ptrLayerType& operator<<(Layer::ptrLayerType& layer, const configuration& config)
{
	layer->Clear();
	configuration::const_iterator it = config.begin(), end = config.end();
	layer_visitor visitor(layer);
	for (; it != end; ++it)
	{
		visitor.energy(config.energy(it));
		rjmcmc::apply_visitor(visitor,config[it]);
	}
	return layer;
}


rjmcmc_building_footprint_extraction_thread::rjmcmc_building_footprint_extraction_thread(
		Layer::ptrLayerType ilayer, 
		Layer::ptrLayerType vlayer,
		rjmcmc_building_footprint_extraction_frame *frame) :
		wxThread(wxTHREAD_JOINABLE), m_ilayer(ilayer),m_vlayer(vlayer), m_frame(frame), m_out(""), m_visitor(m_out)
{
// todo: use ilayer->View() instead...
	param *p = param::Instance();
	boost::filesystem::path path;
	p->get("input",path);
	p->set("xmax",p->get<int>("xmax")-p->get<int>("xmin"));
	p->set("ymin",p->get<int>("ymax")-p->get<int>("ymin"));
	p->set("xmin",0);
	p->set("ymin",0);
	m_ilayer->Save(path.string());
}


void rjmcmc_building_footprint_extraction_thread::begin(int dump, int save, double t, const configuration& config)
{
	m_dump = dump;
	m_save = save;
	m_visitor.begin(dump,save,t,config);
	wxMutexGuiEnter();
	{
		wxLogMessage( wxString( m_out.str().c_str(),*wxConvCurrent ).GetData());
		m_frame->m_chart_frame->begin(dump,save,t,config);
	}
	wxMutexGuiLeave();
	m_out.str("");
}

bool rjmcmc_building_footprint_extraction_thread::iterate(unsigned int i, double t, const configuration& config, const sampler& sample)
{
	if(!m_visitor.iterate(i,t,config,sample))
		return false;
	
	if ( m_save && (i%m_save == 0) )
	{
		std::ostringstream out;
		out.width(8);
		out.fill('0');
		out << i;
		m_vlayer->Save( out.str() + ".shp" );
	}
	if (m_dump && (i % m_dump == 0))
	{
		wxMutexGuiEnter();
		{
			m_vlayer << config;
			m_frame->m_chart_frame->iterate(i,t,config,sample);
			m_frame->Refresh();
			wxLogMessage( wxString( m_out.str().c_str(),*wxConvCurrent ).GetData());
		}
		wxMutexGuiLeave();
		m_out.str("");	
		if ( TestDestroy() )
		{
			wxLogMessage( _("Computation stopped") );
			return false;
		}
	}
	return true;
}

void rjmcmc_building_footprint_extraction_thread::end(const configuration& config)
{
	m_visitor.end(config);
	wxMutexGuiEnter();
	{
		wxLogMessage( wxString( m_out.str().c_str(),*wxConvCurrent ).GetData());
		m_frame->OnThreadEnd();
	}
	wxMutexGuiLeave();
	m_out.str("");
}

void* rjmcmc_building_footprint_extraction_thread::Entry()
{
	rjmcmc_building_footprint_extraction(*this);
	return NULL;
}

