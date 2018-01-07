#include "daq/slc/apps/storagerd/StoragerCallback.h"
#include "daq/slc/apps/storagerd/storage_status.h"

#include "daq/slc/psql/PostgreSQLInterface.h"
#include "daq/slc/nsm/NSMCommunicator.h"

#include "daq/slc/system/LogFile.h"
#include "daq/slc/system/Time.h"

#include "daq/slc/base/StringUtil.h"

#include <sys/statvfs.h>
#include <stdlib.h>
#include <arpa/inet.h>

using namespace Belle2;

StoragerCallback::StoragerCallback()
{
  setTimeout(1);
  m_eb_stat = NULL;
  m_errcount = 0;
  m_expno = -1;
  m_runno = -1;
}

StoragerCallback::~StoragerCallback() throw()
{
  term();
}

void StoragerCallback::initialize(const DBObject& obj) throw(RCHandlerException)
{
  allocData(getNode().getName(), "storage", storage_status_revision);
  configure(obj);
}

void StoragerCallback::configure(const DBObject& obj) throw(RCHandlerException)
{
  abort();
  term();
  try {
    setUseSet("input.buf.name", false);
    setUseSet("input.buf.size", false);
    setUseSet("output.buf.name", false);
    setUseSet("output.buf.size", false);
    setUseSet("record.buf.name", false);
    setUseSet("record.buf.size", false);
    setUseSet("record.dir", false);
    setUseSet("record.ndisks", false);
    setUseSet("record.dbtmp", false);
    const DBObject& record(obj("record"));
    const size_t nproc = record.getInt("nproc");
    m_con = std::vector<ProcessController>();
    for (size_t i = 0; i < 3 + nproc; i++) {
      m_con.push_back(ProcessController(this));
    }
    m_eb2rx = ProcessController(this);
    m_eb2rx.init("eb2rx", 1);
    m_con[0].init("storagein", 2);
    m_con[1].init("storagerecord", 3);
    m_con[2].init("storageout", 4);
    for (size_t i = 3; i < m_con.size(); i++) {
      m_con[i].init(StringUtil::form("basf2[%d]", i - 3), i + 2);
    }
  } catch (const std::out_of_range& e) {
    throw (RCHandlerException("Bad configuration : %s", e.what()));
  }
  const DBObjectList& sender(obj("eb2rx").getObjects("sender"));
  m_nsenders = 0;
  for (DBObjectList::const_iterator it = sender.begin();
       it != sender.end(); it++) {
    std::string vname = StringUtil::form("stat.in[%d].", m_nsenders);
    add(new NSMVHandlerInt(vname + "addr", true, false, 0));
    add(new NSMVHandlerInt(vname + "port", true, false, 0));
    add(new NSMVHandlerInt(vname + "connection", true, false, 0));
    add(new NSMVHandlerInt(vname + "byte", true, false, 0));
    add(new NSMVHandlerInt(vname + "event", true, false, 0));
    add(new NSMVHandlerFloat(vname + "total_byte", true, false, 0));
    add(new NSMVHandlerFloat(vname + "flowrate", true, false, 0));
    add(new NSMVHandlerFloat(vname + "nqueue", true, false, 0));
    m_nsenders++;
  }
  std::string vname = StringUtil::form("stat.out.");
  add(new NSMVHandlerInt(vname + "addr", true, false, 0));
  add(new NSMVHandlerInt(vname + "port", true, false, 0));
  add(new NSMVHandlerInt(vname + "connection", true, false, 0));
  add(new NSMVHandlerInt(vname + "byte", true, false, 0));
  add(new NSMVHandlerInt(vname + "event", true, false, 0));
  add(new NSMVHandlerFloat(vname + "total_byte", true, false, 0));
  add(new NSMVHandlerFloat(vname + "flowrate", true, false, 0));
}

void StoragerCallback::term() throw()
{
  for (size_t i = 0; i < m_con.size(); i++) {
    m_con[i].abort();
    m_con[i].getInfo().unlink();
    switch (i) {
      case 0:
        set("storagein.pid", 0);
        break;
      case 1:
        set("storagerecord.pid", 0);
        break;
      case 2:
        set("storageout.pid", 0);
        break;
      default:
        set(StringUtil::form("basf2[%d].pid", i - 3), 0);
        break;
    }
  }
  m_eb2rx.abort();
  m_eb2rx.getInfo().unlink();
  set("eb2rx.pid", 0);
}

void StoragerCallback::load(const DBObject& obj) throw(RCHandlerException)
{
  const DBObject& input(obj("input"));
  const DBObject& output(obj("output"));
  const DBObject& record(obj("record"));
  const DBObject& ibuf(input("buf"));
  const DBObject& obuf(output("buf"));
  const DBObject& rbuf(record("buf"));
  const DBObject& isocket(input("socket"));
  const DBObject& osocket(output("socket"));

  m_ibuf.open(ibuf.getText("name"), ibuf.getInt("size") * 1000000, true);
  m_rbuf.open(rbuf.getText("name"), rbuf.getInt("size") * 1000000, true);
  m_obuf.open(obuf.getText("name"), obuf.getInt("size") * 1000000, true);

  bool use_eb2 = false;
  if (!m_eb2rx.isAlive() && obj.hasObject("eb2rx") && obj("eb2rx").getBool("used")) {
    const DBObject& eb2rx(obj("eb2rx"));
    use_eb2 = true;
    m_eb2rx.clearArguments();
    m_eb2rx.setExecutable(eb2rx.getText("exe"));
    m_eb2rx.addArgument("-l");
    m_eb2rx.addArgument("%d", eb2rx.getInt("port"));
    const DBObjectList& sender(eb2rx.getObjects("sender"));
    for (DBObjectList::const_iterator it = sender.begin();
         it != sender.end(); it++) {
      if (it->getBool("used")) {
        m_eb2rx.addArgument("%s:%d", it->getText("host").c_str(), it->getInt("port"));
      }
    }
    std::string upname = std::string("/dev/shm/") + getNode().getName() + "_eb2rx_up";
    std::string downname = std::string("/dev/shm/") + getNode().getName() + "_eb2rx_down";
    if (m_eb_stat) delete m_eb_stat;
    LogFile::debug("creating eb_statistics(%s, %d, %s, %d)", upname.c_str(), m_nsenders, downname.c_str(), 1);
    m_eb_stat = new eb_statistics(upname.c_str(), m_nsenders, downname.c_str(), 1);
    m_eb2rx.addArgument("-u");
    m_eb2rx.addArgument(upname);
    m_eb2rx.addArgument("-d");
    m_eb2rx.addArgument(downname);
    m_eb2rx.load(0);
    set("eb2rx.pid", m_eb2rx.getProcess().get_id());
    LogFile::debug("Booted eb2rx");
    m_time = Time();
    try_wait();
  } else {
    LogFile::notice("eb2rx is off");
  }

  const std::string nodename = StringUtil::tolower(getNode().getName());
  if (!m_con[0].isAlive()) {
    m_con[0].clearArguments();
    m_con[0].setExecutable("storagein");
    m_con[0].addArgument(ibuf.getText("name"));
    m_con[0].addArgument("%d", ibuf.getInt("size"));
    m_con[0].addArgument(isocket.getText("host"));
    m_con[0].addArgument("%d", isocket.getInt("port"));
    m_con[0].addArgument(nodename + "_storagein");
    m_con[0].addArgument("2");
    if (!m_con[0].load(10)) {
      std::string emsg = "storagein: Failed to connect to eb2rx";
      throw (RCHandlerException(emsg));
    }
    set("storagein.pid", m_con[0].getProcess().get_id());
    LogFile::debug("Booted storagein");
    try_wait();
  }

  if (!m_con[1].isAlive()) {
    m_con[1].clearArguments();
    m_con[1].setExecutable("storagerecord");
    m_con[1].addArgument(rbuf.getText("name"));
    m_con[1].addArgument("%d", rbuf.getInt("size"));
    m_con[1].addArgument(record.getText("hostname"));
    m_con[1].addArgument(record.getText("runtype"));
    m_con[1].addArgument(record.getText("dir"));
    m_con[1].addArgument("%d", record.getInt("ndisks"));
    m_con[1].addArgument(record.getText("dbtmp"));
    m_con[1].addArgument(obuf.getText("name"));
    m_con[1].addArgument("%d", obuf.getInt("size"));
    m_con[1].addArgument(nodename + "_storagerecord");
    m_con[1].addArgument("3");
    if (!m_con[1].load(30)) {
      std::string emsg = "storagerecord: Failed to start";
      throw (RCHandlerException(emsg));
    }
    set("storagerecord.pid", m_con[1].getProcess().get_id());
    LogFile::debug("Booted storagerecord");
    try_wait();
  }

  if (!m_con[2].isAlive() && output.getBool("used")) {
    m_con[2].clearArguments();
    m_con[2].setExecutable("storageout");
    m_con[2].addArgument(obuf.getText("name"));
    m_con[2].addArgument("%d", obuf.getInt("size"));
    m_con[2].addArgument("%d", osocket.getInt("port"));
    m_con[2].addArgument(nodename + "_storageout");
    m_con[2].addArgument("4");
    if (!m_con[2].load(10)) {
      LogFile::warning("storageout: Not accepted connection from EXPRECO");
    }
    LogFile::debug("Booted storageout");
    set("storageout.pid", m_con[2].getProcess().get_id());
    try_wait();
  } else {
    LogFile::notice("storageout is off");
  }

  for (size_t i = 3; i < m_con.size(); i++) {
    if (!m_con[i].isAlive()) {
      m_con[i].clearArguments();
      m_con[i].setExecutable("basf2");
      m_con[i].addArgument("%s/%s", getenv("BELLE2_LOCAL_DIR"),
                           record.getText("script").c_str());
      m_con[i].addArgument(ibuf.getText("name"));
      m_con[i].addArgument("%d", ibuf.getInt("size"));
      m_con[i].addArgument(rbuf.getText("name"));
      m_con[i].addArgument("%d", rbuf.getInt("size"));
      m_con[i].addArgument("%s_basf2[%d]", nodename.c_str(), i - 3);
      m_con[i].addArgument("%d", i + 2);
      m_con[i].addArgument("%d", (int)use_eb2);
      m_con[i].addArgument("1");
      if (!m_con[i].load(0)) {
        std::string emsg = StringUtil::form("Failed to start %d-th basf2", i - 3);
        throw (RCHandlerException(emsg));
      }
      set(StringUtil::form("basf2[%d].pid", i - 3), m_con[i].getProcess().get_id());
      try_wait();
    }
  }
  for (size_t i = 3; i < m_con.size(); i++) {
    if (m_con[i].isAlive() && m_con[i].waitReady(10)) {
      LogFile::debug("Booted %d-th basf2", i - 3);
    } else {
      throw (RCHandlerException("Failed to boot %d-th basf2", i - 3));
    }
  }
  m_flow = std::vector<FlowMonitor>();
  for (size_t i = 0; i < m_con.size(); i++) {
    FlowMonitor flow;
    flow.open(&(m_con[i].getInfo()));
    m_flow.push_back(flow);
  }
}

void StoragerCallback::start(int expno, int runno) throw(RCHandlerException)
{
  m_expno = expno;
  m_runno = runno;
  try {
    storage_status* status = (storage_status*)m_data.get();
    if (status != NULL)
      status->stime = Time().getSecond();
  } catch (const std::exception& e) {
    LogFile::error(e.what());
  }
  for (size_t i = 0; i < m_con.size(); i++) {
    std::string name = m_con[i].getName();
    try {
      m_con[i].start(expno, runno);
    } catch (const std::exception& e) {

    }
  }
}

void StoragerCallback::stop() throw(RCHandlerException)
{
  for (size_t i = 0; i < m_con.size(); i++) {
    m_con[i].stop();
  }
  m_expno = -1;
  m_runno = -1;
}

void StoragerCallback::recover(const DBObject& obj) throw(RCHandlerException)
{
  abort();
  load(obj);
}

void StoragerCallback::abort() throw(RCHandlerException)
{
  for (size_t i = 0; i < m_con.size(); i++) {
    m_con[i].abort();
    switch (i) {
      case 0:
        set("storagein.pid", 0);
        break;
      case 1:
        set("storagerecord.pid", 0);
        break;
      case 2:
        set("storageout.pid", 0);
        break;
      default:
        set(StringUtil::form("basf2[%d].pid", i - 3), 0);
        break;
    }
  }
  m_eb2rx.abort();
  set("eb2rx.pid", 0);
  m_ibuf.unlink();
  m_rbuf.unlink();
  m_obuf.unlink();
  stop();
}

void StoragerCallback::monitor() throw(RCHandlerException)
{
  const RCState state(getNode().getState());
  if (state == RCState::RUNNING_S || state == RCState::READY_S ||
      state == RCState::PAUSED_S || state == RCState::LOADING_TS ||
      state == RCState::STARTING_TS) {
    if (!m_eb2rx.isAlive()) {
      setState(RCState::ERROR_ES);
      throw (RCHandlerException(m_eb2rx.getParName() + " : crashed"));
    }
    for (size_t i = 0; i < m_con.size(); i++) {
      if (i != 2 && !m_con[i].isAlive()) {
        setState(RCState::ERROR_ES);
        throw (RCHandlerException(m_con[i].getParName() + " : crashed"));
      }
    }
  }
  NSMData& data(getData());
  if (!data.isAvailable()) return;
  storage_status* info = (storage_status*)data.get();
  info->ctime = Time().getSecond();
  info->nnodes = m_con.size();
  double t1 = Time().get();
  double dt = t1 - m_t0;
  if (!(state == RCState::RUNNING_S || state == RCState::READY_S)) {
    memset(info, 0, sizeof(storage_status));
    std::string vname = StringUtil::form("stat.out.");
    set(vname + "event", 0);
    set(vname + "byte", (float)0);
    set(vname + "addr", 0);
    set(vname + "port", 0);
    set(vname + "connection", 0);
    //set(vname + "nevent", 0);
    //set(vname + "evtrate", 0);
    set(vname + "total_byte", 0);
    set(vname + "flowrate", 0);
    for (int i = 0; i < m_nsenders; i++) {
      std::string vname = StringUtil::form("stat.in[%d].", i);
      set(vname + "event", 0);
      set(vname + "byte", 0);
      set(vname + "addr", 0);
      set(vname + "port", 0);
      set(vname + "connection", 0);
      //set(vname + "nevent", 0);
      //set(vname + "evtrate", 0);
      set(vname + "total_byte", 0);
      set(vname + "flowrate", 0);
    }
  } else {
    if (m_eb_stat) {
      std::vector<IOInfo> io_v;
      IOInfo io;
      io.setLocalAddress(ntohl(m_eb_stat->down(0).addr));
      int port = m_eb_stat->down(0).port;//ntohl(m_eb_stat->down(0).port);
      io.setLocalPort(port);
      io_v.push_back(io);
      for (int i = 0; i < m_nsenders; i++) {
        IOInfo io;
        io.setLocalAddress(m_eb_stat->up(i).addr);
        int port = m_eb_stat->up(i).port;//ntohl(m_eb_stat->up(i).port);
        io.setLocalPort(port);
        io_v.push_back(io);
      }
      std::string vname = StringUtil::form("stat.out.");
      set(vname + "event", (int)m_eb_stat->down(0).event);
      set(vname + "byte", (float)(m_eb_stat->down(0).byte / 1024.));
      set(vname + "addr", (int)m_eb_stat->down(0).event);
      port = m_eb_stat->down(0).port;
      set(vname + "port", (int)port);
      set(vname + "connection", port > 0);
      unsigned long long total_byte = m_eb_stat->down(0).total_byte;
      double dbyte = total_byte - m_total_byte_out[0];
      double flowrate = dbyte / dt / 1024 / 1024; //MB
      set(vname + "total_byte", (float)total_byte);
      set(vname + "flowrate", (float)flowrate);
      m_total_byte_out[0] = total_byte;
      for (int i = 0; i < m_nsenders; i++) {
        std::string vname = StringUtil::form("stat.in[%d].", i);
        set(vname + "event", (int)m_eb_stat->up(i).event);
        set(vname + "byte", (float)(m_eb_stat->up(i).byte / 1024.));
        set(vname + "addr", (int)m_eb_stat->up(i).event);
        port = m_eb_stat->up(i).port;
        set(vname + "port", port);
        set(vname + "connection", port > 0);
        total_byte = m_eb_stat->up(i).total_byte;
        dbyte = total_byte - m_total_byte_in[0];
        flowrate = dbyte / dt / 1024 / 1024; //MB
        set(vname + "total_byte", (float)total_byte);
        set(vname + "flowrate", (float)flowrate);
        m_total_byte_in[i] = total_byte;
      }
      IOInfo::checkTCP(io_v);
      info->eb2out.event = m_eb_stat->down(0).event;
      info->eb2out.byte = m_eb_stat->down(0).byte / 1024.;
      info->eb2out.nqueue = io_v[0].getTXQueue() / 1024.;
      info->eb2out.connection = (io_v[0].getState() == 1);
      for (int i = 0; i < m_nsenders; i++) {
        info->eb2in[i].event = m_eb_stat->up(i).event;
        info->eb2in[i].byte = m_eb_stat->up(i).byte / 1024.;
        info->eb2in[i].nqueue = io_v[i + 1].getRXQueue() / 1024.;
        info->eb2in[i].connection = (io_v[i + 1].getState() == 1);
        std::string vname = StringUtil::form("stat.in[%d].", i);
        set(vname + "nqueue", (int)info->eb2in[i].nqueue);
        //set(vname + "connection", (int)info->eb2in[i].connection);
      }
    }
    m_t0 = t1;

    bool connected = false;
    bool writing = false;
    for (size_t i = 0; i < m_flow.size() && i < 8; i++) {
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
      if (i == 0) { // input
        info->eflag = rostatus.eflag;
        info->ctime = rostatus.ctime;
        info->expno = rostatus.expno;
        info->runno = rostatus.runno;
        info->subno = rostatus.subno;
        info->node[0].nqueue_in = rostatus.nqueue_in / 1024; // B -> kB
        if (m_ibuf.isOpened()) {
          SharedEventBuffer::Header* hd = m_ibuf.getHeader();
          info->node[0].nqueue_out = (hd->nword_in - hd->nword_out) * 4 / 1024 / 1024; // word -> MB
        } else {
          info->node[0].nqueue_out = 0;
        }
        connected = (info->node[0].connection_in > 0);
        if (state == RCState::RUNNING_S && info->node[0].nevent_in > 0 &&
            info->node[0].nqueue_out == 0 && info->node[0].evtrate_in == 0) {
          m_errcount++;
          if (m_errcount == 12) {
            log(LogFile::WARNING, "Data flow was stopped over 1 mins");
          }
        } else {
          m_errcount = 0;
        }
      } else if (i == 1) { // record
        if (m_rbuf.isOpened()) {
          SharedEventBuffer::Header* hd = m_rbuf.getHeader();
          info->node[1].nqueue_out = (hd->nword_in - hd->nword_out) * 4 / 1024 / 1024; // word -> MB
        } else {
          info->node[1].nqueue_out = 0;
        }
        info->nfiles = rostatus.reserved_i[0];
        info->diskid = rostatus.reserved_i[1];
        info->nbytes = rostatus.reserved_f[0];
        writing = (rostatus.flowrate_out > 0);
      }
    }
    if (connected) {
      if (writing) info->state = 2;
      else info->state = 1;
    } else {
      info->state = 0;
    }
  }
  struct statvfs statfs;
  const DBObject& record(getDBObject()("record"));
  if (record.hasValue("ndisks")) {
    std::string dir = record.getText("dir");
    info->ndisks = record.getInt("ndisks");
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
