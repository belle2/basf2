/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <mdst/dataobjects/MCParticleGraph.h>
#include <Math/Vector3D.h>
#include <Math/Vector4D.h>
#include <vector>

namespace Belle2 {

  /** Main function scaleParticleEnergies in this header
   * scales momenta in CMS of all particles in the final state by
   * a constant factor such that the overall collision energy is slightly changed.
   * It is used for MC generators which do not provide option to generate events
   * with smeared energy of initial particles. There is an assumption that matrix element
   * of the process does not change much when Ecms is varied by 5MeV/10580MeV = 0.05%
   * (typical Ecms smearing at Belle II).
   * This is typically the case if the cross section does not have the resonance peak around
   * collision energy.
   */

  /** Get total energy of all particles in the provided vector */
  inline double getTotalEnergy(const std::vector<ROOT::Math::PxPyPzMVector>& ps)
  {
    double eTot = 0;
    for (auto p : ps)
      eTot += p.E();
    return eTot;
  }

  /** Find approximative value of the momentum scaling factor which makes the total energy of the particles
      provided in the vector ps equal to EcmsTarget */
  inline double getScaleFactor(const std::vector<ROOT::Math::PxPyPzMVector>& ps, double EcmsTarget)
  {
    double s = 0;
    for (auto p : ps)
      s +=  pow(p.P(), 2) / (2 * p.E());

    double dE = getTotalEnergy(ps) - EcmsTarget;
    double L = dE / s;

    return sqrt(1 - L);
  }

  /** Scale momenta of the particles in the vector ps with a factor C.
      Result is returned as returned as a vector with the scaled momenta */
  inline std::vector<ROOT::Math::PxPyPzMVector> scaleMomenta(const std::vector<ROOT::Math::PxPyPzMVector>& ps, double C)
  {
    std::vector<ROOT::Math::PxPyPzMVector> psS(ps.size());
    for (unsigned i = 0; i < ps.size(); ++i)
      psS[i].SetCoordinates(C * ps[i].Px(), C * ps[i].Py(), C * ps[i].Pz(), ps[i].M());

    return psS;
  }

  /** Scale momenta of the particles by a constant factor such that total energy is changed
      to EcmsTarget. It also changes momenta of the incoming particles such that energy is
      conserved. Should be called in CM reference frame.
   */
  inline void scaleParticleEnergies(MCParticleGraph& mpg, double EcmsTarget)
  {
    // scale energy of incoming particles
    double eIn = EcmsTarget / 2;
    double pIn = sqrt(pow(eIn, 2) - pow(mpg[0].getMass(), 2));

    mpg[0].setMomentum(0, 0,  pIn);
    mpg[1].setMomentum(0, 0, -pIn);
    mpg[0].setEnergy(eIn);
    mpg[1].setEnergy(eIn);


    // extract momenta of final state particles
    std::vector<ROOT::Math::PxPyPzMVector> cmsMomenta(mpg.size() - 2);
    for (size_t i = 2; i < mpg.size(); ++i) {
      ROOT::Math::XYZVector p = mpg[i].getMomentum();
      cmsMomenta[i - 2].SetCoordinates(p.X(), p.Y(), p.Z(), mpg[i].getMass());
    }

    // calculate the scaling factor in an iterative way
    double C = 1;
    for (int i = 0; i < 10; ++i) {
      auto cmsMomentaScaled = scaleMomenta(cmsMomenta, C);
      double dC = getScaleFactor(cmsMomentaScaled, EcmsTarget);
      C *= dC;
      if (abs(dC - 1) < 3 * std::numeric_limits<double>::epsilon()) break;
    }

    // apply scaling factor on final-state particles
    auto cmsMomentaScaled = scaleMomenta(cmsMomenta, C);

    // change momenta in the MCParticleGraph
    for (unsigned i = 2; i < mpg.size(); ++i) {
      mpg[i].setMomentum(cmsMomentaScaled[i - 2].Px(), cmsMomentaScaled[i - 2].Py(), cmsMomentaScaled[i - 2].Pz());
      mpg[i].setEnergy(cmsMomentaScaled[i - 2].E());
    }
  }

}
