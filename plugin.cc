/*
   Copyright (c) 2019, MariaDB Corporation
   Copyright (c) 2026, lefred (Frédéric Descamps)

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 of the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA */

#define MYSQL_SERVER

#include <mariadb.h>
#include "item_uuidfunc.h"
#include <sql_class.h>
#include <mysql/plugin_function.h>

class Create_func_uuid_to_timestamp : public Create_func_arg1
{
public:
   Item *create_1_arg(THD *thd, Item *arg1) override
   {
      return new (thd->mem_root) Item_func_uuid_to_timestamp(thd, arg1);
   }
   static Create_func_uuid_to_timestamp s_singleton;
protected:
   Create_func_uuid_to_timestamp() {}
   ~Create_func_uuid_to_timestamp() override{}
};

Create_func_uuid_to_timestamp Create_func_uuid_to_timestamp::s_singleton;

class Create_func_uuid_to_timestamp_long : public Create_func_arg1
{
public:
   Item *create_1_arg(THD *thd, Item *arg1) override
   {
      return new (thd->mem_root) Item_func_uuid_to_timestamp_long(thd, arg1);
   }
   static Create_func_uuid_to_timestamp_long s_singleton;
protected:
   Create_func_uuid_to_timestamp_long() {}
   ~Create_func_uuid_to_timestamp_long() override{}
};

Create_func_uuid_to_timestamp_long Create_func_uuid_to_timestamp_long::s_singleton;

class Create_func_uuid_to_unixtime : public Create_func_arg1
{
public:
   Item *create_1_arg(THD *thd, Item *arg1) override
   {
      return new (thd->mem_root) Item_func_uuid_to_unixtime(thd, arg1);
   }
   static Create_func_uuid_to_unixtime s_singleton;
protected:
   Create_func_uuid_to_unixtime() {}
   ~Create_func_uuid_to_unixtime() override{}
};

Create_func_uuid_to_unixtime Create_func_uuid_to_unixtime::s_singleton;

class Create_func_uuid_version : public Create_func_arg1
{
public:
   Item *create_1_arg(THD *thd, Item *arg1) override
   {
      return new (thd->mem_root) Item_func_uuid_version(thd, arg1);
   }
   static Create_func_uuid_version s_singleton;
protected:
   Create_func_uuid_version() {}
   ~Create_func_uuid_version() override{}
};

Create_func_uuid_version Create_func_uuid_version::s_singleton;

#define BUILDER(F) & F::s_singleton

static Plugin_function
   plugin_descriptor_function_uuid_to_timestamp(BUILDER(Create_func_uuid_to_timestamp)),
   plugin_descriptor_function_uuid_to_timestamp_long(BUILDER(Create_func_uuid_to_timestamp_long)),
   plugin_descriptor_function_uuid_to_unixtime(BUILDER(Create_func_uuid_to_unixtime)),
   plugin_descriptor_function_uuid_version(BUILDER(Create_func_uuid_version));

/*************************************************************************/

maria_declare_plugin(type_test)
{
  MariaDB_FUNCTION_PLUGIN,                      // the plugin type (see include/mysql/plugin.h)
  &plugin_descriptor_function_uuid_to_timestamp, // pointer to type-specific plugin descriptor
  "uuid_to_timestamp",                           // plugin name
  "lefred",                                     // plugin author
  "Function UUID_TO_TIMESTAMP()",                // the plugin description
  PLUGIN_LICENSE_GPL,                            // the plugin license (see include/mysql/plugin.h)
  0,                                            // Pointer to plugin initialization function
  0,                                            // Pointer to plugin deinitialization function
  0x0100,                                       // Numeric version 0xAABB means AA.BB version
  NULL,                                         // Status variables
  NULL,                                         // System variables
  "1.0",                                       // String version representation
  MariaDB_PLUGIN_MATURITY_EXPERIMENTAL         // Maturity(see include/mysql/plugin.h)*/
},
{
  MariaDB_FUNCTION_PLUGIN,                            // the plugin type (see include/mysql/plugin.h)
  &plugin_descriptor_function_uuid_to_timestamp_long, // pointer to type-specific plugin descriptor
  "uuid_to_timestamp_long",                           // plugin name
  "lefred",                                           // plugin author
  "Function UUID_TO_TIMESTAMP_LONG()",                // the plugin description
  PLUGIN_LICENSE_GPL,                                 // the plugin license (see include/mysql/plugin.h)
  0,                                                  // Pointer to plugin initialization function
  0,                                                  // Pointer to plugin deinitialization function
  0x0100,                                             // Numeric version 0xAABB means AA.BB version
  NULL,                                               // Status variables
  NULL,                                               // System variables
  "1.0",                                              // String version representation
  MariaDB_PLUGIN_MATURITY_EXPERIMENTAL                // Maturity(see include/mysql/plugin.h)*/
},
{
  MariaDB_FUNCTION_PLUGIN,                            // the plugin type (see include/mysql/plugin.h)
  &plugin_descriptor_function_uuid_to_unixtime,       // pointer to type-specific plugin descriptor
  "uuid_to_unixtime",                                 // plugin name
  "lefred",                                           // plugin author
  "Function UUID_TO_UNIXTIME()",                      // the plugin description
  PLUGIN_LICENSE_GPL,                                 // the plugin license (see include/mysql/plugin.h)
  0,                                                  // Pointer to plugin initialization function
  0,                                                  // Pointer to plugin deinitialization function
  0x0100,                                             // Numeric version 0xAABB means AA.BB version
  NULL,                                               // Status variables
  NULL,                                               // System variables
  "1.0",                                              // String version representation
  MariaDB_PLUGIN_MATURITY_EXPERIMENTAL                // Maturity(see include/mysql/plugin.h)*/
},
{
  MariaDB_FUNCTION_PLUGIN,                            // the plugin type (see include/mysql/plugin.h)
  &plugin_descriptor_function_uuid_version,           // pointer to type-specific plugin descriptor
  "uuid_version",                                     // plugin name
  "lefred",                                           // plugin author
  "Function UUID_VERSION()",                          // the plugin description
  PLUGIN_LICENSE_GPL,                                 // the plugin license (see include/mysql/plugin.h)
  0,                                                  // Pointer to plugin initialization function
  0,                                                  // Pointer to plugin deinitialization function
  0x0100,                                             // Numeric version 0xAABB means AA.BB version
  NULL,                                               // Status variables
  NULL,                                               // System variables
  "1.0",                                              // String version representation
  MariaDB_PLUGIN_MATURITY_EXPERIMENTAL                // Maturity(see include/mysql/plugin.h)*/
}
maria_declare_plugin_end;
