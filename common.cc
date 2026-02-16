/* Copyright (c) 2026 lefred (Frédéric Descamps)

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 of the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1335  USA */

#include "mariadb.h"
#include "common.h"
#include <mysqld_error.h>


uint64_t uuid_to_unixts(const std::string &uuid_str) {

  	static constexpr const int MS_FROM_100NS_FACTOR = 10000;
	static constexpr const uint64_t OFFSET_FROM_15_10_1582_TO_EPOCH = 122192928000000000;

  	/* store uuid parts in a vector */
  	std::vector<std::string> uuid_parts;

  	/* split uuid with '-' as delimiter */
  	boost::split(uuid_parts, uuid_str, [](char c){return c == '-';});

  	/* first part of uuid is time-low
     	   second part is time-mid
     	   third part is time high with most significant 4 bits as uuid version
	*/
  	std::string uuid_timestamp = uuid_parts[2].substr(1) + uuid_parts[1] + uuid_parts[0];

  	uint64_t timestamp = std::stoul(uuid_timestamp, nullptr, 16);

  	return (timestamp - OFFSET_FROM_15_10_1582_TO_EPOCH) / MS_FROM_100NS_FACTOR;
}


uint64_t uuidv7_to_unixts(uuid_t uuid) {
        uint64_t unix_ts = 0;

        for (int i = 0; i < UNIX_TS_LENGTH; i++) {
           unix_ts |= ((uint64_t)uuid[UNIX_TS_LENGTH - 1 - i]) << (8 * i);
        }

        return unix_ts;
}

uint64_t uuid_to_unixtime(const std::string &uuid_str) {
    uint64_t unix_ts = 0;
    int uuid_version = return_uuid_version(uuid_str);
    if (uuid_version == 1) {
        unix_ts = uuid_to_unixts(uuid_str);
    } else if (uuid_version == 7) {
        uuid_t uuidv7;
        string_to_uuid(uuid_str, uuidv7);
        unix_ts = uuidv7_to_unixts(uuidv7);
    } else {
        my_printf_error(ER_UNKNOWN_ERROR,
            "uuid_to_timestamp: not a valid UUID or no timestamp available",
            0);
        return 0;
    }
    return unix_ts / 1000;
}

extern "C" std::string uuid_to_ts(const std::string &uuid_str, TimestampFormat format) {
    std::string out;
    int uuid_version = return_uuid_version(uuid_str);
    if (uuid_version == 1) {
        out = uuidv1_to_ts(uuid_str, format);
    } else if (uuid_version == 7) {
        uuid_t uuidv7;
        string_to_uuid(uuid_str, uuidv7);
        out = uuidv7_to_ts(uuidv7, format);
    } else {
        my_printf_error(ER_UNKNOWN_ERROR,
            "uuid_to_timestamp: not a valid UUID or no timestamp available",
            0);
        return "";
    }
    return out;
}

int return_uuid_version(const std::string &str) {
    if (str.size() != 36) {
	    return -1;
    }
    return (str[14] - '0');
}

std::string get_timestamp(uint64_t milliseconds, TimestampFormat format = TS_SHORT) {
    std::time_t seconds = milliseconds / 1000;
    std::tm timeinfo;
#ifdef _WIN32
    localtime_s(&timeinfo, &seconds);
#else
    localtime_r(&seconds, &timeinfo);
#endif

    std::ostringstream oss;
    if (format == TS_SHORT) {
        oss << std::put_time(&timeinfo, "%Y-%m-%d %H:%M:%S")
            << '.' << std::setfill('0') << std::setw(3) << milliseconds % 1000;
    } else {
        oss << std::put_time(&timeinfo, "%c %Z");
    }
    return oss.str();
}

bool is_hex_char(char c) {
    return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

unsigned char hex_to_byte(char c) {
    if (c >= '0' && c <= '9') {
        return c - '0';
    } else if (c >= 'a' && c <= 'f') {
        return c - 'a' + 10;
    } else if (c >= 'A' && c <= 'F') {
        return c - 'A' + 10;
    }
    return 0;
}

int string_to_uuid(const std::string &str, uuid_t uuid) {
    if (str.size() != 36) {
	    return 1;
    }
    if (str[14] != '7') {
	    return 1;
    }

    int idx = 0;
    for (int i = 0; i < 16; ++i) {
        if (str[idx] == '-') {
            ++idx;
        }
        if (!is_hex_char(str[idx]) || !is_hex_char(str[idx + 1])) {
	        return 1;
        }
        uuid[i] = (hex_to_byte(str[idx]) << 4) | hex_to_byte(str[idx + 1]);
        idx += 2;
    }
    return 0;
}


std::string uuidv1_to_ts(const std::string &uuid_str, TimestampFormat format) {
    uint64_t timestamp = uuid_to_unixts(uuid_str);
    return get_timestamp(timestamp, format);
}

std::string uuidv7_to_ts(uuid_t uuid, TimestampFormat format) {
    uint64_t unix_ts = 0;
    for (int i = 0; i < UNIX_TS_LENGTH; i++) {
        unix_ts |= ((uint64_t)uuid[UNIX_TS_LENGTH - 1 - i]) << (8 * i);
    }
    return get_timestamp(unix_ts, format);
}
