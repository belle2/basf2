#include "daq/slc/apps/storagerd/StoragerCallback.h"
#include "daq/slc/apps/storagerd/StoragerMonitor.h"

#include "daq/slc/system/LogFile.h"

#include "daq/slc/base/Debugger.h"
#include "daq/slc/base/StringUtil.h"
#include "daq/slc/base/ConfigFile.h"

using namespace Belle2;

StoragerCallback::StoragerCallback(NSMNode* node)
  : RCCallback(node)
{
  node->setData(new DataObject());
  for (int i = 0; i < 3; i++)
    _con[i].setCallback(this);
}

StoragerCallback::~StoragerCallback() throw()
{
}

void StoragerCallback::init() throw()
{
  LogFile::open("storage");
  _con[0].init("storagein");
  _con[1].init("basf2");
  _con[2].init("storageout");
  PThread(new StoragerMonitor(this));
}

void StoragerCallback::term() throw()
{
  for (int i = 0; i < 3; i++) {
    _con[i].abort();
    _con[i].getInfo().unlink();
  }
}

bool StoragerCallback::boot() throw()
{
  ConfigFile config;
  const std::string irbname = _node->getName() + "_IN";
  const std::string orbname = _node->getName() + "_OUT";
  _con[0].clearArguments();
  _con[0].setExecutable("storagein");
  _con[0].addArgument(irbname);
  _con[0].addArgument(config.get("DATA_STORAGE_FROM_HOST"));
  _con[0].addArgument(config.get("DATA_STORAGE_FROM_PORT"));
  _con[0].addArgument("storagein");
  _con[0].addArgument("1");
  if (!_con[0].load(20)) {
    //std::string emsg = "Failed to start storagein";
    std::string emsg = "storagein: Failed to connect to eb2rx";
    setReply(emsg);
    LogFile::error(emsg);
    return false;
  }
  LogFile::debug("Booted storagein");
  return true;
}

bool StoragerCallback::load() throw()
{
  ConfigFile config;
  const std::string irbname = _node->getName() + "_IN";
  const std::string orbname = _node->getName() + "_OUT";
  _con[1].clearArguments();
  _con[1].addArgument(config.get("BASF2_SCRIPT_PATH"));
  _con[1].addArgument(irbname);
  _con[1].addArgument(config.get("DATA_STORAGE_DIR"));
  _con[1].addArgument(orbname);
  _con[1].addArgument("basf2");
  _con[1].addArgument("1");
  _con[1].addArgument("1");
  if (!_con[1].load(10)) {
    std::string emsg = "Failed to start basf2";
    setReply(emsg);
    LogFile::error(emsg);
    return false;
  }
  LogFile::debug("Booted basf2");

  _con[2].clearArguments();
  _con[2].setExecutable("storageout");
  _con[2].addArgument(orbname);
  _con[2].addArgument(config.get("DATA_STORAGE_TO_HOST"));
  _con[2].addArgument(config.get("DATA_STORAGE_TO_PORT"));
  _con[2].addArgument("storageout");
  _con[2].addArgument("1");
  if (!_con[2].load(10)) {
    //std::string emsg = "Failed to start storageout";
    std::string emsg = "storageout: Not accepted connection from EXPRECO";
    //setReply(emsg);
    //LogFile::error(emsg);
    //return false;
    LogFile::warning(emsg);
  }
  LogFile::debug("Booted storageout");
  return true;
}

bool StoragerCallback::start() throw()
{
  std::string name[3] = {"storagein", "basf2", "storageout"};
  for (int i = 0; i < 3; i++) {
    if (!_con[i].start()) {
      if (i < 2) {
        std::string emsg = name[i] + " is not started";
        setReply(emsg);
        LogFile::error(emsg);
        return false;
      } else {
        std::string emsg = "storageout: Not accepted connection from EXPRECO yet";
        LogFile::warning(emsg);
      }
    }
    LogFile::debug(name[i] + " started");
  }
  return true;
}

bool StoragerCallback::stop() throw()
{
  return true;
}

bool StoragerCallback::resume() throw()
{
  return true;
}

bool StoragerCallback::pause() throw()
{
  return true;
}

bool StoragerCallback::recover() throw()
{
  return abort() && boot() && load();
}

bool StoragerCallback::abort() throw()
{
  for (int i = 0; i < 3; i++)
    _con[i].abort();
  return true;
}

