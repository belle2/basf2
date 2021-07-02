/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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

    std::map<std::string, TF1> m_RMSParameterizationFunctions; /**< RMS parameterization functions */

    ClassDef(ECLTrackClusterMatchingParameterizations, 1); /**< ClassDef */
  };
} // namespace Belle2
