//---------------------------------------------------------
// $Id$
//---------------------------------------------------------
// Filename : TRGECLModule.h
// Section  : TRG ECL
// Owner    : InSu Lee/Yuuji Unno
// Email    : islee@hep.hanyang.ac.kr / yunno@post.kek.jp
//---------------------------------------------------------
// Description : A trigger module for TRG ECL
//---------------------------------------------------------
// 0.00 : 2011/11/12 : First version
//---------------------------------------------------------

#ifndef TRGECLModule_H
#define TRGECLModule_H

#include <string>
#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <framework/database/DBArray.h>
#include <framework/database/DBObjPtr.h>


#include "trg/ecl/TrgEclMaster.h"
#include "trg/ecl/dataobjects/TRGECLHit.h"
#include "trg/ecl/dataobjects/TRGECLTrg.h"
#include "trg/ecl/dataobjects/TRGECLCluster.h"
#include "trg/ecl/dbobjects/TRGECLETMPara.h"

namespace Belle2 {

  /*! A module of ETM */
  class TRGECLModule : public Module {


  public:

    //    virtual ModulePtr newModule(){
    //      ModulePtr nm(new TRGECLModule(false)); return nm;
    //    };

    /** Constructor */
    TRGECLModule();


    /** Destructor  */
    virtual ~TRGECLModule();

    /** Initilizes TRGECLModule.*/
    virtual void initialize() override;

    /** Called when new run started.*/
    virtual void beginRun() override;

    /** Called event by event.*/
    virtual void event() override;

    /** Called when run ended.*/
    virtual void endRun() override;

    /** Called when processing ended.*/
    virtual void terminate() override;

  public:

    /** returns version of TRGECLModule.*/
    std::string version(void) const;

  private: // Parameters

    /** Debug level.*/
    int _debugLevel;

    /** Config. file name.*/
    std::string _configFilename;

    /// A pointer to a TRGECL;*/
    //    TrgEcl* _ecl; */

  protected:
    //! Input array name.
    std::string m_inColName;
    //! Output array name for Xtal
    std::string m_eclHitOutColName;
    //! Output array name for TC
    std::string m_eclTCHitOutColName;

    //! The current number of created hits in an event.
    //! Used to fill the DataStore ECL array.
    int m_hitNum;
    //! TC Hit number
    int m_hitTCNum;

  private:

    // double m_timeCPU;        //  CPU time
    //!  Run number
    int m_nRun;
    //!  Event number
    int m_nEvent;
    //! Bhabha option
    int _Bhabha;
    //! Clustering option
    int _Clustering;
    //! Cluster Limit
    int _ClusterLimit;
    //! Eventtiming option
    int _EventTiming;
    //! Trigger decision time window
    double _TimeWindow;
    //! Trigger decision overlap window in oder to avoid boundary effect
    double _OverlapWindow;
    //! # of considered TC in energy weighted Timing method
    int _NofTopTC;
    //! Event selection
    int _SelectEvent;
    //! Flag to use Condition DB
    int _ConditionDB;
    //! 2D Bhabha Energy Threshold
    std::vector<double> _2DBhabhaThresholdFWD;
    //! 2D Bhabha Energy Threshold
    std::vector<double> _2DBhabhaThresholdBWD;
    //! 3D Bhabha Energy Threshold
    std::vector<double> _3DBhabhaThreshold;
    //! Total Energy Theshold (low, high, lum)
    std::vector<double> _TotalEnergy;
    //! Low Multiplicity Threshold
    std::vector<double> _LowMultiThreshold;

    StoreArray<TRGECLHit> m_TRGECLHit; /**< output for TRGECLHit */
    StoreArray<TRGECLTrg> m_TRGECLTrg; /**< output for TRGECLTrg */
    StoreArray<TRGECLCluster> m_TRGECLCluster; /**< output for TRGECLCluster */
    DBArray<TRGECLETMPara> m_ETMPara; /** ETN Parameters */

  };

} // namespace Belle2

#endif // TRGECLModule_H
