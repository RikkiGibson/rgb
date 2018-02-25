#include <iostream>
#include "z80.cpp"

int main(int argc, char **argv)
{
    Z80 z80 = Z80();
    z80.reg.b = 123;
    z80.LD_rr(z80.reg.a, z80.reg.b);
    std::cout << (int) z80.reg.a << "\n";
    z80.mmu.load_rom();

    std::cout << z80.mmu.rom.size() << "\n";

    return 0;
}