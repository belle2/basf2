/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

// Own include
#include <analysis/variables/HelicityVariables.h>

#include <analysis/variables/EventVariables.h>

#include <analysis/dataobjects/Particle.h>

#include <analysis/utility/ReferenceFrame.h>
#include <analysis/VariableManager/Manager.h>

#include <framework/utilities/Conversion.h>
#include <framework/gearbox/Const.h>

#include <Math/Boost.h>
#include <Math/Vector4D.h>
using namespace ROOT::Math;
#include <cmath>

namespace Belle2 {
  namespace Variable {

    double cosHelicityAngleMomentum(const Particle* part)
    {

      const auto& frame = ReferenceFrame::GetCurrent();
      B2Vector3D motherBoost = frame.getMomentum(part).BoostToCM();
      B2Vector3D motherMomentum = frame.getMomentum(part).Vect();
      const auto& daughters = part -> getDaughters() ;

      if (daughters.size() == 2) {

        bool isOneConversion = false;

        for (auto& idaughter : daughters) {
          if (idaughter -> getNDaughters() == 2) {
            if (std::abs(idaughter -> getDaughters()[0]-> getPDGCode()) == Const::electron.getPDGCode()
                && std::abs(idaughter -> getDaughters()[1]-> getPDGCode()) == Const::electron.getPDGCode()) {
              isOneConversion = true;
            }
          }
        }

        if (isOneConversion) {
          //only for pi0 decay where one gamma converts

          PxPyPzEVector pGamma;

          for (auto& idaughter : daughters) {
            if (idaughter -> getNDaughters() == 2) continue;
            else pGamma = frame.getMomentum(idaughter);
          }

          pGamma = Boost(motherBoost) * pGamma;

          return std::cos(motherMomentum.Angle(pGamma.Vect()));

        } else {
          PxPyPzEVector pDaughter1 = frame.getMomentum(daughters[0]);
          PxPyPzEVector pDaughter2 = frame.getMomentum(daughters[1]);

          pDaughter1 = Boost(motherBoost) * pDaughter1;
          pDaughter2 = Boost(motherBoost) * pDaughter2;

          B2Vector3D p12 = (pDaughter2 - pDaughter1).Vect();

          return std::cos(motherMomentum.Angle(p12));
        }

      } else if (daughters.size() == 3) {

        PxPyPzEVector pDaughter1 = frame.getMomentum(daughters[0]);
        PxPyPzEVector pDaughter2 = frame.getMomentum(daughters[1]);
        PxPyPzEVector pDaughter3 = frame.getMomentum(daughters[2]);

        pDaughter1 = Boost(motherBoost) * pDaughter1;
        pDaughter2 = Boost(motherBoost) * pDaughter2;
        pDaughter3 = Boost(motherBoost) * pDaughter3;

        B2Vector3D p12 = (pDaughter2 - pDaughter1).Vect();
        B2Vector3D p13 = (pDaughter3 - pDaughter1).Vect();

        B2Vector3D n = p12.Cross(p13);

        return std::cos(motherMomentum.Angle(n));

      }  else return std::numeric_limits<float>::quiet_NaN();

    }

    double cosHelicityAngleMomentumPi0Dalitz(const Particle* part)
    {

      const auto& frame = ReferenceFrame::GetCurrent();
      B2Vector3D motherBoost = frame.getMomentum(part).BoostToCM();
      B2Vector3D motherMomentum = frame.getMomentum(part).Vect();
      const auto& daughters = part -> getDaughters() ;


      if (daughters.size() == 3) {

        PxPyPzEVector pGamma;

        for (auto& idaughter : daughters) {
          if (std::abs(idaughter -> getPDGCode()) == Const::photon.getPDGCode()) pGamma = frame.getMomentum(idaughter);
        }

        pGamma = Boost(motherBoost) * pGamma;

        return std::cos(motherMomentum.Angle(pGamma.Vect()));

      }  else return std::numeric_limits<float>::quiet_NaN();

    }


    double cosHelicityAngleBeamMomentum(const Particle* mother, const std::vector<double>& index)
    {
      if (index.size() != 1) {
        B2FATAL("Wrong number of arguments for cosHelicityAngleIfCMSIsTheMother");
      }

      int idau = std::lround(index[0]);

      const Particle* part = mother->getDaughter(idau);
      if (!part) {
        B2FATAL("Couldn't find the " << idau << "th daughter");
      }

      PxPyPzEVector beam4Vector(getBeamPx(nullptr), getBeamPy(nullptr), getBeamPz(nullptr), getBeamE(nullptr));
      PxPyPzEVector part4Vector = part->get4Vector();
      PxPyPzEVector mother4Vector = mother->get4Vector();

      B2Vector3D motherBoost = mother4Vector.BoostToCM();

      beam4Vector = Boost(motherBoost) * beam4Vector;
      part4Vector = Boost(motherBoost) * part4Vector;

      return - part4Vector.Vect().Dot(beam4Vector.Vect()) / part4Vector.P() / beam4Vector.P();
    }


    double cosHelicityAngle(const Particle* mother, const std::vector<double>& indices)
    {
      if (indices.size() != 2) {
        B2FATAL("Wrong number of arguments for cosHelicityAngleIfRefFrameIsTheDaughter: two are needed.");
      }

      int iDau = std::lround(indices[0]);
      int iGrandDau = std::lround(indices[1]);

      const Particle* daughter = mother->getDaughter(iDau);
      if (!daughter)
        B2FATAL("Couldn't find the " << iDau << "th daughter.");

      const Particle* grandDaughter = daughter->getDaughter(iGrandDau);
      if (!grandDaughter)
        B2FATAL("Couldn't find the " << iGrandDau << "th daughter of the " << iDau << "th daughter.");

      PxPyPzEVector mother4Vector = mother->get4Vector();
      PxPyPzEVector daughter4Vector = daughter->get4Vector();
      PxPyPzEVector grandDaughter4Vector = grandDaughter->get4Vector();

      B2Vector3D daughterBoost = daughter4Vector.BoostToCM();

      // We boost the momentum of the mother and of the granddaughter to the reference frame of the daughter.
      grandDaughter4Vector = Boost(daughterBoost) * grandDaughter4Vector;
      mother4Vector = Boost(daughterBoost) * mother4Vector;

      return - grandDaughter4Vector.Vect().Dot(mother4Vector.Vect()) / grandDaughter4Vector.P() / mother4Vector.P();
    }

    double cosAcoplanarityAngle(const Particle* mother, const std::vector<double>& granddaughters)
    {
      if (granddaughters.size() != 2) {
        B2FATAL("Wrong number of arguments for cosAcoplanarityAngleIfRefFrameIsTheMother: two are needed.");
      }

      if (mother->getNDaughters() != 2)
        B2FATAL("cosAcoplanarityAngleIfRefFrameIsTheMother: this variable works only for two-body decays.");

      int iGrandDau1 = std::lround(granddaughters[0]);
      int iGrandDau2 = std::lround(granddaughters[1]);

      const Particle* daughter1 = mother->getDaughter(0);
      const Particle* daughter2 = mother->getDaughter(1);

      const Particle* grandDaughter1 = daughter1->getDaughter(iGrandDau1);
      if (!grandDaughter1)
        B2FATAL("Couldn't find the " << iGrandDau1 << "th daughter of the first daughter.");

      const Particle* grandDaughter2 = daughter2->getDaughter(iGrandDau2);
      if (!grandDaughter2)
        B2FATAL("Couldn't find the " << iGrandDau2 << "th daughter of the second daughter.");

      PxPyPzEVector mother4Vector = mother->get4Vector();
      PxPyPzEVector daughter4Vector1 = daughter1->get4Vector();
      PxPyPzEVector daughter4Vector2 = daughter2->get4Vector();
      PxPyPzEVector grandDaughter4Vector1 = grandDaughter1->get4Vector();
      PxPyPzEVector grandDaughter4Vector2 = grandDaughter2->get4Vector();

      B2Vector3D motherBoost = mother4Vector.BoostToCM();
      B2Vector3D daughter1Boost = daughter4Vector1.BoostToCM();
      B2Vector3D daughter2Boost = daughter4Vector2.BoostToCM();

      // Boosting daughters to reference frame of the mother
      daughter4Vector1 = Boost(motherBoost) * daughter4Vector1;
      daughter4Vector2 = Boost(motherBoost) * daughter4Vector2;

      // Boosting each granddaughter to reference frame of its mother
      grandDaughter4Vector1 = Boost(daughter1Boost) * grandDaughter4Vector1;
      grandDaughter4Vector2 = Boost(daughter2Boost) * grandDaughter4Vector2;

      // We calculate the normal vectors of the decay two planes
      B2Vector3D normalVector1 = daughter4Vector1.Vect().Cross(grandDaughter4Vector1.Vect());
      B2Vector3D normalVector2 = daughter4Vector2.Vect().Cross(grandDaughter4Vector2.Vect());

      return std::cos(normalVector1.Angle(normalVector2));
    }

    double cosHelicityAnglePrimary(const Particle* part)
    {
      return part->getCosHelicity();
    }

    double cosHelicityAngleDaughter(const Particle* part, const std::vector<double>& indices)
    {
      if ((indices.size() == 0) || (indices.size() > 2)) {
        B2FATAL("Wrong number of arguments for cosHelicityAngleDaughter: one or two are needed.");
      }

      int iDaughter = std::lround(indices[0]);
      int iGrandDaughter = 0;
      if (indices.size() == 2) {
        iGrandDaughter = std::lround(indices[1]);
      }

      return part->getCosHelicityDaughter(iDaughter, iGrandDaughter);
    }

    double acoplanarityAngle(const Particle* part)
    {
      return part->getAcoplanarity();
    }


    double cosHelicityAngleForQuasiTwoBodyDecay(const Particle* mother, const std::vector<double>& indices)
    {
      if (indices.size() != 2) {
        B2FATAL("Wrong number of arguments for cosHelicityAngleForQuasiTwoBodyDecay: two are needed.");
      }

      if (mother->getNDaughters() != 3)
        return std::numeric_limits<float>::quiet_NaN();

      int iDau = std::lround(indices[0]);
      int jDau = std::lround(indices[1]);

      const Particle* iDaughter = mother->getDaughter(iDau);
      if (!iDaughter)
        return std::numeric_limits<float>::quiet_NaN();

      const Particle* jDaughter = mother->getDaughter(jDau);
      if (!jDaughter)
        return std::numeric_limits<float>::quiet_NaN();

      PxPyPzEVector mother4Vector = mother->get4Vector();
      PxPyPzEVector iDaughter4Vector = iDaughter->get4Vector();
      PxPyPzEVector jDaughter4Vector = jDaughter->get4Vector();

      PxPyPzEVector resonance4Vector = iDaughter4Vector + jDaughter4Vector;
      B2Vector3D resonanceBoost = resonance4Vector.BoostToCM();

      iDaughter4Vector = Boost(resonanceBoost) * iDaughter4Vector;
      mother4Vector = Boost(resonanceBoost) * mother4Vector;

      return - iDaughter4Vector.Vect().Dot(mother4Vector.Vect()) / iDaughter4Vector.P() / mother4Vector.P();
    }

    double momentaTripleProduct(const Particle* mother, const std::vector<double>& indices)
    {
      if (indices.size() != 3) {
        B2FATAL("Wrong number of arguments for momentaTripleProduct: three are needed.");
      }
      if (mother->getNDaughters() < 3) {
        B2FATAL("Currently momentaTripleProduct() only supports the four-body (M->D1D2D3D4) or three-body (M->[R->D1D2]D3D4) decays.");
      }

      int iDau = std::lround(indices[0]);
      int jDau = std::lround(indices[1]);
      int kDau = std::lround(indices[2]);

      const Particle* iDaughter = mother->getNDaughters() == 3 ? mother->getDaughter(0)->getDaughter(iDau) : mother->getDaughter(iDau);
      if (!iDaughter) {
        if (mother->getNDaughters() == 4) B2FATAL("Couldn't find the " << iDau << "th daughter.");
        else if (mother->getNDaughters() == 3) B2FATAL("Couldn't find the " << iDau <<
                                                         "th granddaughter (daughter of the first daughter).");
      }
      const Particle* jDaughter =  mother->getDaughter(jDau);
      if (!jDaughter)
        B2FATAL("Couldn't find the " << jDau << "th daughter.");
      const Particle* kDaughter =  mother->getDaughter(kDau);
      if (!kDaughter)
        B2FATAL("Couldn't find the " << kDau << "th daughter.");

      PxPyPzEVector mother4Vector = mother->get4Vector();
      PxPyPzEVector iDaughter4Vector = iDaughter->get4Vector();
      PxPyPzEVector jDaughter4Vector = jDaughter->get4Vector();
      PxPyPzEVector kDaughter4Vector = kDaughter->get4Vector();

      B2Vector3D motherBoost = mother4Vector.BoostToCM();

      // We boost the momenta of offspring to the reference frame of the mother.
      iDaughter4Vector = Boost(motherBoost) * iDaughter4Vector;
      jDaughter4Vector = Boost(motherBoost) * jDaughter4Vector;
      kDaughter4Vector = Boost(motherBoost) * kDaughter4Vector;

      // cross product: p_j x p_k
      B2Vector3D jkDaughterCrossProduct = jDaughter4Vector.Vect().Cross(kDaughter4Vector.Vect());
      // triple product: p_i * (p_j x p_k)
      return iDaughter4Vector.Vect().Dot(jkDaughterCrossProduct) ;
    }

    VARIABLE_GROUP("Helicity variables");

    REGISTER_VARIABLE("cosHelicityAngleMomentum", cosHelicityAngleMomentum, R"DOC(
                      If the given particle has two daughters: cosine of the angle between the line defined by the momentum difference of the two daughters
                      in the frame of the given particle (mother)
                      and the momentum of the given particle in the lab frame.

                      If the given particle has three daughters: cosine of the angle between the normal vector of the plane defined by 
                      the momenta of the three daughters in the frame of the given particle (mother)
                      and the momentum of the given particle in the lab frame.

                      Otherwise, it returns 0.)DOC");

    REGISTER_VARIABLE("cosHelicityAngleMomentumPi0Dalitz", cosHelicityAngleMomentumPi0Dalitz, R"DOC(
                      To be used for the decay :math:`\pi^0 \to e^+ e^- \gamma`: 
                      cosine of the angle between the momentum of the gamma in the frame of the given particle (mother)
                      and the momentum of the given particle in the lab frame.

                      Otherwise, it returns 0.)DOC");

    REGISTER_VARIABLE("cosHelicityAngleBeamMomentum(i)", cosHelicityAngleBeamMomentum, R"DOC(
                      Cosine of the helicity angle of the :math:`i`-th daughter of the particle provided,
                      assuming that the mother of the provided particle corresponds to the centre-of-mass system, whose parameters are
                      automatically loaded by the function, given the accelerator's conditions.)DOC");

    REGISTER_VARIABLE("cosHelicityAngle(i, j)", cosHelicityAngle, R"DOC(
                      Cosine of the helicity angle between the momentum of the selected granddaughter and the direction opposite to the momentum of the provided particle 
                      in the reference frame of the selected daughter (:math:`\theta_1` and :math:`\theta_2` in the
                      `PDG <https://journals.aps.org/prd/abstract/10.1103/PhysRevD.98.030001>`_ 2018, p. 722).

                      This variable needs two integer arguments: the first one, ``i``, is the index of the daughter and the second one, ``j`` is the index of the granddaughter.

                      For example, in the decay :math:`B^0 \to \left(J/\psi \to \mu^+ \mu^-\right) \left(K^{*0} \to K^+ \pi^-\right)`, 
                      if the provided particle is :math:`B^0` and the selected indices are (0, 0),
                      the variable will return the angle between the momentum of the :math:`\mu^+` and the direction opposite to the momentum of 
                      the :math:`B^0`, both momenta in the rest frame of the :math:`J/\psi`.

                      This variable is needed for angular analyses of :math:`B`-meson decays into two vector particles.)DOC");

    REGISTER_VARIABLE("cosAcoplanarityAngle(i, j)", cosAcoplanarityAngle, R"DOC(
                      Cosine of the acoplanarity angle (:math:`\Phi` in the `PDG Polarization Review <http://pdg.lbl.gov/2019/reviews/rpp2018-rev-b-decays-polarization.pdf>`_).
                      Given a two-body decay, the acoplanarity angle is defined as
                      the angle between the two decay planes in the reference frame of the mother. 

                      We calculate the acoplanarity angle as the angle between the two
                      normal vectors of the decay planes. Each normal vector is the cross product of the momentum of one daughter (in the frame of the mother) and the
                      momentum of one of the granddaughters (in the reference frame of the daughter).

                      This variable needs two integer arguments: the first one, ``i`` is the index of the first granddaughter, and the second one, ``j`` the index of the
                      second granddaughter. 

                      For example, in the decay :math:`B^0 \to \left(J/\psi \to \mu^+ \mu^-\right) \left(K^{*0} \to K^+ \pi^-\right)`, if the provided particle is :math:`B^0` and the selected indices are (0, 0),
                      the variable will return the acoplanarity using the :math:`\mu^+` and the :math:`K^+` granddaughters.)DOC");

    REGISTER_VARIABLE("cosHelicityAnglePrimary", cosHelicityAnglePrimary, R"DOC(
                      Cosine of the helicity angle (see``Particle::getCosHelicity``) assuming the center of mass system as mother rest frame.
                      See `PDG Polarization Review <http://pdg.lbl.gov/2019/reviews/rpp2018-rev-b-decays-polarization.pdf>`_ for the definition of the helicity angle.)DOC");

    REGISTER_VARIABLE("cosHelicityAngleDaughter(i [, j] )", cosHelicityAngleDaughter, R"DOC(
                      Cosine of the helicity angle of the i-th daughter (see ``Particle::getCosHelicityDaughter``).
                      The optional second argument is the index of the granddaughter that defines the angle, default is 0.

                      For example, in the decay: :math:`B^0 \to \left(J/\psi \to \mu^+ \mu^-\right) \left(K^{*0} \to K^+ \pi^-\right)`, if the provided particle is :math:`B^0` and the selected index is 0,
                      the variable will return the helicity angle of the :math:`\mu^+`.
                      If the selected index is 1 the variable will return the helicity angle of the :math:`K^+` (defined via the rest frame of the :math:`K^{*0}`).
                      In rare cases if one wanted the helicity angle of the second granddaughter, indices 1,1 would return the helicity angle of the :math:`\pi^-`).

                      See `PDG Polarization Review <http://pdg.lbl.gov/2019/reviews/rpp2018-rev-b-decays-polarization.pdf>`_ for the definition of the helicity angle.)DOC");

    REGISTER_VARIABLE("acoplanarityAngle", acoplanarityAngle, R"DOC(
                      Acoplanarity angle (see ``Particle::getAcoplanarity``) assuming a two body decay of the particle and its daughters.
                      See `PDG Polarization Review <http://pdg.lbl.gov/2019/reviews/rpp2018-rev-b-decays-polarization.pdf>`_ for the definition of the acoplanarity angle.)DOC",
                      "rad");

    REGISTER_VARIABLE("cosHelicityAngleForQuasiTwoBodyDecay(i, j)", cosHelicityAngleForQuasiTwoBodyDecay, R"DOC(
                      Cosine of the helicity angle between the momentum of the provided particle and the momentum of the first selected
                      daughter (i-th) in the reference frame of the sum of two selected daughters (i-th + j-th).

                      The variable is supposed to be used for the analysis of a quasi-two-body decay. The number of daughters of the given 
                      particle must be three. Otherwise, the variable returns NaN.

                      For example, in the decay :math:`\bar{B}^0 \to D^+ K^- K^{*0}`, if the provided particle is :math:`\bar{B}^0` and
                      the selected indices are (1, 2), the variable will return the angle between the momentum of the :math:`\bar{B}^0` 
                      and the momentum of the :math:`K^-`, both momenta in the rest frame of the :math:`K^- K^{*0}`.)DOC");

    REGISTER_VARIABLE("momentaTripleProduct(i,j,k)", momentaTripleProduct, R"DOC(
a triple-product of three momenta of final-state particles in the mother rest frame: :math:`C_T=\vec{p}_i\cdot(\vec{p}_j\times\vec{p}_k)`.
For four-body decay M->D1D2D3D3, momentaTripleProduct(0,1,2) returns a triple-product of three momenta of D1D2D3 in the mother M rest frame. 
It also supports the three-body decay in which one daughter has a secondary decay, 
e.g. for M->(R->D1D2)D3D4, momentaTripleProduct(0,1,2) returns a triple-product of three momenta of D1D3D4 in the mother M rest frame.
)DOC"); 

  }
}
