#include "daq/slc/apps/runcontrold/RCGUICommunicator.h"

#include "daq/slc/apps/runcontrold/RCMasterCallback.h"
#include "daq/slc/apps/runcontrold/RCMaster.h"
#include "daq/slc/apps/runcontrold/RCDatabaseManager.h"

#include <daq/slc/system/LogFile.h>

#include <daq/slc/base/Debugger.h>

using namespace Belle2;

RCGUICommunicator::RCGUICommunicator(const TCPSocket& socket,
                                     RCMasterCallback* callback)
  : _socket(socket), _writer(socket), _reader(socket), _callback(callback)
{
  RCMaster* master = _callback->getMaster();
  master->lock();
  master->addMasterCommunicator(this);
  master->unlock();
}

RCGUICommunicator::~RCGUICommunicator() throw()
{
  RCMaster* master = _callback->getMaster();
  master->lock();
  master->removeMasterCommunicator(this);
  master->unlock();
}

bool RCGUICommunicator::sendMessage(const RunControlMessage& msg) throw()
{
  _socket_mutex.lock();
  try {
    LogFile::debug("send message to GUI (%s<<%s)",
                   msg.getNode()->getName().c_str(), msg.getCommand().getAlias());
    _writer.writeInt(msg.getCommand().getId());
    _writer.writeString(msg.getNode()->getName());
    _writer.writeInt(msg.getMessage().getNParams());
    for (size_t i = 0; i < msg.getMessage().getNParams(); i++) {
      _writer.writeInt(msg.getMessage().getParam(i));
    }
    _writer.writeString(msg.getMessage().getData());
  } catch (const IOException& e) {
    Belle2::debug("%s:%d %s", __FILE__, __LINE__, e.what());
    _socket.close();
    _socket_mutex.unlock();
    return false;
  }
  _socket_mutex.unlock();
  return true;
}

bool RCGUICommunicator::sendState(NSMNode* node) throw()
{
  _socket_mutex.lock();
  try {
    LogFile::debug("send state to GUI (%s=%s)", node->getName().c_str(),
                   node->getState().getAlias());
    _writer.writeInt(Command::STATE.getId());
    _writer.writeString(node->getName());
    _writer.writeInt(node->getState().getId());
    _writer.writeInt(node->getConnection().getId());
  } catch (const IOException& e) {
    Belle2::debug("%s:%d %s", __FILE__, __LINE__, e.what());
    _socket.close();
    _socket_mutex.unlock();
    return false;
  }
  _socket_mutex.unlock();
  return true;
}

bool RCGUICommunicator::sendDataObject(const std::string& name,
                                       DataObject* data) throw()
{
  _socket_mutex.lock();
  try {
    _writer.writeInt(Command::DATA.getId());
    _writer.writeString(name);
    _writer.writeObject(*data);
  } catch (const IOException& e) {
    _socket.close();
    _socket_mutex.unlock();
    return false;
  }
  _socket_mutex.unlock();
  return true;
}

void RCGUICommunicator::run()
{
  try {
    NSMMessage msg;
    RCMaster* master = _callback->getMaster();
    while (true) {
      int number = _reader.readInt();
      Command cmd = number;
      if (cmd == Command::DATA) {
        std::string name = _reader.readString();
        RCMaster* master = _callback->getMaster();
        master->lock();
        DataObject* data = master->getData();
        data->print();
        if (name == master->getNode()->getName()) {
          data->readObject(_reader);
        } else {
          DataObject* data_c = data->getObject(name);
          if (data_c != NULL) {
            data_c->readObject(_reader);
          }
        }
        master->unlock();
      } else if (cmd == Command::SAVE) {
        _reader.readObject(msg);
        master->lock();
        RunConfig* run_config = master->getConfig();
        run_config->update();
        int confno = run_config->getConfigNumber() + 1;
        run_config->setConfigNumber(confno);
        master->getStatus()->update();
        master->getDBManager()->writeConfigs();
        master->unlock();
      } else if (cmd == Command::RECALL) {
        _reader.readObject(msg);
        master->lock();
        master->getStatus()->update();
        master->getDBManager()->readConfigs();
        master->unlock();
      } else if (cmd == Command::ACTIVATE ||
                 cmd == Command::INACTIVATE) {
        _reader.readObject(msg);
        master->lock();
        NSMNode* node = master->getNodeByName(msg.getData());
        if (node != NULL) {
          bool activated = cmd == Command::ACTIVATE;
          RunConfig* run_config = master->getConfig();
          node->setUsed(activated);
          run_config->update();
          int confno = run_config->getConfigNumber() + 1;
          run_config->setConfigNumber(confno);
          RunStatus* run_status = master->getStatus();
          run_status->setRunConfig(confno);
          master->getDBManager()->writeConfigs();
        }
        master->unlock();
      } else {
        _reader.readObject(msg);
        std::string text = msg.getData();
        LogFile::debug(text);
        _callback->setForce(text.find("ALL") == std::string::npos);
        _callback->setMessage(msg);
        if (_callback->perform(msg)) {

        }
      }
    }
  } catch (const IOException& e) {
    _socket.close();
    LogFile::debug("Closed connection to GUI");
  }
}
