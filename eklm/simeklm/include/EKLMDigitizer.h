/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Timofey Uglov                                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EKLMDIGITIZER_H
#define EKLMDIGITIZER_H


#include <eklm/dataobjects/EKLMStepHit.h>
#include <eklm/dataobjects/EKLMSimHit.h>
#include <eklm/dataobjects/EKLMDigit.h>
#include <map>
#include <vector>
#include  "CLHEP/Vector/ThreeVector.h"



#include <framework/datastore/StoreArray.h>


namespace Belle2 {


  /**
   * Digitize EKLMSimHits  to get EKLM StripHits.
   * @details
   * Usually called by eklmDigitizerModule.
   */
  class EKLMDigitizer {

  public:

    /**
     * Constructor.
     */
    EKLMDigitizer() {};

    /**
     * Destructor.
     */
    ~EKLMDigitizer() {};

    /**
     * Read hits from the store, sort sim hits and fill m_HitStripMap.
     */
    void readAndSortStepHits();

    /**
     * Create SimHits from StepHits using boost:graph mechanism.
     */
    void makeSimHits();

    /**
     * Read hits from the store, sort sim hits and fill m_HitStripMap.
     */
    void readAndSortSimHits();

    /**
     * Merges hits from the same strip. Create EKLMDigits.
     */
    void mergeSimHitsToStripHits(double);

  private:

    /** Map for EKLMStepHit sorting according sensitive volumes. */
    std::map<const G4VPhysicalVolume*, std::vector<EKLMStepHit*> >
    m_stepHitVolumeMap;

    /** Map for hits sorting according strip name. */
    std::map<const G4VPhysicalVolume*, std::vector<EKLMSimHit*> >
    m_HitStripMap;

    /** Vector of EKLMDigits. */
    std::vector<EKLMDigit*> m_HitVector;

    /** Sim hits vector. */
    std::vector<EKLMSimHit*> m_simHitsVector;

    /** SimHit storage initialization. */
    StoreArray<EKLMSimHit> m_simHitsArray;

    /** StripHit storage initialization. */
    StoreArray<EKLMDigit> m_stripHitsArray;

  };

} // end of namespace Belle2

#endif
