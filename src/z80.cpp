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
        case 0: nop(); break;
        case 1: LD_BC_nn(); break;
        case 2: LD_BCm_A(); break;
        // TODO fill out this table after implementing ops
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

    void ADD_hl()
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

    void ADC_HL()
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

    void AND_HL(uint8_t r)
    {
        reg.a &= mmu.rb(reg.hl());
        reg.f = reg.a ? Flags::None : Flags::Zero;
        reg.m = 2;
        reg.t = 8;
    }

    void AND_n(uint8_t r)
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

    void OR_HL(uint8_t r)
    {
        reg.a |= mmu.rb(reg.hl());
        reg.f = reg.a ? Flags::None : Flags::Zero;
        reg.m = 2;
        reg.t = 8;
    }

    void OR_n(uint8_t r)
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

    void XOR_HL(uint8_t r)
    {
        reg.a ^= mmu.rb(reg.hl());
        reg.f = reg.a ? Flags::None : Flags::Zero;
        reg.m = 2;
        reg.t = 8;
    }

    void XOR_n(uint8_t r)
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
        // FIXME set flag after updating reg.a
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
        uint8_t lsb = reg.has_flags(Flags::Zero) ? 1 : 0;
        reg.f = reg.a ? Flags::None : Flags::Zero;
        if (reg.a & 0x80) {
            reg.f |= Flags::Carry;
        }
        reg.a = (reg.a << 1) + lsb;
        reg.m = 1;
        reg.t = 4;
    }

    // TODO RR_A, RRC_A

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
        reg.m = 2;
        reg.t = 8;
    }

    // TODO RR_r

    void nop()
    {
        clock.m = 1;
        clock.t = 4;
    }

    void pushbc()
    {
        reg.sp--;
        mmu.wb(reg.sp, reg.b);
        reg.sp--;
        mmu.wb(reg.sp, reg.c);
        clock.m = 3;
        clock.t = 12;
    }

    void pophl()
    {
        reg.l = mmu.rb(reg.sp);
        reg.sp++;
        reg.h = mmu.rb(reg.sp);
        reg.sp++;
        clock.m = 3;
        clock.t = 12;
    }

    void ldamm()
    {
        uint16_t addr = mmu.rw(reg.pc);
        reg.pc += 2;
        reg.a = mmu.rb(addr);
        clock.m = 4;
        clock.t = 16;
    }
// }
};