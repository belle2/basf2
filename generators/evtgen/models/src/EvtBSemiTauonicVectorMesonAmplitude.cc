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

#include "EvtGenBase/EvtParticle.hh"
#include "EvtGenBase/EvtPDL.hh"
#include "EvtGenBase/EvtReport.hh"
#include "EvtGenBase/EvtTensor4C.hh"
#include "EvtGenBase/EvtVector4C.hh"
#include "EvtGenBase/EvtScalarParticle.hh"
#include "EvtGenBase/EvtDiracSpinor.hh"
#include "EvtGenBase/EvtId.hh"
#include "EvtGenBase/EvtAmp.hh"

#include "framework/logging/Logger.h"
#include "generators/evtgen/models/EvtBSemiTauonicVectorMesonAmplitude.h"
#include "generators/evtgen/models/EvtBSemiTauonicHelicityAmplitudeCalculator.h"

namespace Belle2 {

  void EvtBSemiTauonicVectorMesonAmplitude::CalcAmp(EvtParticle* p,
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
    // const double w=CalcHelAmp->wfunc(1,q2); avoid w<1 caused by the D* width
    const double costau = p4dln.dot(p4lln) / p4dln.d3mag() / p4lln.d3mag();
    const double ml = p4l.mass();

    // Set D* mass from its momentum (to take into account fluctuation due to its width)
    const double orig_mD = CalcHelAmp->getMDst();
    CalcHelAmp->setMDst(gmd);

    // obtain helicity amplitudes
    EvtComplex helamp[3][2]; // Dhel={1,0,-1}, tauhel={1,-1}
    helamp[0][0] = CalcHelAmp->helAmp(ml, 1, 1, w, costau); // note the parameter order is tauhel, Dhel
    helamp[0][1] = CalcHelAmp->helAmp(ml, -1, 1, w, costau);
    helamp[1][0] = CalcHelAmp->helAmp(ml, 1, 0, w, costau);
    helamp[1][1] = CalcHelAmp->helAmp(ml, -1, 0, w, costau);
    helamp[2][0] = CalcHelAmp->helAmp(ml, 1, -1, w, costau);
    helamp[2][1] = CalcHelAmp->helAmp(ml, -1, -1, w, costau);

    // lepton theta and phi in l+nu rest frame
    // const double l_theta=acos(p4lln.get(3)/p4lln.d3mag());
    // const double l_phi=atan2(p4lln.get(2),p4lln.get(1));

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

    // meson spin state to helicity state
    const double D_theta = acos(p4d.get(3) / p4d.d3mag());
    const double D_phi = atan2(p4d.get(2), p4d.get(1));
    EvtSpinDensity D_HelFromSp = p->getDaug(0)->rotateToHelicityBasis(D_phi, D_theta, -D_phi);

    EvtComplex D_SpFromHel[3][3]; //  {0,1,2} from {1,0,-1}
    D_SpFromHel[0][0] = conj(D_HelFromSp.get(0, 0));
    D_SpFromHel[0][1] = conj(D_HelFromSp.get(1, 0));
    D_SpFromHel[0][2] = conj(D_HelFromSp.get(2, 0));
    D_SpFromHel[1][0] = conj(D_HelFromSp.get(0, 1));
    D_SpFromHel[1][1] = conj(D_HelFromSp.get(1, 1));
    D_SpFromHel[1][2] = conj(D_HelFromSp.get(2, 1));
    D_SpFromHel[2][0] = conj(D_HelFromSp.get(0, 2));
    D_SpFromHel[2][1] = conj(D_HelFromSp.get(1, 2));
    D_SpFromHel[2][2] = conj(D_HelFromSp.get(2, 2));

    // calculate spin amplitudes
    EvtComplex spinamp[3][2];

    for (int dsp = 0; dsp < 3; dsp++) {
      for (int lsp = 0; lsp < 2; lsp++) {
        for (int dhel = 0; dhel < 3; dhel++) {
          for (int lhel = 0; lhel < 2; lhel++) {
            // b -> l, D*+
            if (l_num == EM || l_num == MUM || l_num == TAUM) {
              spinamp[dsp][lsp] += l_SpFromHel[lsp][lhel] * D_SpFromHel[dsp][dhel] * helamp[dhel][lhel];
            }
            // b-bar -> anti-l, D*-
            else {
              spinamp[dsp][lsp] += l_SpFromHel[lsp][lhel] * D_SpFromHel[dsp][dhel] * (lhel == 0 ? +1 : -1) * (dhel == 1 ? +1 : -1)
                                   * conj(helamp[2 - dhel][1 - lhel]);
            }
          }
        }
      }
    }

    amp.vertex(0, 0, spinamp[0][0]);
    amp.vertex(0, 1, spinamp[0][1]);
    amp.vertex(1, 0, spinamp[1][0]);
    amp.vertex(1, 1, spinamp[1][1]);
    amp.vertex(2, 0, spinamp[2][0]);
    amp.vertex(2, 1, spinamp[2][1]);

    // Set D* mass to its original value
    CalcHelAmp->setMDst(orig_mD);

    // consistency check
    double helprob = abs2(helamp[0][0]) + abs2(helamp[0][1]) + abs2(helamp[1][0]) + abs2(helamp[1][1]) + abs2(helamp[2][0])
                     + abs2(helamp[2][1]);
    double spinprob = abs2(spinamp[0][0]) + abs2(spinamp[0][1]) + abs2(spinamp[1][0]) + abs2(spinamp[1][1])
                      + abs2(spinamp[2][0]) + abs2(spinamp[2][1]);
    if (fabs(helprob - spinprob) / helprob > 1E-6 || !finite(helprob) || !finite(spinprob)) {
      B2ERROR("EvtBSemiTauonicVectorMesonAmplitude total helicity prob does not match with total spin prob, or nan.");
      B2ERROR("helprob: " << helprob << " spinprob: " << spinprob);
      B2ERROR("w: " << w << " costau: " << costau << " hel probs: "
              << abs2(helamp[0][0]) << "\t" << abs2(helamp[0][1]) << "\t"
              << abs2(helamp[1][0]) << "\t" << abs2(helamp[1][1]) << "\t"
              << abs2(helamp[2][0]) << "\t" << abs2(helamp[2][1]) << "\t"
              << abs2(helamp[0][0]) + abs2(helamp[0][1]) + abs2(helamp[1][0]) + abs2(helamp[1][1]) + abs2(helamp[2][0]) + abs2(helamp[2][1])
             );

      B2ERROR("w: " << w << " costau: " << costau << " spin probs: "
              << abs2(spinamp[0][0]) << "\t" << abs2(spinamp[0][1]) << "\t"
              << abs2(spinamp[1][0]) << "\t" << abs2(spinamp[1][1]) << "\t"
              << abs2(spinamp[2][0]) << "\t" << abs2(spinamp[2][1]) << "\t"
              << abs2(spinamp[0][0]) + abs2(spinamp[0][1]) + abs2(spinamp[1][0]) + abs2(spinamp[1][1]) + abs2(spinamp[2][0]) + abs2(spinamp[2][1])
             );

//    EvtGenReport(EVTGEN_ERROR, "EvtGen") <<
//                                         "EvtBSemiTauonicVectorMesonAmplitude total helicity prob does not match with total spin prob, or nan."
//                                         << std::endl;
//    EvtGenReport(EVTGEN_ERROR, "EvtGen") "helprob: "<<helprob<<" spinprob: "<<spinprob<< std::endl;
//    EvtGenReport(EVTGEN_ERROR, "EvtGen") "w: "<<w<<" costau: "<<costau<<" hel probs: "
//            <<abs2(helamp[0][0])<<"\t"<<abs2(helamp[0][1])<<"\t"
//            <<abs2(helamp[1][0])<<"\t"<<abs2(helamp[1][1])<<"\t"
//            <<abs2(helamp[2][0])<<"\t"<<abs2(helamp[2][1])<<"\t"
//            <<abs2(helamp[0][0]) + abs2(helamp[0][1]) + abs2(helamp[1][0]) + abs2(helamp[1][1]) + abs2(helamp[2][0]) + abs2(helamp[2][1])
//      <<std::endl;
//
//    EvtGenReport(EVTGEN_ERROR, "EvtGen") "w: "<<w<<" costau: "<<costau<<" spin probs: "
//            <<abs2(spinamp[0][0])<<"\t"<<abs2(spinamp[0][1])<<"\t"
//            <<abs2(spinamp[1][0])<<"\t"<<abs2(spinamp[1][1])<<"\t"
//            <<abs2(spinamp[2][0])<<"\t"<<abs2(spinamp[2][1])<<"\t"
//            <<abs2(spinamp[0][0]) + abs2(spinamp[0][1]) + abs2(spinamp[1][0]) + abs2(spinamp[1][1]) + abs2(spinamp[2][0]) + abs2(spinamp[2][1])
//      <<std::endl;

      // Debugging information
//    fprintf(stderr, "q2 by Helamp: %g\n", CalcHelAmp->q2(1, w));
//    fprintf(stderr, "helampSM by Helamp: %g\n", CalcHelAmp->helampSM(ml, 1, 1, w, costau));
//    fprintf(stderr, "Lep by Helamp: %g\n", CalcHelAmp->Lep(ml, 1, 1, CalcHelAmp->q2(1, w), costau));
//    fprintf(stderr, "HadV2 by Helamp: %g\n", CalcHelAmp->HadV2(1, 1, w));
//    fprintf(stderr, "v by Helamp: %g\n", CalcHelAmp->v(ml, CalcHelAmp->v(ml, CalcHelAmp->q2(1, w))));
//
//    fprintf(stderr, "B mass : %g \t nominal %g\n", gmB, EvtPDL::getMeanMass(p->getId()));
//    fprintf(stderr, "D mass : %g \t nominal %g\n", gmd, EvtPDL::getMeanMass(p->getDaug(0)->getId()));
//    fprintf(stderr, "lepton mass : %g \t nominal %g\n", ml, EvtPDL::getMeanMass(p->getDaug(1)->getId()));

      // abort();
    }

    return;
  }

} // Belle 2 Namespace
