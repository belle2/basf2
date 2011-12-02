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

//#include <eklm/eklmutils/EKLMutils.h>


#include <eklm/dataobjects/EKLMStepHit.h>
#include <eklm/dataobjects/EKLMSimHit.h>
#include <eklm/dataobjects/EKLMStripHit.h>
#include <map>
#include <vector>
#include  "CLHEP/Vector/ThreeVector.h"



#include <framework/datastore/StoreArray.h>


namespace Belle2 {


  /**
   * Digitize EKLMSimHits  to get EKLM StripHits
   * Usually called by eklmDigitizerModule
   */
  class EKLMDigitizer {

  public:

    /**
     * Constructor
     */
    EKLMDigitizer() {};

    /**
     * Destructor
     */
    ~EKLMDigitizer() {};



    /**
     * Read hits from the store, sort sim hits and fill m_HitStripMap
     */
    void readAndSortStepHits();

    /**
     * Creates SimHits from StepHits using boost:graph mechanism
     */
    void makeSimHits();

    //-------------------------------------------------------
    /**
     * Read hits from the store, sort sim hits and fill m_HitStripMap
     */
    void readAndSortSimHits();


    /**
     * merges hits from the same strip. Creates EKLMStripHits
     */
    void mergeSimHitsToStripHits();




  private:

    /**
     * std::map for EKLMStepHit
     *    sorting according sensitive volumes
     *   < PhysVol1 *  , vector< EKLMStepHit1*, EKLMStepHit2*, EKLMStepHit3* ...    > >
     *   < PhysVol2 *  , vector< EKLMStepHit1*, EKLMStepHit2*, EKLMStepHit3* ...    > >
     *   < PhysVol3 *  , vector< EKLMStepHit1*, EKLMStepHit2*, EKLMStepHit3* ...    > >
     */
    std::map<const G4VPhysicalVolume *, std::vector<EKLMStepHit*> > m_stepHitVolumeMap;


    //-------------------------------------------------------

    /**
     * std::map for hits sorting according strip name
     */
    std::map<const G4VPhysicalVolume *, std::vector<EKLMSimHit*> > m_HitStripMap;

    /**
     * vector of EKLMStripHits
     */
    std::vector<EKLMStripHit*> m_HitVector;

    /**
     * sim hits vector
     */
    std::vector<EKLMSimHit*> m_simHitsVector;

    /**
     * SimHit storage initialization
     */
    StoreArray<EKLMSimHit> m_simHitsArray;

    /**
     * StripHit storage initialization
     */
    StoreArray<EKLMStripHit> m_stripHitsArray;


  };

  //! Extern allocator
  extern G4Allocator<EKLMDigitizer> EKLMDigitizerAllocator;

} // end of namespace Belle2

#endif
