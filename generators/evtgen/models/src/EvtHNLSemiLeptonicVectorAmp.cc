/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include "generators/evtgen/models/EvtHNLSemiLeptonicVectorAmp.h"

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
using std::endl;

void EvtHNLSemiLeptonicVectorAmp::CalcAmp(EvtParticle* parent, EvtAmp& amp,
                                          EvtSemiLeptonicFF* FormFactors)
{
  static EvtId EM = EvtPDL::getId("e-");
  static EvtId MUM = EvtPDL::getId("mu-");
  static EvtId TAUM = EvtPDL::getId("tau-");
  static EvtId EP = EvtPDL::getId("e+");
  static EvtId MUP = EvtPDL::getId("mu+");
  static EvtId TAUP = EvtPDL::getId("tau+");

  static EvtId D0 = EvtPDL::getId("D0");
  static EvtId D0B = EvtPDL::getId("anti-D0");
  static EvtId DP = EvtPDL::getId("D+");
  static EvtId DM = EvtPDL::getId("D-");
  static EvtId DSM = EvtPDL::getId("D_s-");
  static EvtId DSP = EvtPDL::getId("D_s+");

  //Add the lepton and neutrino 4 momenta to find q2

  EvtVector4R q = parent->getDaug(1)->getP4() + parent->getDaug(2)->getP4();
  double q2 = (q.mass2());

  double a1f, a2f, vf, a0f, a3f;
  double m_meson = parent->getDaug(0)->mass();

  FormFactors->getvectorff(parent->getId(), parent->getDaug(0)->getId(),
                           q2, m_meson, &a1f, &a2f, &vf, &a0f);

  double costhl_flag = 1.0;

  if (parent->getId() == D0 || parent->getId() == D0B ||
      parent->getId() == DP || parent->getId() == DM) {
    costhl_flag = -1.0;
  }
  if (parent->getId() == DSP || parent->getId() == DSM) {
    costhl_flag = -1.0;
  }
  vf = vf * costhl_flag;

  EvtVector4R p4b;
  p4b.set(parent->mass(), 0.0, 0.0, 0.0);

  EvtVector4R p4meson = parent->getDaug(0)->getP4();

  EvtVector4C l11, l12, l21, l22;

  EvtId l_num = parent->getDaug(1)->getId();
  double m_b = parent->mass();

  a3f = ((m_b + m_meson) / (2.0 * m_meson)) * a1f -
        ((m_b - m_meson) / (2.0 * m_meson)) * a2f;

  EvtTensor4C tds;
  if (l_num == EM || l_num == MUM || l_num == TAUM) {
    tds = a1f * (m_b + m_meson) * EvtTensor4C::g();
    tds.addDirProd((-a2f / (m_b + m_meson)) * p4b, p4b + p4meson);
    tds += EvtComplex(0.0, vf / (m_b + m_meson)) *
           dual(EvtGenFunctions::directProd(p4meson + p4b, p4b - p4meson));
    tds.addDirProd((a0f - a3f) * 2.0 * (m_meson / q2) * p4b,
                   p4b - p4meson);

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
      tds = a1f * (m_b + m_meson) * EvtTensor4C::g();
      tds.addDirProd((-a2f / (m_b + m_meson)) * p4b, p4b + p4meson);
      tds -= EvtComplex(0.0, vf / (m_b + m_meson)) *
             dual(EvtGenFunctions::directProd(p4meson + p4b,
                                              p4b - p4meson));
      tds.addDirProd((a0f - a3f) * 2.0 * (m_meson / q2) * p4b,
                     p4b - p4meson);

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
          << "Wrong lepton number" << endl;
    }
  }

  EvtVector4C et0 = tds.cont1(parent->getDaug(0)->epsParent(0).conj());
  EvtVector4C et1 = tds.cont1(parent->getDaug(0)->epsParent(1).conj());
  EvtVector4C et2 = tds.cont1(parent->getDaug(0)->epsParent(2).conj());

  amp.vertex(0, 0, 0, l11.cont(et0));
  amp.vertex(0, 0, 1, l12.cont(et0));
  amp.vertex(0, 1, 0, l21.cont(et0));
  amp.vertex(0, 1, 1, l22.cont(et0));

  amp.vertex(1, 0, 0, l11.cont(et1));
  amp.vertex(1, 0, 1, l12.cont(et1));
  amp.vertex(1, 1, 0, l21.cont(et1));
  amp.vertex(1, 1, 1, l22.cont(et1));

  amp.vertex(2, 0, 0, l11.cont(et2));
  amp.vertex(2, 0, 1, l12.cont(et2));
  amp.vertex(2, 1, 0, l21.cont(et2));
  amp.vertex(2, 1, 1, l22.cont(et2));

  return;
}
