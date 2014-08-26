#include "daq/slc/apps/storagerd/StoragerCallback.h"
#include "daq/slc/apps/storagerd/storage_status.h"

#include "daq/slc/system/LogFile.h"
#include "daq/slc/system/Time.h"

#include "daq/slc/base/StringUtil.h"

#include <sys/statvfs.h>
#include <stdlib.h>

using namespace Belle2;

StoragerCallback::StoragerCallback(const NSMNode& node)
  : RCCallback(node)
{
  setTimeout(1);
}

StoragerCallback::~StoragerCallback() throw()
{
}

void StoragerCallback::init() throw()
{
  LogFile::open("storage");
  m_data = NSMData("STORAGE_STATUS", "storage_status",
                   storage_status_revision);
  m_data.allocate(getCommunicator());
}

void StoragerCallback::term() throw()
{
  for (size_t i = 0; i < m_con.size(); i++) {
    m_con[i].abort();
    m_con[i].getInfo().unlink();
  }
}

bool StoragerCallback::load() throw()
{
  system("killall storagein");
  system("killall storagerecord");
  system("killall storageout");
  system("killall basf2");

  m_file.read("storage");
  const size_t nproc = m_file.getInt("record.nproc");
  m_con = std::vector<ProcessController>();
  for (size_t i = 0; i < 3 + nproc; i++) {
    m_con.push_back(ProcessController(this));
  }
  m_con[0].init("storagein", 1);
  m_con[1].init("storagerecord", 2);
  m_con[2].init("storageout", 3);
  for (size_t i = 3; i < m_con.size(); i++) {
    m_con[i].init(StringUtil::form("basf2_%d", i - 3), i);
  }

  const std::string ibuf_name = m_file.get("input.buf.name");
  const std::string rbuf_name = m_file.get("record.buf.name");
  const std::string obuf_name = m_file.get("output.buf.name");
  const std::string ibuf_size = m_file.get("input.buf.size");
  const std::string rbuf_size = m_file.get("record.buf.size");
  const std::string obuf_size = m_file.get("output.buf.size");

  m_con[0].clearArguments();
  m_con[0].setExecutable("storagein");
  m_con[0].addArgument(ibuf_name);
  m_con[0].addArgument(ibuf_size);
  m_con[0].addArgument(m_file.get("input.socket.host"));
  m_con[0].addArgument(m_file.get("input.socket.port"));
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
  m_con[1].addArgument(m_file.get("record.dir"));
  m_con[1].addArgument(m_file.get("record.ndisks"));
  m_con[1].addArgument(m_file.get("record.file.diskid"));
  m_con[1].addArgument(m_file.get("record.file.nfiles"));
  m_con[1].addArgument(obuf_name);
  m_con[1].addArgument(obuf_size);
  m_con[1].addArgument("storagerecord");
  m_con[1].addArgument("2");
  LogFile::debug("debug");
  if (!m_con[1].load(10)) {
    std::string emsg = "storageout: Failed to start";
    setReply(emsg);
    LogFile::error(emsg);
    return false;
  }
  LogFile::debug("Booted storagerecord");

  /*
  m_con[2].clearArguments();
  m_con[2].setExecutable("storageout");
  m_con[2].addArgument(output.buf.name);
  m_con[2].addArgument(output.buf.size);
  m_con[2].addArgument(m_file.get("output.socketport"));
  m_con[2].addArgument("storageout");
  m_con[2].addArgument("3");
  if (!m_con[2].load(10)) {
    std::string emsg = "storageout: Not accepted connection from EXPRECO";
    LogFile::warning(emsg);
  }
  LogFile::debug("Booted storageout");
  */

  for (size_t i = 3; i < m_con.size(); i++) {
    m_con[i].clearArguments();
    m_con[i].setExecutable("basf2");
    m_con[i].addArgument(StringUtil::form("%s/%s", getenv("BELLE2_LOCAL_DIR"),
                                          m_file.get("record.script").c_str()));
    m_con[i].addArgument(ibuf_name);
    m_con[i].addArgument(ibuf_size);
    m_con[i].addArgument(rbuf_name);
    m_con[i].addArgument(rbuf_size);
    m_con[i].addArgument(StringUtil::form("basf2_%d", i - 3));
    m_con[i].addArgument(StringUtil::form("%d", i + 1));
    m_con[i].addArgument("1");
    if (!m_con[i].load(10)) {
      std::string emsg = StringUtil::form("Failed to start %d-th basf2", i - 3);
      setReply(emsg);
      LogFile::error(emsg);
      return false;
    }
    LogFile::debug("Booted %d-th basf2", i - 3);
  }

  m_flow = std::vector<FlowMonitor>();
  for (size_t i = 0; i < m_con.size(); i++) {
    FlowMonitor flow;
    flow.open(&(m_con[i].getInfo()));
    m_flow.push_back(flow);
  }
  m_ibuf.open(ibuf_name, atoi(ibuf_size.c_str()) * 1000000);
  m_rbuf.open(rbuf_name, atoi(rbuf_size.c_str()) * 1000000);

  return true;
}

bool StoragerCallback::start() throw()
{
  storage_status* status = (storage_status*)m_data.get();
  status->stime = Time().getSecond();
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
  abort();
  sleep(3);
  return load();
}

bool StoragerCallback::abort() throw()
{
  for (size_t i = 0; i < m_con.size(); i++) {
    m_con[i].abort();
  }
  return true;
}

void StoragerCallback::timeout() throw()
{
  storage_status* info = (storage_status*)m_data.get();
  info->ctime = Time().getSecond();
  info->nnodes = m_flow.size();
  for (size_t i = 0; i < m_flow.size() && i < 14; i++) {
    ronode_status& rostatus(m_flow[i].monitor());
    info->node[i].connection_in = rostatus.connection_in;
    info->node[i].nevent_in = rostatus.nevent_in;
    info->node[i].evtrate_in = rostatus.evtrate_in;
    info->node[i].evtsize_in = rostatus.evtsize_in;
    info->node[i].flowrate_in = rostatus.flowrate_in;
    info->node[i].connection_out = rostatus.connection_out;
    info->node[i].nevent_out = rostatus.nevent_out;
    info->node[i].evtrate_out = rostatus.evtrate_out;
    info->node[i].evtsize_out = rostatus.evtsize_out;
    info->node[i].flowrate_out = rostatus.flowrate_out;
    if (i == 0) { //IN
      info->eflag = rostatus.eflag;
      info->ctime = rostatus.ctime;
      info->expno = rostatus.expno;
      info->runno = rostatus.runno;
      info->subno = rostatus.subno;
      info->node[0].nqueue_in = rostatus.nqueue_in;
      SharedEventBuffer::Header* hd = m_ibuf.getHeader();
      info->node[0].nqueue_out = hd->nword_in - hd->nword_out;
    } else if (i == 1) {
      SharedEventBuffer::Header* hd = m_rbuf.getHeader();
      info->node[1].nqueue_out = hd->nword_in - hd->nword_out;
      info->nfiles = rostatus.reserved_i[0];
      info->diskid = rostatus.reserved_i[1];
      info->nbytes = rostatus.reserved_f[0];
    }
  }
  struct statvfs statfs;
  if (m_file.hasKey("record.ndisks")) {
    std::string dir = m_file.get("record.dir");
    info->ndisks = m_file.getInt("record.ndisks");
    for (unsigned int i = 0; i < info->ndisks; i++) {
      std::string path = StringUtil::form("%s%02d", dir.c_str(), i + 1);
      statvfs(path.c_str(), &statfs);
      info->disk[i].size = (float)statfs.f_frsize * statfs.f_blocks / 1024 / 1024 / 1024;
      info->disk[i].available = 100 - ((float)statfs.f_bfree / statfs.f_blocks * 100);
    }
  }
  double loads[3];
  if (getloadavg(loads, 3) > 0) {
    info->loadavg = (float)loads[0];
  } else {
    info->loadavg = -1;
  }
}
