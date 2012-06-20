/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Poyuan Chen                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#ifndef ECLHITMODULE_H_
#define ECLHITMODULE_H_

//basf2 framework headers
#include <framework/core/Module.h>

#include <ecl/dataobjects/ECLSimHit.h>

//C++/C standard lib elements.
#include <string>
#include <vector>
#include <queue>
#include <map>

//ROOT
#include <TRandom3.h>


namespace Belle2 {
  class ECLHitModule : public Module {

  public:

    typedef std::vector<ECLSimHit*>      ECLSimHitVec;   /**< For input from Geant4 simulation.*/

    /** Constructor.*/
    ECLHitModule();

    /** Destructor.*/
    virtual ~ECLHitModule();

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

    std::string m_inColName;                /**< Input array name. */
    std::string m_eclHitOutColName;         /**< Output array name. */
    std::string m_relColNameSimHitToHit;    /**< Relation collection name - ecl signal (Digit)  <-> MCParticle */
    std::string m_relColNameMCToSim;        /**< Relation collection name - MCParticle        <-> SimTrkHit */

    int m_hitNum; /**< The current number of created hits in an event. Used to fill the DataStore ECL array.*/


  private:

    double m_timeCPU;                /*!< CPU time     */
    int    m_nRun;                   /*!< Run number   */
    int    m_nEvent;                 /*!< Event number */

  };

} // end of Belle2 namespace

#endif // ECLDIGI_H
