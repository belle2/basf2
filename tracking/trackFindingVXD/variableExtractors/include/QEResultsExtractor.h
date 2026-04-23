/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <tracking/trackFindingVXD/variableExtractors/VariableExtractor.h>
#include <tracking/trackFindingVXD/trackQualityEstimators/QualityEstimatorBase.h>


namespace Belle2 {
  /// class to extract results from qualityEstimation
  class QEResultsExtractor : public VariableExtractor {
  public:

    /// Define names of variables that get extracted
    QEResultsExtractor(const std::string& method, std::vector<TrackingUtilities::Named<float*>>& variableSet,
                       const std::string& prefix = ""):
      VariableExtractor(), m_method(method), m_prefix(prefix + method)
    {
      addVariable(m_prefix + "_QI", variableSet);
      addVariable(m_prefix + "_Chi2", variableSet);
      addVariable(m_prefix + "_Pt", variableSet);
      addVariable(m_prefix + "_PMag", variableSet);
      addVariable(m_prefix + "_P_Mag", variableSet);
      addVariable(m_prefix + "_P_Eta", variableSet);
      addVariable(m_prefix + "_P_Phi", variableSet);
      addVariable(m_prefix + "_P_X", variableSet);
      addVariable(m_prefix + "_P_Y", variableSet);
      addVariable(m_prefix + "_P_Z", variableSet);
    }

    /// extract the actual variables and write into a variable set
    void extractVariables(QualityEstimationResults const& values)
    {
      m_variables.at(m_prefix + "_QI") = values.qualityIndicator;

      float saneValue;
      saneValue = values.chiSquared ? *(values.chiSquared) : -1;
      m_variables.at(m_prefix + "_Chi2") = saneValue;
      saneValue = values.pt ? *(values.pt) : -1;
      m_variables.at(m_prefix + "_Pt") = saneValue;
      saneValue = values.pmag ? *(values.pmag) : -1;
      m_variables.at(m_prefix + "_PMag") = saneValue;


      if (values.p) {
        auto& vector = *(values.p);
        m_variables.at(m_prefix + "_P_Mag") = vector.Mag();
        m_variables.at(m_prefix + "_P_Eta") = vector.Eta();
        m_variables.at(m_prefix + "_P_Phi") = vector.Phi();
        m_variables.at(m_prefix + "_P_X") = vector.X();
        m_variables.at(m_prefix + "_P_Y") = vector.Y();
        m_variables.at(m_prefix + "_P_Z") = vector.Z();
      } else {
        m_variables.at(m_prefix + "_P_Mag") = -1;
        m_variables.at(m_prefix + "_P_Eta") = -1;
        m_variables.at(m_prefix + "_P_Phi") = -1;
        m_variables.at(m_prefix + "_P_X") = -1;
        m_variables.at(m_prefix + "_P_Y") = -1;
        m_variables.at(m_prefix + "_P_Z") = -1;
      }
    }

  protected:
    /// name of estimation method
    std::string m_method;
    /// prefix that will be added before the variable names. Includes the name of method.
    std::string m_prefix;
  };
}
