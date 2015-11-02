#include "daq/slc/apps/ecldspd/ECLShaperControllerCallback.h"

#include "daq/slc/apps/ecldspd/ECLShaperControllerHandler.h"

#include <daq/slc/system/LogFile.h>
#include <daq/slc/base/StringUtil.h>

#include <ecldaq/ecl_collector_lib.h>
extern "C" {
  const char* col_status(const char* ip_addr);
}

using namespace Belle2;

void ECLShaperControllerCallback::initialize(const DBObject& obj)
throw(RCHandlerException)
{
  configure(obj);
}

void ECLShaperControllerCallback::configure(const DBObject& obj)
throw(RCHandlerException)
{
  const DBObject& o_cols(obj("cols"));
  for (int col = o_cols.getInt("min");
       col <= o_cols.getInt("max"); col++) {
    std::string vname = StringUtil::form("col[%d].status", col);
    add(new ECLShaperStatusHandler(*this, vname, col));
  }
  add(new ECLShaperBootHandler(*this, "boot"));
}

void ECLShaperControllerCallback::boot(const DBObject& obj)
throw(RCHandlerException)
{
  if (obj.hasObject("cols")) {
    const DBObject& o_cols(obj("cols"));
    char ip [256];
    // loshf-all
    const unsigned int mem_bin_addr = 0xA7000000;
    const unsigned int mem_ecldsp_addr = 0xA8000000;
    for (int col = o_cols.getInt("min");
         col <= o_cols.getInt("max"); col++) {
      sprintf(ip, "192.168.1.%d", col);
      printf("booting ip %s\n", ip);
      sh_boot(ip, 16, mem_bin_addr);
    }
    // loshc-all 1
    for (int col = o_cols.getInt("min");
         col <= o_cols.getInt("max"); col++) {
      sprintf(ip, "192.168.1.%d", col);
      sh_init_ecldsp(ip, 16, mem_ecldsp_addr);
    }
  }
}

void ECLShaperControllerCallback::load(const DBObject& obj)
throw(RCHandlerException)
{
  if (!m_forced) return;
  if (obj.hasObject("cols")) {
    const DBObject& o_cols(obj("cols"));
    char ip [256];
    // loshf-all
    for (int col = o_cols.getInt("min");
         col <= o_cols.getInt("max"); col++) {
      sprintf(ip, "192.168.1.%d", col);
      w_sh_reg_io(ip, 0x502, obj.getInt("thread_af"));
      w_sh_reg_io(ip, 0x500, obj.getInt("trbuf"));
      w_sh_reg_io(ip, 0x501, obj.getInt("uthread_af"));

      w_sh_reg_io(ip, 0x200, obj.getInt("shaper_proc_mask"));
      w_sh_reg_io(ip, 0x210, obj.getInt("shaper_proc_num"));

      w_sh_reg_io(ip, 0x208, obj.getInt("shaper_adc_mask"));
      w_sh_reg_io(ip, 0x218, obj.getInt("shaper_adc_num"));

      w_sh_reg_io(ip, 0x220, obj.getInt("trg2adc_data_end"));

      w_sh_reg_io(ip, 0x20, obj.getInt("adc1_comp_value"));
      w_sh_reg_io(ip, 0x21, obj.getInt("adc2_comp_value"));
      w_sh_reg_io(ip, 0x22, obj.getInt("adc3_comp_value"));
      w_sh_reg_io(ip, 0x23, obj.getInt("adc4_comp_value"));
      w_sh_reg_io(ip, 0x24, obj.getInt("adc5_comp_value"));
      w_sh_reg_io(ip, 0x25, obj.getInt("adc6_comp_value"));
      w_sh_reg_io(ip, 0x26, obj.getInt("adc7_comp_value"));
      w_sh_reg_io(ip, 0x27, obj.getInt("adc8_comp_value"));
      w_sh_reg_io(ip, 0x28, obj.getInt("adc9_comp_value"));
      w_sh_reg_io(ip, 0x29, obj.getInt("adc10_comp_value"));
      w_sh_reg_io(ip, 0x2A, obj.getInt("adc11_comp_value"));
      w_sh_reg_io(ip, 0x2B, obj.getInt("adc12_comp_value"));
      w_sh_reg_io(ip, 0x2C, obj.getInt("adc13_comp_value"));
      w_sh_reg_io(ip, 0x2D, obj.getInt("adc14_comp_value"));
      w_sh_reg_io(ip, 0x2E, obj.getInt("adc15_comp_value"));
      w_sh_reg_io(ip, 0x2F, obj.getInt("adc16_comp_value"));

      w_sh_reg_io(ip, 0x40, obj.getInt("sh_relay"));

      w_sh_reg_io(ip, 0x223, obj.getInt("adc_data_len"));
      w_sh_reg_io(ip, 0x800, 1);

      // reset DAC
      w_col_reg_io(ip, 0xC000, 1);

      w_sh_reg_io(ip, 0x30, obj.getInt("pot1"));
      w_sh_reg_io(ip, 0x31, 0x10);
      usleep(100000);

      w_sh_reg_io(ip, 0x30, obj.getInt("pot2"));
      w_sh_reg_io(ip, 0x31, 0x11);
      usleep(100000);

      w_sh_reg_io(ip, 0x30, obj.getInt("pot3"));
      w_sh_reg_io(ip, 0x31, 0x12);
      usleep(100000);

      w_sh_reg_io(ip, 0x30, obj.getInt("pot4"));
      w_sh_reg_io(ip, 0x31, 0x13);
      usleep(100000);

      w_sh_reg_io(ip, 0x30, obj.getInt("pot5"));
      w_sh_reg_io(ip, 0x31, 0x14);
      usleep(100000);

      w_sh_reg_io(ip, 0x30, obj.getInt("pot6"));
      w_sh_reg_io(ip, 0x31 , 0x15);
      usleep(100000);

      w_sh_reg_io(ip, 0x30, obj.getInt("pot7"));
      w_sh_reg_io(ip, 0x31, 0x16);
      usleep(100000);

      w_sh_reg_io(ip, 0x30, obj.getInt("pot8"));
      w_sh_reg_io(ip, 0x31, 0x17);
      usleep(100000);

      w_sh_reg_io(ip, 0x30, obj.getInt("pot9"));
      w_sh_reg_io(ip, 0x31, 0x18);
      usleep(100000);

      w_sh_reg_io(ip, 0x30, obj.getInt("pot10"));
      w_sh_reg_io(ip, 0x31, 0x19);
      usleep(100000);

      w_sh_reg_io(ip, 0x30, obj.getInt("pot11"));
      w_sh_reg_io(ip, 0x31, 0x1A);
      usleep(100000);

      w_sh_reg_io(ip, 0x30, obj.getInt("pot12"));
      w_sh_reg_io(ip, 0x31, 0x1B);
      usleep(100000);

      w_sh_reg_io(ip, 0x30, obj.getInt("pot13"));
      w_sh_reg_io(ip, 0x31, 0x1C);
      usleep(100000);

      w_sh_reg_io(ip, 0x30, obj.getInt("pot14"));
      w_sh_reg_io(ip, 0x31, 0x1D);
      usleep(100000);

      w_sh_reg_io(ip, 0x30, obj.getInt("pot15"));
      w_sh_reg_io(ip, 0x31, 0x1E);

      w_sh_reg_io(ip, 0x30, obj.getInt("pot16"));
      w_sh_reg_io(ip, 0x31, 0x1F);
      usleep(100000);
    }
  }
  m_forced = false;
}

void ECLShaperControllerCallback::recover(const DBObject& obj)
throw(RCHandlerException)
{
  m_forced = true;
  boot(obj);
  load(obj);
}

/*
void ECLShaperControllerCallback::sh_boot(int sh_num) throw(RCHandlerException)
{
  std::string hostname = StringUtil::form("192.168.1.%d", sh_num);
  int err;
  if ((err = ::sh_boot(hostname.c_str(), 0, 0xA7000000)) > 0) {
    std::string msg;
    if (err == 1) msg = "Connection error";
    else if (err == 2) msg = "Failed to recieve reply";
    else if (err == 3) msg = "Boot failed";
    throw (RCHandlerException("Failed to boot Shaper : %s", msg.c_str()));
  }
}

void ECLShaperControllerCallback::sh_init_ecldsp(int sh_num, int adr) throw(RCHandlerException)
{
  std::string hostname = StringUtil::form("192.168.1.%d", sh_num);
  int err;
  if ((err = ::sh_init_ecldsp(hostname.c_str(), sh_num, adr)) > 0) {
    std::string msg;
    if (err == 1) msg = "Connection error";
    else if (err == 2) msg = "Failed to recieve reply";
    else if (err == 3) msg = "Init failed";
    throw (RCHandlerException("Failed to boot Shaper : %s", msg.c_str()));
  }
}
*/

void ECLShaperControllerCallback::w_sh_reg_io(const char* ip,
                                              int reg_num, int wdata) throw(RCHandlerException)
{
  char msg[100];
  memset(msg, 0, 100);
  int ret;
  if ((ret = sh_reg_io(ip, "w", 16, reg_num, wdata, NULL, msg)) != 0) {
    if (msg[0] != 0) {
      throw (RCHandlerException("error: %d >> %s", ret, msg));
    }
  }
}

void ECLShaperControllerCallback::w_col_reg_io(const char* ip,
                                               int reg_num, int wdata) throw(RCHandlerException)
{
  char msg[100];
  memset(msg, 0, 100);
  col_reg_io(ip, "w", reg_num, wdata, NULL, msg);
  if (msg[0] != 0) {
    throw (RCHandlerException("error on col_reg_io: %s", msg));
  }
}

std::string ECLShaperControllerCallback::col_status(int sh_num) throw(RCHandlerException)
{
  std::string hostname = StringUtil::form("192.168.1.%d", sh_num);
  const char* ret = NULL;
  if ((ret = ::col_status(hostname.c_str())) == NULL) {
    throw (RCHandlerException("Failed to read status"));
  }
  return std::string(ret);
}
