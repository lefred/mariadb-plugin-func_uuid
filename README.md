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
| uuid_version           | FUNCTION    | func_uuid.so   | Function UUID_VERSION()           | lefred        |
| uuid_to_bin            | FUNCTION    | func_uuid.so   | Function UUID_TO_BIN()            | lefred        |
| bin_to_uuid            | FUNCTION    | func_uuid.so   | Function BIN_TO_UUID()            | lefred        |
+------------------------+-------------+----------------+-----------------------------------+---------------+
6 rows in set (0.001 sec)
```

In the error log, we can see:

```
2026-02-16 19:42:41 4 [Warning] Plugin 'uuid_to_timestamp' is of maturity level experimental while the server is gamma
2026-02-16 19:42:41 4 [Warning] Plugin 'uuid_to_timestamp_long' is of maturity level experimental while the server is gamma
2026-02-16 19:42:41 4 [Warning] Plugin 'uuid_to_unixtime' is of maturity level experimental while the server is gamma
2026-02-17 11:11:47 3 [Warning] Plugin 'uuid_version' is of maturity level experimental while the server is gamma
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

### uuid_version

This function returns the detected UUID's version. By default only 1, 4 and 7 are valid in MariaDB:

```
MariaDB > select uuid_version(uuid());
+----------------------+
| uuid_version(uuid()) |
+----------------------+
|                    1 |
+----------------------+
1 row in set (0.000 sec)

MariaDB > select uuid_version(uuid_v4());
+-------------------------+
| uuid_version(uuid_v4()) |
+-------------------------+
|                       4 |
+-------------------------+
1 row in set (0.000 sec)

MariaDB > select uuid_version(uuid_v7());
+-------------------------+
| uuid_version(uuid_v7()) |
+-------------------------+
|                       7 |
+-------------------------+
1 row in set (0.000 sec)
```

### uuid_to_bin and bin_to_uuid

`uuid_to_bin()` converts a UUID string to binary and `bin_to_uuid()` does the opposite.

By default, `uuid_to_bin()` returns 17 bytes for UUIDv1, with the timestamp part swapped to allow better sorting by time. The 17th byte is a flag to indicate if the swap is enabled or not. For other UUID versions, no swap is applied and only 16 bytes are returned.:
- first 16 bytes: UUID binary payload
- last byte: swap flag (`0x00` no swap, `0x01` swap enabled)

Examples:

```
MariaDB [test]> SELECT HEX(UUID_TO_BIN('69378f8b-0c46-11f1-a0bc-3ecdbfc29ea1', 1));
+-------------------------------------------------------------+
| HEX(UUID_TO_BIN('69378f8b-0c46-11f1-a0bc-3ecdbfc29ea1', 1)) |
+-------------------------------------------------------------+
| A0BC3ECDBFC29EA111F10C4669378F8B01                          |
+-------------------------------------------------------------+
1 row in set (0.001 sec)

MariaDB [test]> SELECT HEX(UUID_TO_BIN('69378f8b-0c46-11f1-a0bc-3ecdbfc29ea1'));
+----------------------------------------------------------+
| HEX(UUID_TO_BIN('69378f8b-0c46-11f1-a0bc-3ecdbfc29ea1')) |
+----------------------------------------------------------+
| 69378F8B0C4611F1A0BC3ECDBFC29EA100                       |
+----------------------------------------------------------+
1 row in set (0.001 sec)
```

### uuid_to_bin_swap (global + session)

The plugin variable `uuid_to_bin_swap` controls the default swap behavior when calling
`UUID_TO_BIN(uuid)` without the second argument.

Session default:

```
MariaDB > SET SESSION uuid_to_bin_swap = 1;
MariaDB > SELECT HEX(UUID_TO_BIN(UUID()));
```

Global default:

```
MariaDB > SET GLOBAL uuid_to_bin_swap = 1;
```

Rules:
- `UUID_TO_BIN(uuid, flag)` always uses `flag`
- `UUID_TO_BIN(uuid)` uses `uuid_to_bin_swap`
- `BIN_TO_UUID(bin17)` uses the embedded 17th-byte flag
- `BIN_TO_UUID(bin16)` uses arg2 if provided, otherwise `uuid_to_bin_swap`

<img width="1358" height="427" alt="Screenshot From 2026-02-17 22-11-19" src="https://github.com/user-attachments/assets/1bd96e52-d04f-4a19-a99c-d43707a2d78f" />


## Errors

Now, when the UUID is valid, but it doesn't contain any timestamp like in UUIDv4, NULL is returned, but if the UUID is not valid, an error is still returned:

```
MariaDB > select uuid_to_timestamp(uuid_v4());
+------------------------------+
| uuid_to_timestamp(uuid_v4()) |
+------------------------------+
| NULL                         |
+------------------------------+
1 row in set (0.000 sec)

MariaDB > select uuid_to_timestamp("lefred");
ERROR: 1105 (HY000): uuid_to_timestamp: not a valid UUID
```

For `uuid_version()`, an error is returned when the argument is not a valid UUID:

```
MariaDB > select uuid_version("fred");
ERROR 1105 (HY000): uuid_version: not a valid UUID
```

## Example

Let's have a look at this example:

```
MariaDB [test]> CREATE TABLE t1 (uuid CHAR(36) PRIMARY KEY, name VARCHAR(255) NOT NULL);
Query OK, 0 rows affected (0.000 sec)

MariaDB [test]> INSERT INTO t1 VALUES(UUID(), 'first note');
Query OK, 1 row affected (0.013 sec)

MariaDB [test]> INSERT INTO t1 VALUES(UUID(), 'second note');
Query OK, 1 row affected (0.001 sec)

MariaDB [test]> INSERT INTO t1 VALUES(UUID_v4(), 'third note');
Query OK, 1 row affected (0.000 sec)

MariaDB [test]> INSERT INTO t1 VALUES(UUID_v7(), 'fourth note');
Query OK, 1 row affected (0.000 sec)
```

And now we can list them:

```
MariaDB [test]> select uuid, uuid_version(uuid) version, uuid_to_timestamp(uuid), name from t1;
+--------------------------------------+---------+-------------------------+-------------+
| uuid                                 | version | uuid_to_timestamp(uuid) | name        |
+--------------------------------------+---------+-------------------------+-------------+
| 207c783b-0be9-11f1-bd09-5e1b9081e705 |       1 | 2026-02-17 11:12:12.550 | first note  |
| 21f8a2f6-0be9-11f1-bd09-5e1b9081e705 |       1 | 2026-02-17 11:12:15.042 | second note |
| ccce8c94-187d-4fe8-b5c5-df808e6bc647 |       4 | NULL                    | third note  |
| 019c6b16-2736-7c48-ba54-b5084ffd51dc |       7 | 2026-02-17 11:12:19.894 | fourth note |
+--------------------------------------+---------+-------------------------+-------------+
4 rows in set (0.000 sec)
```

We can also use this function in CHECK CONSTRAINTS. If we want to
force the use of UUIDv7:

```
MariaDB [test]> CREATE TABLE t2 (uuid CHAR(36) PRIMARY KEY
                CHECK(uuid_version(uuid) = 7),
                name VARCHAR(255) NOT NULL);
Query OK, 0 rows affected (0.000 sec)

MariaDB [test]> INSERT INTO t2 VALUES(UUID_v4(), 'a UUID v4');
ERROR 4025 (23000): CONSTRAINT `t2.uuid` failed for `test`.`t2`

MariaDB [test]> INSERT INTO t2 VALUES(UUID_v7(), 'a UUID v7');
Query OK, 1 row affected (0.000 sec)
```
