#include "daq/slc/apps/ecldspd/ECLShaperControllerCallback.h"

#include <daq/slc/system/LogFile.h>

#include <daq/slc/base/StringUtil.h>

using namespace Belle2;

void ECLShaperControllerCallback::load(const DBObject& obj) throw(RCHandlerException)
{
  m_config = ECLShaperConfig();
  if (obj.hasObject("shaper")) {
    const DBObjectList& c_shrs(obj.getObjects("shaper"));
    for (size_t i = 0; i < c_shrs.size(); i++) {
      const DBObject& c_shr(c_shrs[i]);
      int sh_num = c_shr.getInt("sh_num");
      const DBObjectList& c_regs(c_shr.getObjects("register"));
      ECLShaperRegisterList regs;
      for (size_t j = 0; j < c_regs.size(); j++) {
        ECLShaperRegister reg;
        reg.name = c_regs[j].getText("paramname");
        reg.adr = c_regs[j].getInt("address");
        regs.insert(ECLShaperRegisterList::value_type(reg.name, reg));
      }
      const DBObjectList& c_pars(c_shr.getObjects("parameter"));
      for (size_t j = 0; j < c_pars.size(); j++) {
        ECLShaperRegisterList::iterator it =
          regs.find(c_pars[j].getText("paramname"));
        if (it != regs.end()) {
          it->second.val = c_pars[j].getInt("value");
        }
      }
      m_config.insert(ECLShaperConfig::value_type(sh_num, regs));
    }
    if (m_forced) {
      recover(obj);
    }
  }
}

void ECLShaperControllerCallback::recover(const DBObject&) throw(RCHandlerException)
{
  try {
    m_con.boot(m_config);
    m_con.init(m_config, 2);
  } catch (const IOException& e) {
    m_forced = true;
    throw (RCHandlerException("Failed to revocer : %s", e.what()));
  }
  m_forced = false;
}

void ECLShaperControllerCallback::abort() throw(RCHandlerException)
{
  getNode().setState(RCState::NOTREADY_S);
}

