#ifndef ARRAYS_HXX_INCLUDED
#define ARRAYS_HXX_INCLUDED

#include "Basic.hxx"

#include <array>

extern std::array<byte, 256> const Rcon;
extern std::array<byte, 256> const SBox;
extern std::array<byte, 256> const InverseSBox;

inline word getRcon(size_t i)
{
    return word(Rcon[i]) << 24;
}

inline byte SubByte( byte b )
{
    return SBox[b];
}

inline word SubWord(word w)
{
    return word_from( SBox[ get_byte(w, 0) ],
                      SBox[ get_byte(w, 1) ],
                      SBox[ get_byte(w, 2) ],
                      SBox[ get_byte(w, 3) ] );
}

inline byte InvSubByte( byte b )
{
    return InverseSBox[b];
}

#endif // ARRAYS_HXX_INCLUDED
