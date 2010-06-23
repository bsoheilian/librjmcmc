#ifndef SZU_HARTLEY_SAMPLER_HPP
#define SZU_HARTLEY_SAMPLER_HPP

#include "sampler_base.hpp"

namespace rjmcmc
{

template<RJMCMC_SAMPLER_TYPENAMES>
  class szu_hartley_sampler
  : public sampler_base<szu_hartley_sampler<RJMCMC_SAMPLER_TYPES>, RJMCMC_SAMPLER_TYPES>
{
private:
  typedef szu_hartley_sampler<RJMCMC_SAMPLER_TYPES> self;
  typedef sampler_base<self, RJMCMC_SAMPLER_TYPES> base;
  
public:
  szu_hartley_sampler(RJMCMC_SAMPLER_ARGS) : base(RJMCMC_SAMPLER_PARAMS) {}
  
  inline double acceptance_probability() const {
    return base::m_green_ratio/(1.+exp(base::m_delta/base::m_temperature));
  }
};

} // namespace rjmcmc

#endif // SZU_HARTLEY_SAMPLER_HPP