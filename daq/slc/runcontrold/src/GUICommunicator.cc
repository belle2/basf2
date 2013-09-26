#include "GUICommunicator.hh"
#include "MessageBox.hh"

#include <runcontrol/RCState.hh>

#include <node/Connection.hh>

#include <util/Debugger.hh>
#include <util/StringUtil.hh>

#include <fstream>

#include <unistd.h>

using namespace B2DAQ;

bool GUICommunicator::init() throw(IOException)
{
  _is_ready = false;
  _socket = _server_socket.accept();
  _writer = TCPSocketWriter(_socket);
  _reader = TCPSocketReader(_socket);

  std::vector<std::string>& file_path_v(_loader->getFilePathList());
  _writer.writeInt((int)file_path_v.size());
  for (size_t i = 0; i < file_path_v.size(); i++) {
    std::vector<std::string> str_v = B2DAQ::split(file_path_v[i], '/');
    std::string label = str_v[str_v.size() - 1];
    _writer.writeString(label);
    std::ifstream fin(file_path_v[i].c_str());
    std::string buf;
    while (fin && getline(fin, buf)) {
      for (size_t j = 0; j < buf.size(); j++) {
        _writer.writeChar(buf.at(j));
      }
      _writer.writeChar('\n');
    }
    _writer.writeChar('\0');
  }
  _writer.writeString(_loader->getEntryLabel());

  RunControlMessage msg(RunControlMessage::GUI);
  msg.setCommand(RCCommand::SET);
  NSMMessage nsm;

  nsm.setNParams(1);
  nsm.setParam(0, RunControlMessage::FLAG_RUN_TYPE);
  nsm.setData(_data->getRunConfig()->getRunType());
  msg.setMessage(nsm);
  sendMessage(msg);
  usleep(50000);

  nsm.setParam(0, RunControlMessage::FLAG_OPERATORS);
  nsm.setData(_data->getRunConfig()->getOperators());
  msg.setMessage(nsm);
  sendMessage(msg);
  usleep(50000);

  nsm.setNParams(2);
  nsm.setParam(0, RunControlMessage::FLAG_RUN_VERSION);
  nsm.setParam(1, _data->getRunConfig()->getVersion());
  nsm.setData("");
  msg.setMessage(nsm);
  sendMessage(msg);
  usleep(50000);

  nsm.setParam(0, RunControlMessage::FLAG_RUN_NO);
  nsm.setParam(1, _data->getRunStatus()->getRunNumber());
  msg.setMessage(nsm);
  sendMessage(msg);
  usleep(50000);

  nsm.setParam(0, RunControlMessage::FLAG_EXP_NO);
  nsm.setParam(1, _data->getRunStatus()->getExpNumber());
  msg.setMessage(nsm);
  sendMessage(msg);
  usleep(50000);

  nsm.setParam(0, RunControlMessage::FLAG_START_TIME);
  nsm.setParam(1, _data->getRunStatus()->getStartTime());
  msg.setMessage(nsm);
  sendMessage(msg);
  usleep(50000);

  nsm.setParam(0, RunControlMessage::FLAG_END_TIME);
  nsm.setParam(1, _data->getRunStatus()->getEndTime());
  msg.setMessage(nsm);
  sendMessage(msg);
  usleep(50000);

  msg.setCommand(RCCommand::STATE);
  nsm.setNParams(3);
  nsm.setParam(0, -1);
  nsm.setParam(1, _system->getRunControlNode()->getConnection().getId());
  nsm.setParam(2, _system->getRunControlNode()->getState().getId());
  msg.setMessage(nsm);
  sendMessage(msg);
  usleep(50000);
  _is_ready = true;

  return true;
}

bool GUICommunicator::reset() throw()
{
  _is_ready = false;
  _socket.close();
  return true;
}

RunControlMessage GUICommunicator::waitMessage() throw(IOException)
{
  RunControlMessage msg(RunControlMessage::GUI);
  msg.setCommand(RCCommand(_reader.readString().c_str()));
  NSMMessage nsm;
  nsm.setNParams(_reader.readInt());
  for (size_t i = 0; i < nsm.getNParams(); i++) {
    nsm.setParam(i, _reader.readInt());
  }
  nsm.setData(_reader.readString());
  msg.setMessage(nsm);
  return msg;
}

void GUICommunicator::sendMessage(const RunControlMessage& msg)
throw(IOException)
{
  if (_is_ready) {
    try {
      _mutex.lock();
      _writer.writeString(msg.getCommand().getLabelString());
      _writer.writeInt(msg.getMessage().getNParams());
      for (size_t i = 0; i < msg.getMessage().getNParams(); i++) {
        _writer.writeInt(msg.getMessage().getParam(i));
      }
      _writer.writeString(msg.getMessage().getData());
      _mutex.unlock();
    } catch (const IOException& e) {
      _is_ready = false;
      _socket.close();
      _mutex.unlock();
      throw (e);
    }
  }
}
