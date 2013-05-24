#include "Cipher.hxx"

std::array<byte, 4> const mixColCoefficientsCipher       { 2,   3,   1,   1   },
                          mixColCoefficientsInverseCipher{ 0xE, 0xB, 0xD, 0x9 };


void AddRoundKey( state_type_reference state,
                  word const* w)
{
    for( size_t r = 0; r < 4; ++r )
        for( size_t c = 0; c < Nb; ++c )
            state[r][c] ^= get_byte(w[c], 4-r-1);
}

void SubBytes( state_type_reference state, bool inverse )
{
    /// Replace all bytes B_i by B_i' = func(B_i)
    std::transform( reinterpret_cast<byte*>(state),
                    reinterpret_cast<byte*>(state) + 4*Nb,
                    reinterpret_cast<byte*>(state),
                    inverse ? InvSubByte : SubByte );
}

/// direction=false is left-shift, and true is right-shift.
 // (true for inverse cipher)
void ShiftRows( state_type_reference state, bool direction )
{
    auto shift_func = ( direction ? cyclicBitLeftShift : cyclicBitRightShift );
    for( unsigned r = 1; r < 4; ++r )
    {
        word& w = *reinterpret_cast<word*>(state[r]);
        w = shift_func( w, 8*(4-r) );
    }
}

byte gf28_mul( byte lhs, byte rhs )
{
    byte p{0};

    for( unsigned i = 0; i < 8 ; ++i )
    {
        if( rhs & 1 )
            p ^= lhs;

        rhs >>= 1;

        bool carry = lhs & 0x80;

        lhs <<= 1;

        if( carry )
            lhs ^= 0x1B;
    }

    return p;
}

void MixColumns( state_type_reference state, std::array<byte, 4> const& coeff )
{
    for( unsigned col = 0; col < Nb; ++col )
    {
        byte const a = state[0][col],
                   b = state[1][col],
                   c = state[2][col],
                   d = state[3][col];

        state[0][col] = gf28_mul(coeff[0], a) ^ gf28_mul(coeff[1], b) ^ gf28_mul(coeff[2], c) ^ gf28_mul(coeff[3], d);
        state[1][col] = gf28_mul(coeff[3], a) ^ gf28_mul(coeff[0], b) ^ gf28_mul(coeff[1], c) ^ gf28_mul(coeff[2], d);
        state[2][col] = gf28_mul(coeff[2], a) ^ gf28_mul(coeff[3], b) ^ gf28_mul(coeff[0], c) ^ gf28_mul(coeff[1], d);
        state[3][col] = gf28_mul(coeff[1], a) ^ gf28_mul(coeff[2], b) ^ gf28_mul(coeff[3], c) ^ gf28_mul(coeff[0], d);
    }
}
