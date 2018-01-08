#pragma once
#include <framework/core/Module.h>
#include <cdc/dataobjects/CDCHit.h>
#include <framework/datastore/StoreArray.h>
#include <trg/cdc/dataobjects/Bitstream.h>
#include <trg/cdc/dataobjects/CDCTriggerSegmentHit.h>
#include <trg/cdc/Cosim.h>

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
    /* map and function to convert priorityPosition to Priority class */
    std::unordered_map<unsigned, Priority> toPriorityMap = {{0, Priority::nothing},
      {1, Priority::second},
      {2, Priority::second},
      {3, Priority::first}
    };
    Priority toPriority(unsigned priorityPosition)
    {
      return toPriorityMap[priorityPosition];
    }
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

    /** number of TSF to simulate */
    static constexpr int m_nSubModules = 5;

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

    /* Number of wire/cells in a single layer per merger unit */
    static constexpr int nCellsInLayer = 16;

    /* number of track segments in a single merger unit */
    static constexpr int nSegmentsInMerger = 16;

    /* bit width for priority time and fast time */
    static constexpr size_t timeWidth = 4;

    /** how many clocks to simulate per event */
    static constexpr int m_nClockPerEvent = 44;

    // data clock period (32ns) in unit of 2ns
    static constexpr int clockPeriod = 16;

  protected:
    /** Name of the StoreArray containing the input CDC hits. */
    std::string m_hitCollectionName;

    /** Name of the StoreArray holding the found TS hits. */
    std::string m_outputCollectionName;

    /** Name of the StoreArray holding the raw bit content to 2D trackers. */
    std::string m_outputBitstreamNameTo2D;

    /** Name of the StoreArray holding the raw bit content to ETF. */
    std::string m_outputBitstreamNameToETF;

    /** CDCHit array */
    Belle2::StoreArray<CDCHit> m_cdcHits;

    using outputVector = std::array<char, width_out>;
    using outputArray = std::array<outputVector, nTrackers>;

    using signalBus = std::array<outputArray, m_nSubModules>;
    using signalBitStream = Bitstream<signalBus>;

    /** bitstream of TSF output to 2D tracker */
    StoreArray<signalBitStream> m_bitsTo2D;

    /** unpacked track segment hit */
    StoreArray<CDCTriggerSegmentHit> m_tsHits;

    /** flag to only simulation merger and not TSF */
    bool m_mergerOnly;

    /** flag to simulate front-end clock counter */
    bool m_simulateCC;

    /** list of flags to run a TSF firmware simulation with dummy L/R LUT (to speed up loading) */
    std::vector<bool> m_stubLUT;

    /** debug level specified in the steering file */
    int m_debugLevel;

    int m_TDCCountForT0 = 4988;

    /** switch
        If true, the trigger time of the hit with largest TDC count becomes 0.
        Otherwise, m_TDCCountForT0 becomes 0 (as in the fast TSIM).
     */
    bool m_allPositiveTime = true;

    std::string lib_extension = ".so";
    std::string cwd = getcurrentdir();
    /** path to the simulation snapshot */
    std::string design_libname_pre = cwd + "/xsim.dir/tsf";
    std::string design_libname_post = "/xsimk" + lib_extension;
    /** path to the simulation engine */
    std::string simengine_libname = "librdi_simulator_kernel" + lib_extension;

    std::string wdbName_pre = "tsf";
    std::string wdbName_post = ".wdb";

    /** '1' in XSI VHDL simulation */
    /* static constexpr char one_val  = 3; */
    /** '0' in XSI VHDL simulation */
    /* static constexpr char zero_val = 2; */

    /** array holding child process ID */
    std::array<pid_t, m_nSubModules> m_pid;

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

    /**
     *  write TSF input signals to the worker
     *
     *  @param message    words to write
     *
     *  @param outstream  output file descriptor
     */
    void write(const char* message, FILE* outstream);

    /**
     *  write TSF output signals from the worker
     *
     *  @param instream  input file descriptor
     *
     *  @return          array holding TSF output
     */
    outputArray read(FILE* instream);

    std::istream* ins;

    /**************************************************
     *  Merger simulation
     **************************************************/

    using timeVec = std::bitset<timeWidth>;
    // data structure to hold merger output
    // <priority time (4 bits x 16),
    // fast time (4 bits x 16),
    // edge timing (4 bits x 3 or 5),
    // hitmap (80 bits x 1),
    // 2nd priority hit (16 bit x 1)>
    template<size_t nEdges>
    using mergerStructElement = std::tuple <
                                std::array<timeVec, nSegmentsInMerger>,
                                std::array<timeVec, nSegmentsInMerger>,
                                std::array<timeVec, nEdges>,
                                std::array<std::bitset<nWiresInMerger>, 1>,
                                std::array<std::bitset<nSegmentsInMerger>, 1> >;
    template<size_t nEdges>
    using mergerStruct = std::vector<mergerStructElement<nEdges> >;
    std::map<unsigned, mergerStruct<5> > dataAcrossClocks;

    // record when a time slow has been registered by a hit
    // <priority time, fast time, edge timing>
    using registeredStructElement = std::array<std::bitset<nCellsInLayer>, 3>;
    using registeredStruct = std::vector<registeredStructElement>;

    using priorityHitInMerger = std::map<unsigned, int>;
    using priorityHitStructInSL = std::vector<priorityHitInMerger>;
    using priorityHitStructInClock = std::map<unsigned, priorityHitStructInSL>;
    using priorityHitStruct = std::array<priorityHitStructInClock, m_nClockPerEvent>;
    /* list keeping the index of priority hit of a TS for making fastsim ts hit object */
    priorityHitStruct m_priorityHit;
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

    /**
     *  write TSF input signals to the worker
     *
     *  @param index  index of CDC hit
     *
     *  @return       what type of priority hit it is
     */
    CDCTrigger::Priority priority(int index);

    /**
     *  Get the trigger time of the CDC hit
     *
     *  @param index      index of the CDC hit in question
     *
     *  @param iFirstHit  index of the first hit
     *
     *  @return           the trigger time of the hit (in unit of about 2ns)
     */
    unsigned short trgTime(int index, int iFirstHit);

    /**
     *  Get the cell ID in the merger
     *
     *  @param index      index of the CDC hit in question
     *
     *  @return           the cell ID (0-79)
     */
    unsigned short mergerCellID(int index);

    /**
     *  Get the merger unit ID in a super layer
     *
     *  @param index      index of the CDC hit in question
     *
     *  @return           the merger unit ID
     */
    unsigned short mergerNumber(int index);

    /**
     *  Get the list of associated track segments with a hit
     *
     *  @param index      index of the CDC hit in question
     *
     *  @return           list of TS ID (0-15) containing the hit
     */
    WireSet segmentID(int iHit);

    /**
     *  Get the trigger time stamp of a hit
     *
     *  @param index      index of the CDC hit in question
     *
     *  @param iFirstHit  index of the first hit
     *
     *  @return           trigger time stamp (least significant 4 bits)
     */
    std::bitset<4> timeStamp(int index, int iFirstHit);

    /**
     *  Compute the map from merger cell ID to all its related edge fields
     */
    void computeEdges();

    /**
     *  Get CDC hits from the DataStore and distribute them to clocks.
     *  Within each clock, faster hit has a smaller index
     */
    void initializeMerger();

    /**
     *  Simulate 1 clock of merger
     *
     *  @param iclock  index of data clock
     */
    void simulateMerger(unsigned iclock);

    /**
     *  Pack the merger output data structure to TSF input vector
     *
     *  @param field   type of output to be packed
     *
     *  @param width   bit width of a single output unit
     *
     *  @param rInput  iterator of the TSF input vector
     *
     *  @param number  total number of the output units in a merger unit
     *
     *  @param output  merger output data structre
     */
    template<CDCTrigger::MergerOut field, size_t width>
    void pack(inputVector::reverse_iterator& rInput,
              unsigned number, mergerStructElement<5>& output);

    /**
     *  Whether a time field in a merger has been hit in the clock cycle
     *
     *  @param field   one of (priority, fastest, edge) time
     *
     *  @param iTS     index of the track segment or edge
     *
     *  @param reg     record of the timing fields in merger
     *
     *  @return        true if the TS has not been hit
     */
    bool notHit(CDCTrigger::MergerOut field, unsigned iTS, registeredStructElement& reg);

    /**
     *  Register the timing field so that later hits won't overwrite it
     *
     *  @param field   one of (priority, fastest, edge) time
     *
     *  @param iTS     index of the track segment or edge
     *
     *  @param reg     record of the timing fields in merger
     */
    void registerHit(CDCTrigger::MergerOut field, unsigned iTS, registeredStructElement& reg);

    void saveFirmwareOutput();

    void saveFastOutput(short iclock);

    void setSecondPriority(unsigned priTS,
                           unsigned iHit,
                           timeVec hitTime,
                           unsigned lr,
                           mergerStructElement<5>& mergerData,
                           registeredStructElement& registeredCell,
                           priorityHitInMerger& priorityHit);
  };
}
