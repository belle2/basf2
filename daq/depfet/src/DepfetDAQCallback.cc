#include "daq/depfet/DepfetDAQCallback.h"

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

DepfetDAQCallback::DepfetDAQCallback()
{
  setTimeout(1);
  m_errcount = 0;
}

DepfetDAQCallback::~DepfetDAQCallback() throw()
{
  term();
}

void DepfetDAQCallback::initialize(const DBObject& obj) throw(RCHandlerException)
{
  allocData(getNode().getName(), "storage",
            storage_status_revision);
  m_con = std::vector<ProcessController>();
  configure(obj);
}

void DepfetDAQCallback::configure(const DBObject& obj) throw(RCHandlerException)
{
  /*
  abort();
  term();
  */
  try {
    const DBObjectList& o_buf(obj.getObjects("buf"));
    m_buf.resize(o_buf.size());
    const DBObjectList& o_process(obj.getObjects("process"));
    const size_t nproc = o_process.size();
    m_con = std::vector<ProcessController>();
    for (size_t i = 0; i < nproc; i++) {
      m_con.push_back(ProcessController(this));
    }
    for (size_t i = 0; i < m_con.size(); i++) {
      const DBObject& process(o_process[i]);
      m_con[i].init(process.getText("name"), i);
    }
  } catch (const std::out_of_range& e) {
    throw (RCHandlerException("Bad configuration : %s", e.what()));
  }
}

void DepfetDAQCallback::term() throw()
{
  for (size_t i = 0; i < m_con.size(); i++) {
    m_con[i].abort();
  }
}

void DepfetDAQCallback::load(const DBObject& obj) throw(RCHandlerException)
{
  const DBObjectList& o_buf(obj.getObjects("buf"));
  for (size_t i = 0; i < o_buf.size(); i++) {
    const DBObject& buf(o_buf[i]);
    LogFile::info("open buf[%d] : %s (%d)", i, buf.getText("name").c_str(), buf.getInt("size"));
    m_buf[i].open(buf.getText("name"), buf.getInt("size") * 1000000, true);
  }

  const DBObjectList& o_process(obj.getObjects("process"));
  const std::string nodename = StringUtil::tolower(getNode().getName());
  for (size_t i = 0; i < o_process.size(); i++) {
    const DBObject& process(o_process[i]);
    std::string name = process.getText("name");
    std::string exe = process.getText("exe");
    if (m_con[i].isAlive()) {
      m_con[i].abort();
    }
    m_con[i].clearArguments();
    m_con[i].setExecutable(exe);
    const DBObjectList& o_arg(process.getObjects("arg"));
    for (size_t j = 0; j < o_arg.size(); j++) {
      const DBObject& arg(o_arg[j]);
      m_con[i].addArgument(arg.getValueText("val"));
    }
    if (exe != "basf2") {
      m_con[i].addArgument("-m");
    }
    //m_con[i].addArgument("%s_%s:%d", nodename.c_str(), name.c_str(), i);
    m_con[i].addArgument("%s_%s", nodename.c_str(), name.c_str());
    //m_con[i].addArgument("%s", name.c_str());
    if (!m_con[i].load(-1)) {
      std::string emsg = name + ": Failed to start";
      LogFile::error(emsg);
      throw (RCHandlerException(emsg));
    }
    set(name + ".pid", m_con[i].getProcess().get_id());
    LogFile::debug("Booted " + name);
    try_wait();
  }

  m_flow = std::vector<FlowMonitor>();
  for (size_t i = 0; i < o_process.size(); i++) {
    FlowMonitor flow;
    flow.open(&(m_con[i].getInfo()));
    m_flow.push_back(flow);
  }
}

void DepfetDAQCallback::start(int expno, int runno) throw(RCHandlerException)
{
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

void DepfetDAQCallback::stop() throw(RCHandlerException)
{
  for (size_t i = 0; i < m_con.size(); i++) {
    m_con[i].stop();
  }
}

void DepfetDAQCallback::recover(const DBObject& obj) throw(RCHandlerException)
{
  abort();
  load(obj);
}

void DepfetDAQCallback::abort() throw(RCHandlerException)
{
  for (size_t i = 0; i < m_con.size(); i++) {
    m_con[i].abort();
  }
  const DBObjectList& o_buf(getDBObject().getObjects("buf"));
  for (size_t i = 0; i < o_buf.size(); i++) {
    m_buf[i].unlink();
  }
}

void DepfetDAQCallback::monitor() throw(RCHandlerException)
{
  const RCState state(getNode().getState());
  if (state == RCState::RUNNING_S || state == RCState::READY_S ||
      state == RCState::PAUSED_S || state == RCState::LOADING_TS ||
      state == RCState::STARTING_TS) {
    for (size_t i = 0; i < m_con.size(); i++) {
      if (!m_con[i].isAlive()) {
        setState(RCState::ERROR_ES);
        throw (RCHandlerException(m_con[i].getParName() + " : crashed"));
      }
    }
  }
  NSMData& data(getData());
  if (!data.isAvailable()) return;
  storage_status* info = (storage_status*)data.get();
  if (info == NULL) return;
  info->ctime = Time().getSecond();
  info->nnodes = m_con.size();
  if (!(state == RCState::RUNNING_S || state == RCState::READY_S)) {
    memset(info, 0, sizeof(storage_status));
  } else {
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
    }
  }
}
