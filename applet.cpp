#include <modbus/modbus.h>
#include <modbus/modbus-rtu.h>
#include <iostream>
#include <sstream>
#include <thread>
#include <chrono>

std::string printRegister(uint16_t *rx_buf, int words_received, char disp)
{
    std::stringstream ss;
    switch (disp)
    {
    case 'I':
        ss << rx_buf[0] + rx_buf[1] * 0x10000;
        break;
    case 'F':
    {
        void *buf_ptr = static_cast<void *>(rx_buf);
        ss << *(static_cast<float *>(buf_ptr));
    }
    break;
    case 'S':
    {
        void *buf_ptr = static_cast<void *>(rx_buf);
        char *str_rv = static_cast<char *>(buf_ptr);
        std::string_view sv(str_rv, words_received * 2);
        ss << sv;
    }
    break;
    case 'H':
        for (int i = 0; i < words_received; i++)
        {
            ss << std::hex << MODBUS_GET_LOW_BYTE(rx_buf[i]) << ' ' << MODBUS_GET_HIGH_BYTE(rx_buf[i]) << ' ';
            if (i % 4 == 3)
            {
                ss << std::endl;
            }
        }
        break;

    default:
        break;
    }
    return ss.str();
}

#define DEVICE_FIRMWARE 1000 // 12 S
#define DEVICE_MANU 1024     // 12 S
#define DEVICE_NAME 1048     // 12 S
#define DEVICE_ORDER 1072    // 12 S
#define DEVICE_SERIAL 1096   // 12 S
#define CH2 +10000
#define SENSOR_INFO 678     // 1 HEX
#define SENSOR_TYPE 680     // 1 HEX
#define SENSOR_MANU 500     // 12 S
#define SENSOR_ORDER 524    // 12 S
#define SENSOR_SERIAL 548   // 12 S
#define SENSOR_NAME 572     // 12 S
#define SENSOR_SOFTWARE 596 // 12 S
#define SENSOR_HARDWARE 620 // 12 S

#define PH_VAL 2066     // 2 F
#define PH_VTG 2024     // 2 F
#define PH_TMP 2012     // 2 F
#define PH_RES 2036     // 2 F
#define PH_ORP 2048     // 2 F
#define PH_ORP_RES 2060 // 2 F
#define PH_LEAK 2084    // 3 F+HEX

#define CI_TMP 5012
#define CI_CDC 5024
#define CI_CDT 5036
#define CI_RES 5042

#define NEWLINE std::cout << std::endl;

std::string read(modbus_t *mb, int addr, int qty, char disp)
{
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(500ns);
    uint16_t buf[32] = {0};
    int ret;
    int retry = 10;
    while (retry > 0)
    {
        retry--;
        ret = modbus_read_registers(mb, addr - 1, qty, buf);
        if (ret > 0)
            return printRegister(buf, ret, disp);
    }
    return " ### ";
}

void pH_proc(modbus_t *mb, bool ch2)
{
    if (ch2)
    {
        std::cout << "\tpH=" << read(mb, PH_VAL CH2, 2, 'F') << " " << std::flush;
        std::cout << read(mb, PH_VTG CH2, 2, 'F') << "mV " << std::flush;
        std::cout << read(mb, PH_RES CH2, 2, 'F') << "Ω " << std::flush;
        std::cout << "\tORP=" << read(mb, PH_ORP CH2, 2, 'F') << "mV " << std::flush;
        std::cout << read(mb, PH_ORP_RES CH2, 2, 'F') << "Ω " << std::flush;
        std::cout << read(mb, PH_TMP CH2, 2, 'F') << "C " << std::flush;
    }
    else
    {
        std::cout << "pH=" << read(mb, PH_VAL, 2, 'F') << " " << std::flush;
        std::cout << read(mb, PH_VTG, 2, 'F') << "mV " << std::flush;
        std::cout << read(mb, PH_RES, 2, 'F') << "Ω " << std::flush;
        std::cout << "\tORP=" << read(mb, PH_ORP, 2, 'F') << "mV " << std::flush;
        std::cout << read(mb, PH_ORP_RES, 2, 'F') << "Ω " << std::flush;
        std::cout << read(mb, PH_TMP, 2, 'F') << "C " << std::flush;
    }
}

void condi_proc(modbus_t *mb, bool ch2)
{
    if (ch2)
    {
        std::cout << "\tcond=" << read(mb, CI_CDC CH2, 2, 'F') << "μS " << std::flush;
        std::cout << read(mb, CI_CDT CH2, 2, 'F') << "μS/cm " << std::flush;
        std::cout << read(mb, CI_RES CH2, 2, 'F') << "Ωm " << std::flush;
        std::cout << read(mb, CI_TMP CH2, 2, 'F') << "C " << std::flush;
    }
    else
    {
        std::cout << "cond=" << read(mb, CI_CDC, 2, 'F') << "μS " << std::flush;
        std::cout << read(mb, CI_CDT, 2, 'F') << "μS/cm " << std::flush;
        std::cout << read(mb, CI_RES, 2, 'F') << "Ωm " << std::flush;
        std::cout << read(mb, CI_TMP, 2, 'F') << "C " << std::flush;
    }
}

void applet(modbus_t *mb)
{
    // uint32_t payload = 70000;
    // void *ptr = static_cast<void *>(&payload);
    // int ret = modbus_write_registers(mb, 1200 - 1, 2, static_cast<uint16_t *>(ptr));

    std::cout << "MemoRail" << std::endl;
    std::cout << read(mb, DEVICE_MANU, 12, 'S') << std::flush;
    std::cout << read(mb, DEVICE_NAME, 12, 'S') << std::endl;
    std::cout << read(mb, DEVICE_ORDER, 12, 'S') << std::flush;
    std::cout << read(mb, DEVICE_SERIAL, 12, 'S') << std::endl;
    std::cout << read(mb, DEVICE_FIRMWARE, 12, 'S') << std::endl;
    std::cout << std::endl;

    int ch1type = 0, ch2type = 0;

    int ch1info = std::stoi(read(mb, SENSOR_INFO, 1, 'I'));
    if (0 == ch1info)
    {
        ch1type = std::stoi(read(mb, SENSOR_TYPE, 1, 'I'));
        std::cout << "CH1 Sensor Type: "
                  << ((1 == ch1type)
                          ? "PH"
                      : (4 == ch1type) ? "CONDI"
                                       : "???")
                  << std::endl;
        std::cout << read(mb, SENSOR_MANU, 12, 'S') << std::flush;
        std::cout << read(mb, SENSOR_NAME, 12, 'S') << std::endl;
        std::cout << read(mb, SENSOR_SERIAL, 12, 'S') << std::flush;
        std::cout << read(mb, SENSOR_ORDER, 12, 'S') << std::endl;
        std::cout << read(mb, SENSOR_SOFTWARE, 12, 'S') << std::flush;
        std::cout << read(mb, SENSOR_HARDWARE, 12, 'S') << std::endl;
    }
    else
    {
        std::cout << "CH1 ";
        if (ch1info & 0x1)
            std::cout << "No Sensor ";
        if (ch1info & 0x2)
            std::cout << "Unknown Sensor";
        if (ch1info & 0x4)
            std::cout << "Invalid Calib Param";
        std::cout << std::endl;
    }
    std::cout << std::endl;
    int ch2info = std::stoi(read(mb, SENSOR_INFO CH2, 1, 'I'));
    if (0 == ch1info)
    {
        ch2type = std::stoi(read(mb, SENSOR_TYPE CH2, 1, 'I'));
        std::cout << "CH2 Sensor Type: "
                  << ((1 == ch2type)
                          ? "PH"
                      : (4 == ch2type) ? "CONDI"
                                       : "???")
                  << std::endl;
        std::cout << read(mb, SENSOR_MANU  CH2, 12, 'S') << std::flush;
        std::cout << read(mb, SENSOR_NAME CH2, 12, 'S') << std::endl;
        std::cout << read(mb, SENSOR_SERIAL CH2, 12, 'S') << std::flush;
        std::cout << read(mb, SENSOR_ORDER CH2, 12, 'S') << std::endl;
        std::cout << read(mb, SENSOR_SOFTWARE CH2, 12, 'S') << std::flush;
        std::cout << read(mb, SENSOR_HARDWARE CH2, 12, 'S') << std::endl;
    }
    else
    {
        std::cout << "CH2 ";
        if (ch2info & 0x1)
            std::cout << "No Sensor ";
        if (ch2info & 0x2)
            std::cout << "Unknown Sensor";
        if (ch2info & 0x4)
            std::cout << "Invalid Calib Param";
        std::cout << std::endl;
    }
    std::cout << std::endl;
    // loop
    using namespace std::chrono_literals;
    auto sleep = 1s;
    while (true)
    {
        pH_proc(mb, false);
        condi_proc(mb, true);
        std::cout << std::endl;

        std::this_thread::sleep_for(sleep);
    }
}

/* args: dev_path */
int main(int argc, char *argv[])
{
    modbus_t *mb;
    int ret;
    // open context
    if (argc < 2)
    {
        mb = modbus_new_rtu("/dev/ttyUSB0", 115200, 'E', 8, 1);
    }
    else
    {

        mb = modbus_new_rtu(argv[1], 19200, 'E', 8, 1);
    }
    if (mb == nullptr)
    {
        std::cout << "modbus new rtu error" << std::endl;
        goto cleanup;
    }
    ret = modbus_set_slave(mb, 0x1);
    if (ret < 0)
    {
        std::cout << "modbus set slave error" << std::endl;
        goto cleanup;
    }
    ret = modbus_connect(mb);
    if (ret < 0)
    {
        std::cout << "modbus connect error" << std::endl;
        goto cleanup;
    }

    applet(mb);

cleanup:
    modbus_close(mb);
    modbus_free(mb);
}