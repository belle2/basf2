/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <cmath>

namespace Belle2 {
  namespace TrackFindingCDC {

    class EnergyLossEstimator {

    public:
      /**
       *  Create an energy loss estimator with the material properties of the CDC
       */
      static EnergyLossEstimator forCDC();

      /**
       *  Lookup the mass for the given pdg code.
       */
      static double getMass(int pdgCode);

      /**
       *  Lookup the charge for the given pdg code.
       */
      static int getCharge(int pdgCode);

      /**
       *  Constructor from the material properties
       *
       *  @param eDensity Electron density in mol / cm ^ 3, e.g. from Z * density / A
       *  @param I        Excitation energy (must be given in standard GeV units)
       *  @param bZ       Mean magentic field in the z direction
       */
      EnergyLossEstimator(double eDensity, double I, double bZ = NAN);

      /**
       *  Stopping power aka energy loss / arc length
       *
       *  @param p       absolute momentum
       *  @param pdgCode
       */
      double getBetheStoppingPower(double p, int pdgCode) const;

      /**
       *  Calculates the total energy loss after travelling the given distance
       */
      double getEnergyLoss(double p, int pdgCode, double arcLength) const;

      /**
       *  Calculates a factor applicable scaling the current momentum to the momentum after
       *  traveling given arc length.
       */
      double getMomentumLossFactor(double p, int pdgCode, double arcLength) const;

      /**
       *  Calculates a correction term for the two dimensional distance undoing the energy loss after the given arc length
       */
      double getLossDist2D(double pt, int pdgCode, double arcLength2D) const;

    private:
      /// Electron density in mol / cm^3
      double m_eDensity;

      /// Mean excitation energy in GeV
      double m_I;

      /// B field to be used for the distance translation
      double m_bZ;
    };
  }
}
