/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
// Junewoo Park      December 02, 2021

#pragma once

#include "EvtGenBase/EvtDecayIncoherent.hh"
#include "EvtGenBase/EvtParticle.hh"

namespace Belle2 {
  class EvtBtoXsnunu_FERMI : public  EvtDecayIncoherent {

  public:

    EvtBtoXsnunu_FERMI() {}
    virtual ~EvtBtoXsnunu_FERMI();

    std::string getName();

    EvtDecayBase* clone();

    void initProbMax();

    void init();

    void decay(EvtParticle* p);

    double dGdsbProb(double _sb);
    double FermiMomentum(double pf);
    double FermiMomentumProb(double pb, double pf);

  private:
    double         _dGdsbProbMax{0.0};
    double         _mb{4.8};          // b-quark mass for fermi motion
    double         _ms{0.2};          // s-quark mass for fermi motion
    double         _mq{0.0};          // spectator quark mass for fermi motion
    double         _pf{0.461};          // Fermi motion parameter for fermi motion
    double         _mxmin{1.1};

    double _mb_prob{4.8};          // b-quark mass for dGdsb
    double _ms_prob{0.2};          // s-quark mass for dGdsb

  };

} // Belle 2 Namespace
