#include <iostream>
#include <cstdint>
#include "mmu.cpp"

enum class Flags: uint8_t {
    Zero = 0x80,
    Operation = 0x40,
    HalfCarry = 0x20,
    Carry = 0x10,
    None = 0
};

constexpr enum Flags operator | (const enum Flags self, const enum Flags other)
{
    return (enum Flags)(uint8_t(self) | uint8_t(other));
}

constexpr enum Flags& operator |= (enum Flags &self, const enum Flags other)
{
    return self = (self | other);
}
constexpr enum Flags operator & (const enum Flags self, const enum Flags other)
{
    return (enum Flags)(uint8_t(self) & uint8_t(other));
}

constexpr enum Flags& operator &= (enum Flags &self, const enum Flags other)
{
    return self = (self | other);
}

constexpr enum Flags operator ~ (const enum Flags self)
{
    return (enum Flags) ~(uint8_t) self;
}

constexpr enum Flags operator ^ (const enum Flags self, const enum Flags other)
{
    return (enum Flags)(uint8_t(self) ^ uint8_t(other));
}

int16_t decode_2c(uint8_t byte)
{
    int16_t ret;
    if (byte > 127) {
        // Decode 2's complement negative
        ret = -(~byte+1);
    } else {
        ret = byte;
    }
    return ret;
}

class Clock {
  public:
    uint8_t m, t;
};

class Registers {
  public:
    uint8_t a, b, c, d, e, h, l, m, t, i, r;
    uint8_t ime;
    Flags f;
    uint16_t pc, sp;

    constexpr bool has_flags(enum Flags other)
    {
        return (this->f & other) == other;
    }

    uint16_t hl()
    {
        uint16_t addr = ((uint16_t) h) << 8 | l;
        return addr;
    }

    void set_hl(uint16_t value)
    {
        h = value >> 8;
        l = value & 0xff;
    }

    uint16_t bc()
    {
        uint16_t addr = ((uint16_t) b) << 8 | c;
        return addr;
    }

    uint16_t de()
    {
        uint16_t addr = ((uint16_t) b) << 8 | c;
        return addr;
    }
};

class Z80 {
  public:
    Clock clock;
    Registers reg;
    MMU mmu;

    bool halt;
    bool stop;

    void reset()
    {
        reg.a = reg.b = reg.c = reg.d = reg.e = reg.h = reg.l = reg.m = reg.t = reg.i = reg.r = 0;
        reg.ime = 1;
        reg.f = Flags::None;
        reg.pc = reg.sp = 0;
        clock.m = clock.t = 0;
        halt = false;
        stop = false;
    }

    void exec()
    {
        reg.r = (reg.r + 1) & 0x7f;
        uint8_t op = mmu.rb(reg.pc);
        switch (op) {
        case 0x00: NOP(); break;
        case 0x01: LD_BC_nn(); break;
        case 0x02: LD_BCm_A(); break;
        case 0x03: INC_BC(); break;
        case 0x04: INC_r(reg.b); break;
        case 0x05: DEC_r(reg.b); break;
        case 0x06: LD_r_n(reg.b); break;
        case 0x07: RLC_A(); break;
        case 0x08: LD_mm_SP(); break;
        case 0x09: ADD_HL(reg.bc()); break;
        case 0x0a: LD_A_BCm(); break;
        case 0x0b: DEC_BC(); break;
        case 0x0c: INC_r(reg.c); break;
        case 0x0d: DEC_r(reg.c); break;
        case 0x0e: LD_r_n(reg.c); break;
        case 0x0f: RRC_A(); break;

        case 0x10: DJNZn(); break;
        case 0x11: LD_DE_nn(); break;
        case 0x12: LD_DEm_A(); break;
        case 0x13: INC_DE(); break;
        case 0x14: INC_r(reg.d); break;
        case 0x15: DEC_r(reg.d); break;
        case 0x16: LD_r_n(reg.d); break;
        case 0x17: RL_A(); break;
        case 0x18: JRn(); break;
        case 0x19: ADD_HL(reg.de()); break;
        case 0x1a: LD_A_DEm(); break;
        case 0x1b: DEC_DE(); break;
        case 0x1c: INC_r(reg.e); break;
        case 0x1d: DEC_r(reg.e); break;
        case 0x1e: LD_r_n(reg.e); break;
        case 0x1f: RR_A(); break;

        case 0x20: JRNZn(); break;
        case 0x21: LD_HL_nn(); break;
        case 0x22: LD_HLI_A(); break;
        case 0x23: INC_HL(); break;
        case 0x24: INC_r(reg.h); break;
        case 0x25: DEC_r(reg.h); break;
        case 0x26: LD_r_n(reg.h); break;
        case 0x27: panic(); break;
        case 0x28: JRZn(); break;
        case 0x29: ADD_HL(reg.hl()); break;
        case 0x2a: LD_A_HLI(); break;
        case 0x2b: DEC_HL(); break;
        case 0x2c: INC_r(reg.l); break;
        case 0x2d: DEC_r(reg.l); break;
        case 0x2e: LD_r_n(reg.l); break;
        case 0x2f: CPL(); break;

        case 0x30: JRNCn(); break;
        case 0x31: LD_SP_nn(); break;
        case 0x32: LD_HLD_A(); break;
        case 0x33: INC_SP(); break;
        case 0x34: INC_HLm(); break;
        case 0x35: DEC_HLm(); break;
        case 0x36: LD_HL_mn(); break;
        case 0x37: SCF(); break;
        case 0x38: JRCn(); break;
        case 0x39: ADD_HL(reg.sp); break;
        case 0x3a: LD_A_HLD(); break;
        case 0x3b: DEC_SP(); break;
        case 0x3c: INC_r(reg.a); break;
        case 0x3d: DEC_r(reg.a); break;
        case 0x3e: LD_r_n(reg.a); break;
        case 0x3f: CCF(); break;

        case 0x40: LD_rr(reg.b, reg.b); break;
        case 0x41: LD_rr(reg.b, reg.c); break;
        case 0x42: LD_rr(reg.b, reg.d); break;
        case 0x43: LD_rr(reg.b, reg.e); break;
        case 0x44: LD_rr(reg.b, reg.h); break;
        case 0x45: LD_rr(reg.b, reg.l); break;
        case 0x46: LD_r_HLm(reg.b); break;
        case 0x47: LD_rr(reg.b, reg.a); break;
        case 0x48: LD_rr(reg.c, reg.b); break;
        case 0x49: LD_rr(reg.c, reg.c); break;
        case 0x4a: LD_rr(reg.c, reg.d); break;
        case 0x4b: LD_rr(reg.c, reg.e); break;
        case 0x4c: LD_rr(reg.c, reg.h); break;
        case 0x4d: LD_rr(reg.c, reg.l); break;
        case 0x4e: LD_r_HLm(reg.c); break;
        case 0x4f: LD_rr(reg.c, reg.a); break;

        case 0x50: LD_rr(reg.d, reg.b); break;
        case 0x51: LD_rr(reg.d, reg.c); break;
        case 0x52: LD_rr(reg.d, reg.d); break;
        case 0x53: LD_rr(reg.d, reg.e); break;
        case 0x54: LD_rr(reg.d, reg.h); break;
        case 0x55: LD_rr(reg.d, reg.l); break;
        case 0x56: LD_r_HLm(reg.d); break;
        case 0x57: LD_rr(reg.d, reg.a); break;
        case 0x58: LD_rr(reg.e, reg.b); break;
        case 0x59: LD_rr(reg.e, reg.c); break;
        case 0x5a: LD_rr(reg.e, reg.d); break;
        case 0x5b: LD_rr(reg.e, reg.e); break;
        case 0x5c: LD_rr(reg.e, reg.h); break;
        case 0x5d: LD_rr(reg.e, reg.l); break;
        case 0x5e: LD_r_HLm(reg.e); break;
        case 0x5f: LD_rr(reg.e, reg.a); break;

        case 0x60: LD_rr(reg.h, reg.b); break;
        case 0x61: LD_rr(reg.h, reg.c); break;
        case 0x62: LD_rr(reg.h, reg.d); break;
        case 0x63: LD_rr(reg.h, reg.e); break;
        case 0x64: LD_rr(reg.h, reg.h); break;
        case 0x65: LD_rr(reg.h, reg.l); break;
        case 0x66: LD_r_HLm(reg.h); break;
        case 0x67: LD_rr(reg.h, reg.a); break;
        case 0x68: LD_rr(reg.l, reg.b); break;
        case 0x69: LD_rr(reg.l, reg.c); break;
        case 0x6a: LD_rr(reg.l, reg.d); break;
        case 0x6b: LD_rr(reg.l, reg.e); break;
        case 0x6c: LD_rr(reg.l, reg.h); break;
        case 0x6d: LD_rr(reg.l, reg.l); break;
        case 0x6e: LD_r_HLm(reg.l); break;
        case 0x6f: LD_rr(reg.l, reg.a); break;

        case 0x70: LD_HLm_r(reg.b); break;
        case 0x71: LD_HLm_r(reg.c); break;
        case 0x72: LD_HLm_r(reg.d); break;
        case 0x73: LD_HLm_r(reg.e); break;
        case 0x74: LD_HLm_r(reg.h); break;
        case 0x75: LD_HLm_r(reg.l); break;
        case 0x76: HALT(); break;
        case 0x77: LD_HLm_r(reg.a); break;
        case 0x78: LD_rr(reg.a, reg.b); break;
        case 0x79: LD_rr(reg.a, reg.c); break;
        case 0x7a: LD_rr(reg.a, reg.d); break;
        case 0x7b: LD_rr(reg.a, reg.e); break;
        case 0x7c: LD_rr(reg.a, reg.h); break;
        case 0x7d: LD_rr(reg.a, reg.l); break;
        case 0x7e: LD_r_HLm(reg.a); break;
        case 0x7f: LD_rr(reg.a, reg.a); break;

        case 0x80: ADD_r(reg.b); break;
        case 0x81: ADD_r(reg.c); break;
        case 0x82: ADD_r(reg.d); break;
        case 0x83: ADD_r(reg.e); break;
        case 0x84: ADD_r(reg.h); break;
        case 0x85: ADD_r(reg.l); break;
        case 0x86: ADD_A_HL(); break;
        case 0x87: ADD_r(reg.a); break;
        case 0x88: ADC_r(reg.b); break;
        case 0x89: ADC_r(reg.c); break;
        case 0x8a: ADC_r(reg.d); break;
        case 0x8b: ADC_r(reg.e); break;
        case 0x8c: ADC_r(reg.h); break;
        case 0x8d: ADC_r(reg.l); break;
        case 0x8e: ADC_A_HL(); break;
        case 0x8f: ADC_r(reg.a); break;

        case 0x90: SUB_r(reg.b); break;
        case 0x91: SUB_r(reg.c); break;
        case 0x92: SUB_r(reg.d); break;
        case 0x93: SUB_r(reg.e); break;
        case 0x94: SUB_r(reg.h); break;
        case 0x95: SUB_r(reg.l); break;
        case 0x96: SUB_HL(); break;
        case 0x97: SUB_r(reg.a); break;
        case 0x98: SBC_r(reg.b); break;
        case 0x99: SBC_r(reg.c); break;
        case 0x9a: SBC_r(reg.d); break;
        case 0x9b: SBC_r(reg.e); break;
        case 0x9c: SBC_r(reg.h); break;
        case 0x9d: SBC_r(reg.l); break;
        case 0x9e: SBC_HL(); break;
        case 0x9f: SBC_r(reg.a); break;

        case 0xa0: AND_r(reg.b); break;
        case 0xa1: AND_r(reg.c); break;
        case 0xa2: AND_r(reg.d); break;
        case 0xa3: AND_r(reg.e); break;
        case 0xa4: AND_r(reg.h); break;
        case 0xa5: AND_r(reg.l); break;
        case 0xa6: AND_HL(); break;
        case 0xa7: AND_r(reg.a); break;
        case 0xa8: XOR_r(reg.b); break;
        case 0xa9: XOR_r(reg.c); break;
        case 0xaa: XOR_r(reg.d); break;
        case 0xab: XOR_r(reg.e); break;
        case 0xac: XOR_r(reg.h); break;
        case 0xad: XOR_r(reg.l); break;
        case 0xae: XOR_HL(); break;
        case 0xaf: XOR_r(reg.a); break;

        case 0xb0: OR_r(reg.b); break;
        case 0xb1: OR_r(reg.c); break;
        case 0xb2: OR_r(reg.d); break;
        case 0xb3: OR_r(reg.e); break;
        case 0xb4: OR_r(reg.h); break;
        case 0xb5: OR_r(reg.l); break;
        case 0xb6: OR_HL(); break;
        case 0xb7: OR_r(reg.a); break;
        case 0xb8: CP_r(reg.b); break;
        case 0xb9: CP_r(reg.c); break;
        case 0xba: CP_r(reg.d); break;
        case 0xbb: CP_r(reg.e); break;
        case 0xbc: CP_r(reg.h); break;
        case 0xbd: CP_r(reg.l); break;
        case 0xbe: CP_HL(); break;
        case 0xbf: CP_r(reg.a); break;

        case 0xc0: RETNZ(); break;
        case 0xc1: POP(reg.b, reg.c); break;
        case 0xc2: JPNZnn(); break;
        case 0xc3: JPnn(); break;
        case 0xc4: CALLNZnn(); break;
        case 0xc5: PUSH(reg.b, reg.c); break;
        case 0xc6: ADD_n(); break;
        case 0xc7: RST(0x00); break;
        case 0xc8: RETZ(); break;
        case 0xc9: RET(); break;
        case 0xca: JPZnn(); break;
        case 0xcb: panic(); break; /*** TODO: MAPcb ***/
        case 0xcc: CALLZnn(); break;
        case 0xcd: CALLnn(); break;
        case 0xce: ADC_n(); break;
        case 0xcf: RST(0x08); break;

        case 0xd0: RETNC(); break;
        case 0xd1: POP(reg.d, reg.e); break;
        case 0xd2: JPNCnn(); break;
        case 0xd3: panic(); break;
        case 0xd4: CALLNCnn(); break;
        case 0xd5: PUSH(reg.d, reg.e); break;
        case 0xd6: SUB_n(); break;
        case 0xd7: RST(0x10); break;
        case 0xd8: RETC(); break;
        case 0xd9: RETI(); break;
        case 0xda: JPCnn(); break;
        case 0xdb: panic(); break;
        case 0xdc: CALLCnn(); break;
        case 0xdd: panic(); break;
        case 0xde: SBC_n(); break;
        case 0xdf: RST(0x18); break;

        case 0xe0: LD_IOn_A(); break;
        case 0xe1: POP(reg.h, reg.l); break;
        case 0xe2: LD_IOC_A(); break;
        case 0xe3: panic(); break;
        case 0xe4: panic(); break;
        case 0xe5: PUSH(reg.h, reg.l); break;
        case 0xe6: AND_n(); break;
        case 0xe7: RST(0x20); break;
        case 0xe8: ADD_SP_n(); break;
        case 0xe9: JPHL(); break;
        case 0xea: LD_mm_A(); break;
        case 0xeb: panic(); break;
        case 0xec: panic(); break;
        case 0xed: panic(); break;
        case 0xee: OR_n(); break;
        case 0xef: RST(0x28); break;

        case 0xf0: LD_A_IOn(); break;
        case 0xf1: POP(reg.a, (uint8_t &) reg.f); break;
        case 0xf2: LD_A_IOC(); break;
        case 0xf3: DI(); break;
        case 0xf4: panic(); break;
        case 0xf5: PUSH(reg.a, (uint8_t &) reg.f); break;
        case 0xf6: XOR_n(); break;
        case 0xf7: RST(0x30); break;
        case 0xf8: LD_HL_SPn(); break;
        case 0xf9: panic(); break;
        case 0xfa: LD_A_mm(); break;
        case 0xfb: EI(); break;
        case 0xfc: panic(); break;
        case 0xfd: panic(); break;
        case 0xfe: CP_n(); break;
        case 0xff: RST(0x38); break;
        }

        clock.m += reg.m;
        clock.t += clock.t;
        if (mmu.inbios && reg.pc == 0x0100) {
            mmu.inbios = false;
        }

    }

// { Ops

    // Load to register dst from register src
    void LD_rr(uint8_t &dst, uint8_t src)
    {
        dst = src;
        reg.m = 1;
        reg.t = 4;
    }

    // Load to register dst from H/L memory location
    void LD_r_HLm(uint8_t &dst)
    {
        dst = mmu.rb(reg.hl());
        reg.m = 2;
        reg.t = 8;
    }

    // Load to H/L memory location from register src
    void LD_HLm_r(uint8_t src)
    {
        mmu.wb(reg.hl(), src);
        reg.m = 2;
        reg.t = 8;
    }

    // Load to register dst by dereferencing program counter
    void LD_r_n(uint8_t &dst)
    {
        dst = mmu.rb(reg.pc);
        reg.pc++;
        reg.m = 2;
        reg.t = 8;
    }

    // Load to H/L memory location from PC memory location
    void LD_HL_mn()
    {
        mmu.wb(reg.hl(), mmu.rb(reg.pc));
        reg.pc++;
        reg.m = 3;
        reg.t = 12;
    }

    // Load to B/C memory location from register A
    void LD_BCm_A()
    {
        uint16_t addr = ((uint16_t) reg.b) << 8 | reg.c;
        mmu.wb(addr, reg.a);
        reg.m = 2;
        reg.t = 8;
    }

    // Load to D/E memory location from register A
    void LD_DEm_A()
    {
        uint16_t addr = ((uint16_t) reg.d) << 8 | reg.e;
        mmu.wb(addr, reg.a);
        reg.m = 2;
        reg.t = 8;
    }

    // Load to memory location in PC from register A
    void LD_mm_A()
    {
        mmu.wb(mmu.rw(reg.pc), reg.a);
        reg.pc += 2;
        reg.m = 4;
        reg.t = 16;
    }

    // Load to A from B/C memory location
    void LD_A_BCm()
    {
        uint16_t addr = ((uint16_t) reg.b) << 8 | reg.c;
        reg.a = mmu.rb(addr);
        reg.m = 2;
        reg.t = 8;
    }

    // Load to A from D/E memory location
    void LD_A_DEm()
    {
        uint16_t addr = ((uint16_t) reg.d) << 8 | reg.e;
        reg.a = mmu.rb(addr);
        reg.m = 2;
        reg.t = 8;
    }

    void LD_A_mm()
    {
        reg.a = mmu.rb(mmu.rw(reg.pc));
        reg.pc += 2;
        reg.m = 4;
        reg.t = 16;
    }

    // Load to B/C from PC
    void LD_BC_nn()
    {
        reg.c = mmu.rb(reg.pc++);
        reg.b = mmu.rb(reg.pc++);
        reg.m = 3;
        reg.t = 12;
    }

    // Load to B/C from PC
    void LD_DE_nn()
    {
        reg.e = mmu.rb(reg.pc++);
        reg.d = mmu.rb(reg.pc++);
        reg.m = 3;
        reg.t = 12;
    }

    // Load to B/C from PC
    void LD_HL_nn()
    {
        reg.l = mmu.rb(reg.pc++);
        reg.h = mmu.rb(reg.pc++);
        reg.m = 3;
        reg.t = 12;
    }

    // Load to SP from PC
    void LD_SP_nn()
    {
        reg.sp = mmu.rw(reg.pc);
        reg.pc += 2;
        reg.m = 3;
        reg.t = 12;
    }

    // Load to H/L registers from PC memory location
    void LD_HL_mm()
    {
        uint16_t addr = mmu.rw(reg.pc);
        reg.pc += 2;
        reg.l = mmu.rb(addr);
        reg.h = mmu.rb(addr+1);
        reg.m = 5;
        reg.t = 20;
    }

    // Load to address in PC from H/L register values
    void LD_mm_HL()
    {
        uint16_t addr = mmu.rw(reg.pc);
        reg.pc += 2;
        mmu.ww(addr, reg.hl());
        reg.m = 5;
        reg.t = 20;
    }

    // Load to address in HL the value in register A. Increment HL.
    void LD_HLI_A()
    {
        mmu.wb(reg.hl(), reg.a);
        if (reg.l == 0xff) {
            reg.l = 0;
            reg.h++;
        } else {
            reg.l++;
        }
        reg.m = 2;
        reg.t = 8;
    }

    void LD_A_HLI()
    {
        mmu.wb(reg.hl(), reg.a);
        if (reg.l == 0xff) {
            reg.l = 0;
            reg.h++;
        } else {
            reg.l++;
        }
        reg.m = 2;
        reg.t = 8;
    }

    void LD_HLD_A()
    {
        mmu.wb(reg.hl(), reg.a);
        if (reg.l == 0) {
            reg.l = 0xff;
            reg.h--;
        } else {
            reg.l--;
        }
        reg.m = 2;
        reg.t = 8;
    }

    void LD_A_HLD()
    {
        reg.a = mmu.rb(reg.hl());
        if (reg.l == 0) {
            reg.l = 0xff;
            reg.h--;
        } else {
            reg.l--;
        }
        reg.m = 2;
        reg.t = 8;
    }

    void LD_A_IOn()
    {
        reg.a = mmu.rb(0xff00 | mmu.rb(reg.pc));
        reg.pc++;
        reg.m = 3;
        reg.t = 12;
    }

    void LD_IOn_A()
    {
        mmu.wb(0xff00 | mmu.rb(reg.pc), reg.a);
        reg.pc++;
        reg.m = 3;
        reg.t = 12;
    }

    void LD_A_IOC()
    {
        reg.a = mmu.rb(0xff00 | reg.c);
        reg.m = 2;
        reg.t = 8;
    }

    void LD_IOC_A()
    {
        mmu.wb(0xff00 | reg.c, reg.a);
        reg.m = 2;
        reg.t = 8;
    }

    void LD_mm_SP()
    {
        // Unsure if this is correct--just guessing
        mmu.wb(mmu.rw(reg.pc), mmu.rb(reg.sp));
        reg.pc += 2;
        reg.m = 5;
        reg.t = 20;
    }

    void LD_HL_SPn()
    {
        uint16_t value = mmu.rb(reg.pc);
        if (value > 0x7f) {
            value = -((~value+1)&0xff);
        }
        reg.pc++;
        value += reg.sp;
        reg.h = value >> 8;
        reg.l = value & 0xff;
    }

    void SWAP_r(uint8_t &r)
    {
        uint8_t temp = r;
        r = mmu.rb(reg.hl());
        mmu.wb(reg.hl(), temp);
        reg.m = 4;
        reg.t = 16;
    }

    void set_sum(uint16_t sum)
    {
        reg.f = (sum & 0xff) ? Flags::None : Flags::Zero;
        if (sum > 0xff) {
            reg.f |= Flags::Carry;
        }
        reg.a = (uint8_t) sum;
    }

    void ADD_r(uint8_t r)
    {
        uint16_t sum = reg.a + r;
        set_sum(sum);
        reg.m = 1;
        reg.t = 4;
    }

    void ADD_A_HL()
    {
        uint16_t sum = reg.a + mmu.rb(reg.hl());
        reg.f = (sum & 0xff) ? Flags::None : Flags::Zero;
        if (sum > 0xff) {
            reg.f |= Flags::Carry;
        }
        reg.a = (uint8_t) sum;
        reg.m = 2;
        reg.t = 8;
    }

    void ADD_n()
    {
        uint16_t sum = reg.a + mmu.rb(reg.pc);
        reg.pc++;
        reg.f = (sum & 0xff) ? Flags::None : Flags::Zero;
        if (sum > 0xff) {
            reg.f |= Flags::Carry;
        }
        reg.a = (uint8_t) sum;
        reg.m = 2;
        reg.t = 8;
    }

    void ADD_HL(uint16_t value)
    {
        uint32_t sum = reg.hl() + value;
        reg.f = (sum & 0xff) ? Flags::None : Flags::Zero;
        if (sum > 0xffff) {
            reg.f |= Flags::Carry;
        } else {
            // Not sure why it's done this way
            reg.f &= ~Flags::Zero;
        }
        reg.m = 3;
        reg.t = 12;
    }

    void ADD_SP_n()
    {
        uint8_t value = mmu.rb(reg.pc);
        if (value > 0x7f) {
            value = -(~value+1);
        }
        reg.pc++;
        reg.sp += value;
        reg.m = 4;
        reg.t = 16;
    }

    void ADC_r(uint8_t r)
    {
        uint16_t sum = reg.a + r;
        if ((reg.f & Flags::Carry) == Flags::Carry) {
            sum++;
        }
        reg.f = (sum & 0xff) ? Flags::None : Flags::Zero;
        if (sum > 0xff) {
            reg.f |= Flags::Carry;
        }
        reg.a = sum;
        reg.m = 1;
        reg.t = 4;
    }

    void ADC_A_HL()
    {
        uint16_t sum = reg.a + mmu.rb(reg.hl());
        if (reg.has_flags(Flags::Carry)) {
            sum++;
        }
        reg.f = (sum & 0xff) ? Flags::None : Flags::Zero;
        if (sum > 0xff) {
            reg.f |= Flags::Carry;
        }
        reg.a = (uint8_t) sum;
        reg.m = 2;
        reg.t = 8;
    }

    void ADC_n()
    {
        uint16_t sum = reg.a + mmu.rb(reg.pc);
        reg.pc++;
        if (reg.has_flags(Flags::Carry)) {
            sum++;
        }
        reg.f = (sum & 0xff) ? Flags::None : Flags::Zero;
        if (sum > 0xff) {
            reg.f |= Flags::Carry;
        }
        reg.a = (uint8_t) sum;
        reg.m = 2;
        reg.t = 8;
    }


    void set_diff_flags(int16_t diff)
    {
        reg.f = (diff & 0xff) ? Flags::None : Flags::Zero;
        reg.f |= Flags::Operation;
        if (diff < 0) {
            reg.f |= Flags::Carry;
        }
    }

    void set_diff(int16_t diff)
    {
        set_diff_flags(diff);
        reg.a = diff & 0xff;
    }

    void SUB_r(uint8_t r)
    {
        int16_t diff = reg.a - r;
        set_diff(diff);
        reg.m = 1;
        reg.t = 4;
    }

    void SUB_HL()
    {
        int16_t diff = reg.a - mmu.rb(reg.hl());
        set_diff(diff);
        reg.m = 2;
        reg.t = 8;
    }

    void SUB_n()
    {
        int16_t diff = reg.a - mmu.rb(reg.pc);
        reg.pc++;
        set_diff(diff);
        reg.m = 2;
        reg.t = 8;
    }

    void SBC_r(uint8_t r)
    {
        int16_t diff = reg.a - r;
        if (reg.has_flags(Flags::Carry)) {
            diff -= 1;
        }
        set_diff(diff);
        reg.m = 1;
        reg.t = 4;
    }

    void SBC_HL()
    {
        int16_t diff = reg.a - mmu.rb(reg.hl());
        if (reg.has_flags(Flags::Carry)) {
            diff -= 1;
        }
        set_diff(diff);
        reg.m = 2;
        reg.t = 8;
    }

    void SBC_n()
    {
        int16_t diff = reg.a - mmu.rb(reg.pc);
        reg.pc++;
        if (reg.has_flags(Flags::Carry)) {
            diff -= 1;
        }
        set_diff(diff);
        reg.m = 2;
        reg.t = 8;
    }

    void CP_r(uint8_t r)
    {
        int16_t sum = reg.a - r;
        set_diff_flags(sum);
        reg.m = 1;
        reg.t = 4;
    }

    void CP_HL()
    {
        int16_t sum = reg.a - mmu.rb(reg.hl());
        set_diff_flags(sum);
        reg.m = 2;
        reg.t = 8;
    }

    void CP_n()
    {
        int16_t sum = reg.a - mmu.rb(reg.pc);
        reg.pc++;
        set_diff_flags(sum);
        reg.m = 2;
        reg.t = 8;
    }

    void AND_r(uint8_t r)
    {
        reg.a &= r;
        reg.f = reg.a ? Flags::None : Flags::Zero;
        reg.m = 1;
        reg.t = 4;
    }

    void AND_HL()
    {
        reg.a &= mmu.rb(reg.hl());
        reg.f = reg.a ? Flags::None : Flags::Zero;
        reg.m = 2;
        reg.t = 8;
    }

    void AND_n()
    {
        reg.a &= mmu.rb(reg.pc);
        reg.pc++;
        reg.f = reg.a ? Flags::None : Flags::Zero;
        reg.m = 2;
        reg.t = 8;
    }

    void OR_r(uint8_t r)
    {
        reg.a |= r;
        reg.f = reg.a ? Flags::None : Flags::Zero;
        reg.m = 1;
        reg.t = 4;
    }

    void OR_HL()
    {
        reg.a |= mmu.rb(reg.hl());
        reg.f = reg.a ? Flags::None : Flags::Zero;
        reg.m = 2;
        reg.t = 8;
    }

    void OR_n()
    {
        reg.a |= mmu.rb(reg.pc);
        reg.pc++;
        reg.f = reg.a ? Flags::None : Flags::Zero;
        reg.m = 2;
        reg.t = 8;
    }

    void XOR_r(uint8_t r)
    {
        reg.a ^= r;
        reg.f = reg.a ? Flags::None : Flags::Zero;
        reg.m = 1;
        reg.t = 4;
    }

    void XOR_HL()
    {
        reg.a ^= mmu.rb(reg.hl());
        reg.f = reg.a ? Flags::None : Flags::Zero;
        reg.m = 2;
        reg.t = 8;
    }

    void XOR_n()
    {
        reg.a ^= mmu.rb(reg.pc);
        reg.pc++;
        reg.f = reg.a ? Flags::None : Flags::Zero;
        reg.m = 2;
        reg.t = 8;
    }

    void INC_r(uint8_t &r)
    {
        r++;
        reg.f = r ? Flags::None : Flags::Zero;
        reg.m = 1;
        reg.t = 4;
    }

    void INC_HLm()
    {
        uint8_t res = mmu.rb(reg.hl()) + 1;
        mmu.wb(reg.hl(), res);
        reg.f = res ? Flags::None : Flags::Zero;
        reg.m = 3;
        reg.t = 12;
    }

    void DEC_r(uint8_t &r)
    {
        r--;
        reg.f = r ? Flags::None : Flags::Zero;
        reg.m = 1;
        reg.t = 4;
    }

    void DEC_HLm()
    {
        uint8_t res = mmu.rb(reg.hl()) - 1;
        mmu.wb(reg.hl(), res);
        reg.f = res ? Flags::None : Flags::Zero;
        reg.m = 3;
        reg.t = 12;
    }

    void INC_BC()
    {
        reg.c++;
        if (!reg.c) {
            reg.b++;
        }
        reg.m = 1;
        reg.t = 4;
    }

    void INC_DE()
    {
        reg.e++;
        if (!reg.e) {
            reg.d++;
        }
        reg.m = 1;
        reg.t = 4;
    }

    void INC_HL()
    {
        reg.l++;
        if (!reg.l) {
            reg.h++;
        }
        reg.m = 1;
        reg.t = 4;
    }

    void INC_SP()
    {
        reg.sp++;
        reg.m = 1;
        reg.t = 4;
    }

    void DEC_BC()
    {
        reg.c--;
        if (reg.c == 0xff) {
            reg.b--;
        }
        reg.m = 1;
        reg.t = 4;
    }

    void DEC_DE()
    {
        reg.e--;
        if (reg.e == 0xff) {
            reg.d--;
        }
        reg.m = 1;
        reg.t = 4;
    }

    void DEC_HL()
    {
        reg.l--;
        if (reg.l == 0xff) {
            reg.h--;
        }
        reg.m = 1;
        reg.t = 4;
    }

    void DEC_SP()
    {
        reg.sp--;
        reg.m = 1;
        reg.t = 4;
    }

    void BIT_r(uint8_t r, uint8_t pos)
    {
        reg.f = r & (1 << pos) ? Flags::None : Flags::Zero;
        reg.m = 2;
        reg.t = 8;
    }

    void RL_A()
    {
        uint8_t lsb = reg.has_flags(Flags::Carry) ? 1 : 0;
        reg.f = reg.a ? Flags::None : Flags::Zero;
        if (reg.a & 0x80) {
            reg.f |= Flags::Carry;
        }
        reg.a = (reg.a << 1) + lsb;
        reg.m = 1;
        reg.t = 4;
    }

    void RLC_A()
    {
        uint8_t lsb = (reg.a & 0x80) ? 1 : 0;
        reg.a = (reg.a << 1) + lsb;
        if (!lsb) {
            reg.f &= ~Flags::Zero;
        }
        reg.m = 1;
        reg.t = 4;
    }

    void RR_A() {
        uint8_t msb = reg.has_flags(Flags::Carry) ? 0x80 : 0;
        if (~reg.a & 1) {
            reg.f &= ~Flags::Carry;
        }
        reg.a = (reg.a >> 1) + msb;
        reg.m = 1;
        reg.t = 4;
    }

    void RRC_A() {
        uint8_t msb = (reg.a & 1) ? 0x80 : 0;
        if (~reg.a & 1) {
            reg.f &= ~Flags::Carry;
        }
        reg.a = (reg.a >> 1) + msb;
        reg.m = 1;
        reg.t = 4;
    }

    void RL_r(uint8_t &r)
    {
        uint8_t lsb = reg.has_flags(Flags::Carry) ? 1 : 0;
        reg.f = r ? Flags::None : Flags::Zero;
        if (r & 0x80) {
            reg.f |= Flags::Carry;
        }
        r = (r << 1) + lsb;
        reg.m = 2;
        reg.t = 8;
    }

    void RL_HL()
    {
        uint8_t value = mmu.rb(reg.hl());
        uint8_t lsb = reg.has_flags(Flags::Carry) ? 1 : 0;
        reg.f = value ? Flags::None : Flags::Zero;
        if (value & 0x80) {
            reg.f |= Flags::Carry;
        }
        value = (value << 1) + lsb;
        mmu.wb(reg.hl(), value);
        reg.m = 4;
        reg.t = 16;
    }

    void RLC_r(uint8_t &r) {
        uint8_t lsb = (r & 0x80) ? 1 : 0;
        if (~r & 1) {
            reg.f &= ~Flags::Carry;
        }
        r = (r >> 1) + lsb;
        reg.m = 2;
        reg.t = 8;
    }

    void RLC_HL() {
        uint8_t value = mmu.rb(reg.hl());
        uint8_t lsb = (value & 0x80) ? 1 : 0;
        if (~value & 1) {
            reg.f &= ~Flags::Carry;
        }
        value = (value >> 1) + lsb;
        mmu.wb(reg.hl(), value);
        reg.m = 4;
        reg.t = 16;
    }

    void RR_r(uint8_t &r) {
        uint8_t msb = reg.has_flags(Flags::Carry) ? 0x80 : 0;
        if (~r & 1) {
            reg.f &= ~Flags::Carry;
        }
        r = (r >> 1) + msb;
        reg.m = 2;
        reg.t = 8;
    }

    void RR_HL()
    {
        uint8_t value = mmu.rb(reg.hl());
        uint8_t msb = reg.has_flags(Flags::Carry) ? 0x80 : 0;
        if (~value & 1) {
            reg.f &= ~Flags::Carry;
        }
        value = (value >> 1) + msb;
        mmu.wb(reg.hl(), value);
        reg.m = 4;
        reg.t = 16;
    }

    void RRC_r(uint8_t &r) {
        uint8_t msb = (r & 1) ? 0x80 : 0;
        if (~r & 1) {
            reg.f &= ~Flags::Carry;
        }
        r = (r >> 1) + msb;
        reg.m = 2;
        reg.t = 8;
    }

    void RRC_HL()
    {
        uint8_t value = mmu.rb(reg.hl());
        uint8_t msb = (value & 1) ? 0x80 : 0;
        if (~value & 1) {
            reg.f &= ~Flags::Carry;
        }
        value = (value >> 1) + msb;
        mmu.wb(reg.hl(), value);
        reg.m = 4;
        reg.t = 16;
    }

    void SLA_r(uint8_t &r)
    {
        reg.f = (r & 0x80) ? Flags::Carry : Flags::None;
        r <<= 1;
        if (!r) {
            reg.f |= Flags::Zero;
        }
        reg.m = 2;
        reg.t = 8;
    }

    void SRA_r(uint8_t &r)
    {
        reg.f = (r & 1) ? Flags::Carry : Flags::None;
        r = (r >> 1) | (r & 0x80);
        if (!r) {
            reg.f |= Flags::Zero;
        }
        reg.m = 2;
        reg.t = 8;
    }

    void SRL_r(uint8_t &r)
    {
        reg.f = (r & 1) ? Flags::Carry : Flags::None;
        r = r >> 1;
        if (!r) {
            reg.f |= Flags::Zero;
        }
        reg.m = 2;
        reg.t = 8;
    }

    void CPL()
    {
        reg.a = ~reg.a;
        reg.f = Flags::Operation;
        if (!reg.a) {
            reg.f |= Flags::Zero;
        }
        reg.m = 1;
        reg.t = 4;
    }

    void NEG()
    {
        reg.a = (~reg.a) + 1;
        reg.f = Flags::Operation;
        // Set Carry flag if result is negative (2's complement)?
        if (reg.a & 0x80) {
            reg.f |= Flags::Carry;
        }
        if (!reg.a) {
            reg.f |= Flags::Zero;
        }
        reg.m = 2;
        reg.t = 8;
    }

    void CCF()
    {
        reg.f = reg.f ^ Flags::Carry;
        reg.m = 1;
        reg.t = 4;
    }

    void SCF()
    {
        reg.f = reg.f | Flags::Carry;
        reg.m = 1;
        reg.t = 4;
    }

    // Valid pairs: BC, DE, HL, AF
    void PUSH(uint8_t r1, uint8_t r2)
    {
        mmu.wb(--reg.sp, r1);
        mmu.wb(--reg.sp, r2);
        reg.m = 3;
        reg.t = 12;
    }

    void POP(uint8_t &r1, uint8_t &r2)
    {
        r2 = mmu.rb(reg.sp++);
        r1 = mmu.rb(reg.sp++);
        reg.m = 3;
        reg.t = 12;
    }

    void JPnn()
    {
        reg.pc = mmu.rw(reg.pc);
        reg.m = 3;
        reg.t = 12;
    }

    void JPHL()
    {
        reg.pc = reg.hl();
        reg.m = 1;
        reg.t = 4;
    }

    void JPNZnn()
    {
        if (reg.has_flags(Flags::Zero)) {
            reg.pc += 2;
            reg.m = 3;
            reg.t = 12;
        } else {
            reg.pc = mmu.rw(reg.pc);
            reg.m = 4;
            reg.t = 12;
        }
    }

    void JPZnn()
    {
        if (reg.has_flags(Flags::Zero)) {
            reg.pc = mmu.rw(reg.pc);
            reg.m = 4;
            reg.t = 12;
        } else {
            reg.pc += 2;
            reg.m = 3;
            reg.t = 12;
        }
    }

    void JPNCnn()
    {
        if (reg.has_flags(Flags::Carry)) {
            reg.pc += 2;
            reg.m = 3;
            reg.t = 12;
        } else {
            reg.pc = mmu.rw(reg.pc);
            reg.m = 4;
            reg.t = 12;
        }
    }

    void JPCnn()
    {
        if (reg.has_flags(Flags::Carry)) {
            reg.pc = mmu.rw(reg.pc);
            reg.m = 4;
            reg.t = 12;
        } else {
            reg.pc += 2;
            reg.m = 3;
            reg.t = 12;
        }
    }

    void JRn()
    {
        int16_t value = decode_2c(mmu.rb(reg.pc));
        reg.pc += value + 1;
        reg.m += 3;
        reg.t += 12;
    }

    void JRNZn()
    {
        if (reg.has_flags(Flags::Zero)) {
            reg.pc++;
            reg.m = 2;
            reg.t = 8;
        } else {
            int16_t value = decode_2c(mmu.rb(reg.pc));
            reg.pc += value + 1;
            reg.m += 3;
            reg.t += 12;
        }
    }

    void JRZn()
    {
        if (reg.has_flags(Flags::Zero)) {
            int16_t value = decode_2c(mmu.rb(reg.pc));
            reg.pc += value + 1;
            reg.m += 3;
            reg.t += 12;
        } else {
            reg.pc++;
            reg.m = 2;
            reg.t = 8;
        }
    }

    void JRNCn()
    {
        if (reg.has_flags(Flags::Carry)) {
            reg.pc++;
            reg.m = 2;
            reg.t = 8;
        } else {
            int16_t value = decode_2c(mmu.rb(reg.pc));
            reg.pc += value + 1;
            reg.m += 3;
            reg.t += 12;
        }
    }

    void JRCn()
    {
        if (reg.has_flags(Flags::Carry)) {
            int16_t value = decode_2c(mmu.rb(reg.pc));
            reg.pc += value + 1;
            reg.m += 3;
            reg.t += 12;
        } else {
            reg.pc++;
            reg.m = 2;
            reg.t = 8;
        }
    }

    void DJNZn()
    {
        reg.b--;
        if (reg.b) {
            int16_t value = decode_2c(mmu.rb(reg.pc));
            reg.pc += value + 1;
            reg.m = 3;
            reg.t = 12;
        } else {
            reg.pc++;
            reg.m = 2;
            reg.t = 8;
        }
    }

    void CALLnn()
    {
        reg.sp -= 2;
        mmu.ww(reg.sp, reg.pc+2);
        reg.pc = mmu.rw(reg.pc);
        reg.m = 5;
        reg.t = 20;
    }

    void CALL_cond(bool cond)
    {
        if (reg.has_flags(Flags::Zero)) {
            reg.sp -= 2;
            mmu.ww(reg.sp, reg.pc+2);
            reg.pc = mmu.rw(reg.pc);
            reg.m = 5;
            reg.t = 20;
        } else {
            reg.pc += 2;
            reg.m = 3;
            reg.t = 12;
        }
    }

    void CALLNZnn()
    {
        CALL_cond(!reg.has_flags(Flags::Zero));
    }

    void CALLZnn()
    {
        CALL_cond(reg.has_flags(Flags::Zero));
    }

    void CALLNCnn()
    {
        CALL_cond(!reg.has_flags(Flags::Carry));
    }

    void CALLCnn()
    {
        CALL_cond(reg.has_flags(Flags::Carry));
    }

    void RET()
    {
        reg.pc = mmu.rw(reg.sp);
        reg.sp += 2;
        reg.m = 3;
        reg.t = 12;
    }

    void RETI()
    {
        reg.ime = 1;
        reg.pc = mmu.rw(reg.sp);
        reg.sp += 2;
        reg.m = 3;
        reg.t = 12;
    }

    void RET_cond(bool cond) {
        if (cond) {
            reg.pc = mmu.rw(reg.sp);
            reg.sp += 2;
            reg.m = 3;
            reg.t = 12;
        } else {
            reg.m = 1;
            reg.t = 4;
        }
    }

    void RETNZ()
    {
        RET_cond(!reg.has_flags(Flags::Zero));
    }

    void RETZ()
    {
        RET_cond(reg.has_flags(Flags::Zero));
    }

    void RETNC()
    {
        RET_cond(!reg.has_flags(Flags::Carry));
    }

    void RETC()
    {
        RET_cond(reg.has_flags(Flags::Carry));
    }

    void RST(uint16_t addr)
    {
        reg.sp -= 2;
        mmu.ww(reg.sp, reg.pc);
        reg.pc = addr;
        reg.m = 3;
        reg.t = 12;
    }

    void NOP()
    {
        clock.m = 1;
        clock.t = 4;
    }

    void HALT()
    {
        halt = true;
        reg.m = 1;
        reg.t = 4;
    }

    void DI()
    {
        reg.ime = 0;
        reg.m = 1;
        reg.t = 4;
    }

    void EI()
    {
        reg.ime = 1;
        reg.m = 1;
        reg.t = 4;
    }

    void panic()
    {
        std::cerr << "Unknown instruction at address " << reg.pc-1 << "\n";
        stop = 1;
    }
// }
};