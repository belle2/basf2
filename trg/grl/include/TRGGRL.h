/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : TRGGRL.h
// Section  : TRG GRL
// Owner    : Yun-Tsung Lai
// Email    : ytlai@hep1.phys.ntu.edu.tw
//-----------------------------------------------------------------------------
// Description : A class to represent GRL.
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#ifndef TRGGRL_FLAG_
#define TRGGRL_FLAG_

#define TRGGRL_UNDEFINED 999999

#include <vector>
#include "trg/trg/Clock.h"
#include "trg/grl/TRGGRLMatch.h"
#include <TFile.h>
#include <TTree.h>


namespace HepGeom {
  template <class T> class Point3D;
}

namespace Belle2 {

  class TRGTime;
  class TRGClock;
  class TRGLink;
  class TRGGRL;

//  The instance of TRGGRL is a singleton. 'TRGGRL::getTRGGRL()'
//  gives you a pointer to access the TRGGRL. Geometrical information
//  is initialized automatically. Before accessing hit information,
//  user has to call 'update()' to initialize hit information event
//  by event.

  /// a class for TRGGRL
  class TRGGRL {

  public:

    /// returns TRGGRL object with specific configuration.
    static TRGGRL* getTRGGRL(const std::string& configFile,
                             unsigned simulationMode = 0,
                             unsigned fastSimulationMode = 0,
                             unsigned firmwareSimulationMode = 0);

    /// returns TRGGRL object. TRGGRL should be created with specific
    /// configuration before calling this function.
    static TRGGRL* getTRGGRL(void);

  private:

    /// Constructor
    TRGGRL(const std::string& configFile,
           unsigned simulationMode,
           unsigned fastSimulationMode,
           unsigned firmwareSimulationMode);

    /// Destructor
    virtual ~TRGGRL();

    /// initializes GRL.
    void initialize(void);

    /// configures trigger modules for firmware simulation.
    void configure(void);

    /// fast trigger simulation.
    void simulate(void);

    /// terminates when run is finished
    void terminate(void);

    /// matching function
    //bool barrel_matching_2D(TRGCDCTrack *, TRGECLCluster *);

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

    /// clears all TRGGRL information.
    void clear(void);

    /// clears TRGGRL information.
    void fastClear(void);

    /// updates TRGGRL information.
    void update(bool mcAnalysis = true);

  public:// Utility functions

  public:// TRG information

    /// returns the system clock.
    const TRGClock& systemClock(void) const;

  private:

    /// Fast simulation
    void fastSimulation(void);

    /// Firmware simulation
    void firmwareSimulation(void);

  private:

    /// GRL singleton.
    static TRGGRL* _grl;

    /// Debug level.
    mutable int _debugLevel;

    /// root file name.
    std::string _configFilename;

    /// Simulation mode.
    unsigned _simulationMode;

    /// Fast simulation mode.
    unsigned _fastSimulationMode;

    /// Firmware simulation mode.
    unsigned _firmwareSimulationMode;

    /// GRL trigger system clock.
    const TRGClock& _clock;

    /// root file
    TFile* m_file;

    /// root tree
    TTree* h1;

    /** Temporary variables to make tree in root files */
    double x0;
    /** Temporary variables to make tree in root files */
    double x1;
    /** Temporary variables to make tree in root files */
    double x2;
    /** Temporary variables to make tree in root files */
    double x3;
    /** Temporary variables to make tree in root files */
    double x4;
    /** Temporary variables to make tree in root files */
    double x5;
    /** Temporary variables to make tree in root files */
    double x6;
    /** Temporary variables to make tree in root files */
    double x7;
    /** Temporary variables to make tree in root files */
    double x8;
    /** Temporary variables to make tree in root files */
    double x9;
    /**  Vector which stores list of TRGGRLMatch without 3D information */
    std::vector<TRGGRLMatch* > matchList;
    /**  Vector which stores list of TRGGRLMatch with 3D information */
    std::vector<TRGGRLMatch* > matchList3D;

    friend class TRGGRLModule;
  };

//-----------------------------------------------------------------------------

  inline
  int
  TRGGRL::debugLevel(void) const
  {
    return _debugLevel;
  }

  inline
  int
  TRGGRL::debugLevel(int a) const
  {
    return _debugLevel = a;
  }

  inline
  const TRGClock&
  TRGGRL::systemClock(void) const
  {
    return _clock;
  }

  inline
  unsigned
  TRGGRL::firmwareSimulationMode(void) const
  {
    return _firmwareSimulationMode;
  }

  inline
  std::string
  TRGGRL::configFile(void) const
  {
    return _configFilename;
  }

} // namespace Belle2

#endif /* TRGGRL_FLAG_ */
