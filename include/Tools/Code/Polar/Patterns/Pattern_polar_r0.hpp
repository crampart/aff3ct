/*!
 * \file
 * \brief Class tools::Pattern_polar_r0.
 */
#ifndef PATTERN_POLAR_RATE_0_HPP_
#define PATTERN_POLAR_RATE_0_HPP_

#include <string>

#include "Tools/Algo/Tree/Binary/Binary_node.hpp"
#include "Tools/Code/Polar/Patterns/Pattern_polar_i.hpp"

namespace aff3ct
{
namespace tools
{
class Pattern_polar_r0 : public Pattern_polar_i
{
  protected:
    inline Pattern_polar_r0(const int& N,
                            const Binary_node<Pattern_polar_i>* node,
                            const int min_level = 0,
                            const int max_level = -1);

  public:
    inline Pattern_polar_r0(const int min_level = 0, const int max_level = -1);

    virtual inline Pattern_polar_r0* alloc(const int& N, const Binary_node<Pattern_polar_i>* node) const;

    virtual ~Pattern_polar_r0() = default;

    virtual inline polar_node_t type() const;
    virtual inline std::string name() const;
    virtual inline std::string short_name() const;
    virtual inline std::string fill_color() const;
    virtual inline std::string font_color() const;

    virtual inline std::string f() const;
    virtual inline std::string g() const;
    virtual inline std::string h() const;

    static inline bool recursive_check(const int reverse_graph_depth, const Binary_node<Pattern_polar_i>* node_curr);

    virtual inline int _match(const int& reverse_graph_depth, const Binary_node<Pattern_polar_i>* node_curr) const;

    virtual inline bool is_terminal() const;
};
}
}

#ifndef DOXYGEN_SHOULD_SKIP_THIS
#include "Tools/Code/Polar/Patterns/Pattern_polar_r0.hxx"
#endif

#endif /* PATTERN_POLAR_RATE_0_HPP_ */
