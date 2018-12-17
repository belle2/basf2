/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Frank Meier                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <TF1.h>
#include <TObject.h>

namespace Belle2 {

  /**
   * Class to hold the parameterizations of the RMS
   * for the difference in polar and azimuthal angle between tracks and ECL clusters.
   */
  class ECLTrackClusterMatchingParameterizations: public TObject {

  public:

    /** Default constructor */
    ECLTrackClusterMatchingParameterizations() {};

    /** Constructor */
    explicit ECLTrackClusterMatchingParameterizations(const std::map<std::string, TF1>& RMSParameterizationFunctions):
      m_RMSParameterizationFunctions(RMSParameterizationFunctions)
    {
    }

    /** Destructor. */
    ~ECLTrackClusterMatchingParameterizations() {}

    /** Get RMS parameterization */
    const std::map<std::string, TF1>& getRMSParameterizations() const {return m_RMSParameterizationFunctions;}

  private:

    std::map<std::string, TF1> m_RMSParameterizationFunctions;

    ClassDef(ECLTrackClusterMatchingParameterizations, 1); /**< ClassDef */
  };
} // namespace Belle2
