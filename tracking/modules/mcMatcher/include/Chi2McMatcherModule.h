/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/


#pragma once

#include <fstream> // only for debugging 

#include <framework/core/Module.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/MCParticle.h>
#include <framework/datastore/StoreArray.h>

namespace Belle2 {
  /**
   * Monte Carlo matcher using the helix parameters for matching by chi2-method
   */
  class Chi2McMatcherModule : public Module {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    Chi2McMatcherModule();


    /** Register input and output data */
    void initialize() override;


    /** Do matching for each event */
    void event() override;

  private: //Parameters

    /** Parameter : Defines the Cut Off values for each charged particle. The Cut Off values in the list for
     *              the  different pdgs have to be in the following order  [11,13,211,2212,321,1000010020]
     */
    std::vector<double> m_param_CutOffs;
    /** Parameter: Posebility to switch beween ROOT and Eigen for inversion of the covariance matrix */
    std::string m_param_linalg;
    /** Variable: Makes MCParticles and Tracks available in whole class */
    StoreArray<MCParticle> MCParticles;
    StoreArray<Track> Tracks;
  };
}
