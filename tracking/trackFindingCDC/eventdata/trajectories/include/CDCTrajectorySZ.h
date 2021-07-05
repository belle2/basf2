/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/geometry/UncertainSZLine.h>
#include <tracking/trackFindingCDC/geometry/Vector2D.h>

#include <TMath.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /**
     *  Linear trajectory in sz space.
     *
     *  Particle trajectories have a linear dependence between the travel distance s and \n
     *  the z coordinate as the magnetic fields along z does not disturb the free z movement. \n
     *  This class presents this trajectory projection by a line representation in sz space.
     *  The s coordinate is interpreted as the transvers travel distance, hence  \n
     *  the part of the travel distance see in the xy projection.
     */
    class CDCTrajectorySZ  {

    public:
      /// Getter for a default assumption about the parameter covariances.
      static SZCovariance getBasicSZCovariance();

    public:
      /// Default constructor for ROOT compatibility.
      CDCTrajectorySZ() {}

      /// Constructs the trajectory from the given two dimensional sz line.
      explicit CDCTrajectorySZ(const UncertainSZLine& szLine)
        : m_szLine(szLine)
      {}

      /// Constructs the trajectory with the given slope and start z.
      CDCTrajectorySZ(const double tanLambda, const double z0)
        : m_szLine(tanLambda, z0, getBasicSZCovariance())
      {}

      /// Constucts a basic assumption, what the z0 start position and the sz slope are, including some broad values for the covariance matrix.
      static CDCTrajectorySZ basicAssumption();

      /// Translates the travel distance to the z coordinate
      double mapSToZ(const double s = 0) const
      { return getSZLine()->map(s); }

      /**
       *  Translates the absolute value of the transvers momentum to the momentum in z direction.
       *  Scales the absolute value of  the transvers momentum to the z component of the momentum
       *  vector. The z component comes with the correct sign as reflected by increasing or
       *  decreasing z component. Note : This assumes s to be the transvers travel distance.
       */
      double mapPtToPz(const double pt) const
      { return getSZLine()->slope() * pt; }

      /// Calculates the distance along between the given point an the sz trajectory.
      double getZDist(const double s, const double z) const
      { return mapSToZ(s) - z; }

      /**
       *  Gets the signed distance of the sz point to the line in the sz plane
       *  This gives the signed distance of a given point in the sy plane \n
       *  Errors in s and z are equaly weighted in this distance \n
       *  ( which might be an incorrect choice to fit to ).
       *  The plus sign mean the point is below the line. For minus it is above.
       *  The later is naturally choosen as the direction of positiv advance is \n
       *  coaligned with the s axes
       */
      double getSZDist(const double s, const double z) const
      { return getSZLine()->distance(Vector2D(s, z)) ; }

      /**
       *  Passively Shift the s coordinate by the amount given.
       *  Adjust all transvers travel distance measurements by deltaS. \n
       *  The old values of s before the shift corresponds the new scale \n
       *  like old_s - deltaS = new_s. This corresponds to an passive transformation \n
       *  of s by deltaS. This can be used in conjunction with the return value of \n
       *  CDCTrajectory2D::setStartPos2D() in order to move the transvers travel distance \n
       *  by the same amount in both trajectories.
       */
      void passiveMoveS(const double deltaS)
      { m_szLine.passiveMoveBy(Vector2D(deltaS, 0)); }

      /// Indicates if the line has been fitted
      bool isFitted() const
      { return not m_szLine->isInvalid(); }

      /// Clears all information from this trajectory line
      void clear()
      { m_szLine.invalidate(); }

      /// Getter for the slope over the travel distance coordinate
      double getTanLambda() const
      { return getSZLine()->slope(); }

      /// Getter for the z coordinate at zero travel distance
      double getZ0() const
      { return getSZLine()->intercept(); }

      /// Getter for p-value
      double getPValue() const
      { return TMath::Prob(getChi2(), getNDF()); }

      /// Getter for the chi square value of the line fit
      double getChi2() const
      { return getSZLine().chi2(); }

      /// Setter for the chi square value of the line fit
      void setChi2(const double chi2)
      { return m_szLine.setChi2(chi2); }

      /// Getter for the number of degrees of freedom of the line fit.
      size_t getNDF() const
      { return getSZLine().ndf(); }

      /// Setter for the number of degrees of freedom of the line fit.
      void setNDF(std::size_t ndf)
      { return m_szLine.setNDF(ndf); }

      /// Getter for the line in sz space
      const UncertainSZLine& getSZLine() const
      { return m_szLine; }

      /// Setter for the line in sz space
      void setSZLine(const UncertainSZLine& szLine)
      { m_szLine = szLine; }

    private:
      /// Memory for the line representation
      UncertainSZLine m_szLine;
    };
  }
}
