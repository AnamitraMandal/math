///////////////////////////////////////////////////////////////////////////////
//  Copyright 2014 Anton Bikineev
//  Copyright 2014 Christopher Kormanyos
//  Copyright 2014 John Maddock
//  Copyright 2014 Paul Bristow
//  Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_MATH_HYPERGEOMETRIC_1F1_HPP
#define BOOST_MATH_HYPERGEOMETRIC_1F1_HPP

#include <boost/math/policies/policy.hpp>
#include <boost/math/policies/error_handling.hpp>
#include <boost/math/special_functions/detail/hypergeometric_series.hpp>
#include <boost/math/special_functions/detail/hypergeometric_asym.hpp>
#include <boost/math/special_functions/detail/hypergeometric_rational.hpp>
#include <boost/math/special_functions/detail/hypergeometric_1F1_recurrence.hpp>
#include <boost/math/special_functions/detail/hypergeometric_1F1_by_ratios.hpp>
#include <boost/math/special_functions/detail/hypergeometric_pade.hpp>
#include <boost/math/special_functions/detail/hypergeometric_1F1_bessel.hpp>
#include <boost/math/special_functions/detail/hypergeometric_1F1_scaled_series.hpp>
#include <boost/math/special_functions/detail/hypergeometric_pFq_checked_series.hpp>
#include <boost/math/special_functions/detail/hypergeometric_1F1_addition_theorems_on_z.hpp>
#include <boost/math/special_functions/detail/hypergeometric_1F1_large_abz.hpp>
#include <boost/math/special_functions/detail/hypergeometric_1F1_small_a_negative_b_by_ratio.hpp>

namespace boost { namespace math { namespace detail {

   // check when 1F1 series can't decay to polynom
   template <class T>
   inline bool check_hypergeometric_1F1_parameters(const T& a, const T& b)
   {
      BOOST_MATH_STD_USING

         if ((b <= 0) && (b == floor(b)))
         {
            if ((a >= 0) || (a < b) || (a != floor(a)))
               return false;
         }

      return true;
   }

   template <class T, class Policy>
   T hypergeometric_1F1_divergent_fallback(const T& a, const T& b, const T& z, const Policy& pol, int& log_scaling)
   {
      BOOST_MATH_STD_USING
      const char* function = "hypergeometric_1F1_divergent_fallback<%1%>(%1%,%1%,%1%)";
      //
      // We get here if either:
      // 1) We decide up front that Tricomi's method won't work, or:
      // 2) We've called Tricomi's method and it's failed.
      //
      if (b > 0)
      {
         // Commented out since recurrence seems to always be better?
#if 0
         if ((z < b) && (a > -50))
            // Might as well use a recurrence in preference to z-recurrence:
            return hypergeometric_1F1_backward_recurrence_for_negative_a(a, b, z, pol, function, log_scaling);
         T z_limit = fabs((2 * a - b) / (sqrt(fabs(a))));
         int k = 1 + itrunc(z - z_limit);
         // If k is too large we destroy all the digits in the result:
         T convergence_at_50 = (b - a + 50) * k / (z * 50);
         if ((k > 0) && (k < 50) && (fabs(convergence_at_50) < 1) && (z > z_limit))
         {
            return boost::math::detail::hypergeometric_1f1_recurrence_on_z_minus_zero(a, b, T(z - k), k, pol, log_scaling);
         }
#endif
         if (z < b)
            return hypergeometric_1F1_backward_recurrence_for_negative_a(a, b, z, pol, function, log_scaling);
         else
            return hypergeometric_1F1_backwards_recursion_on_b_for_negative_a(a, b, z, pol, function, log_scaling);
      }
      else  // b < 0
      {
         if (a < 0)
         {
            if (a > 5 * b)
               return hypergeometric_1F1_from_function_ratio_negative_ab(a, b, z, pol, log_scaling);
            else
            {
               //
               // Solve (a+n)z/((b+n)n) == 1 for n, the number of iterations till the series starts to converge.
               // If this is well away from the origin then it's probably better to use the series to evaluate this.
               // Note that if sqr is negative then we have no solution, so assign an arbitrarily large value to the
               // number of iterations.
               //
               T sqr = 4 * a * z + b * b - 2 * b * z + z * z;
               T iterations_to_convergence = sqr > 0 ? T(0.5f * (-sqrt(sqr) - b + z)) : T(-a - b);
               if((std::max)(a, b) + iterations_to_convergence > -300)
                  return hypergeometric_1F1_backwards_recursion_on_b_for_negative_a(a, b, z, pol, function, log_scaling);
            }
         }
         else
         {
            if (is_in_hypergeometric_1F1_from_function_ratio_negative_b_region(a, b, z))
               return hypergeometric_1F1_from_function_ratio_negative_b(a, b, z, pol, log_scaling);
            else if (hypergeometric_1F1_is_in_forwards_recurence_for_negative_b_region(a, b, z))
               return hypergeometric_1F1_from_function_ratio_negative_b_forwards(a, b, z, pol, log_scaling);
         }
      }

      // If we get here, then we've run out of methods to try, use the checked series which will
      // raise an error if the result is garbage:
      return hypergeometric_1F1_checked_series_impl(a, b, z, pol, log_scaling);
   }

   template <class T>
   bool is_convergent_negative_z_series(const T& a, const T& b, const T& z)
   {
      BOOST_MATH_STD_USING
      //
      // Generic check: we have small initial divergence and are convergent after 10 terms:
      //
      if ((fabs(z * a / b) < 2) && (fabs(z * (a + 10) / ((b + 10) * 10)) < 1))
      {
         // Double check for divergence when we cross the origin on a and b:
         if (a < 0)
         {
            T n = 300 - floor(a);
            if (fabs((a + n) * z / ((b + n) * n)) < 1)
            {
               if (b < 0)
               {
                  T m = 3 - floor(b);
                  if (fabs((a + m) * z / ((b + m) * m)) < 1)
                     return true;
               }
               else
                  return true;
            }
         }
         else if (b < 0)
         {
            T n = 3 - floor(b);
            if (fabs((a + n) * z / ((b + n) * n)) < 1)
               return true;
         }
      }
      if ((b > 0) && (a < 0))
      {
         //
         // For a and z both negative, we're OK with some initial divergence as long as
         // it occurs before we hit the origin, as to start with all the terms have the
         // same sign.
         //
         // https://www.wolframalpha.com/input/?i=solve+(a%2Bn)z+%2F+((b%2Bn)n)+%3D%3D+1+for+n
         //
         T sqr = 4 * a * z + b * b - 2 * b * z + z * z;
         T iterations_to_convergence = sqr > 0 ? T(0.5f * (-sqrt(sqr) - b + z)) : T(-a + b);
         if (iterations_to_convergence < 0)
            iterations_to_convergence = 0.5f * (sqrt(sqr) - b + z);
         if (a + iterations_to_convergence < -50)
         {
            // Need to check for divergence when we cross the origin on a:
            if (a > -1)
               return true;
            T n = 300 - floor(a);
            if(fabs((a + n) * z / ((b + n) * n)) < 1)
               return true;
         }
      }
      return false;
   }

   template <class T>
   inline T cyl_bessel_i_shrinkage_rate(const T& z)
   {
      // Approximately the ratio I_10.5(z/2) / I_9.5(z/2), this gives us an idea of how quickly
      // the Bessel terms in A&S 13.6.4 are converging:
      if (z < 160)
         return 1;
      if (z < 40)
         return 0.75f;
      if (z < 20)
         return 0.5f;
      if (z < 7)
         return 0.25f;
      if (z < 2)
         return 0.1f;
      return 0.05f;
   }

   template <class T>
   inline bool hypergeometric_1F1_is_13_3_6_region(const T& a, const T& b, const T& z)
   {
      BOOST_MATH_STD_USING
      if(fabs(a) == 0.5)
         return false;
      if ((z < 0) && (fabs(10 * a / b) < 1) && (fabs(a) < 50))
      {
         T shrinkage = cyl_bessel_i_shrinkage_rate(z);
         // We want the first term not too divergent, and convergence by term 10:
         if ((fabs((2 * a - 1) * (2 * a - b) / b) < 2) && (fabs(shrinkage * (2 * a + 9) * (2 * a - b + 10) / (10 * (b + 10))) < 0.75))
            return true;
      }
      return false;
   }

   template <class T>
   inline bool hypergeometric_1F1_need_kummer_reflection(const T& a, const T& b, const T& z)
   {
      BOOST_MATH_STD_USING
      //
      // Check to see if we should apply Kummer's relation or not:
      //
      if (z > 0)
         return false;
      if (z < -1)
         return true;
      //
      // When z is small and negative, things get more complex.
      // More often than not we do not need apply Kummer's relation and the
      // series is convergent as is, but we do need to check:
      //
      if (a > 0)
      {
         if (b > 0)
         {
            return fabs((a + 10) * z / (10 * (b + 10))) < 1;  // Is the 10'th term convergent?
         }
         else
         {
            return true;  // Likely to be divergent as b crosses the origin
         }
      }
      else // a < 0
      {
         if (b > 0)
         {
            return false;  // Terms start off all positive and then by the time a crosses the origin we *must* be convergent.
         }
         else
         {
            return true;  // Likely to be divergent as b crosses the origin, but hard to rationalise about!
         }
      }
   }

      
   template <class T, class Policy>
   T hypergeometric_1F1_imp(const T& a, const T& b, const T& z, const Policy& pol, int& log_scaling)
   {
      BOOST_MATH_STD_USING // exp, fabs, sqrt

      static const char* const function = "boost::math::hypergeometric_1F1<%1%,%1%,%1%>(%1%,%1%,%1%)";

      if ((z == 0) || (a == 0))
         return T(1);

      // undefined result:
      if (!detail::check_hypergeometric_1F1_parameters(a, b))
         return policies::raise_domain_error<T>(
            function,
            "Function is indeterminate for negative integer b = %1%.",
            b,
            pol);

      // other checks:
      if (a == -1)
         return 1 - (z / b);

      const T b_minus_a = b - a;

      // 0f0 a == b case;
      if (b_minus_a == 0)
      {
         int scale = itrunc(z, pol);
         log_scaling += scale;
         return exp(z - scale);
      }
      // Special case for b-a = -1, we don't use for small a as it throws the digits of a away and leads to large errors:
      if ((b_minus_a == -1) && (fabs(a) > 0.5))
      {
         // for negative small integer a it is reasonable to use truncated series - polynomial
         if ((a < 0) && (a == ceil(a)) && (a > -50))
            return detail::hypergeometric_1F1_generic_series(a, b, z, pol, log_scaling, function);

         return (b + z) * exp(z) / b;
      }

      if ((a == 1) && (b == 2))
         return boost::math::expm1(z, pol) / z;

      if ((b - a == b) && (fabs(z / b) < policies::get_epsilon<T, Policy>()))
         return 1;
      //
      // Special case for A&S 13.3.6:
      //
      if (z < 0)
      {
         if (hypergeometric_1F1_is_13_3_6_region(a, b, z))
         {
            // a is tiny compared to b, and z < 0
            // 13.3.6 appears to be the most efficient and often the most accurate method.
            T r = boost::math::detail::hypergeometric_1F1_AS_13_3_6(b_minus_a, b, T(-z), a, pol, log_scaling);
            int scale = itrunc(z, pol);
            log_scaling += scale;
            return r * exp(z - scale);
         }
         if ((b < 0) && (fabs(a) < 1e-2))
         {
            //
            // This is a tricky area, potentially we have no good method at all:
            //
            if (b - ceil(b) == a)
            {
               // Fractional parts of a and b are genuinely equal, we might as well
               // apply Kummer's relation and get a truncated series:
               int scaling = itrunc(z);
               T r = exp(z - scaling) * detail::hypergeometric_1F1_imp<T>(b_minus_a, b, -z, pol, log_scaling);
               log_scaling += scaling;
               return r;
            }
            if ((b < -1) && (max_b_for_1F1_small_a_negative_b_by_ratio(z) < b))
               return hypergeometric_1F1_small_a_negative_b_by_ratio(a, b, z, pol, log_scaling);
            if ((b > -1) && (b < -0.5f))
            {
               // Recursion is meta-stable:
               T first = hypergeometric_1F1_imp(a, T(b + 2), z, pol);
               T second = hypergeometric_1F1_imp(a, T(b + 1), z, pol);
               return tools::apply_recurrence_relation_backward(hypergeometric_1F1_recurrence_small_b_coefficients<T>(a, b, z, 1), 1, first, second);
            }
            //
            // We've got nothing left but 13.3.6, even though it may be initially divergent:
            //
            T r = boost::math::detail::hypergeometric_1F1_AS_13_3_6(b_minus_a, b, T(-z), a, pol, log_scaling);
            int scale = itrunc(z, pol);
            log_scaling += scale;
            return r * exp(z - scale);
         }
      }
      //
      // Asymptotic expansion for large z
      // TODO: check region for higher precision types.
      // Use recurrence relations to move to this region when a and b are also large.
      //
      if (detail::hypergeometric_1F1_asym_region(a, b, z, pol))
      {
         int saved_scale = log_scaling;
         try
         {
            return hypergeometric_1F1_asym_large_z_series(a, b, z, pol, log_scaling);
         }
         catch (const evaluation_error&)
         {
         }
         //
         // Very occationally our convergence criteria don't quite go to full precision
         // and we have to try another method:
         //
         log_scaling = saved_scale;
      }

      if ((fabs(a * z / b) < 3.5) && (fabs(z * 100) < fabs(b)) && ((fabs(a) > 1e-2) || (b < -5)))
         return detail::hypergeometric_1F1_rational(a, b, z, pol);

      if (hypergeometric_1F1_need_kummer_reflection(a, b, z))
      {
         if (a == 1)
            return detail::hypergeometric_1F1_pade(b, z, pol);
         if (is_convergent_negative_z_series(a, b, z))
         {
            if ((boost::math::sign(b - a) == boost::math::sign(b)) && ((b > 0) || (b < -200)))
            {
               // Series is close enough to convergent that we should be OK,
               // In this domain b - a ~ b and since 1F1[a, a, z] = e^z 1F1[b-a, b, -z]
               // and 1F1[a, a, -z] = e^-z the result must necessarily be somewhere near unity.
               // We have to rule out b small and negative becuase if b crosses the origin early
               // in the series (before we're pretty much converged) then all bets are off.
               // Note that this can go badly wrong when b and z are both large and negative,
               // in that situation the series goes in waves of large and small values which
               // may or may not cancel out.  Likewise the initial part of the series may or may
               // not converge, and even if it does may or may not give a correct answer!
               // For example 1F1[-small, -1252.5, -1043.7] can loose up to ~800 digits due to
               // cancellation and is basically incalculable via this method.
               return hypergeometric_1F1_checked_series_impl(a, b, z, pol, log_scaling);
            }
         }
         // Let's otherwise make z positive (almost always)
         // by Kummer's transformation
         // (we also don't transform if z belongs to [-1,0])
         int scaling = itrunc(z);
         T r = exp(z - scaling) * detail::hypergeometric_1F1_imp<T>(b_minus_a, b, -z, pol, log_scaling);
         log_scaling += scaling;
         return r;
      }
      //
      // Check for initial divergence:
      //
      bool series_is_divergent = (a + 1) * z / (b + 1) < -1;
      if (series_is_divergent && (a < 0) && (b < 0) && (a > -1))
         series_is_divergent = false;   // Best off taking the series in this situation
      //
      // If series starts off non-divergent, and becomes divergent later
      // then it's because both a and b are negative, so check for later
      // divergence as well:
      //
      if (!series_is_divergent && (a < 0) && (b < 0) && (b > a))
      {
         //
         // We need to exclude situations where we're over the initial "hump"
         // in the series terms (ie series has already converged by the time
         // b crosses the origin:
         //
         //T fa = fabs(a);
         //T fb = fabs(b);
         T convergence_point = sqrt((a - 1) * (a - b)) - a;
         if (-b < convergence_point)
         {
            T n = -floor(b);
            series_is_divergent = (a + n) * z / ((b + n) * n) < -1;
         }
      }
      if (series_is_divergent && (b < -1) && (b > -5) && (a > b))
         series_is_divergent = false;  // don't bother with divergence, series will be OK

      //
      // Test for alternating series due to negative a,
      // in particular, see if the series is initially divergent
      // If so use the recurrence relation on a:
      //
      if (series_is_divergent)
      {
         if((a < 0) && (floor(a) == a) && (-a < policies::get_max_series_iterations<Policy>()))
            // This works amazingly well for negative integer a:
            return hypergeometric_1F1_backward_recurrence_for_negative_a(a, b, z, pol, function, log_scaling);
         //
         // In what follows we have to set limits on how large z can be otherwise
         // the Bessel series become large and divergent and all the digits cancel out.
         // The criteria are distinctly empiracle rather than based on a firm analysis
         // of the terms in the series.
         //
         if (b > 0)
         {
            T z_limit = fabs((2 * a - b) / (sqrt(fabs(a))));
            if ((z < z_limit) && hypergeometric_1F1_is_tricomi_viable_positive_b(a, b, z))
               return detail::hypergeometric_1F1_AS_13_3_7_tricomi(a, b, z, pol, log_scaling);
         }
         else  // b < 0
         {
            if (a < 0)
            {
               T z_limit = fabs((2 * a - b) / (sqrt(fabs(a))));
               //
               // I hate these hard limits, but they're about the best we can do to try and avoid
               // Bessel function internal failures: these will be caught and handled
               // but up the expense of this function call:
               //
               if (((z < z_limit) || (a > -500)) && ((b > -500) || (b - 2 * a > 0)) && (z < -a))
                  return detail::hypergeometric_1F1_AS_13_3_7_tricomi(a, b, z, pol, log_scaling);
            }
            else
            {
               T aa = a < 1 ? T(1) : a;
               if (z < fabs((2 * aa - b) / (sqrt(fabs(aa * b)))))
                  return detail::hypergeometric_1F1_AS_13_3_7_tricomi(a, b, z, pol, log_scaling);
            }
         }

         return hypergeometric_1F1_divergent_fallback(a, b, z, pol, log_scaling);
      }

      if (hypergeometric_1F1_is_13_3_6_region(b_minus_a, b, T(-z)))
      {
         // b_minus_a is tiny compared to b, and -z < 0
         // 13.3.6 appears to be the most efficient and often the most accurate method.
         return boost::math::detail::hypergeometric_1F1_AS_13_3_6(a, b, z, b_minus_a, pol, log_scaling);
      }
      if ((a > 0) && (b > 0) && (a * z / b > 2))
      {
         //
         // Series is initially divergent and slow to converge, see if applying
         // Kummer's relation can improve things:
         //
         if (is_convergent_negative_z_series(b_minus_a, b, T(-z)))
         {
            int scaling = itrunc(z);
            T r = exp(z - scaling) * detail::hypergeometric_1F1_checked_series_impl(b_minus_a, b, T(-z), pol, log_scaling);
            log_scaling += scaling;
            return r;
         }

      }
      if ((a > 0) && (b > 0) && (a * z > 50))
         return detail::hypergeometric_1F1_large_abz(a, b, z, pol, log_scaling);
      
      return detail::hypergeometric_1F1_generic_series(a, b, z, pol, log_scaling, function);
   }

   template <class T, class Policy>
   inline T hypergeometric_1F1_imp(const T& a, const T& b, const T& z, const Policy& pol)
   {
      BOOST_MATH_STD_USING // exp, fabs, sqrt
      int log_scaling = 0;
      T result = hypergeometric_1F1_imp(a, b, z, pol, log_scaling);
      //
      // Actual result will be result * e^log_scaling.
      //
      int max_scaling = itrunc(boost::math::tools::log_max_value<T>()) - 2;
      T max_scale_factor = exp(T(max_scaling));

      while (log_scaling > max_scaling)
      {
         result *= max_scale_factor;
         log_scaling -= max_scaling;
      }
      while (log_scaling < -max_scaling)
      {
         result /= max_scale_factor;
         log_scaling += max_scaling;
      }
      if (log_scaling)
         result *= exp(T(log_scaling));
      return result;
   }

} // namespace detail

template <class T1, class T2, class T3, class Policy>
inline typename tools::promote_args<T1, T2, T3>::type hypergeometric_1F1(T1 a, T2 b, T3 z, const Policy& /* pol */)
{
   BOOST_FPU_EXCEPTION_GUARD
      typedef typename tools::promote_args<T1, T2, T3>::type result_type;
   typedef typename policies::evaluation<result_type, Policy>::type value_type;
   typedef typename policies::normalise<
      Policy,
      policies::promote_float<false>,
      policies::promote_double<false>,
      policies::discrete_quantile<>,
      policies::assert_undefined<> >::type forwarding_policy;
   return policies::checked_narrowing_cast<result_type, Policy>(
      detail::hypergeometric_1F1_imp<value_type>(
         static_cast<value_type>(a),
         static_cast<value_type>(b),
         static_cast<value_type>(z),
         forwarding_policy()),
      "boost::math::hypergeometric_1F1<%1%>(%1%,%1%,%1%)");
}

template <class T1, class T2, class T3>
inline typename tools::promote_args<T1, T2, T3>::type hypergeometric_1F1(T1 a, T2 b, T3 z)
{
   return hypergeometric_1F1(a, b, z, policies::policy<>());
}


  } } // namespace boost::math

#endif // BOOST_MATH_HYPERGEOMETRIC_HPP
