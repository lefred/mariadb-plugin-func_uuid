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
#include <cctype>
#include <mysqld_error.h>

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

static bool is_valid_uuid_format_any(const std::string &str) {
    if (str.size() != 36) {
        return false;
    }
    if (str[8] != '-' || str[13] != '-' || str[18] != '-' || str[23] != '-') {
        return false;
    }
    for (size_t i = 0; i < str.size(); ++i) {
        if (i == 8 || i == 13 || i == 18 || i == 23) {
            continue;
        }
        if (!is_hex_char(str[i])) {
            return false;
        }
    }
    return true;
}

static bool is_valid_uuid_format_version(const std::string &str, char version) {
    if (!is_valid_uuid_format_any(str)) {
        return false;
    }
    return str[14] == version;
}

bool uuid_to_unixtime(const std::string &uuid_str, uint64_t *out) {
    if (!out) {
        return false;
    }

    uint64_t unix_ts = 0;
    if (!is_valid_uuid_format_any(uuid_str)) {
        my_printf_error(ER_UNKNOWN_ERROR,
            "uuid_to_unixtime: not a valid UUID",
            0);
        return false;
    }

    int uuid_version = return_uuid_version(uuid_str);
    if (uuid_version == 1) {
        if (!is_valid_uuid_format_version(uuid_str, '1')) {
            return false;
        }
        unix_ts = uuid_to_unixts(uuid_str);
    } else if (uuid_version == 7) {
        if (!is_valid_uuid_format_version(uuid_str, '7')) {
            return false;
        }
        uuid_t uuidv7;
        if (string_to_uuid(uuid_str, uuidv7) != 0) {
            return false;
        }
        unix_ts = uuidv7_to_unixts(uuidv7);
    } else {
        // Valid UUID without a timestamp (e.g. v4)
        return false;
    }

    *out = unix_ts / 1000;
    return true;
}

extern "C" std::string uuid_to_ts(const std::string &uuid_str, TimestampFormat format) {
    std::string out;
    if (!is_valid_uuid_format_any(uuid_str)) {
        my_printf_error(ER_UNKNOWN_ERROR,
            "uuid_to_timestamp: not a valid UUID",
            0);
        return "";
    }

    int uuid_version = return_uuid_version(uuid_str);
    if (uuid_version == 1) {
        if (!is_valid_uuid_format_version(uuid_str, '1')) {
            return "";
        }
        out = uuidv1_to_ts(uuid_str, format);
    } else if (uuid_version == 7) {
        if (!is_valid_uuid_format_version(uuid_str, '7')) {
            return "";
        }
        uuid_t uuidv7;
        if (string_to_uuid(uuid_str, uuidv7) != 0) {
            return "";
        }
        out = uuidv7_to_ts(uuidv7, format);
    } else {
        // Valid UUID without a timestamp (e.g. v4)
        return "";
    }
    return out;
}

int return_uuid_version(const std::string &str) {
    if (!is_valid_uuid_format_any(str)) {
	    return -1;
    }
    if (!is_hex_char(str[14])) {
        return -1;
    }
    return (str[14] <= '9') ? (str[14] - '0') : (std::tolower(static_cast<unsigned char>(str[14])) - 'a' + 10);
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

int uuid_string_to_binary(const std::string &str, uuid_t uuid) {
    if (str.size() != 36) {
	    return 1;
    }
    if (!is_valid_uuid_format_any(str)) {
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

std::string uuid_binary_to_string(const uuid_t uuid) {
    char buffer[37]; // 36 chars + null terminator
    snprintf(buffer, sizeof(buffer),
             "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
             uuid[0], uuid[1], uuid[2], uuid[3],
             uuid[4], uuid[5],
             uuid[6], uuid[7],
             uuid[8], uuid[9],
             uuid[10], uuid[11], uuid[12], uuid[13], uuid[14], uuid[15]);
    return std::string(buffer);
}

int uuid_string_to_binary_swap(const std::string &str, uuid_t uuid) {
    // First convert normally
    if (uuid_string_to_binary(str, uuid) != 0) {
        return 1;
    }

    // Check if this is a UUID with timestamp (versions 1 or 7)
    int version = return_uuid_version(str);
    if (version != 1 && version != 7) {
        // Not a timestamped UUID, return as-is
        return 0;
    }

    // For timestamped UUIDs, swap the time-related bytes
    // This puts the timestamp at the end for better index performance
    // In the standard UUID format:
    // - Bytes 0-3: time_low (4 bytes)
    // - Bytes 4-5: time_mid (2 bytes)
    // - Bytes 6-7: time_hi_version (2 bytes)
    // - Bytes 8-15: clock_seq + node (8 bytes)
    //
    // We want: [8-15] + [6-7] + [4-5] + [0-3] for better indexing
    uuid_t temp;
    memcpy(temp, uuid, UUID_T_LENGTH);

    // Move clock_seq + node to front (bytes 8-15 -> 0-7)
    memcpy(uuid + 0, temp + 8, 8);
    // Move time_hi_version (bytes 6-7 -> 8-9)
    memcpy(uuid + 8, temp + 6, 2);
    // Move time_mid (bytes 4-5 -> 10-11)
    memcpy(uuid + 10, temp + 4, 2);
    // Move time_low (bytes 0-3 -> 12-15)
    memcpy(uuid + 12, temp + 0, 4);

    return 0;
}

std::string uuid_binary_to_string_swap(const uuid_t uuid) {
    // Reverse the byte swapping that was done in uuid_string_to_binary_swap
    // Original format: [8-15] + [6-7] + [4-5] + [0-3]
    // Need to convert back to: [0-3] + [4-5] + [6-7] + [8-15]
    uuid_t temp;
    uuid_t result;

    memcpy(temp, uuid, UUID_T_LENGTH);

    // Move time_low (bytes 12-15 -> 0-3)
    memcpy(result + 0, temp + 12, 4);
    // Move time_mid (bytes 10-11 -> 4-5)
    memcpy(result + 4, temp + 10, 2);
    // Move time_hi_version (bytes 8-9 -> 6-7)
    memcpy(result + 6, temp + 8, 2);
    // Move clock_seq + node (bytes 0-7 -> 8-15)
    memcpy(result + 8, temp + 0, 8);

    // Now convert to string
    return uuid_binary_to_string(result);
}
