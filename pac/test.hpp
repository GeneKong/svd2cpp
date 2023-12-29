//
// Created by GeneKong on 2023/12/28.
//

#pragma once

#include "RegBase.h"

// typedef struct
//{
//  __IO uint32_t MODER;        /*!< GPIO port mode register,                     Address offset: 0x00      */
//  __IO uint32_t OTYPER;       /*!< GPIO port output type register,              Address offset: 0x04      */
//  __IO uint32_t OSPEEDR;      /*!< GPIO port output speed register,             Address offset: 0x08      */
//  __IO uint32_t PUPDR;        /*!< GPIO port pull-up/pull-down register,        Address offset: 0x0C      */
//  __IO uint32_t IDR;          /*!< GPIO port input data register,               Address offset: 0x10      */
//  __IO uint32_t ODR;          /*!< GPIO port output data register,              Address offset: 0x14      */
//  __IO uint32_t BSRR;         /*!< GPIO port bit set/reset register,      Address offset: 0x1A */
//  __IO uint32_t LCKR;         /*!< GPIO port configuration lock register,       Address offset: 0x1C      */
//  __IO uint32_t AFR[2];       /*!< GPIO alternate function low register,  Address offset: 0x20-0x24 */
//  __IO uint32_t BRR;          /*!< GPIO bit reset register,                     Address offset: 0x28      */
//} GPIO_TypeDef;

namespace FEmbed {

using namespace pac;

template<typename T = uint32_t, T BaseAddr = 0x0>
class Gpio_ {
  public:
    class Moder : public Register<T, BaseAddr + 0> {
      public:
        constexpr static T RegisterAddr = BaseAddr + 0;
        constexpr static inline unsigned int address() { return RegisterAddr; }

        // Method for tuple in all
        constexpr auto static MODEs = make_filed_array<T, BaseAddr + 0, 0, 2, 16>();

        // Method for individual (recommend)
        Filed<T, RegisterAddr, 0, 2> MODER0;
        Filed<T, RegisterAddr, 2, 2> MODER1;
        Filed<T, BaseAddr + 0, 4, 2> MODER2;
        Filed<T, BaseAddr + 0, 6, 2> MODER3;
        Filed<T, BaseAddr + 0, 8, 2> MODER4;
        Filed<T, BaseAddr + 0, 10, 2> MODER5;
        Filed<T, BaseAddr + 0, 12, 2> MODER6;
        Filed<T, BaseAddr + 0, 14, 2> MODER7;
        Filed<T, BaseAddr + 0, 16, 2> MODER8;
        Filed<T, BaseAddr + 0, 18, 2> MODER9;
        Filed<T, BaseAddr + 0, 20, 2> MODER10;
        Filed<T, BaseAddr + 0, 22, 2> MODER11;
        Filed<T, BaseAddr + 0, 24, 2> MODER12;
        Filed<T, BaseAddr + 0, 26, 2> MODER13;
        Filed<T, BaseAddr + 0, 28, 2> MODER14;
        Filed<T, BaseAddr + 0, 30, 2> MODER15;
    } MODER;

    class Otyper : public Register<T, BaseAddr + 4> {
      public:
        constexpr static inline unsigned int address() { return BaseAddr + 4; }

        // Method for tuple in all
        constexpr auto static OTYPEs = make_filed_array<T, BaseAddr + 4, 0, 1, 16>();

        // Method for individual (recommend)
        Filed<T, BaseAddr + 4, 0, 1> OTYPE0;
        Filed<T, BaseAddr + 4, 1, 1> OTYPE1;
        Filed<T, BaseAddr + 4, 2, 1> OTYPE2;
        Filed<T, BaseAddr + 4, 3, 1> OTYPE3;
        Filed<T, BaseAddr + 4, 4, 1> OTYPE4;
        Filed<T, BaseAddr + 4, 5, 1> OTYPE5;
        Filed<T, BaseAddr + 4, 6, 1> OTYPE6;
        Filed<T, BaseAddr + 4, 7, 1> OTYPE7;
        Filed<T, BaseAddr + 4, 8, 1> OTYPE8;
        Filed<T, BaseAddr + 4, 9, 1> OTYPE9;
        Filed<T, BaseAddr + 4, 10, 1> OTYPE10;
        Filed<T, BaseAddr + 4, 11, 1> OTYPE11;
        Filed<T, BaseAddr + 4, 12, 1> OTYPE12;
        Filed<T, BaseAddr + 4, 13, 1> OTYPE13;
        Filed<T, BaseAddr + 4, 14, 1> OTYPE14;
        Filed<T, BaseAddr + 4, 15, 1> OTYPE15;
    } OTYPER;
};

// 这里设计一个GPIO的驱动
using GpioA = pac::Gpioa<>;

enum class GpioPort {
    A,
    B,
    C,
    D,
    E,
    F,
    G,
    H,
    I,
    J,
    K
};

// 设计GPIO的合并和自动展开测试
constexpr auto static Gpios = std::make_tuple(Gpioa<>(), Gpiob<>(), Gpio_<>());

template<GpioPort PORT, size_t PIN>
class Gpio {
  public:
    constexpr static void setValid()
    {
        auto gpiox = std::get<static_cast<size_t>(PORT)>(Gpios);
        auto pin = std::get<PIN>(gpiox.MODER.MODEs);
        pin.set();
    }
};

using PC2 = Gpio<GpioPort::C, 2>;

}

