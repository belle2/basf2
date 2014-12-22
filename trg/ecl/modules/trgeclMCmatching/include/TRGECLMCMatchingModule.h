//---------------------------------------------------------
// $Id$
//---------------------------------------------------------
// Filename : TRGECLModule.h
// Section  : TRG ECL
// Owner    : InSoo Lee/Yuuji Unno
// Email    : islee@hep.hanyang.ac.kr / yunno@post.kek.jp
//---------------------------------------------------------
// Description : A MC Matching module for TRG ECL
//---------------------------------------------------------
// 0.00 : 2014/12/22 : First version
//                     Relation between MCParicle and
//                     TRGECLHit.
//---------------------------------------------------------

#ifndef TRGECLMCMATCHINGMODULE_H_
#define TRGECLMCMATCHINGMODULE_H_

//basf2 framework headers
#include <framework/core/Module.h>
#include "trg/ecl/TrgEclMapping.h"

//#include <ecl/geometry/ECLGeometryPar.h>
//#include <ecl/dataobjects/ECLSimHit.h>

//C++/C standard lib elements.
#include <string>
#include <vector>
#include <queue>
#include <map>

//ROOT
#include <TRandom3.h>


namespace Belle2 {
  //namespace ECL {

  /** Class to represent the hit of one cell */
  class TRGECLMCMatchingModule : public Module {

  public:
    /** Constructor.*/
    TRGECLMCMatchingModule();

    /** Destructor.*/
    virtual ~TRGECLMCMatchingModule();

    /** Initialize variables, print info, and start CPU clock. */
    virtual void initialize();

    /** Nothing so far.*/
    virtual void beginRun();

    /** Actual digitization of all hits in the ECL.
     *
     *  The digitized hits are written into the DataStore.
     */
    virtual void event();

    /** Nothing so far. */
    virtual void endRun();

    /** Stopping of CPU clock.*/
    virtual void terminate();

    /** define a map for Primary Track*/
    typedef std::map< int, int>  PrimaryTrackMap;
    /** define a multimap for hits in showers */
    typedef std::multimap< int, int> MultiMap;


  private:



    /** CPU time     */
    double m_timeCPU;
    /** Run number   */
    int    m_nRun;
    /** Event number */
    int    m_nEvent;

    TrgEclMapping* _TCMap;


  };
  //} //TRGECL
} // end of Belle2 namespace

#endif // TRGECLDIGI_H
