/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Wessel                                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>
#include <framework/datastore/StoreArray.h>

#include <string>
#include <vector>

namespace Belle2 {
  class DATCONSVDDigit;
  class SVDShaperDigit;
  class SVDTrueHit;
  class MCParticle;

  class ModuleParamList;

  /**
   * Findlet for loading the seeds from the data store.
   * Also, the tracks are fitted and only the fittable tracks are passed on.
   *
   * If a direction != "invalid" is given, the relations of the tracks to the given store array are checked.
   * If there is a relation with the weight equal to the given direction (meaning there is already a
   * partner for this direction), the track is not passed on.
   */
  class SVDShaperDigitConverter : public TrackFindingCDC::Findlet<DATCONSVDDigit, DATCONSVDDigit> {
    /// Parent class
    using Super = TrackFindingCDC::Findlet<DATCONSVDDigit, DATCONSVDDigit>;

  public:
    /// Add the subfindlets
    SVDShaperDigitConverter();

    virtual ~SVDShaperDigitConverter() = default;

    /// Expose the parameters of the sub findlets.
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override;

    /// Create the store arrays
    void initialize() override;

    /// Load in the reco tracks and the hits
    void apply(std::vector<DATCONSVDDigit>& svdUDigits, std::vector<DATCONSVDDigit>& svdVDigits) override;

  private:
    // Parameters
    /// SVDShaperDigits StoreArray name
    std::string m_param_storeSVDShaperDigitsName = "SVDShaperDigits";
    /// DATCONSVDDigits StoreArray name
    std::string m_param_storeDATCONSVDDigitsName = "DATCONSVDDigits";
    /// SVDTrueHits StoreArray name
    std::string m_param_storeTrueHitsName = "SVDTrueHits";
    /// MCParticles StoreArray name
    std::string m_param_storeMCParticlesName = "MCParticles";

    /// save relations to SVDTrueHits and MCParticles
    bool m_param_saveStoreArrays = false;


    StoreArray<SVDShaperDigit> m_storeSVDShaperDigits;    /**< SVDShaperDigits StoreArray */
    StoreArray<DATCONSVDDigit> m_storeDATCONSVDDigits;    /**< DATCONSVDDigits StoreArray */
    StoreArray<SVDTrueHit> m_storeTrueHits;               /**< SVDTrueHits StoreArray */
    StoreArray<MCParticle> m_storeMCParticles;            /**< MCParticles StoreArray */

  };
}
