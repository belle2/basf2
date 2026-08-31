/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

// Own header.
#include <analysis/variables/HelicityVariables.h>

#include <analysis/variables/EventVariables.h>

#include <analysis/dataobjects/Particle.h>

#include <analysis/utility/ReferenceFrame.h>
#include <analysis/VariableManager/Manager.h>

#include <framework/gearbox/Const.h>

#include <Math/Boost.h>
#include <Math/Vector4D.h>
#include <Math/VectorUtil.h>
using namespace ROOT::Math;
#include <cmath>

namespace Belle2 {
  namespace Variable {

    double cosHelicityAngleMomentum(const Particle* part)
    {

      const auto& frame = ReferenceFrame::GetCurrent();
      XYZVector motherBoost = frame.getMomentum(part).BoostToCM();
      PxPyPzEVector motherMomentum = frame.getMomentum(part);
      const auto& daughters = part -> getDaughters() ;

      if (daughters.size() == 2) {

        // Only for pi0 -> gamma gamma, gamma -> e+ e-
        bool isOneConversion = false;
        if (part->getPDGCode() == Const::pi0.getPDGCode()) {
          for (const auto* idaughter : daughters) {
            // both daughter must be gamma
            if (idaughter -> getPDGCode() != Const::photon.getPDGCode()) {
              isOneConversion = false;
              break;
            }
            // check if one of gammas has two daughters
            if (idaughter -> getNDaughters() == 2) {
              if (std::abs(idaughter -> getDaughters()[0]-> getPDGCode()) == Const::electron.getPDGCode()
                  && std::abs(idaughter -> getDaughters()[1]-> getPDGCode()) == Const::electron.getPDGCode()) { // e+ e-
                isOneConversion = true;
              }
            }
          }
        }

        if (isOneConversion) {
          B2WARNING("cosHelicityAngleMomentum: Special treatment for pi0->gamma gamma, gamma -> e+ e-, is called. "
                    "This treatment is going to be deprecated and we recommend using ``cosHelicityAngleMomentumPi0Dalitz`` "
                    "If you find this message in another case, it must be a bug. Please report it to the software mailing list.");

          //only for pi0 decay where one gamma converts
          PxPyPzEVector pGamma;
          for (const auto* idaughter : daughters) {
            if (idaughter -> getNDaughters() == 2) continue;
            else pGamma = frame.getMomentum(idaughter);
          }

          pGamma = Boost(motherBoost) * pGamma;

          return VectorUtil::CosTheta(motherMomentum, pGamma);

        } else {
          PxPyPzEVector pDaughter1 = frame.getMomentum(daughters[0]);
          PxPyPzEVector pDaughter2 = frame.getMomentum(daughters[1]);

          pDaughter1 = Boost(motherBoost) * pDaughter1;
          pDaughter2 = Boost(motherBoost) * pDaughter2;

          PxPyPzEVector p12 = pDaughter2 - pDaughter1;

          return VectorUtil::CosTheta(motherMomentum, p12);
        }

      } else if (daughters.size() == 3) {

        PxPyPzEVector pDaughter1 = frame.getMomentum(daughters[0]);
        PxPyPzEVector pDaughter2 = frame.getMomentum(daughters[1]);
        PxPyPzEVector pDaughter3 = frame.getMomentum(daughters[2]);

        pDaughter1 = Boost(motherBoost) * pDaughter1;
        pDaughter2 = Boost(motherBoost) * pDaughter2;
        pDaughter3 = Boost(motherBoost) * pDaughter3;

        XYZVector p12 = (pDaughter2 - pDaughter1).Vect();
        XYZVector p13 = (pDaughter3 - pDaughter1).Vect();

        XYZVector n = p12.Cross(p13);

        return VectorUtil::CosTheta(motherMomentum, n);

      }  else return Const::doubleNaN;

    }

    double cosHelicityAngleMomentumPi0Dalitz(const Particle* part)
    {

      const auto& frame = ReferenceFrame::GetCurrent();
      XYZVector motherBoost = frame.getMomentum(part).BoostToCM();
      PxPyPzEVector motherMomentum = frame.getMomentum(part);
      const auto& daughters = part -> getDaughters() ;


      if (daughters.size() == 3) {

        PxPyPzEVector pGamma;

        for (const auto* idaughter : daughters) {
          if (std::abs(idaughter -> getPDGCode()) == Const::photon.getPDGCode()) {
            pGamma = frame.getMomentum(idaughter);
            break;
          }
        }
        pGamma = Boost(motherBoost) * pGamma;

        return VectorUtil::CosTheta(motherMomentum, pGamma);

      } else if (daughters.size() == 2) { // only for pi0 -> gamma gamma, gamma -> e+ e-

        PxPyPzEVector pGamma;

        // both daughters must be gamma
        if (daughters[0] -> getPDGCode() != Const::photon.getPDGCode() or
            daughters[1] -> getPDGCode() != Const::photon.getPDGCode())
          return Const::doubleNaN;

        if (daughters[0] -> getNDaughters() == 2 and daughters[1] -> getNDaughters() == 0) {
          if (std::abs(daughters[0] -> getDaughters()[0]-> getPDGCode()) == Const::electron.getPDGCode()
              && std::abs(daughters[0] -> getDaughters()[1]-> getPDGCode()) == Const::electron.getPDGCode()) { // e+ e-
            pGamma = frame.getMomentum(daughters[1]);
          } else {
            return Const::doubleNaN;
          }
        } else if (daughters[0] -> getNDaughters() == 0 and daughters[1] -> getNDaughters() == 2) {
          if (std::abs(daughters[1] -> getDaughters()[0]-> getPDGCode()) == Const::electron.getPDGCode()
              && std::abs(daughters[1] -> getDaughters()[1]-> getPDGCode()) == Const::electron.getPDGCode()) { // e+ e-
            pGamma = frame.getMomentum(daughters[0]);
          } else {
            return Const::doubleNaN;
          }
        } else {
          return Const::doubleNaN;
        }

        pGamma = Boost(motherBoost) * pGamma;

        return VectorUtil::CosTheta(motherMomentum, pGamma);

      } else return Const::doubleNaN;

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

      XYZVector motherBoost = mother4Vector.BoostToCM();

      beam4Vector = Boost(motherBoost) * beam4Vector;
      part4Vector = Boost(motherBoost) * part4Vector;

      return - VectorUtil::CosTheta(part4Vector, beam4Vector);
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

      XYZVector daughterBoost = daughter4Vector.BoostToCM();

      // We boost the momentum of the mother and of the granddaughter to the reference frame of the daughter.
      grandDaughter4Vector = Boost(daughterBoost) * grandDaughter4Vector;
      mother4Vector = Boost(daughterBoost) * mother4Vector;

      return - VectorUtil::CosTheta(grandDaughter4Vector, mother4Vector);
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

      XYZVector motherBoost = mother4Vector.BoostToCM();
      XYZVector daughter1Boost = daughter4Vector1.BoostToCM();
      XYZVector daughter2Boost = daughter4Vector2.BoostToCM();

      // Boosting daughters to reference frame of the mother
      daughter4Vector1 = Boost(motherBoost) * daughter4Vector1;
      daughter4Vector2 = Boost(motherBoost) * daughter4Vector2;

      // Boosting each granddaughter to reference frame of its mother
      grandDaughter4Vector1 = Boost(daughter1Boost) * grandDaughter4Vector1;
      grandDaughter4Vector2 = Boost(daughter2Boost) * grandDaughter4Vector2;

      // We calculate the normal vectors of the decay two planes
      XYZVector normalVector1 = daughter4Vector1.Vect().Cross(grandDaughter4Vector1.Vect());
      XYZVector normalVector2 = daughter4Vector2.Vect().Cross(grandDaughter4Vector2.Vect());

      return VectorUtil::CosTheta(normalVector1, normalVector2);
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
        return Const::doubleNaN;

      int iDau = std::lround(indices[0]);
      int jDau = std::lround(indices[1]);

      const Particle* iDaughter = mother->getDaughter(iDau);
      if (!iDaughter)
        return Const::doubleNaN;

      const Particle* jDaughter = mother->getDaughter(jDau);
      if (!jDaughter)
        return Const::doubleNaN;

      PxPyPzEVector mother4Vector = mother->get4Vector();
      PxPyPzEVector iDaughter4Vector = iDaughter->get4Vector();
      PxPyPzEVector jDaughter4Vector = jDaughter->get4Vector();

      PxPyPzEVector resonance4Vector = iDaughter4Vector + jDaughter4Vector;
      XYZVector resonanceBoost = resonance4Vector.BoostToCM();

      iDaughter4Vector = Boost(resonanceBoost) * iDaughter4Vector;
      mother4Vector = Boost(resonanceBoost) * mother4Vector;

      return - VectorUtil::CosTheta(iDaughter4Vector, mother4Vector);
    }

    Manager::FunctionPtr momentaTripleProduct(const std::vector<std::string>& arguments)
    {
      if (arguments.size() != 3) {
        B2FATAL("Wrong number of arguments for momentaTripleProduct: three (particles) are needed.");
      }

      // wrap with func and return it
      auto func = [arguments](const Particle * mother) -> double {
        auto iDau = arguments[0];
        auto jDau = arguments[1];
        auto kDau = arguments[2];

        const Particle* iDaughter = mother->getParticleFromGeneralizedIndexString(iDau);
        if (!iDaughter) B2FATAL("Couldn't find the " << iDau << "th daughter.");
        const Particle* jDaughter =  mother->getParticleFromGeneralizedIndexString(jDau);
        if (!jDaughter) B2FATAL("Couldn't find the " << jDau << "th daughter.");
        const Particle* kDaughter =  mother->getParticleFromGeneralizedIndexString(kDau);
        if (!kDaughter) B2FATAL("Couldn't find the " << kDau << "th daughter.");

        PxPyPzEVector mother4Vector = mother->get4Vector();
        PxPyPzEVector iDaughter4Vector = iDaughter->get4Vector();
        PxPyPzEVector jDaughter4Vector = jDaughter->get4Vector();
        PxPyPzEVector kDaughter4Vector = kDaughter->get4Vector();

        XYZVector motherBoost = mother4Vector.BoostToCM();

        // We boost the momenta of offspring to the reference frame of the mother.
        iDaughter4Vector = Boost(motherBoost) * iDaughter4Vector;
        jDaughter4Vector = Boost(motherBoost) * jDaughter4Vector;
        kDaughter4Vector = Boost(motherBoost) * kDaughter4Vector;

        // cross product: p_j x p_k
        XYZVector jkDaughterCrossProduct = jDaughter4Vector.Vect().Cross(kDaughter4Vector.Vect());
        // triple product: p_i * (p_j x p_k)
        return iDaughter4Vector.Vect().Dot(jkDaughterCrossProduct) ;
      };
      return func;
    }


    VARIABLE_GROUP("Helicity variables");

    REGISTER_VARIABLE("cosHelicityAngleMomentum", cosHelicityAngleMomentum, R"DOC(
Returns the cosine of an angle whose definition changes depending on how many daughters the particle has; otherwise it returns 0.0.

.. topic:: For two daughters

    The angle is between the vector defined by the momentum difference of the two daughters in the frame of the given particle
    (the mother) and the momentum of the given particle in the lab frame.

.. topic:: For three daughters

    The angle is between the normal vector of the plane defined by the momenta of the daughters in the frame of the given particle
    (the mother) and the momentum of the given particle in the lab frame.

)DOC");
    REGISTER_VARIABLE("cosHelicityAngleMomentumPi0Dalitz", cosHelicityAngleMomentumPi0Dalitz, R"DOC(
Returns the cosine of the angle of the photon in the frame of the given particle (the mother) and the momentum
of the given particle in the lab frame, otherwise it returns 0.0.

.. attention:: 

    This variable should only be used for the decays :math:`\pi^0 \to e^+ e^- \gamma` and
    :math:`\pi^0 \to \gamma \gamma, \gamma \to e^+ e^-`.

)DOC");
    REGISTER_VARIABLE("cosHelicityAngleBeamMomentum(i)", cosHelicityAngleBeamMomentum, R"DOC(
Returns the  cosine of the helicity angle of the :math:`i`-th daughter of the particle provided,
assuming that the mother of the provided particle corresponds to the centre-of-mass system, whose parameters are
automatically loaded by the function, given the accelerator's conditions.

)DOC");
    REGISTER_VARIABLE("cosHelicityAngle(i, j)", cosHelicityAngle, R"DOC(
Returns the cosine of the helicity angle between the momentum of the selected granddaughter (index ``j``) and
the direction opposite to the momentum of the provided particle, both calculated in the reference frame of the selected daughter (index ``i``).
This variable is useful for angular analyses of :math:`B`-meson decays into two vector particles.

For example, for the decay :math:`B^0 \to \left(J/\psi \to \mu^+ \mu^-\right) \left(K^{*0} \to K^+ \pi^-\right)`, if the provided particle
is :math:`B^0` and the selected indices are (0, 0), the variable will return the angle between the momentum of the :math:`\mu^+` and the
direction opposite to the momentum of the :math:`B^0`, with both momenta in the rest frame of the :math:`J/\psi`.

.. seealso:: The polarisation of :math:`B` decays is reviewed in this `PDG review <https://pdg.lbl.gov/2026/web/viewer.html?file=../reviews/rpp2026-rev-b-decays-polarization.pdf>`_.

)DOC");
    REGISTER_VARIABLE("cosAcoplanarityAngle(i, j)", cosAcoplanarityAngle, R"DOC(
Returns the cosine of the acoplanarity angle which, for a two-body decay, is defined as the angle between the two normal vectors
of the decay planes in the reference frame of the mother. Each normal vector is defined as the cross product of the momentum of
one daughter (in the frame of the mother) and the momentum of one of its granddaughters (in the frame of the daughter). The two integers
``i`` and ``j`` index the granddaughters for the first and second daughters, respectively. 

For example, for the decay  :math:`B^0 \to \left(J/\psi \to \mu^+ \mu^-\right) \left(K^{*0} \to K^+ \pi^-\right)`, if the provided particle
is :math:`B^0` and the selected indices are (0, 0), the variable will return the acoplanarity using the :math:`\mu^+` and :math:`K^+` granddaughters.

.. seealso:: The polarisation of :math:`B` decays is reviewed in this `PDG review <https://pdg.lbl.gov/2026/web/viewer.html?file=../reviews/rpp2026-rev-b-decays-polarization.pdf>`_.

)DOC");
    REGISTER_VARIABLE("cosHelicityAnglePrimary", cosHelicityAnglePrimary, R"DOC(
Returns the cosine of the helicity angle (see ``cosHelicityAngle``) assuming the CM system as the mother rest frame.

.. seealso:: The polarisation of :math:`B` decays is reviewed in this `PDG review <https://pdg.lbl.gov/2026/web/viewer.html?file=../reviews/rpp2026-rev-b-decays-polarization.pdf>`_.

)DOC");
    REGISTER_VARIABLE("cosHelicityAngleDaughter(i [, j] )", cosHelicityAngleDaughter, R"DOC(
Returns the cosine of the helicity angle of the daughter at index ``i``. The optional second argument is the index of the granddaughter that defines the angle
and by default is 0. 

For example, for the decay: :math:`B^0 \to \left(J/\psi \to \mu^+ \mu^-\right) \left(K^{*0} \to K^+ \pi^-\right)`, if the provided particle is :math:`B^0`
and the selected index is 0, the variable will return the helicity angle of the :math:`\mu^+`. If the selected index is 1 the variable will return the
helicity angle of the :math:`K^+` (defined via the rest frame of the :math:`K^{*0}`). In rare cases, if one wanted the helicity angle of the second granddaughter,
indices (1, 1) would return the helicity angle of the :math:`\pi^-`.

.. seealso:: The polarisation of :math:`B` decays is reviewed in this `PDG review <https://pdg.lbl.gov/2026/web/viewer.html?file=../reviews/rpp2026-rev-b-decays-polarization.pdf>`_.

)DOC");
    REGISTER_VARIABLE("acoplanarityAngle", acoplanarityAngle, R"DOC(
Returns the acoplanarity angle, as described in the definition for ``cosAcoplanarityAngle``, assuming a two body decay of the particle and
its daughters. 

.. seealso:: The polarisation of :math:`B` decays is reviewed in this `PDG review <https://pdg.lbl.gov/2026/web/viewer.html?file=../reviews/rpp2026-rev-b-decays-polarization.pdf>`_.

)DOC", "rad");
    REGISTER_VARIABLE("cosHelicityAngleForQuasiTwoBodyDecay(i, j)", cosHelicityAngleForQuasiTwoBodyDecay, R"DOC(
Returns the cosine of the helicity angle between the momentum of the provided particle and the momentum of the daughter at
index :math:`i` in the reference frame of the two selected daughters (at index :math:`i` and :math:`j`) combined. 

For example, for the decay :math:`\bar{B}^0 \to D^+ K^- K^{*0}`, if the provided particle is :math:`\bar{B}^0` and
the selected indices are (1, 2), the variable will return the angle between the momentum of the :math:`\bar{B}^0` and
the momentum of the :math:`K^-`, both in the rest frame of the :math:`K^- K^{*0}` combination.

.. important:: 
    The variable is supposed to be used for analyses of quasi-two-body decays. The number of daughters of the given particle
    must be three, otherwise the variable returns ``NaN``.

)DOC");
    REGISTER_METAVARIABLE("momentaTripleProduct(i,j,k)", momentaTripleProduct, R"DOC(
Returns a triple product of the three momenta as defined by 

.. math::
    C_T=\vec{p}_i\cdot(\vec{p}_j\times\vec{p}_k)

where :math:`i, j` and :math:`k` are indices of the daughters of the provided particle.

For example, in a four body decay :math:`M \rightarrow d_1 d_2 d_3 d_4`, for the selected indices (0, 1, 2), the variable will return
:math:`C_T` calculated using the momenta of the particles :math:`d_1, d_2` and :math:`d_3`. In instances of secondary decays such as
:math:`M \rightarrow R (\rightarrow d_1 d_2) d_3 d_4`, the selected indices (0:0, 1, 2) returns :math:`C_T` calculated using the
momenta of the particles :math:`d_1, d_3` and :math:`d_4`.

)DOC", Manager::VariableDataType::c_double); 

  }
}
