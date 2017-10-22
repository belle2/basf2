#include "trg/cdc/modules/houghtracking/xsi_loader.h"

#include <trg/cdc/modules/houghtracking/CDCTrigger2DFinderModule.h>
#include <trg/cdc/modules/houghtracking/CDCTrigger2DFinderFirmware.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <trg/cdc/dataobjects/CDCTriggerSegmentHit.h>

#include <string>
#include <cstring>
#include <iostream>
#include <vector>

namespace Cosim {
  const char* std_logic_literal[] = {"U", "X", "0", "1", "Z", "W", "L", "H", "-"};

  void vector(const std::string word, char*& vec)
  {
    // char vec[word.size()];
    vec = (char*) malloc(word.size() + 1);
    for (unsigned i = 0; i < word.size(); ++i) {
      vec[i] = (word[i] == '0') ? 2 : 3;
    }
  }

  std::string display_value(const char* count, int size)
  {
    std::string res;
    for (int i = 0; i < size; i++)
      res +=  std_logic_literal[(int) count[i]];
    return res;
  }
}

int Xsi::Loader::get_port_number_or_exit(std::string name)
{
  int portNumber = get_port_number(name.c_str());
  if (portNumber < 0) {
    std::cerr << "ERROR: " << name << " not found" << std::endl;
    throw;
  }
  return portNumber;
}


using namespace Belle2;
using namespace Cosim;

CDCTrigger2DFinderFirmware::CDCTrigger2DFinderFirmware(const CDCTrigger2DFinderModule* mod) :
  Xsi_Instance(design_libname, simengine_libname),
  m_mod(mod)
{

};

CDCTrigger2DFinderFirmware::~CDCTrigger2DFinderFirmware()
{
  Xsi_Instance.close();
}

void CDCTrigger2DFinderFirmware::initialize()
{
  std::cout << "Design DLL     : " << design_libname << std::endl;
  std::cout << "Sim Engine DLL : " << simengine_libname << std::endl;
  try {
    memset(&info, 0, sizeof(info));
    char logName[] = "firmware.log";
    // info.logFileName = NULL;
    info.logFileName = logName;
    char wdbName[] = "test.wdb";
    info.wdbFileName = wdbName;
    Xsi_Instance.open(&info);
    Xsi_Instance.trace_all();

    clk  = Xsi_Instance.get_port_number_or_exit("Top_clkData_s");
    tsf0 = Xsi_Instance.get_port_number_or_exit("TSF0_input_i");
    tsf2 = Xsi_Instance.get_port_number_or_exit("TSF2_input_i");
    tsf4 = Xsi_Instance.get_port_number_or_exit("TSF4_input_i");
    tsf6 = Xsi_Instance.get_port_number_or_exit("TSF6_input_i");
    tsf8 = Xsi_Instance.get_port_number_or_exit("TSF8_input_i");
    out  = Xsi_Instance.get_port_number_or_exit("Main_out");

    memset(tsf0_in, 2, sizeof(tsf0_in));
    memset(tsf2_in, 2, sizeof(tsf2_in));
    memset(tsf4_in, 2, sizeof(tsf4_in));
    memset(tsf6_in, 2, sizeof(tsf6_in));
    memset(tsf8_in, 2, sizeof(tsf8_in));

    char* i0 = 0;
    char* i1 = 0;
    char* i2 = 0;
    char* i3 = 0;

    char* p00 = 0;
    char* p10 = 0;
    char* p01 = 0;
    char* p11 = 0;

    std::vector<char*> ids;
    std::vector<char*> pris;

    vector("00", p00);
    vector("11", p11);
    vector("01", p01);
    vector("10", p10);

    vector("00010101", i0);
    vector("00010110", i1);
    vector("00110010", i2);
    vector("00110011", i3);

    pris.insert(pris.end(), {p10, p11, p11, p01});
    ids.insert(ids.end(), {i0, i1, i2, i3});
    for (unsigned i = 0; i < ids.size(); ++i) {
      memcpy(tsf0_in + 21 * i + 9, ids[i], 8);
      memcpy(tsf0_in + 21 * i + 26, pris[i], 2);
      memcpy(tsf0_in + 21 * i + 28, pris[i], 2);
    }
    pris.clear();
    ids.clear();

    vector("00010110", i0);
    vector("00010111", i1);
    vector("00111010", i2);
    pris.insert(pris.end(), {p10, p11, p11});
    ids.insert(ids.end(), {i0, i1, i2});
    for (unsigned i = 0; i < ids.size(); ++i) {
      memcpy(tsf2_in + 21 * i + 9, ids[i], 8);
      memcpy(tsf2_in + 21 * i + 26, pris[i], 2);
      memcpy(tsf2_in + 21 * i + 28, pris[i], 2);
    }
    pris.clear();
    ids.clear();

    vector("00011000", i0);
    vector("00011001", i1);
    vector("01001001", i2);
    vector("01001010", i3);
    pris.insert(pris.end(), {p11, p11, p11, p01});
    ids.insert(ids.end(), {i0, i1, i2, i3});
    for (unsigned i = 0; i < ids.size(); ++i) {
      memcpy(tsf4_in + 21 * i + 9, ids[i], 8);
      memcpy(tsf4_in + 21 * i + 26, pris[i], 2);
      memcpy(tsf4_in + 21 * i + 28, pris[i], 2);
    }
    pris.clear();
    ids.clear();

    vector("00010111", i0);
    vector("01010111", i1);
    pris.insert(pris.end(), {p11, p11});
    ids.insert(ids.end(), {i0, i1});
    for (unsigned i = 0; i < ids.size(); ++i) {
      memcpy(tsf6_in + 21 * i + 9, ids[i], 8);
      memcpy(tsf6_in + 21 * i + 26, pris[i], 2);
      memcpy(tsf6_in + 21 * i + 28, pris[i], 2);
    }
    pris.clear();
    ids.clear();

    vector("01110011", i0);
    pris.insert(pris.end(), {p11});
    ids.insert(ids.end(), {i0});
    for (unsigned i = 0; i < ids.size(); ++i) {
      memcpy(tsf8_in + 21 * i + 9, ids[i], 8);
      memcpy(tsf8_in + 21 * i + 26, pris[i], 2);
      memcpy(tsf8_in + 21 * i + 28, pris[i], 2);
    }
    pris.clear();
    ids.clear();

    std::cout << display_value(tsf0_in, width_in) << "\n";
    std::cout << display_value(tsf2_in, width_in) << "\n";
    std::cout << display_value(tsf4_in, width_in) << "\n";
    std::cout << display_value(tsf6_in, width_in) << "\n";
    std::cout << display_value(tsf8_in, width_in) << "\n";

    Xsi_Instance.put_value(tsf0, &tsf0_in);
    Xsi_Instance.put_value(tsf2, &tsf2_in);
    Xsi_Instance.put_value(tsf4, &tsf4_in);
    Xsi_Instance.put_value(tsf6, &tsf6_in);
    Xsi_Instance.put_value(tsf8, &tsf8_in);

    // Start low clock
    Xsi_Instance.put_value(clk, &zero_val);
    Xsi_Instance.run(10);

    // Reset to 1 and clock it
    Xsi_Instance.put_value(clk, &one_val);
    Xsi_Instance.run(10);

    // Put clk to 0, reset to 0 and enable to 1
    Xsi_Instance.put_value(clk, &zero_val);

  } catch (std::exception& e) {
    std::cerr << "ERROR: An exception occurred: " << e.what() << std::endl;
  } catch (...) {
    std::cerr << "ERROR: An unknown exception occurred." << std::endl;
    // Xsi_Instance.get_error_info();
    throw;
  }
};

void CDCTrigger2DFinderFirmware::event()
{

  // my variables
  int status = 0;

  StoreArray<CDCTriggerSegmentHit> segmentHits(m_mod->m_hitCollectionName);

  std::vector<unsigned> iHit;

  try {
    for (int i = 0; i < 12; i++) {
      // Put clk to one
      Xsi_Instance.put_value(clk, &one_val);
      Xsi_Instance.run(10);

      // read the output
      Xsi_Instance.get_value(out, t2d_out);

      std::cout << "clock # " << i << "\n";

      std::cout << display_value(t2d_out, 6) << "\n";
      for (int j = 0; j < 4; j++) {
        std::cout << display_value(t2d_out + 6 + 121 * j, 121) << "\n";
      }
      // Put clk to zero
      Xsi_Instance.put_value(clk, &zero_val);
      Xsi_Instance.run(10);
    }

    std::cout << status << std::endl;

  } catch (std::exception& e) {
    std::cerr << "ERROR: An exception occurred: " << e.what() << std::endl;
    status = 2;
  } catch (...) {
    std::cerr << "ERROR: An unknown exception occurred." << std::endl;
    status = 3;
  }

};

