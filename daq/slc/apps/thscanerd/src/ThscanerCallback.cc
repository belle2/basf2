#include "daq/slc/apps/thscanerd/ThscanerCallback.h"

#include "daq/slc/apps/thscanerd/sa02board.h"

#include <daq/slc/runcontrol/RCNode.h>
#include <daq/slc/runcontrol/RCState.h>
#include <daq/slc/runcontrol/RCCommand.h>

#include <daq/slc/database/DBObjectLoader.h>

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
      if (start == "on") {
        LogFile::info("Starting threshold scan");
        try {
          std::string filename = Date().toString("thscan.%Y-%m-%d-%H-%M-%S");
          LogFile::debug("filename : " + filename);
          m_callback.set("filename", filename);
          m_callback.set(m_ronode, "process.arg[2].val", filename);

          float th0 = 0, dth = 0;
          m_callback.get("th0", th0);
          m_callback.get("dth", dth);
          int nevents = 0, nth = 0;
          m_callback.get("nevents", nevents);
          m_callback.get("nth", nth);

          std::string filepath = StringUtil::form("/disk/data/histofiles/%s.conf", filename.c_str());
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

void ThscanerCallback::init(NSMCommunicator&) throw()
{
  get(m_ttdnode, "trigger_limit", m_tlimit);
  get(m_ttdnode, "ftstate", m_ftstate);
  get(m_ttdnode, "toutcnt", m_toutcnt);
  int endft;
  get(m_ttdnode, "endft", endft);
  m_nevents_cur = 0;
  m_nth_cur = 0;
  add(new ThscanerStartHandler(*this, m_rcnode, m_ronode, "start"));
  add(new NSMVHandlerText("filename", true, false, "arich"));
  add(new NSMVHandlerText("state", true, false, "READY"));
  add(new NSMVHandlerInt("nevents.cur", true, true, m_nevents_cur));
  add(new NSMVHandlerInt("nth.cur", true, true, m_nth_cur));
  m_adj = m_config.getBool("adj");
  add(new NSMVHandlerInt("adjust", true, true, (int)m_adj));
  std::string file = m_config.get("file");
  m_obj = DBObjectLoader::load(file);
  m_obj.print();
  addDB(m_obj);
  m_i_adj = 0;
  add(new NSMVHandlerInt("nadj.cur", true, true, (int)m_i_adj));
}

void ThscanerCallback::vset(NSMCommunicator& com, const NSMVar& var) throw()
{
  if (var.getNode() == m_ttdnode.getName()) {
    if (var.getName() == "toutcnt" && var.getType() == NSMVar::INT) {
      m_toutcnt = var.getInt();
    } else if (var.getName() == "ftstate" && var.getType() == NSMVar::TEXT) {
      std::string ftstate = m_ftstate;
      m_ftstate = var.getText();
    } else if (var.getName() == "endft" && var.getType() == NSMVar::INT) {
      LogFile::info("endit");
      stopRun();
    }
  } else {
    NSMCallback::vset(com, var);
  }
}

void ThscanerCallback::vreply(NSMCommunicator& com, const std::string& name, bool ret) throw()
{
  if (ret && name.find("arich") != std::string::npos) {
    const NSMMessage& msg(com.getMessage());
    std::string nodename = msg.getNodeName();
    LogFile::info(nodename);
    m_count--;
  }
}

void ThscanerCallback::ok(const char* node, const char* data) throw()
{
  std::string nodename = node;
  RCState state = data;
  if (nodename == m_rcnode.getName()) {
    if (state == RCState::NOTREADY_S || state == RCState::ERROR_ES) {
      LogFile::info("Run control was aborted");
      set("start", "off");
      set("state", "READY");
      set("nth.cur", m_nth_cur);
    } else if (state == RCState::READY_S) {
      LogFile::info("Run control got ready");
      std::string thstate, started;
      get("start", started);
      if (started == "on") {
        get("state", thstate);
        if (thstate != "RUNNING") {
          initRun();
        }
      }
    }
  }
}

void ThscanerCallback::timeout(NSMCommunicator&) throw()
{
  //get(m_ttdnode, "trigger_limit", m_tlimit);
}

void ThscanerCallback::stopRun()
{
  std::string state;
  get("state", state);
  if (state != "RUNNING") {
    return;
  }
  LogFile::info("Run end");
  int index = m_nth_cur - 1;
  int nth = 0;
  float th0 = 0, dth = 0;
  get("nth", nth);
  get("th0", th0);
  get("dth", dth);
  LogFile::debug("thscan[%d].expno : %d", index, m_expno);
  LogFile::debug("thscan[%d].runno : %d", index, m_runno);
  LogFile::debug("thscan[%d].dth : %f", index, dth);
  LogFile::debug("thscan[%d].th0 : %f", index, th0);
  double Vth = th0 + index * dth;
  unsigned int val = int((Vth - VTH_MIN) / VTH_STEP);
  LogFile::debug("thscan[%d].val : %d", index, val);
  if (m_nth_cur >= nth) {
    set("start", "off");
    set("state", "READY");
    m_nth_cur = 0;
    m_nevents_cur = 0;
    set("nevents_cur", m_nevents_cur);
    set("nth.cur", m_nth_cur);
    LogFile::info("thscan end");
    int adj = 0;
    get("adjust", adj);
    m_adj = adj;
    if (m_adj && m_obj.hasObject("param")) {
      const DBObjectList& o_params(m_obj.getObjects("param"));
      if (o_params.size() > m_i_adj) {
        initRun();
      }
    } else {
      std::string filename;
      get("filename", filename);
      set(m_ronode, "process.arg[2].val", "arich");
      system(("basf2 --no-stat /home/usr/hvala/scripts/test_thscan.py " + filename).c_str());
    }
    return;
  }
  m_runno++;
  LogFile::info("New run start");
  startRun(m_expno, m_runno);
}

bool ThscanerCallback::initRun()
{
  float th0 = 0, dth = 0;
  get("th0", th0);
  get("dth", dth);
  int nevents = 0, nth = 0;
  get("nevents", nevents);
  get("nth", nth);

  /*
  std::string filename;
  get("filename", filename);
  std::string filepath = StringUtil::form("/disk/data/histofiles/%s.conf", filename.c_str());
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
  */

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
  m_copper = std::vector<NSMNode>();
  m_hslb = std::vector<std::string>();
  for (size_t i = 0 ; i < o_cprs.size(); i++) {
    StringList cname = StringUtil::split(o_cprs[i].getText("name"), ':');
    m_copper.push_back(NSMNode(cname[0]));
    if (cname.size() > 1) {
      m_hslb.push_back(cname[1]);
    } else {
      m_hslb.push_back("abcd");
    }
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
      for (size_t i = 0; i < m_copper.size(); i++) {
        for (int j = 0; j < 4; j++) {
          std::string label = StringUtil::form("%c", 'a' + j);
          if (m_hslb[i].find(label) != std::string::npos) {
            NSMCommunicator::send(NSMMessage(m_copper[i], NSMVar("arich[0].loadparam", path)));
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
        NSMCommunicator::send(NSMMessage(m_copper[i], NSMVar(StringUtil::form("arich[%d].th0", j), th0)));
        NSMCommunicator::send(NSMMessage(m_copper[i], NSMVar(StringUtil::form("arich[%d].dth", j), dth)));
        m_count++;
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
  m_runno = 0;
  m_expno = 0;
  LogFile::debug("thscan start");
  startRun(m_expno, m_runno);
  return true;
}

void ThscanerCallback::startRun(int expno, int runno)
{
  m_count = 0;
  for (size_t i = 0; i < m_copper.size(); i++) {
    for (int j = 0; j < 4; j++) {
      std::string label = StringUtil::form("%c", 'a' + j);
      if (m_hslb[i].find(label) != std::string::npos) {
        NSMVar var(StringUtil::form("arich[%d].thindex", j), m_nth_cur);
        NSMCommunicator::send(NSMMessage(m_copper[i], var));
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
  set("nth.cur", m_nth_cur);
  m_nth_cur++;
  usleep(100000);
  set(m_ttdnode, "startft", (int)((expno << 24) + runno), 10);
}
