/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/


#include <analysis/variables/HelicityVariables.h>
#include <analysis/variables/EventVariables.h>

#include <analysis/dataobjects/Particle.h>

#include <analysis/utility/ReferenceFrame.h>

#include <framework/utilities/Conversion.h>
#include <framework/gearbox/Const.h>

#include <TLorentzVector.h>
#include <TVector3.h>
#include <cmath>

namespace Belle2 {
  namespace Variable {

    double cosHelicityAngleMomentum(const Particle* part)
    {

      const auto& frame = ReferenceFrame::GetCurrent();
      TVector3 motherBoost = - frame.getMomentum(part).BoostVector();
      TVector3 motherMomentum = frame.getMomentum(part).Vect();
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

          TLorentzVector pGamma;

          for (auto& idaughter : daughters) {
            if (idaughter -> getNDaughters() == 2) continue;
            else pGamma = frame.getMomentum(idaughter);
          }

          pGamma.Boost(motherBoost);

          return std::cos(motherMomentum.Angle(pGamma.Vect()));

        } else {
          TLorentzVector pDaughter1 = frame.getMomentum(daughters[0]);
          TLorentzVector pDaughter2 = frame.getMomentum(daughters[1]);

          pDaughter1.Boost(motherBoost);
          pDaughter2.Boost(motherBoost);

          TVector3 p12 = (pDaughter2 - pDaughter1).Vect();

          return std::cos(motherMomentum.Angle(p12));
        }

      } else if (daughters.size() == 3) {

        TLorentzVector pDaughter1 = frame.getMomentum(daughters[0]);
        TLorentzVector pDaughter2 = frame.getMomentum(daughters[1]);
        TLorentzVector pDaughter3 = frame.getMomentum(daughters[2]);

        pDaughter1.Boost(motherBoost);
        pDaughter2.Boost(motherBoost);
        pDaughter3.Boost(motherBoost);

        TVector3 p12 = (pDaughter2 - pDaughter1).Vect();
        TVector3 p13 = (pDaughter3 - pDaughter1).Vect();

        TVector3 n = p12.Cross(p13);

        return std::cos(motherMomentum.Angle(n));

      }  else return std::numeric_limits<float>::quiet_NaN();

    }

    double cosHelicityAngleMomentumPi0Dalitz(const Particle* part)
    {

      const auto& frame = ReferenceFrame::GetCurrent();
      TVector3 motherBoost = - frame.getMomentum(part).BoostVector();
      TVector3 motherMomentum = frame.getMomentum(part).Vect();
      const auto& daughters = part -> getDaughters() ;


      if (daughters.size() == 3) {

        TLorentzVector pGamma;

        for (auto& idaughter : daughters) {
          if (std::abs(idaughter -> getPDGCode()) == Const::photon.getPDGCode()) pGamma = frame.getMomentum(idaughter);
        }

        pGamma.Boost(motherBoost);

        return std::cos(motherMomentum.Angle(pGamma.Vect()));

      }  else return std::numeric_limits<float>::quiet_NaN();

    }


    Manager::FunctionPtr cosHelicityAngleBeamMomentum(const std::vector<std::string>& arguments)
    {
      int idau = 0;
      if (arguments.size() == 1) {
        try {
          idau = Belle2::convertString<int>(arguments[0]);
        } catch (std::invalid_argument&) {
          B2FATAL("The argument of cosHelicityAngleWrtCMSFrame must be an integer!");
        }
      } else {
        B2FATAL("Wrong number of arguments for cosHelicityAngleIfCMSIsTheMother");
      }
      auto func = [idau](const Particle * mother) -> double {
        const Particle* part = mother->getDaughter(idau);
        if (!part)
        {
          B2FATAL("Couldn't find the " << idau << "th daughter");
        }

        TLorentzVector beam4Vector(getBeamPx(nullptr), getBeamPy(nullptr), getBeamPz(nullptr), getBeamE(nullptr));
        TLorentzVector part4Vector = part->get4Vector();
        TLorentzVector mother4Vector = mother->get4Vector();

        TVector3 motherBoost = -(mother4Vector.BoostVector());

        beam4Vector.Boost(motherBoost);
        part4Vector.Boost(motherBoost);

        return - part4Vector.Vect().Dot(beam4Vector.Vect()) / part4Vector.Vect().Mag() / beam4Vector.Vect().Mag();
      };
      return func;
    }


    Manager::FunctionPtr cosHelicityAngle(const std::vector<std::string>& arguments)
    {
      int iDau = 0;
      int iGrandDau = 0;
      if (arguments.size() == 2) {
        try {
          iDau = Belle2::convertString<int>(arguments[0]);
          iGrandDau = Belle2::convertString<int>(arguments[1]);
        } catch (std::invalid_argument&) {
          B2FATAL("The two arguments of cosHelicityAngleIfRefFrameIsTheDaughter must be integers!");
        }
      } else {
        B2FATAL("Wrong number of arguments for cosHelicityAngleIfRefFrameIsTheDaughter: two are needed.");
      }
      auto func = [iDau, iGrandDau](const Particle * mother) -> double {

        const Particle* daughter = mother->getDaughter(iDau);
        if (!daughter)
          B2FATAL("Couldn't find the " << iDau << "th daughter.");

        const Particle* grandDaughter = daughter->getDaughter(iGrandDau);
        if (!grandDaughter)
          B2FATAL("Couldn't find the " << iGrandDau << "th daughter of the " << iDau << "th daughter.");


        TLorentzVector mother4Vector = mother->get4Vector();
        TLorentzVector daughter4Vector = daughter->get4Vector();
        TLorentzVector grandDaughter4Vector = grandDaughter->get4Vector();

        TVector3 daughterBoost = -(daughter4Vector.BoostVector());

        // We boost the momentum of the mother and of the granddaughter to the reference frame of the daughter.
        grandDaughter4Vector.Boost(daughterBoost);
        mother4Vector.Boost(daughterBoost);

        return - grandDaughter4Vector.Vect().Dot(mother4Vector.Vect()) / grandDaughter4Vector.Vect().Mag() / mother4Vector.Vect().Mag();

      };
      return func;
    }

    Manager::FunctionPtr cosAcoplanarityAngle(const std::vector<std::string>& arguments)
    {
      int iGrandDau1 = 0;
      int iGrandDau2 = 0;
      if (arguments.size() == 2) {
        try {
          iGrandDau1 = Belle2::convertString<int>(arguments[0]);
          iGrandDau2 = Belle2::convertString<int>(arguments[1]);
        } catch (std::invalid_argument&) {
          B2FATAL("The two arguments of cosAcoplanarityAngleIfRefFrameIsTheMother must be integers!");
        }
      } else {
        B2FATAL("Wrong number of arguments for cosAcoplanarityAngleIfRefFrameIsTheMother: two are needed.");
      }
      auto func = [iGrandDau1, iGrandDau2](const Particle * mother) -> double {

        if (mother->getNDaughters() != 2)
          B2FATAL("cosAcoplanarityAngleIfRefFrameIsTheMother: this variable works only for two-body decays.");

        const Particle* daughter1 = mother-> getDaughter(0);
        const Particle* daughter2 = mother-> getDaughter(1);

        const Particle* grandDaughter1 = daughter1 -> getDaughter(iGrandDau1);
        if (!grandDaughter1)
          B2FATAL("Couldn't find the " << iGrandDau1 << "th daughter of the first daughter.");

        const Particle* grandDaughter2 = daughter2 -> getDaughter(iGrandDau2);
        if (!grandDaughter2)
          B2FATAL("Couldn't find the " << iGrandDau2 << "th daughter of the second daughter.");

        TLorentzVector mother4Vector = mother->get4Vector();
        TLorentzVector daughter4Vector1 = daughter1->get4Vector();
        TLorentzVector daughter4Vector2 = daughter2->get4Vector();
        TLorentzVector grandDaughter4Vector1 = grandDaughter1->get4Vector();
        TLorentzVector grandDaughter4Vector2 = grandDaughter2->get4Vector();

        TVector3 motherBoost = -(mother4Vector.BoostVector());
        TVector3 daughter1Boost = -(daughter4Vector1.BoostVector());
        TVector3 daughter2Boost = -(daughter4Vector2.BoostVector());

        // Boosting daughters to reference frame of the mother
        daughter4Vector1.Boost(motherBoost);
        daughter4Vector2.Boost(motherBoost);

        // Boosting each granddaughter to reference frame of its mother
        grandDaughter4Vector1.Boost(daughter1Boost);
        grandDaughter4Vector2.Boost(daughter2Boost);

        // We calculate the normal vectors of the decay two planes
        TVector3 normalVector1 = daughter4Vector1.Vect().Cross(grandDaughter4Vector1.Vect());
        TVector3 normalVector2 = daughter4Vector2.Vect().Cross(grandDaughter4Vector2.Vect());

        return std::cos(normalVector1.Angle(normalVector2));

      };
      return func;
    }


    double cosHelicityAnglePrimary(const Particle* part)
    {
      return part->getCosHelicity();
    }

    Manager::FunctionPtr cosHelicityAngleDaughter(const std::vector<std::string>& arguments)
    {
      int iDaughter = 0;
      int iGrandDaughter = 0;
      if ((arguments.size() == 0) || (arguments.size() > 2)) {
        B2FATAL("Wrong number of arguments for cosHelicityAngleDaughter: one or two are needed.");
      }
      try {
        iDaughter = Belle2::convertString<int>(arguments[0]);
        if (arguments.size() == 2) {
          iGrandDaughter = Belle2::convertString<int>(arguments[1]);
        }
      } catch (std::invalid_argument&) {
        B2FATAL("The arguments of cosHelicityAngleDaughter must be integers!");
      }

      auto func = [iDaughter, iGrandDaughter](const Particle * part) -> double {
        return part->getCosHelicityDaughter(iDaughter, iGrandDaughter);
      };
      return func;
    }

    double acoplanarityAngle(const Particle* part)
    {
      return part->getAcoplanarity();
    }


    VARIABLE_GROUP("Helicity variables");

    REGISTER_VARIABLE("cosHelicityAngleMomentum",
                      cosHelicityAngleMomentum,
                      R"DOC(
                      If the given particle has two daughters: cosine of the angle between the line defined by the momentum difference of the two daughters
                      in the frame of the given particle (mother)
                      and the momentum of the given particle in the lab frame.

                      If the given particle has three daughters: cosine of the angle between the normal vector of the plane defined by 
                      the momenta of the three daughters in the frame of the given particle (mother)
                      and the momentum of the given particle in the lab frame.

                      Otherwise, it returns 0.)DOC");

    REGISTER_VARIABLE("cosHelicityAngleMomentumPi0Dalitz",
                      cosHelicityAngleMomentumPi0Dalitz,
                      R"DOC(
                      To be used for the decay :math:`\pi^0 \to e^+ e^- \gamma`: 
                      cosine of the angle between the momentum of the gamma in the frame of the given particle (mother)
                      and the momentum of the given particle in the lab frame.

                      Otherwise, it returns 0.)DOC");

    REGISTER_VARIABLE("cosHelicityAngleBeamMomentum(i)", cosHelicityAngleBeamMomentum,
                      R"DOC(
                      Cosine of the helicity angle of the :math:`i`-th daughter of the particle provided,
                      assuming that the mother of the provided particle corresponds to the centre-of-mass system, whose parameters are
                      automatically loaded by the function, given the accelerator's conditions.)DOC");

    REGISTER_VARIABLE("cosHelicityAngle(i, j)", cosHelicityAngle,
                      R"DOC(
                      Cosine of the helicity angle between the momentum of the provided particle and the momentum of the selected granddaughter
                      in the reference frame of the selected daughter (:math:`\theta_1` and :math:`\theta_2` in the
                      `PDG <https://journals.aps.org/prd/abstract/10.1103/PhysRevD.98.030001>`_ 2018, p. 722).

                      This variable needs two integer arguments: the first one, ``i``, is the index of the daughter and the second one, ``j`` is the index of the granddaughter.

                      For example, in the decay :math:`B^0 \to \left(J/\psi \to \mu^+ \mu^-\right) \left(K^{*0} \to K^+ \pi^-\right)`, 
                      if the provided particle is :math:`B^0` and the selected indices are (0, 0),
                      the variable will return the angle between the momentum of the :math:`B^0` and the momentum of the :math:`\mu^+`,
                      both momenta in the rest frame of the :math:`J/\psi`.

                      This variable is needed for angular analyses of :math:`B`-meson decays into two vector particles.)DOC");

    REGISTER_VARIABLE("cosAcoplanarityAngle(i, j)", cosAcoplanarityAngle,
                      R"DOC(
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

    REGISTER_VARIABLE("cosHelicityAnglePrimary", cosHelicityAnglePrimary,
                      R"DOC(
                      Cosine of the helicity angle (see``Particle::getCosHelicity``) assuming the center of mass system as mother rest frame.
                      See `PDG Polarization Review <http://pdg.lbl.gov/2019/reviews/rpp2018-rev-b-decays-polarization.pdf>`_ for the definition of the helicity angle.)DOC");

    REGISTER_VARIABLE("cosHelicityAngleDaughter(i [, j] )", cosHelicityAngleDaughter,
                      R"DOC(
                      Cosine of the helicity angle of the i-th daughter (see ``Particle::getCosHelicityDaughter``).
                      The optional second argument is the index of the granddaughter that defines the angle, default is 0.

                      For example, in the decay: :math:`B^0 \to \left(J/\psi \to \mu^+ \mu^-\right) \left(K^{*0} \to K^+ \pi^-\right)`, if the provided particle is :math:`B^0` and the selected index is 0,
                      the variable will return the helicity angle of the :math:`\mu^+`.
                      If the selected index is 1 the variable will return the helicity angle of the :math:`K^+` (defined via the rest frame of the :math:`K^{*0}`).
                      In rare cases if one wanted the helicity angle of the second granddaughter, indices 1,1 would return the helicity angle of the :math:`\pi^-`).

                      See `PDG Polarization Review <http://pdg.lbl.gov/2019/reviews/rpp2018-rev-b-decays-polarization.pdf>`_ for the definition of the helicity angle.)DOC");

    REGISTER_VARIABLE("acoplanarityAngle", acoplanarityAngle,
                      R"DOC(
                      Acoplanarity angle (see ``Particle::getAcoplanarity``) assuming a two body decay of the particle and its daughters.
                      See `PDG Polarization Review <http://pdg.lbl.gov/2019/reviews/rpp2018-rev-b-decays-polarization.pdf>`_ for the definition of the acoplanarity angle.)DOC");

  }
}
