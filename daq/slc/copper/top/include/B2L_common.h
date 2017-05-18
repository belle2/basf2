#ifndef __B2LCOMMON__
#define __B2LCOMMON__

#include <map>
#include <string>
#include <vector>

namespace Belle2 {
  class HSLB;
}

extern const int FEEDBEES;
extern const int TIMEOUT;
extern const int DEADBEEF;

extern "C" {
  //read and write register fucntions to call from python
  int Read_Register(const int addr, const int scrod, const int carrier, const int asic);
  int Write_Register(const int addr, const int value, const int scrod, const int carrier, const int asic);
  int Write_Asic_Direct_Register(const int scrod, const int carrier, const int asic, const int addr, const int value);

  //Belle2 link status
  int GetNumberOfCarriers(const int scrod);
  void Show_Status_Of_Hslb_Boards();
  void Get_List_Of_Installed_Boardstacks_With_B2L_Up(int* list, const unsigned length);
  void Get_List_Of_Installed_Boardstacks_With_B2L_Down(int* list, const unsigned length);
  int Get_Number_Of_Boardstacks_With_B2L_Up();
  int Get_Number_Of_Installed_HSLB_Boards();
  int Is_B2L_Up(const unsigned scrod);
  void Reset_B2L(const int* const list, const unsigned length);
  void Reset_B2L_If_Necessary(const int* const list, const unsigned length);
  void Bring_Up_B2L_Wherever_Possible(const int* const list, const unsigned length);

  //conversion functions for temperature sensors, voltages etc.
  float convert_humidity(const int value);
  float convert_humidity_temp(const int value);
  float convert_FPGA_current(const int value);
  float convert_FPGA_temp(const int value);
  float convert_STTS751_temp(const int value);
  float convert_FPGA_voltage(const int value);
  float convert_FPGA_voltage_from_external_voltage_divider(const int value);
  float convert_ad7923_voltage_from_external_voltage_divider(const int value);
  float convert_ad7689_voltage_from_external_voltage_divider(const int value);
  int convert_ad7689_current(const int value);
  float convert_ad7689_temperature(const int value);
  float convert_transceiver_temperature(const int value);
}

//read and write register functions for c++ programs (used by nsm2 slow control framework)
int B2L_Address_Carrier_Asic(const int addr, const int carrier, const int asic);

int Read_Register(Belle2::HSLB& hslb, const int addr, const int carrier = 0, const int asic = 0);
int Raw_Read_Register(Belle2::HSLB& hslb, const int addr, const int max_tries = 40);

int Write_Register(Belle2::HSLB& hslb, const int addr, const int value, const int carrier, const int asic);
int Raw_Write_Register(Belle2::HSLB& hslb, const int addr, const int value, const int max_tries = 20);
int Raw_Write_Register_noReadBack(Belle2::HSLB& hslb, const int addr, const int value);
int Write_Asic_Direct_Register(Belle2::HSLB& hslb, const int carrier, const int asic, const int addr, const int value);
int Intermediate_Write_Asic_Direct_Register(Belle2::HSLB hslb, const int writeAddress, const int readAddr, const int value,
                                            const int retries = 4);

std::map<std::string, std::vector<int> > Get_Status_Of_all_Hslb_Boards();

#endif
