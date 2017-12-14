#include "daq/slc/copper/cdc/CDCFEEHandler.h"

#include <daq/slc/runcontrol/RCCallback.h>

#include <daq/slc/copper/FEE.h>
#include <daq/slc/copper/HSLB.h>

using namespace Belle2;

bool CDCDateHandler::handleGetText(std::string& val)
{
  try {
    int ival = m_hslb.readfee32(0x0010);
    int year = (ival >> 24) & 0xFF;
    int month = (ival >> 16) & 0xFF;
    int day = (ival >> 8) & 0xFF;
    int revision = ival & 0xFF;
    val = StringUtil::form("synthesized date = %d/%02d/%02d (revision=%d)",
                           year, month, day, revision);
    LogFile::info(val);
  } catch (const IOException& e) {
    LogFile::error(e.what());
  }
  return true;
}

bool CDCFirmwareHandler::handleGetInt(int& val)
{
  try {
    val = m_hslb.readfee32(0x0011) & 0xFF;
    LogFile::info("CDC FEE Firmware version : %d", val);
  } catch (const IOException& e) {
    LogFile::error(e.what());
  }
  return true;
}

bool CDCDataFormatHandler::handleGetText(std::string& val)
{
  try {
    int ival = (m_hslb.readfee32(0x0012) >> 24) & 0x3;
    switch (ival) {
      case 0x1: val = "raw"; break;
      case 0x2: val = "suppress"; break;
      case 0x3: val = "raw-suppress"; break;
      default: val = "no readout"; break;
    }
    LogFile::info("CDC FEE Data format : " + val);
  } catch (const IOException& e) {
    LogFile::error(e.what());
  }
  return true;
}

bool CDCWindowHandler::handleGetInt(int& val)
{
  try {
    val = (m_hslb.readfee32(0x0012) >> 8) & 0xFF;
    LogFile::info("CDC FEE Window : %d", val);
  } catch (const IOException& e) {
    LogFile::error(e.what());
  }
  return true;
}

bool CDCDelayHandler::handleGetInt(int& val)
{
  try {
    val = (m_hslb.readfee32(0x0012)) & 0xFF;
    LogFile::info("CDC FEE Trigger Delay : %d", val);
  } catch (const IOException& e) {
    LogFile::error(e.what());
  }
  return true;
}

bool CDCADCThresholdHandler::handleGetInt(int& val)
{
  try {
    val = (m_hslb.readfee32(0x0013)) & 0xFFFF;
    LogFile::info("CDC FEE ADC threshold : %d", val);
  } catch (const IOException& e) {
    LogFile::error(e.what());
  }
  return true;
}

bool CDCTDCThresholdHandler::handleGetInt(int& val)
{
  try {
    val = (m_hslb.readfee32(0x0015)) & 0xFFFF;
    LogFile::info("CDC FEE ADC threshold : %d", val);
  } catch (const IOException& e) {
    LogFile::error(e.what());
  }
  return true;
}

bool CDCIndirectADCAccessHandler::handleGetInt(int& val)
{
  try {
    val = 0;
    //val = (m_hslb.readfee32(0x0014)) & 0xFF;
    //LogFile::info("CDC FEE ADC threshold : %d", val);
  } catch (const IOException& e) {
    LogFile::error(e.what());
  }
  return true;
}

bool CDCDACControlHandler::handleGetInt(int& val)
{
  try {
    val = (m_hslb.readfee32(0x0015)) & (0xFFFF >> 4);
    LogFile::info("CDC FEE DAQ control : %d", val);
  } catch (const IOException& e) {
    LogFile::error(e.what());
  }
  return true;
}

bool CDCIndirectMonitorAccessHandler::handleGetInt(int& val)
{
  try {
    val = 0;
    //val = (m_hslb.readfee32(0x0014)) & 0xFF;
    //LogFile::info("CDC FEE ADC threshold : %d", val);
  } catch (const IOException& e) {
    LogFile::error(e.what());
  }
  return true;
}

bool CDCPedestalHandler::handleGetInt(int& val)
{
  try {
    int i = m_index / 2;
    val = m_hslb.readfee32(0x0020 + i);
    if (m_index % 2 == 0) {
      val = val & (0xFFFF >> 6);
    } else {
      val = (val >> 16) & (0xFFFF >> 6);
    }
    LogFile::info("CDC FEE Pedestal[%d] : %d", m_index, val);
  } catch (const IOException& e) {
    LogFile::error(e.what());
  }
  return true;
}

