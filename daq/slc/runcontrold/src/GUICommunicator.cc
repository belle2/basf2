#include "GUICommunicator.hh"
#include "MessageBox.hh"

#include <node/State.hh>
#include <node/Connection.hh>

#include <util/Debugger.hh>
#include <util/StringUtil.hh>

#include <fstream>

#include <string.h>
#include <unistd.h>

using namespace B2DAQ;

bool GUICommunicator::init() throw(IOException)
{
  _is_ready = false;
  _socket = _server_socket.accept();
  _writer = TCPSocketWriter(_socket);
  _reader = TCPSocketReader(_socket);
  _buf = BufferedWriter(2048);
  std::vector<std::string>& file_path_v(_loader->getFilePathList());
  _writer.writeInt((int)file_path_v.size());
  for (size_t i = 0; i < file_path_v.size(); i++) {
    _buf.seekTo(0);
    std::vector<std::string> str_v = B2DAQ::split(file_path_v[i], '/');
    std::string label = str_v[str_v.size() - 1];
    _writer.writeString(label);
    std::ifstream fin(file_path_v[i].c_str());
    std::string buf;
    while (fin && getline(fin, buf)) {
      for (size_t j = 0; j < buf.size(); j++) {
        _buf.writeChar(buf.at(j));
      }
      _buf.writeChar('\n');
    }
    _buf.writeChar('\0');
    _writer.write(_buf.ptr(), _buf.count());
  }
  _buf.seekTo(0);
  _buf.writeString(_loader->getEntryLabel());
  _buf.writeString(_db->getHostName());
  _buf.writeString(_db->getDatabase());
  _buf.writeString(_db->getUserName());
  _buf.writeString(_db->getPassword());
  _buf.writeInt(_db->getPort());
  _writer.write(_buf.ptr(), _buf.count());
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
      _buf.seekTo(0);
      _buf.writeString(msg.getCommand().getLabelString());
      _buf.writeInt(msg.getMessage().getNParams());
      for (size_t i = 0; i < msg.getMessage().getNParams(); i++) {
        _buf.writeInt(msg.getMessage().getParam(i));
      }
      _buf.writeString(msg.getMessage().getData());
      _writer.write(_buf.ptr(), _buf.count());
      /*
      _writer.writeString(msg.getCommand().getLabelString());
      _writer.writeInt(msg.getMessage().getNParams());
      for (size_t i = 0; i < msg.getMessage().getNParams(); i++) {
        _writer.writeInt(msg.getMessage().getParam(i));
      }
      _writer.writeString(msg.getMessage().getData());
      */
      _mutex.unlock();
    } catch (const IOException& e) {
      B2DAQ::debug("[DEBUG] %s:%d error=%s", __FILE__, __LINE__, e.what());
      _is_ready = false;
      _socket.close();
      _mutex.unlock();
      throw (e);
    }
  }
}
