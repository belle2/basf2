//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : TRGGRLProjectsModule.h
// Section  : TRG GRL
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A trigger module for GRL
//-----------------------------------------------------------------------------
// 0.00 : 2013/12/13 : First version
//-----------------------------------------------------------------------------

#ifndef TRGGRLProjectsModule_H
#define TRGGRLProjectsModule_H

#include <string>
#include "framework/core/Module.h"
#include "trg/grl/TRGGRL.h"
//#include "trg/cdc/dataobjects/CDCTriggerTrack.h"

namespace Belle2 {

/// A module to simulate the Global Decision Logic. This module
/// requires sub-trigger simulation data (CDC, ECL, TOP, and KLM). The
/// output is GRL response.

  class TRGGRLProjectsModule : public Module {

  public:

    /// Constructor
    TRGGRLProjectsModule();
//  TRGGRLProjectsModule(const std::string & type);

    /// Destructor
    virtual ~TRGGRLProjectsModule();

    /// Initilizes TRGGRLProjectsModule.
    virtual void initialize();

    /// Called when new run started.
    virtual void beginRun();

    /// Called event by event.
    virtual void event();

    /// Called when run ended.
    virtual void endRun();

    /// Called when processing ended.
    virtual void terminate();


  public:

    /// returns version of TRGGRLProjectsModule.
    std::string version(void) const;

  private: // Parameters

    /// Debug level.
    int _debugLevel;

    /// Config. file name.
    std::string _configFilename;

    /// Mode for TRGGRL simulation. 0th bit : fast simulation switch,
    /// 1st bit : firmware simulation switch.
    int _simulationMode;

    /// Switch for the fast simulation. 0:do everything, 1:stop after
    /// the track segment simulation. Default is 0.
    int _fastSimulationMode;

    /// Switch for the firmware simulation. 0:do nothing, 1:do everything
    int _firmwareSimulationMode;

    /** Name of the StoreArray holding the matched 2D tracks */
    std::string m_2DmatchCollectionName;
    /** Name of the StoreArray holding the matched 3D tracks */
    std::string m_3DmatchCollectionName;
    /** Name of the StoreArray holding the tracks made by the 2D finder */
    std::string m_2DfinderCollectionName;
    /** Name of the StoreArray holding the tracks made by the 2D fitter */
    std::string m_2DfitterCollectionName;
    /** Name of the StoreArray holding the tracks made by the 3D fitter. */
    std::string m_3DfitterCollectionName;
    /** Name of the StoreArray holding projects information from grk */
    std::string m_TrgGrlInformationName;

    /** Name of the StoreArray holding the tracks made by NN*/
    std::string m_NNCollectionName;
    /** Name of the StoreArray holding the eclclusters*/
    std::string m_TrgECLClusterName;
    /** Name of the StoreArray holding the ecl trg result*/
    std::string m_TrgECLTrgsName;
    /**the collection of energy threshold used in the projects*/
    std::vector<double> m_energythreshold;
    /**ecl cluster time window to suppress beam induced background*/
    double m_eclClusterTimeWindow;


  };

} // namespace Belle2

#endif // TRGGRLProjectsModule_H
