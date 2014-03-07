#include "daq/slc/apps/hvcontrold/HVNodeInfo.h"

#include <daq/slc/base/Writer.h>
#include <daq/slc/base/Reader.h>
#include <daq/slc/base/StringUtil.h>

#include <fstream>
#include <sstream>
#include <cstring>

using namespace Belle2;

int HVNodeInfo::getLatestConfigId(DBInterface* db) throw()
{
  const std::string name = _node->getName();
  db->execute("select configid from \"%s\" order by configid desc limit 1;", name.c_str());
  DBRecordList& record_v(db->loadRecords());
  int configid = 0;//_configid;
  if (record_v.size() > 0) {
    configid = record_v[0].getFieldValueInt("configid");
  }
  return configid;
}

int HVNodeInfo::create(DBInterface* db) throw()
{
  if (_node == NULL) return 0;
  const std::string name = _node->getName();
  if (!db->checkTable(name)) {
    db->execute(HVChannelInfo().getConfig().toSQLCreate(name));
    return _configid = 0;
  } else {
    return getLatestConfigId(db);
  }
}

int HVNodeInfo::save(DBInterface* db) throw()
{
  if (_node == NULL) return 0;
  const std::string name = _node->getName();
  if (db->checkTable(name)) {
    _configid = getLatestConfigId(db) + 1;
    int index = 0;
    for (HVChannelInfoList::iterator it = _info_v.begin();
         it != _info_v.end(); it++) {
      HVChannelInfo& info(*it);
      info.getConfig().setConfigId(_configid);
      db->execute(info.getConfig().toSQLInsert(name, index++));
    }
  }
  return _configid;
}

int HVNodeInfo::load(DBInterface* db) throw()
{
  if (_node == NULL) return 0;
  const std::string name = _node->getName();
  if (db->checkTable(name)) {
    db->execute("select * from \"%s\" where configid = %d", name.c_str(), _configid);
    DBRecordList& record_v(db->loadRecords());
    if (record_v.size() == 0) return 0;
    for (size_t i = 0; i < record_v.size(); i++) {
      DBRecord& record(record_v[i]);
      size_t index = record.getFieldValueInt("index");
      if (index < _info_v.size()) {
        HVChannelInfo& info(_info_v[index]);
        info.getConfig().setValues(record);
      }
    }
  }
  return _configid;
}

void HVNodeInfo::writeConfig(Writer& writer) const throw(IOException)
{
  writer.writeString(_node->getName());
  writer.writeInt(_configid);
  writer.writeInt(_info_v.size());
  for (HVChannelInfoList::iterator it = _info_v.begin();
       it != _info_v.end(); it++) {
    HVChannelConfig& config(it->getConfig());
    writer.writeObject(config);
  }
}

void HVNodeInfo::writeStatus(Writer& writer) const throw(IOException)
{
  writer.writeString(_node->getName());
  writer.writeInt(_node->getState().getId());
  writer.writeInt(_info_v.size());
  for (HVChannelInfoList::iterator it = _info_v.begin();
       it != _info_v.end(); it++) {
    HVChannelStatus& status(it->getStatus());
    writer.writeObject(status);
  }
}

void HVNodeInfo::readConfig(Reader& reader) throw(IOException)
{
  _node->setName(reader.readString());
  _configid = reader.readInt();
  size_t ninfos = reader.readInt();
  if (ninfos != _info_v.size()) reset(ninfos);
  for (HVChannelInfoList::iterator it = _info_v.begin();
       it != _info_v.end(); it++) {
    HVChannelConfig& config(it->getConfig());
    reader.readObject(config);
  }
}

void HVNodeInfo::readStatus(Reader& reader) throw(IOException)
{
  _node->setName(reader.readString());
  _node->setState(reader.readInt());
  size_t ninfos = reader.readInt();
  if (ninfos != _info_v.size()) reset(ninfos);
  for (HVChannelInfoList::iterator it = _info_v.begin();
       it != _info_v.end(); it++) {
    HVChannelStatus& status(it->getStatus());
    reader.readObject(status);
  }
}

void HVNodeInfo::loadStatus(hv_status* status) throw()
{
  if (status == NULL) return;
  _node->setState(HVState(status->state));
  for (size_t i = 0; i < _info_v.size(); i++) {
    HVChannelStatus& ch_status(_info_v[i].getStatus());
    ch_status.setState(HVState(status->ch_state[i]));
    ch_status.setVoltageMon(status->voltage_mon[i]);
    ch_status.setCurrentMon(status->current_mon[i]);
  }
}

void HVNodeInfo::readFile(const std::string& filename) throw()
{
  std::ifstream fin(filename.c_str());
  std::string line;
  while (fin && getline(fin, line)) {
    if (line.size() == 0 || line.at(0) == '#') continue;
    HVChannelInfo ch_info;
    HVChannelConfig& ch_config(ch_info.getConfig());
    int count = 0;
    std::stringstream ss;
    for (size_t i = 0; i < line.size(); i++) {
      char c = line.at(i);
      if (c == ' ' || c == '\t') continue;
      if (c == ',') {
        std::string s = ss.str();
        if (count == 0) ch_config.setCrate(atoi(s.c_str()));
        else if (count == 1) ch_config.setSlot(atoi(s.c_str()));
        else if (count == 2) ch_config.setChannel(atoi(s.c_str()));
        else if (count == 3) ch_config.setTurnOn(s == "true" || s == "t" || s == "TRUE");
        else if (count == 4) ch_config.setRampUpSpeed(atof(s.c_str()));
        else if (count == 5) ch_config.setRampDownSpeed(atof(s.c_str()));
        else if (count == 6) ch_config.setVoltageLimit(atof(s.c_str()));
        else if (count == 7) ch_config.setCurrentLimit(atof(s.c_str()));
        else if (count == 8) {
          for (; i < line.size(); i++) {
            c = line.at(i);
            if (c == '}') {
              s = ss.str();
              break;
            }
            ss << c;
          }
          s = Belle2::replace(s, "{", "");
          std::vector<std::string> s_v = split(s, ',');
          size_t nj = ch_config.getNVoltageDemands();
          for (size_t j = 0; j < s_v.size() && j < nj; j++) {
            ch_config.setVoltageDemand((HVDemand)j, atof(s_v[j].c_str()));
          }
        } else if (count == 9) {
          for (; i < line.size(); i++) {
            c = line.at(i);
            if (c == '}') {
              s = ss.str();
              break;
            }
            ss << c;
          }
          s = Belle2::replace(s, "{", "");
          std::vector<std::string> s_v = split(s, ',');
          size_t nj = ch_config.getNReserveds();
          for (size_t j = 0; j < s_v.size() && j < nj; j++) {
            ch_config.setReserved(j, atof(s_v[j].c_str()));
          }
        } else if (count == 9) {
          for (; i < line.size(); i++) {
            c = line.at(i);
            if (c == '}') {
              s = ss.str();
              break;
            }
            ss << c;
          }
          s = Belle2::replace(s, "{", "");
          std::vector<std::string> s_v = split(s, ',');
          size_t nj = ch_config.getNReserveds();
          for (size_t j = 0; j < s_v.size() && j < nj; j++) {
            ch_config.setReserved(j, atof(s_v[j].c_str()));
          }
        }
        count++;
        ss.str("");
        ch_config.print();
        continue;
      }
      ss << c;
    }
    addChannel(ch_info);
  }
}
