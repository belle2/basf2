//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : TRGGRLProjectsModule.h
// Section  : TRG GRL
// Owner    : Chunhua Li
// Email    : chunhua.li@unimelb.edu.au
//-----------------------------------------------------------------------------
// Description : A trigger module for GRL
//-----------------------------------------------------------------------------
// 0.00 : 2017/03/01 : First version
//-----------------------------------------------------------------------------

#ifndef TRGGRLProjectsModule_H
#define TRGGRLProjectsModule_H

#include <string>
#include "framework/core/Module.h"
#include <trg/grl/dataobjects/TRGGRLInfo.h>
#include <framework/database/DBObjPtr.h>
#include <mdst/dbobjects/TRGGDLDBInputBits.h>

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
    virtual ~TRGGRLProjectsModule() {};

    /// Initilizes TRGGRLProjectsModule.
    virtual void initialize() override;

    /// Called when new run started.
    virtual void beginRun() override;

    /// Called event by event.
    virtual void event() override;

    /// Called when run ended.
    virtual void endRun() override;

    /// Called when processing ended.
    virtual void terminate() override;


  public:

    /// returns version of TRGGRLProjectsModule.
    //std::string version(void) const;

  private: // Parameters
//    StoreObjPtr<TRGGRLInfo> m_TRGGRLInfo; /**< output for TRGGRLInfo */

    /// Debug level.
    int _debugLevel;

    /**choose the corresponding trigger menu of Belle2 phase*/
    std::string m_belle2phase;

    /// Config. file name.
    std::string m_configFilename;

    /// Mode for TRGGRL simulation. 0th bit : fast simulation switch,
    /// 1st bit : firmware simulation switch.
    int m_simulationMode;

    /// Switch for the fast simulation. 0:do everything, 1:stop after
    /// the track segment simulation. Default is 0.
    int m_fastSimulationMode;

    /// Switch for the firmware simulation. 0:do nothing, 1:do everything
    int m_firmwareSimulationMode;

    /** Name of the StoreArray holding the matched 2D tracks */
    std::string m_2DmatchCollectionName;
    /**the matched 2d track list by phi matching*/
    std::string m_phimatch_tracklist;
    /** Name of the StoreArray holding the matched 3D tracks */
    std::string m_3DmatchCollectionName;
    /**the matched 2d track list by KLM matching*/
    std::string m_klmmatch_tracklist;
    /** Name of the StoreArray holding the tracks made by the 2D finder */
    std::string m_2DfinderCollectionName;
    /** Name of the StoreArray holding the tracks made by the 2D fitter */
    std::string m_2DfitterCollectionName;
    /** Name of the StoreArray holding the tracks made by the 3D fitter. */
    std::string m_3DfitterCollectionName;
    /** Name of the StoreArray holding projects information from grl */
    std::string m_TrgGrlInformationName;
    /** Name of the StoreArray holding projects information from grlphoton */
    std::string m_grlphotonlist;

    /** Name of the StoreArray holding the tracks made by NN*/
    std::string m_NNCollectionName;
    /** Name of the StoreArray holding the eclclusters*/
    std::string m_TrgECLClusterName;
    /** Name of the StoreArray holding the ecl trg result*/
    std::string m_TrgECLTrgsName;
    /**Name of the collection of KLM TRG*/
    std::string m_KLMTrgSummaryName;
    /**the collection of energy threshold used in the projects*/
    std::vector<double> m_energythreshold;
    /**ecl cluster time window to suppress beam induced background*/
    double m_eclClusterTimeWindow;
    /**Data base of GDL input bits**/
    DBObjPtr<TRGGDLDBInputBits> m_InputBitsDB;

    /**convert the unit of angle from rad to degree*/
    //double m_RtD;


  };

} // namespace Belle2

#endif // TRGGRLProjectsModule_H
