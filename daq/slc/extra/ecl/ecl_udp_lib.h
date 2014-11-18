#ifndef _ECL_UDP_LIB_H
#define _ECL_UDP_LIB_H

#define ECL_COL_UDP_PORT 6000
#define ECL_UDP_PROTOCOL_ID 0xABCD

#if defined(__cplusplus)
extern "C" {
#endif

int ecl_udp_init(void);
int ecl_udp_init_ip(const char* pzIP_ADDR);
int ecl_udp_read_reg(unsigned short int reg_num, unsigned short int* reg_data);
int ecl_udp_write_reg(unsigned short int reg_num, unsigned short int reg_data);
int ecl_udp_write_mem(unsigned int addr, unsigned short int length, unsigned short int* buf);
int ecl_udp_read_mem(unsigned int addr, unsigned short int length, unsigned short int* buf);
void ecl_udp_close(void);

extern int ecl_udp_socket;
extern int ecl_udp_lib_debug;
extern char str_ecl_remote_host_name[];
extern char str_ecl_local_host_name[];
extern struct sockaddr_in dest_addr;
extern unsigned short int ecl_udp_packet_cnt;

#if defined(__cplusplus)
}
#endif

#endif
