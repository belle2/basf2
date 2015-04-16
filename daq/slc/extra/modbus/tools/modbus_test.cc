#include "modbus/modbustcp.h"

#include <stdio.h>

int main()
{
  int fd = modbus_connect("modbus", 502);
  modbus_adu_t adu;
  adu.hdr.trans_id = 0x01;
  adu.hdr.unit_id = 0x02;
  if (fd <= 0) return -1;
  return 0;
}
