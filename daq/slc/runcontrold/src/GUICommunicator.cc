#include "GUICommunicator.hh"
#include "MessageBox.hh"

#include <node/State.hh>
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
  _writer.writeString(_db->getHostName());
  _writer.writeString(_db->getDatabase());
  _writer.writeString(_db->getUserName());
  _writer.writeString(_db->getPassword());
  _writer.writeInt(_db->getPort());

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
  msg.setCommand(_reader.readString().c_str());
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
