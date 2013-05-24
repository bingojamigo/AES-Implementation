#ifndef BASIC_HXX_INCLUDED
#define BASIC_HXX_INCLUDED

#include <type_traits> // enable_if

#define REQUIRES(...) ,class=typename std::enable_if<(__VA_ARGS__)>::type
#define AUTO_RETURN(...) decltype(__VA_ARGS__) { return (__VA_ARGS__); }

#include <cstdint>

typedef uint8_t byte;
typedef uint32_t word;

inline word word_from( byte a, byte b, byte c, byte d )
{
    return a
         | static_cast<word>(b) << 8
         | static_cast<word>(c) << 16
         | static_cast<word>(d) << 24;
}

inline byte get_byte( word w, uint8_t N )
{
    return w >> N*8;
}

inline word cyclicBitLeftShift( word val, std::size_t width )
{
    return (val << width) | val >> (32 - width);
}

inline word cyclicBitRightShift( word val, std::size_t width )
{
    return (val >> width) | val << (32 - width);
}

#endif // BASIC_HXX_INCLUDED
