#pragma once
#include <framework/core/Module.h>
#include <cdc/dataobjects/CDCHit.h>
#include <framework/datastore/StoreArray.h>

#include <memory>
#include <string>
#include <cstring>
#include <vector>
#include <bitset>
#include <array>
#include <tuple>
#include <unordered_map>

#include <unistd.h>
#include <cstdlib>
#include <cstdio>

namespace Belle2 {

  namespace CDCTrigger {
    enum class Priority : unsigned char {nothing, first, second};
    enum MergerOut : long unsigned int {priorityTime, fastestTime, edgeTime, hitmap, secondPriorityHit};
  }

  /**
   * This class is the interface between TSim/Basf2 TSF module and the firmware
   * simulation core of XSim/ISim.
   *
   */
  class CDCTriggerTSFFirmwareModule : public Module {
  public:
    /** Constructor.  */
    CDCTriggerTSFFirmwareModule();

    virtual ~CDCTriggerTSFFirmwareModule();

    /**
     * spawn child process for workers, open pipes to pass data
     */
    void initialize();

    /**
     * close the pipes and wait for children to die.
     */
    void terminate();

    /**
     * Things to do for each event.
     *
     * It gets the CDCHits from DataStore, simulate the Merger output, pass them
     * to the firmware simulation process, and collect TSF firmware response.
     */
    void event();

  protected:
    /** Name of the StoreArray containing the input track segment hits. */
    std::string m_hitCollectionName;

    /** Name of the StoreArray containing the tracks found by the Hough tracking. */
    std::string m_outputCollectionName;

    /** how many clocks to simulate per event */
    static const int m_nClockPerEvent = 16;

    /** CDCHit array */
    Belle2::StoreArray<CDCHit> m_cdcHits;

    int m_TDCCountForT0 = 4988;

    /** switch
        If true, the trigger time of the hit with largest TDC count becomes 0.
        Otherwise, m_TDCCountForT0 becomes 0 (as in the fast TSIM).
     */
    bool m_allPositiveTime = true;

    /**
     * Calculate local segment ID from global ID
     *
     * @param globalID global TS ID
     *
     * @return local TS ID in the Super Layer. TS in SL8 is shifted by 16.
     */
    int localSegmentID(int globalID);

    std::string lib_extension = ".so";
    std::string libdir = "/home/belle2/tasheng/tsim";
    /** path to the simulation snapshot */
    std::string design_libname_pre = libdir + "/xsim.dir/tsf";
    std::string design_libname_post = "/xsimk" + lib_extension;
    /** path to the simulation engine */
    std::string simengine_libname = "librdi_simulator_kernel" + lib_extension;

    std::string wdbName_pre = "tsf";
    std::string wdbName_post = ".wdb";

    /** number of TSF to simulate */
    static const int m_nSubModules = 5;

    /** '1' in XSI VHDL simulation */
    /* static constexpr char one_val  = 3; */
    /** '0' in XSI VHDL simulation */
    /* static constexpr char zero_val = 2; */

    /** array holding child process ID */
    std::array<pid_t, m_nSubModules> m_pid;

    /** number of mergers in each super layer */
    static constexpr std::array<int, 9> nMergers = {10, 10, 12, 14, 16, 18, 20, 22, 24};

    /* number of mergers in axial super layers */
    static constexpr std::array<int, m_nSubModules> nAxialMergers = {10, 12, 16, 20, 24};

    /* number of trackers */
    static constexpr int nTrackers = 4;

    /** merger output data width */
    static constexpr int mergerWidth = 256;

    /** width of output data width*/
    static constexpr int width_out = 429;

    /* number of wire/cell in a single merger unit */
    static constexpr int nWiresInMerger = 80;

    /* number of track segments in a single merger unit */
    static constexpr int nSegmentsInMerger = 16;

    static constexpr int nCellsInLayer = 16;

    using mergerVector = std::bitset<mergerWidth>;
    using mergerOutput = std::vector<mergerVector>;
    using mergerOutArray = std::array<mergerOutput, m_nSubModules>;
    /* bits format of merger output / TSF input */
    mergerOutArray outputFromMerger;

    using inputVector = std::array<char, mergerWidth>;
    using inputFromMerger = std::vector<inputVector>;
    using inputToTSFArray = std::array<inputFromMerger, m_nSubModules>;
    /* XSI compliant format of input to TSF */
    inputToTSFArray inputToTSF;

    using outputVector = std::array<char, width_out>;
    using outputArray = std::array<outputVector, nTrackers>;
    /** array holding TSF output data */
    std::array<outputArray, m_nSubModules> outputToTracker;

    /* get the XSI compliant format from the bits format TSF input */
    template<int iSL>
    char* getData(inputToTSFArray);

    using streamPair = std::array<FILE*, 2>;
    /* array holding file handlers of pipes */
    std::array<streamPair, m_nSubModules> stream;

    /* array holding file descriptors of pipes */
    std::array<std::array<int, 2>, m_nSubModules> inputFileDescriptor;
    std::array<std::array<int, 2>, m_nSubModules> outputFileDescriptor;

    outputArray writeRead(const char*,  FILE*, FILE*);

    void write(const char*, FILE*);
    outputArray read(FILE*);
    std::istream* ins;

    /**************************************************
     *  Merger simulation
     **************************************************/

    // bit width for priority time and fast time
    static constexpr size_t timeWidth = 4;

    using timeVec = std::bitset<timeWidth>;
    // data structure to hold merger output
    // <priority time (4 bits x 16),
    // fast time (4 bits x 16),
    // edge timing (4 bits x 3 or 5),
    // hitmap (80 bits),
    // 2nd priority hit (1 bit x 16)>
    template<size_t nEdges>
    using mergerStructElement = std::tuple <
                                std::array<timeVec, nSegmentsInMerger>,
                                std::array<timeVec, nSegmentsInMerger>,
                                std::array<timeVec, nEdges>,
                                std::array<std::bitset<nWiresInMerger>, 1>,
                                std::array<std::bitset<nSegmentsInMerger>, 1> >;
    template<size_t nEdges>
    using mergerStruct = std::vector<mergerStructElement<nEdges> >;
    // record when a time slow has been registered by a hit
    // <priority time, fast time, edge timing>
    using registeredStructElement = std::array<std::bitset<nCellsInLayer>, 3>;
    using registeredStruct = std::vector<registeredStructElement>;

    /** CDC hit ID in each clock */
    std::vector<std::vector<int> > iAxialHitInClock;

    using WireSet = std::vector<short>;
    using TSMap = std::unordered_map<short, WireSet>;
    /* map from cell ID to TS ID, for inner and outer Merger */
    std::array<TSMap, 2> m_tsMap;

    using edgeMap = std::unordered_map<unsigned short, timeVec*>;
    using cellList = std::vector<unsigned short>;
    /* list of cell ID related to edge timing */
    std::array<cellList, 5> innerInvEdge = {cellList {31},
                                            cellList {64},
                                            cellList {32, 48, 64, 65},
                                            cellList {31, 47, 62, 63, 78},
                                            cellList {63, 79}
                                           };

    std::array<cellList, 3> outerInvEdge = {cellList {63},
                                            cellList {0, 64},
                                            cellList {15, 31, 63, 79}
                                           };
    using edgeList = std::unordered_map<unsigned short, std::vector<unsigned short>>;
    /* map from cell ID to related edge ID */
    std::array<edgeList, 2> m_edge;

    /* ID of the earlist CDC hit in an event */
    int m_iFirstHit;

    CDCTrigger::Priority priority(int);

    unsigned short trgTime(int, int);

    unsigned short mergerCellID(int);

    unsigned short mergerNumber(int);

    WireSet segmentID(int iHit);

    std::bitset<4> timeStamp(int index, int iFirstHit);

    void computeEdges();

    timeVec& edge();

    void writeMergerOutput();

    void initializeMerger();

    void simulateMerger(unsigned);

    template<CDCTrigger::MergerOut, size_t>
    void pack(inputVector::iterator&, unsigned, mergerStructElement<5>&);

    bool notHit(CDCTrigger::MergerOut, unsigned, registeredStructElement&);

    void registerHit(CDCTrigger::MergerOut, unsigned, registeredStructElement&);

  };
}
