#include <iostream>
#include <bitset>

#include "Key Schedule/Key Schedule.hxx"

#include "Cipher.hxx"

int main()
{
    Key<Nk_4> key{ 0x2b7e1516,
                   0x28aed2a6,
                   0xabf71588,
                   0x09cf4f3c };

    RijndaelCrypt<Nk_4> cryptor( key );

    std::array<byte, 16> message{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};

    cryptor.Encrypt( std::begin(message), std::end(message) );

    std::cout << "Encrypted: ";
    std::copy( message.begin(), message.end(), std::ostream_iterator<short>(std::cout, " ") );

    cryptor.Decrypt( std::begin(message), std::end(message) );

    std::cout << "\nDecrypted: ";
    std::copy( message.begin(), message.end(), std::ostream_iterator<short>(std::cout, " ") );
}
