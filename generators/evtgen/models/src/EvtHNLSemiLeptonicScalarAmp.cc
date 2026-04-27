/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include "generators/evtgen/models/EvtHNLSemiLeptonicScalarAmp.h"

#include "EvtGenBase/EvtAmp.hh"
#include "EvtGenBase/EvtDiracSpinor.hh"
#include "EvtGenBase/EvtGenKine.hh"
#include "EvtGenBase/EvtId.hh"
#include "EvtGenBase/EvtPDL.hh"
#include "EvtGenBase/EvtParticle.hh"
#include "EvtGenBase/EvtPatches.hh"
#include "EvtGenBase/EvtReport.hh"
#include "EvtGenBase/EvtSemiLeptonicFF.hh"
#include "EvtGenBase/EvtTensor4C.hh"
#include "EvtGenBase/EvtVector4C.hh"

#include <iostream>
#include <fstream>
#include <random>
#include <vector>
#include <sstream>
#include <cstdlib>
using std::endl;

void EvtHNLSemiLeptonicScalarAmp::CalcAmp(EvtParticle* parent, EvtAmp& amp,
                                          EvtSemiLeptonicFF* FormFactors)
{
  static EvtId EM = EvtPDL::getId("e-");
  static EvtId MUM = EvtPDL::getId("mu-");
  static EvtId TAUM = EvtPDL::getId("tau-");
  static EvtId EP = EvtPDL::getId("e+");
  static EvtId MUP = EvtPDL::getId("mu+");
  static EvtId TAUP = EvtPDL::getId("tau+");

  //Add the lepton and neutrino 4 momenta to find q2

  EvtVector4R q = parent->getDaug(1)->getP4() + parent->getDaug(2)->getP4();
  double q2 = (q.mass2());

  double fpf, f0f;
  double mesonmass = parent->getDaug(0)->mass();
  double parentmass = parent->mass();

  FormFactors->getscalarff(parent->getId(), parent->getDaug(0)->getId(),
                           q2, mesonmass, &fpf, &f0f);

  EvtVector4R p4b;
  p4b.set(parent->mass(), 0.0, 0.0, 0.0);
  EvtVector4R p4meson = parent->getDaug(0)->getP4();
  double mdiffoverq2;
  mdiffoverq2 = parentmass * parentmass - mesonmass * mesonmass;
  mdiffoverq2 = mdiffoverq2 / q2;

  EvtVector4C l11, l12, l21, l22;

  EvtId l_num = parent->getDaug(1)->getId();
  EvtVector4C tds;

  if (l_num == EM || l_num == MUM || l_num == TAUM) {
    tds = EvtVector4C(
            fpf * (p4b + p4meson - (mdiffoverq2 * (p4b - p4meson))) +
            +f0f * mdiffoverq2 * (p4b - p4meson));

    l11 = EvtLeptonVACurrent(parent->getDaug(1)->spParent(0),
                             parent->getDaug(2)->spParent(0));
    l12 = EvtLeptonVACurrent(parent->getDaug(1)->spParent(0),
                             parent->getDaug(2)->spParent(1));
    l21 = EvtLeptonVACurrent(parent->getDaug(1)->spParent(1),
                             parent->getDaug(2)->spParent(0));
    l22 = EvtLeptonVACurrent(parent->getDaug(1)->spParent(1),
                             parent->getDaug(2)->spParent(1));
  } else {
    if (l_num == EP || l_num == MUP || l_num == TAUP) {
      tds = EvtVector4C(
              fpf * (p4b + p4meson - (mdiffoverq2 * (p4b - p4meson))) +
              +f0f * mdiffoverq2 * (p4b - p4meson));

      l11 = EvtLeptonVACurrent(parent->getDaug(2)->spParent(0),
                               parent->getDaug(1)->spParent(0));
      l12 = EvtLeptonVACurrent(parent->getDaug(2)->spParent(0),
                               parent->getDaug(1)->spParent(1));
      l21 = EvtLeptonVACurrent(parent->getDaug(2)->spParent(1),
                               parent->getDaug(1)->spParent(0));
      l22 = EvtLeptonVACurrent(parent->getDaug(2)->spParent(1),
                               parent->getDaug(1)->spParent(1));
    } else {
      EvtGenReport(EVTGEN_ERROR, "EvtGen")
          << "dfnb89agngri wrong lepton number\n";
    }
  }

  amp.vertex(0, 0, l11 * tds);
  amp.vertex(0, 1, l12 * tds);
  amp.vertex(1, 0, l21 * tds);
  amp.vertex(1, 1, l22 * tds);

}
