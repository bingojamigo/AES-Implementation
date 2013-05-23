#include "Cipher.hxx"

std::array<byte, 4> const mixColCoefficientsCipher       { 2,   3,   1,   1   },
                          mixColCoefficientsInverseCipher{ 0xE, 0xB, 0xD, 0x9 };

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
    auto shift_func = ( direction ? cyclicBitLeftShift<word> : cyclicBitRightShift<word> );
    for( unsigned r = 1; r < 4; ++r )
    {
        word& w = *reinterpret_cast<word*>(state[r]);
        w = shift_func( w, 8*(4-r) );
    }
}

void MixColumns( state_type_reference state, std::array<byte, 4> const& coeff )
{
    for( unsigned col = 0; col < Nb; ++col )
    {
        GF8_int const a = state[0][col],
                      b = state[1][col],
                      c = state[2][col],
                      d = state[3][col];

        state[0][col] = (coeff[0]*a + coeff[1]*b + coeff[2]*c + coeff[3]*d).value;
        state[1][col] = (coeff[3]*a + coeff[0]*b + coeff[1]*c + coeff[2]*d).value;
        state[2][col] = (coeff[2]*a + coeff[3]*b + coeff[0]*c + coeff[1]*d).value;
        state[3][col] = (coeff[1]*a + coeff[2]*b + coeff[3]*c + coeff[0]*d).value;
    }
}
