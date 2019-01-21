//
// Created by Khyber on 1/21/2019.
//

#ifndef SiliconScratch_BitFlagSetter_H
#define SiliconScratch_BitFlagSetter_H

#include <type_traits>

template <typename BitFlag>
class BitFlagSetter {

public:
    
    using IntType = typename std::underlying_type<BitFlag>::type;
    
private:

    IntType& raw;

public:
    
    explicit BitFlagSetter(IntType& raw = 0) : raw(raw) {}
    
    BitFlagSetter& operator|=(BitFlag flag) {
        raw |= static_cast<IntType>(flag);
        return *this;
    }
    
    BitFlagSetter& operator&=(BitFlag flag) {
        raw &= static_cast<IntType>(flag);
        return *this;
    }
    
    BitFlagSetter& operator^=(BitFlag flag) {
        raw ^= static_cast<IntType>(flag);
        return *this;
    }
    
};

#endif // SiliconScratch_BitFlagSetter_H
