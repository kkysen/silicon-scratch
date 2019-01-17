#pragma once

#include <cstdint>
#include <ctime>

#include "src/main/util/numbers.h"

namespace utils::time {
    
    void timeStampToDateTime(time_t dateTime, u16& date, u16& time);
    
    time_t dateTimeToTimeStamp(uint16_t date, uint16_t time);
    
}