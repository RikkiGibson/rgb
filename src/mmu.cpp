#include <cstdint>

class MMU {
  public:
    bool inbios;

    uint8_t rb(uint16_t addr)
    {
        return 0;
    }

    uint16_t rw(uint16_t addr)
    {
        return 0;
    }

    void wb(uint16_t addr, uint8_t value)
    {

    }

    void ww(uint16_t addr, uint8_t value)
    {

    }
};