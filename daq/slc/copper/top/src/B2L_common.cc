#include "daq/slc/copper/top/B2L_common.h"
#include "daq/slc/copper/top/B2L_defs.h"
#include "daq/slc/copper/top/ErrorStats.h"

#include "daq/slc/copper/HSLB.h"
#include <mgt/hsreg.h>

#include <algorithm>
#include <stdio.h>
#include <unistd.h>
#include <iostream>
#include <sstream>

const int FEEDBEES = 0xfeedbee5;
const int TIMEOUT = 0xa16d32ff;
const int DEADBEEF = 0xdeadbeef;

using namespace std;
std::map<std::string, std::vector<int> > hslb_status_map;
string hslb_list[4] = {"a", "b", "c", "d"};

inline string NumberToHexString(const int number)
{
  stringstream ss;
  ss << hex << number;
  return ss.str();
}

/******************************* Register access function ******************************/
int B2L_Address_Carrier_Asic(const int addr, const int carrier, const int asic)
{
  return addr + carrier * 0x2000 + asic * 0x400;
}


/******************************* Read Register Functions ******************************/
int Read_Register(const int addr, const int scrod, const int carrier, const int asic)
{
  int status = -1;
  Belle2::HSLB hslb;
  try {
    status = hslb.open(scrod);
    if (status > 0) {
      errorStats.IncrementTransactionCount("read; hslb " + hslb_list[scrod]);
      int registerAddr = B2L_Address_Carrier_Asic(addr, carrier, asic);
      status = Raw_Read_Register(hslb, registerAddr);
    }
    hslb.close();
  } catch (...) {}
  return status;
}

int Read_Register(Belle2::HSLB& hslb, const int addr, const int carrier, const int asic)
{
  errorStats.IncrementTransactionCount("read; hslb " + hslb_list[hslb.get_finid()]);
  int registerAddr = B2L_Address_Carrier_Asic(addr, carrier, asic);
  return Raw_Read_Register(hslb, registerAddr);
}

int Raw_Read_Register(Belle2::HSLB& hslb, const int addr, const int max_tries)
{
  int scrod = hslb.get_finid();
  for (int i = 0; i < max_tries; ++i) {
    hslb.writefn32(HSREGL_RESET, 0x100);
    int value = hslb.readfee32(addr);
    if (value == DEADBEEF || value == TIMEOUT) {
      errorStats.IncrementErrorCount("total read retries; hslb " + hslb_list[scrod]);
      errorStats.IncrementErrorCount("busy; hslb " + hslb_list[scrod]);
    } else {
      return value;
    }
  }
  errorStats.IncrementErrorCount("read; hslb " + hslb_list[scrod]);
  return FEEDBEES;
}

/****************************** Write Register Functions ******************************/
int Raw_Write_Register(Belle2::HSLB& hslb, const int addr, const int value, const int max_tries)
{
  int scrod = hslb.get_finid();
  for (int i = 0; i < max_tries; ++i) {
    hslb.writefn32(HSREGL_RESET, 0x100);
    hslb.writefee32(addr, value);
    usleep(3000);
    //readback written value
    int rbValue = hslb.readfee32(addr);
    if (rbValue == value) {
      return 0;
    } else {
      cout << endl;
      cout << "!!!!!Readback error on non-asic register: " << addr << endl;
      cout << endl;
      errorStats.IncrementErrorCount("total write retries; hslb " + hslb_list[scrod]);
    }
  }
  errorStats.IncrementErrorCount("write; hslb " + hslb_list[scrod]);
  return -1;
}

int Raw_Write_Register_noReadBack(Belle2::HSLB& hslb, const int addr, const int value)
{
  hslb.writefn32(HSREGL_RESET, 0x100);
  hslb.writefee32(addr, value);
  usleep(3000);
  return 0;
}

int Write_Register(const int addr, const int value, const int scrod, const int carrier, const int asic)
{
  int status = -1;
  Belle2::HSLB hslb;
  try {
    status = hslb.open(scrod);
    if (status > 0) {
      errorStats.IncrementTransactionCount("write; hslb " + hslb_list[scrod]);
      int registerAddr = B2L_Address_Carrier_Asic(addr, carrier, asic);
      status = Raw_Write_Register(hslb, registerAddr, value);
    }
    hslb.close();
  } catch (...) {}
  return status;
}

int Write_Register(Belle2::HSLB& hslb, const int addr, const int value, const int carrier, const int asic)
{
  int scrod = hslb.get_finid();
  errorStats.IncrementTransactionCount("write; hslb " + hslb_list[scrod]);
  int registerAddr = B2L_Address_Carrier_Asic(addr, carrier, asic);
  return Raw_Write_Register(hslb, registerAddr, value);
}

int Write_Asic_Direct_Register(const int scrod, const int carrier, const int asic, const int addr, const int value)
{
  int status = -1;
  Belle2::HSLB hslb;
  try {
    status = hslb.open(scrod);
    if (status > 0) {
      status = Write_Asic_Direct_Register(hslb, carrier, asic, addr, value);
    }
    hslb.close();
  } catch (...) {}
  return status;
}

int Write_Asic_Direct_Register(Belle2::HSLB& hslb, const int carrier, const int asic, const int addr, const int value)
{
  int scrod = hslb.get_finid();
  errorStats.IncrementTransactionCount("write; hslb " + hslb_list[scrod]);
  int word = (addr << 12) | value;
  int write_addr = B2L_Address_Carrier_Asic(CARRIER_IRSX_irsxDirect, carrier, asic);
  int read_addr = B2L_Address_Carrier_Asic(CARRIER_IRSX_irsxRegWordOut, carrier, asic);
  Raw_Write_Register_noReadBack(hslb, write_addr, word);
  Raw_Write_Register_noReadBack(hslb, write_addr, word);//shift out written value for readback
  Intermediate_Write_Asic_Direct_Register(hslb, write_addr, read_addr, word);
  return 0;
}

int Intermediate_Write_Asic_Direct_Register(Belle2::HSLB hslb, const int writeAddress, const int readAddr, const int value,
                                            const int retries)
{
  int scrod = hslb.get_finid();
  for (int i = 0; i < retries; ++i) {
    int readbackValue = Raw_Read_Register(hslb, readAddr);
    if (readbackValue != value) {
      int modifiedValue = (value & 0x7ffff) << 1 | (value & 0x80000) >> 19;
      if (modifiedValue == readbackValue) {
        errorStats.IncrementErrorCount("bitshift readback necessary for address " + NumberToHexString(
                                         writeAddress) + "; hslb " + hslb_list[scrod]);
        return 0;
      }
      cout << endl;
      cout << "!!!! readback error on asic address: " << writeAddress << endl;
      cout << endl;
      errorStats.IncrementErrorCount("readback; hslb " + hslb_list[scrod]);
      Raw_Write_Register_noReadBack(hslb, writeAddress, value);
      Raw_Write_Register_noReadBack(hslb, writeAddress, value);//shift out written value for readback
    } else {
      return 0;
    }
  }
  errorStats.IncrementErrorCount("write; hslb " + hslb_list[scrod]);
  return -1;
}

/******************************* Belle2 link status ******************************/
int GetNumberOfCarriers(const int scrod)
{
  int nCarriers = Read_Register(SCROD_PS_carriersDetected, scrod, 0, 0);
  if (nCarriers == 5) {
    sleep(1);
    nCarriers = Read_Register(SCROD_PS_carriersDetected, scrod, 0, 0);
  }
  if (nCarriers > 4) {
    printf("ERROR: number of reported carriers is %d/n", nCarriers);
    nCarriers = 0;
  }
  return nCarriers;
}

std::map<std::string, std::vector<int> > Get_Status_Of_all_Hslb_Boards()
{
  Belle2::HSLB hslb;
  hslb_status_map.clear();
  hslb_status_map.insert(pair<string, vector<int> > ("b2lup", vector<int>()));
  hslb_status_map.insert(pair<string, vector<int> > ("b2ldown", vector<int>()));
  hslb_status_map.insert(pair<string, vector<int> > ("notpresent", vector<int>()));
  for (int scrod = 0; scrod < 4; ++scrod) {
    try {
      hslb.open(scrod);
      hslb.monitor();
      if (hslb.isBelle2LinkDown()) hslb_status_map["b2ldown"].push_back(scrod);
      else hslb_status_map["b2lup"].push_back(scrod);
      hslb.close();
    } catch (...) {
      hslb_status_map["notpresent"].push_back(scrod);
    }
  }
  return hslb_status_map;
}

void Show_Status_Of_Hslb_Boards()
{
  std::map<std::string, std::vector<int> > map = Get_Status_Of_all_Hslb_Boards();
  std::vector<int>::iterator it;
  for (int i = 0; i < 4; ++i) {
    try {
      it = find(map["b2lup"].begin(), map["b2lup"].end(), i);
      if (it != map["b2lup"].end()) printf("b2l up for %d\n", i);
      it = find(map["b2ldown"].begin(), map["b2ldown"].end(), i);
      if (it != map["b2ldown"].end()) printf("b2l down for %d\n", i);
      it = find(map["notpresent"].begin(), map["notpresent"].end(), i);
      if (it != map["notpresent"].end()) printf("hslb %d not present\n", i);
    } catch (...) {}
  }
}

void Get_List_Of_Installed_Boardstacks_With_B2L_Up(int* list, const unsigned length)
{
  std::map<std::string, std::vector<int> > map = Get_Status_Of_all_Hslb_Boards();
  std::vector<int> vec = map["b2lup"];
  for (unsigned i = 0; i < vec.size(); ++i) {
    if (i < length) list[i] = vec[i];
    else break;
  }
}

void Get_List_Of_Installed_Boardstacks_With_B2L_Down(int* list, const unsigned length)
{
  std::map<std::string, std::vector<int> > map = Get_Status_Of_all_Hslb_Boards();
  std::vector<int> vec = map["b2ldown"];
  for (unsigned i = 0; i < vec.size(); ++i) {
    if (i < length) list[i] = vec[i];
    else break;
  }
}

int Get_Number_Of_Boardstacks_With_B2L_Up()
{
  int boardstacks[4] = { -1, -1, -1, -1};
  Get_List_Of_Installed_Boardstacks_With_B2L_Up(boardstacks, 4);
  int count = 0;
  for (int i = 0; i < 4; ++i) {
    if (boardstacks[i] > -1) count++;
  }
  return count;
}

int Get_Number_Of_Installed_HSLB_Boards()
{
  int boardstacks_up[4] = { -1, -1, -1, -1};
  int boardstacks_down[4] = { -1, -1, -1, -1};
  Get_List_Of_Installed_Boardstacks_With_B2L_Up(boardstacks_up, 4);
  Get_List_Of_Installed_Boardstacks_With_B2L_Down(boardstacks_down, 4);
  int count = 0;
  for (int i = 0; i < 4; ++i) {
    if (boardstacks_up[i] > -1 || boardstacks_down[i] > -1) count++;
  }
  return count;
}

int Is_B2L_Up(const unsigned scrod)
{
  if (scrod > 4) return -1;
  int boardstacks_up[4] = { -1, -1, -1, -1};
  Get_List_Of_Installed_Boardstacks_With_B2L_Up(boardstacks_up, 4);
  for (int i = 0; i < 4; ++i) {
    if ((int)scrod == boardstacks_up[i]) return 1;
  }
  return -1;
}

void Reset_B2L(const int* const list, const unsigned length)
{
  Belle2::HSLB hslb;
  for (unsigned i = 0; i < length; ++i) {
    printf("attempting to reset b2l connection on hslb %d \n", list[i]);
    try {
      hslb.open(list[i]);
      int csr = 0;
      hslb.reset_b2l(csr);
      hslb.close();
    } catch (...) {
      printf("ERROR: setting up connection to hslb not successfull\n");
    }
    if (Is_B2L_Up(list[i]) == 1) printf("reset succesfull\n");
    else printf("reset failed\n");
  }
}

void Reset_B2L_If_Necessary(const int* const list, const unsigned length)
{
  int boardstacks_down[4] = { -1, -1, -1, -1};
  Get_List_Of_Installed_Boardstacks_With_B2L_Down(boardstacks_down, 4);
  for (unsigned i = 0; i < length; ++i) {
    for (unsigned j = 0; j < length; ++j) {
      if (list[i] == boardstacks_down[j]) Reset_B2L(&boardstacks_down[j], 1);
    }
  }
}

void Bring_Up_B2L_Wherever_Possible(const int* const list, const unsigned length)
{
  Reset_B2L_If_Necessary(list, length);
}

/******************************* Conversion functions ******************************/
float convert_humidity(const int value)
{
  return 100.*value / 16382.;
}

float convert_humidity_temp(const int value)
{
  return value / 256.;
}

float convert_FPGA_current(const int value)
{
  float voltageCalib = 1000.0 * (500.0 / 120.5) * (1.0 / 4096.0);
  return (value >> 4) * voltageCalib;
}

float convert_FPGA_temp(const int value)
{
  float tempCalib = 1.0 / 16.0 * (503.975 / 4096.0);
  float tempOffset = 273.15;
  return value * tempCalib - tempOffset;
}

float convert_STTS751_temp(const int value)
{
  float temp751Calib = 1.0 / 16.0;
  int temp = value;
  if (temp == (int) 0xffffffff) {
    return 0.0;
  }

  if ((temp & (1 < 15)) != 0) {
    temp = temp - (1 << 16);
  }
  return temp * temp751Calib;
}

float convert_FPGA_voltage(const int value)
{
  float voltageCalib = 1.0 / 16.0 * 8.15 / 4096.0;
  return value * voltageCalib;
}

float convert_FPGA_voltage_from_external_voltage_divider(const int value)
{
  float voltageCalib = 1.0 / 16.0 * 8.15 / 4096.0;
  return value * voltageCalib;
}

float convert_ad7923_voltage_from_external_voltage_divider(const int value)
{
  return value / 1000.0;
}

float convert_ad7689_voltage_from_external_voltage_divider(const int value)
{
  return value / 1000.0;
}

int convert_ad7689_current(const int value)
{
  return value;
}

float convert_ad7689_temperature(const int value)
{
  return value / 256.0;
}

float convert_transceiver_temperature(const int value)
{
  return value / 256.0;
}
