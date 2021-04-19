/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2020 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/datastore/RelationsObject.h>


namespace Belle2 {

  /**
   * Simulated hardware clock state.
   * This class is used to distribute to sub-detectors the clock state generated in EventT0Generator.
   *
   * Note: StoreObjPtr must be registered as optional and the validity must be checked before using it,
   * because the module that creates it is optional in add_simulation.
   */
  class SimClockState: public RelationsObject {

  public:

    /**
     * Default constructor
     */
    SimClockState()
    {}

    /**
     * Sets the number of system clock ticks since last revo9 marker.
     * @param count number of system clock ticks
     */
    void setRevo9Count(unsigned count) {m_revo9Count = count;}

    /**
     * Sets RF bucket number of the collision
     * @param bucket bucket number
     */
    void setBucketNumber(unsigned bucket) {m_bucketNumber = bucket;}

    /**
     * Sets beam revolution count since last revo9 marker.
     * @param beamCycle beam revolution count
     */
    void setBeamCycleNumber(unsigned beamCycle) {m_beamCycle = beamCycle;}

    /**
     * Returns the number of system clock ticks since last revo9 marker.
     * System clock runs with a frequency of RF/4 ~ 127 MHz.
     * @return number of system clock ticks
     */
    unsigned getRevo9Count() const {return m_revo9Count;}

    /**
     * Returns RF bucket number of the collision
     * @return bucket number
     */
    unsigned getBucketNumber() const {return m_bucketNumber;}

    /**
     * Returs beam revolution count since last revo9 marker.
     * @return beam revolution count
     */
    unsigned getBeamCycleNumber() const {return m_beamCycle;}

    /**
     * Returns SVD trigger bin.
     * Default value for offset determined on rawdata of run 7/1500 using SVDEventInfo and TOPRawDigits.
     * @param offset offset
     * @return SVD trigger bin
     */
    unsigned getSVDTriggerBin(unsigned offset = 2) const {return (m_revo9Count + offset) % 4;}

    /**
    * Returns CDC trigger bin.
    * Default value for offset put as 0 for now.
    * @param offset offset
    * @return CDC trigger bin
    */
    unsigned getCDCTriggerBin(unsigned offset = 0) const {return (m_revo9Count + offset) % 4;}



  private:

    unsigned m_revo9Count = 0; /**< number of system clock ticks since last revo9 marker */
    unsigned m_bucketNumber = 0; /**< RF bucket number of the collision */
    unsigned m_beamCycle = 0; /**< beam revolution count since last revo9 marker */

    ClassDef(SimClockState, 1); /**< ClassDef */

  };

} //Belle2 namespace

