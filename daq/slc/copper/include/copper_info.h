#ifndef _copper_info_h
#define _copper_info_h

struct copper_info {
  unsigned int ff_sta;
  unsigned int conf_w_ae[4];
  unsigned int conf_w_ff[4];
  unsigned int conf_w_af[4];
  unsigned int ff_rst;
  unsigned int finesse_sta;
  unsigned int almfull_enb;
  unsigned int version;
  unsigned int trgcm;
  unsigned int trg_leng;
  unsigned int lef_readsel;
  unsigned int lef_total;
  unsigned int lef[2];
  unsigned int lef_sta;
  unsigned int lef_w_ff[4];
  unsigned int lef_w_af[4];
  unsigned int int_sta;
  unsigned int int_mask;
  unsigned int ff_rw;
  unsigned int int_factor;
  unsigned int ff_w[4];
  unsigned int ff_r[4];
  unsigned int ff_dma[4];
  unsigned int ewrp_w[4];
  unsigned int we_counter[4];
  unsigned int lwr_dma[4];
  unsigned int dma_trans;
  unsigned int dma_ts_enb;
};

#endif
