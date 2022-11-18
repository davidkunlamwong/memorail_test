#include <modbus/modbus.h>
#include <modbus/modbus-rtu.h>
#include <iostream>
#include <string_view>

#define RS485_DEVICE "/dev/ttyUSB0"
#define SLAVE_ADDR 0x1

void help_msg()
{
    std::cout << "Arguments: R/W Register Quatity [Display(I/F/H/S)]" << std::endl;
}

/* args: R/W Register Quantity [Display] */
int main(int argc, char *argv[])
{
    // parse argument
    if (argc != 4 and argc != 5)
    {
        help_msg();
        return 0;
    }
    char rw = *(argv[1]);
    if (rw != 'R' and rw != 'W')
    {
        help_msg();
        return -1;
    }
    int reg = std::stoi(argv[2]) - 1;
    int qty = std::stoi(argv[3]);

    char disp = 'H';
    if (argc == 5)
    {
        disp = *(argv[4]);
        if (disp != 'I' and disp != 'F' and disp != 'H' and disp != 'S')
        {
            help_msg();
            return -1;
        }
        else if (disp == 'I' or disp == 'F')
        {
            if (qty > 4)
            {
                disp = 'H';
            }
        }
    }

    // comm
    int ret;
    modbus_t *mb = modbus_new_rtu(RS485_DEVICE, 115200, 'E', 8, 1);
    if (mb == nullptr)
    {
        std::cout << "modbus new rtu error" << std::endl;
        goto cleanup;
    }
    ret = modbus_set_slave(mb, SLAVE_ADDR);
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

    if (rw == 'R')
    {
        std::cout << "READ " << disp << " " << reg << " " << qty << std::endl;
        uint16_t rx_buf[32] = {};
        ret = modbus_read_registers(mb, reg, qty, rx_buf);
        if (ret != qty)
        {
            std::cout << "modbus read error ret=" << ret << std::endl;
            goto cleanup;
        }
        int words_received = ret;

        switch (disp)
        {
        case 'I':
            std::cout << "u16=" << rx_buf[0] << " u32=" << rx_buf[0] + rx_buf[1] * 0x10000 << std::endl;
            break;
        case 'F':
            {
                void *buf_ptr = static_cast<void *>(rx_buf);
                std::cout << "f32=" << *(static_cast<float *>(buf_ptr)) << std::endl;
            }
            break;
        case 'S':
            {
                void *buf_ptr = static_cast<void *>(rx_buf);
                char *str_rv = static_cast<char *>(buf_ptr);
                std::string_view sv(str_rv, words_received*2);
                std::cout << sv << std::endl;
            }
            break;
        case 'H':
            for (int i = 0; i < words_received; i++)
            {
                std::cout << std::hex << MODBUS_GET_LOW_BYTE(rx_buf[i]) << ' ' << MODBUS_GET_HIGH_BYTE(rx_buf[i]) << ' ';
                if (i % 4 == 3)
                {
                    std::cout << std::endl;
                }
            }
            std::cout << std::endl;
            break;

        default:
            break;
        }
    }
    else
    {
        std::cout << "modbus write NOT IMPLEMENTED!!!" << std::endl;
        goto cleanup;
    }

cleanup:
    modbus_close(mb);
    modbus_free(mb);

    return 0;
}