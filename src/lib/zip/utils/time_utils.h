#pragma once

#include <cstdint>
#include <ctime>

#include "src/main/util/numbers.h"

namespace utils::time {
    
    void timeStampToDateTime(time_t dateTime, u16& date, u16& time) noexcept;
    
    time_t dateTimeToTimeStamp(u16 date, u16 time) noexcept;
    
}