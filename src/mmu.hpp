//
// Created by Rikki Gibson on 3/4/18.
//

#ifndef RGB_MMU_HPP
#define RGB_MMU_HPP

#include <cstdint>
#include <vector>

class MMU {
private:
    std::vector<uint8_t> rom;
    std::vector<uint8_t> gram = std::vector<uint8_t>(0x2000);
    std::vector<uint8_t> eram = std::vector<uint8_t>(0x2000);
    std::vector<uint8_t> wram = std::vector<uint8_t>(0x2000);
    std::vector<uint8_t> zram = std::vector<uint8_t>(0x80);
    void load_rom();

  public:
    MMU();
    bool inbios;
    uint8_t rb(uint16_t addr);
    uint16_t rw(uint16_t addr);
    void wb(uint16_t addr, uint8_t value);
    void ww(uint16_t addr, uint16_t value);
};

#endif //RGB_MMU_HPP
