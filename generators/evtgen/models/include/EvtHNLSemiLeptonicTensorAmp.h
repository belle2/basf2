/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <EvtGenBase/EvtSemiLeptonicAmp.hh>

class EvtParticle;
class EvtSemiLeptonicFF;
class EvtAmp;

class EvtHNLSemiLeptonicTensorAmp : public EvtSemiLeptonicAmp {
public:
  //Daughters are initialized and have been added to the parent.
  //No need to carry around the daughters seperately!
  void CalcAmp(EvtParticle* parent, EvtAmp& amp,
               EvtSemiLeptonicFF* FormFactors) override;
};

