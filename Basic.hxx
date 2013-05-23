#ifndef BASIC_HXX_INCLUDED
#define BASIC_HXX_INCLUDED

#include <cstdint>

#define REQUIRES(...) ,class=typename std::enable_if<(__VA_ARGS__)>::type
#define AUTO_RETURN(...) decltype(__VA_ARGS__) { return (__VA_ARGS__); }

typedef uint8_t byte;
typedef uint32_t word;

inline word word_from( byte a, byte b, byte c, byte d )
{
    return a
         | static_cast<word>(b) << 8
         | static_cast<word>(c) << 16
         | static_cast<word>(d) << 24;
}

#include <climits>

inline byte get_byte( word w, uint8_t N )
{
    return w >> N*8;
}

template<typename Int>
Int cyclicBitLeftShift( Int val, std::size_t width )
{
    return (val << width) | val >> (sizeof(Int)*CHAR_BIT - width);
}

template<typename Int>
Int cyclicBitRightShift( Int val, std::size_t width )
{
    return (val >> width) | val << (sizeof(Int)*CHAR_BIT - width);
}

inline word reverseBytes( word a )
{
    return word_from( a >> 24,
                      a >> 16,
                      a >> 8,
                      a );
}

#endif // BASIC_HXX_INCLUDED
