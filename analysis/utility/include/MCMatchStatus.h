enum  MCMatchStatus {
  c_MissFSR             = 1,  /**< bit 0: Final State Radiation (FSR) photon is missing. */
  c_MisID               = 2,  /**< bit 1: One of the charged final state particles is mis-identified. */
  c_MissGamma           = 4,  /**< bit 2: Photon (not FSR) is missing. */
  c_MissMassiveParticle = 8,  /**< bit 3: Massive particle is missing. */
  c_MissNeutrino        = 16, /**< bit 4: Neutrino is missing. */
  c_MissKlong           = 32  /**< bit 5: Klong is missing. */
};
