#include "daq/slc/apps/hvcontrold/HVChannelConfig.h"

#include <daq/slc/base/Writer.h>
#include <daq/slc/base/Reader.h>

#include <cstring>
#include <sstream>
#include <iostream>

using namespace Belle2;

HVChannelConfig::HVChannelConfig(uint32 master, uint32 crate,
                                 uint32 slot, uint32 channel)
{
  memset(&_config, 0, sizeof(hv_channel_config));
  _config.master = master;
  _config.crate = crate;
  _config.slot = slot;
  _config.channel = channel;
}

void HVChannelConfig::print() throw()
{
  std::cout << "configid = " << _config.configid << std::endl;
  std::cout << "master = " << _config.master << std::endl;
  std::cout << "crate = " << _config.crate << std::endl;
  std::cout << "slot = " << _config.slot << std::endl;
  std::cout << "channel = " << _config.channel << std::endl;
  std::cout << "turnon = " << _config.turnon << std::endl;
  std::cout << "rampup_speed = " << _config.rampup_speed << std::endl;
  std::cout << "rampdown_speed = " << _config.rampdown_speed << std::endl;
  std::cout << "voltage_limit = " << _config.voltage_limit << std::endl;
  std::cout << "current_limit = " << _config.current_limit << std::endl;
  const size_t nvoltage = getNVoltageDemands();
  const size_t nreserved = getNReserveds();
  for (size_t i = 0; i < nvoltage; i++) {
    std::cout << "voltage_demand[" << i << "] = " << _config.voltage_demand[i] << std::endl;
  }
  for (size_t i = 0; i < nreserved; i++) {
    std::cout << "reserved[" << i << "] = " << _config.reserved[i] << std::endl;
  }

}

void HVChannelConfig::setConfig(const hv_channel_config& config)
{
  memcpy(&_config, &config, sizeof(hv_channel_config));
}

void HVChannelConfig::writeObject(Writer& writer) const throw(IOException)
{
  writer.writeInt(_config.configid);
  writer.writeInt(_config.master);
  writer.writeInt(_config.crate);
  writer.writeInt(_config.slot);
  writer.writeInt(_config.channel);
  writer.writeInt(_config.turnon);
  writer.writeFloat(_config.rampup_speed);
  writer.writeFloat(_config.rampdown_speed);
  writer.writeFloat(_config.voltage_limit);
  writer.writeFloat(_config.current_limit);
  const size_t nvoltage = getNVoltageDemands();
  const size_t nreserved = getNReserveds();
  for (size_t i = 0; i < nvoltage; i++) {
    writer.writeFloat(_config.voltage_demand[i]);
  }
  for (size_t i = 0; i < nreserved; i++) {
    writer.writeFloat(_config.reserved[i]);
  }
}

void HVChannelConfig::readObject(Reader& reader) throw(IOException)
{
  _config.configid = reader.readInt();
  _config.master = reader.readInt();
  _config.crate = reader.readInt();
  _config.slot = reader.readInt();
  _config.channel = reader.readInt();
  _config.turnon = reader.readInt();
  _config.rampup_speed = reader.readFloat();
  _config.rampdown_speed = reader.readFloat();
  _config.voltage_limit = reader.readFloat();
  _config.current_limit = reader.readFloat();
  const size_t nvoltage = getNVoltageDemands();
  const size_t nreserved = getNReserveds();
  for (size_t i = 0; i < nvoltage; i++) {
    _config.voltage_demand[i] = reader.readFloat();
  }
  for (size_t i = 0; i < nreserved; i++) {
    _config.reserved[i] = reader.readFloat();
  }
}

void HVChannelConfig::setValues(DBRecord& record) throw()
{
  const size_t nvoltage = getNVoltageDemands();
  const size_t nreserved = getNReserveds();
  setConfigId(record.getFieldValueInt("configid"));
  setMaster(record.getFieldValueInt("master"));
  setCrate(record.getFieldValueInt("crate"));
  setSlot(record.getFieldValueInt("slot"));
  setChannel(record.getFieldValueInt("channel"));
  setTurnOn(record.getFieldValue("turnon") == "t" ||
            record.getFieldValue("turnon") == "true" ||
            record.getFieldValue("turnon") == "TRUE" ||
            record.getFieldValue("turnon") == "1");
  setRampUpSpeed(record.getFieldValueInt("rampup_speed"));
  setRampDownSpeed(record.getFieldValueInt("rampdown_speed"));
  setVoltageLimit(record.getFieldValueFloat("voltage_limit"));
  setCurrentLimit(record.getFieldValueFloat("current_limit"));
  std::vector<float> value_v = record.getFieldValueFloatArray("voltage_demand");
  for (size_t i = 0; i < nvoltage && i < value_v.size(); i++) {
    setVoltageDemand((HVDemand)i, value_v[i]);
  }
  value_v = record.getFieldValueFloatArray("reversed");
  for (size_t i = 0; i < nreserved && i < value_v.size(); i++) {
    setReserved(i, value_v[i]);
  }
}

const std::string HVChannelConfig::toSQLCreate(const std::string& tablename) throw()
{
  const size_t nvoltage = getNVoltageDemands();
  const size_t nreserved = getNReserveds();
  std::stringstream ss;
  ss << "create table \"" << tablename << "\" ("
     << "record_date timestamp, "
     << "configid int, index int, master int, crate int, slot int, channel int, "
     << "turnon boolean, rampup_speed float, rampdown_speed float, "
     << "voltage_demand float [" << nvoltage << "], voltage_limit float, "
     << "current_limit float, reserved float [" << nreserved << "])";
  return ss.str();
}

const std::string HVChannelConfig::toSQLInsert(const std::string& tablename, int index) throw()
{
  const size_t nvoltage = getNVoltageDemands();
  const size_t nreserved = getNReserveds();
  std::stringstream ss;
  ss << "'{";
  for (size_t n = 0; n < nvoltage; n++) {
    ss << _config.voltage_demand[n];
    if (n < nvoltage - 1) ss << ",";
  }
  ss << "}'";
  std::string s_voltage = ss.str();

  ss.str("");
  ss << "'{";
  for (size_t n = 0; n < nreserved; n++) {
    ss << _config.reserved[n];
    if (n < nreserved - 1) ss << ",";
  }
  ss << "}'";
  std::string s_reserved = ss.str();
  ss.str("");

  ss << "insert into \"" << tablename << "\" (record_date, "
     << "configid, index, master, crate, slot, channel, "
     << "turnon, rampup_speed, rampdown_speed, "
     << "voltage_demand, voltage_limit, current_limit, reserved) values ("
     << "current_timestamp,"
     << _config.configid << "," << index << "," << _config.master << ","
     << _config.crate << "," << _config.slot << "," << _config.channel << ","
     << ((_config.turnon > 0) ? "true" : "false") << "," << _config.rampup_speed << ","
     << _config.rampdown_speed << "," << s_voltage << "," << _config.voltage_limit << ","
     << _config.current_limit << "," << s_reserved << ")";
  return ss.str();
}
