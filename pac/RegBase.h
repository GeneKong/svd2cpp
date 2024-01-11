//
// Created by GeneKong on 2023/12/28.
//

#ifndef PACTEST_SRC_REGBASE_H_
#define PACTEST_SRC_REGBASE_H_

#include <cstdint>
#include <string_view>
#include <tuple>

namespace pac {
enum class AccessType {
    ReadWrite,
    ReadOnly,
    WriteOnly
};

template<typename T, T Addr, uint8_t BitOffset, uint8_t Width, AccessType Access = AccessType::ReadWrite, typename RT = T, typename WT = T>
class Filed {
  public:
    static_assert(BitOffset + Width <= sizeof(T) * 8, "BitOffset + Width must be less than or equal to the width of the register");

    constexpr unsigned int address() { return Addr; }

    constexpr void set() const
    {
        static_assert(Access != AccessType::ReadOnly, "Cannot write to a read-only register");

        T value = *(volatile T *)Addr;
        value = value | (bitMask() << bitOffset());
        *(volatile T *)Addr = value;
    }

    constexpr void set(const WT reg) const
    {
        static_assert(Access != AccessType::ReadOnly, "Cannot write to a read-only register");

        T wreg = static_cast<T>(reg);
        T value = *(volatile T *)Addr;
        value = ((wreg & bitMask()) << bitOffset()) | (value & (~(bitMask() << bitOffset())));
        *(volatile T *)Addr = value;
    }

    constexpr T evalSet(const T ov, const WT nv) const
    {
        T wreg = static_cast<T>(nv);
        return ((wreg & bitMask()) << bitOffset()) | (ov & (~(bitMask() << bitOffset())));
    }

    constexpr void clear() const
    {
        static_assert(Access != AccessType::WriteOnly, "Cannot clear from a write-only register");

        T value = *(volatile T *)Addr;
        value = value & (~(bitMask() << bitOffset()));
        *(volatile T *)Addr = value;
    }

    //    Filed don't need this
    //    constexpr void clear(T mask) {
    //        static_assert(Access != AccessType::WriteOnly, "Cannot clear from a write-only register");
    //
    //        T value = *(volatile T *)Addr;
    //        value = value & (~(mask << bitOffset()));
    //        *(volatile T *)Addr = value;
    //    }

    constexpr RT get() const
    {
        static_assert(Access != AccessType::WriteOnly, "Cannot read from a write-only register");

        T value = *(volatile T *)Addr;
        value = (value >> bitOffset()) & bitMask();
        return static_cast<RT>(value);
    }

    //    Filed don't need this
    //    constexpr bool test(T mask) const {
    //        static_assert(Access != AccessType::WriteOnly, "Cannot read from a write-only register");
    //
    //        T value = *(volatile T *)Addr;
    //        return (((value >> bitOffset()) & bitMask()) & mask) == mask;
    //    }

  private:
    constexpr static inline unsigned int bitOffset() { return BitOffset; }
    constexpr static inline unsigned int bitMask() { return (1 << Width) - 1; }
};


template<typename T, T Addr, uint8_t BitOffset, uint8_t Width, std::size_t... I>
constexpr auto make_filed_array(std::index_sequence<I...>) {
    return std::tuple<Filed<T, Addr, BitOffset + I * Width, Width> ...>{};
}

template<typename T, T Addr, uint8_t BitOffset, uint8_t Width, std::size_t N>
constexpr auto make_filed_array() {
    return make_filed_array<T, Addr, BitOffset, Width>(std::make_index_sequence<N>{});
}

template<typename T, T Addr, AccessType Access = AccessType::ReadWrite>
class Register {
  public:
    constexpr unsigned int address() { return Addr; }

    constexpr void set()  const
    {
        static_assert(Access != AccessType::ReadOnly, "Cannot write to a read-only register");

        *(volatile T *)Addr = (~(T)0);
    }

    constexpr void set(const T reg)  const
    {
        static_assert(Access != AccessType::ReadOnly, "Cannot write to a read-only register");

        *(volatile T *)Addr = reg;
    }

    constexpr void clear()  const
    {
        static_assert(Access != AccessType::WriteOnly, "Cannot clear from a write-only register");

        *(volatile T *)Addr = 0;
    }

    constexpr T get() const
    {
        static_assert(Access != AccessType::WriteOnly, "Cannot read from a write-only register");

        return *(volatile T *)Addr;
    }

    constexpr bool test(T mask) const
    {
        static_assert(Access != AccessType::WriteOnly, "Cannot read from a write-only register");

        return (*(volatile T *)Addr & mask) == mask;
    }

    constexpr void modify(T mask, T value) const
    {
        static_assert(Access != AccessType::WriteOnly, "Cannot modify a write-only register");

        *(volatile T *)Addr = (*(volatile T *)Addr & ~mask) | value;
    }

    constexpr void modify(T mask, T value, T *oldValue) const
    {
        static_assert(Access != AccessType::WriteOnly, "Cannot modify a write-only register");

        *oldValue = *(volatile T *)Addr;
        *(volatile T *)Addr = (*(volatile T *)Addr & ~mask) | value;
    }
};

template<typename T, T Addr, std::size_t Range, std::size_t... I>
constexpr auto make_register_array(std::index_sequence<I...>) {
    return std::tuple<Register<T, Addr + I * Range> ...>{};
}

template<typename T, T Addr, std::size_t Range, std::size_t N>
constexpr auto make_register_array() {
    return make_register_array<T, Addr, Range>(std::make_index_sequence<N>{});
}
}

#endif //PACTEST_SRC_REGBASE_H_
