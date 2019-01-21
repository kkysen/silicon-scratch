//
// Created by Khyber on 1/20/2019.
//

#ifndef SiliconScratch_Serializable_H
#define SiliconScratch_Serializable_H

#include <iostream>

#include "serialization.h"

template <typename T>
struct Serializable {
    
    static constexpr size_t size = ::unPaddedSize<T>();
    
    void deserializeBase(std::istream& stream) {
        ::deserializeWithPadding<T>(stream, reinterpret_cast<T&>(*this));
    }
    
    void serializeBase(std::ostream& stream) const {
        ::serializeWithPadding<T>(stream, reinterpret_cast<const T&>(*this));
    }
    
};

#endif // SiliconScratch_Serializable_H
