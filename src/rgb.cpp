#include <iostream>
#include "z80.cpp"

int main(int argc, char **argv)
{
    Z80 z80 = Z80();
    z80.mmu.load_rom();

    for (int i = 0; i < 128; i++) {
        z80.dump_state(std::cout);
        z80.exec();
    }

    z80.dump_state(std::cout);

    return 0;
}
