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

#include <complex>
#include <vector>

class EvtParticle;
struct EvtLinSample;
typedef std::pair<double, std::complex<double>> helem_t;
typedef std::vector<helem_t> hvec_t;

// Description: Implementation of the b->sll decays with resonances according to Rusa and Rahul

class EvtbTosllNPR : public EvtDecayAmp {
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
  EvtComplex m_dc7;     // delta C_7eff -- addition to NNLO SM value
  EvtComplex m_dc9;     // delta C_9eff -- addition to NNLO SM value
  EvtComplex m_dc10;    // delta C_10eff -- addition to NNLO SM value
  EvtComplex m_c7p;     // C'_7eff -- right hand polarizations
  EvtComplex m_c9p;     // C'_9eff -- right hand polarizations
  EvtComplex m_c10p;    // c'_10eff -- right hand polarizations
  EvtComplex m_cS;      // (C_S - C'_S) -- scalar right and left polarizations
  EvtComplex m_cP;    // (C_P - C'_P) -- pseudo-scalar right and left polarizations
  int m_flag{0}; // flag is set nonzero to include resonances
  EvtLinSample* m_ls{0}; // piece-wise interpolation of maximum of the matrix element depend on q^2

  hvec_t m_reso;    // tabulated resonance contribution

  /**
   * The method to evaluate the maximum decay amplitude.
   */
  double CalcMaxProb();

  /**
   * The method to evaluate the decay amplitude.
   */
  void CalcAmp(EvtParticle*, EvtAmp&);

  /**
   * The method to evaluate the scalar kaon decay amplitude.
   */
  void CalcSAmp(EvtParticle*, EvtAmp&);

  /**
   * The method to evaluate the vector kaon decay amplitude.
   */
  void CalcVAmp(EvtParticle*, EvtAmp&);
};
