#include "daq/slc/apps/thscanerd/ThscanerCallback.h"

#include "daq/slc/apps/thscanerd/sa02board.h"

#include <daq/slc/runcontrol/RCNode.h>
#include <daq/slc/runcontrol/RCState.h>
#include <daq/slc/runcontrol/RCCommand.h>

#include <daq/slc/database/RunNumberTable.h>
#include <daq/slc/database/DBObjectLoader.h>

#include <daq/slc/psql/PostgreSQLInterface.h>

#include <daq/slc/nsm/NSMCommunicator.h>

#include <daq/slc/system/TCPServerSocket.h>
#include <daq/slc/system/TCPSocket.h>
#include <daq/slc/system/TCPSocketWriter.h>
#include <daq/slc/system/LogFile.h>

#include <daq/slc/base/StringUtil.h>
#include <daq/slc/base/Date.h>

#include <stdlib.h>
#include <unistd.h>

namespace Belle2 {

  class ThscanerStartHandler : public NSMVHandlerText {
  public:
    ThscanerStartHandler(ThscanerCallback& callback, NSMNode& rcnode,
                         NSMNode& ronode,
                         const std::string& name)
      : NSMVHandlerText(name, true, true, "off"),
        m_callback(callback), m_rcnode(rcnode), m_ronode(ronode)
    {
    }
    virtual ~ThscanerStartHandler() throw() {}

  public:
    bool handleSetText(const std::string& start)
    {
      NSMVHandlerText::handleSetText(start);
      if (start == "on") {
        LogFile::info("Starting threshold scan");
        try {
          std::string cmd = StringUtil::form("resetft -%d", m_callback.getFTSW());
          LogFile::debug(cmd);
          system(cmd.c_str());
          m_callback.set(m_rcnode, "ttd_arich.used", (int)0);
          //std::string filename = Date().toString("thscan.%Y-%m-%d-%H-%M-%S");
          NSMCommunicator::send(NSMMessage(m_rcnode, RCCommand::LOAD));
        } catch (const NSMHandlerException& e) {
          LogFile::error("Faild to load run control");
          m_callback.set("start", "off");
          m_callback.set("nevents.cur", 0);
          m_callback.set("nth.cur", 0);
          m_callback.set("state", "READY");
        }
      } else {
        m_callback.set("nevents.cur", 0);
        m_callback.set("nth.cur", 0);
        m_callback.set("state", "READY");
      }
      return true;
    }
  private:
    ThscanerCallback& m_callback;
    NSMNode& m_rcnode;
    NSMNode& m_ronode;
    ConfigFile m_config;

  };

}

using namespace Belle2;

ThscanerCallback::ThscanerCallback(const std::string& name,
                                   ConfigFile& config,
                                   int timeout)
  : m_config(config)
{
  setNode(NSMNode(name));
  setTimeout(timeout);
  m_ttdnode = NSMNode(m_config.get("ttd"));
  m_rcnode = NSMNode(m_config.get("runcontrol"));
  m_ronode = NSMNode(m_config.get("ronode"));
}

ThscanerCallback::~ThscanerCallback() throw()
{
}

void ThscanerCallback::init(NSMCommunicator& com) throw()
{
  add(new ThscanerStartHandler(*this, m_rcnode, m_ronode, "start"));
  add(new NSMVHandlerText("filename", true, false, "arich"));
  add(new NSMVHandlerText("state", true, false, "READY"));
  add(new NSMVHandlerInt("nevents.cur", true, true, m_nevents_cur));
  add(new NSMVHandlerInt("nth.cur", true, true, m_nth_cur));
  add(new NSMVHandlerInt("ftsw", true, true, 183));
  add(new NSMVHandlerText("trg_type", true, true, "pulse"));
  add(new NSMVHandlerInt("rate", true, true, 1000));
  add(new NSMVHandlerInt("nevent", true, true, 1000));
  m_adj = m_config.getBool("adj");
  add(new NSMVHandlerInt("adjust", true, true, (int)m_adj));
  std::string file = m_config.get("file");
  m_obj = DBObjectLoader::load(file);
  m_obj.print();
  addDB(m_obj);
  m_i_adj = 0;
  add(new NSMVHandlerInt("nadj.cur", true, true, (int)m_i_adj));
  initialize();
}

void ThscanerCallback::initialize() throw()
{
  get(m_ttdnode, "dummy_rate", m_trate);
  get(m_ttdnode, "trigger_limit", m_tlimit);
  get(m_ttdnode, "trigger_type", m_ttype);
  get(m_ttdnode, "ftstate", m_ftstate);
  get(m_ttdnode, "toutcnt", m_toutcnt);
  get(m_ttdnode, "ftsw", m_ftsw);
  m_nevents_cur = 0;
  m_nth_cur = 0;
  m_next_run = false;
  set("start", "off");
  set("filename", "arich");
  set("state", "READY");
  set("nevents.cur", 0);
  set("nth.cur", 0);
  set("ftsw", m_ftsw);
  set("trg_type", m_ttype);
  //set("rate", 10000);
  //set("nevent", 1000);
  //set("adjust", (int)m_adj));
  m_i_adj = 0;
  set("nadj.cur", (int)m_i_adj);
  m_recovering = false;
  set("nth.cur", m_nth_cur);
  set(m_rcnode, "ttd_arich.used", 1);
}

void ThscanerCallback::vset(NSMCommunicator& com, const NSMVar& var) throw()
{
  //LogFile::info("%s=%d", var.getName().c_str(), var.getInt());
  if (var.getNode() == m_ttdnode.getName()) {
    if (var.getName() == "toutcnt" && var.getType() == NSMVar::INT) {
      m_toutcnt = var.getInt();
    } else if (var.getName() == "ftstate" && var.getType() == NSMVar::TEXT) {
      std::string ftstate = m_ftstate;
      m_ftstate = var.getText();
    }
  } else if (var.getName().find("hslb") != std::string::npos &&
             var.getName().find("nevent") != std::string::npos &&
             var.getType() == NSMVar::INT) {
    if (m_hslbnevt.find(var.getName()) != m_hslbnevt.end()) {
      m_hslbnevt[var.getName()] = var.getInt();
    }
  } else {
    NSMCallback::vset(com, var);
  }
}

void ThscanerCallback::vreply(NSMCommunicator& com, const std::string& name, bool ret) throw()
{
  const NSMMessage& msg(com.getMessage());
  std::string nodename = msg.getNodeName();
  if (ret && name.find("arich") != std::string::npos) {
    m_count--;
  } else {
    NSMCallback::vreply(com, name, ret);
  }
}

void ThscanerCallback::ok(const char* node, const char* data) throw()
{
  std::string nodename = node;
  RCState state = data;
  if (nodename == m_rcnode.getName()) {
    m_rcnode.setState(state);
    if (state == RCState::ABORTING_RS) {
      LogFile::info("Run control was aborted");
      if (!m_next_run) {
        initialize();
      }
    } else if (state == RCState::NOTREADY_S) {
      if (m_next_run) {
        LogFile::info("Run control is notready");
        std::string cmd = StringUtil::form("resetft -%d", getFTSW());
        LogFile::debug(cmd);
        system(cmd.c_str());
        set(m_rcnode, "ttd_arich.used", (int)0);
        NSMCommunicator::send(NSMMessage(m_rcnode, RCCommand::LOAD));
      }
    } else if (state == RCState::READY_S) {
      LogFile::info("Run control got ready");
      NSMCommunicator::send(NSMMessage(m_rcnode, RCCommand::START));
      m_next_run = true;
    } else if (state == RCState::RUNNING_S) {
      LogFile::info("Run control got running");
      std::string thstate, started;
      get("start", started);
      if (started == "on") {
        get("state", thstate);
        LogFile::info("state=" + thstate);
        m_next_run = false;
        initRun();
      }
    }
  }
}

void ThscanerCallback::timeout(NSMCommunicator&) throw()
{
}

void ThscanerCallback::stopRun()
{
  std::string state;
  get("state", state);
  if (state != "RUNNING") {
    initialize();
    return;
  }
  //m_nth_cur = 0;
  m_nevents_cur = 0;
  set("nevents_cur", m_nevents_cur);
  set("nth.cur", m_nth_cur);
  LogFile::info("Run end");
  int index = m_nth_cur - 1;
  int nth = 0;
  float th0 = 0, dth = 0;
  get("nth", nth);
  get("th0", th0);
  get("dth", dth);
  LogFile::debug("thscan[%d].expno : %d", index, m_expno);
  LogFile::debug("thscan[%d].runno : %d", index, m_runno);
  LogFile::debug("thscan[%d].dth   : %f", index, dth);
  LogFile::debug("thscan[%d].th0   : %f", index, th0);
  double Vth = th0 + index * dth;
  unsigned int val = int((Vth - VTH_MIN) / VTH_STEP);
  LogFile::debug("thscan[%d].val : %d", index, val);
  if (m_nth_cur >= nth) {
    LogFile::info("thscan end");
    int adj = 0;
    get("adjust", adj);
    m_adj = adj;
    if (m_adj && m_obj.hasObject("param")) {
      const DBObjectList& o_params(m_obj.getObjects("param"));
      if (o_params.size() > m_i_adj) {
        m_nth_cur = 0;
        m_nevents_cur = 0;
        m_next_run = true;
        NSMCommunicator::send(NSMMessage(m_rcnode, RCCommand::ABORT));
        return;
      } else {
        initialize();
        std::string cmd = StringUtil::form("resetft -%d", m_ftsw);
        LogFile::debug(cmd);
        system(cmd.c_str());
        return;
      }
    } else {
      sleep(3);
      set(m_rcnode, "ttd_arich.used", (int)1);
      NSMCommunicator::send(NSMMessage(m_rcnode, RCCommand::ABORT));
    }
  }
  LogFile::info("New run start");
  set("nth.cur", m_nth_cur);
  startRun(m_expno, m_runno);
}

bool ThscanerCallback::initRun()
{
  m_nrun_total = 0;
  float th0 = 0, dth = 0;
  get("th0", th0);
  get("dth", dth);
  int nevents = 0, nth = 0;
  get("nevents", nevents);
  get("nth", nth);
  try {
    set(m_ttdnode, "trigger_limit", nevents);
  } catch (const TimeoutException& e) {
    LogFile::error(e.what());
    return false;
  }
  m_nth_cur = 0;
  m_nevents_cur = 0;
  set("nth_cur", 0);
  set("nevents_cur", 0);
  int adj = 0;
  get("adjust", adj);
  m_adj = adj;

  const DBObjectList& o_cprs(m_obj.getObjects("cpr"));
  m_host = std::vector<NSMNode>();
  m_copper = std::vector<std::string>();
  m_hslb = std::vector<std::string>();
  for (size_t i = 0 ; i < o_cprs.size(); i++) {
    std::string host = o_cprs[i].getText("host");
    StringList cname = StringUtil::split(o_cprs[i].getText("name"), ':');
    NSMNode node(host);
    m_host.push_back(NSMNode(host));
    m_copper.push_back(cname[0]);
    std::string hlabel;
    for (int j = 0; j < 4; j++) {
      std::string label = StringUtil::form("%c", 'a' + j);
      int used = 0;
      get(node, m_copper[i] + StringUtil::form("@hslb[%d].used", j), used);
      if (used) {
        hlabel += label;
      }
    }
    LogFile::info("%s", (m_copper[i] + "=" + hlabel).c_str());
    m_hslb.push_back(hlabel);
  }
  if (m_adj && m_obj.hasObject("param")) {
    m_count = 0;
    const DBObjectList& o_params(m_obj.getObjects("param"));
    if (o_params.size() > m_i_adj) {
      const DBObject& o_param(o_params[m_i_adj]);
      std::string path = o_param.getText("path");
      LogFile::notice("offset adjust run[%d] : %s", m_i_adj, path.c_str());
      m_i_adj++;
      set("nadj.cur", (int)m_i_adj);
      for (size_t i = 0; i < m_host.size(); i++) {
        for (int j = 0; j < 4; j++) {
          std::string label = StringUtil::form("%c", 'a' + j);
          if (m_hslb[i].find(label) != std::string::npos) {
            LogFile::info(m_host[i].getName() + " << " + m_copper[i] + StringUtil::form("@arich[%d].loadparam", j) + "=" + path);
            NSMCommunicator::send(NSMMessage(m_host[i], NSMVar(m_copper[i] + StringUtil::form("@arich[%d].loadparam", j), path)));
            m_count++;
          }
        }
      }
      while (m_count > 0) {
        try {
          perform(NSMCallback::wait(NSMNode(), NSMCommand::UNKNOWN, 10));
        } catch (const TimeoutException& e) {
        }
      }
    }
  }
  m_count = 0;
  for (size_t i = 0; i < m_copper.size(); i++) {
    for (int j = 0; j < 4; j++) {
      std::string label = StringUtil::form("%c", 'a' + j);
      if (m_hslb[i].find(label) != std::string::npos) {
        NSMCommunicator::send(NSMMessage(m_host[i], NSMVar(m_copper[i] + StringUtil::form("@arich[%d].th0", j), th0)));
        m_count++;
        NSMCommunicator::send(NSMMessage(m_host[i], NSMVar(m_copper[i] + StringUtil::form("@arich[%d].dth", j), dth)));
        m_count++;
      }
    }
  }
  while (m_count > 0) {
    try {
      perform(NSMCallback::wait(NSMNode(), NSMCommand::UNKNOWN, 10));
    } catch (const TimeoutException& e) {
    }
  }
  set("state", "RUNNING");
  LogFile::debug("thscan start");
  startRun(0, 0);
  return true;
}

void ThscanerCallback::startRun(int expno, int runno)
{
  m_nrun_total++;
  if (m_nrun_total % 100 == 0) {
    LogFile::info("10 sec sleep for rest...");
    struct timespec ts = {10, 0};
    struct timespec rem;
    while (nanosleep(&ts, &rem) != 0) {
      ts = rem;
    }
  }
  m_count = 0;
  for (size_t i = 0; i < m_host.size(); i++) {
    for (int j = 0; j < 4; j++) {
      std::string label = StringUtil::form("%c", 'a' + j);
      if (m_hslb[i].find(label) != std::string::npos) {
        NSMVar var(m_copper[i] + StringUtil::form("@arich[%d].thindex", j), m_nth_cur);
        NSMCommunicator::send(NSMMessage(m_host[i], var));
        m_count++;
      }
    }
  }
  while (m_count > 0) {
    try {
      perform(NSMCallback::wait(NSMNode(), NSMCommand::UNKNOWN, 10));
    } catch (const TimeoutException& e) {
    }
  }
  std::string cmd = StringUtil::form("resetft -%d", m_ftsw);
  if (m_nth_cur == 0) {
    get(m_rcnode, "runno", m_runno);
    get(m_rcnode, "expno", m_expno);
    LogFile::info("initRun");
    std::string filename = StringUtil::form("thscan.%04d.%05d", m_expno, m_runno);
    LogFile::debug("filename : " + filename);
    set("filename", filename);
    float th0 = 0, dth = 0;
    get("th0", th0);
    get("dth", dth);
    int nevents = 0, nth = 0;
    get("nevents", nevents);
    get("nth", nth);
    std::string filepath = StringUtil::form("/home/group/b2daq/slc/data/database/arich/thscan/%s.conf",
                                            filename.c_str());
    std::ofstream fout(filepath.c_str());
    fout << "config  : " << StringUtil::replace(filename, ".", ":") << std::endl
         << "th0     : " << th0 << std::endl
         << "dth     : " << dth << std::endl
         << "nevents : " << nevents << std::endl
         << "nth : " << nth << std::endl;
    fout.close();
    std::string cmd = StringUtil::form("daqdbcreate %s arich_th", filepath.c_str());
    LogFile::debug(cmd);
    system(cmd.c_str());

    LogFile::debug(cmd);
    system(cmd.c_str());
    usleep(100000);
    cmd = StringUtil::form("regft -%d 160 0x%x", m_ftsw, (m_expno << 22) + (m_runno << 8));
    LogFile::debug("runno=%d", m_runno);
    LogFile::debug(cmd);
    system(cmd.c_str());
    usleep(100000);
  }
  usleep(100000);
  int ntrg = m_tlimit;
  if (m_ttype == "aux") {
    cmd = StringUtil::form("trigft -%d -w -n %s %d", m_ftsw, m_ttype.c_str(), ntrg);
  } else {
    cmd = StringUtil::form("trigft -%d -w -n %s %d %d", m_ftsw, m_ttype.c_str(), m_trate, ntrg);
  }
  LogFile::debug(cmd);
  int ret = system(cmd.c_str());
  if (WIFSIGNALED(ret) &&
      (WTERMSIG(ret) == SIGINT || WTERMSIG(ret) == SIGQUIT)) {
    LogFile::warning("trigft was interrupted");
  }
  m_nth_cur++;
  m_hslbnevt = std::map<std::string, int>();
  for (size_t i = 0; i < m_copper.size(); i++) {
    for (int j = 0; j < 4; j++) {
      std::string label = StringUtil::form("%c", 'a' + j);
      if (m_hslb[i].find(label) != std::string::npos) {
        std::string vname = m_copper[i] + StringUtil::form("@hslb[%d].nevent", j);
        m_hslbnevt.insert(std::pair<std::string, int>(vname, 0));
        //LogFile::info(m_host[i].getName() + "<<" + vname);
        NSMCommunicator::send(NSMMessage(m_host[i], NSMCommand::VGET, vname));
      }
    }
  }
  while (true) {
    bool completed = true;
    for (std::map<std::string, int>::iterator it = m_hslbnevt.begin();
         it != m_hslbnevt.end(); it++) {
      if (it->second != m_tlimit) {
        //LogFile::warning("%s=%d", it->first.c_str(), it->second);
        completed = false;
        break;
      }
    }
    if (completed) break;
    try {
      perform(NSMCallback::wait(NSMNode(), NSMCommand::UNKNOWN, 10));
    } catch (const TimeoutException& e) {
    }
  }
  ///*
  std::string start;
  std::string state;
  get("start", start);
  get("state", state);
  if (start == "on" && state == "RUNNING") {
    stopRun();
  }
  //*/
}
