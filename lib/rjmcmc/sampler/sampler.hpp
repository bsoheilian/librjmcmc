#ifndef __RJMCMC_SAMPLER_HPP__
#define __RJMCMC_SAMPLER_HPP__

#include "rjmcmc/tuple.hpp"
#include "rjmcmc/random.hpp"
#include "rjmcmc/random_apply.hpp"
#include "rjmcmc/kernel/kernel_traits.hpp"
#include "rjmcmc/kernel/unary_kernel.hpp"
#include "rjmcmc/kernel/binary_kernel.hpp"
#include <iomanip>

namespace rjmcmc {

    // Derived: Curiously recurring template pattern
    template<typename Density, typename Acceptance, RJMCMC_TUPLE_TYPENAMES >
    class sampler
    {
    public:
        sampler(const Density& d, const Acceptance& a, RJMCMC_TUPLE_ARGS) :
                m_die(random(), boost::uniform_real<>(0,1)),
                m_density(d),
                m_acceptance(a),
                m_kernel(RJMCMC_TUPLE_PARAMS)
        {}

        typedef tuple<RJMCMC_TUPLE_TYPES> Kernels;
        typedef boost::variate_generator<rjmcmc::mt19937_generator&, boost::uniform_real<> > die_t;

        enum { size = tuple_size<Kernels>::value };
        enum { kernel_size = kernel_traits<Kernels>::size };

        template<typename Configuration, typename Modification>
        struct kernel_functor
        {
            Configuration& m_c;
            Modification& m_m;
            typedef double result_type;
            kernel_functor(Configuration &c, Modification &m) :m_c(c), m_m(m) {}
            template<typename T> inline result_type operator()(double x, const T& t) {
                return t(x,m_c,m_m);
            }
        };

        // main sampling function
        template<typename Configuration>
        void operator()(Configuration &c, double temp)
        {
            typedef typename Configuration::modification Modification;

            //1 & 2
            Modification modif;
            m_temperature = temp;
            kernel_functor<Configuration,Modification> kf(c,modif);
            m_green_ratio  = random_apply(m_kernel_id,m_die(),m_kernel,kf);

            //3
            m_green_ratio *= m_density.pdf_ratio(c,modif);

            //4
            if(m_green_ratio<=0) {
                m_delta   =0;
                m_accepted=false;
                return;
            }
            m_delta       = c.delta_energy(modif);
            m_acceptance_probability  = m_acceptance(m_delta,m_temperature,m_green_ratio);

            //5
            m_accepted    = ( m_die() < m_acceptance_probability );
            if (m_accepted) c.apply(modif);
        }

        // statistics accessors
    private: // helpers
        template<unsigned int I, unsigned int N> struct get_name {
            inline const char *operator()(unsigned int i, const Kernels& k) const {
                enum { ks = tuple_element<I,Kernels>::type::size };
                if(i<ks) return get<I>(k).name(i);
                return get_name<I+1,N>()(i-ks,k);
            }
        };
        template<unsigned int N> struct get_name<N,N> {
            inline const char *operator()(unsigned int i, const Kernels& k) const { return ""; }
        };

        template<unsigned int I, unsigned int N> struct get_kernel_id {
            inline unsigned int operator()(unsigned int i, const Kernels& k) const {
                enum { ks = tuple_element<I,Kernels>::type::size };
                if(i==I) return get<I>(k).kernel_id();
                return ks+get_kernel_id<I+1,N>()(i,k);
            }
        };
        template<unsigned int N> struct get_kernel_id<N,N> {
            inline unsigned int operator()(unsigned int i, const Kernels& k) const { return 0; }
        };

    public:
        // data accessors
        inline const Density& density() const { return m_density; }

        // statistics accessors
        inline double acceptance_probability() const { return m_acceptance_probability; }
        inline double temperature() const { return m_temperature; }
        inline double delta() const { return m_delta; }
        inline double green_ratio() const { return m_green_ratio; }
        inline bool accepted() const { return m_accepted; }
        inline const char * kernel_name(unsigned int i) const { return get_name     <0,size>()(i,m_kernel); }
        inline unsigned int kernel_id  () const { return get_kernel_id<0,size>()(m_kernel_id,m_kernel); }

    private:
        // data
        die_t      m_die;
        Density    m_density;
        Acceptance m_acceptance;
        Kernels    m_kernel;

        // statistics
        double  m_acceptance_probability;
        bool    m_accepted;
        unsigned int m_kernel_id;
        double  m_temperature;
        double  m_delta;
        double  m_green_ratio;
    };

    /*
    // convenience creation functions
    template<typename Density, typename Acceptance, RJMCMC_TUPLE_TYPENAMES >
    sampler<Density,Acceptance,RJMCMC_TUPLE_TYPES> make_sampler(const Density& d, const Acceptance& a, RJMCMC_TUPLE_ARGS)
    {
        return sampler<Density,Acceptance,RJMCMC_TUPLE_TYPES>(d,a,RJMCMC_TUPLE_PARAMS);
    }

    template<typename Density, typename Acceptance, RJMCMC_TUPLE_TYPENAMES >
    sampler<Density,Acceptance,RJMCMC_TUPLE_TYPES> * new_sampler(const Density& d, const Acceptance& a, RJMCMC_TUPLE_ARGS)
    {
        return new sampler<Density,Acceptance,RJMCMC_TUPLE_TYPES>(d,a,RJMCMC_TUPLE_PARAMS);
    }
    */
}; // namespace rjmcmc

#endif // __RJMCMC_SAMPLER_HPP__