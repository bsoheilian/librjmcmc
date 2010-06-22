#ifndef __BUILDING_FOOTPRINT_RECTANGLE_HPP__
#define __BUILDING_FOOTPRINT_RECTANGLE_HPP__

/************** application-specific types ****************/

#include "core/geometry.h"
typedef Rectangle_2 object;
//#include <boost/variant.hpp>
//typedef boost::variant<Rectangle_2> object;

#include "energy/box_is_valid.hpp"
typedef box_is_valid                         is_valid;

#include "energy/image_gradient_unary_energy.hpp"
#include "image/gradient_functor.hpp"
#include "image/oriented.hpp"
typedef oriented<gradient_image_t> oriented_gradient_view;
typedef image_gradient_unary_energy<oriented_gradient_view> unary_energy;

#include "energy/intersection_area_binary_energy.hpp"
typedef intersection_area_binary_energy      binary_energy;

#include "energy/kernels.hpp"
typedef generator<is_valid>          generator_kernel;
typedef modifier <is_valid>          modifier_kernel;

/************** rjmcmc library types ****************/

#include "rjmcmc/temperature.hpp"
typedef rjmcmc::geometric_temperature                    temperature;

#include "rjmcmc/max_iteration_end_test.hpp"
typedef max_iteration_end_test                           end_test;

#include "rjmcmc/configuration.hpp"
typedef rjmcmc::graph_configuration
//typedef rjmcmc::vector_configuration
	<object, unary_energy, binary_energy>                configuration;

#include "rjmcmc/sampler_metropolis.hpp"
typedef rjmcmc::uniform_birth_kernel<generator_kernel>          birth_kernel;
typedef rjmcmc::uniform_death_kernel                            death_kernel;
typedef rjmcmc::binary_kernel<birth_kernel,death_kernel>        birth_death_kernel;
typedef rjmcmc::modification_kernel<modifier_kernel>            modification_kernel;
typedef rjmcmc::sampler_metropolis<birth_death_kernel,modification_kernel> sampler;

/************** main ****************/

Iso_rectangle_2 get_bbox(const param *p) {
    int x0 = p->get<int>("xmin");
    int x1 = p->get<int>("xmax");
    int y0 = p->get<int>("ymin");
    int y1 = p->get<int>("ymax");
	if(x0>x1) std::swap(x0,x1);
    if(y0>y1) std::swap(y0,y1);
	return Iso_rectangle_2(x0,y0,x1,y1);
}

void set_bbox(param *p, const Iso_rectangle_2& r) {
	p->set("xmin",(int) r.min().x());
	p->set("ymin",(int) r.min().y());
	p->set("xmax",(int) r.max().x());
	p->set("ymax",(int) r.max().y());
}

void create_configuration(const param *p, const oriented_gradient_view& grad, configuration *&c) {
	// energies
	unary_energy e1( grad,
		p->get<double>("energy"),
        p->get<double>("ponderation_grad")
	);

	binary_energy e2(
		p->get<double>("surface")
	);

	// empty initial configuration
	c = new configuration(e1,e2);
}

void create_sampler(const param *p, sampler *&s) {
	is_valid valid(
		get_bbox(p),
		p->get<double>("minsize"),
		p->get<double>("maxratio")
	);

	generator_kernel birth(valid);
	modifier_kernel  modif(valid);

	birth_kernel        kbirth(birth);
	death_kernel        kdeath(      p->get<double>("poisson"));
	modification_kernel kmodif(modif,p->get<double>("poisson"));

	birth_death_kernel kbirthdeath(
		kbirth, kdeath,
		p->get<double>("pbirth"),
		p->get<double>("pdeath")
	);

        s = new sampler( kbirthdeath, kmodif );
}

void create_temperature(const param *p, temperature *&t)
{
	t = new temperature(
		p->get<double>("temp"),
		p->get<double>("deccoef")
	);
}

void create_end_test(const param *p, end_test *&e)
{
	e = new end_test(
		p->get<int>("nbiter")
	);
}

template<typename Visitor>
void init_visitor(const param *p, Visitor& v)
{
	v.init(
		p->get<int>("nbdump"),
		p->get<int>("nbsave")
	);
}

#endif // __BUILDING_FOOTPRINT_RECTANGLE_HPP__
