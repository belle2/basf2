/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Guofu Cao, Martin Heck                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#ifndef ECLHITMODULE_H_
#define ECLHITMODULE_H_

//basf2 framework headers
#include <framework/core/Module.h>

#include <ecl/hitecl/ECLSimHit.h>

//C++/C standard lib elements.
#include <string>
#include <vector>
#include <queue>
#include <map>

//ROOT
#include <TRandom3.h>


namespace Belle2 {


  /** The Class for Detailed Digitization of ECL.
   *
   *  Currently a double Gaussian with steerable parameters is used for the digitization.
   *  If there are two or more hits in one cell, only the shortest drift length is selected.
   *  The signal amplitude is the sum of all hits deposited energy in this cell.
   *
   *  @todo More details will be considered soon, like X-T function, transfer time and so on.
   *        There is a bug in the Relation between the first ECLHit and the ECLSignal.
   *        This has to be corrected, but it is not critical for the current release.
   *        Therefore I stop working on it for the moment and focus on the parts, that have to be finished this week.
   *
   *  @author  Guofu Cao;<br>
   *           Issues regarding the Relations between SimHits and Hits: Martin Heck;
   *
   */

  class ECLHitModule : public Module {

  public:

    typedef std::vector<ECLSimHit*>      ECLSimHitVec;   /**< For input from Geant4 simulation.*/
//poyuan    typedef std::map< int, ECLSignal *>  ECLSignalMap;   /**< Map of Cell ID -> signal.*/
//poyuan    typedef std::pair< int, ECLSignal *> vpair;          /**< Pair of ECLSignalMap. */

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


    void printModuleParams() const;
    /** Method to print SimHit information. */

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
