#include <modbus/modbus.h>
#include <modbus/modbus-rtu.h>
#include <iostream>

/* args: dev_path fncode start_reg qty_reg */
int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        return -1;
    }
    modbus_t *mb = modbus_new_rtu(argv[1], 19200, 'E', 8, 1);
    if (mb == nullptr) {
        return -2;
    }
    uint16_t tab_reg[32];

    int ret = modbus_set_slave(mb, 0x1);
    std::cout << "modbus set slave: " << ret << std::endl;
    ret = modbus_connect(mb);
    std::cout << "modbus connect ret: " << ret << std::endl;


    // send raw req
    // slave, fncode, addr_hi, addr_lo, qty_hi, qty_lo
    int addr = 212;
    int qty = 1;
    uint32_t payload = 6; 
    void* ptr = static_cast<void*>(&payload);
    ret = modbus_write_registers(mb, addr-1, qty, static_cast<uint16_t*>(ptr));
    std::cout << "modbus write register ret: " << ret << std::endl;




    modbus_close(mb);
    modbus_free(mb);
}