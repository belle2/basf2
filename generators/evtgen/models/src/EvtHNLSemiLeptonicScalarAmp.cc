/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <generators/evtgen/models/EvtHNLSemiLeptonicScalarAmp.h>

#include <EvtGenBase/EvtAmp.hh>
#include <EvtGenBase/EvtDiracSpinor.hh>
#include <EvtGenBase/EvtId.hh>
#include <EvtGenBase/EvtPDL.hh>
#include <EvtGenBase/EvtParticle.hh>
#include <EvtGenBase/EvtSemiLeptonicFF.hh>
#include <EvtGenBase/EvtVector4C.hh>

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

  EvtId l_num = parent->getDaug(1)->getId();
  EvtVector4C tds;
  tds = EvtVector4C(
          fpf * (p4b + p4meson - (mdiffoverq2 * (p4b - p4meson))) +
          +f0f * mdiffoverq2 * (p4b - p4meson));

  for (int i{0}; i < 2; ++i) {
    for (int j{0}; j < 2; ++j) {
      const EvtVector4C current{
        EvtLeptonVACurrent(parent->getDaug(2)->spParent(j),
                           parent->getDaug(1)->spParent(i))};

      if (l_num == EM || l_num == MUM || l_num == TAUM) {

        amp.vertex(i, j, current.conj() * tds);

      } else if (l_num == EP || l_num == MUP || l_num == TAUP) {

        amp.vertex(i, j, current * tds);

      } else {
        EvtGenReport(EVTGEN_ERROR, "EvtGen")
            << "Wrong lepton number\n";
      }
    }
  }
}
