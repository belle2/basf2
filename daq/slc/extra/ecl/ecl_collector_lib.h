#ifndef _ECL_COLLECTOR_LIB_H
#define _ECL_COLLECTOR_LIB_H

#ifdef __cplusplus
extern "C" {
#endif

int sh_read_reg(int sh_num, int sh_reg_num, int* sh_reg_data, unsigned short int* read_ok_mask);
int sh_write_reg(int sh_num, int sh_reg_num, int sh_reg_wdata);
int sh_read_dsp_data(int sh_num, int sh_dsp_data_type, unsigned short int* buf);
int sh_write_dsp_data(int sh_num, int sh_dsp_data_type, unsigned short int* buf);

int sh_write_sdram_data(int sh_num, int chip, int bank, int row, int col, unsigned short int* buf);


int sh_get_event(int sh_num, int* event_data);
int sh_get_mem_data(int sh_num, int* mem_data);

int sh_boot(const char* ip_addr, int sh_num, unsigned int mem_addr);
int sh_init_ecldsp(const char* ip_addr, int sh_num, unsigned int mem_addr);
int sh_reg_io(const char* ip_addr, const char* opt, int sh_num, int reg_num, int reg_wdata);

#ifdef __cplusplus
}
#endif

#endif
