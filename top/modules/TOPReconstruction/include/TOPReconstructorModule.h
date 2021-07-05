/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
#include <top/dataobjects/TOPLikelihood.h>
#include <top/dataobjects/TOPPull.h>
#include <string>

namespace Belle2 {

  /**
   * TOP reconstruction module.
   */
  class TOPReconstructorModule : public Module {

  public:

    /**
     * Constructor
     */
    TOPReconstructorModule();

    /**
     * Destructor
     */
    virtual ~TOPReconstructorModule()
    {}

    /**
     * Initialize the Module.
     *
     * This method is called at the beginning of data processing.
     */
    virtual void initialize() override;

    /**
     * Event processor.
     */
    virtual void event() override;

  private:

    // Module steering parameters

    double m_minTime = 0;      /**< optional lower time limit for photons */
    double m_maxTime = 0;      /**< optional upper time limit for photons */
    int m_PDGCode = 0;   /**< PDG code of hypothesis to construct pulls */
    std::string m_topDigitCollectionName; /**< name of the collection of TOPDigits */
    std::string m_topLikelihoodCollectionName; /**< name of the collection of created TOPLikelihoods */
    std::string m_topPullCollectionName; /**< name of the collection of created TOPPulls */

    // datastore objects

    StoreArray<TOPDigit> m_digits; /**< collection of digits */
    StoreArray<Track> m_tracks; /**< collection of tracks */
    StoreArray<ExtHit> m_extHits; /**< collection of extrapolated hits */
    StoreArray<TOPBarHit> m_barHits; /**< collection of MCParticle hits at TOP */
    StoreObjPtr<TOPRecBunch> m_recBunch; /**< reconstructed bunch */
    StoreArray<TOPLikelihood> m_likelihoods; /**< collection of likelihoods */
    StoreArray<TOPPull> m_topPulls; /**< collection of pulls */

  };

} // Belle2 namespace

