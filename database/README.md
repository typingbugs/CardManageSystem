请在 MySQL 数据库中执行 [此初始化脚本](./initDb.sql) 。

请按照实际情况在设备表中插入设备信息。

软件访问数据库软件时，固定使用用户 `CardManageSystem` 和数据库 `CardManageSystem` ，如需更改，请对应修改源码 [databaseAPI.cpp](../databaseAPI.cpp) 。