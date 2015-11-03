/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Benjamin Oberhof                                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#ifndef MCMATCHERECLCLUSTERSMODULE_H_
#define MCMATCHERECLCLUSTERSMODULE_H_

#include <framework/core/Module.h>
#include <ecl/geometry/ECLGeometryPar.h>
#include <ecl/dataobjects/ECLSimHit.h>

#include <string>
#include <vector>
#include <queue>
#include <map>

#include <TRandom3.h>


namespace Belle2 {
  namespace ECL {

    /** Class to represent the hit of one cell */
    class MCMatcherECLClustersModule : public Module {

    public:
      /** Constructor.*/
      MCMatcherECLClustersModule();

      /** Destructor.*/
      virtual ~MCMatcherECLClustersModule();

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

    };
  } //ECL
} // end of Belle2 namespace

#endif // ECLDIGI_H
