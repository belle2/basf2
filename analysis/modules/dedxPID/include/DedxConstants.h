#ifndef DEDXCONSTANTS_H
#define DEDXCONSTANTS_H

namespace Belle2 {
  const int c_Dedx_num_detectors = 3;
  const int c_Dedx_num_particles = 5;

  enum DedxDetector { c_PXD, c_SVD, c_CDC };

  //TODO think of something better.
  const int c_Dedx_pdg_codes[c_Dedx_num_particles] = { 211, 321, 11, 13, 2212 };

  /** List of particle type hypotheses. */
  enum DedxParticle { c_Pion, c_Kaon, c_Electron, c_Muon, c_Proton };
}
#endif
