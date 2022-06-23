
//          Copyright Ted Middleton 2022.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef INCLUDED_mainframe_simd_h
#define INCLUDED_mainframe_simd_h

#include <iostream>

#if __AVX__
#include <immintrin.h>
#endif

#include "mainframe/base.hpp"

namespace mf
{
namespace detail
{
template<typename T>
T
mean(const T* t, size_t num)
{
    const T* e = t + num;
    T m        = static_cast<T>(0);
    for (const T* c = t; c != e; c++) {
        m += *c;
    }
    return static_cast<T>(m / num);
}

#ifdef __AVX__
float
mean(const float* t, size_t num)
{
    size_t i     = 0;
    __m256 accum = _mm256_setzero_ps();
    for (; i + 8 < num; i += 8) {
        __m256 vals = _mm256_loadu_ps(t + i);
        accum       = _mm256_add_ps(accum, vals);
    }
    float ms[8];
    _mm256_storeu_ps(ms, accum);
    float m = ms[0] + ms[1] + ms[2] + ms[3] + ms[4] + ms[5] + ms[6] + ms[7];
    for (; i < num; i += 1) {
        m += t[i];
    }
    return m / num;
}

double
mean(const double* t, size_t num)
{
    size_t i      = 0;
    __m256d accum = _mm256_setzero_pd();
    for (; i + 4 < num; i += 4) {
        __m256d vals = _mm256_loadu_pd(t + i);
        accum        = _mm256_add_pd(accum, vals);
    }
    double ms[4];
    _mm256_storeu_pd(ms, accum);
    double m = ms[0] + ms[1] + ms[2] + ms[3];
    for (; i < num; i += 1) {
        m += t[i];
    }
    return m / num;
}
#endif

template<typename A, typename B>
auto
correlate_pearson(const A* a, const B* b, size_t num) -> decltype(a[0] * b[0])
{
    using T        = decltype(a[0] * b[0]);
    A amean        = mean(a, num);
    B bmean        = mean(b, num);
    const A* aend  = a + num;
    const A* acurr = a;
    const B* bcurr = b;
    A aaccum       = static_cast<A>(0);
    B baccum       = static_cast<B>(0);
    double cov     = 0.0;
    for (; acurr != aend; ++acurr, ++bcurr) {
        A xa      = *acurr;
        B xb      = *bcurr;
        A adiff   = xa - amean;
        B bdiff   = xb - bmean;
        A adiffsq = adiff * adiff;
        B bdiffsq = bdiff * bdiff;
        T abdiff  = adiff * bdiff;
        aaccum += adiffsq;
        baccum += bdiffsq;
        cov += abdiff;
    }

    T corr = static_cast<T>(cov / std::sqrt(aaccum * baccum));
    return corr;
}

#ifdef __AVX__
double
correlate_pearson(const double* a, const double* b, size_t num)
{
    double samean  = mean(a, num);
    double sbmean  = mean(b, num);
    __m256d amean  = _mm256_set1_pd(samean);
    __m256d bmean  = _mm256_set1_pd(sbmean);
    __m256d aaccum = _mm256_setzero_pd();
    __m256d baccum = _mm256_setzero_pd();
    __m256d cov    = _mm256_setzero_pd();
    size_t k       = 0;
    for (; k + 4 < num; k += 4) {
        __m256d xa, xb, adiff, bdiff, adiffsq, bdiffsq, abdiff;
        xa      = _mm256_loadu_pd(a + k);
        xb      = _mm256_loadu_pd(b + k);
        adiff   = _mm256_sub_pd(xa, amean);
        bdiff   = _mm256_sub_pd(xb, bmean);
        adiffsq = _mm256_mul_pd(adiff, adiff);
        bdiffsq = _mm256_mul_pd(bdiff, bdiff);
        abdiff  = _mm256_mul_pd(adiff, bdiff);
        aaccum  = _mm256_add_pd(aaccum, adiffsq);
        baccum  = _mm256_add_pd(baccum, bdiffsq);
        cov     = _mm256_add_pd(cov, abdiff);
    }
    double mcov[4];
    _mm256_storeu_pd(mcov, cov);
    double fcov = mcov[0] + mcov[1] + mcov[2] + mcov[3];
    double maaccum[4];
    _mm256_storeu_pd(maaccum, aaccum);
    double faaccum = maaccum[0] + maaccum[1] + maaccum[2] + maaccum[3];
    double mbaccum[4];
    _mm256_storeu_pd(mbaccum, baccum);
    double fbaccum = mbaccum[0] + mbaccum[1] + mbaccum[2] + mbaccum[3];

    for (; k < num; k += 1) {
        double xa      = a[k];
        double xb      = b[k];
        double adiff   = xa - samean;
        double bdiff   = xb - sbmean;
        double adiffsq = adiff * adiff;
        double bdiffsq = bdiff * bdiff;
        double abdiff  = adiff * bdiff;
        faaccum += adiffsq;
        fbaccum += bdiffsq;
        fcov += abdiff;
    }
    double corr = fcov / std::sqrt(faaccum * fbaccum);
    return corr;
}

float
correlate_pearson(const float* a, const float* b, size_t num)
{
    float samean  = mean(a, num);
    float sbmean  = mean(b, num);
    __m256 amean  = _mm256_set1_ps(samean);
    __m256 bmean  = _mm256_set1_ps(sbmean);
    __m256 aaccum = _mm256_setzero_ps();
    __m256 baccum = _mm256_setzero_ps();
    __m256 cov    = _mm256_setzero_ps();
    size_t k      = 0;
    for (; k + 8 < num; k += 8) {
        __m256 xa, xb, adiff, bdiff, adiffsq, bdiffsq, abdiff;
        xa      = _mm256_loadu_ps(a + k);
        xb      = _mm256_loadu_ps(b + k);
        adiff   = _mm256_sub_ps(xa, amean);
        bdiff   = _mm256_sub_ps(xb, bmean);
        adiffsq = _mm256_mul_ps(adiff, adiff);
        bdiffsq = _mm256_mul_ps(bdiff, bdiff);
        abdiff  = _mm256_mul_ps(adiff, bdiff);
        aaccum  = _mm256_add_ps(aaccum, adiffsq);
        baccum  = _mm256_add_ps(baccum, bdiffsq);
        cov     = _mm256_add_ps(cov, abdiff);
    }
    float mcov[8];
    _mm256_storeu_ps(mcov, cov);
    float fcov = mcov[0] + mcov[1] + mcov[2] + mcov[3] + mcov[4] + mcov[5] + mcov[6] + mcov[7];
    float maaccum[8];
    _mm256_storeu_ps(maaccum, aaccum);
    float faaccum = maaccum[0] + maaccum[1] + maaccum[2] + maaccum[3] + maaccum[4] + maaccum[5] +
        maaccum[6] + maaccum[7];
    float mbaccum[8];
    _mm256_storeu_ps(mbaccum, baccum);
    float fbaccum = mbaccum[0] + mbaccum[1] + mbaccum[2] + mbaccum[3] + mbaccum[4] + mbaccum[5] +
        mbaccum[6] + mbaccum[7];

    for (; k < num; k += 1) {
        float xa      = a[k];
        float xb      = b[k];
        float adiff   = xa - samean;
        float bdiff   = xb - sbmean;
        float adiffsq = adiff * adiff;
        float bdiffsq = bdiff * bdiff;
        float abdiff  = adiff * bdiff;
        faaccum += adiffsq;
        fbaccum += bdiffsq;
        fcov += abdiff;
    }
    float corr = fcov / std::sqrt(faaccum * fbaccum);

    return corr;
}

#endif

}
} // namespace mf::detail

#endif // INCLUDED_mainframe_simd_h
