#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#endif
#include <cmath>
#include <mipp.h>
#include <sstream>
#include <streampu.hpp>
#include <string>

#include "Module/Decoder/RSC/BCJR/Intra/Decoder_RSC_BCJR_intra_fast.hpp"

namespace aff3ct
{
namespace module
{
template<typename B, typename R, tools::proto_max_i<R> MAX>
Decoder_RSC_BCJR_intra_fast<B, R, MAX>::Decoder_RSC_BCJR_intra_fast(const int& K,
                                                                    const std::vector<std::vector<int>>& trellis,
                                                                    const bool buffered_encoding)
  : Decoder_RSC_BCJR_intra<B, R>(K, trellis, buffered_encoding)
{
    const std::string name = "Decoder_RSC_BCJR_intra_fast";
    this->set_name(name);

    if (mipp::nElReg<R>() != 8)
    {
        std::stringstream message;
        message << "'mipp::nElReg<R>()' has to be equal to 8 ('mipp::nElReg<R>()' = " << mipp::nElReg<R>() << ").";
        throw spu::tools::runtime_error(__FILE__, __LINE__, __func__, message.str());
    }

    if (K % 8)
    {
        std::stringstream message;
        message << "'K' has to be divisible by 8 ('K' = " << K << ").";
        throw spu::tools::invalid_argument(__FILE__, __LINE__, __func__, message.str());
    }
}

template<typename B, typename R, tools::proto_max_i<R> MAX>
Decoder_RSC_BCJR_intra_fast<B, R, MAX>*
Decoder_RSC_BCJR_intra_fast<B, R, MAX>::clone() const
{
    auto m = new Decoder_RSC_BCJR_intra_fast(*this);
    m->deep_copy(*this);
    return m;
}

template<typename B, typename R, tools::proto_max_i<R> MAX>
void
Decoder_RSC_BCJR_intra_fast<B, R, MAX>::compute_gamma(const R* sys, const R* par)
{
    // compute gamma values
    for (auto i = 0; i < this->K + 3; i += mipp::nElReg<R>())
    {
        const auto r_sys = mipp::Reg<R>(&sys[i]);
        const auto r_par = mipp::Reg<R>(&par[i]);

        // there is a big loss of precision here in fixed point
        // compute the two required gamma values
        const auto r_g0 = RSC_BCJR_intra_div_or_not<R>::apply(r_sys + r_par);
        const auto r_g1 = RSC_BCJR_intra_div_or_not<R>::apply(r_sys - r_par);

        const auto r_g0g1 = mipp::interleave(r_g0, r_g1);

        r_g0g1.val[0].store(&this->gamma[i * 2 + 0 * mipp::nElReg<R>()]);
        r_g0g1.val[1].store(&this->gamma[i * 2 + 1 * mipp::nElReg<R>()]);
    }
}

template<typename B, typename R, tools::proto_max_i<R> MAX>
void
Decoder_RSC_BCJR_intra_fast<B, R, MAX>::compute_alpha()
{
    constexpr unsigned cmask_a0[8] = { 0, 3, 4, 7, 1, 2, 5, 6 };   // alpha trellis transitions 0.
    constexpr unsigned cmask_a1[8] = { 1, 2, 5, 6, 0, 3, 4, 7 };   // alpha trellis transitions 1.
    constexpr unsigned cmask_ga0[8] = { 0, 1, 1, 0, 0, 1, 1, 0 };  // mask0 to construct the gamma0/1 vector.
    constexpr unsigned cmask_ga1[8] = { 2, 3, 3, 2, 2, 3, 3, 2 };  // mask1 to construct the gamma0/1 vector.
    constexpr unsigned cmask_ga2[8] = { 4, 5, 5, 4, 4, 5, 5, 4 };  // mask2 to construct the gamma0/1 vector.
    constexpr unsigned cmask_ga3[8] = { 6, 7, 7, 6, 6, 7, 7, 6 };  // mask3 to construct the gamma0/1 vector.
    constexpr unsigned cmask_norm[8] = { 0, 0, 0, 0, 0, 0, 0, 0 }; // mask to broadcast the first alpha value in the
                                                                   // normalization process.
    const auto r_cmask_a0 = mipp::Reg<R>::cmask(cmask_a0);
    const auto r_cmask_a1 = mipp::Reg<R>::cmask(cmask_a1);
    const auto r_cmask_g0 = mipp::Reg<R>::cmask(cmask_ga0);
    const auto r_cmask_g1 = mipp::Reg<R>::cmask(cmask_ga1);
    const auto r_cmask_g2 = mipp::Reg<R>::cmask(cmask_ga2);
    const auto r_cmask_g3 = mipp::Reg<R>::cmask(cmask_ga3);
    const auto r_cmask_norm = mipp::Reg<R>::cmask(cmask_norm);

    auto r_a_prev = mipp::Reg<R>(&this->alpha[0]);
    for (auto i = 0; i < this->K + 3; i += 4)
    {
        // load 4 gamma0 and 4 gamma1
        auto r_g4 = mipp::Reg<R>(&this->gamma[i * 2]);

        // compute alpha[0..8] for section i+0
        const auto r_g__0 = r_g4.shuff(r_cmask_g0);
        const auto r_a0__0 = r_a_prev.shuff(r_cmask_a0);
        const auto r_a1__0 = r_a_prev.shuff(r_cmask_a1);
        auto r_a__0 = MAX(r_a0__0 + r_g__0, r_a1__0 - r_g__0);
        r_a__0 = RSC_BCJR_intra_normalize<R, 0>::apply(r_a__0, r_cmask_norm);
        r_a__0.store(&this->alpha[(i + 1) * 8]);

        // compute alpha[0..8] for section i+1
        const auto r_g__1 = r_g4.shuff(r_cmask_g1);
        const auto r_a0__1 = r_a__0.shuff(r_cmask_a0);
        const auto r_a1__1 = r_a__0.shuff(r_cmask_a1);
        auto r_a__1 = MAX(r_a0__1 + r_g__1, r_a1__1 - r_g__1);
        r_a__1 = RSC_BCJR_intra_normalize<R, 1>::apply(r_a__1, r_cmask_norm);
        r_a__1.store(&this->alpha[(i + 2) * 8]);

        // compute alpha[0..8] for section i+2
        const auto r_g__2 = r_g4.shuff(r_cmask_g2);
        const auto r_a0__2 = r_a__1.shuff(r_cmask_a0);
        const auto r_a1__2 = r_a__1.shuff(r_cmask_a1);
        auto r_a__2 = MAX(r_a0__2 + r_g__2, r_a1__2 - r_g__2);
        r_a__2 = RSC_BCJR_intra_normalize<R, 2>::apply(r_a__2, r_cmask_norm);
        r_a__2.store(&this->alpha[(i + 3) * 8]);

        // compute alpha[0..8] for section i+3
        const auto r_g__3 = r_g4.shuff(r_cmask_g3);
        const auto r_a0__3 = r_a__2.shuff(r_cmask_a0);
        const auto r_a1__3 = r_a__2.shuff(r_cmask_a1);
        auto r_a__3 = MAX(r_a0__3 + r_g__3, r_a1__3 - r_g__3);
        r_a__3 = RSC_BCJR_intra_normalize<R, 3>::apply(r_a__3, r_cmask_norm);
        r_a__3.store(&this->alpha[(i + 4) * 8]);

        r_a_prev = r_a__3;
    }
}

template<typename B, typename R, tools::proto_max_i<R> MAX>
void
Decoder_RSC_BCJR_intra_fast<B, R, MAX>::compute_beta_ext(const R* sys, R* ext)
{
    constexpr unsigned cmask_b0[8] = { 0, 4, 5, 1, 2, 6, 7, 3 };   // beta trellis transitions 0.
    constexpr unsigned cmask_b1[8] = { 4, 0, 1, 5, 6, 2, 3, 7 };   // beta trellis transitions 1.
    constexpr unsigned cmask_g0[8] = { 0, 0, 1, 1, 1, 1, 0, 0 };   // mask0 to construct the gamma0/1 vector.
    constexpr unsigned cmask_g1[8] = { 2, 2, 3, 3, 3, 3, 2, 2 };   // mask1 to construct the gamma0/1 vector.
    constexpr unsigned cmask_g2[8] = { 4, 4, 5, 5, 5, 5, 4, 4 };   // mask2 to construct the gamma0/1 vector.
    constexpr unsigned cmask_g3[8] = { 6, 6, 7, 7, 7, 7, 6, 6 };   // mask3 to construct the gamma0/1 vector.
    constexpr unsigned cmask_norm[8] = { 0, 0, 0, 0, 0, 0, 0, 0 }; // mask to broadcast the first alpha value in the
                                                                   // normalization process.
    const auto r_cmask_b0 = mipp::Reg<R>::cmask(cmask_b0);
    const auto r_cmask_b1 = mipp::Reg<R>::cmask(cmask_b1);
    const auto r_cmask_norm = mipp::Reg<R>::cmask(cmask_norm);
    const auto r_cmask_g0 = mipp::Reg<R>::cmask(cmask_g0);
    const auto r_cmask_g1 = mipp::Reg<R>::cmask(cmask_g1);
    const auto r_cmask_g2 = mipp::Reg<R>::cmask(cmask_g2);
    const auto r_cmask_g3 = mipp::Reg<R>::cmask(cmask_g3);

    // compute the first beta values
    auto r_b_prev = mipp::Reg<R>(&this->alpha[0]);
    auto r_g4_bis = mipp::Reg<R>(&this->gamma[this->K * 2]);
    mipp::Reg<R> r_cmask_g[4] = { r_cmask_g0, r_cmask_g1, r_cmask_g2, r_cmask_g3 };
    for (unsigned i = (unsigned)(this->K + 2); i >= (unsigned)this->K; i--)
    {
        const auto r_b0 = r_b_prev.shuff(r_cmask_b0);
        const auto r_b1 = r_b_prev.shuff(r_cmask_b1);
        const auto r_g = r_g4_bis.shuff(r_cmask_g[i % 4]);
        r_b_prev = MAX(r_b0 + r_g, r_b1 - r_g);

        // normalization
        r_b_prev = RSC_BCJR_intra_normalize<R>::apply(r_b_prev, r_cmask_norm, i);
    }

    // compute beta values and the extrinsic values [trellis backward traversal <-] (vectorized)
    for (auto i = this->K - 8; i >= 0; i -= 8)
    {
        // load 4 gamma0 and 4 gamma1
        auto r_g4 = mipp::Reg<R>(&this->gamma[(i + 4) * 2]);
        mipp::Reg<R> r_max0[8], r_max1[8];

        // compute beta[0..8] for section i+7
        const auto r_a__7 = mipp::Reg<R>(&this->alpha[(i + 7) * 8]);
        const auto r_g__7 = r_g4.shuff(r_cmask_g3);
        const auto r_b0__7 = r_b_prev.shuff(r_cmask_b0);
        const auto r_b1__7 = r_b_prev.shuff(r_cmask_b1);
        auto r_b__7 = MAX(r_b0__7 + r_g__7, r_b1__7 - r_g__7);
        r_b__7 = RSC_BCJR_intra_normalize<R, 0>::apply(r_b__7, r_cmask_norm);
        // buffer the alpha+beta+gamma for the section i+7
        r_max0[7] = r_a__7 + r_b0__7 + r_g__7;
        r_max1[7] = r_a__7 + r_b1__7 - r_g__7;

        // compute beta[0..8] for section i+6
        const auto r_a__6 = mipp::Reg<R>(&this->alpha[(i + 6) * 8]);
        const auto r_g__6 = r_g4.shuff(r_cmask_g2);
        const auto r_b0__6 = r_b__7.shuff(r_cmask_b0);
        const auto r_b1__6 = r_b__7.shuff(r_cmask_b1);
        auto r_b__6 = MAX(r_b0__6 + r_g__6, r_b1__6 - r_g__6);
        r_b__6 = RSC_BCJR_intra_normalize<R, 1>::apply(r_b__6, r_cmask_norm);
        // buffer the alpha+beta+gamma for the section i+6
        r_max0[6] = r_a__6 + r_b0__6 + r_g__6;
        r_max1[6] = r_a__6 + r_b1__6 - r_g__6;

        // compute beta[0..8] for section i+5
        const auto r_a__5 = mipp::Reg<R>(&this->alpha[(i + 5) * 8]);
        const auto r_g__5 = r_g4.shuff(r_cmask_g1);
        const auto r_b0__5 = r_b__6.shuff(r_cmask_b0);
        const auto r_b1__5 = r_b__6.shuff(r_cmask_b1);
        auto r_b__5 = MAX(r_b0__5 + r_g__5, r_b1__5 - r_g__5);
        r_b__5 = RSC_BCJR_intra_normalize<R, 2>::apply(r_b__5, r_cmask_norm);
        // buffer the alpha+beta+gamma for the section i+5
        r_max0[5] = r_a__5 + r_b0__5 + r_g__5;
        r_max1[5] = r_a__5 + r_b1__5 - r_g__5;

        // compute beta[0..8] for section i+4
        const auto r_a__4 = mipp::Reg<R>(&this->alpha[(i + 4) * 8]);
        const auto r_g__4 = r_g4.shuff(r_cmask_g0);
        const auto r_b0__4 = r_b__5.shuff(r_cmask_b0);
        const auto r_b1__4 = r_b__5.shuff(r_cmask_b1);
        auto r_b__4 = MAX(r_b0__4 + r_g__4, r_b1__4 - r_g__4);
        r_b__4 = RSC_BCJR_intra_normalize<R, 3>::apply(r_b__4, r_cmask_norm);
        // buffer the alpha+beta+gamma for the section i+4
        r_max0[4] = r_a__4 + r_b0__4 + r_g__4;
        r_max1[4] = r_a__4 + r_b1__4 - r_g__4;

        // load 4 gamma0 and 4 gamma1
        r_g4.load(&this->gamma[(i + 0) * 2]);

        // compute beta[0..8] for section i+3
        const auto r_a__3 = mipp::Reg<R>(&this->alpha[(i + 3) * 8]);
        const auto r_g__3 = r_g4.shuff(r_cmask_g3);
        const auto r_b0__3 = r_b__4.shuff(r_cmask_b0);
        const auto r_b1__3 = r_b__4.shuff(r_cmask_b1);
        auto r_b__3 = MAX(r_b0__3 + r_g__3, r_b1__3 - r_g__3);
        r_b__3 = RSC_BCJR_intra_normalize<R, 0>::apply(r_b__3, r_cmask_norm);
        // buffer the alpha+beta+gamma for the section i+3
        r_max0[3] = r_a__3 + r_b0__3 + r_g__3;
        r_max1[3] = r_a__3 + r_b1__3 - r_g__3;

        // compute beta[0..8] for section i+2
        const auto r_a__2 = mipp::Reg<R>(&this->alpha[(i + 2) * 8]);
        const auto r_g__2 = r_g4.shuff(r_cmask_g2);
        const auto r_b0__2 = r_b__3.shuff(r_cmask_b0);
        const auto r_b1__2 = r_b__3.shuff(r_cmask_b1);
        auto r_b__2 = MAX(r_b0__2 + r_g__2, r_b1__2 - r_g__2);
        r_b__2 = RSC_BCJR_intra_normalize<R, 1>::apply(r_b__2, r_cmask_norm);
        // buffer the alpha+beta+gamma for the section i+2
        r_max0[2] = r_a__2 + r_b0__2 + r_g__2;
        r_max1[2] = r_a__2 + r_b1__2 - r_g__2;

        // compute beta[0..8] for section i+1
        const auto r_a__1 = mipp::Reg<R>(&this->alpha[(i + 1) * 8]);
        const auto r_g__1 = r_g4.shuff(r_cmask_g1);
        const auto r_b0__1 = r_b__2.shuff(r_cmask_b0);
        const auto r_b1__1 = r_b__2.shuff(r_cmask_b1);
        auto r_b__1 = MAX(r_b0__1 + r_g__1, r_b1__1 - r_g__1);
        r_b__1 = RSC_BCJR_intra_normalize<R, 2>::apply(r_b__1, r_cmask_norm);
        // buffer the alpha+beta+gamma for the section i+1
        r_max0[1] = r_a__1 + r_b0__1 + r_g__1;
        r_max1[1] = r_a__1 + r_b1__1 - r_g__1;

        // compute beta[0..8] for section i+1
        const auto r_a__0 = mipp::Reg<R>(&this->alpha[(i + 0) * 8]);
        const auto r_g__0 = r_g4.shuff(r_cmask_g0);
        const auto r_b0__0 = r_b__1.shuff(r_cmask_b0);
        const auto r_b1__0 = r_b__1.shuff(r_cmask_b1);
        auto r_b__0 = MAX(r_b0__0 + r_g__0, r_b1__0 - r_g__0);
        r_b_prev = r_b__0 = RSC_BCJR_intra_normalize<R, 3>::apply(r_b__0, r_cmask_norm);
        // buffer the alpha+beta+gamma for the section i+0
        r_max0[0] = r_a__0 + r_b0__0 + r_g__0;
        r_max1[0] = r_a__0 + r_b1__0 - r_g__0;

        // transpose the buffered vector
        mipp::Reg<R>::transpose(r_max0);
        mipp::Reg<R>::transpose(r_max1);

        // perform the final MAX operations in parallel
        r_max0[0] = MAX(r_max0[0], r_max0[1]);
        r_max1[0] = MAX(r_max1[0], r_max1[1]);
        r_max0[1] = MAX(r_max0[2], r_max0[3]);
        r_max1[1] = MAX(r_max1[2], r_max1[3]);
        r_max0[2] = MAX(r_max0[4], r_max0[5]);
        r_max1[2] = MAX(r_max1[4], r_max1[5]);
        r_max0[3] = MAX(r_max0[6], r_max0[7]);
        r_max1[3] = MAX(r_max1[6], r_max1[7]);

        r_max0[0] = MAX(r_max0[0], r_max0[1]);
        r_max1[0] = MAX(r_max1[0], r_max1[1]);
        r_max0[1] = MAX(r_max0[2], r_max0[3]);
        r_max1[1] = MAX(r_max1[2], r_max1[3]);

        r_max0[0] = MAX(r_max0[0], r_max0[1]);
        r_max1[0] = MAX(r_max1[0], r_max1[1]);

        // saturate r_post if the computation are made in 8-bit, do nothing else.
        auto r_post = RSC_BCJR_intra_post<R>::compute(r_max0[0] - r_max1[0]);

        // store the extrinsic values
        const auto r_ext = r_post - &sys[i];
        r_ext.store(&ext[i]);
    }
}
}
}
