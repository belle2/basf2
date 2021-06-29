/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2021 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Umberto Tamponi                                          *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#pragma once

#include <framework/core/Module.h>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <mdst/dataobjects/Track.h>
#include <tracking/dataobjects/ExtHit.h>
#include <top/dataobjects/TOPDigit.h>
#include <top/dataobjects/TOPRecBunch.h>
#include <top/dataobjects/TOPBarHit.h>
#include <top/dataobjects/TOPLikelihoodScanResult.h>
#include <string>

namespace Belle2 {
  /**
   * A module to perform the TOP PID likelihood scan and find the actual minimum as function of the mass
   */
  class TOPLLScannerModule : public Module {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    TOPLLScannerModule();

    /** Default destructor, Nothing to see here */
    ~TOPLLScannerModule() override;

    /** Setup the storearrays */
    void initialize() override;

    /** Performs the scan */
    void event() override;

    /** Saves the results */
    void terminate() override;


  private:

    /** Finds best fit value and confidence interval form a LL vie direct scan */
    void scanLikelihood(std::vector<float>masses, std::vector<float>logLs, float deltaLL, float& maxLL, float& massMax,
                        float& minMassRange, float& maxMassRange);

    std::vector<float> m_massPoints = {0}; /**< vector with the mass points used in the coarse scan */
    short m_nFineScanPoints = 10; /**< number of points for the fine-graned scan */
    // datastore objects
    StoreArray<TOPDigit> m_digits; /**< collection of digits */
    StoreArray<Track> m_tracks; /**< collection of tracks */
    StoreArray<ExtHit> m_extHits; /**< collection of extrapolated hits */
    StoreArray<TOPBarHit> m_barHits; /**< collection of MCParticle hits at TOP */
    StoreObjPtr<TOPRecBunch> m_recBunch; /**< reconstructed bunch */
    StoreArray<TOPLikelihoodScanResult> m_likelihoodScanResults; /**< collection of likelihoods */
  };
}
