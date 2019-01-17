//
// Created by Khyber on 1/17/2019.
//

#include <cstring>
#include "src/lib/zip/utils/time_utils.h"

#if defined(_MSC_VER)
# define streamLocalTime(dt, ts)  do { localtime_s((ts), (dt)); } while (0)
#elif defined(__GNUC__) || defined(__GNUG__)
# define streamLocalTime(dt, ts)  do { localtime_r((dt), (ts)); } while (0)
#else
# define streamLocalTime(dt, ts)  do { tm* _tmp = localtime(dt); memcpy((ts), _tmp, sizeof(tm)); } while (0)
#endif

namespace utils::time {
    
    void timeStampToDateTime(time_t dateTime, u16& date, u16& time) {
        tm timeStruct = {};
        streamLocalTime(&dateTime, &timeStruct);
        date = static_cast<u16>(((timeStruct.tm_year - 80) << 9) + ((timeStruct.tm_mon + 1) << 5) + timeStruct.tm_mday);
        time = static_cast<u16>((timeStruct.tm_hour << 11) + (timeStruct.tm_min << 5) + (timeStruct.tm_sec >> 1));
    }
    
    time_t dateTimeToTimeStamp(uint16_t date, uint16_t time) {
        tm timeStruct {
                .tm_sec = ((time << 1) & 0x3f),
                .tm_min = ((time >> 5) & 0x3f),
                .tm_hour = ((time >> 11) & 0x1f),
                
                .tm_mday = ((date) & 0x1f),
                .tm_mon = ((date >> 5) & 0x0f) - 1,
                .tm_year = ((date >> 9) & 0x7f) + 80,
                
                .tm_wday = 0,
                .tm_yday = 0,
                .tm_isdst = 0,
                .tm_gmtoff = 0,
                .tm_zone = nullptr,
        };
        return mktime(&timeStruct);
    }
    
}

#undef streamLocalTime
