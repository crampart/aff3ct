/*!
 * \file
 * \brief Class tools::Flip_and_check_DB.
 */
#ifndef FLIP_AND_CHECK_DB_HPP
#define FLIP_AND_CHECK_DB_HPP

#include <mipp.h>
#include <vector>

#include "Module/CRC/CRC.hpp"
#include "Tools/Code/Turbo/Post_processing_SISO/CRC/CRC_checker_DB.hpp"

namespace aff3ct
{
namespace tools
{
template<typename B = int, typename R = float>
class Flip_and_check_DB : public CRC_checker_DB<B, R>
{
  private:
    const int K;
    const int q;
    mipp::vector<R> metric;
    mipp::vector<B> s_tmp;
    mipp::vector<mipp::vector<B>> tab_flips; // contains all the binary masks (ie: 1000, 1100 ...)
    mipp::vector<B> fnc_ite;

    std::vector<int> symb_sorted;

  public:
    Flip_and_check_DB(const int K,
                      const int n_ite,
                      const module::CRC<B>& crc,
                      const int start_crc_check_ite,
                      const int q,
                      const int m = 1,
                      const int M = -1,
                      const int s = 1);

    virtual ~Flip_and_check_DB() = default;

    virtual Flip_and_check_DB<B, R>* clone() const;

    bool siso_n(const int ite, const mipp::vector<R>& sys, mipp::vector<R>& ext, mipp::vector<B>& s);

    virtual void set_n_frames(const size_t n_frames);

  private:
    bool apply_flip_and_check(const mipp::vector<R>& sys, const mipp::vector<R>& ext, mipp::vector<B>& s);
};
}
}

#endif /* FLIP_AND_CHECK_HPP */
