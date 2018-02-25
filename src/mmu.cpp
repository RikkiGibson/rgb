#include <iostream>
#include <iomanip>
#include <fstream>
#include <cstdint>
#include <vector>
#include "util/util.cpp"

class MMU {
  public:
    bool inbios;
	std::vector<uint8_t> rom;
	std::vector<uint8_t> eram = std::vector<uint8_t>(0x2000);
	std::vector<uint8_t> wram = std::vector<uint8_t>(0x2000);
	std::vector<uint8_t> zram = std::vector<uint8_t>(0x80);

	void load_rom()
	{
		std::ifstream file("rom/opus5.gb", std::ios::in | std::ios::binary);
		rom.reserve(0x8000);
		rom.insert(
			rom.begin(),
			std::istream_iterator<uint8_t>(file),
			std::istream_iterator<uint8_t>());
		file.close();
	}

    uint8_t rb(uint16_t addr)
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
			// Not implemented
			return 0;

		// External RAM
		case 0xa000:
		case 0xb000:
			return eram.at(addr & 0x1fff);

		// Working RAM
		case 0xc000:
		case 0xd000:
			return wram.at(addr & 0x1fff);

		// Working RAM shadow
		case 0xe000:
			return wram.at(addr & 0x1fff);
		case 0xf000:
			switch (addr & 0x0f00) {
				// Object Attribute memory
				case 0x0e00:
					return 0;
				case 0x0f00:
					if (addr >= 0xff80) {
						return zram.at(addr & 0x7f);
					} else {
						// I/O control handling
						return 0;
					}
				default:
					// Working RAM
					return wram.at(addr & 0x1fff);
			}
		}
        return 0;
    }

    uint16_t rw(uint16_t addr)
    {
		uint16_t res = rb(addr);
		res |= ((uint16_t) rb(addr+1)) << 8;
        return res;
    }

    void wb(uint16_t addr, uint8_t value)
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
			// Not implemented
			break;

		// External RAM
		case 0xa000:
		case 0xb000:
			eram.at(addr & 0x1fff) = value;
			break;

		// Working RAM
		case 0xc000:
		case 0xd000:
			wram.at(addr & 0x1fff) = value;
			break;

		// Working RAM shadow
		case 0xe000:
			wram.at(addr & 0x1fff) = value;
			break;

		case 0xf000:
			switch (addr & 0x0f00) {
				// Object Attribute memory
				case 0x0e00:
					// unimplemented
					break;
				case 0x0f00:
					if (addr >= 0xff80) {
						zram.at(addr & 0x7f) = value;
					} else {
						// I/O control handling
					}
					break;
				default:
					// Working RAM
					wram.at(addr & 0x1fff) = value;
					break;
			}
        	break;
		}
    }

    void ww(uint16_t addr, uint16_t value)
    {
		wb(addr, (value & 255));
		wb(addr+1, (value >> 8));
    }
};