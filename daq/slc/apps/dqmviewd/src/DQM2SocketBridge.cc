#include "daq/slc/apps/dqmviewd/DQM2SocketBridge.h"

#include "daq/slc/apps/dqmviewd/DQMViewCallback.h"

#include <daq/slc/database/ConfigInfoTable.h>
#include <daq/slc/database/ConfigObjectTable.h>
#include <daq/slc/database/LoggerObjectTable.h>
#include <daq/slc/database/RunNumberInfoTable.h>

#include <daq/slc/nsm/NSMCommunicator.h>

#include <daq/slc/dqm/DQMCommand.h>

#include <daq/slc/system/LogFile.h>
#include <daq/slc/base/StringUtil.h>

#include <unistd.h>
#include <sstream>

using namespace Belle2;

DQM2SocketBridge::DQM2SocketBridge(const TCPSocket& socket,
                                   DQMViewCallback* callback,
                                   DBInterface* db)
  : m_callback(callback), m_db(db), m_socket(socket),
    m_writer(socket), m_reader(socket)
{
  //  m_callback->setBridge(this);
}

DQM2SocketBridge::~DQM2SocketBridge() throw()
{
}

bool DQM2SocketBridge::sendMessage(const NSMMessage& msg) throw()
{
  m_mutex.lock();
  try {
    m_writer.writeObject(msg);
    m_mutex.unlock();
    return true;
  } catch (const IOException& e) {
    LogFile::warning("Connection failed for writing");
    m_socket.close();
  }
  m_mutex.unlock();
  return false;
}

bool DQM2SocketBridge::recieveMessage(NSMMessage& msg) throw()
{
  try {
    msg.init();
    m_reader.readObject(msg);
    return true;
  } catch (const IOException& e) {
    LogFile::warning("Connection failed for reading");
    m_socket.close();
  }
  return false;
}

void DQM2SocketBridge::run() throw()
{
  sleep(5);
  while (true) {
    NSMMessage msg_out(m_callback->getNode().getName());
    NSMMessage msg;
    if (!recieveMessage(msg)) {
      return;
    }
    const std::string reqname = msg.getRequestName();
    const std::string nodename = m_callback->getNode().getName();
    DQMCommand cmd(reqname.c_str());
    if (cmd == DQMCommand::DQMDIRGET) {
      StringList str_v = StringUtil::split(msg.getData(), '\n', 5);
      msg_out.setNodeName(nodename);
      msg_out.setRequestName(DQMCommand::DQMDIRSET);
      //msg_out.setData(obj);
      sendMessage(msg_out);
    } else if (cmd == DQMCommand::DQMHISTGET) {

    } else {
      LogFile::warning("Unknown request : %s", reqname.c_str());
    }
  }
}

