/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * This module calculates shower shape variables.                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Torben Ferber (ferber@physics.ubc.ca)                    *
 *               Guglielmo De Nardo (denardo@na.infn.it)                  *
 *               Alon Hershenhorn                                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ECLSHOWERSHAPEMODULE_H_
#define ECLSHOWERSHAPEMODULE_H_

// FRAMEWORK
#include <framework/core/Module.h>

// ECL
#include <ecl/dataobjects/ECLShower.h>
#include <ecl/geometry/ECLNeighbours.h>

namespace Belle2 {
  namespace ECL {

    /** Class to perform the shower correction */
    class ECLShowerShapeModule : public Module {

    public:
      /** Constructor. */
      ECLShowerShapeModule();

      /** Destructor. */
      ~ECLShowerShapeModule();

      /** Initialize. */
      virtual void initialize();

      /** Begin run. */
      virtual void beginRun();

      /** Event. */
      virtual void event();

      /** End run. */
      virtual void endRun();

      /** Terminate. */
      virtual void terminate();

    private:

      // Module Parameters
      /** Scaling factor for radial distances in perpendicular plane */
      double m_rho0;

      /** Average crystal dimension [cm] */
      double m_avgCrystalDimension;

      /**Struct used to hold information of the digits projected to a plane perpendicular to the shower direction */
      struct ProjectedECLDigit {

        /** weighted energy */
        double energy;

        /** radial distance */
        double rho;

        /** polar angel */
        double alpha;
      };

      /** Neighbour map 9 neighbours, for E9oE25 and E1oE9. */
      ECLNeighbours* m_neighbourMap9;

      /** Neighbour map 25 neighbours, for E9oE25. */
      ECLNeighbours* m_neighbourMap25;

      /** Shower shape variable: Lateral energy. */
      double computeLateralEnergy(const ECLShower&) const;

      /** Compute the absolute value of the complex Zernike moment Znm.
          The moments are computed in a plane perpendicular to the direction of the shower.
          The plane's origin is at the intersection of the shower direction with the plane.
          The origin is at a distance from the interaction point equal to the shower distance from the interaction point.

          n, m - are the Zernike polynomial rank
          R0 - is a scaling factor used to normalize the distances in the described plane.
          It also sets the maximum distance from the origin (the Zernike polynomials are defined only on the unit circle).
          All points in the plane with a distance larger than R0 from the origin are ignored.

          Valid values of n,m are n,m >= 0, m <= n.
          If n or m are invalid the function returns 0.0.
          */

      double computeAbsZernikeMoment(const std::vector<ProjectedECLDigit>& shower, const double totalEnergy, const int n, const int m,
                                     const double rho) const;

      /** Compute the second moment in the plane perpendicular to the direction of the shower.
          The plane's origin is at the intersection of the shower direction with the plane.
          The origin is at a distance from the interaction point equal to the shower distance from the interaction point. */
      double computeSecondMoment(const std::vector<ProjectedECLDigit>& shower, const double totalEnergy) const;

      /** Compute projections of the ECLCalDigits to the perpendicular plane */
      std::vector<ProjectedECLDigit> projectECLDigits(const ECLShower& shower) const;

      /** The radial part of the Zernike polynomial
       * n,m - Zernike polynomial rank
       * rho - radial distance             */
      double Rnm(const int n, const int m, const double rho) const;

      /** Return the complex value of the Zernike polynomial of rank n,m.
      Znm(rho,alpha) = Rnm(rho) * exp(i*m*alpha)
      rho - radial distance
      alpha - polar angle */
      std::complex<double> zernikeValue(const double rho, const double alpha, const int n, const int m) const;

      /** Shower shape variable: E9oE25
       The energy ratio is calculated taking the weighted 3x3 (=9) and the weighted 5x5 (=25) crystals around the central crystal.
       If the shower is smaller than this, the reduced number is used for this. */
      double computeE9oE25(const ECLShower&) const;

      /** Shower shape variable: E1oE9
       The energy ratio is calculated taking the weighted central (=1) and the weighted 3x3 (=9) crystals around the central crystal.
       If the shower is smaller than this, the reduced number is used for this. */
      double computeE1oE9(const ECLShower&) const;


    public:
      /** We need names for the data objects to differentiate between PureCsI and default*/

      /** Default name ECLShowers */
      virtual const char* eclShowerArrayName() const
      { return "ECLShowers" ; }

      /** Default name ECLCalDigits */
      virtual const char* eclCalDigitArrayName() const
      { return "ECLCalDigits" ; }

      /** Default name ECLConnectedRegions */
      virtual const char* eclConnectedRegionArrayName() const
      { return "ECLConnectedRegions" ; }

    }; // end of ECLShowerShapeModule


    /** The very same module but for PureCsI */
    class ECLShowerShapePureCsIModule : public ECLShowerShapeModule {
    public:

      /** PureCsI name ECLShowersPureCsI */
      virtual const char* eclShowerArrayName() const override
      { return "ECLShowersPureCsI" ; }

      /** PureCsI name ECLCalDigitsPureCsI */
      virtual const char* eclCalDigitArrayName() const override
      { return "ECLCalDigitsPureCsI" ; }

      /** PureCsI name ECLConnectedRegionsPureCsI */
      virtual const char* eclConnectedRegionArrayName() const override
      { return "ECLConnectedRegionsPureCsI" ; }

    }; // end of ECLShowerShapePureCsIModule

  } // end of ECL namespace
} // end of Belle2 namespace

#endif
