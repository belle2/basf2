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
 * Routine to decay vector-> scalar scalar with BB-like mixing and
 * decoherence parameter lambda.  Based on
 * https://doi.org/10.1103/PhysRevD.64.056004 aka the Lindblad
 * model. It has a single dissipative parameter \f$\lambda\f$. The model
 * name is VSSBMixNP.
 */
class EvtVSSBMixNP : public EvtDecayAmp {
public:

  /**
   * Constructor.
   */
  EvtVSSBMixNP() {}

  /**
   * Destructor.
   */
  virtual ~EvtVSSBMixNP();

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
  /** delta gamma in hbar/mm */
  double _dGamma;
  /** lambda in hbar/mm */
  double _lambda;
};
