#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>
#include "mmu.hpp"
#include "util/util.cpp"

MMU::MMU() {
    load_rom();
}

void MMU::load_rom()
{
    std::ifstream file("../rom/opus5.gb", std::ios::in | std::ios::binary);
    if (!file.good()) {
        throw std::exception();
    }

    rom.reserve(0x8000);
    rom.insert(
        rom.begin(),
        std::istream_iterator<uint8_t>(file),
        std::istream_iterator<uint8_t>());
    file.close();
}

uint8_t MMU::rb(uint16_t addr)
{
    switch (addr & 0xf000) {
    // ROM 0
    case 0x0000:
        if (inbios) {
            if (addr < 0x100) {
                // BIOS
                return 0xff;
            } else {
                throw std::out_of_range(
                    std::string("Unexpected memory read at ") + std::to_string(addr) + "\n");
            }
        } else {
            return rom.at(addr);
        }
    case 0x1000:
    case 0x2000:
    case 0x3000:
        return rom.at(addr);

    // ROM bank 1
    case 0x4000:
    case 0x5000:
    case 0x6000:
    case 0x7000:
        return rom.at(addr);

    // Graphics
    case 0x8000:
    case 0x9000:
        return gram.at(addr & 0x1fffu);

    // External RAM
    case 0xa000:
    case 0xb000:
        return eram.at(addr & 0x1fffu);

    // Working RAM
    case 0xc000:
    case 0xd000:
        return wram.at(addr & 0x1fffu);

    // Working RAM shadow
    case 0xe000:
        return wram.at(addr & 0x1fffu);
    case 0xf000:
        switch (addr & 0x0f00) {
            // Object Attribute memory
            case 0x0e00:
                return 0;
            case 0x0f00:
                if (addr >= 0xff80) {
                    return zram.at(addr & 0x7fu);
                } else {
                    // I/O control handling
                    return 0;
                }
            default:
                // Working RAM
                return wram.at(addr & 0x1fffu);
        }
    default:
        throw std::out_of_range("Unexpected access: " + std::to_string(addr));
    }
    return 0;
}

uint16_t MMU::rw(uint16_t addr)
{
    uint16_t res = rb(addr);
    res |= ((uint16_t) rb(addr+1)) << 8;
    return res;
}

void MMU::wb(uint16_t addr, uint8_t value)
{
    switch (addr & 0xf000) {
    // ROM 0
    case 0x0000:
        if (inbios) {
            if (addr < 0x100) {
                // BIOS, no-op
            } else {
                throw std::out_of_range(
                    std::string("Unexpected memory write at ") + std::to_string(addr) + "\n");
            }
        } else {
            rom.at(addr) = value;
        }
        break;
    case 0x1000:
    case 0x2000:
    case 0x3000:
        rom.at(addr) = value;
        break;

    // ROM bank 1
    case 0x4000:
    case 0x5000:
    case 0x6000:
    case 0x7000:
        rom.at(addr) = value;
        break;

    // Graphics
    case 0x8000:
    case 0x9000:
        gram.at(addr & 0x1fffu) = value;
        break;

    // External RAM
    case 0xa000:
    case 0xb000:
        eram.at(addr & 0x1fffu) = value;
        break;

    // Working RAM
    case 0xc000:
    case 0xd000:
        wram.at(addr & 0x1fffu) = value;
        break;

    // Working RAM shadow
    case 0xe000:
        wram.at(addr & 0x1fffu) = value;
        break;

    case 0xf000:
        switch (addr & 0x0f00) {
            // Object Attribute memory
            case 0x0e00:
                // unimplemented
                break;
            case 0x0f00:
                if (addr >= 0xff80) {
                    zram.at(addr & 0x7fu) = value;
                } else {
                    // I/O control handling
                }
                break;
            default:
                // Working RAM
                wram.at(addr & 0x1fffu) = value;
                break;
        }
        break;
    default:
        throw std::out_of_range("Unexpected access: " + std::to_string(addr));
    }
}

void MMU::ww(uint16_t addr, uint16_t value)
{
    wb(addr, (value & 0xff));
    wb(addr+1, (value >> 8));
}