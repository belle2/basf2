#include "daq/slc/apps/storagerd/StoragerCallback.h"
#include "daq/slc/apps/storagerd/StoragerMonitor.h"

#include "daq/storage/storage_info.h"

#include "daq/slc/system/LogFile.h"

#include "daq/slc/base/StringUtil.h"
#include "daq/slc/base/ConfigFile.h"

using namespace Belle2;

StoragerCallback::StoragerCallback(const NSMNode& node)
  : RCCallback(node)
{
}

StoragerCallback::~StoragerCallback() throw()
{
}

void StoragerCallback::init() throw()
{
  LogFile::open("storage");
  ConfigFile config;
  size_t nproc = config.getInt("NUM_OF_BASF2_PROCESS");
  _con.resize(3 + nproc);
  for (size_t i = 0; i < _con.size(); i++) {
    _con[i].setCallback(this);
  }
  _con[0].init("storagein", sizeof(storage_info) / sizeof(int));
  _con[1].init("storagerecord", sizeof(storage_info) / sizeof(int));
  _con[2].init("storageout", sizeof(storage_info) / sizeof(int));
  for (size_t i = 3; i < _con.size(); i++) {
    _con[i].init(StringUtil::form("basf2_%d", i - 3),
                 sizeof(storage_info) / sizeof(int));
  }
  _data = NSMData("STORAGE_DATA", "storage_info_all", 1);
  _data.allocate(getCommunicator());
  PThread(new StoragerMonitor(this));
}

void StoragerCallback::term() throw()
{
  for (size_t i = 0; i < _con.size(); i++) {
    _con[i].abort();
    _con[i].getInfo().unlink();
  }
}

bool StoragerCallback::boot() throw()
{
  ConfigFile config;
  const std::string ibuf_name = getNode().getName() + "_IN";
  const std::string rbuf_name = getNode().getName() + "_RECORD";
  const std::string obuf_name = getNode().getName() + "_OUT";
  _con[0].clearArguments();
  _con[0].setExecutable("storagein");
  _con[0].addArgument(ibuf_name);
  _con[0].addArgument(config.get("DATA_STORAGE_FROM_HOST"));
  _con[0].addArgument(config.get("DATA_STORAGE_FROM_PORT"));
  _con[0].addArgument("storagein");
  _con[0].addArgument("1");
  if (!_con[0].load(20)) {
    std::string emsg = "storagein: Failed to connect to eb2rx";
    setReply(emsg);
    LogFile::error(emsg);
    return false;
  }
  LogFile::debug("Booted storagein");

  _con[1].clearArguments();
  _con[1].setExecutable("storagerecord");
  _con[1].addArgument(rbuf_name);
  _con[1].addArgument(config.get("DATA_STORAGE_DIR"));
  _con[1].addArgument(obuf_name);
  _con[1].addArgument("storagerecord");
  _con[1].addArgument("2");
  if (!_con[1].load(10)) {
    std::string emsg = "storageout: Not accepted connection from EXPRECO";
    setReply(emsg);
    LogFile::error(emsg);
    return false;
  }
  LogFile::debug("Booted storagerecord");

  _con[2].clearArguments();
  _con[2].setExecutable("storageout");
  _con[2].addArgument(obuf_name);
  _con[2].addArgument(config.get("DATA_STORAGE_TO_HOST"));
  _con[2].addArgument(config.get("DATA_STORAGE_TO_PORT"));
  _con[2].addArgument("storageout");
  _con[2].addArgument("3");
  if (!_con[2].load(10)) {
    std::string emsg = "storageout: Not accepted connection from EXPRECO";
    LogFile::warning(emsg);
  }
  LogFile::debug("Booted storageout");

  return true;
}

bool StoragerCallback::load() throw()
{
  ConfigFile config;
  const std::string ibuf_name = getNode().getName() + "_IN";
  const std::string rbuf_name = getNode().getName() + "_RECORD";
  for (size_t i = 3; i < _con.size(); i++) {
    _con[i].clearArguments();
    _con[i].addArgument(config.get("BASF2_SCRIPT_PATH"));
    _con[i].addArgument(ibuf_name);
    _con[i].addArgument(rbuf_name);
    _con[i].addArgument(StringUtil::form("basf2_%d", i - 3));
    _con[i].addArgument(StringUtil::form("%d", i + 1));
    _con[i].addArgument("1");
    if (!_con[i].load(10)) {
      std::string emsg = StringUtil::form("Failed to start %d-th basf2", i);
      setReply(emsg);
      LogFile::error(emsg);
      return false;
    }
    LogFile::debug("Booted %d-th basf2", i);
  }
  return true;
}

bool StoragerCallback::start() throw()
{
  for (size_t i = 0; i < _con.size(); i++) {
    std::string name = _con[i].getName();
    if (!_con[i].start()) {
      if (i != 2) {
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
  for (size_t i = 0; i < _con.size(); i++) {
    _con[i].abort();
  }
  return true;
}

