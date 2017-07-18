#include <iostream>

#include "Tools/Codec/Polar/Codec_polar.hpp"
#include "Launcher_polar.hpp"

namespace aff3ct
{
namespace launcher
{
template <class C, typename B, typename R, typename Q>
Launcher_polar<C,B,R,Q>
::Launcher_polar(const int argc, const char **argv, std::ostream &stream)
: C(argc, argv, stream)
{
	params_fb  = new tools::Factory_frozenbits_generator::parameters();
	params_pct = new tools::Factory_puncturer_polar     ::parameters();
	params_enc = new tools::Factory_encoder_polar       ::parameters();
	params_dec = new tools::Factory_decoder_polar       ::parameters();

	if (this->params->enc != nullptr) { delete this->params->enc; this->params->enc = params_enc; }
	if (this->params->pct != nullptr) { delete this->params->pct; this->params->pct = params_pct; }
	if (this->params->dec != nullptr) { delete this->params->dec; this->params->dec = params_dec; }
}

template <class C, typename B, typename R, typename Q>
Launcher_polar<C,B,R,Q>
::~Launcher_polar()
{
	if (params_fb != nullptr)
	{
		delete params_fb;
		params_fb = nullptr;
	}
}

template <class C, typename B, typename R, typename Q>
void Launcher_polar<C,B,R,Q>
::build_args()
{
	tools::Factory_frozenbits_generator::build_args(this->req_args, this->opt_args);
	tools::Factory_encoder_polar       ::build_args(this->req_args, this->opt_args);
	tools::Factory_puncturer_polar     ::build_args(this->req_args, this->opt_args);
	tools::Factory_decoder_polar       ::build_args(this->req_args, this->opt_args);

	this->opt_args.erase({"pct-fra",       "F"});
	this->req_args.erase({"enc-cw-size",   "N"});
	this->req_args.erase({"enc-info-bits", "K"});
	this->req_args.erase({"enc-info-bits", "K"});
	this->opt_args.erase({"enc-fra",       "F"});
	this->opt_args.erase({"enc-fb-sigma"      });
	this->opt_args.erase({"enc-seed",      "S"});
	this->req_args.erase({"dec-cw-size",   "N"});
	this->req_args.erase({"dec-info-bits", "K"});
	this->opt_args.erase({"dec-fra",       "F"});
	this->opt_args.erase({"dec-no-sys"        });

	C::build_args();
}

template <class C, typename B, typename R, typename Q>
void Launcher_polar<C,B,R,Q>
::store_args()
{
	tools::Factory_puncturer_polar::store_args(this->ar, *params_pct);

	params_fb->K    = params_pct->K;
	params_fb->N_cw = params_pct->N_cw;

	tools::Factory_frozenbits_generator::store_args(this->ar, *params_fb);

	params_enc->K    = params_pct->K;
	params_enc->N_cw = params_pct->N_cw;

	tools::Factory_encoder_polar::store_args(this->ar, *params_enc);

	params_dec->K          = params_pct->K;
	params_dec->N_cw       = params_pct->N_cw;
	params_dec->systematic = params_enc->systematic;

	tools::Factory_decoder_polar::store_args(this->ar, *params_dec);

	C::store_args();
}

template <class C, typename B, typename R, typename Q>
void Launcher_polar<C,B,R,Q>
::group_args()
{
	tools::Factory_frozenbits_generator::group_args(this->arg_group);
	tools::Factory_puncturer_polar     ::group_args(this->arg_group);
	tools::Factory_encoder_polar       ::group_args(this->arg_group);
	tools::Factory_decoder_polar       ::group_args(this->arg_group);

	C::group_args();
}

template <class C, typename B, typename R, typename Q>
void Launcher_polar<C,B,R,Q>
::print_header()
{
	tools::Factory_encoder_polar       ::header(this->pl_enc, *params_enc);
	tools::Factory_puncturer_polar     ::header(this->pl_pct, *params_pct);
	tools::Factory_frozenbits_generator::header(this->pl_enc, *params_fb);
	tools::Factory_decoder_polar       ::header(this->pl_dec, *params_dec);

	C::print_header();
}

template <class C, typename B, typename R, typename Q>
void Launcher_polar<C,B,R,Q>
::build_codec()
{
	this->codec = new tools::Codec_polar<B,Q>(*params_fb, *params_pct, *params_enc, *params_dec, this->params->n_threads);
}
}
}
