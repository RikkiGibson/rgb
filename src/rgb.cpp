#include <iostream>
#include "z80.cpp"
#include "gpu.cpp"

int main(int argc, char **argv)
{
    GPU gpu = GPU();
    gpu.init();

    Z80 z80 = Z80();
    z80.mmu.load_rom();

    for (int i = 0; i < 128; i++) {
        z80.exec();
    }

    return 0;
}
