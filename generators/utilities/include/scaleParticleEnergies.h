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

  inline double getTotalEnergy(const std::vector<ROOT::Math::PxPyPzMVector>& ps)
  {
    double eTot = 0;
    for (auto p : ps)
      eTot += p.E();
    return eTot;
  }

  inline double getScaleFactor(const std::vector<ROOT::Math::PxPyPzMVector>& ps, double EcmsTarget)
  {
    double s = 0;
    for (auto p : ps)
      s +=  pow(p.P(), 2) / (2 * p.E());

    double dE = getTotalEnergy(ps) - EcmsTarget;
    double L = dE / s;

    return sqrt(1 - L);
  }


  inline std::vector<ROOT::Math::PxPyPzMVector> scaleMomenta(const std::vector<ROOT::Math::PxPyPzMVector>& ps, double C)
  {
    std::vector<ROOT::Math::PxPyPzMVector> psS(ps.size());
    for (unsigned i = 0; i < ps.size(); ++i)
      psS[i].SetCoordinates(C * ps[i].Px(), C * ps[i].Py(), C * ps[i].Pz(), ps[i].M());

    return psS;
  }


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
