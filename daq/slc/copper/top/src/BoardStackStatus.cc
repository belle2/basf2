#include "daq/slc/copper/top/BoardStackStatus.h"
#include "daq/slc/copper/top/B2L_common.h"
#include "daq/slc/copper/top/B2L_defs.h"

#include <daq/slc/copper/HSLB.h>
#include <daq/slc/runcontrol/RCCallback.h>
#include <daq/slc/nsm/NSMVHandler.h>
#include <daq/slc/system/LogFile.h>
#include <daq/slc/base/StringUtil.h>

#include <unistd.h>

using namespace Belle2;
using namespace std;

const float vRAW_low_margin = 0.00;
const float vRAW_high_margin = 0.275;
const float vRAW1_nominal = 1.765;
const float vRAW2_nominal = 3.099;
const float vRAW3_nominal = 4.424;
const float fpgaTempHigh = 85;
const float walltempHigh = 80;
const float asictempHigh = 80;

BoardStackStatus::BoardStackStatus()
{
  m_nCarriers = 0;
}

BoardStackStatus::BoardStackStatus(const unsigned nCarriers)
{
  SetNumberOfCarriers(nCarriers);
}

void BoardStackStatus::SetNumberOfCarriers(const unsigned nCarriers)
{
  m_boardstackObservables.SetNumberOfCarriers(nCarriers);
  m_nCarriers = nCarriers;
}

void BoardStackStatus::InitNSMCallbacks(Belle2::HSLB& hslb, Belle2::RCCallback& callback)
{
  InitNSMCallbacksSCROD(hslb, callback);
  for (unsigned carrier = 0; carrier < m_nCarriers; ++carrier) {
    InitNSMCallbacksCarrier(hslb, callback, carrier);
  }
}

bool BoardStackStatus::UpdateNSMCallbacks(Belle2::HSLB& hslb, Belle2::RCCallback& callback)
{
  UpdateARMLiveCounters(hslb, callback);
  ReadSCRODObervables(hslb);
  UpdateNSMCallbacksSCROD(hslb, callback);
  for (unsigned carrier = 0; carrier < m_nCarriers; ++carrier) {
    ReadCarrierObservables(hslb, carrier);
    UpdateNSMCallbacksCarrier(hslb, callback, carrier);
  }

  bool boardstackIsGood = IsBoardstackGood();
  std::string vname = StringUtil::form("top[%d].scrod.", hslb.get_finid());
  if (boardstackIsGood) {
    callback.set(vname + "statusOK", 1);
    return true;
  } else {
    callback.set(vname + "statusOK", 0);
    return false;
  }
}

bool BoardStackStatus::IsBoardstackGood()
{
  //check temperatures
  if (m_boardstackObservables.sDieTemperature > fpgaTempHigh || m_boardstackObservables.sWallTemperature > walltempHigh) return false;
  for (unsigned i = 0; i < m_nCarriers; ++i) {
    if (m_boardstackObservables.cDieTemperature[i] > fpgaTempHigh
        || m_boardstackObservables.cWallTemperature[i] > walltempHigh) return false;
    if (m_boardstackObservables.cASIC01Temperature[i] > asictempHigh
        || m_boardstackObservables.cASIC23Temperature[i] > asictempHigh) return false;
  }
  //check raw voltages
  if (vRAW1_nominal + vRAW_high_margin < m_boardstackObservables.sVoltageRaw1
      || m_boardstackObservables.sVoltageRaw1 < vRAW1_nominal - vRAW_low_margin) {
    return false;
  }
  if (vRAW2_nominal + vRAW_high_margin < m_boardstackObservables.sVoltageRaw2
      || m_boardstackObservables.sVoltageRaw2 < vRAW2_nominal - vRAW_low_margin) {
    return false;
  }
  if (vRAW3_nominal + vRAW_high_margin < m_boardstackObservables.sVoltageRaw3
      || m_boardstackObservables.sVoltageRaw3 < vRAW3_nominal - vRAW_low_margin) {
    return false;
  }
  //check ps live
  if (m_boardstackObservables.sARMState.compare("Running") != 0) return false;
  for (unsigned i = 0; i < m_nCarriers; ++i) {
    if (m_boardstackObservables.cARMState[i].compare("Running") != 0) return false;
  }
  return true;
}

void BoardStackStatus::PrintBoardstackStatus(Belle2::HSLB& hslb)
{
  RCCallback dummyCallback;
  UpdateARMLiveCounters(hslb, dummyCallback);
  ReadSCRODObervables(hslb);
  PrintSCRODStatus();
  for (unsigned carrier = 0; carrier < m_nCarriers; ++carrier) {
    ReadCarrierObservables(hslb, carrier);
    PrintCarrierStatus(carrier);
  }
}

void BoardStackStatus::InitNSMCallbacksSCROD(Belle2::HSLB& hslb, Belle2::RCCallback& callback)
{
  std::string vname = StringUtil::form("top[%d].scrod.", hslb.get_finid());
  callback.add(new NSMVHandlerInt(vname + "scrod", true, false, 0));
  callback.add(new NSMVHandlerInt(vname + "fwVersion", true, false, 0));
  callback.add(new NSMVHandlerInt(vname + "elfVersion", true, false, 0));
  callback.add(new NSMVHandlerInt(vname + "trigMask", true, false, 0));
  callback.add(new NSMVHandlerInt(vname + "SCRODFeModeGet", true, false, 0));
  callback.add(new NSMVHandlerFloat(vname + "tempFPGA", true, false, 0));
  callback.add(new NSMVHandlerFloat(vname + "vRAW1", true, false, 0));
  callback.add(new NSMVHandlerFloat(vname + "vRAW2", true, false, 0));
  callback.add(new NSMVHandlerFloat(vname + "vRAW3", true, false, 0));
  callback.add(new NSMVHandlerText(vname + "live", true, false, ""));
  callback.add(new NSMVHandlerFloat(vname + "tempWall", true, false, 0));
  callback.add(new NSMVHandlerFloat(vname + "humidityTemp", true, false, 0));
  callback.add(new NSMVHandlerFloat(vname + "humidity", true, false, 0));
  callback.add(new NSMVHandlerInt(vname + "statusOK", true, false, 0));
}

void BoardStackStatus::InitNSMCallbacksCarrier(Belle2::HSLB& hslb, Belle2::RCCallback& callback,
                                               const unsigned carrier)
{
  std::string vname = StringUtil::form("top[%d].carrier[%d].", hslb.get_finid(), carrier);
  callback.add(new NSMVHandlerInt(vname + "fwVersion", true, false, 0));
  callback.add(new NSMVHandlerInt(vname + "elfVersion", true, false, 0));
  callback.add(new NSMVHandlerInt(vname + "trigMask", true, false, 0));
  callback.add(new NSMVHandlerInt(vname + "temp2", true, false, 0));
  callback.add(new NSMVHandlerFloat(vname + "tempFPGA", true, false, 0));
  callback.add(new NSMVHandlerText(vname + "live", true, false, ""));
  callback.add(new NSMVHandlerFloat(vname + "tempWall", true, false, 0));
  callback.add(new NSMVHandlerFloat(vname + "tempASIC01", true, false, 0));
  callback.add(new NSMVHandlerFloat(vname + "tempASIC23", true, false, 0));
  callback.add(new NSMVHandlerFloat(vname + "lookbackGet", true, false, 0));
}

void BoardStackStatus::UpdateNSMCallbacksSCROD(Belle2::HSLB& hslb, Belle2::RCCallback& callback)
{
  // create a dictionary of values to return
  std::string vname = StringUtil::form("top[%d].scrod.", hslb.get_finid());
  callback.set(vname + "scrod", m_boardstackObservables.scrodID);
  callback.set(vname + "fwVersion", m_boardstackObservables.sFirmwareVersion);
  callback.set(vname + "elfVersion", m_boardstackObservables.sSoftwareVersion);
  callback.set(vname + "trigMask", m_boardstackObservables.sTriggerMask);

  // on-FPGA XADC readings
  callback.set(vname + "tempFPGA", m_boardstackObservables.sDieTemperature);
  callback.set(vname + "vRAW1", m_boardstackObservables.sVoltageRaw1);
  callback.set(vname + "vRAW2", m_boardstackObservables.sVoltageRaw2);
  callback.set(vname + "vRAW3", m_boardstackObservables.sVoltageRaw3);
  callback.set(vname + "live", m_boardstackObservables.sARMState);
  // external sensor readings
  callback.set(vname + "tempWall", m_boardstackObservables.sWallTemperature);
  callback.set(vname + "humidity", m_boardstackObservables.sHumidity);
  callback.set(vname + "humidityTemp", m_boardstackObservables.sHumidityTemperature);

  callback.set(vname + "SCRODFeModeGet", m_boardstackObservables.sFEMode);

  std::string str = StringUtil::form("(BS: %d, SCROD: %d)", hslb.get_finid(), m_boardstackObservables.scrodID / 16);
  if (vRAW1_nominal + vRAW_high_margin < m_boardstackObservables.sVoltageRaw1) {
    callback.log(LogFile::WARNING, "RAW1 voltage unnecessarily high " + str);
  }
  if (m_boardstackObservables.sVoltageRaw1 < vRAW1_nominal - vRAW_low_margin) {
    callback.log(LogFile::WARNING, "RAW1 voltage too low! " + str);
  }
  if (vRAW2_nominal + vRAW_high_margin < m_boardstackObservables.sVoltageRaw2) {
    callback.log(LogFile::WARNING, "RAW2 voltage unnecessarily high " + str);
  }
  if (m_boardstackObservables.sVoltageRaw2 < vRAW2_nominal - vRAW_low_margin) {
    callback.log(LogFile::WARNING, "RAW2 voltage too low! " + str);
  }
  if (vRAW3_nominal + vRAW_high_margin < m_boardstackObservables.sVoltageRaw3) {
    callback.log(LogFile::WARNING, "RAW3 voltage unnecessarily high " + str);
  }
  if (m_boardstackObservables.sVoltageRaw3 < vRAW3_nominal - vRAW_low_margin) {
    callback.log(LogFile::WARNING, "RAW3 voltage too low!" + str);
  }
}

void BoardStackStatus::UpdateNSMCallbacksCarrier(Belle2::HSLB& hslb, Belle2::RCCallback& callback, const unsigned carrier)
{
  std::string vname = StringUtil::form("top[%d].carrier[%d].", hslb.get_finid(), carrier);
  callback.set(vname + "fwVersion", m_boardstackObservables.cFirmwareVersion.at(carrier));
  callback.set(vname + "elfVersion", m_boardstackObservables.cSoftwareVersion.at(carrier));
  callback.set(vname + "trigMask", m_boardstackObservables.cTriggerMask.at(carrier));
  // on-FPGA XADC readings
  callback.set(vname + "tempFPGA", m_boardstackObservables.cDieTemperature.at(carrier));
  callback.set(vname + "live", m_boardstackObservables.cARMState.at(carrier));
  // external sensor readings
  callback.set(vname + "tempWall", m_boardstackObservables.cWallTemperature.at(carrier));
  callback.set(vname + "tempASIC01", m_boardstackObservables.cASIC01Temperature.at(carrier));
  callback.set(vname + "tempASIC23", m_boardstackObservables.cASIC23Temperature.at(carrier));
  //lookback window setting
  callback.set(vname + "lookbackGet", Read_Register(hslb, CARRIER_IRSX_readoutLookback, carrier, 0));
}

void BoardStackStatus::PrintSCRODStatus()
{
  if (vRAW1_nominal + vRAW_high_margin < m_boardstackObservables.sVoltageRaw1) {
    cout << "Warning: RAW1 voltage unnecessarily high " << m_boardstackObservables.sVoltageRaw1 << endl;
  }
  if (m_boardstackObservables.sVoltageRaw1 < vRAW1_nominal - vRAW_low_margin) {
    cout << "ERROR: RAW1 voltage too low!" << endl;
  }
  if (vRAW2_nominal + vRAW_high_margin < m_boardstackObservables.sVoltageRaw2) {
    cout << "Warning: RAW2 voltage unnecessarily high " << m_boardstackObservables.sVoltageRaw2 << endl;
  }
  if (m_boardstackObservables.sVoltageRaw2 < vRAW2_nominal - vRAW_low_margin) {
    cout << "ERROR: RAW2 voltage too low!" << endl;
  }
  if (vRAW3_nominal + vRAW_high_margin < m_boardstackObservables.sVoltageRaw3) {
    cout << "Warning: RAW3 voltage unnecessarily high " << m_boardstackObservables.sVoltageRaw3 << endl;
  }
  if (m_boardstackObservables.sVoltageRaw3 < vRAW3_nominal - vRAW_low_margin) {
    cout << "ERROR: RAW3 voltage too low!" << endl;
  }
  printf("           FPGA_ver ARM_Ver  trigMask DieTemp WallTemp -----other------   ARM Stat\n");
  printf("    SCROD  %08x %08x %08x %5.1fC %5.1fC     %5.1fC %5.1f%% RH   %s \n", m_boardstackObservables.sFirmwareVersion,
         m_boardstackObservables.sSoftwareVersion, m_boardstackObservables.sTriggerMask, m_boardstackObservables.sDieTemperature,
         m_boardstackObservables.sWallTemperature, m_boardstackObservables.sHumidityTemperature, m_boardstackObservables.sHumidity,
         m_boardstackObservables.sARMState.c_str());
}


void BoardStackStatus::PrintCarrierStatus(const unsigned carrier)
{
  if (m_boardstackObservables.cPGPup.at(carrier) != 0x1f) {
    cout << "high performance link from scrod to carrier " << carrier << " is down" << endl;
    return ;
  }
  printf("Carrier %d  %08x %08x %08x %5.1fC %5.1fC     %5.1fC %5.1fC      %s\n", carrier,
         m_boardstackObservables.cFirmwareVersion.at(carrier), m_boardstackObservables.cSoftwareVersion.at(carrier),
         m_boardstackObservables.cTriggerMask.at(carrier), m_boardstackObservables.cDieTemperature.at(carrier),
         m_boardstackObservables.cWallTemperature.at(carrier), m_boardstackObservables.cASIC01Temperature.at(carrier),
         m_boardstackObservables.cASIC23Temperature.at(carrier), m_boardstackObservables.cARMState.at(carrier).c_str());
}

void BoardStackStatus::ReadSCRODObervables(Belle2::HSLB& hslb)
{
  m_boardstackObservables.scrodID = Read_Register(hslb, SCROD_AxiVersion_UserID);
  m_boardstackObservables.sFirmwareVersion  = Read_Register(hslb, SCROD_AxiVersion_FpgaVersion);
  m_boardstackObservables.sSoftwareVersion = Read_Register(hslb, SCROD_PS_elfVersion);
  m_boardstackObservables.sTriggerMask = Read_Register(hslb, SCROD_AxiCommon_trigMask);
  m_boardstackObservables.sDieTemperature = convert_FPGA_temp(Read_Register(hslb, SCROD_XADC_Temperature));
  m_boardstackObservables.sWallTemperature = convert_STTS751_temp(Read_Register(hslb, SCROD_PS_WallTemp));
  m_boardstackObservables.sHumidityTemperature = convert_humidity_temp(Read_Register(hslb, SCROD_PS_temperature_of_humidity_sensor));
  m_boardstackObservables.sHumidity = convert_humidity(Read_Register(hslb, SCROD_PS_relative_humidity));
  m_boardstackObservables.sVoltageRaw1 = convert_FPGA_voltage_from_external_voltage_divider(Read_Register(hslb, SCROD_XADC_V_RAW1));
  m_boardstackObservables.sVoltageRaw2 = convert_FPGA_voltage_from_external_voltage_divider(Read_Register(hslb, SCROD_XADC_V_RAW2));
  m_boardstackObservables.sVoltageRaw3 = convert_FPGA_voltage_from_external_voltage_divider(Read_Register(hslb, SCROD_XADC_V_RAW3));
  m_boardstackObservables.sFEMode = Read_Register(hslb, SCROD_PS_featureExtMode);
}

void BoardStackStatus::ReadCarrierObservables(Belle2::HSLB& hslb, const unsigned carrier)
{
  m_boardstackObservables.cPGPup.at(carrier) = Read_Register(hslb, PGP_Status + carrier * 0x200);
  m_boardstackObservables.cFirmwareVersion.at(carrier) = Read_Register(hslb, CARRIER_AxiVersion_FpgaVersion, carrier);
  m_boardstackObservables.cSoftwareVersion.at(carrier) = Read_Register(hslb, CARRIER_PS_elfversion, carrier);
  m_boardstackObservables.cTriggerMask.at(carrier) = Read_Register(hslb, CARRIER_AxiCommon_trigMask, carrier);
  m_boardstackObservables.cDieTemperature.at(carrier) = convert_FPGA_temp(Read_Register(hslb, CARRIER_XADC_Temperature, carrier));
  m_boardstackObservables.cWallTemperature.at(carrier) = convert_STTS751_temp(Read_Register(hslb, CARRIER_PS_WallTemp, carrier));
  m_boardstackObservables.cASIC01Temperature.at(carrier) = convert_STTS751_temp(Read_Register(hslb, CARRIER_PS_ASIC01Temp, carrier));
  m_boardstackObservables.cASIC23Temperature.at(carrier) = convert_STTS751_temp(Read_Register(hslb, CARRIER_PS_ASIC23Temp, carrier));
}

void BoardStackStatus::UpdateARMLiveCounters(Belle2::HSLB& hslb, Belle2::RCCallback& callback)
{
  vector<int> sliveCounterCarriers;
  int sliveCounter1 = Read_Register(hslb, SCROD_PS_liveCounter);
  for (unsigned i = 0; i < m_nCarriers; ++i) {
    int liveCounter = Read_Register(hslb, CARRIER_PS_livecounter, i);
    if ((unsigned) liveCounter != 0xDEADBEEF) sliveCounterCarriers.push_back(liveCounter);
    else sliveCounterCarriers.push_back(-1);
  }

  callback.wait(0.5);

  int sliveCounter2 = Read_Register(hslb, SCROD_PS_liveCounter);
  if (sliveCounter1 != sliveCounter2) m_boardstackObservables.sARMState = "Running";
  else m_boardstackObservables.sARMState = "Stopped";
  for (unsigned i = 0; i < m_nCarriers; ++i) {
    int liveCounter = Read_Register(hslb, CARRIER_PS_livecounter, i);
    if ((unsigned) liveCounter == 0xDEADBEEF) m_boardstackObservables.cARMState.at(i) = "Unknown";
    else if (sliveCounterCarriers.at(i) != liveCounter) m_boardstackObservables.cARMState.at(i) = "Running";
    else m_boardstackObservables.cARMState.at(i) = "Stopped";
  }
}
