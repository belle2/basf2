
#include "daq/slc/apps/runcontrold/RCMasterCallback.h"

#include "daq/slc/apps/runcontrold/RCCommunicator.h"
#include "daq/slc/apps/runcontrold/RCMaster.h"
#include "daq/slc/apps/runcontrold/RCSequencer.h"
#include "daq/slc/apps/runcontrold/RCNSMCommunicator.h"
#include "daq/slc/apps/runcontrold/RCDatabaseManager.h"

#include <daq/slc/nsm/NSMCommunicator.h>

#include <daq/slc/system/PThread.h>
#include <daq/slc/system/Time.h>
#include <daq/slc/system/LogFile.h>

#include <daq/slc/base/State.h>
#include <daq/slc/base/StringUtil.h>
#include <daq/slc/base/Debugger.h>

#include <fstream>
#include <sstream>

using namespace Belle2;

RCMasterCallback::RCMasterCallback(NSMNode* node)
  : RCCallback(node)
{
  _rc_node = NULL;
}

RCMasterCallback::~RCMasterCallback() throw()
{

}

void RCMasterCallback::init() throw()
{
}

bool RCMasterCallback::distribute(Command command,
                                  int num0, int num1, int num2)
{
  _master->lock();
  std::string str = getMessage().getData();
  std::vector<std::string> str_v = Belle2::split(str, ' ');
  std::string name = (str_v.size() > 0) ? str_v[0] : "";
  NSMNode* node = _master->getNodeByName(name);
  RunControlMessage msg(node, command, num0, num1, num2);
  if (str_v.size() > 1) {
    msg.getMessage().setData(str_v[1]);
  }
  if (node != NULL) {
    RCCommunicator* comm = _master->getClientCommunicator();
    if (command == Command::LOAD &&
        num0 < 0 && node->getData() != NULL) {
      msg.getMessage().setParam(0, node->getData()->getConfigNumber());
    }
    try {
      if (!comm->isOnline(node)) {
        if (node->getConnection() == Connection::ONLINE) {
          node->setState(State::ERROR_ES);
          node->setConnection(Connection::OFFLINE);
          _master->sendStateToMaster(node);
        }
      } else {
        if (!comm->sendMessage(msg)) {
          _master->getNode()->setState(State::ERROR_ES);
        } else {
          State state = msg.getCommand().nextTState();
          if (state != State::UNKNOWN)
            node->setState(state);
        }
        _master->sendStateToMaster(node);
      }
    } catch (const NSMHandlerException& e) {
      LogFile::fatal("NSM error : %s", e.what());
      setReply("NSM error");
      _master->unlock();
      return false;
    }
  } else {
    PThread(new RCSequencer(_master, msg));
  }
  _master->unlock();
  return true;
}

bool RCMasterCallback::boot() throw()
{
  _node->setState(State::BOOTING_TS);
  return distribute(Command::BOOT, 0, 0, 0);
}

bool RCMasterCallback::load() throw()
{
  _node->setState(State::LOADING_TS);
  _master->lock();
  _master->getStatus()->update();
  int confno = (getMessage().getNParams() > 0) ? getMessage().getParam(0) : -1;
  confno = _master->getDBManager()->readConfigs(confno);
  _master->sendDataObjectToMaster(_master->getNode()->getName(),
                                  _master->getData());
  _master->sendDataObjectToMaster(_master->getConfig()->getClassName(),
                                  _master->getConfig());
  _master->sendDataObjectToMaster(_master->getStatus()->getClassName(),
                                  _master->getStatus());
  _master->unlock();
  //trigft();
  return distribute(Command::LOAD, -1, 0, 0);
}

bool RCMasterCallback::start() throw()
{
  _node->setState(State::STARTING_TS);
  NSMMessage& nsm(getMessage());
  int expno = nsm.getParam(0);
  int runno = nsm.getParam(1);
  int subno = 0;
  _master->lock();
  RunStatus* status = _master->getStatus();
  std::string data = nsm.getData();
  std::string nodename, run_typre, operators, comment;
  int index = 0;
  std::stringstream ss;
  LogFile::debug(nsm.getData());
  for (size_t i = 0; i < data.size(); i++) {
    char c = data.at(i);
    if (index == 0 && c == ' ') {
      nsm.setData(ss.str());
      ss.str("");
      index++;
      continue;
    }
    if (index == 1 && c == '|') {
      status->setRunType(ss.str());
      ss.str("");
      index++;
      continue;
    }
    if (index == 2 && c == '|') {
      std::string s = Belle2::replace(ss.str(), "\'", "\\\'");
      s = Belle2::replace(s, "\"", "\\\"");
      s = Belle2::replace(s, "\r\n", "<br/>");
      s = Belle2::replace(s, "\n", "<br/>");
      s = Belle2::replace(s, "\r", "<br/>");
      status->setOperators(s);
      ss.str("");
      index++;
      continue;
    }
    ss << c;
  }
  if (index > 2 && ss.str().size() > 0) {
    std::string s = Belle2::replace(ss.str(), "\'", "\\\'");
    s = Belle2::replace(s, "\"", "\\\"");
    s = Belle2::replace(s, "\r\n", "<br/>");
    s = Belle2::replace(s, "\n", "<br/>");
    s = Belle2::replace(s, "\r", "<br/>");
    status->setComment(s);
  }
  if (expno < 0) expno = 0;
  if (runno < 0) runno = status->getColdNumber() + 1;
  if (_master->isGlobal()) {
    std::ofstream fout(Belle2::form("%s/daq/slc/data/config/runnumber.conf",
                                    getenv("BELLE2_LOCAL_DIR")).c_str());
    fout << "#" << std::endl
         << "#" << std::endl
         << "#" << std::endl
         << "" << std::endl
         << "EXP_NUMBER  : " << expno << std::endl
         << "RUN_NUMBER  : " << runno << std::endl
         << "" << std::endl
         << "#" << std::endl
         << "#" << std::endl
         << "#" << std::endl;
    fout.close();
  }
  status->setExpNumber(expno);
  status->setColdNumber(runno);
  status->setHotNumber(subno);
  status->setStartTime(Time().getSecond());
  status->setEndTime(-1);
  _master->getDBManager()->writeStatus();
  _master->sendDataObjectToMaster(status->getClassName(), status);
  LogFile::info("New run status: run#%04d.%06d.%04d, opearator=%s, comment=%s",
                expno, runno, subno, status->getOperators().c_str(),
                status->getComment().c_str());
  _master->unlock();
  return distribute(Command::START, expno, runno, subno);
}

bool RCMasterCallback::stop() throw()
{
  _node->setState(State::STOPPING_TS);
  _master->lock();
  RunStatus* status = _master->getStatus();
  status ->setEndTime(Time().getSecond());
  _master->getDBManager()->writeStatus();
  _master->sendDataObjectToMaster(status->getClassName(), status);
  _master->unlock();
  return distribute(Command::STOP, 0, 0, 0);
}

bool RCMasterCallback::trigft() throw()
{
  _master->lock();
  RCCommunicator* comm = _master->getClientCommunicator();
  for (RCMaster::NSMNodeList::iterator it = _master->getNSMNodes().begin();
       it != _master->getNSMNodes().end(); it++) {
    NSMNode* node = *it;
    if (!comm->isOnline(node)) {
      if (node->getConnection() == Connection::ONLINE) {
        node->setState(State::ERROR_ES);
        node->setConnection(Connection::OFFLINE);
        _master->getNode()->setState(State::ERROR_ES);
        _master->sendStateToMaster(node);
      }
    } else if (node->getData() != NULL && node->getData()->hasObject("ftsw")) {
      DataObject* ftsw = node->getData()->getObject("ftsw");
      if (ftsw->hasValue("trigger_type") && ftsw->hasValue("dummy_rate") &&
          ftsw->hasValue("trigger_limit")) {
        NSMMessage& nsm(getMessage());
        if (nsm.getNParams() > 2 && (int)nsm.getParam(0) >= 0) {
          ftsw->setInt("trigger_type", nsm.getParam(0));
          ftsw->setInt("dummy_rate", nsm.getParam(1));
          ftsw->setInt("trigger_limit", nsm.getParam(2));
          ftsw->setConfigNumber(ftsw->getConfigNumber() + 1);
          RunConfig* run_config =  _master->getConfig();
          RunStatus* run_status =  _master->getStatus();
          run_config->update();
          int confno = run_config->getConfigNumber() + 1;
          run_config->setConfigNumber(confno);
          run_status->setRunConfig(confno);
          _master->getDBManager()->writeConfigs();
        }
        nsm.setNParams(3);
        nsm.setParam(0, ftsw->getInt("trigger_type"));
        nsm.setParam(1, ftsw->getInt("dummy_rate"));
        nsm.setParam(2, ftsw->getInt("trigger_limit"));
        RunControlMessage msg(node, Command::TRIGFT, nsm);
        try {
          comm->sendMessage(msg);
        } catch (const NSMHandlerException& e) {
          LogFile::fatal("NSM error : %s", e.what());
          setReply("NSM error");
          _master->unlock();
          return false;
        }
      }
    }
  }
  _master->unlock();
  return true;
}

bool RCMasterCallback::recover() throw()
{
  _node->setState(State::RECOVERING_RS);
  return distribute(Command::RECOVER, 0, 0, 0);
}

bool RCMasterCallback::abort() throw()
{
  _node->setState(State::ABORTING_RS);
  return distribute(Command::ABORT, 0, 0, 0);
}

bool RCMasterCallback::pause() throw()
{
  return distribute(Command::PAUSE, 0, 0, 0);
}

bool RCMasterCallback::resume() throw()
{
  return distribute(Command::RESUME, 0, 0, 0);
}

bool RCMasterCallback::stateCheck() throw()
{
  if (_rc_node == NULL) {
    _rc_node = new NSMNode(getMessage().getNodeName());
    RCNSMCommunicator* com = new RCNSMCommunicator(getCommunicator(), _rc_node);
    _master->lock();
    _master->addMasterCommunicator(com);
    _master->unlock();
  }
  return true;
}

