#ifndef KEY_HXX_INCLUDED
#define KEY_HXX_INCLUDED

#include "Basic.hxx"

enum KeySize
{
    Nk_4 = 4,
    Nk_6 = 6,
    Nk_8 = 8
};

auto constexpr Nr( KeySize k ) -> AUTO_RETURN( k + 6 )

constexpr int Nb = 4;

/// ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////7

#include <array>
#include <initializer_list>

#include "Arrays.hxx"

template<KeySize Nk>
struct Key
{
    static KeySize constexpr keysize = Nk;

    static auto constexpr round_number = Nr(keysize),
                          word_number = Nb*(Nr(keysize)+1);

private:
    std::array<word, word_number> array;

protected:

    void KeyExpansion( word const* key )
    {
        std::copy( key, key + Nk, std::begin(array) );

        word temp;
        for( std::size_t i = Nk; i < word_number; ++i )
        {
            temp = array[i-1];

            if (i % Nk == 0)
                temp = SubWord(cyclicBitLeftShift(temp, 8)) ^ getRcon(i/keysize);
            else if (Nk > 6 && i % Nk == 4)
                temp = SubWord(temp); // TODO: Implement all the functions! Follow the PDF!

            array[i] = temp ^ array[i-Nk];
        }
    }

public:

    operator word const*() const { return array.data(); }

    /// pointer to the initial key
    Key( word const* ptr )
    { KeyExpansion( ptr ); }

    Key( std::initializer_list<word> list )
    { KeyExpansion( std::begin(list) ); }
};


#endif // KEY_HXX_INCLUDED
