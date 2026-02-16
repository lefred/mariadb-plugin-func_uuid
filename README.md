# mariadb-plugin-func_uuid

Plugin to extend MariaDB with timestamp decoding from UUIDs when available (v1 and v7).

## Installation

```
MariaDB > install soname 'func_uuid.so';
Query OK, 0 rows affected (0.0014 sec)

MariaDB > SELECT plugin_name, plugin_type, plugin_library, plugin_description, plugin_author 
          FROM information_schema.PLUGINS WHERE PLUGIN_TYPE = 'FUNCTION' and plugin_library='func_uuid.so';
+------------------------+-------------+----------------+-----------------------------------+---------------+
| plugin_name            | plugin_type | plugin_library | plugin_description                | plugin_author |
+------------------------+-------------+----------------+-----------------------------------+---------------+
| uuid_to_timestamp      | FUNCTION    | func_uuid.so   | Function UUID_TO_TIMESTAMP()      | lefred        |
| uuid_to_timestamp_long | FUNCTION    | func_uuid.so   | Function UUID_TO_TIMESTAMP_LONG() | lefred        |
| uuid_to_unixtime       | FUNCTION    | func_uuid.so   | Function UUID_TO_UNIXTIME()       | lefred        |
+------------------------+-------------+----------------+-----------------------------------+---------------+
3 rows in set (0.0030 sec)
```

In the error log, we can see: 

```
2026-02-16 19:42:41 4 [Warning] Plugin 'uuid_to_timestamp' is of maturity level experimental while the server is gamma
2026-02-16 19:42:41 4 [Warning] Plugin 'uuid_to_timestamp_long' is of maturity level experimental while the server is gamma
2026-02-16 19:42:41 4 [Warning] Plugin 'uuid_to_unixtime' is of maturity level experimental while the server is gamma
```

## Usage

All functions work with uuid v1 (default) and uuid v7.

### uuid_to_timestamp

This function extract the timestamp of an uuid and print it as a human readeable timestamp:

```
MariaDB > select uuid_to_timestamp(uuid());
+---------------------------+
| uuid_to_timestamp(uuid()) |
+---------------------------+
| 2026-02-16 19:43:41.500   |
+---------------------------+
1 row in set (0.000 sec)

MariaDB > select uuid_to_timestamp(uuid_v7());
+------------------------------+
| uuid_to_timestamp(uuid_v7()) |
+------------------------------+
| 2026-02-16 19:44:07.776      |
+------------------------------+
1 row in set (0.001 sec)
```

### uuid_to_timestamp_long

This function acts like `uuid_to_timestamp()` but provides more details, like the week day, month and timezone:

```
MariaDB > select uuid_to_timestamp_long(uuid());
+--------------------------------+
| uuid_to_timestamp_long(uuid()) |
+--------------------------------+
| Mon Feb 16 19:44:32 2026 CET   |
+--------------------------------+
1 row in set (0.000 sec)

MariaDB > select uuid_to_timestamp_long(uuid_v7());
+-----------------------------------+
| uuid_to_timestamp_long(uuid_v7()) |
+-----------------------------------+
| Mon Feb 16 19:44:49 2026 CET      |
+-----------------------------------+
1 row in set (0.000 sec)
```

### uuid_to_unixtime

This function displays the UNIX Time (since EPOCH):

```
MariaDB > select uuid_to_unixtime(uuid());
+--------------------------+
| uuid_to_unixtime(uuid()) |
+--------------------------+
|               1771267518 |
+--------------------------+
1 row in set (0.000 sec)

MariaDB >  select uuid_to_unixtime(uuid_v7());
+-----------------------------+
| uuid_to_unixtime(uuid_v7()) |
+-----------------------------+
|                  1771267535 |
+-----------------------------+
1 row in set (0.000 sec)

MariaDB > select uuid_to_timestamp_long(uuid()) uuidv1, 
                 from_unixtime(uuid_to_unixtime(uuid_v7())) uuidv7_unix, 
                 from_unixtime(uuid_to_unixtime(uuid())) uuidv1_unix, 
                 uuid_to_timestamp(uuid_v7()) uuidv7\G
*************************** 1. row ***************************
     uuidv1: Mon Feb 16 19:45:52 2026 CET
uuidv7_unix: 2026-02-16 19:45:52
uuidv1_unix: 2026-02-16 19:45:52
     uuidv7: 2026-02-16 19:45:52.189
1 row in set (0.000 sec)
```

## Errors

There is a minimal check performed:

```
MariaDB > select uuid_to_timestamp(uuid_v4());
ERROR: 1105 (HY000): uuid_to_timestamp: not a valid UUID or no timestamp available

MariaDB > select uuid_to_timestamp("lefred");
ERROR: 1105 (HY000): uuid_to_timestamp: not a valid UUID or no timestamp available
```
