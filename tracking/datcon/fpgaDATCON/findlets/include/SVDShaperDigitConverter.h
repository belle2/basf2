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
  class ModuleParamList;

  /**
   * Findlet for converting SVDShaperDigits into DATCONSVDDigits.
   * DATCONSVDDigtis are a simplified digit that contains different information of the SVD hits to have information
   * similar as in the real FPGA DATCON.
   */
  class SVDShaperDigitConverter : public TrackFindingCDC::Findlet<DATCONSVDDigit, DATCONSVDDigit> {
    /// Parent class
    using Super = TrackFindingCDC::Findlet<DATCONSVDDigit, DATCONSVDDigit>;

  public:
    /// Add the subfindlets
    SVDShaperDigitConverter();

    /// default destructor
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

    /// SVDShaperDigit StoreArray
    StoreArray<SVDShaperDigit> m_storeSVDShaperDigits;
  };
}
