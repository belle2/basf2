/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include "EvtGenBase/EvtDecayAmp.hh"
#include "EvtGenBase/EvtParticle.hh"

/**
 * Routine to decay vector -> scalar scalar with Spontaneous
 * Disentanglement. The model name is VSS_BMIX_SD.
 */
class EvtVSSBMixSD : public EvtDecayAmp {
public:

  /**
   * Constructor.
   */
  EvtVSSBMixSD() {}

  /**
   * Destructor.
   */
  virtual ~EvtVSSBMixSD();

  /**
   * The function which returns the name of the model.
   */
  std::string getName() override;

  /**
   * The function which makes a copy of the model.
   */
  EvtDecayBase* clone() override;

  /**
   * The function to calculate a quark decay amplitude.
   */
  void decay(EvtParticle* p) override;

  /**
   * The function for an initialization.
   */
  void init() override;

  /**
   * The function to sets a maximum probability.
   */
  void initProbMax() override;

  /**
   * The number of real daughters
   */
  int nRealDaughters() override { return 2; }

private:
  /** mixing frequency in hbar/mm */
  double _freq;
  /** probability of the spontaneous disentanglement [0,1] */
  double _sdprob;
};
