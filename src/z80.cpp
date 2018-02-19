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

class Clock {
  public:
    uint8_t m, t;
};

class Registers {
  public:
    uint8_t a, b, c, d, e, h, l, m, t;
    uint8_t ime;
    Flags f;
    uint16_t pc, sp;
};

class Z80 {
  public:
    Clock clock;
    Registers reg;
    MMU mmu;

    bool halt;
    bool stop;

    void add()
    {
        int res = reg.a + reg.e;
        reg.f = Flags::None;
        if (res > 255) {
            reg.f |= Flags::Carry;
        }
        reg.a = (uint8_t) res; // truncates over 8 bits
        if (!reg.a) {
            reg.f |= Flags::Zero;
        }
        clock.m = 1;
        clock.t = 4;
    }

    void cp()
    {
        char i = reg.a;
        i -= reg.b;
        reg.f |= Flags::Operation;
        if (!i) {
            reg.f |= Flags::Zero;
        }
        clock.m = 1;
        clock.t = 4;
    }

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

    void reset()
    {
        reg.a = reg.b = reg.c = reg.d = reg.e = reg.h = reg.l = reg.m = reg.t = 0;
        reg.ime = 1;
        reg.f = Flags::None;
        reg.pc = reg.sp = 0;
        clock.m = clock.t = 0;
        halt = false;
        stop = false;
    }
};