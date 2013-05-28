#ifndef CIPHER_HXX_INCLUDED
#define CIPHER_HXX_INCLUDED

#include <algorithm>

#include "Key.hxx"
#include "Arrays.hxx"

typedef byte state_type[4][Nb];
typedef byte (&state_type_reference)[4][Nb];
typedef byte const (&state_type_const_reference)[4][Nb];
typedef byte (*state_type_pointer)[4][Nb];


/**
 * \param state The state-Array
 * \param w A pointer to Nb consecutive words in memory who are the keys
 */
void AddRoundKey( state_type_reference state,
                  word const* w );

void SubBytes( state_type_reference state, bool inverse );

/// direction=false is left-shift, and true is right-shift.
 // (true for inverse cipher)
void ShiftRows( state_type_reference state, bool direction );

void MixColumns( state_type_reference state, std::array<byte, 4> const& coeff );

/// /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<bool inputToState>
void copy( byte* first, byte* last, state_type_reference state )
{
    for( unsigned r = 0; r < 4; ++r )
        for( unsigned c = 0; c < static_cast<unsigned>(Nb); ++c )
        {
            auto index = r + 4*c;
            auto iter = first + index;

            if( iter < last )
            {
                if(inputToState)
                    state[r][c] = first[index];
                else
                    first[index] = state[r][c];
            }
        }
}

/// [message, message + maximum) is being crypted
template<KeySize Nk>
void Cipher( byte* message,
             std::size_t maximum, /// message + maximum is considered the end
             word const* w )
{
    state_type state;

    static std::array<byte, 4> constexpr mixColCoefficientsCipher{{ 2, 3, 1, 1}};

    copy<true>( message, message + maximum, state );

    AddRoundKey(state, w);

    for( unsigned round = 1; round < static_cast<unsigned>(Nr(Nk) + 1); ++round )
    {
        SubBytes(state, false);

        ShiftRows(state, false);

        if( round != static_cast<unsigned>(Nr(Nk)) ) // skip mix-columns in last round
            MixColumns(state, mixColCoefficientsCipher);

        AddRoundKey(state, w + round*Nb);
    }

    copy<false>( message, message + maximum, state );
}

template<KeySize Nk>
void InvCipher( byte* message, /// An iterator pointing to the message
                std::size_t maximum, /// message + maximum is considered the end
                word const* w )
{
    state_type state;

    static std::array<byte, 4> constexpr mixColCoefficientsInverseCipher{{ 0xE, 0xB, 0xD, 0x9 }};

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

    typedef Key<Nk> key_type;

    static constexpr KeySize keysize = key_type::keysize;

    key_type const key;

    RijndaelCrypt( key_type const& key ):
        key{key} {}

    template<typename CryptFunc>
    void applyCrypt( byte* first, byte* last, CryptFunc crypter )
    {
        int dist = std::distance(first, last);

        while( dist > 0 )
        {
            crypter( first, dist, key );
            dist -= 4 * Nb;
            first += 4 * Nb;
        }
    }

    void encrypt( byte* first, byte* last )
    { applyCrypt(first, last, Cipher<keysize>); }

    void decrypt( byte* first, byte* last )
    { applyCrypt(first, last, InvCipher<keysize>); }

};


#endif // CIPHER_HXX_INCLUDED
