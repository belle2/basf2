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
#include <iostream>
#include <vector>
#include "trg/trg/Clock.h"
#include "trg/trg/SignalVector.h"
#include "trg/trg/SignalBundle.h"

namespace HepGeom {
  template <class T> class Point3D;
}

namespace Belle2 {

  class TRGTime;
  class TRGClock;
  class TRGLink;
  class TRGGDL;

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
                             const std::string& Phase = "Phase");

    /// returns TRGGDL object. TRGGDL should be created with specific
    /// configuration before calling this function.
    static TRGGDL* getTRGGDL(void);

  private:

    /// Constructor
    TRGGDL(const std::string& configFile,
           unsigned simulationMode,
           unsigned fastSimulationMode,
           unsigned firmwareSimulationMode,
           const std::string& Phase);

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

  public:// Utility functions

  public:// TRG information

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
    int doprescale(int f);
  private:

    /// updates TRGGDL information for MC.
    void updateMC(void);

    /// Fast simulation
    void fastSimulation(void);

    /// Firmware simulation
    void firmwareSimulation(void);

    /// Read input data definition.
    void getInput(std::ifstream& ifs);

    /// Read output data definition.
    void getOutput(std::ifstream& ifs);

    /// Read algorithm data definition.
    void getAlgorithm(std::ifstream& ifs);

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
