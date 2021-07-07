/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

//STL
#include <string>

//Framework
#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>

namespace Belle2 {
  class ECLDebugHit;
  class ECLSimHit;

  /** Class to represent the hit of one cell */
  class ECLHitDebugModule : public Module {

  public:
    /** Constructor.*/
    ECLHitDebugModule();

    /** Destructor.*/
    virtual ~ECLHitDebugModule();

    /** Initialize variables, print info, and start CPU clock. */
    virtual void initialize() override;

    /** Nothing so far.*/
    virtual void beginRun() override;

    /** Actual digitization of all hits in the ECL.
     *
     *  The digitized hits are written into the DataStore.
     */
    virtual void event() override;

    /** Nothing so far. */
    virtual void endRun() override;

    /** Stopping of CPU clock.*/
    virtual void terminate() override;

  protected:
    /** Input array name. */
    std::string m_inColName;
    /** Output array name. */
    std::string m_eclHitOutColName;

    /** The current number of created hits in an event. Used to fill the DataStore ECL array.*/
    int m_hitNum{ -1};

  private:
    /** CPU time     */
    double m_timeCPU{ -1.0};
    /** Run number   */
    int    m_nRun{ -1};
    /** Event number */
    int    m_nEvent{ -1};

    //DataStore variables
    StoreArray<ECLDebugHit> m_eclDebugHits; /**< ECLDebugHit datastore object */
    StoreArray<ECLSimHit> m_eclSimArray; /**< StoreArray ECLSimHit */

  };
} // end of Belle2 namespace
