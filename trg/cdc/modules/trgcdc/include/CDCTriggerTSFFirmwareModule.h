#pragma once
#include <framework/core/Module.h>
#include <cdc/dataobjects/CDCHit.h>
#include <framework/datastore/StoreArray.h>

#include <memory>
#include <string>
#include <cstring>
#include <iostream>
#include <vector>
#include <bitset>
#include <array>

#include "trg/cdc/modules/houghtracking/xsi_loader.h"

namespace Belle2 {

  class CDCTriggerSegmentHit;

  using tsOut = std::array<int, 4>;
  using tsOutArray = std::array<tsOut, 5>;
  struct TRGFinderTrack {
    double omega;
    double phi0;
    tsOutArray ts;
  };

  /**
   * This class is the interface between the 2D finder fast simulation
   * module and the firmware simulation core of ISim.
   *
   */
  class CDCTriggerTSFFirmwareModule : public Module {
  public:
    /** Constructor.  */
    CDCTriggerTSFFirmwareModule();

    virtual ~CDCTriggerTSFFirmwareModule();

    /**
     * Load the design snapshot and register the signal ports.
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

  protected:
    /** Name of the StoreArray containing the input track segment hits. */
    std::string m_hitCollectionName;

    /** Name of the StoreArray containing the tracks found by the Hough tracking. */
    std::string m_outputCollectionName;

    /** how many clocks to simulate per event */
    static const int m_nClockPerEvent = 32;

    /** width of merger input signal vector */
    static const int m_mergerVectorWidth = 256;

    /** CDCHit array */
    Belle2::StoreArray<CDCHit> m_cdcHits;

    /**
     * Calculate local segment ID from global ID
     *
     * @param globalID global TS ID
     *
     * @return local TS ID in the Super Layer. TS in SL8 is shifted by 16.
     */
    int localSegmentID(int globalID);


    /**
     * Turn TS bitset into char array
     *
     * @param ts the TS hit in question
     *
     * @return the equivalent of std_logic_vector(20 downto 0),
     * represented with a std::array<char, 21>
     */
    /* auto toTSSLV(const std::bitset<m_tsVectorWidth>); */


    tsOut decodeTSHit(std::string tsIn);

    TRGFinderTrack decodeTrack(std::string trackIn);

    void decodeOutput(short latency);

    std::string lib_extension = ".so";
    std::string cwd = "/home/belle2/tasheng/tsim";
    /** path to the simulation snapshot */
    std::string design_libname_pre = cwd + "/xsim.dir/tsf";
    std::string design_libname_post = "/xsimk" + lib_extension;
    /** path to the simulation engine */
    std::string simengine_libname = "librdi_simulator_kernel" + lib_extension;

    static const int m_nSubModules = 5;

    /* std::vector<Xsi::Loader> loader = { */
    /*   Xsi::Loader(design_libname_pre + '0' + design_libname_post, simengine_libname), */
    /*   Xsi::Loader(design_libname_pre + '2' + design_libname_post, simengine_libname), */
    /*   Xsi::Loader(design_libname_pre + '4' + design_libname_post, simengine_libname), */
    /*   Xsi::Loader(design_libname_pre + '6' + design_libname_post, simengine_libname), */
    /*   Xsi::Loader(design_libname_pre + '8' + design_libname_post, simengine_libname)}; */


    // We are forced to hold pointers to the xsi loaders, otherwise its copy constructor will
    // be called when being put to the STL container, even with vector::emplace!
    // The copy constructor cannot be used because 1. there should only be 1 instance talking to the
    // simulation kernel and 2. compilation will fail because xsi_shared library is made private.
    std::vector<std::unique_ptr<Xsi::Loader> > loader;

    std::array<s_xsi_setup_info, m_nSubModules> info;

    /** '1' in XSI VHDL simulation */
    const char one_val  = 3;
    /** '0' in XSI VHDL simulation */
    const char zero_val = 2;

    static constexpr std::array<int, m_nSubModules> nMergers = {10, 12, 16, 20, 24};

    /** width of input data width */
    static constexpr int width_in = 256;
    /** width of output data width*/
    static constexpr int width_out = 429;

    /* using bmplVector = boost::mpl::vector< */
    /*   std::array<char, width_in * nMergers[0]>, */
    /*   std::array<char, width_in * nMergers[1]>, */
    /*   std::array<char, width_in * nMergers[2]>, */
    /*   std::array<char, width_in * nMergers[3]>, */
    /*   std::array<char, width_in * nMergers[4]>>; */
    /* using inputVector = boost::fusion::result_of::as_vector<bmplVector>::type; */

    using inputVector = std::array<char, width_in>;
    using inputFromMerger = std::vector<inputVector>;
    using inputToTSFArray = std::array<inputFromMerger, m_nSubModules>;
    inputToTSFArray inputToTSF;

    using outputVector = std::array<char, width_out>;
    /** array holding TSF output data */
    std::array<outputVector, m_nSubModules> outputToTracker;

    template<int iSL>
    char* getData(inputToTSFArray);

    /** clock signal port handle */
    std::array<int, m_nSubModules> clk;
    /** input signal port handle */
    std::array<int, m_nSubModules> inPort;
    /** output signal port handle */
    std::array<int, m_nSubModules> outPort;
    /** input enable port handle */
    std::array<int, m_nSubModules> enableIn;
    /** output enable port handle */
    std::array<int, m_nSubModules> enableOutTracker;
    /** output enable port handle */
    std::array<int, m_nSubModules> enableOutEVT;
  };
}
