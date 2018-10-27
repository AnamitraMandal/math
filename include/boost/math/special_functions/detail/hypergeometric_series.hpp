///////////////////////////////////////////////////////////////////////////////
//  Copyright 2014 Anton Bikineev
//  Copyright 2014 Christopher Kormanyos
//  Copyright 2014 John Maddock
//  Copyright 2014 Paul Bristow
//  Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef BOOST_MATH_DETAIL_HYPERGEOMETRIC_SERIES_HPP
#define BOOST_MATH_DETAIL_HYPERGEOMETRIC_SERIES_HPP

#include <boost/math/tools/series.hpp>
#include <boost/math/special_functions/trunc.hpp>
#include <boost/math/policies/error_handling.hpp>

  namespace boost { namespace math { namespace detail {

  // primary template for term of Taylor series
  template <class T, unsigned p, unsigned q>
  struct hypergeometric_pFq_generic_series_term;

  // partial specialization for 0F1
  template <class T>
  struct hypergeometric_pFq_generic_series_term<T, 0u, 1u>
  {
    typedef T result_type;

    hypergeometric_pFq_generic_series_term(const T& b, const T& z)
       : n(0), term(1), b(b), z(z)
    {
    }

    T operator()()
    {
      BOOST_MATH_STD_USING
      const T r = term;
      term *= ((1 / ((b + n) * (n + 1))) * z);
      ++n;
      return r;
    }

  private:
    unsigned n;
    T term;
    const T b, z;
  };

  // partial specialization for 1F0
  template <class T>
  struct hypergeometric_pFq_generic_series_term<T, 1u, 0u>
  {
    typedef T result_type;

    hypergeometric_pFq_generic_series_term(const T& a, const T& z)
       : n(0), term(1), a(a), z(z)
    {
    }

    T operator()()
    {
      BOOST_MATH_STD_USING
      const T r = term;
      term *= (((a + n) / (n + 1)) * z);
      ++n;
      return r;
    }

  private:
    unsigned n;
    T term;
    const T a, z;
  };

  // partial specialization for 1F1
  template <class T>
  struct hypergeometric_pFq_generic_series_term<T, 1u, 1u>
  {
    typedef T result_type;

    hypergeometric_pFq_generic_series_term(const T& a, const T& b, const T& z)
       : n(0), term(1), a(a), b(b), z(z)
    {
    }

    T operator()()
    {
      BOOST_MATH_STD_USING
      const T r = term;
      term *= (((a + n) / ((b + n) * (n + 1))) * z);
      ++n;
      return r;
    }

  private:
    unsigned n;
    T term;
    const T a, b, z;
  };

  // partial specialization for 1F2
  template <class T>
  struct hypergeometric_pFq_generic_series_term<T, 1u, 2u>
  {
    typedef T result_type;

    hypergeometric_pFq_generic_series_term(const T& a, const T& b1, const T& b2, const T& z)
       : n(0), term(1), a(a), b1(b1), b2(b2), z(z)
    {
    }

    T operator()()
    {
      BOOST_MATH_STD_USING
      const T r = term;
      term *= (((a + n) / ((b1 + n) * (b2 + n) * (n + 1))) * z);
      ++n;
      return r;
    }

  private:
    unsigned n;
    T term;
    const T a, b1, b2, z;
  };

  // partial specialization for 2F0
  template <class T>
  struct hypergeometric_pFq_generic_series_term<T, 2u, 0u>
  {
    typedef T result_type;

    hypergeometric_pFq_generic_series_term(const T& a1, const T& a2, const T& z)
       : n(0), term(1), a1(a1), a2(a2), z(z)
    {
    }

    T operator()()
    {
      BOOST_MATH_STD_USING
      const T r = term;
      term *= (((a1 + n) * (a2 + n) / (n + 1)) * z);
      ++n;
      return r;
    }

  private:
    unsigned n;
    T term;
    const T a1, a2, z;
  };

  // partial specialization for 2F1
  template <class T>
  struct hypergeometric_pFq_generic_series_term<T, 2u, 1u>
  {
    typedef T result_type;

    hypergeometric_pFq_generic_series_term(const T& a1, const T& a2, const T& b, const T& z)
       : n(0), term(1), a1(a1), a2(a2), b(b), z(z)
    {
    }

    T operator()()
    {
      BOOST_MATH_STD_USING
      const T r = term;
      term *= (((a1 + n) * (a2 + n) / ((b + n) * (n + 1))) * z);
      ++n;
      return r;
    }

  private:
    unsigned n;
    T term;
    const T a1, a2, b, z;
  };

  // we don't need to define extra check and make a polinom from
  // series, when p(i) and q(i) are negative integers and p(i) >= q(i)
  // as described in functions.wolfram.alpha, because we always
  // stop summation when result (in this case numerator) is zero.
  template <class T, unsigned p, unsigned q, class Policy>
  inline T sum_pFq_series(detail::hypergeometric_pFq_generic_series_term<T, p, q>& term, const Policy& pol)
  {
    BOOST_MATH_STD_USING
    boost::uintmax_t max_iter = policies::get_max_series_iterations<Policy>();
#if BOOST_WORKAROUND(__BORLANDC__, BOOST_TESTED_AT(0x582))
    const T zero = 0;
    const T result = boost::math::tools::sum_series(term, boost::math::policies::get_epsilon<T, Policy>(), max_iter, zero);
#else
    const T result = boost::math::tools::sum_series(term, boost::math::policies::get_epsilon<T, Policy>(), max_iter);
#endif
    policies::check_series_iterations<T>("boost::math::hypergeometric_pFq_generic_series<%1%>(%1%,%1%,%1%)", max_iter, pol);
    return result;
  }

  template <class T, class Policy>
  inline T hypergeometric_0F1_generic_series(const T& b, const T& z, const Policy& pol)
  {
    detail::hypergeometric_pFq_generic_series_term<T, 0u, 1u> s(b, z);
    return detail::sum_pFq_series(s, pol);
  }

  template <class T, class Policy>
  inline T hypergeometric_1F0_generic_series(const T& a, const T& z, const Policy& pol)
  {
    detail::hypergeometric_pFq_generic_series_term<T, 1u, 0u> s(a, z);
    return detail::sum_pFq_series(s, pol);
  }

  template <class T, class Policy>
  inline T hypergeometric_1F1_generic_series(const T& a, const T& b, const T& z, const Policy& pol, int& log_scaling, const char* function)
  {
     BOOST_MATH_STD_USING_CORE
     T sum(0), term(1), upper_limit(sqrt(boost::math::tools::max_value<T>())), diff;
     T lower_limit(1 / upper_limit);
     unsigned n = 0;
     int log_scaling_factor = boost::math::itrunc(boost::math::tools::log_max_value<T>()) - 2;
     T scaling_factor = exp(log_scaling_factor);

     do
     {
        sum += term;
        if (fabs(sum) >= upper_limit)
        {
           sum /= scaling_factor;
           term /= scaling_factor;
           log_scaling += log_scaling_factor;
        }
        if (fabs(sum) < lower_limit)
        {
           sum *= scaling_factor;
           term *= scaling_factor;
           log_scaling -= log_scaling_factor;
        }
        term *= (((a + n) / ((b + n) * (n + 1))) * z);
        if (n > boost::math::policies::get_max_series_iterations<Policy>())
           return boost::math::policies::raise_evaluation_error(function, "Series did not converge, best value is %1%", sum, pol);
        ++n;
        diff = fabs(term / sum);
     } while (diff > boost::math::policies::get_epsilon<T, Policy>());

     return sum;
  }

  template <class T, class Policy>
  inline T hypergeometric_1F2_generic_series(const T& a, const T& b1, const T& b2, const T& z, const Policy& pol)
  {
    detail::hypergeometric_pFq_generic_series_term<T, 1u, 2u> s(a, b1, b2, z);
    return detail::sum_pFq_series(s, pol);
  }

  template <class T, class Policy>
  inline T hypergeometric_2F0_generic_series(const T& a1, const T& a2, const T& z, const Policy& pol)
  {
    detail::hypergeometric_pFq_generic_series_term<T, 2u, 0u> s(a1, a2, z);
    return detail::sum_pFq_series(s, pol);
  }

  template <class T, class Policy>
  inline T hypergeometric_2F1_generic_series(const T& a1, const T& a2, const T& b, const T& z, const Policy& pol)
  {
    detail::hypergeometric_pFq_generic_series_term<T, 2u, 1u> s(a1, a2, b, z);
    return detail::sum_pFq_series(s, pol);
  }

  } } } // namespaces

#endif // BOOST_MATH_DETAIL_HYPERGEOMETRIC_SERIES_HPP
