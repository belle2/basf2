#pragma once

#include <string>
#include <cstring>
#include <iostream>
#include <vector>
#include <bitset>
#include <array>

/* #include <trg/cdc/modules/houghtracking/CDCTrigger2DFinderModule.h> */
#include "trg/cdc/modules/houghtracking/xsi_loader.h"

namespace Belle2 {

  class CDCTrigger2DFinderModule;
  class CDCTriggerSegmentHit;

  /**
   * This class is the interface between the 2D finder fast simulation
   * module and the firmware simulation core of ISim.
   *
   */
  class CDCTrigger2DFinderFirmware {
  public:
    CDCTrigger2DFinderFirmware(CDCTrigger2DFinderModule const*);
    virtual ~CDCTrigger2DFinderFirmware();

    /**
     *
     */
    void initialize();

    /**
     * Things to do for each event.
     *
     * When used with upstream firmware simulation module (i.e. TSF),
     * read the input signal vectors from it.
     *
     * When used with fast simulation, the input will be prepared by
     * reading the CDC segment hits from the DataStore, and assigning
     * them with clocks by found time.
     *
     * The signals are written to the firmware sim clock-by-clock.
     */
    void event();

  private:
    /** how many clocks to simulate per event */
    static const int m_nClockPerEvent = 32;

    /** width of TS hit input signal vector */
    static const int m_tsVectorWidth = 21;

    /** number of input ts hits per clock */
    static const int m_nInTSPerClock = 10;

    using tsVector = std::array<char, m_tsVectorWidth>;

    /**
     * Calculate local segment ID from global ID
     *
     * @param globalID global TS ID
     *
     * @return local TS ID in the Super Layer. TS in SL8 is shifted by 16.
     */
    int localSegmentID(int globalID);

    /**
     * Encode a TS hit into std_logic_vector.
     *
     * @param CDCTriggerSegmentHit the TS hit in question
     *
     * @return the equivalent of std_logic_vector(20 downto 0),
     * represented with a std::bitset<21>
     */
    /* std::bitset<m_tsVectorWidth> encodeTSHit(CDCTriggerSegmentHit const *); */
    auto encodeTSHit(CDCTriggerSegmentHit const*);

    /**
     * Turn TS bitset into char array
     *
     * @param ts the TS hit in question
     *
     * @return the equivalent of std_logic_vector(20 downto 0),
     * represented with a std::array<char, 21>
     */
    auto toTSSLV(const std::bitset<m_tsVectorWidth>);

    std::string lib_extension = ".so";
    std::string cwd = "/home/belle2/tasheng/tsim";
    /** path to the simulation snapshot */
    std::string design_libname = cwd + "/xsim.dir/t2d/xsimk" + lib_extension;
    /** path to the simulation engine */
    std::string simengine_libname = "librdi_simulator_kernel" + lib_extension;

    Xsi::Loader Xsi_Instance;
    s_xsi_setup_info info;

    /** '1' in XSI VHDL simulation */
    const char one_val  = 3;
    /** '0' in XSI VHDL simulation */
    const char zero_val = 2;

    /** width of (half-speed) input data width */
    static const int width_in = 219;
    /** width of output data width, not including the clock counter */
    static const int width_out = 732;

    using inputVector = std::array<char, width_in>;
    /** array holding 5 axial TSF input data */
    std::array<inputVector, 5> tsfInput;

    /** char array holding 2D output data */
    char t2d_out[width_out] = {};

    /** clock signal port handle */
    int clk;
    /** input signal port handle */
    int tsf[5];
    /** output signal port handle */
    int out;

    CDCTrigger2DFinderModule const* m_mod;
  };
}
