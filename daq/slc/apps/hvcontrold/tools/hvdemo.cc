#include "daq/slc/apps/hvcontrold/HVMasterCallback.h"

#include <daq/slc/hvcontrol/HVCallback.h>

#include <daq/slc/nsm/NSMNodeDaemon.h>

#include <daq/slc/system/Daemon.h>

#include <daq/slc/base/ConfigFile.h>
#include <daq/slc/base/StringUtil.h>

namespace Belle2 {

  class HVDemoCallback : public HVCallback {
  public:
    HVDemoCallback() throw() {}
    virtual ~HVDemoCallback() throw() {}

  public:
    virtual void turnon() throw(HVHandlerException)
    {
      LogFile::info("turnOn");
      getNode().setState(m_state_demand);
      set("state", getNode().getState().getLabel());
      reply(NSMMessage(NSMCommand::OK, getNode().getState().getLabel()));
    }
    virtual void turnoff() throw(HVHandlerException)
    {
      LogFile::info("turnOff");
      getNode().setState(m_state_demand);
      set("state", getNode().getState().getLabel());
      reply(NSMMessage(NSMCommand::OK, getNode().getState().getLabel()));
    }
    virtual void standby() throw(HVHandlerException)
    {
      LogFile::info("standby");
      getNode().setState(m_state_demand);
      set("state", getNode().getState().getLabel());
      reply(NSMMessage(NSMCommand::OK, getNode().getState().getLabel()));
    }
    virtual void shoulder() throw(HVHandlerException)
    {
      LogFile::info("shoulder");
      getNode().setState(m_state_demand);
      set("state", getNode().getState().getLabel());
      reply(NSMMessage(NSMCommand::OK, getNode().getState().getLabel()));
    }
    virtual void peak() throw(HVHandlerException)
    {
      LogFile::info("peak");
      getNode().setState(m_state_demand);
      set("state", getNode().getState().getLabel());
      reply(NSMMessage(NSMCommand::OK, getNode().getState().getLabel()));
    }
    virtual void recover() throw(HVHandlerException)
    {
      LogFile::info("recover");
      getNode().setState(HVState::OFF_S);
      set("state", getNode().getState().getLabel());
      reply(NSMMessage(NSMCommand::OK, getNode().getState().getLabel()));
    }
    virtual void configure(const HVConfig&) throw(HVHandlerException)
    {
      LogFile::info("configure");
      getNode().setState(HVState::OFF_S);
      set("state", getNode().getState().getLabel());
      reply(NSMMessage(NSMCommand::OK, getNode().getState().getLabel()));
    }
    virtual void init(NSMCommunicator&) throw()
    {
      add(new NSMVHandlerText("configs", true, false, m_confignames));
      getNode().setState(HVState::OFF_S);
      add(new NSMVHandlerText("state", true, false, getNode().getState().getLabel()));
    }

  protected:
    virtual void dbload(const std::string& confignames) throw(IOException) {}
    virtual void load(const HVConfig& config,
                      bool alloff, bool loadpars) throw(HVHandlerException) {}
  };

}

using namespace Belle2;

int main(int argc, char** argv)
{
  if (Daemon::start(argv[1], argc, argv, 1, "<config>")) {
    HVDemoCallback* callback = new HVDemoCallback();
    ConfigFile config("slowcontrol");
    callback->getNode().setName(argv[1]);
    NSMNodeDaemon(callback, config.get("nsm.host"),
                  config.getInt("nsm.port")).run();
  }
  return 0;
}

