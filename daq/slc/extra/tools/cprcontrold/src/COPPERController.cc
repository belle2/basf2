#include "COPPERController.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <mgt/copper.h>

using namespace Belle2;

bool COPPERController::open() throw()
{
  if ((m_fd = ::open("/dev/copper/copper_ctl", O_RDONLY)) < 0) {
    printf("can't open /dev/copper/copper_ctl, %s\n",
           strerror(errno));
    m_fd = -1;
    return false;
  }
  return true;
}

bool COPPERController::close() throw()
{
  if (m_fd > 0) {
    ::close(m_fd);
    m_fd = -1;
  }
  return true;
}

int COPPERController::read(int request) throw(IOException)
{
  int ret;
  int val;
  if ((ret = ioctl(m_fd, request, &val)) < 0) {
    throw (IOException("can't ioctl: %s\n", strerror(errno)));
  }
  return val;
}

copper_info& COPPERController::monitor() throw()
{
  memset(&m_info, 0, sizeof(copper_info));
  try {
    m_info.ff_sta = read(CPRIOGET_FF_STA);
    m_info.conf_w_ae[0] = read(CPRIOGET_CONF_WA_AE);
    m_info.conf_w_ae[1] = read(CPRIOGET_CONF_WB_AE);
    m_info.conf_w_ae[2] = read(CPRIOGET_CONF_WC_AE);
    m_info.conf_w_ae[3] = read(CPRIOGET_CONF_WD_AE);
    m_info.conf_w_ff[0] = read(CPRIOGET_CONF_WA_FF);
    m_info.conf_w_ff[1] = read(CPRIOGET_CONF_WB_FF);
    m_info.conf_w_ff[2] = read(CPRIOGET_CONF_WC_FF);
    m_info.conf_w_ff[3] = read(CPRIOGET_CONF_WD_FF);
    m_info.conf_w_af[0] = read(CPRIOGET_CONF_WA_AF);
    m_info.conf_w_af[1] = read(CPRIOGET_CONF_WB_AF);
    m_info.conf_w_af[2] = read(CPRIOGET_CONF_WC_AF);
    m_info.conf_w_af[3] = read(CPRIOGET_CONF_WD_AF);
    //m_info.ff_rst = read(CPRIOGET_FF_RST);
    m_info.finesse_sta = read(CPRIOGET_FINESSE_STA);
    //m_info.almfull_enb = read(CPRIOGET_ALMFULL_ENB);
    m_info.version = read(CPRIOGET_VERSION);

    //m_info.trgcm = read(CPRIOGET_TRGCM);
    //m_info.trg_leng = read(CPRIOGET_TRG_LENG);
    //m_info.lef_readsel = read(CPRIOGET_LEF_READSEL);
    //m_info.lef_total = read(CPRIOGET_LEF_TOTAL);
    ///*
    m_info.lef[0] = read(CPRIOGET_LEF_AB);
    m_info.lef[1] = read(CPRIOGET_LEF_CD);
    m_info.lef_sta = read(CPRIOGET_LEF_STA);
    m_info.lef_w_ff[0] = read(CPRIOGET_LEF_WA_FF);
    m_info.lef_w_ff[1] = read(CPRIOGET_LEF_WB_FF);
    m_info.lef_w_ff[2] = read(CPRIOGET_LEF_WC_FF);
    m_info.lef_w_ff[3] = read(CPRIOGET_LEF_WD_FF);
    m_info.lef_w_af[0] = read(CPRIOGET_LEF_WA_AF);
    m_info.lef_w_af[1] = read(CPRIOGET_LEF_WB_AF);
    m_info.lef_w_af[2] = read(CPRIOGET_LEF_WC_AF);
    m_info.lef_w_af[3] = read(CPRIOGET_LEF_WD_AF);
    //*/
    //m_info.int_sta = read(CPRIOGET_INT_STA);
    //m_info.int_mask = read(CPRIOGET_INT_MASK);
    //m_info.ff_rw = read(CPRIOGET_FF_RW);
    //m_info.int_factor = read(CPRIOGET_INT_FACTOR);
    /*
    m_info.ewrp_w[0] = read(CPRIOGET_EWRP_WA);
    m_info.ewrp_w[1] = read(CPRIOGET_EWRP_WB);
    m_info.ewrp_w[2] = read(CPRIOGET_EWRP_WC);
    m_info.ewrp_w[3] = read(CPRIOGET_EWRP_WD);
    m_info.we_counter[0] = read(CPRIOGET_WEA_COUNTER);
    m_info.we_counter[1] = read(CPRIOGET_WEB_COUNTER);
    m_info.we_counter[2] = read(CPRIOGET_WEC_COUNTER);
    m_info.we_counter[3] = read(CPRIOGET_WED_COUNTER);
    */

    /*
    m_info.lwr_dma[0] = read(CPRIOGET_LWRA_DMA);
    m_info.lwr_dma[1] = read(CPRIOGET_LWRB_DMA);
    m_info.lwr_dma[2] = read(CPRIOGET_LWRC_DMA);
    m_info.lwr_dma[3] = read(CPRIOGET_LWRD_DMA);
    */
    /*
    m_info.dma_trans = read(CPRIOGET_DMA_TRANS);
    m_info.dma_ts_enb = read(CPRIOGET_TS_ENV);
    */
  } catch (const IOException& e) {
    printf("IO error : %s", e.what());
    ::close(m_fd);
    m_fd = -1;
  }
  return m_info;
}

void COPPERController::print() throw()
{
  printf("%x\n", m_info.ff_sta);
  printf("%x\n", m_info.conf_w_ae[0]);
  printf("%x\n", m_info.conf_w_ae[1]);
  printf("%x\n", m_info.conf_w_ae[2]);
  printf("%x\n", m_info.conf_w_ae[3]);
  printf("%x\n", m_info.conf_w_ff[0]);
  printf("%x\n", m_info.conf_w_ff[1]);
  printf("%x\n", m_info.conf_w_ff[2]);
  printf("%x\n", m_info.conf_w_ff[3]);
  printf("%x\n", m_info.conf_w_af[0]);
  printf("%x\n", m_info.conf_w_af[1]);
  printf("%x\n", m_info.conf_w_af[2]);
  printf("%x\n", m_info.conf_w_af[3]);
  printf("%x\n", m_info.ff_rst);
  printf("%x\n", m_info.finesse_sta);
  printf("%x\n", m_info.almfull_enb);
  printf("%x\n", m_info.version);
  printf("%x\n", m_info.trgcm);
  printf("%x\n", m_info.trg_leng);
  printf("%x\n", m_info.lef_readsel);
  printf("%x\n", m_info.lef_total);
  printf("%x\n", m_info.lef[0]);
  printf("%x\n", m_info.lef[1]);
  printf("%x\n", m_info.lef_sta);
  printf("%x\n", m_info.lef_w_ff[0]);
  printf("%x\n", m_info.lef_w_ff[1]);
  printf("%x\n", m_info.lef_w_ff[2]);
  printf("%x\n", m_info.lef_w_ff[3]);
  printf("%x\n", m_info.lef_w_af[0]);
  printf("%x\n", m_info.lef_w_af[1]);
  printf("%x\n", m_info.lef_w_af[2]);
  printf("%x\n", m_info.lef_w_af[3]);
  printf("%x\n", m_info.int_sta);
  printf("%x\n", m_info.int_mask);
  printf("%x\n", m_info.ff_rw);
  printf("%x\n", m_info.int_factor);
  printf("%x\n", m_info.ewrp_w[0]);
  printf("%x\n", m_info.ewrp_w[1]);
  printf("%x\n", m_info.ewrp_w[2]);
  printf("%x\n", m_info.ewrp_w[3]);

  printf("%x\n", m_info.we_counter[0]);
  printf("%x\n", m_info.we_counter[1]);
  printf("%x\n", m_info.we_counter[2]);
  printf("%x\n", m_info.we_counter[3]);

  printf("%x\n", m_info.lwr_dma[0]);
  printf("%x\n", m_info.lwr_dma[1]);
  printf("%x\n", m_info.lwr_dma[2]);
  printf("%x\n", m_info.lwr_dma[3]);

  printf("%x\n", m_info.dma_trans);
  printf("%x\n", m_info.dma_ts_enb);
}

bool COPPERController::isFifoFull() throw()
{
  if (m_fd <= 0) return false;
  return (m_info.ff_sta & 0xff) == 0x3c
         || (m_info.ff_sta & 0xff) == 0x2c;
}

bool COPPERController::isFifoEmpty() throw()
{
  if (m_fd <= 0) return false;
  return (m_info.ff_sta & 0xff) == 0x23;
}

bool COPPERController::isLengthFifoFull() throw()
{
  if (m_fd <= 0) return false;
  return (m_info.lef_sta & 0xf) == 0xe;
}

