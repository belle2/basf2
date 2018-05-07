/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Koji Hara                                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <cstdio>

#include "EvtGenBase/EvtPatches.hh"
#include "EvtGenBase/EvtParticle.hh"
#include "EvtGenBase/EvtPDL.hh"
#include "EvtGenBase/EvtTensor4C.hh"
#include "EvtGenBase/EvtVector4C.hh"
#include "EvtGenBase/EvtScalarParticle.hh"
#include "EvtGenBase/EvtDiracSpinor.hh"
#include "EvtGenBase/EvtId.hh"
#include "EvtGenBase/EvtAmp.hh"
#include "EvtGenBase/EvtReport.hh"

#include "framework/logging/Logger.h"
#include "generators/evtgen/models/EvtBSemiTauonicScalarMesonAmplitude.h"
#include "generators/evtgen/models/EvtBSemiTauonicHelicityAmplitudeCalculator.h"

namespace Belle2 {


  void EvtBSemiTauonicScalarMesonAmplitude::CalcAmp(EvtParticle* p,
                                                    EvtAmp& amp,
                                                    EvtBSemiTauonicHelicityAmplitudeCalculator* CalcHelAmp)
  {

    static EvtId EM = EvtPDL::getId("e-");
    static EvtId MUM = EvtPDL::getId("mu-");
    static EvtId TAUM = EvtPDL::getId("tau-");
//  static EvtId EP = EvtPDL::getId("e+");
//  static EvtId MUP = EvtPDL::getId("mu+");
//  static EvtId TAUP = EvtPDL::getId("tau+");

    // calculate w and costau

    EvtVector4R p4d = p->getDaug(0)->getP4();
    EvtVector4R p4l = p->getDaug(1)->getP4();
    EvtVector4R p4n = p->getDaug(2)->getP4();
    EvtVector4R p4ln(p4l + p4n);

    EvtVector4R p4dln = boostTo(p4d, p4ln, true);
    EvtVector4R p4lln = boostTo(p4l, p4ln, true);

    const double gmB = p->getP4().mass();
    const double gmd = p4d.mass();
    const double gr = gmd / gmB;

    const double q2 = (p4l + p4n).mass2();
    const double w = (1. + gr * gr - q2 / gmB / gmB) / 2. / gr;
    // const double w=CalcHelAmp->wfunc(2,q2); avoid possible w<0 caused by the decay width
    const double costau = p4dln.dot(p4lln) / p4dln.d3mag() / p4lln.d3mag();
    const double ml = p4l.mass();

    // obtain helicity amplitudes
    EvtComplex helamp[2]; // tauhel={1,-1}
    helamp[0] = CalcHelAmp->helAmp(ml, 1, 2, w, costau); // note the parameter order is tauhel, Dhel
    helamp[1] = CalcHelAmp->helAmp(ml, -1, 2, w, costau); // Dhel=2 ==> D meson

    // lepton theta and phi in l+nu rest frame
    //const double l_theta=acos(p4lln.get(3)/p4lln.d3mag());
    //const double l_phi=atan2(p4lln.get(2),p4lln.get(1));

    // spin (in l rest frame) -> helicity (in l+nu rest frame) rotation matrix
    // ( sp0->hel0 , sp1->hel0 )
    // ( sp0->hel1 , sp1->hel1 )
    EvtSpinDensity l_HelFromSp = RotateToHelicityBasisInBoostedFrame(p->getDaug(1),
                                 p4ln);
//                 l_phi,
//                 l_theta,
//                 -l_phi);

    // helicity (in l+nu rest frame) -> spin (in l rest frame) rotation matrix
    // ( hel0->sp0 , hel1->sp0 )
    // ( hel0->sp1 , hel1->sp1 )
    EvtComplex l_SpFromHel[2][2]; // {0,1} from {1,-1}
    EvtId l_num = p->getDaug(1)->getId();
//  if (l_num == EM || l_num == MUM || l_num == TAUM) {
    l_SpFromHel[0][0] = conj(l_HelFromSp.get(0, 0));
    l_SpFromHel[0][1] = conj(l_HelFromSp.get(1, 0));
    l_SpFromHel[1][0] = conj(l_HelFromSp.get(0, 1));
    l_SpFromHel[1][1] = conj(l_HelFromSp.get(1, 1));
//  } else {
//    l_SpFromHel[0][1] = conj(l_HelFromSp.get(0, 0));
//    l_SpFromHel[0][0] = conj(l_HelFromSp.get(1, 0));
//    l_SpFromHel[1][1] = conj(l_HelFromSp.get(0, 1));
//    l_SpFromHel[1][0] = conj(l_HelFromSp.get(1, 1));
//  }

    // calculate spin amplitudes
    EvtComplex spinamp[2];

    for (int lsp = 0; lsp < 2; lsp++) {
      for (int lhel = 0; lhel < 2; lhel++) {
        // b -> l
        if (l_num == EM || l_num == MUM || l_num == TAUM) {
          spinamp[lsp] += l_SpFromHel[lsp][lhel] * helamp[lhel];
        }
        // b-bar -> anti-l
        else {
          spinamp[lsp] += l_SpFromHel[lsp][lhel] * (lhel == 0 ? +1 : -1) * conj(helamp[1 - lhel]);
        }
      }
    }

    amp.vertex(0, spinamp[0]);
    amp.vertex(1, spinamp[1]);

    // consistency check
    double helprob = abs2(helamp[0]) + abs2(helamp[1]);
    double spinprob = abs2(spinamp[0]) + abs2(spinamp[1]);
    if (fabs(helprob - spinprob) / helprob > 1E-6 || !finite(helprob) || !finite(spinprob)) {
      B2ERROR("EvtBSemiTauonicScalarMesonAmplitude total helicity prob does not match with total spin prob.");
      B2ERROR("helprob: " << helprob << " spinprob: " << spinprob);
      B2ERROR("w: " << w << " costau: " << costau << " hel probs: " << abs2(helamp[0])
              << "\t" << abs2(helamp[1])
              << "\t" << abs2(helamp[0]) + abs2(helamp[1]));

      B2ERROR("w: " << w << " costau: " << costau << " spin probs: " << abs2(spinamp[0])
              << "\t" << abs2(spinamp[1])
              << "\t" << abs2(spinamp[0]) + abs2(spinamp[1]));

//    EvtGenReport(EVTGEN_ERROR, "EvtGen") <<
//                                         "EvtBSemiTauonicScalarMesonAmplitude total helicity prob does not match with total spin prob."
//                                         << std::endl;
//    EvtGenReport(EVTGEN_ERROR, "EvtGen") << "helprob: "<<helprob<<" spinprob: "<<spinprob<<std::endl;
//    EvtGenReport(EVTGEN_ERROR, "EvtGen") << "w: "<<w<<" costau: "<<costau
//                                         <<" hel probs: "<<abs2(helamp[0])
//                                         <<"\t"<<abs2(helamp[1])
//                                   <<"\t"<<abs2(helamp[0])+abs2(helamp[1])<<std::endl;
//
//    EvtGenReport(EVTGEN_ERROR, "EvtGen") << "w: "<<w<<" costau: "<<costau
//                                         <<" spin probs: "<<abs2(spinamp[0])
//                                         <<"\t"<<abs2(spinamp[1])
//                                   <<"\t"<<abs2(spinamp[0])+abs2(spinamp[1])<<std::endl;
      // abort();
    }

    return;
  }

} // Belle 2 Namespace
