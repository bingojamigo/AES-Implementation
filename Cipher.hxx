#ifndef CIPHER_HXX_INCLUDED
#define CIPHER_HXX_INCLUDED

#include <algorithm>

#include "Key.hxx"
#include "Arrays.hxx"
#include "GF8_int.hxx"

typedef byte state_type[4][Nb];
typedef byte (&state_type_reference)[4][Nb];
typedef byte const (&state_type_const_reference)[4][Nb];
typedef byte (*state_type_pointer)[4][Nb];

template<KeySize key_size>
using expanded_key_type = std::array<word, Nb*(Nr(key_size)+1)> ;

/// The coefficients of the matrix.
extern std::array<byte, 4> const mixColCoefficientsCipher,
                                 mixColCoefficientsInverseCipher;

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

void SubBytes( state_type_reference state, bool inverse );

/// direction=false is left-shift, and true is right-shift.
 // (true for inverse cipher)
void ShiftRows( state_type_reference state, bool direction );

void MixColumns( state_type_reference state, std::array<byte, 4> const& coeff );

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
