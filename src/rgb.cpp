#include <iostream>
#include "z80.cpp"
#include "gpu.cpp"

class RGB {
    MMU mmu = MMU();
    Z80 z80 = Z80(mmu);
    GPU gpu = GPU(mmu);

  public:
    void run_loop() {
        while (!z80.halt && !z80.stop) {
            z80.exec();
            gpu.step(z80.reg.t);
        }
    }
};

int main(int argc, char **argv)
{
    RGB rgb = RGB();
    rgb.run_loop();

    // rgb.mmu.load_rom();
    return 0;
}
