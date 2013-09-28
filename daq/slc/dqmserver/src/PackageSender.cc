#include "PackageSender.hh"

#include "HistoServer.hh"
#include "PackageManager.hh"

#include <system/TCPSocketWriter.hh>
#include <system/TCPSocketReader.hh>

#include <util/Debugger.hh>

#include <cstdlib>
#include <map>

using namespace B2DQM;

const int PackageSender::FLAG_ALL = 0;
const int PackageSender::FLAG_UPDATE = 1;
const int PackageSender::FLAG_LIST = 2;

PackageSender::~PackageSender()
{
  _socket.close();
  for (size_t i = 0; i < _conf_v.size(); i++) {
    if (_conf_v[i].buf != NULL) free(_conf_v[i].buf);
  }
  for (size_t i = 0; i < _contents_v.size(); i++) {
    if (_contents_v[i].buf != NULL) free(_contents_v[i].buf);
  }
}

bool PackageSender::sendUpdates(B2DAQ::Writer& writer,
                                size_t i, int n)
throw(B2DAQ::IOException)
{
  PackageManager* manager = _server->getManager(i);
  if (!manager->isAvailable()) {
    writer.writeInt(-(1 + i));
  } else {
    int id = _update_id_v[i];
    size_t count = manager->copyContents(_contents_v[i].buf,
                                         _contents_v[i].size, id);
    _contents_v[i].count = count;
    if (count > 0) {
      writer.writeInt(FLAG_UPDATE);
      writer.writeInt(n);
      writer.writeInt(_contents_v[i].count);
      writer.write(_contents_v[i].buf, _contents_v[i].count);
      if (_update_id_v[i] > id) {
        _update_id_v[i] = id;
        return true;
      }
      _update_id_v[i] = id;
    }
  }
  return false;
}

void PackageSender::run()
{
  using namespace B2DAQ;
  TCPSocketWriter writer(_socket);
  TCPSocketReader reader(_socket);

  while (true) {
    for (size_t i = 0; i < _conf_v.size(); i++) {
      if (_conf_v[i].buf != NULL) free(_conf_v[i].buf);
    }
    for (size_t i = 0; i < _contents_v.size(); i++) {
      if (_contents_v[i].buf != NULL) free(_contents_v[i].buf);
    }
    _conf_v = std::vector<zipped_buf>();
    _contents_v = std::vector<zipped_buf>();
    _xml_v = std::vector<std::string>();
    _update_id_v = std::vector<int>();

    writer.writeInt(FLAG_LIST);
    writer.writeInt((int)_server->getManagers().size());
    for (size_t i = 0; i < _server->getManagers().size(); i++) {
      PackageManager* manager = _server->getManager(i);
      writer.writeString(manager->getName());
    }
    std::vector<bool> monitored_v;
    int npacks = 0;
    std::map<int, int> id_m;
    for (size_t i = 0; i < _server->getManagers().size(); i++) {
      bool monitored = (bool)reader.readChar();
      monitored_v.push_back(monitored);
      id_m.insert(std::map<int, int>::value_type(i, npacks));
      if (monitored) npacks++;
    }

    for (size_t i = 0; i < _server->getManagers().size(); i++) {
      PackageManager* manager = _server->getManager(i);
      size_t size = 0, count;
      char* buf = manager->createConfig(size);
      count = manager->copyConfig(buf, size);
      zipped_buf conf = {buf, size, count};
      _conf_v.push_back(conf);
      buf = manager->createContentsAll(size);
      int id = 0;
      count = manager->copyContentsAll(buf, size, id);
      zipped_buf contents = {buf, size, count};
      _contents_v.push_back(contents);
      _xml_v.push_back(manager->createXML());
      _update_id_v.push_back(id);
    }

    writer.writeInt(FLAG_ALL);
    writer.writeInt(npacks);
    for (size_t i = 0; i < _conf_v.size(); i++) {
      if (monitored_v[i]) {
        try {
          writer.writeInt(_conf_v[i].count);
          writer.write(_conf_v[i].buf, _conf_v[i].count);
          writer.writeInt(_contents_v[i].count);
          writer.write(_contents_v[i].buf, _contents_v[i].count);
          writer.write(_xml_v[i].c_str(), _xml_v[i].size());
          writer.writeChar('\0');
        } catch (const IOException& e) { return; }
      }
      free(_conf_v[i].buf);
      _conf_v[i].buf = NULL;
    }
    _conf_v = std::vector<zipped_buf>();

    try {
      while (true) {
        int i = _server->wait(10);
        if (i < 0) continue;
        if (monitored_v[i] && sendUpdates(writer, i, id_m[i])) break;
        bool breaked = false;
        for (i = 0; i < (int)_contents_v.size(); i++) {
          if (monitored_v[i] &&
              (breaked = sendUpdates(writer, i, id_m[i])))
            break;
        }
        if (breaked) break;
      }
    } catch (const IOException& e) { return; }
  }
}

