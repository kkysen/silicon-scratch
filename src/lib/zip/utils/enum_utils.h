#pragma once
#include <cstdint>

#include "src/main/util/numbers.h"

#define MARK_AS_TYPED_ENUM_FLAGS_BASE(EnumType, type, friend)                                                        \
  friend bool     operator !  (EnumType  lhs)                { return bool(!static_cast<type>(lhs)); }                \
  friend EnumType operator ~  (EnumType  lhs)                { return EnumType(~static_cast<type>(lhs) ); }            \
  friend EnumType operator |  (EnumType  lhs, EnumType rhs)  { return EnumType(static_cast<type>(lhs) | static_cast<type>(rhs)); } \
  friend EnumType operator &  (EnumType  lhs, EnumType rhs)  { return EnumType(static_cast<type>(lhs) & static_cast<type>(rhs)); } \
  friend EnumType operator ^  (EnumType  lhs, EnumType rhs)  { return EnumType(static_cast<type>(lhs) ^ static_cast<type>(rhs)); } \
  friend EnumType operator |= (EnumType& lhs, EnumType rhs)  { return lhs = lhs | rhs; }                     \
  friend EnumType operator &= (EnumType& lhs, EnumType rhs)  { return lhs = lhs & rhs; }                     \
  friend EnumType operator ^= (EnumType& lhs, EnumType rhs)  { return lhs = lhs ^ rhs; }

#define MARK_AS_TYPED_ENUM_FLAGS(EnumType)         MARK_AS_TYPED_ENUM_FLAGS_BASE(EnumType, u32, )
#define MARK_AS_TYPED_ENUM_FLAGS_FRIEND(EnumType)  MARK_AS_TYPED_ENUM_FLAGS_BASE(EnumType, u32, friend)
