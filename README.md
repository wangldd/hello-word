# C++ 电力通讯管理机规约转换示例

本仓库提供一个使用 **C++17** 编写的规约转换业务代码骨架，面向电力通讯管理机/边缘网关场景。后端提供可编译的 C++ 核心库、REST API 服务和测试程序，前端仍采用静态 HTML/CSS/JavaScript，以前后端分离方式通过 Web 配置通道、点表映射并联调转换。

## 已覆盖能力

- 规约插件注册：内置 IEC 60870-5-101、IEC 60870-5-102、IEC 60870-5-103、IEC 60870-5-104、Modbus、IEC 61850。
- C++ 业务模型：通道、点表映射、统一遥测点、转换请求和转换结果均以强类型结构体描述。
- C++ 适配器边界：每种规约实现 `ProtocolAdapter`，负责将规约业务报文归一化为统一点模型，并重新编码为目标规约业务报文。
- 转换引擎：执行“归一化 -> 点表映射 -> 倍率/偏移 -> 目标规约编码”。
- 配置持久化：通道和点表映射保存到 JSON 文件，便于 Web 配置下发。
- REST API：C++ 服务提供 `/api/protocols`、`/api/channels`、`/api/mappings`、`/api/convert`、`/api/health`。
- 前后端分离：`backend/` 编译为 C++ API 服务，`frontend/` 可由任意静态文件服务器部署。

> 说明：真实工程中 IEC/Modbus 的链路层、加密认证、报文收发、IEC 61850 MMS/GOOSE 栈通常应接入厂商或认证协议栈。本示例聚焦“可配置规约转换业务层”和可扩展 C++ 代码结构。

## 目录结构

```text
backend/
  CMakeLists.txt
  include/protocol_gateway/
    adapters.hpp      # 规约适配器抽象
    engine.hpp        # 转换引擎
    http_server.hpp   # C++ REST 服务声明
    json.hpp          # 轻量 JSON 类型/解析/序列化
    models.hpp        # 通道、点表、遥测点等领域模型
    registry.hpp      # 规约插件注册表
    store.hpp         # JSON 配置持久化
  src/
    adapters.cpp      # IEC/Modbus/IEC61850 适配器实现
    engine.cpp        # 规约转换编排实现
    http_server.cpp   # 简易 HTTP/REST API 实现
    json.cpp          # 轻量 JSON 实现
    main.cpp          # 服务入口
    models.cpp        # 模型转换和规约枚举实现
    registry.cpp      # 默认规约注册
    store.cpp         # 配置读写
  tests/
    test_conversion.cpp
frontend/
  index.html          # 配置页面
  app.js              # 调用后端 API 的前端逻辑
  styles.css          # 页面样式
```

## 构建与测试

```bash
cmake -S backend -B build/backend
cmake --build build/backend
ctest --test-dir build/backend --output-on-failure
```

也可以直接运行测试二进制：

```bash
./build/backend/protocol_gateway_tests
```

## 本地运行

启动 C++ 后端 API：

```bash
./build/backend/protocol_gateway_server --host 127.0.0.1 --port 8080 --config data/gateway-config.json
```

启动前端（另开终端）：

```bash
python -m http.server 5173 -d frontend
```

浏览器打开 `http://localhost:5173`，即可配置通道、点表映射并提交转换联调。前端默认访问 `http://localhost:8080/api`。

## API 示例

新增 Modbus 到 IEC 61850 的点表映射：

```bash
curl -X POST http://localhost:8080/api/mappings \
  -H 'Content-Type: application/json' \
  -d '{
    "mapping_id":"map-modbus-to-61850-1",
    "source_protocol":"modbus",
    "source_address":"holding_register:40001",
    "target_protocol":"iec61850",
    "target_address":"IED1/MMXU1/PhV.phsA/cVal.mag.f",
    "value_type":"float",
    "scale":0.1,
    "offset":0,
    "description":"A 相电压"
  }'
```

执行一次转换：

```bash
curl -X POST http://localhost:8080/api/convert \
  -H 'Content-Type: application/json' \
  -d '{
    "source_protocol":"modbus",
    "target_protocol":"iec61850",
    "payload":{"function":"holding_register","address":40001,"value":2205,"type":"float","unitId":1}
  }'
```

## 扩展新规约

1. 在 `backend/include/protocol_gateway/models.hpp` 的 `ProtocolType` 添加稳定枚举值。
2. 在 `backend/src/models.cpp` 的 `to_string()` 和 `protocol_from_string()` 中添加 Web/API 字符串映射。
3. 在 `backend/include/protocol_gateway/adapters.hpp` 声明新的 `ProtocolAdapter` 实现。
4. 在 `backend/src/adapters.cpp` 实现 `normalize()` 和 `encode()`。
5. 在 `backend/src/registry.cpp` 的 `create_default_registry()` 中注册适配器。
6. 前端会通过 `/api/protocols` 自动加载规约列表，无需硬编码选项。
