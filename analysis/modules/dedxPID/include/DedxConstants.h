#ifndef DEDXCONSTANTS_H
#define DEDXCONSTANTS_H

namespace Belle2 {
  /** total number of tracking detectors considered */
  const int c_Dedx_num_detectors = 3;

  /** list of trackig detectors, starting with innermost detector */
  enum DedxDetector { c_PXD, c_SVD, c_CDC };

  /** total number of particle hypotheses */
  const int c_Dedx_num_particles = 5;

  /** List of particle type hypotheses. */
  enum DedxParticle { c_Pion, c_Kaon, c_Electron, c_Muon, c_Proton };

  /** List of PDG codes corresponding to DedxParticle enum */
  const int c_Dedx_pdg_codes[c_Dedx_num_particles] = { 211, 321, 11, 13, 2212 };

}
#endif
