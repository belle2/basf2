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
#include "trg/ecl/TrgEcl.h"

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

    /** Debug level.*/
    int _debugLevel;

    /** Config. file name.*/
    std::string _configFilename;

    /// A pointer to a TRGECL;*/
    //    TrgEcl* _ecl; */

  protected:

    std::string m_inColName;          // Input array name.
    std::string m_eclHitOutColName;   // Output array name for Xtal
    std::string m_eclTCHitOutColName;   // Output array name for TC

    // The current number of created hits in an event.
    // Used to fill the DataStore ECL array.
    int m_hitNum;
    int m_hitTCNum;

  private:

    // double m_timeCPU;        //  CPU time
    int    m_nRun;           //  Run number
    int    m_nEvent;         //  Event number

    int _Bhabha;  // Bhabha option
    int _Clustering; // Clustering option
    int _EventTiming;  // Eventtiming option


  };

} // namespace Belle2

#endif // TRGECLModule_H
