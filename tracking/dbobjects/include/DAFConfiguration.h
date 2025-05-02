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

#include <tracking/dbobjects/DAFparameters.h>

namespace Belle2 {

  /** The payload containing all the DAFparameters configuration,
   * one for each different track fit option
   */
  class DAFConfiguration: public TObject {

  public:

    /**
     * Enum for identifying the type of track fit algorythm
     * ( or cosmic)
     */
    enum ETrackFitType {
      c_Default,
      c_CDConly,
      c_Cosmics,
      c_Custom1,
      c_Custom2,
      c_Custom3
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

    /** Get the DAFparameters for the specific track fit type
     */
    DAFparameters* getDAFparameters(DAFConfiguration::ETrackFitType trackFitType) const
    {
      std::map<DAFConfiguration::ETrackFitType, DAFparameters*>::const_iterator it = m_DAFparameters.find(trackFitType);
      if (it != m_DAFparameters.end()) {
        return it->second;
      } else {
        B2FATAL("Track Fit option " << trackFitType << " not found");
      }
    }

  private:
    /** The minimum allowed pValue for the convergence criterion */
    std::map<DAFConfiguration::ETrackFitType, DAFparameters*> m_DAFparameters;

    ClassDef(DAFConfiguration, 1);  /**< ClassDef, necessary for ROOT */
  };
}
