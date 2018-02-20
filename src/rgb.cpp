#include <iostream>
#include "z80.cpp"

int main(int argc, char **argv)
{
    Z80 z80 = Z80();
    z80.reg.b = 123;
    z80.LDrr(z80.reg.a, z80.reg.b);
    std::cout << (int) z80.reg.a << "\n";

    return 0;
}