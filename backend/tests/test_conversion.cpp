#include <cassert>
#include <cstdio>
#include <iostream>

#include "protocol_gateway/engine.hpp"

using namespace protocol_gateway;

void test_modbus_to_iec61850_mapping_applies_scale_and_target_address() {
    const std::string path = "/tmp/protocol-gateway-test-1.json";
    std::remove(path.c_str());
    JsonConfigStore store(path);
    store.save_mapping(PointMapping{
        "m1",
        ProtocolType::Modbus,
        "holding_register:40001",
        ProtocolType::Iec61850,
        "IED1/MMXU1/PhV.phsA/cVal.mag.f",
        "float",
        0.1,
        0.0,
        "A phase voltage",
    });

    ProtocolRegistry registry = create_default_registry();
    ConversionEngine engine(registry, store);
    ConversionResult result = engine.convert(ConversionRequest{
        ProtocolType::Modbus,
        ProtocolType::Iec61850,
        Json::parse(R"({"function":"holding_register","address":40001,"value":2205,"type":"float","unitId":1})"),
    });

    assert(result.target.protocol == ProtocolType::Iec61850);
    assert(result.target.address == "IED1/MMXU1/PhV.phsA/cVal.mag.f");
    assert(result.target.value.as_number() == 220.5);
    assert(result.encoded_payload["reference"].as_string() == "IED1/MMXU1/PhV.phsA/cVal.mag.f");
    std::remove(path.c_str());
}

void test_iec104_to_modbus_without_mapping_keeps_value_for_diagnostics() {
    const std::string path = "/tmp/protocol-gateway-test-2.json";
    std::remove(path.c_str());
    JsonConfigStore store(path);
    ProtocolRegistry registry = create_default_registry();
    ConversionEngine engine(registry, store);

    ConversionResult result = engine.convert(ConversionRequest{
        ProtocolType::Iec104,
        ProtocolType::Modbus,
        Json::parse(R"({"ioa":"30001","value":1,"type":"bool"})"),
    });

    assert(result.target.protocol == ProtocolType::Modbus);
    assert(result.encoded_payload["protocol"].as_string() == "modbus");
    assert(result.encoded_payload["value"].as_number() == 1);
    std::remove(path.c_str());
}

int main() {
    test_modbus_to_iec61850_mapping_applies_scale_and_target_address();
    test_iec104_to_modbus_without_mapping_keeps_value_for_diagnostics();
    std::cout << "All protocol gateway C++ tests passed\n";
    return 0;
}
