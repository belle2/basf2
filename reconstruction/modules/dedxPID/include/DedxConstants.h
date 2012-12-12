/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Pulvermacher
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef DEDXCONSTANTS_H
#define DEDXCONSTANTS_H

namespace Belle2 {
  namespace Dedx {
    /** total number of tracking detectors considered */
    const int c_num_detectors = 3;

    /** list of trackig detectors, starting with innermost detector */
    enum Detector { c_PXD, c_SVD, c_CDC };

    /** total number of particle hypotheses */
    const int c_num_particles = 5;

    /** List of particle type hypotheses. */
    enum Particle { c_Pion, c_Kaon, c_Electron, c_Muon, c_Proton };

    /** List of PDG codes corresponding to Dedx::Particle enum */
    const int c_pdg_codes[c_num_particles] = { 211, 321, 11, 13, 2212 };

  }
}
#endif
