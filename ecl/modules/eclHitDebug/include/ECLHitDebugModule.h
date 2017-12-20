/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Poyuan Chen                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#pragma once

//basf2 framework headers
#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <ecl/geometry/ECLGeometryPar.h>
#include <ecl/dataobjects/ECLSimHit.h>

//C++/C standard lib elements.
#include <string>
#include <vector>
#include <queue>
#include <map>

//ROOT
#include <TRandom3.h>


namespace Belle2 {
  class ECLDebugHit;
  namespace ECL {



    /** Class to represent the hit of one cell */
    class ECLHitDebugModule : public Module {

    public:
      /** Constructor.*/
      ECLHitDebugModule();

      /** Destructor.*/
      virtual ~ECLHitDebugModule();

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

    protected:
      /** Input array name. */
      std::string m_inColName;
      /** Output array name. */
      std::string m_eclHitOutColName;

      /** The current number of created hits in an event. Used to fill the DataStore ECL array.*/
      int m_hitNum;

    private:
      /** CPU time     */
      double m_timeCPU;
      /** Run number   */
      int    m_nRun;
      /** Event number */
      int    m_nEvent;

      //DataStore variables
      StoreArray<ECLDebugHit> m_eclDebugHits; /**< ECLDebugHit datastore object */

    };
  } //ECL
} // end of Belle2 namespace
