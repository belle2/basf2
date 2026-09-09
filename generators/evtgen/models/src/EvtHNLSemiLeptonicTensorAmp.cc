/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <generators/evtgen/models/EvtHNLSemiLeptonicTensorAmp.h>

#include <framework/logging/Logger.h>
#include <EvtGenBase/EvtAmp.hh>
#include <EvtGenBase/EvtDiracSpinor.hh>
#include <EvtGenBase/EvtId.hh>
#include <EvtGenBase/EvtPDL.hh>
#include <EvtGenBase/EvtParticle.hh>
#include <EvtGenBase/EvtSemiLeptonicFF.hh>
#include <EvtGenBase/EvtTensor4C.hh>
#include <EvtGenBase/EvtVector4C.hh>

void EvtHNLSemiLeptonicTensorAmp::CalcAmp(EvtParticle* parent, EvtAmp& amp,
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

  double hf, kf, bpf, bmf;

  FormFactors->gettensorff(parent->getId(), parent->getDaug(0)->getId(),
                           q2, parent->getDaug(0)->mass(), &hf, &kf, &bpf,
                           &bmf);

  double costhl_flag = 1.0;

  if (parent->getId() == D0 || parent->getId() == D0B ||
      parent->getId() == DP || parent->getId() == DM) {
    costhl_flag = -1.0;
  }
  if (parent->getId() == DSP || parent->getId() == DSM) {
    costhl_flag = -1.0;
  }
  hf = hf * costhl_flag;

  EvtVector4R p4b;
  p4b.set(parent->mass(), 0.0, 0.0, 0.0);

  EvtVector4R p4meson = parent->getDaug(0)->getP4();

  EvtId l_num = parent->getDaug(1)->getId();

  EvtVector4C ep_meson_b[5];

  ep_meson_b[0] =
    ((parent->getDaug(0)->epsTensorParent(0)).cont2(p4b)).conj();
  ep_meson_b[1] =
    ((parent->getDaug(0)->epsTensorParent(1)).cont2(p4b)).conj();
  ep_meson_b[2] =
    ((parent->getDaug(0)->epsTensorParent(2)).cont2(p4b)).conj();
  ep_meson_b[3] =
    ((parent->getDaug(0)->epsTensorParent(3)).cont2(p4b)).conj();
  ep_meson_b[4] =
    ((parent->getDaug(0)->epsTensorParent(4)).cont2(p4b)).conj();

  EvtVector4R pp, pm;

  pp = p4b + p4meson;
  pm = p4b - p4meson;

  //lange - October 31,2002 - try to lessen the mass dependence of probmax
  double q2max = p4b.mass2() + p4meson.mass2() -
                 2.0 * p4b.mass() * p4meson.mass();
  double q2maxin = 1.0 / q2max;

  EvtComplex ep_meson_bb[5];

  ep_meson_bb[0] = ep_meson_b[0] * (p4b);
  ep_meson_bb[1] = ep_meson_b[1] * (p4b);
  ep_meson_bb[2] = ep_meson_b[2] * (p4b);
  ep_meson_bb[3] = ep_meson_b[3] * (p4b);
  ep_meson_bb[4] = ep_meson_b[4] * (p4b);

  EvtTensor4C tdual;

  if (l_num == EM || l_num == MUM || l_num == TAUM) {
    tdual = EvtComplex(0.0, hf) *
            dual(EvtGenFunctions::directProd(pp, pm));
  } else if (l_num == EP || l_num == MUP || l_num == TAUP) {
    tdual = EvtComplex(0.0, -hf) *
            dual(EvtGenFunctions::directProd(pp, pm));
  } else {
    B2ERROR("HNLSemileptonicTensorAmp: Wrong lepton number");
  }

  std::array<EvtVector4C, 5> tdsList;

  for (std::size_t k{0}; k < tdsList.size(); ++k) {
    tdsList[k] = tdual.cont2(ep_meson_b[k]) - kf * ep_meson_b[k] -
                 bpf * ep_meson_bb[k] * pp - bmf * ep_meson_bb[k] * pm;
    tdsList[k] *= q2maxin;
  }

  for (int i{0}; i < 2; ++i) {
    for (int j{0}; j < 2; ++j) {
      const EvtVector4C current{
        EvtLeptonVACurrent(parent->getDaug(2)->spParent(j),
                           parent->getDaug(1)->spParent(i))};

      for (int k{0}; k < static_cast<int>(tdsList.size()); ++k) {

        if (l_num == EM || l_num == MUM || l_num == TAUM) {

          amp.vertex(k, i, j, current.conj() * tdsList[k]);

        } else if (l_num == EP || l_num == MUP || l_num == TAUP) {

          amp.vertex(k, i, j, current * tdsList[k]);

        }

      }
    }
  }

  return;
}
