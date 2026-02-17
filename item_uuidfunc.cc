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

