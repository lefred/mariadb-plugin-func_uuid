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

#ifndef FUNC_UUID_COMMON_INCLUDED
#define FUNC_UUID_COMMON_INCLUDED

#include <boost/locale/date_time.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/algorithm/string.hpp>

#include <iomanip>

#define UNIX_TS_LENGTH (6)
#define UUID_T_LENGTH (16)

enum TimestampFormat {
    TS_SHORT,
    TS_LONG
};


typedef uint8_t uuid_t[UUID_T_LENGTH];

extern "C" std::string uuid_to_ts(const std::string &uuid_str, TimestampFormat format = TS_SHORT);

std::string uuidv1_to_ts(const std::string &uuid_str, TimestampFormat format = TS_SHORT);
std::string uuidv7_to_ts(uuid_t uuid, TimestampFormat format = TS_SHORT);

int return_uuid_version(const std::string &str);
int string_to_uuid(const std::string &str, uuid_t uuid);

uint64_t uuid_to_unixts(const std::string &uuid_str);
uint64_t uuid_to_unixtime(const std::string &uuid_str);

bool is_hex_char(char c);
unsigned char hex_to_byte(char c);

#endif // FUNC_UUID_COMMON_INCLUDED

