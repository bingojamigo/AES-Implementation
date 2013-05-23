#ifndef CIPHER_HXX_INCLUDED
#define CIPHER_HXX_INCLUDED

#include <algorithm>
#include <iostream>
#include <iterator>

#include "GF8_int.hxx"

typedef byte state_type[4][Nb];
typedef byte (&state_type_reference)[4][Nb];
typedef byte const (&state_type_const_reference)[4][Nb];
typedef byte (*state_type_pointer)[4][Nb];

template<KeySize key_size>
using expanded_key_type = std::array<word, Nb*(Nr(key_size)+1)> ;

/// The coefficients of the matrix.
std::array<byte, 4> const mixColCoefficientsCipher       { 2,   3,   1,   1   },
                          mixColCoefficientsInverseCipher{ 0xE, 0xB, 0xD, 0x9 };

/**
 * \param state The state-Array
 * \param w A pointer to Nb consecutive words in memory who are the keys
 */
template<typename KeyIter>
void AddRoundKey( state_type_reference state,
                  KeyIter w)
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

template<KeySize Nk>
void KeyExpansion( Key<Nk> const& key, expanded_key_type<Nk>& w )
{
    std::copy( key.begin(), key.end(), w.begin() );

    word temp;
    for( std::size_t i = Nk; i < w.size(); ++i )
    {
        temp = w[i-1];

        if (i % Nk == 0)
            temp = SubWord(cyclicBitLeftShift(temp, 8)) ^ getRcon(i/Nk);
        else if (Nk > 6 && i % Nk == 4)
            temp = SubWord(temp); // TODO: Implement all the functions! Follow the PDF!

        w[i] = temp ^ w[i-Nk];
    }
}

/// /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<bool inputToState,
         typename Iter>
void copy( Iter first, Iter last, state_type_reference state )
{
    for( unsigned r = 0; r < 4; ++r )
        for( unsigned c = 0; c < Nb; ++c )
        {
            auto index = r + 4*c;
            auto iter = first + index;

            if( iter < last )
                if(inputToState)
                    state[r][c] = first[index];
                else
                    first[index] = state[r][c];
        }
}

/// [message, message + 16) is being crypted
template<KeySize Nk,
         typename Iter,
         typename Size,
         typename KeyIter>
void Cipher( Iter message,
             Size maximum, /// message + maximum is considered the end
             KeyIter w )
{
    state_type state;

    copy<true>( message, message + maximum, state );

    AddRoundKey(state, w);

    for( unsigned round = 1; round < Nr(Nk) + 1; ++round )
    {
        SubBytes(state, false);

        ShiftRows(state, false);

        if( round != Nr(Nk) ) // skip mix-columns in last round
            MixColumns(state, mixColCoefficientsCipher);

        AddRoundKey(state, w + round*Nb);
    }

    copy<false>( message, message + maximum, state );
}

template<KeySize Nk,
         typename Iter,
         typename Size,
         typename KeyIter>
void InvCipher( Iter message, /// An iterator pointing to the message
                Size maximum, /// message + maximum is considered the end
                KeyIter w )
{
    state_type state;

    copy<true>( message, message + maximum, state );

    AddRoundKey(state, w + Nr(Nk)*Nb);

    for( int round = Nr(Nk) - 1; round >= 0; --round )
    {
        SubBytes(state, true);

        ShiftRows(state, true);

        AddRoundKey(state, w + round*Nb);

        if( round != 0 ) // skip mix-columns in last round
            MixColumns(state, mixColCoefficientsInverseCipher);
    }


    copy<false>( message, message + maximum, state );
}

/// ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////7

template<KeySize Nk>
class RijndaelCrypt
{
public:

    static constexpr KeySize key_size = Nk;

    typedef Key<key_size> key_type;

    typedef expanded_key_type<key_size> this_expanded_key_type;

private:

    this_expanded_key_type mKeyArray;

public:

    RijndaelCrypt( key_type const& key )
    { KeyExpansion<key_size>( key, mKeyArray ); }

    RijndaelCrypt( this_expanded_key_type const& ex_keys ):
        mKeyArray{ex_keys} {}

    template<typename Iter,
             typename CryptFunc>
    void ApplyCrypt( Iter first, Iter last, CryptFunc crypter )
    {
        int dist = std::distance(first, last);

        while( dist > 0 )
        {
            crypter( first, dist, std::begin(mKeyArray) );
            dist -= 4 * Nb;
            first += 4 * Nb;
        }
    }

    template<typename Iter>
    void Encrypt( Iter first, Iter last )
    { ApplyCrypt(first, last, Cipher<key_size, Iter, int, typename this_expanded_key_type::iterator>); }

    template<typename Iter>
    void Decrypt( Iter first, Iter last )
    { ApplyCrypt(first, last, InvCipher<key_size, Iter, int, typename this_expanded_key_type::iterator>); }

};


#endif // CIPHER_HXX_INCLUDED
