/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <TObject.h>
#include <map>

#include <tracking/dbobjects/DAFParameters.h>

namespace Belle2 {

  /** The payload containing all the DAFParameters configuration,
   * one for each different track fit option
   */
  class DAFConfiguration: public TObject {

  public:

    /**
     * Enum for identifying the type of track fit algorythm
     * ( or cosmic)
     */
    enum ETrackFitType {
      c_Default = 0,     /**< default configuration */
      c_Cosmics = 1,     /**< configuration for cosmics data reconstruction */
      c_CDConly = 2      /**< configuration for the CDC-only track fitting */
    };

    /** Default constructor */
    DAFConfiguration() {}
    /** Destructor */
    ~DAFConfiguration() {}

    // /** Set the probability cut for the weight calculation for the hits
    // * @param probabilitycut probability cut used in setProbCut and addProbCut method from DAF
    // */
    // void setProbabilityCut(const float probabilitycut)
    // {
    //   m_ProbabilityCut = probabilitycut;
    // }

    /** Get the DAFParameters for the specific track fit type
     */
    DAFParameters* getDAFParameters(DAFConfiguration::ETrackFitType trackFitType) const
    {
      std::map<DAFConfiguration::ETrackFitType, DAFParameters*>::const_iterator it = m_DAFParameters.find(trackFitType);
      if (it != m_DAFParameters.end()) {
        return it->second;
      } else {
        B2FATAL("Track Fit option " << trackFitType << " not found");
      }
    }

  private:
    /** The minimum allowed pValue for the convergence criterion */
    std::map<DAFConfiguration::ETrackFitType, DAFParameters*> m_DAFParameters;

    ClassDef(DAFConfiguration, 1);  /**< ClassDef, necessary for ROOT */
  };
}
