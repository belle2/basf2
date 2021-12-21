/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <generators/evtgen/EvtGenModelRegister.h>

#include <stdlib.h>
#include <cmath>
#include "EvtGenBase/EvtRandom.hh"
#include "EvtGenBase/EvtParticle.hh"
#include "EvtGenBase/EvtGenKine.hh"
#include "EvtGenBase/EvtPDL.hh"
#include "EvtGenBase/EvtReport.hh"
#include "EvtGenBase/EvtConst.hh"
#include "EvtGenBase/EvtId.hh"

#include "generators/evtgen/models/EvtBtoXsnunu_FERMI.h"

using std::endl;

namespace Belle2 {

  /** register the model in EvtGen */
  B2_EVTGEN_REGISTER_MODEL(EvtBtoXsnunu_FERMI);

  EvtBtoXsnunu_FERMI::~EvtBtoXsnunu_FERMI() {}

  std::string EvtBtoXsnunu_FERMI::getName()
  {
    return "BTOXSNUNU_FERMI";
  }

  EvtDecayBase* EvtBtoXsnunu_FERMI::clone()
  {
    return new EvtBtoXsnunu_FERMI;
  }

  void EvtBtoXsnunu_FERMI::decay(EvtParticle* p)
  {
    p->makeDaughters(getNDaug(), getDaugs());

    EvtParticle* xhadron = p->getDaug(0);
    EvtParticle* leptonp = p->getDaug(1);
    EvtParticle* leptonn = p->getDaug(2);

    double mass[3];

    findMasses(p, getNDaug(), getDaugs(), mass);

    double mB = p->mass();
    double ml = mass[1];
    double pb(0.); // fermi momentum of b-quark

    double xhadronMass = -999.0;

    EvtVector4R p4xhadron;
    EvtVector4R p4leptonp;
    EvtVector4R p4leptonn;

    while (xhadronMass < m_mxmin) {

      // Apply Fermi motion and determine effective b-quark mass

      double mb = 0.0;

      bool FailToSetmb = true; // true when an appropriate mb cannot be found
      while (FailToSetmb) {
        pb = FermiMomentum(m_pf);

        // effective b-quark mass
        mb = mB * mB + m_mq * m_mq - 2.0 * mB * sqrt(pb * pb + m_mq * m_mq);
        if (mb > 0. && sqrt(mb) - m_ms < 2.0 * ml) FailToSetmb = true;
        else if (mb <= 0.0) FailToSetmb = true;
        else FailToSetmb = false;
      }
      mb = sqrt(mb);

      double mb_prob = m_mb_prob; // b-quark mass for probability density
      double ms_prob = m_ms_prob; // s-quark mass for probability density
      double mstilda = ms_prob / mb_prob;
      double sb = 0.0;
      double sbmin = 0;
      double sbmax = (1 - mstilda) * (1 - mstilda);
      while (sb == 0.0) {
        double xbox = EvtRandom::Flat(sbmin, sbmax);
        double ybox = EvtRandom::Flat(m_dGdsbProbMax);
        double prob = dGdsbProb(xbox);
        if (ybox < prob) sb = xbox;
      }

      // b->s (nu nubar)
      EvtVector4R p4sdilep[2];

      double msdilep[2];
      msdilep[0] = m_ms;
      msdilep[1] = sqrt(sb * mb_prob * mb_prob);

      EvtGenKine::PhaseSpace(2, msdilep, p4sdilep, mb);

      // we do not care about neutrino
      // just (nu nubar) -> nu nubar by phase space
      EvtVector4R p4ll[2];

      double mll[2];
      mll[0] = ml;
      mll[1] = ml;

      EvtGenKine::PhaseSpace(2, mll, p4ll, msdilep[1]);

      // boost to b-quark rest frame

      p4ll[0] = boostTo(p4ll[0], p4sdilep[1]);
      p4ll[1] = boostTo(p4ll[1], p4sdilep[1]);

      // assign 4-momenta to valence quarks inside B meson in B rest frame
      double phi = EvtRandom::Flat(EvtConst::twoPi);
      double costh = EvtRandom::Flat(-1.0, 1.0);
      double sinth = sqrt(1.0 - costh * costh);

      // b-quark four-momentum in B meson rest frame

      EvtVector4R p4b(sqrt(mb * mb + pb * pb),
                      pb * sinth * sin(phi),
                      pb * sinth * cos(phi),
                      pb * costh);

      EvtVector4R p4s = boostTo(p4sdilep[0], p4b);
      p4leptonp = boostTo(p4ll[0], p4b);
      p4leptonn = boostTo(p4ll[1], p4b);

      // spectator quark in B meson rest frame
      EvtVector4R p4q(sqrt(pb * pb + m_mq * m_mq), -p4b.get(1), -p4b.get(2), -p4b.get(3));

      // hadron system in B meson rest frame
      p4xhadron = p4s + p4q;
      xhadronMass = p4xhadron.mass();

    }

    // initialize the decay products
    xhadron->init(getDaug(0), p4xhadron);

    // assign the momentum of neutrino
    // it is out of our interest
    leptonp->init(getDaug(1), p4leptonp);
    leptonn->init(getDaug(2), p4leptonn);

    return;

  }


  void EvtBtoXsnunu_FERMI::initProbMax()
  {
    noProbMax();
  }


  void EvtBtoXsnunu_FERMI::init()
  {

    // check that there are no arguments

    checkNArg(0, 3, 4, 6);

    checkNDaug(3);

    // Check that the two leptons are the same type

    EvtId lepton1type = getDaug(1);
    EvtId lepton2type = getDaug(2);

    int etyp = 0;
    int mutyp = 0;
    int tautyp = 0;
    if (lepton1type == EvtPDL::getId("anti-nu_e") ||
        lepton1type == EvtPDL::getId("nu_e")) {
      etyp++;
    }
    if (lepton2type == EvtPDL::getId("anti-nu_e") ||
        lepton2type == EvtPDL::getId("nu_e")) {
      etyp++;
    }
    if (lepton1type == EvtPDL::getId("anti-nu_mu") ||
        lepton1type == EvtPDL::getId("nu_mu")) {
      mutyp++;
    }
    if (lepton2type == EvtPDL::getId("anti-nu_mu") ||
        lepton2type == EvtPDL::getId("nu_mu")) {
      mutyp++;
    }
    if (lepton1type == EvtPDL::getId("anti-nu_tau") ||
        lepton1type == EvtPDL::getId("nu_tau")) {
      tautyp++;
    }
    if (lepton2type == EvtPDL::getId("anti-nu_tau") ||
        lepton2type == EvtPDL::getId("nu_tau")) {
      tautyp++;
    }

    if (etyp != 2 && mutyp != 2 && tautyp != 2) {

      std::cout << "Expect two neutrinos of the same type in EvtBtoXsnunu.cc\n";
      ::abort();
    }

    // Check that the second and third entries are leptons with positive
    // and negative charge, respectively

    int lpos = 0;
    int lneg = 0;
    if (lepton1type == EvtPDL::getId("anti-nu_e") ||
        lepton1type == EvtPDL::getId("anti-nu_mu") ||
        lepton1type == EvtPDL::getId("anti-nu_tau")) {
      lpos++;
    }
    if (lepton2type == EvtPDL::getId("nu_e") ||
        lepton2type == EvtPDL::getId("nu_mu") ||
        lepton2type == EvtPDL::getId("nu_tau")) {
      lneg++;
    }

    if (lpos != 1 || lneg != 1) {

      std::cout << "Expect 2nd and 3rd particles to be anti-particle and particle of neutrinos in EvtBtoXsnunu.cc\n";
      ::abort();
    }

    if (getNArg() >= 3) {
      // s-quark mass for fermi motion
      m_ms = getArg(0);
      // spectator quark mass for fermi motion
      m_mq = getArg(1);
      // Fermi motion parameter for fermi motion
      m_pf = getArg(2);
    }
    if (getNArg() >= 4) {
      m_mxmin = getArg(3);
    }
    if (getNArg() == 6) {
      m_mb_prob = getArg(4);
      m_ms_prob = getArg(5);
    }

    // get a maximum probability
    double mb = m_mb_prob;
    double ms = m_ms_prob;
    double mstilda = ms / mb;
    double mstilda2 = mstilda * mstilda;

    int nsteps = 100;
    double sbmin = 0;
    double sbmax = (1 - mstilda) * (1 - mstilda);
    double probMax = -10000.0;
    double sbProbMax = -10.0;

    for (int i = 0; i < nsteps; i++) {
      double sb = sbmin + (i + 0.0005) * (sbmax - sbmin) / (double)nsteps;
      double lambda = 1 + mstilda2 * mstilda2 + sb * sb - 2 * (mstilda2 + sb + mstilda2 * sb);
      double prob = sqrt(lambda) * (3 * sb * (1 + mstilda2 - sb) + lambda);
      if (prob > probMax) {
        sbProbMax = sb;
        probMax = prob;
      }
    }

    if (verbose()) {
      std::cout << "dGdsbProbMax = " << probMax << " for sb = " << sbProbMax << std::endl;
    }

    m_dGdsbProbMax = probMax;

  }

  double EvtBtoXsnunu_FERMI::dGdsbProb(double _sb)
  {
    // dGdsb: arXiv:1509.06248v2
    // see (eq.41)

    double mb = m_mb_prob; // b-quark mass for probability density
    double ms = m_ms_prob; // s-quark mass for probability density
    double mstilda = ms / mb;

    double sb = _sb;
    double mstilda2 = mstilda * mstilda;

    double lambda = 1 + mstilda2 * mstilda2 + sb * sb - 2 * (mstilda2 + sb + mstilda2 * sb);

    double prob = sqrt(lambda) * (3 * sb * (1 + mstilda2 - sb) + lambda);

    return prob;
  }

  double EvtBtoXsnunu_FERMI::FermiMomentum(double pf)
  {
    // Pick a value for the b-quark Fermi motion momentum
    // according to Ali's Gaussian model

    // reference: Ali, Ahmed, et al. "Power corrections in the decay rate and distributions in B->Xs l+l- 2 in the standard model"
    // see (eq.57)

    double pb, pbmax;
    pb = 0.0;
    pbmax = 5.0 * pf;

    while (pb == 0.0) {
      double xbox = EvtRandom::Flat(pbmax);
      double ybox = EvtRandom::Flat();
      if (ybox < FermiMomentumProb(xbox, pf)) { pb = xbox; }
    }

    return pb;
  }

  double EvtBtoXsnunu_FERMI::FermiMomentumProb(double pb, double pf)
  {
    // Compute probability according to Ali's Gaussian model
    // the function chosen has a convenient maximum value of 1 for pb = pf

    // reference: Ali, Ahmed, et al. "Power corrections in the decay rate and distributions in B->Xs l+l- 2 in the standard model"
    // see (eq.57)

    double prsq = (pb * pb) / (pf * pf);
    double prob = prsq * exp(1.0 - prsq);

    return prob;
  }

}

