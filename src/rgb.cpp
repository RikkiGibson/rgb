#include <iostream>
#include "z80.cpp"

void run_operation(Z80 &z80, uint8_t op)
{
    switch (op) {
    case 0: z80.nop(); break;
    }
}

int main(int argc, char **argv)
{
    Z80 z80 = Z80();
    while (true) {
        uint8_t op = z80.mmu.rb(z80.reg.pc);
        run_operation(z80, op);
        z80.clock.m += z80.reg.m;
        z80.clock.t += z80.clock.t;
    }

    return 0;
}