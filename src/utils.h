#include <stdio.h>
#include <string>
// #include <cstring>

char* intToPadZeroes(unsigned int frame)
{
    std::string plain = std::to_string(frame);
    // std::string padded = std::string( (4 - strlen(plain) ), '0').append(plain);
    std::string new_str = std::string(4 - plain.length(), '0') + plain;

    return new_str.data();
}