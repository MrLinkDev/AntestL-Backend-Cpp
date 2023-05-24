#include <iostream>
#include <visa.h>
#include "json.hpp"

#define MAX_CNT 200

int main(int argc, char* argv[]) {
   /** ViStatus status;
    ViSession defaultRM, instr;
    ViPUInt32 retCount;
    ViPBuf buffer[MAX_CNT];

    status = viOpenDefaultRM(&defaultRM);

    if (status < VI_SUCCESS)
        return -1;

    status = viOpen(
            defaultRM,
            "TCPIP0::K-N9020B-11111::5025::SOCKET",
            VI_NULL,
            VI_NULL,
            &instr);

    status = viSetAttribute(instr, VI_ATTR_TMO_VALUE, 5000);

    status = viWrite(instr, reinterpret_cast<ViConstBuf>("*IDN?\n"), 6, reinterpret_cast<ViPUInt32>(&retCount));
    status = viRead(instr, reinterpret_cast<ViPBuf>(buffer), MAX_CNT, reinterpret_cast<ViPUInt32>(&retCount));

    printf("%s", buffer);

    status = viClose(instr);
    status = viClose(defaultRM);*/

   std::string test_json1 = R"({"key_1":"test_value","key_2":123,"key_3":[123,456,789],"key_4":{"key_4_1":"value_4_1","key_4_2":"value_4_2","key_4_3":"value_4_3"},"key_5":[123,456,789]})";
   std::string test_json2 = R"({"key_1":"test_value1", "key_2":"test_value2", "key_3":"test_value3"})";
   std::string test_json3 = R"({"key_1":123, "key_2":456, "key_3":789})";
   std::string test_json4 = R"({"key_1":[123,456,789], "key_2":[123,456,789], "key_3":[123,456,789]})";
   std::string test_json5 = R"({"key_1":123, "key_2":"test_value2", "key_3":[123,456,789]})";
   std::string test_json6 = "{\"key_1\":123, \"key_2\":\"test_value2\", \"key_3\":[123,456,789]}\r\n";

   json_t json;
   json.parse_string(test_json1);

    return 0;
}