#include "daq/slc/apps/storagerd/StoragerCallback.h"

#include "daq/slc/system/LogFile.h"

#include "daq/slc/base/Debugger.h"
#include "daq/slc/base/StringUtil.h"
#include "daq/slc/base/ConfigFile.h"

using namespace Belle2;

StoragerCallback::StoragerCallback(NSMNode* node, const std::string& dir)
  : RCCallback(node), _dir(dir)
{
  node->setData(new DataObject());
  node->setState(State::INITIAL_S);
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
}

bool StoragerCallback::boot() throw()
{
  Belle2::debug("BOOT");
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
  _con[0].load(10);

  return true;
}

bool StoragerCallback::load() throw()
{
  Belle2::debug("LOAD");
  ConfigFile config;
  const std::string irbname = _node->getName() + "_IN";
  const std::string orbname = _node->getName() + "_OUT";
  _con[1].clearArguments();
  _con[1].addArgument(config.get("BASF2_SCRIPT_DIR")
                      + "/DataStorager.py");
  _con[1].addArgument(irbname);
  _con[1].addArgument(config.get("DATA_STORAGE_DIR"));
  _con[1].addArgument(orbname);
  _con[1].addArgument("basf2");
  _con[1].addArgument("1");
  _con[1].addArgument("1");
  _con[1].load(10);

  _con[2].clearArguments();
  _con[2].setExecutable("storageout");
  _con[2].addArgument(orbname);
  _con[2].addArgument(config.get("DATA_STORAGE_TO_HOST"));
  _con[2].addArgument(config.get("DATA_STORAGE_TO_PORT"));
  _con[2].addArgument("storageout");
  _con[2].addArgument("1");
  _con[2].load(10);
  return true;
}

bool StoragerCallback::start() throw()
{
  Belle2::debug("START");
  for (int i = 0; i < 3; i++)
    _con[i].start();

  return true;
}

bool StoragerCallback::stop() throw()
{
  Belle2::debug("STOP");
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
  Belle2::debug("RECOVER");
  return abort() && boot() && load();
}

bool StoragerCallback::abort() throw()
{
  Belle2::debug("ABORT");
  for (int i = 0; i < 3; i++)
    _con[i].abort();
  return true;
}

