#ifndef _Belle2_ARICHFEBSA0x_h
#define _Belle2_ARICHFEBSA0x_h

#include "daq/slc/copper/arich/ARICHFEB.h"

#define ADC_VCC 3.3

namespace Belle2 {

  class ARICHFEBSA0x : public ARICHFEB {
  public:
    /*  enum { ADDR_CLEAR        = 0x10000000,
     ADDR_RUNCLEAR     = 0x10000001,
     ADDR_STAT         = 0x10000002,
     ADDR_CSR1         = 0x10000003, // { ro_mode, polarity, lvds_trig,
     //                             mode_edge, vetoen }
     ADDR_HDCYCLE      = 0x10000004, // { hdcycle[5:0] }
     ADDR_TRGDELAY     = 0x10000005, // { trgdelay[5:0] }
     ADDR_CSR2         = 0x10000006, // { pio_trigmode, pio_halfcycle[2:0] }
     ADDR_SOFTTRIG     = 0x10000007,
     ADDR_TP           = 0x10000008, // { tp }
     ADDR_TRIGEN       = 0x10000009, // { trigen }
     ADDR_UTILBUF0     = 0x1000000a,
     ADDR_UTILBUF1     = 0x1000000b,
     //  ADDR_SELSA    = 0x0c, // reserved for sa03tmu
     //  ADDR_SELCH    = 0x0d, // reserved for sa03tmu
     ADDR_INIT         = 0x10000010,
     ADDR_SEL          = 0x10000011,
     ADDR_PRMSET       = 0x10000012,
     ADDR_NDRO         = 0x10000013,
     ADDR_LASTSEL      = 0x10000014, // 4 bytes
     ADDR_WPARAM       = 0x10000020, // 4 bytes
     ADDR_RPARAM       = 0x10000024, // 4 bytes
     ADDR_PTM1_DATA    = 0x10000040, // 2 bytes
     ADDR_PTM1_CMD     = 0x10000042,
     ADDR_PTM1_RB_DATA = 0x10000044, // 2 bytes
     ADDR_PTM1_RB_CMD  = 0x10000046,
     ADDR_PTM2_DATA    = 0x10000050, // 2 bytes
     ADDR_PTM2_CMD     = 0x10000052,
     ADDR_PTM2_RB_DATA = 0x10000054, // 2 bytes
     ADDR_PTM2_RB_CMD  = 0x10000056,
     ADDR_MUX          = 0x10000060, // { mux_inh, mux_b, mux_a }
     ADDR_TS1_DATA     = 0x10000068, // 2 bytes
     ADDR_TS2_DATA     = 0x1000006a, // 2 bytes

       ADDR_MG_RST     = 0x05, // 2 bytes
       ADDR_MG_TPG     = 0x08, // 2 bytes
    */

    enum { ADDR_CLEAR        = 0x00,
           ADDR_RUNCLEAR     = 0x01,
           ADDR_STAT         = 0x02,
           ADDR_CSR1         = 0x03, // { ro_mode, polarity, lvds_trig,
           //                             mode_edge, vetoen }
           ADDR_HDCYCLE      = 0x04, // { hdcycle[5:0] }
           ADDR_TRGDELAY     = 0x05, // { trgdelay[5:0] }
           ADDR_CSR2         = 0x06, // { pio_trigmode, pio_halfcycle[2:0] }
           ADDR_SOFTTRIG     = 0x07,
           ADDR_TP           = 0x08, // { tp }
           ADDR_TRIGEN       = 0x09, // { trigen }
           ADDR_UTILBUF0     = 0x0a,
           ADDR_UTILBUF1     = 0x0b,
           //  ADDR_SELSA    = 0x0c, // reserved for sa03tmu
           //  ADDR_SELCH    = 0x0d, // reserved for sa03tmu
           ADDR_INIT         = 0x10,
           ADDR_SEL          = 0x11,
           ADDR_PRMSET       = 0x12,
           ADDR_NDRO         = 0x13,
           ADDR_LASTSEL      = 0x14, // 4 bytes
           ADDR_WPARAM       = 0x20, // 4 bytes
           ADDR_RPARAM       = 0x24, // 4 bytes
           ADDR_PTM1_DATA    = 0x40, // 2 bytes
           ADDR_PTM1_CMD     = 0x42,
           ADDR_PTM1_RB_DATA = 0x44, // 2 bytes
           ADDR_PTM1_RB_CMD  = 0x46,
           ADDR_PTM2_DATA    = 0x50, // 2 bytes
           ADDR_PTM2_CMD     = 0x52,
           ADDR_PTM2_RB_DATA = 0x54, // 2 bytes
           ADDR_PTM2_RB_CMD  = 0x56,
           ADDR_MUX          = 0x60, // { mux_inh, mux_b, mux_a }
           ADDR_ADC_DATA0    = 0x64,
           ADDR_ADC_DATA1    = 0x65,
           ADDR_TS1_DATA     = 0x68, // 2 bytes
           ADDR_TS2_DATA     = 0x6a, // 2 bytes

         };
    enum { MASK_VETOEN       =  0x1, SHIFT_VETOEN        = 0,
           MASK_MODE_EDGE    =  0x2, SHIFT_MODE_EDGE     = 1,
           MASK_LVDS_TRIG    =  0x8, SHIFT_LVDS_TRIG     = 2,
           MASK_POLARITY     =  0x8, SHIFT_POLARITY      = 3,
           MASK_RO_MODE      = 0x10, SHIFT_RO_MODE       = 4,
         };
  public:
    ARICHFEBSA0x(HSLB& hslb, int febnum);
    ~ARICHFEBSA0x() {}
    /* write operation */
    void clear() { write1(ADDR_CLEAR); }
    void runclear() { write1(ADDR_RUNCLEAR); }
    void csr1(unsigned char v) { write1(ADDR_CSR1, v); }
    void hdcycle(unsigned char v) { write1(ADDR_HDCYCLE, v); }
    void trgdelay(unsigned char v) { write1(ADDR_TRGDELAY, v); }
    void csr2(unsigned char v) { write1(ADDR_CSR2, v); }
    void softtrig() { write1(ADDR_SOFTTRIG); }
    void tp(unsigned char v) { write1(ADDR_TP, v); }
    void trigen(unsigned char v) { write1(ADDR_TRIGEN, v); }
    void init() { write1(ADDR_INIT); }
    void utilbuf0(unsigned char v) { write1(ADDR_UTILBUF0, v); }
    void utilbuf1(unsigned char v) { write1(ADDR_UTILBUF1, v); }
    void utilbuf(unsigned int v) { write2(ADDR_UTILBUF0, v); }
    void sel(unsigned char v, int usec = 100);
    void prmset() { write1(ADDR_PRMSET); }
    void ndro() { write1(ADDR_NDRO); }
    void wparam(unsigned int data, int usec = 100);
    void mux(unsigned char v) { write1(ADDR_MUX, v); }
    void ptm1_data(unsigned int d) { write2(ADDR_PTM1_DATA, d); }
    void ptm1_cmd(unsigned char v) { write2(ADDR_PTM1_CMD, v); }
    void ptm2_data(unsigned int d) { write2(ADDR_PTM2_DATA, d); }
    void ptm2_cmd(unsigned char v) { write2(ADDR_PTM2_CMD, v); }
    void ts1_update() { write1(ADDR_TS1_DATA, 0); }
    void ts2_update() { write1(ADDR_TS2_DATA, 0); }
    //void mg_rst(unsigned char v ) { write1( febnum, ADDR_MG_RST, v ); }
    //void mg_tpg(unsigned char v ) { write1( febnum, ADDR_MG_TPG, v ); }
    /* read operation */
    unsigned char stat() { return read1(ADDR_STAT); }
    unsigned char csr1() { return read1(ADDR_CSR1); }
    unsigned char hdcycle() { return read1(ADDR_HDCYCLE); }
    unsigned char trgdelay() { return read1(ADDR_TRGDELAY); }
    unsigned char csr2() { return read1(ADDR_CSR2); }
    unsigned char tp() { return read1(ADDR_TP); }
    unsigned char trigen() { return read1(ADDR_TRIGEN); }
    unsigned char utilbuf0() { return read1(ADDR_UTILBUF0); }
    unsigned char utilbuf1() { return read1(ADDR_UTILBUF1); }
    unsigned int utilbuf() { return read2(ADDR_UTILBUF0); }
    unsigned char sel() { return read1(ADDR_SEL); }
    unsigned int lastsel() { return read4(ADDR_LASTSEL); }
    unsigned char lastsel(int chip) { return read1(ADDR_LASTSEL + chip); }
    unsigned int wparam() { return read4(ADDR_WPARAM); }
    unsigned int rparam();// { return read4(ADDR_RPARAM); }
    unsigned char mux() { return read1(ADDR_MUX); }
    unsigned int ptm1_data() { return read2(ADDR_PTM1_DATA); }
    unsigned char ptm1_cmd() { return read2(ADDR_PTM1_CMD); }
    unsigned int ptm1_rb_data() { return read2(ADDR_PTM1_RB_DATA); }
    unsigned char ptm1_rb_cmd() { return read2(ADDR_PTM1_RB_CMD); }
    unsigned int ptm2_data() { return read2(ADDR_PTM2_DATA); }
    unsigned char ptm2_cmd() { return read2(ADDR_PTM2_CMD); }
    unsigned int ptm2_rb_data() { return read2(ADDR_PTM2_RB_DATA); }
    unsigned char ptm2_rb_cmd() { return read2(ADDR_PTM2_RB_CMD); }
    unsigned int adc();
    unsigned int ts1_data() { return read2(ADDR_TS1_DATA); }
    unsigned int ts2_data() { return read2(ADDR_TS2_DATA); }
    /* useful functions */
    unsigned int select(unsigned int chip, int ch = -1, int usec = 100);
    unsigned int setparam(unsigned int chip, int ch, unsigned int data);
    unsigned int ndro_param(unsigned int chip, int ch);
    /* functions to set CSR */
    void vetoen(int v) { setcsr1(v, MASK_VETOEN, SHIFT_VETOEN); }
    void mode_edge(int v) { setcsr1(v, MASK_MODE_EDGE, SHIFT_MODE_EDGE); }
    void polarity(int v) { setcsr1(v, MASK_POLARITY, SHIFT_POLARITY); }
    double voltage(unsigned int imux);
    void go();

  private:
    void setcsr1(int value, unsigned int mask, unsigned int shift);
  };

}

#endif
