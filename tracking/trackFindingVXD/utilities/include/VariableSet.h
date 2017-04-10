/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jonas Wagner                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once
#include <tracking/trackFindingVXD/utilities/Named.h>
#include <tracking/trackFindingVXD/trackQualityEstimators/QualityEstimatorBase.h>

namespace Belle2 {

  class MVAVariableSet : public std::vector<Named<float>> {

  public:

    void setVariable(std::string identifier, float value)
    {
      value = isnanf(value) ? -1 : value;
      auto entry = std::find(begin(), end(), identifier);
      if (entry != end()) {
        *(entry) = value;
      } else push_back(Named<float>(identifier, value));
    }

    // convenience method
    void setVariables(std::string identifier, QualityEstimationResults values)
    {
      setVariable(identifier + "_QI", values.qualityIndicator);

      float saneValue;
      saneValue = values.chiSquared ? *(values.chiSquared) : -1;
      setVariable(identifier + "_Chi2", saneValue);
      saneValue = values.pt ? *(values.pt) : -1;
      setVariable(identifier + "_Pt", saneValue);
      saneValue = values.pmag ? *(values.pmag) : -1;
      setVariable(identifier + "_PMag", saneValue);


      if (values.p) {
        auto vector = *(values.p);
        setVariable(identifier + "_P_Mag", vector.Mag());
        setVariable(identifier + "_P_Eta", vector.Eta());
        setVariable(identifier + "_P_Phi", vector.Phi());
      } else {
        setVariable(identifier + "_P_Mag", -1);
        setVariable(identifier + "_P_Eta", -1);
        setVariable(identifier + "_P_Phi", -1);
      }
    }
  };
}
