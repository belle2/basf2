#include "daq/slc/apps/eclcollectord/ECLShaperControllerCallback.h"

#include <daq/slc/system/LogFile.h>

#include <daq/slc/base/StringUtil.h>

using namespace Belle2;

ECLShaperControllerCallback::ECLShaperControllerCallback(const NSMNode& node)
  : RCCallback(node, 5)
{
  //m_data = NSMData("STATUS_" + node.getName(), format, revision);
}

ECLShaperControllerCallback::~ECLShaperControllerCallback() throw()
{
}

void ECLShaperControllerCallback::init() throw()
{
  //m_data.allocate(getCommunicator());
  m_forced = true;
}

void ECLShaperControllerCallback::term() throw()
{
}

void ECLShaperControllerCallback::timeout() throw()
{
}

bool ECLShaperControllerCallback::load() throw()
{
  m_config = ECLShaperConfig();
  ConfigObjectList& shaper(getConfig().getObject().getObjects("shaper"));
  for (size_t i = 0; i < shaper.size(); i++) {
    ConfigObject& sh(shaper[i]);
    int sh_num = sh.getInt("sh_num");
    ConfigObjectList& registers(sh.getObjects("register"));
    ECLShaperRegisterList regs;
    for (size_t j = 0; j < registers.size(); j++) {
      ECLShaperRegister reg;
      reg.name = registers[i].getText("paramname");
      reg.adr = registers[i].getInt("address");
      regs.insert(ECLShaperRegisterList::value_type(reg.name, reg));
    }
    ConfigObjectList& parameters(sh.getObjects("parameter"));
    for (size_t j = 0; j < parameters.size(); j++) {
      ECLShaperRegisterList::iterator it =
        regs.find(parameters[j].getText("paramname"));
      if (it != regs.end()) {
        it->second.val = parameters[j].getInt("value");
      }
    }
    m_config.insert(ECLShaperConfig::value_type(sh_num, regs));
  }
  if (m_forced) {
    return recover();
  }
  return true;
}

bool ECLShaperControllerCallback::start() throw()
{
  NSMMessage& msg(getMessage());
  LogFile::debug("run # = %04d.%04d.%03d",
                 msg.getParam(0), msg.getParam(1),
                 msg.getParam(2));
  return true;
}

bool ECLShaperControllerCallback::stop() throw()
{
  return true;
}

bool ECLShaperControllerCallback::resume() throw()
{
  return true;
}

bool ECLShaperControllerCallback::pause() throw()
{
  return true;
}

bool ECLShaperControllerCallback::recover() throw()
{
  try {
    m_con.boot(m_config);
    m_con.init(m_config, 2);
  } catch (const IOException& e) {
    m_forced = true;
    getNode().setState(RCState::NOTREADY_S);
    return false;
  }
  getNode().setState(RCState::READY_S);
  m_forced = false;
  return false;
}

bool ECLShaperControllerCallback::abort() throw()
{
  getNode().setState(RCState::NOTREADY_S);
  return true;
}

