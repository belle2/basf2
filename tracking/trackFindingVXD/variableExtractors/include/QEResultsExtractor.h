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

#include <tracking/trackFindingVXD/variableExtractors/VariableExtractor.h>
#include <tracking/trackFindingVXD/trackQualityEstimators/QualityEstimatorBase.h>


namespace Belle2 {
  /// class to extract results from qualityEstimation
  class QEResultsExtractor : public VariableExtractor {
  public:

    /// Define names of variables that get extracted
    QEResultsExtractor(std::string method, std::vector<Named<float*>>& variableSet):
      VariableExtractor(), m_method(method)
    {
      addVariable(method + "_QI", variableSet);
      addVariable(method + "_Chi2", variableSet);
      addVariable(method + "_Pt", variableSet);
      addVariable(method + "_PMag", variableSet);
      addVariable(method + "_P_Mag", variableSet);
      addVariable(method + "_P_Eta", variableSet);
      addVariable(method + "_P_Phi", variableSet);
      addVariable(method + "_P_X", variableSet);
      addVariable(method + "_P_Y", variableSet);
      addVariable(method + "_P_Z", variableSet);
    }

    /// extract the actual variables and write into a variable set
    void extractVariables(QualityEstimationResults const& values)
    {
      m_variables.at(m_method + "_QI") = values.qualityIndicator;

      float saneValue;
      saneValue = values.chiSquared ? *(values.chiSquared) : -1;
      m_variables.at(m_method + "_Chi2") = saneValue;
      saneValue = values.pt ? *(values.pt) : -1;
      m_variables.at(m_method + "_Pt") = saneValue;
      saneValue = values.pmag ? *(values.pmag) : -1;
      m_variables.at(m_method + "_PMag") = saneValue;


      if (values.p) {
        auto& vector = *(values.p);
        m_variables.at(m_method + "_P_Mag") = vector.Mag();
        m_variables.at(m_method + "_P_Eta") = vector.Eta();
        m_variables.at(m_method + "_P_Phi") = vector.Phi();
        m_variables.at(m_method + "_P_X") = vector.X();
        m_variables.at(m_method + "_P_Y") = vector.Y();
        m_variables.at(m_method + "_P_Z") = vector.Z();
      } else {
        m_variables.at(m_method + "_P_Mag") = -1;
        m_variables.at(m_method + "_P_Eta") = -1;
        m_variables.at(m_method + "_P_Phi") = -1;
        m_variables.at(m_method + "_P_X") = -1;
        m_variables.at(m_method + "_P_Y") = -1;
        m_variables.at(m_method + "_P_Z") = -1;
      }
    }

  protected:
    /// name of estimation method
    std::string m_method;
  };
}
