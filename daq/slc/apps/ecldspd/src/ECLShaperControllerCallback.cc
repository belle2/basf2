#include "daq/slc/apps/ecldspd/ECLShaperControllerCallback.h"

#include "daq/slc/apps/ecldspd/ECLShaperControllerHandler.h"

#include <daq/slc/system/LogFile.h>
#include <daq/slc/base/StringUtil.h>

extern "C" {
#include <ecl/ecl_collector_lib.h>
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
  const DBObjectList& o_shs(obj.getObjects("sh"));
  // loshf-all
  for (DBObjectList::const_iterator i_sh = o_shs.begin();
       i_sh != o_shs.end(); i_sh++) {
    int sh_num = i_sh->getInt("num");
    std::string vname = StringUtil::form("col[%d].status", sh_num);
    add(new NSMVHandlerColStatus(*this, vname, sh_num));
  }
}

void ECLShaperControllerCallback::load(const DBObject& obj)
throw(RCHandlerException)
{
  if (!m_forced) return;
  if (obj.hasObject("sh")) {
    const DBObjectList& o_shs(obj.getObjects("sh"));
    // loshf-all
    for (DBObjectList::const_iterator i_sh = o_shs.begin();
         i_sh != o_shs.end(); i_sh++) {
      sh_boot(i_sh->getInt("num"));
    }
    // loshc-all 1
    for (DBObjectList::const_iterator i_sh = o_shs.begin();
         i_sh != o_shs.end(); i_sh++) {
      sh_init_ecldsp(i_sh->getInt("num"), 0xA8000000);
    }
    // loshp-all
    const DBObjectList& o_pars(obj.getObjects("par"));
    const DBObjectList& o_pots(obj.getObjects("pot"));
    for (DBObjectList::const_iterator i_sh = o_shs.begin();
         i_sh != o_shs.end(); i_sh++) {
      int sh_num = i_sh->getInt("num");
      std::string hostname = StringUtil::form("192.168.1.%d", sh_num);
      for (DBObjectList::const_iterator i_par = o_pars.begin();
           i_par != o_pars.end(); i_par++) {
        int adr = i_par->getInt("adr");
        int val = i_par->getInt("val");
        sh_reg_io_write(hostname, sh_num, adr, val);
      }
      col_reg_io_write(hostname, 0xC000, 0x1);
      for (size_t i_pot = 0; i_pot < o_pots.size(); i_pot++) {
        sh_reg_io_write(hostname, sh_num, 0x30, o_pots[i_pot].getInt("val"));
        sh_reg_io_write(hostname, sh_num, 0x31, 0x10 + i_pot);
      }
    }
  }
  m_forced = false;
}

void ECLShaperControllerCallback::recover(const DBObject& obj)
throw(RCHandlerException)
{
  m_forced = true;
  load(obj);
}

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

void ECLShaperControllerCallback::sh_init_ecldsp(int sh_num, int adr) throw(IOException)
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


void ECLShaperControllerCallback::sh_reg_io_write(const std::string& hostname,
                                                  int sh_num, int adr, int val)
throw(RCHandlerException)
{
  static char msg[255];

  if (::sh_reg_io(hostname.c_str(), "w", sh_num, adr, val, NULL, msg) > 0) {
    throw (RCHandlerException("Failed to write to Shaper (adr:%x << val:%x) : %s",
                              adr, val, msg));
  }
}

void ECLShaperControllerCallback::col_reg_io_write(const std::string& hostname,
                                                   int adr, int val)
throw(RCHandlerException)
{
  static char msg[255];
  if (::col_reg_io(hostname.c_str(), "w", 0xC000, 0x1, NULL, msg) > 0) {
    throw (RCHandlerException("Failed to write to Shaper : %s", msg));
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
