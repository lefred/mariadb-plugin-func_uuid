#ifndef ITEM_UUIDFUNC_INCLUDED
#define ITEM_UUIDFUNC_INCLUDED

/* Copyright (c) 2011, 2013, Oracle and/or its affiliates. All rights reserved.
   Copyright (c) 2014 MariaDB Foundation
   Copyright (c) 2019 MariaDB Corporation
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


#include "item.h"
#include "common.h"

class Item_func_uuid_to_timestamp : public Item_str_func
{
    public:
        Item_func_uuid_to_timestamp(THD *thd, Item *arg1)
            : Item_str_func(thd, arg1) {}

        Item *shallow_copy(THD *thd) const override
        {
            return do_get_copy(thd);
        }

        LEX_CSTRING func_name_cstring() const override
        {
            static LEX_CSTRING name = {STRING_WITH_LEN("uuid_to_timestamp") };
            return name;
        }

        String *val_str(String *str) override;
        bool fix_length_and_dec(THD *thd) override
        {
            collation.set(DTCollation_numeric());
            fix_char_length(23);
            return FALSE;
        }

        Item *do_get_copy(THD *thd) const
        {
            return get_item_copy<Item_func_uuid_to_timestamp>(thd, this);
        }

};

class Item_func_uuid_to_timestamp_long : public Item_str_func
{
    public:
        Item_func_uuid_to_timestamp_long(THD *thd, Item *arg1)
            : Item_str_func(thd, arg1) {}

        Item *shallow_copy(THD *thd) const override
        {
            return do_get_copy(thd);
        }

        LEX_CSTRING func_name_cstring() const override
        {
            static LEX_CSTRING name = {STRING_WITH_LEN("uuid_to_timestamp_long") };
            return name;
        }

        String *val_str(String *str) override;
        bool fix_length_and_dec(THD *thd) override
        {
            collation.set(DTCollation_numeric());
            fix_char_length(32);
            return FALSE;
        }

        Item *do_get_copy(THD *thd) const
        {
            return get_item_copy<Item_func_uuid_to_timestamp_long>(thd, this);
        }

};

class Item_func_uuid_to_unixtime : public Item_longlong_func
{
    public:
        Item_func_uuid_to_unixtime(THD *thd, Item *arg1)
            : Item_longlong_func(thd, arg1) {}

        Item *shallow_copy(THD *thd) const override
        {
            return do_get_copy(thd);
        }

        LEX_CSTRING func_name_cstring() const override
        {
            static LEX_CSTRING name = {STRING_WITH_LEN("uuid_to_unixtime") };
            return name;
        }

        longlong val_int() override;
        bool fix_length_and_dec(THD *thd) override
        {
            decimals = 0;
            max_length = 10;
            set_maybe_null();
            unsigned_flag = 1;
            return FALSE;
        }

        Item *do_get_copy(THD *thd) const
        {
            return get_item_copy<Item_func_uuid_to_unixtime>(thd, this);
        }

};

#endif
