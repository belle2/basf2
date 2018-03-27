//---------------------------------------------------------------
// $Id$
//---------------------------------------------------------------
// Filename : TRGECLRawdataAnalysisModule.cc
// Section  : TRG ECL
// Owner    : InSoo Lee / Yuuji Unno / SungHyun Kim
// Email    : islee@hep.hanyang.ac.kr / yunno@post.kek.jp / sunghun.kim@belle2.org
//---------------------------------------------------------------
// Description : A trigger module for TRG ECL
//---------------------------------------------------------------
// 1.00 : 2017/08/01 : Trigger
//                     using  Unpacker version 1.1
//---------------------------------------------------------------


#ifndef TRGECLRAWDATAANALYSISModule_H
#define TRGECLRAWDATAANALYSISModule_H

#include <string>
#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include "trg/ecl/TrgEclCluster.h"
#include "trg/ecl/dataobjects/TRGECLCluster.h"
#include "trg/ecl/dataobjects/TRGECLUnpackerStore.h"
#include "trg/ecl/dataobjects/TRGECLTiming.h"


namespace Belle2 {

  /*! A module of ETM */
  class TRGECLRawdataAnalysisModule : public Module {


  public:

    //    virtual ModulePtr newModule(){
    //      ModulePtr nm(new TRGECLModule(false)); return nm;
    //    };

    /** Constructor */
    TRGECLRawdataAnalysisModule();


    /** Destructor  */
    virtual ~TRGECLRawdataAnalysisModule();

    /** Initilizes TRGECLModule.*/
    virtual void initialize();

    /** Called when new run started.*/
    virtual void beginRun();

    /** Called event by event.*/
    virtual void event();

    /** Called when run ended.*/
    virtual void endRun();

    /** Called when processing ended.*/
    virtual void terminate();

  public:

    /** returns version of TRGECLModule.*/
    std::string version(void) const;

  private: // Parameters
    StoreArray<TRGECLUnpackerStore> m_TRGECLUnpackerStore; /**< output for TRGECLUnpackerStore */
    StoreArray<TRGECLCluster> m_TRGECLCluster; /**< output for TRGECLCluster */
    StoreArray<TRGECLTiming> m_TRGECLTiming; /**< output for TRGECLTiming */

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
    int  m_nRun;
    //!  Event number
    int  m_nEvent;
    //! Clustering option
    int _Clustering;
    //! Object of Clustering Util
    TrgEclCluster* obj_cluster;

  };

} // namespace Belle2

#endif // TRGECLModule_H
