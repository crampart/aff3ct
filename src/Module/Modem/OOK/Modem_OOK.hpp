#ifndef MODEM_OOK_HPP_
#define MODEM_OOK_HPP_

#include "../Modem.hpp"

namespace aff3ct
{
namespace module
{
template <typename B = int, typename R = float, typename Q = R>
class Modem_OOK : public Modem<B,R,Q>
{
protected:
	const bool disable_sig2;
	R sigma_factor;

public:
	Modem_OOK(const int N, const R sigma = (R)1, const bool disable_sig2 = false, const int n_frames = 1,
	           const std::string name = "Modem_OOK");
	virtual ~Modem_OOK();

	void set_sigma(const R sigma);

	void   modulate           (const B *X_N1,                              R *X_N2); using Modem<B,R,Q>::modulate;
	void     filter           (const R *Y_N1,                              R *Y_N2); using Modem<B,R,Q>::filter;
	void demodulate           (const Q *Y_N1,                              Q *Y_N2);
	void demodulate_with_gains(const Q *Y_N1, const R *H_N,                Q *Y_N2);
	void demodulate           (const Q *Y_N1,               const Q *Y_N2, Q *Y_N3); using Modem<B,R,Q>::demodulate;
	void demodulate_with_gains(const Q *Y_N1, const R *H_N, const Q *Y_N2, Q *Y_N3); using Modem<B,R,Q>::demodulate_with_gains;

	static int size_mod(const int N)
	{
		return Modem<B,R,Q>::get_buffer_size_after_modulation(N, 1, 0, 1, false);
	}

	static int size_fil(const int N)
	{
		return Modem<B,R,Q>::get_buffer_size_after_filtering(N, 1, 0, 1, false);
	}
};
}
}

#endif /* MODEM_OOK_HPP_ */
