/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include "EvtGenBase/EvtDecayAmp.hh"
#include "EvtGenBase/EvtComplex.hh"

class EvtParticle;

/**
 * Implementation of the B -> D^* \ell \bar{\nu}_\ell with new physics
 * contributions and the BGL hadronic form factor parameterization.
 * Based on the publication:
 * Phys.Rev.D 107 (2023) 1, 015011; e-Print:2206.11283 [hep-ph]
 */
class EvtBToDstarlnuNP : public EvtDecayAmp {
public:
  /**
   * The function which returns the name of the model.
   */
  std::string getName() override;

  /**
   * The function which makes a copy of the model.
   */
  EvtDecayBase* clone() override;

  /**
   * The method to calculate a decay amplitude.
   */
  void decay(EvtParticle* p) override;

  /**
   * Initialization method
   */
  void init() override;

  /**
   * The method to evaluate the maximum amplitude.
   */
  void initProbMax() override;

private:
  /** Strength of the left handed part of the vector hadronic current */
  EvtComplex _Cvl;

  /** Strength of the Right handed part of the vector hadronic current */
  EvtComplex _Cvr;

  /** (g_S - g_P)  strength of pseudoscalar current */
  EvtComplex _Csl;

  /** (g_S + g_P)  strength of pseudoscalar current with constraint _Csl = -_Csr */
  EvtComplex _Csr;

  /** C_T -- tensor current */
  EvtComplex _cT;

  /**
   * The method to evaluate the maximum decay amplitude.
   */
  double CalcMaxProb();

  /**
   * The method to evaluate the decay amplitude.
   */
  void CalcAmp(EvtParticle*, EvtAmp&);
};
