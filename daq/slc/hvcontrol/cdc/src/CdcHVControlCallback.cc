#include "daq/slc/hvcontrol/cdc/CdcHVControlCallback.h"

#include "daq/slc/system/LogFile.h"

// add
//#include <string>
#include <string.h>
#include <iostream>
#include <unistd.h>

// 20150220
// 20170711 Modified records snmpget/set commands into log file (T.Konno)

using namespace Belle2;

void CdcHVControlCallback::initialize() throw()
{
  const DBObject& config(getConfig().get());
  const DBObjectList& c_crate_v(config.getObjects("crate"));
  for (size_t i = 0; i < c_crate_v.size(); i++) {
    int crateid = i + 1;
    const DBObject& c_crate(c_crate_v[i]);
    const DBObjectList& c_channel_v(c_crate.getObjects("channel"));
    for (size_t j = 0; j < c_channel_v.size(); j++) {
      const DBObject& c_channel(c_channel_v[j]);
      int slot = c_channel.getInt("slot");
      int channel = c_channel.getInt("channel");
      LogFile::debug("crate : %d, slot : %d, channel: %d", crateid, slot, channel);
    }
  }
  // crate power ON
  // Mpod ON
  // system("snmpset -v 2c -m +WIENER-CRATE-MIB -c private 192.168.0.21 sysMainSwitch.0 i 1");
  // printf(" !! crate power ON !!\n");
  //
  /*
  float v = 0.;
  float cur = 0.;
  int crate = 1;
  int slot = 1;
  int channel = 1;
  int nch = 100 * (slot - 1) + (channel - 1);
  int nch2 = 100 * (2 - 1) + (channel - 1);
  char buf[256];
  memset((void*) buf, (int)'\0', sizeof(buf));

  float voltage = 0.;
  float current = 0.;
  for (int i = 0; i < 5; i++) {
    nch = 100 * (slot - 1) + i;
    nch2 = 100 * (2 - 1) + i;
    v = getConfig().getChannel(crate, slot, channel).getVoltageDemand();
    // set
    voltage = v;
    sprintf(buf, "snmpset -v 2c -m +WIENER-CRATE-MIB -c guru 192.168.0.21 outputVoltage.u%d F %f", nch, voltage);
    system(buf);
    sprintf(buf, "snmpset -v 2c -m +WIENER-CRATE-MIB -c guru 192.168.0.21 outputVoltage.u%d F %f", nch2, voltage);
    system(buf);
    cur = getConfig().getChannel(crate, slot, channel).getCurrentLimit();
    current = cur / 1000000.;
    sprintf(buf, "snmpset -v 2c -m +WIENER-CRATE-MIB -c guru 192.168.0.21 outputCurrent.u%d F %f", nch, current);
    system(buf);
    sprintf(buf, "snmpset -v 2c -m +WIENER-CRATE-MIB -c guru 192.168.0.21 outputCurrent.u%d F %f", nch2, current);
    system(buf);
  }
  // ramp up/down
  sprintf(buf, "snmpset -v 2c -m +WIENER-CRATE-MIB -c guru 192.168.0.21 outputVoltageRiseRate.u%d F %f", 0, voltage - 10.);
  system(buf);
  sprintf(buf, "snmpset -v 2c -m +WIENER-CRATE-MIB -c guru 192.168.0.21 outputVoltageRiseRate.u%d F %f", 100, voltage - 15.);
  system(buf);
  */
}

void CdcHVControlCallback::timeout() throw()
{

}

void CdcHVControlCallback::store(int index) throw(IOException)
{
  LogFile::notice("store called : index = %d", index);
}

void CdcHVControlCallback::recall(int index) throw(IOException)
{
  LogFile::notice("recall called : index = %d", index);
}

void CdcHVControlCallback::setSwitch(int crate, int slot, int channel, bool switchon) throw(IOException)
{
  char buf[256];
  memset((void*) buf, (int)'\0', sizeof(buf));
  int nch = 100 * (slot - 1) + (channel - 1);
  sprintf(buf, "snmpset -v 2c -m +WIENER-CRATE-MIB -c guru 192.168.0.21 outputSwitch.u%d i %d", nch, (int)switchon);
  LogFile::info("%s", buf);
  system(buf);
}

void CdcHVControlCallback::setRampUpSpeed(int crate, int slot, int channel, float voltage) throw(IOException)
{
  char buf[256];
  memset((void*) buf, (int)'\0', sizeof(buf));
  //  int nch = 100*(slot-1) + (channel-1);
  int nch = 100 * (slot - 1) + (0); //.u0
  // koko
  sprintf(buf, "snmpset -v 2c -m +WIENER-CRATE-MIB -c guru 192.168.0.21 outputVoltageRiseRate.u%d F %f", nch, voltage);
  //  sprintf(buf,"snmpset -Oqv -v 2c -m +WIENER-CRATE-MIB -c guru 192.168.0.21 outputVoltageRiseRate.u%d F %f", nch, voltage);
  LogFile::info("%s", buf);
  system(buf);
}

void CdcHVControlCallback::setRampDownSpeed(int crate, int slot, int channel, float voltage) throw(IOException)
{
  char buf[256];
  memset((void*) buf, (int)'\0', sizeof(buf));
  //  int nch = 100*(slot-1) + (channel-1);
  int nch = 100 * (slot - 1) + (0);
  // koko
  sprintf(buf, "snmpset -v 2c -m +WIENER-CRATE-MIB -c guru 192.168.0.21 outputVoltageFallRate.u%d F %f", nch, voltage);
  //  sprintf(buf,"snmpset -Oqv -v 2c -m +WIENER-CRATE-MIB -c guru 192.168.0.21 outputVoltageFallRate.u%d F %f", nch, voltage);
  LogFile::info("%s", buf);
  system(buf);
}

void CdcHVControlCallback::setVoltageDemand(int crate, int slot, int channel, float voltage) throw(IOException)
{
  // set
  char buf[256];
  memset((void*) buf, (int)'\0', sizeof(buf));

  int nch = 100 * (slot - 1) + (channel - 1);
  sprintf(buf, "snmpset -v 2c -m +WIENER-CRATE-MIB -c guru 192.168.0.21 outputVoltage.u%d F %f", nch, voltage);
  LogFile::info("%s", buf);
  system(buf);

  // get and check
  memset((void*) buf, (int)'\0', sizeof(buf));
  sprintf(buf, "snmpget -Oqv -v 2c -m +WIENER-CRATE-MIB -c public 192.168.0.21 outputVoltage.u%d", nch);
  LogFile::info("%s", buf);

  FILE* fp;
  fp = popen(buf, "r");
  float v = 0;
  while (fgets(buf, sizeof(buf), fp) != NULL) {
    sscanf(buf, "%fV\n", &v);
    LogFile::info("%s", buf);
  }
  pclose(fp);
}

void CdcHVControlCallback::setVoltageLimit(int crate, int slot, int channel, float voltage) throw(IOException)
{
  // ???
  LogFile::info("setvoltagelimit called : crate = %d, slot = %d, channel = %d, voltage: %f",
                crate, slot, channel, voltage);
}

void CdcHVControlCallback::setCurrentLimit(int crate, int slot, int channel, float current) throw(IOException)
{
  char buf[256];
  memset((void*) buf, (int)'\0', sizeof(buf));
  int nch = 100 * (slot - 1) + (channel - 1);
  sprintf(buf, "snmpset -v 2c -m +WIENER-CRATE-MIB -c guru 192.168.0.21 outputCurrent.u%d F %f", nch, current);
  LogFile::info("%s", buf);
  system(buf);
}

float CdcHVControlCallback::getRampUpSpeed(int crate, int slot, int channel) throw(IOException)
{
  // koko
  char buf[256];
  memset((void*) buf, (int)'\0', sizeof(buf));

  int nch = 100 * (slot - 1) + (channel - 1);
  sprintf(buf, "snmpget -Oqv -v 2c -m +WIENER-CRATE-MIB -c public 192.168.0.21 outputVoltageRiseRate.u%d", nch);
  LogFile::info("%s", buf);

  FILE* fp;
  fp = popen(buf, "r");
  float v = 0.;
  while (fgets(buf, sizeof(buf), fp) != NULL) {
    sscanf(buf, "%fV", &v);
    LogFile::info("%s", buf);
  }
  pclose(fp);

  return v;
}

float CdcHVControlCallback::getRampDownSpeed(int crate, int slot, int channel) throw(IOException)
{

  char buf[256];
  memset((void*) buf, (int)'\0', sizeof(buf));
  int nch = 100 * (slot - 1) + (channel - 1);
  sprintf(buf, "snmpget -Oqv -v 2c -m +WIENER-CRATE-MIB -c public 192.168.0.21 outputVoltageFallRate.u%d", nch);
  LogFile::info("%s", buf);

  FILE* fp;
  fp = popen(buf, "r");
  float v = 0;
  while (fgets(buf, sizeof(buf), fp) != NULL) {
    sscanf(buf, "%fV", &v);
    LogFile::info("%s", buf);
  }
  pclose(fp);

  return v;
}

float CdcHVControlCallback::getVoltageDemand(int crate, int slot, int channel) throw(IOException)
{
  char buf[256];
  memset((void*) buf, (int)'\0', sizeof(buf));
  int nch = 100 * (slot - 1) + (channel - 1);
  sprintf(buf, "snmpget -Oqv -v 2c -m +WIENER-CRATE-MIB -c public 192.168.0.21 outputVoltage.u%d", nch);
  LogFile::info("%s", buf);

  FILE* fp;
  fp = popen(buf, "r");
  float v = 0;
  while (fgets(buf, sizeof(buf), fp) != NULL) {
    sscanf(buf, "%fV", &v);
    LogFile::info("%s", buf);
  }
  pclose(fp);

  return v;
}

float CdcHVControlCallback::getVoltageLimit(int crate, int slot, int channel) throw(IOException)
{
  //??
  //  return getConfig().getChannel(crate, slot, channel).getVoltageLimit();
  return 510.;
}

float CdcHVControlCallback::getCurrentLimit(int crate, int slot, int channel) throw(IOException)
{
  char buf[256];
  memset((void*) buf, (int)'\0', sizeof(buf));
  int nch = 100 * (slot - 1) + (channel - 1);
  sprintf(buf, "snmpget -Oqv -v 2c -m +WIENER-CRATE-MIB -c public 192.168.0.21 outputCurrent.u%d", nch);
  LogFile::info("%s", buf);

  FILE* fp;
  fp = popen(buf, "r");
  float v = 0;
  while (fgets(buf, sizeof(buf), fp) != NULL) {
    sscanf(buf, "%fV", &v);
    LogFile::info("%s", buf);
  }
  pclose(fp);
  return v * 1000000.;
}

float CdcHVControlCallback::getVoltageMonitor(int crate, int slot, int channel) throw(IOException)
{
  // koko
  char buf[256];
  memset((void*) buf, (int)'\0', sizeof(buf));

  int nch = 100 * (slot - 1) + (channel - 1);
  sprintf(buf, "snmpget -Oqv -v 2c -m +WIENER-CRATE-MIB -c public 192.168.0.21 outputMeasurementSenseVoltage.u%d", nch);
  LogFile::info("%s", buf);

  FILE* fp;
  fp = popen(buf, "r");
  float v = 0;
  while (fgets(buf, sizeof(buf), fp) != NULL) {
    sscanf(buf, "%fV", &v);
    LogFile::info("%s", buf);
  }
  pclose(fp);
  return v;
}

float CdcHVControlCallback::getCurrentMonitor(int crate, int slot, int channel) throw(IOException)
{

  char buf[256];
  memset((void*) buf, (int)'\0', sizeof(buf));

  int nch = 100 * (slot - 1) + (channel - 1);
  sprintf(buf, "snmpget -Oqv -v 2c -m +WIENER-CRATE-MIB -c public 192.168.0.21 outputMeasurementCurrent.u%d", nch);
  LogFile::info("%s", buf);

  FILE* fp;
  fp = popen(buf, "r");
  float v = 0;
  while (fgets(buf, sizeof(buf), fp) != NULL) {
    sscanf(buf, "%fV", &v);
    LogFile::info("%s", buf);
  }
  pclose(fp);

  return v * 1000000.;
}

bool CdcHVControlCallback::getSwitch(int crate, int slot, int channel) throw(IOException)
{
  char buf[256];
  memset((void*) buf, (int)'\0', sizeof(buf));
  int nch = 100 * (slot - 1) + (channel - 1);
  // get switch on/off
  sprintf(buf, "snmpget -Oqv -v 2c -m +WIENER-CRATE-MIB -c guru 192.168.0.21 outputSwitch.u%d", nch);
  LogFile::info("%s", buf);
  FILE* fp;
  fp = popen(buf, "r");
  while (fgets(buf, sizeof(buf), fp) != NULL) {
    std::string tmp = std::string(buf);
    LogFile::info("%s", buf);
    if (strncmp(tmp.c_str(), "on", 2) == 0) {
      pclose(fp);
      return true;
    } else if (strncmp(tmp.c_str(), "off", 3) == 0) {
      pclose(fp);
      return false;
    }
  }
  pclose(fp);
  return false;
}

int CdcHVControlCallback::getState(int crate, int slot, int channel) throw(IOException)
{
  //  return HVMessage::OCP;
  char buf[256];
  memset((void*) buf, (int)'\0', sizeof(buf));
  int nch = 100 * (slot - 1) + (channel - 1);
  // get switch on/off
  sprintf(buf, "snmpget -Oqv -v 2c -m +WIENER-CRATE-MIB -c guru 192.168.0.21 outputSwitch.u%d", nch);
  LogFile::info("%s", buf);
  FILE* fp;
  fp = popen(buf, "r");
  while (fgets(buf, sizeof(buf), fp) != NULL) {
    std::string tmp = std::string(buf);
    LogFile::info("%s", buf);
    if (strncmp(tmp.c_str(), "on", 2) == 0) {
      pclose(fp);
      return HVMessage::ON;
    } else if (strncmp(tmp.c_str(), "off", 3) == 0) {
      pclose(fp);
      return HVMessage::OFF;
    }
  }
  pclose(fp);
  return HVMessage::ERR;
}

