#ifndef GF8_INT_HXX_INCLUDED
#define GF8_INT_HXX_INCLUDED

#include <cstdint>

struct GF8_int;

inline GF8_int operator+( GF8_int a, GF8_int b );
inline GF8_int operator-( GF8_int a, GF8_int b );
inline GF8_int operator*( GF8_int a, GF8_int b );

struct GF8_int
{
    byte value;

    GF8_int( byte val ):
        value{val} {}

    GF8_int& operator*=( GF8_int b )
    {
        return *this = *this * b;
    }

    GF8_int& operator+=( GF8_int b )
    {
        return *this = *this + b;
    }

    GF8_int& operator-=( GF8_int b )
    {
        return *this = *this - b;
    }
};

  /// For dumping HEX-values
inline std::ostream& operator<<( std::ostream& os, GF8_int i )
{
    return os << std::hex << short(i.value);
}

inline GF8_int operator+( GF8_int a, GF8_int b )
{
    return a.value ^ b.value;
}

inline GF8_int operator-( GF8_int a, GF8_int b )
{
    return a + b;
}

inline GF8_int operator*( GF8_int a, GF8_int b )
{
    GF8_int p{0};

    for( unsigned i = 0; i < 8 ; ++i )
    {
        if( b.value & 1 )
            p += a;

        b.value >>= 1;

        bool carry = a.value & 0x80;

        a.value <<= 1;

        if( carry )
            a += 0x1B;
    }

    return p;
}

#endif // GF8_INT_HXX_INCLUDED
