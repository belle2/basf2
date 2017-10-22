#pragma once

#include <string>
#include <cstring>
#include <iostream>
#include <vector>

/* #include <trg/cdc/modules/houghtracking/CDCTrigger2DFinderModule.h> */
#include "trg/cdc/modules/houghtracking/xsi_loader.h"

namespace Belle2 {

  /* #ifndef CDCTrigger2DFinderModule_H */
  class CDCTrigger2DFinderModule;
  /* #endif */

  class CDCTrigger2DFinderFirmware {
  public:
    CDCTrigger2DFinderFirmware(CDCTrigger2DFinderModule const*);
    virtual ~CDCTrigger2DFinderFirmware();
    void initialize();
    void event();

  private:
    std::string lib_extension = ".so";
    std::string cwd = "/home/belle2/tasheng/tsim";
    std::string design_libname = cwd + "/xsim.dir/t2d/xsimk" + lib_extension;
    std::string simengine_libname = "librdi_simulator_kernel" + lib_extension;

    Xsi::Loader Xsi_Instance;
    s_xsi_setup_info info;

    const char one_val  = 3;
    const char zero_val = 2;
    const char zero_val_vec[2] = {2, 2};
    const char one_val_vec[2]  = {3, 3};

    static const int width_in = 219;
    static const int width_out = 732;

    // Input values
    char tsf0_in[width_in] = {};
    char tsf2_in[width_in] = {};
    char tsf4_in[width_in] = {};
    char tsf6_in[width_in] = {};
    char tsf8_in[width_in] = {};

    // Output value
    char t2d_out[width_out] = {};

    // Ports
    int clk;
    int tsf0;
    int tsf2;
    int tsf4;
    int tsf6;
    int tsf8;
    int out;

    CDCTrigger2DFinderModule const* m_mod;
  };
}
