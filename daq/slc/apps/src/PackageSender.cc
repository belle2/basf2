#include "daq/slc/apps/PackageSender.h"

#include "daq/slc/apps/PackageManager.h"

#include <daq/slc/system/TCPSocketWriter.h>
#include <daq/slc/system/TCPSocketReader.h>
#include <daq/slc/system/LogFile.h>

#include <cstdlib>
#include <map>

using namespace Belle2;

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

bool PackageSender::sendUpdates(Writer& writer,
                                size_t i, int n)
throw(IOException)
{
  PackageManager* manager = _master->getManager(i);
  int id = _update_id_v[i];
  size_t count = manager->copyContents(_contents_v[i].buf,
                                       _contents_v[i].size, id);
  _contents_v[i].count = count;
  if (count > 0) {
    writer.writeInt(FLAG_UPDATE);
    writer.writeInt(n);
    //writer.writeInt(_contents_v[i].count);
    writer.write(_contents_v[i].buf, _contents_v[i].count);
    if (_update_id_v[i] > id) {
      _update_id_v[i] = id;
      return true;
    }
    _update_id_v[i] = id;
  }
  return false;
}

void PackageSender::run()
{
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

    size_t nman = 0;
    std::vector<PackageManager*>& manager_v(_master->getManagers());
    for (size_t i = 0; i < manager_v.size(); i++) {
      if (manager_v[i]->isAvailable()) {
        nman++;
      }
    }
    writer.writeInt(FLAG_LIST);
    writer.writeInt((int)nman);
    for (size_t i = 0; i < manager_v.size(); i++) {
      if (manager_v[i]->isAvailable())
        writer.writeString(manager_v[i]->getName());
    }
    std::vector<bool> monitored_v;
    int npacks = 0;
    std::map<int, int> id_m;
    for (size_t i = 0; i < manager_v.size(); i++) {
      if (manager_v[i]->isAvailable()) {
        bool monitored = (bool)reader.readChar();
        monitored_v.push_back(monitored);
        id_m.insert(std::map<int, int>::value_type(i, npacks));
        if (monitored) npacks++;
      }
    }

    for (size_t i = 0; i < manager_v.size(); i++) {
      if (manager_v[i]->isAvailable()) {
        PackageManager* manager = manager_v[i];
        size_t size = 0, count = 0;
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
    }

    writer.writeInt(FLAG_ALL);
    writer.writeInt(npacks);
    for (size_t i = 0; i < manager_v.size(); i++) {
      if (monitored_v[i]) {
        try {
          //writer.writeInt(_conf_v[i].count);
          writer.write(_conf_v[i].buf, _conf_v[i].count);
          //writer.writeInt(_contents_v[i].count);
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
        int id = _master->wait(10);
        if (id == 0) continue;
        if (id < 0) break;
        id--;
        if (manager_v[id]->isAvailable()) {
          LogFile::debug("send update : %s (updateid=%d)",
                         manager_v[id]->getPackage()->getName().c_str(),
                         manager_v[id]->getPackage()->getUpdateId());
          if (monitored_v[id]) sendUpdates(writer, id, id_m[id]);
          for (id = 0; id < (int)_contents_v.size(); id++) {
            if (monitored_v[id]) sendUpdates(writer, id, id_m[id]);
          }
        }
      }
    } catch (const IOException& e) {
      _socket.close();
      return;
    }
  }
}

