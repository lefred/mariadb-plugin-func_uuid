/* Copyright (c) 2019,2024,2025,2036 MariaDB Corporation
   Copyright (c) 2026 lefred (Frédéric Descamps)

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

#define MYSQL_SERVER
#include "mariadb.h"
#include "item_uuidfunc.h"

#include "m_ctype.h"

#include <cstdio>
#include <mysqld_error.h>

#include "common.h"

String *Item_func_uuid_to_timestamp::val_str(String *str) {
    String in_tmp;
    String *uuid_arg = args[0]->val_str(&in_tmp);
    if (!uuid_arg) { null_value = true; return nullptr; }

    std::string in_str(uuid_arg->ptr(), uuid_arg->length());
    std::string out_str = uuid_to_ts(in_str);

    if (out_str.empty()) {
        null_value = true;
        return nullptr;
    }

    // copy() allocates, copies, sets length, and NUL-terminates
    if (str->copy(out_str.c_str(),
                  static_cast<uint>(out_str.size()),
                  collation.collation)) {
        null_value = true;
        return nullptr;
    }

    null_value = false;
    return str;
}

String *Item_func_uuid_to_timestamp_long::val_str(String *str) {
    // Implement the conversion from UUID to DETAILED TIMESTAMP
    String in_tmp;
    String *uuid_arg = args[0]->val_str(&in_tmp);
    if (!uuid_arg) { null_value = true; return nullptr; }

    std::string in_str(uuid_arg->ptr(), uuid_arg->length());
    std::string out_str = uuid_to_ts(in_str, TS_LONG);

    if (out_str.empty()) {
        null_value = true;
        return nullptr;
    }

    if (str->copy(out_str.c_str(), static_cast<uint>(out_str.size()), collation.collation)) {
        null_value = true;
        return nullptr;
    }

    null_value = false;
    return str;
}


longlong Item_func_uuid_to_unixtime::val_int() {
    // Implement the conversion from UUID to UNIXTIME
    String in_tmp;
    String *uuid_arg = args[0]->val_str(&in_tmp);
    if (!uuid_arg) {
        null_value = true;
        return 0;
    }

    std::string in_str(uuid_arg->ptr(), uuid_arg->length());
    uint64_t out = 0;
    if (!uuid_to_unixtime(in_str, &out)) {
        null_value = true;
        return 0;
    }

    null_value = false;
    return out;
}

longlong Item_func_uuid_version::val_int() {
    String in_tmp;
    String *uuid_arg = args[0]->val_str(&in_tmp);
    if (!uuid_arg) {
        null_value = true;
        return 0;
    }

    std::string in_str(uuid_arg->ptr(), uuid_arg->length());
    int version = return_uuid_version(in_str);
    if (version < 0) {
        my_printf_error(ER_UNKNOWN_ERROR,
            "uuid_version: not a valid UUID",
            0);
        null_value = true;
        return 0;
    }

    null_value = false;
    return version;
}

String *Item_func_uuid_to_bin::val_str(String *str) {
    String in_tmp;
    String *uuid_arg = args[0]->val_str(&in_tmp);
    if (!uuid_arg) {
        null_value = true;
        return nullptr;
    }

    std::string in_str(uuid_arg->ptr(), uuid_arg->length());
    uuid_t binary_uuid;

    // Check if we can use the swap flag (only UUID versions with embedded timestamps)
    bool use_swap = false;
    int uuid_version = return_uuid_version(in_str);
    if (uuid_version == 1) {
        if (arg_count == 2) {
            longlong flag = args[1]->val_int();
            use_swap = (flag != 0);
        } else {
            use_swap = get_uuid_to_bin_swap(current_thd);
        }
    }
    // Choose conversion method based on swap flag
    int result;
    if (use_swap) {
        result = uuid_string_to_binary_swap(in_str, binary_uuid);
    } else {
        result = uuid_string_to_binary(in_str, binary_uuid);
    }

    if (result != 0) {
        my_printf_error(ER_UNKNOWN_ERROR,
            "uuid_to_bin: not a valid UUID",
            0);
        null_value = true;
        return nullptr;
    }

    if (uuid_version == 1) {
        // UUIDv1: append flag byte (17 bytes total)
        char buffer[UUID_T_WITH_FLAG];
        memcpy(buffer, reinterpret_cast<const char*>(binary_uuid), UUID_T_LENGTH);
        buffer[UUID_T_LENGTH] = use_swap ? 0x01 : 0x00;

        if (str->copy(buffer, UUID_T_WITH_FLAG, collation.collation)) {
            null_value = true;
            return nullptr;
        }
    } else {
        // Non-v1: keep standard 16-byte binary format
        if (str->copy(reinterpret_cast<const char*>(binary_uuid),
                      UUID_T_LENGTH,
                      collation.collation)) {
            null_value = true;
            return nullptr;
        }
    }

    null_value = false;
    return str;
}

String *Item_func_bin_to_uuid::val_str(String *str) {
    String in_tmp;
    String *bin_arg = args[0]->val_str(&in_tmp);
    if (!bin_arg) {
        null_value = true;
        return nullptr;
    }

    uint32 input_len = bin_arg->length();
    bool use_swap = false;

    // Check if input has the flag byte (17 bytes) or is legacy (16 bytes)
    if (input_len == UUID_T_WITH_FLAG) {
        // New format with flag byte
        const unsigned char *ptr = reinterpret_cast<const unsigned char*>(bin_arg->ptr());
        unsigned char flag_byte = ptr[UUID_T_LENGTH];
        use_swap = (flag_byte == 0x01);
    } else if (input_len == UUID_T_LENGTH) {
        // Legacy format without flag byte
        // Check if we have a second argument (swap flag)
        if (arg_count == 2) {
            longlong flag = args[1]->val_int();
            use_swap = (flag != 0);
        } else {
            // Default behavior from session/global variable, but only auto-swap
            // when the payload looks like a swapped UUIDv1 binary.
            bool default_swap = get_uuid_to_bin_swap(current_thd);
            if (default_swap) {
                const unsigned char *ptr = reinterpret_cast<const unsigned char*>(bin_arg->ptr());
                int canonical_version = (ptr[6] >> 4) & 0x0F;
                int swapped_version = (ptr[8] >> 4) & 0x0F;
                use_swap = (canonical_version != 1 && swapped_version == 1);
            }
        }
    } else {
        my_printf_error(ER_UNKNOWN_ERROR,
            "bin_to_uuid: binary value must be exactly 16 or 17 bytes",
            0);
        null_value = true;
        return nullptr;
    }

    uuid_t uuid_binary;
    memcpy(uuid_binary, bin_arg->ptr(), UUID_T_LENGTH);

    std::string uuid_str;
    if (use_swap) {
        uuid_str = uuid_binary_to_string_swap(uuid_binary);
    } else {
        uuid_str = uuid_binary_to_string(uuid_binary);
    }

    if (str->copy(uuid_str.c_str(),
                  static_cast<uint>(uuid_str.size()),
                  collation.collation)) {
        null_value = true;
        return nullptr;
    }

    null_value = false;
    return str;
}
