//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : TRGGDLModule.h
// Section  : TRG GDL
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A trigger module for GDL
//-----------------------------------------------------------------------------
// 0.00 : 2013/12/13 : First version
//-----------------------------------------------------------------------------

#ifndef TRGGDLModule_H
#define TRGGDLModule_H

#include <string>
// #include "framework/core/Module.h"
#include <framework/core/HistoModule.h>
#include <framework/datastore/StoreObjPtr.h>
#include <mdst/dataobjects/TRGSummary.h>
#include <trg/grl/dataobjects/TRGGRLInfo.h>
#include "trg/gdl/TRGGDL.h"
#include <simulation/dataobjects/SimClockState.h>
#include <TDirectory.h>
#include <TH1I.h>

namespace Belle2 {

/// A module to simulate the Global Decision Logic. This module
/// requires sub-trigger simulation data (CDC, ECL, TOP, and KLM). The
/// output is GDL response.

//class TRGGDLModule : public Module {
  class TRGGDLModule : public HistoModule {

  public:

    /// Constructor
    TRGGDLModule();
//  TRGGDLModule(const std::string & type);

    /// Destructor
    virtual ~TRGGDLModule();

    /// Initilizes TRGGDLModule.
    virtual void initialize() override;

    /// Called when new run started.
    virtual void beginRun() override;

    /// Called event by event.
    virtual void event() override;

    /// Called when run ended.
    virtual void endRun() override;

    /// Called when processing ended.
    virtual void terminate() override;

    //! Define Histogram
    virtual void defineHisto() override;

  public:

    /// returns version of TRGGDLModule.
    std::string version(void) const;

  private: // Parameters

    /**< required input for TRGGRLInfo */
    StoreObjPtr<TRGGRLInfo> m_TRGGRLInfo;

    /**< output for TRGSummary */
    StoreObjPtr<TRGSummary> m_TRGSummary;

    /// Debug level.
    int _debugLevel;

    /// Config. file name.
    std::string _configFilename;

    /// Mode for TRGGDL simulation. 0th bit : fast simulation switch,
    /// 1st bit : firmware simulation switch.
    int _simulationMode;

    /// Switch for the fast simulation. 0:do everything, 1:stop after
    /// the track segment simulation. Default is 0.
    int _fastSimulationMode;

    /// Switch for the firmware simulation. 0:do nothing, 1:do everything
    int _firmwareSimulationMode;

    /// Switch for algorithm source. False when using local .alg file.
    bool _algFromDB;

    // Phase
    std::string _Phase;

    // Path to algorithm file
    std::string _algFilePath;

    /// Threshold to determine timing quality flag with MC truth: super fine
    double _timquality_threshold_sfin;

    /// Threshold to determine timing quality flag with MC truth: fine
    double _timquality_threshold_fine;

    bool m_simulateT0Jitter; /**< if True, L1 jitter is simulated by EventT0Generator*/

  private:

    /// A pointer to a TRGGDL;
    static TRGGDL* _gdl;

    StoreObjPtr<SimClockState> m_simClockState; /**< generated hardware clock state */

  protected:

    TDirectory* oldDir = nullptr;
    TDirectory* newDir = nullptr;

    TH1I* h_inp = nullptr;
    TH1I* h_ftd = nullptr;
    TH1I* h_psn = nullptr;

  };

} // namespace Belle2

#endif // TRGGDLModule_H
