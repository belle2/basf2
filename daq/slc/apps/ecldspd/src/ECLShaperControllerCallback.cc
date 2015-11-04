#include "daq/slc/apps/ecldspd/ECLShaperControllerCallback.h"

#include "daq/slc/apps/ecldspd/ECLShaperControllerHandler.h"

#include <daq/slc/system/LogFile.h>
#include <daq/slc/system/PThread.h>
#include <daq/slc/system/Process.h>

#include <daq/slc/nsm/NSMCommunicator.h>

#include <daq/slc/base/StringUtil.h>

#include <ecldaq/ecl_collector_lib.h>
extern "C" {
  const char* col_status(const char* ip_addr);
}

#include <vector>

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
}

void ECLShaperControllerCallback::boot(const DBObject& obj)
throw(RCHandlerException)
{
  if (obj.hasObject("cols")) {
    NSMNode node1("ECL01");
    if (NSMCommunicator::send(NSMMessage(node1, RCCommand::BOOT))) {
      try {
        wait(node1, RCCommand::OK, 10);
      } catch (const TimeoutException& e) {
        LogFile::warning("%s %s:%d", e.what(), __FILE__, __LINE__);
      } catch (const IOException& e) {
        LogFile::error("%s %s:%d", e.what(), __FILE__, __LINE__);
      }
    } else {
      LogFile::warning("%s is down.", node1.getName().c_str());
    }
    NSMNode node2("ECL02");
    if (NSMCommunicator::send(NSMMessage(node2, RCCommand::BOOT))) {
      try {
        wait(node2, RCCommand::OK, 10);
      } catch (const TimeoutException& e) {
        LogFile::warning("%s %s:%d", e.what(), __FILE__, __LINE__);
      } catch (const IOException& e) {
        LogFile::error("%s %s:%d", e.what(), __FILE__, __LINE__);
      }
    } else {
      LogFile::warning("%s is down.", node2.getName().c_str());
    }
    const DBObject& o_cols(obj("cols"));
    std::vector<PThread> ths;
    // loshf-all
    for (int col = o_cols.getInt("min");
         col <= o_cols.getInt("max"); col++) {
      ths.push_back(PThread(new Boot(col, obj), true, false));
    }
    for (std::vector<PThread>::iterator it = ths.begin();
         it != ths.end(); it++) {
      it->join();
    }
    ths = std::vector<PThread>();
    // loshc-all 1
    for (int col = o_cols.getInt("min");
         col <= o_cols.getInt("max"); col++) {
      ths.push_back(PThread(new Init(col, obj), true, false));
    }
    for (std::vector<PThread>::iterator it = ths.begin();
         it != ths.end(); it++) {
      it->join();
    }
  }
  printf("Boot sequence done!\n");
}

void ECLShaperControllerCallback::load(const DBObject& obj)
throw(RCHandlerException)
{
  if (!m_forced) return;
  if (obj.hasObject("cols")) {
    const DBObject& o_cols(obj("cols"));
    std::vector<Process> prs;
    // loshp-all
    for (int col = o_cols.getInt("min");
         col <= o_cols.getInt("max"); col++) {
      prs.push_back(Process(new Load(col, obj)));
    }
    for (std::vector<Process>::iterator it = prs.begin();
         it != prs.end(); it++) {
      it->wait();
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

std::string ECLShaperControllerCallback::col_status(int sh_num) throw(RCHandlerException)
{
  std::string hostname = StringUtil::form("192.168.1.%d", sh_num);
  const char* ret = NULL;
  if ((ret = ::col_status(hostname.c_str())) == NULL) {
    throw (RCHandlerException("Failed to read status"));
  }
  return std::string(ret);
}

void ECLShaperControllerCallback::Boot::run()
{
  const unsigned int mem_bin_addr = 0xA7000000;
  char ip[100];
  sprintf(ip, "192.168.1.%d", m_id);
  printf("booting ip %s\n", ip);
  sh_boot(ip, 16, mem_bin_addr);
}

void ECLShaperControllerCallback::Init::run()
{
  const unsigned int mem_ecldsp_addr = 0xA8000000;
  char ip[100];
  sprintf(ip, "192.168.1.%d", m_id);
  printf("initializing ip %s\n", ip);
  sh_init_ecldsp(ip, 16, mem_ecldsp_addr);
}

void ECLShaperControllerCallback::Load::run()
{
  const DBObject& obj(m_obj);
  char ip[100];
  sprintf(ip, "192.168.1.%d", m_id);
  printf("loading ip %s\n", ip);
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
  exit(0);
}

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
  if (col_reg_io(ip, "w", reg_num, wdata, NULL, msg) != 0) {
    if (msg[0] != 0) {
      throw (RCHandlerException("error on col_reg_io: %s", msg));
    }
  }
}

