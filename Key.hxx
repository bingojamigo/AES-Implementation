#ifndef KEY_HXX_INCLUDED
#define KEY_HXX_INCLUDED

#include "Basic.hxx"

#include <array>

enum KeySize
{
    Nk_4 = 4,
    Nk_6 = 6,
    Nk_8 = 8
};

auto constexpr Nr( KeySize k ) -> AUTO_RETURN( k + 6 )

constexpr int Nb = 4;

template<KeySize keysize>
using Key = std::array< word, keysize >;

#endif // KEY_HXX_INCLUDED
