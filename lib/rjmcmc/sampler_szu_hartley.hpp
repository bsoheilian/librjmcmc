#ifndef SAMPLER_SZU_HARTLEY_HPP
#define SAMPLER_SZU_HARTLEY_HPP

#include "sampler.hpp"

namespace rjmcmc
{

template<RJMCMC_SAMPLER_ENUM_PARAMS_WITH_A_DEFAULT(typename T,boost::tuples::null_type)>
class szu_hartley_sampler : public sampler_base<RJMCMC_SAMPLER_ENUM_PARAMS(T)>
{
private:
    typedef sampler_base<RJMCMC_SAMPLER_ENUM_PARAMS(T)> base;

public:
    szu_hartley_sampler(RJMCMC_SAMPLER_ENUM(RJMCMC_SAMPLER_ARG)) : base(RJMCMC_SAMPLER_ENUM_PARAMS(t)) {}
    // main sampling function
    template<typename Configuration>
    void operator()(Configuration &c, double temp)
    {
        typename Configuration::modification modif;
        base::m_temperature = temp;
        base::m_kernel_id   = internal::random_apply(base::m_die(),base::m_kernel,c,modif,base::m_green_ratio);
        if(base::m_green_ratio<=0) {
            base::m_delta   =0;
            base::m_accepted=false;
            return;
        }
        base::m_delta       = c.delta_energy(modif);
        base::m_green_ratio*= 1./(1.+exp(base::m_delta/base::m_temperature));
        base::m_accepted    = ( base::m_die() < base::m_green_ratio );
        if (base::m_accepted) c.apply(modif);
    }
};

} // namespace rjmcmc

#endif // SAMPLER_SZU_HARTLEY_HPP
