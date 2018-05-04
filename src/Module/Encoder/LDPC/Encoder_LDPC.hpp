#ifndef ENCODER_LDPC_HPP_
#define ENCODER_LDPC_HPP_

#include <vector>

#include "Tools/Algo/Sparse_matrix/Sparse_matrix.hpp"

#include "../Encoder.hpp"

namespace aff3ct
{
namespace module
{

template <typename B = int>
class Encoder_LDPC : public Encoder<B>
{
protected:
	tools::Sparse_matrix G; // the generator matrix (automatically transposed if needed in constructor)
	                        // G cols are the K dimension
	                        // G rows are the N dimension
	tools::Sparse_matrix H; // the decodeur matrix (automatically transposed if needed in constructor)
	                        // H cols are the M dimension (often M = N - K)
	                        // H rows are the N dimension

protected:
	Encoder_LDPC(const int K, const int N, const int n_frames = 1);

public:
	Encoder_LDPC(const int K, const int N, const tools::Sparse_matrix &G, const int n_frames = 1);
	Encoder_LDPC(const int K, const int N, const tools::Sparse_matrix &G, const tools::Sparse_matrix &H, const int n_frames = 1);
	virtual ~Encoder_LDPC() = default;

	virtual const std::vector<uint32_t>& get_info_bits_pos() const;
	virtual bool is_sys() const;
	virtual bool is_codeword(const B *X_N);

protected:
	virtual void _encode(const B *U_K, B *X_N, const int frame_id);

	void check_G_dimensions();
	void check_H_dimensions();
	virtual void _check_G_dimensions();
	virtual void _check_H_dimensions();
};

}
}

#endif /* ENCODER_LDPC_HPP_ */
