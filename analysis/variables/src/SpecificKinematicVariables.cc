/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

// Own include
#include <analysis/variables/SpecificKinematicVariables.h>
#include <analysis/utility/PCmsLabTransform.h>
#include <analysis/variables/Variables.h>

// dataobjects
#include <analysis/dataobjects/Particle.h>

// framework aux
#include <framework/logging/Logger.h>

#include <TRandom.h>
#include <TMath.h>
#include <iostream>
using namespace std;

namespace Belle2 {
  namespace Variable {


    double REC_q2BhSimple(const Particle* particle)
    {
      // calculates q^2 = (p_B - p_h) in decays of B -> h_1 .. h_n ell nu_ell,
      // where p_h = Sum_i^n p_h_i is the 4-momentum of hadrons in the final
      // state. The calculation is performed in the CMS system, where B-meson
      // is assumed to be at rest p_B = (m_B, 0).

      TLorentzVector hadron4vec;

      unsigned n = particle->getNDaughters();

      if (n < 1)
        return std::numeric_limits<float>::quiet_NaN();

      for (unsigned i = 0; i < n; i++) {
        int absPDG = abs(particle->getDaughter(i)->getPDGCode());
        if (absPDG == Const::electron.getPDGCode() || absPDG == Const::muon.getPDGCode() || absPDG == 15)
          continue;

        hadron4vec += particle->getDaughter(i)->get4Vector();
      }

      // boost to CMS
      PCmsLabTransform T;
      TLorentzVector phCMS = T.rotateLabToCms() * hadron4vec;
      TLorentzVector pBCMS;
      pBCMS.SetXYZM(0.0, 0.0, 0.0, particle->getPDGMass());
      return (pBCMS - phCMS).Mag2();
    }

    double REC_q2Bh(const Particle* particle)
    {
      // calculates q^2 = (p_B - p_h) in decays of B -> h_1 .. h_n ell nu_ell,
      // where p_h = Sum_i^n p_h_i is the 4-momentum of hadrons in the final
      // state. The calculation is performed in the CMS system,
      // with a weighter average in a cone around the true B direction

      TLorentzVector hadron4vec;

      unsigned n = particle->getNDaughters();

      if (n < 1)
        return std::numeric_limits<float>::quiet_NaN();

      for (unsigned i = 0; i < n; i++) {
        int absPDG = abs(particle->getDaughter(i)->getPDGCode());
        if (absPDG == Const::electron.getPDGCode() || absPDG ==  Const::muon.getPDGCode() || absPDG == 15)
          continue;

        hadron4vec += particle->getDaughter(i)->get4Vector();
      }

      // boost to CMS
      PCmsLabTransform T;
      TLorentzVector had_cm = T.rotateLabToCms() * hadron4vec;
      TLorentzVector Y_cm = T.rotateLabToCms() * particle->get4Vector();


      double bmass = particle->getPDGMass();
      // B theta angle
      double cos_cone_angle = Variable::cosThetaBetweenParticleAndNominalB(particle);
      if (abs(cos_cone_angle) > 1) {
        //makes no sense in this case, return simple value
        double q2 = Variable::REC_q2BhSimple(particle);
        if (q2 < 0) {
          return 0;
        }
        return q2;
      }
      double thetaBY = TMath::ACos(cos_cone_angle);
      const double E_B = T.getCMSEnergy() / 2.0;
      const double p_B = sqrt(E_B * E_B - bmass * bmass);

      double phi_start = gRandom->Uniform(0, TMath::Pi() / 2);

      double q2 = 0;
      double denom = 0;

      TVector3 zHatY(Y_cm.Vect().Unit());
      TVector3 yHatY((had_cm.Vect().Cross(zHatY)).Unit());
      TVector3 xHatY(yHatY.Cross(zHatY));

      double phi = phi_start;
      double dphi = TMath::Pi() / 2;
      TLorentzVector static_B(0, 0, 0, bmass);

      for (int around_the_cone = 0; around_the_cone < 4; around_the_cone++) {
        //Define the momentum of B in the Y rest frame using the angles thetaBY and the
        //current phi in the loop.

        //The purpose of these lines is to calculate the B momentum in the BY cone in the coordinate system previously developed.
        double B0_px_Y_frame = p_B * TMath::Sin(thetaBY) * TMath::Cos(phi);
        double B0_py_Y_frame = p_B * TMath::Sin(thetaBY) * TMath::Sin(phi);
        double B0_pz_Y_frame = p_B * TMath::Cos(thetaBY);

        //The 3 components of the guess for the B0 p direction at the current phi are
        //calculated by scaling the 3 basis vectors computed before by the corresponding B
        //momentum in that direction.

        TVector3 B0_px_Dframe(xHatY);
        B0_px_Dframe *= B0_px_Y_frame;
        TVector3 B0_py_Dframe(yHatY);
        B0_py_Dframe *= B0_py_Y_frame;
        TVector3 B0_pz_Dframe(zHatY);
        B0_pz_Dframe *= B0_pz_Y_frame;
        //Construct the B0 p 3-vector with the current phi by summing the 3 components.

        TVector3 B0_p3_Dframe(B0_px_Dframe + B0_py_Dframe + B0_pz_Dframe);
        TLorentzVector B0_p4_Dframe(B0_p3_Dframe, E_B);

        //This is the polar angle of B0.

        double cosThetaB = B0_p3_Dframe.CosTheta();
        double sinThetaB2 = (1 - cosThetaB * cosThetaB);

        //The weight is given by the sin squared of such angle.
        double wt = sinThetaB2;

        //Boost the hadronic daughter to the computed B0 rest frame.
        // In that frame, q2 can simply be calculated by subtracting the hadron 4-momentum from the momentum of a static B.
        TLorentzVector had_B0(had_cm);
        had_B0.Boost(-B0_p4_Dframe.BoostVector());
        q2 += wt * ((static_B - had_B0).M2());
        denom += wt;
        phi += dphi;
      }

      q2 /= denom;
      if (q2 < 0) {
        q2 = 0.0;
      }
      return q2;
    }

    double REC_MissM2(const Particle* particle)
    {
      PCmsLabTransform T;
      TLorentzVector rec4vecLAB = particle->get4Vector();
      TLorentzVector rec4vec = T.rotateLabToCms() * rec4vecLAB;

      TLorentzVector miss4vec;
      double E_beam_cms = T.getCMSEnergy() / 2.0;

      miss4vec.SetVect(-rec4vec.Vect());
      miss4vec.SetE(E_beam_cms - rec4vec.Energy());

      return miss4vec.Mag2();
    }


    VARIABLE_GROUP("Specific kinematic variables");

    REGISTER_VARIABLE("recQ2BhSimple", REC_q2BhSimple,
                      "Returns the momentum transfer squared, :math:`q^2`, calculated in CMS as :math:`q^2 = (p_B - p_h)^2`, \n"
                      "where p_h is the CMS momentum of all hadrons in the decay :math:`B \\to H_1 ... H_n \\ell \\nu_\\ell`.\n"
                      "The B meson momentum in CMS is assumed to be 0.", Manager::VariableDataType::c_double);

    REGISTER_VARIABLE("recQ2Bh", REC_q2Bh,
                      "Returns the momentum transfer squared, :math:`q^2`, calculated in CMS as :math:`q^2 = (p_B - p_h)^2`, \n"
                      "where p_h is the CMS momentum of all hadrons in the decay :math:`B \\to H_1\\dots H_n \\ell \\nu_\\ell`.\n"
                      "This calculation uses a weighted average of the B meson around the reco B cone. \n"
                      "Based on diamond frame calculation of :math:`q^2` following the idea presented in https://www.osti.gov/biblio/1442697 \n"
                      "It will switch to use of :b2:var:`recQ2BhSimple` if absolute of :b2:var:`cosThetaBetweenParticleAndNominalB`  > 1.",
                      Manager::VariableDataType::c_double);

    REGISTER_VARIABLE("recMissM2", REC_MissM2,
                      "Returns the invariant mass squared of the missing momentum calculated assumings the"
                      "reco B is at rest and calculating the neutrino (missing) momentum from :math:`p_\\nu = p_B - p_\\mathrm{had} - p_\\mathrm{lep}`",
                      Manager::VariableDataType::c_double);



  }
}
