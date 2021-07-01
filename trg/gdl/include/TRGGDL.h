//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : TRGGDL.h
// Section  : TRG GDL
// Owner    : Yoshihito Iwasaki, Chunhua Li
// Email    : yoshihito.iwasaki@kek.jp, chunhua.li@unimelb.edu.au
//-----------------------------------------------------------------------------
// Description : A class to represent GDL.
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#ifndef TRGGDL_FLAG_
#define TRGGDL_FLAG_

#define TRGGDL_UNDEFINED 999999

#include <fstream>
#include <vector>
#include "trg/trg/Clock.h"
#include "trg/trg/SignalVector.h"
#include "trg/trg/SignalBundle.h"

#include <framework/database/DBObjPtr.h>
#include <mdst/dbobjects/TRGGDLDBInputBits.h>
#include <mdst/dbobjects/TRGGDLDBFTDLBits.h>
#include <mdst/dbobjects/TRGGDLDBPrescales.h>
#include <trg/gdl/dbobjects/TRGGDLDBAlgs.h>
#include <simulation/dataobjects/SimClockState.h>
#include <framework/dbobjects/HardwareClockSettings.h>
#include <TH1I.h>

namespace Belle2 {

  class TRGClock;
  class TRGLink;

///  The instance of TRGGDL is a singleton. 'TRGGDL::getTRGGDL()'
///  gives you a pointer to access the TRGGDL. Geometrical information
///  is initialized automatically. Before accessing hit information,
///  user has to call 'update()' to initialize hit information event
///  by event.

  class TRGGDL {

  public:

    /// returns TRGGDL object with specific configuration.
    static TRGGDL* getTRGGDL(const std::string& configFile,
                             unsigned simulationMode = 0,
                             unsigned fastSimulationMode = 0,
                             unsigned firmwareSimulationMode = 0,
                             const std::string& Phase = "Phase",
                             bool algFromDB = true,
                             const std::string& algFilePath = "ftd.alg",
                             int debugLevel = 0,
                             double timquality_threshold_sfin = 0,
                             double timquality_threshold_fine = 0);

    /// returns TRGGDL object. TRGGDL should be created with specific
    /// configuration before calling this function.
    static TRGGDL* getTRGGDL(void);

  private:

    /// Constructor
    TRGGDL(const std::string& configFile,
           unsigned simulationMode,
           unsigned fastSimulationMode,
           unsigned firmwareSimulationMode,
           const std::string& Phase,
           bool algFromDB = true,
           const std::string& algFilePath = "ftd.alg",
           int debugLevel = 0,
           double timquality_threshold_sfin = 0,
           double timquality_threshold_fine = 0);

    /// Destructor
    virtual ~TRGGDL();

    /// initializes GDL.
    void initialize(void);

    /// configures trigger modules for firmware simulation.
    void configure(void);

    /// fast trigger simulation.
    void simulate(void);

    /// terminates when run is finished
    void terminate(void);

  public:// Selectors

    /// returns name.
    std::string name(void) const;

    /// returns version.
    std::string version(void) const;

    /// returns configuration file name.
    std::string configFile(void) const;

    /// returns simulation mode.
    unsigned mode(void) const;

    /// sets and returns simulation mode.
    unsigned mode(unsigned);

    /// returns firmware simulation mode.
    unsigned firmwareSimulationMode(void) const;

    /// dumps debug information.
    void dump(const std::string& message) const;

    /// returns debug level.
    int debugLevel(void) const;

    /// sets and returns debug level.
    int debugLevel(int) const;

  public:// Event by event information.

    /// clears all TRGGDL information.
    void clear(void);

    /// clears TRGGDL information.
    void fastClear(void);

    /// updates TRGGDL information.
    void update(bool mcAnalysis = true);

  public:

    /// returns the system clock.
    const TRGClock& systemClock(void) const;

    /// returns the system offset in MC.
    double systemOffsetMC(void) const;

    /// Makes bit pattern(state) using input bit pattern(state).
    static TRGState decision(const TRGState& input);

    /// Function to simulate final trigger decision.
    static void (* _ftd)(bool* out, bool* in);

    /// Makes timing decision.
    static TRGState timingDecision(const TRGState& input,
                                   TRGState& registers,
                                   bool& logicStillActive);

    bool doprescale(int f);

    bool isFiredFTDL(std::vector<bool> input, std::string alg);

    /// Check the content of the DBObjects used by this class.
    void checkDatabase() const;

    std::vector<bool> getInpBits(void) {return _inpBits;}

    std::vector<bool> getFtdBits(void) {return _ftdBits;}

    std::vector<bool> getPsnBits(void) {return _psnBits;}

  private:

    /// updates TRGGDL information for MC.
    void updateMC(void);

    /// Fast simulation
    void fastSimulation(void);

    /// Firmware simulation
    void firmwareSimulation(void);

    /// Data simulation
    void dataSimulation(void);

    /// Read input data definition.
    void getInput(std::ifstream& ifs);

    /// Read output data definition.
    void getOutput(std::ifstream& ifs);

    /// Read algorithm data definition.
    void getAlgorithm(std::ifstream& ifs);

    /// Accumulate bit info in histogram
    void accumulateInp(TH1I*);

    /// Accumulate bit info in histogram
    void accumulateFtd(TH1I*);

    /// Accumulate bit info in histogram
    void accumulatePsn(TH1I*);

    /// Returns fired/not for input bits.
    bool isFiredInput(int n) {return _inpBits[n];}

    /// Returns fired/not for ftdl bits.
    bool isFiredFtdl(int n) {return _ftdBits[n];}

    /// Returns fired/not for psnm bits.
    bool isFiredPsnm(int n) {return _psnBits[n];}

  private:

    /// GDL singleton.
    static TRGGDL* _gdl;

    /// Debug level.
    mutable int _debugLevel;

    /// GDL configuration filename.
    std::string _configFilename;

    /// Simulation mode.
    unsigned _simulationMode;

    /// Fast simulation mode.
    unsigned _fastSimulationMode;

    /// Firmware simulation mode.
    unsigned _firmwareSimulationMode;

    //Phase
    std::string _Phase;

    // Path to algorithm file
    std::string _algFilePath;

    /// GDL trigger system clock.
    const TRGClock& _clock;

    /// Timing offset of GDL.
    const double _offset;

    /// All serial links.
    std::vector<TRGLink*> _links;

    /// Input names.
    std::vector<std::string> _input;

    /// Output names.
    std::vector<std::string> _output;

    /// Algorithm
    std::vector<std::string> _algorithm;

    /// Input signal bundle.
    TRGSignalBundle* _isb;

    /// Output signal bundle.
    TRGSignalBundle* _osb;

    /// Timing input signal bundle.
    TRGSignalBundle* _tsb;

    /// Timing output signal bundle.
    TRGSignalBundle* _tosb;

    /**Data base of GDL input bits**/
    DBObjPtr<TRGGDLDBInputBits> m_InputBitsDB;
    DBObjPtr<TRGGDLDBFTDLBits>  m_FTDLBitsDB;
    DBObjPtr<TRGGDLDBPrescales> m_PrescalesDB;
    DBObjPtr<TRGGDLDBAlgs> m_AlgsDB;

    /// Threshold to determine timing quality flag with MC truth: super fine
    double _timquality_threshold_sfin;

    /// Threshold to determine timing quality flag with MC truth: fine
    double _timquality_threshold_fine;

    ///
    std::vector<bool> _inpBits;
    std::vector<bool> _ftdBits;
    std::vector<bool> _psnBits;
    std::vector<std::string> _inpBitNames;
    std::vector<std::string> _oupBitNames;

    int getNbitsOup(void) {return _oupBitNames.size();}
    int getNbitsInp(void) {return _inpBitNames.size();}

    bool _algFromDB;

    /** generated hardware clock state */
    StoreObjPtr<SimClockState> m_simClockState;
    /** Hardware Clocks*/
    DBObjPtr<HardwareClockSettings> m_hwClock;


    friend class TRGGDLModule;
  };

//-----------------------------------------------------------------------------

  inline
  int
  TRGGDL::debugLevel(void) const
  {
    return _debugLevel;
  }

  inline
  int
  TRGGDL::debugLevel(int a) const
  {
    return _debugLevel = a;
  }

  inline
  const TRGClock&
  TRGGDL::systemClock(void) const
  {
    return _clock;
  }

  inline
  double
  TRGGDL::systemOffsetMC(void) const
  {
    return _offset;
  }

  inline
  unsigned
  TRGGDL::firmwareSimulationMode(void) const
  {
    return _firmwareSimulationMode;
  }

  inline
  std::string
  TRGGDL::configFile(void) const
  {
    return _configFilename;
  }


} // namespace Belle2

#endif /* TRGGDL_FLAG_ */
