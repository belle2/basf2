#include "daq/slc/apps/storagerd/StoragerCallback.h"
#include "daq/slc/apps/storagerd/StoragerMonitor.h"
#include "daq/slc/apps/storagerd/storage_info_all.h"

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
  ConfigFile config("storage");
  std::string runtype = config.get("runtype");
  NSMMessage msg(getNode(), NSMCommand::DBGET, runtype);
  msg.setNParams(1);
  msg.setParam(0, NSMCommand::DBGET.getId());
  msg.setData(runtype);
  preload(msg);

  m_data = NSMData("STORAGE_DATA", "storage_info_all",
                   storage_info_all_revision);
  m_data.allocate(getCommunicator());
}

void StoragerCallback::term() throw()
{
  for (size_t i = 0; i < m_con.size(); i++) {
    m_con[i].abort();
    m_con[i].getInfo().unlink();
  }
}

bool StoragerCallback::boot() throw()
{
  if (m_thread.is_alive()) m_thread.cancel();

  ConfigObject& obj(getConfig().getObject());
  const size_t nproc = obj.getInt("nworkers");
  m_con = std::vector<ProcessController>();
  for (size_t i = 0; i < 3 + nproc; i++) {
    m_con.push_back(ProcessController(this));
  }
  m_con[0].init("storagein", sizeof(storage_info));
  m_con[1].init("storagerecord", sizeof(storage_info));
  m_con[2].init("storageout", sizeof(storage_info));
  for (size_t i = 3; i < m_con.size(); i++) {
    m_con[i].init(StringUtil::form("basf2_%d", i - 3),
                  sizeof(storage_info));
  }

  const std::string ibuf_name = obj.getText("ibuf_name");
  const std::string rbuf_name = obj.getText("rbuf_name");
  const std::string obuf_name = obj.getText("obuf_name");
  const std::string ibuf_size = obj.getValueText("ibuf_size");
  const std::string rbuf_size = obj.getValueText("rbuf_size");
  const std::string obuf_size = obj.getValueText("obuf_size");

  m_con[0].clearArguments();
  m_con[0].setExecutable("storagein");
  m_con[0].addArgument(ibuf_name);
  m_con[0].addArgument(ibuf_size);
  m_con[0].addArgument(obj.getText("storagein_host"));
  m_con[0].addArgument(obj.getValueText("storagein_port"));
  m_con[0].addArgument("storagein");
  m_con[0].addArgument("1");
  if (!m_con[0].load(20)) {
    std::string emsg = "storagein: Failed to connect to eb2rx";
    setReply(emsg);
    LogFile::error(emsg);
    return false;
  }
  LogFile::debug("Booted storagein");

  m_con[1].clearArguments();
  m_con[1].setExecutable("storagerecord");
  m_con[1].addArgument(rbuf_name);
  m_con[1].addArgument(rbuf_size);
  m_con[1].addArgument(obj.getText("storagerecord_dir"));
  m_con[1].addArgument(obuf_name);
  m_con[1].addArgument(obuf_size);
  m_con[1].addArgument("storagerecord");
  m_con[1].addArgument("2");
  if (!m_con[1].load(10)) {
    std::string emsg = "storageout: Not accepted connection from EXPRECO";
    setReply(emsg);
    LogFile::error(emsg);
    return false;
  }
  LogFile::debug("Booted storagerecord");

  m_con[2].clearArguments();
  m_con[2].setExecutable("storageout");
  m_con[2].addArgument(obuf_name);
  m_con[2].addArgument(obuf_size);
  m_con[2].addArgument(obj.getValueText("storageout_port"));
  m_con[2].addArgument("storageout");
  m_con[2].addArgument("3");
  if (!m_con[2].load(10)) {
    std::string emsg = "storageout: Not accepted connection from EXPRECO";
    LogFile::warning(emsg);
  }
  LogFile::debug("Booted storageout");

  m_thread = PThread(new StoragerMonitor(this));
  return true;
}

bool StoragerCallback::load() throw()
{
  ConfigObject& obj(getConfig().getObject());
  for (size_t i = 3; i < m_con.size(); i++) {
    m_con[i].clearArguments();
    m_con[i].setExecutable("basf2");
    m_con[i].addArgument(obj.getText("scriptpath"));
    m_con[i].addArgument(obj.getText("ibuf_name"));
    m_con[i].addArgument(obj.getValueText("ibuf_size"));
    m_con[i].addArgument(obj.getText("rbuf_name"));
    m_con[i].addArgument(obj.getValueText("rbuf_size"));
    m_con[i].addArgument(StringUtil::form("basf2_%d", i - 3));
    m_con[i].addArgument(StringUtil::form("%d", i + 1));
    m_con[i].addArgument("1");
    if (!m_con[i].load(10)) {
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
  for (size_t i = 0; i < m_con.size(); i++) {
    std::string name = m_con[i].getName();
    if (!m_con[i].start()) {
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
  for (size_t i = 0; i < m_con.size(); i++) {
    m_con[i].abort();
  }
  return true;
}

