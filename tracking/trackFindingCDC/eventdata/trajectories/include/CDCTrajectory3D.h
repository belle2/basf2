/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectory2D.h>
#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectorySZ.h>

#include <tracking/trackFindingCDC/topology/ISuperLayer.h>

#include <tracking/trackFindingCDC/geometry/UncertainHelix.h>
#include <tracking/trackFindingCDC/geometry/Vector3D.h>
#include <tracking/trackFindingCDC/geometry/Vector2D.h>

#include <framework/datastore/StoreArray.h>

#include <TMath.h>
#include <cmath>

namespace genfit {
  class TrackCand;
}

namespace Belle2 {
  class MCParticle;
  class RecoTrack;


  namespace TrackFindingCDC {

    /// Particle full three dimensional trajectory.
    class CDCTrajectory3D  {

    public:
      /// Default constructor for ROOT compatibility.
      CDCTrajectory3D()
        : m_localOrigin(),
          m_localHelix()
      {}

      /// Constructs a trajectory from a helix with reference point equivalent to the origin.
      explicit CDCTrajectory3D(const UncertainHelix& helix)
        : m_localOrigin(0.0, 0.0, 0.0),
          m_localHelix(helix),
          m_flightTime(0.0)
      {}

      /// Constructs a trajectory from a local helix taken as relative to the given origin.
      CDCTrajectory3D(const Vector3D& localOrigin,
                      const UncertainHelix& localHelix,
                      double flightTime = NAN)
        : m_localOrigin(localOrigin),
          m_localHelix(localHelix),
          m_flightTime(flightTime)
      {}

      /// Construct a three dimensional trajectory from a two dimensional circular trajectory and sz linear trajectory
      CDCTrajectory3D(const CDCTrajectory2D& trajectory2D,
                      const CDCTrajectorySZ& trajectorySZ);

      /**
       *  Construct a trajectory from a two dimensional circular trajectory
       *  filling the remaining two parameters and covariance matrix with default values.
       */
      explicit CDCTrajectory3D(const CDCTrajectory2D& trajectory2D);

      /**
       *  Construct a trajectory with given start point, momentum at the start point and given charge.
       *  Additionally this can takes an explicit bZ value instead of a field value from the instance BFieldMap.
       */
      CDCTrajectory3D(const Vector3D& startPoint,
                      const double startTime,
                      const Vector3D& startMomentum,
                      const double charge,
                      const double bZ);

      /// Construct a trajectory with given start point, momentum at the start point and given charge.
      CDCTrajectory3D(const Vector3D& startPoint,
                      const double startTime,
                      const Vector3D& startMomentum,
                      const double charge);

      /// Construct a trajectory from the MCParticles vertex and momentum.
      CDCTrajectory3D(const MCParticle& mcParticle, const double bZ);

      /// Construct a trajectory from the MCParticles vertex and momentum.
      explicit CDCTrajectory3D(const MCParticle& mcParticle);

      /// Construct a trajectory by extracting the seed position of the genfit::TrackCand
      CDCTrajectory3D(const genfit::TrackCand& gfTrackCand, const double bZ);

      /// Construct a trajectory by extracting the seed position of the genfit::TrackCand
      explicit CDCTrajectory3D(const genfit::TrackCand& gfTrackCand);

    public:
      /// Checks if the trajectory is already set to a valid value.
      bool isInvalid() const
      { return m_localHelix->isInvalid(); }

      /// Checks if the trajectory has already been set to a valid value.
      bool isFitted() const
      { return not isInvalid(); }

    public:
      /// Copies the trajectory information to the Genfit track candidate.
      bool fillInto(genfit::TrackCand& trackCand) const;

      /// Copies the trajectory information to the Genfit track candidate
      bool fillInto(genfit::TrackCand& trackCand, const double bZ) const;

      /// Copies the trajectory information to the RecoTrack
      RecoTrack* storeInto(StoreArray<RecoTrack>& recoTracks) const;

      /// Copies the trajectory information to the RecoTrack
      RecoTrack* storeInto(StoreArray<RecoTrack>& recoTracks, const double bZ) const;


    public:
      /// Calculates the closest approach on the trajectory to the global origin
      Vector2D getGlobalPerigee() const
      { return getGlobalCircle().perigee(); }

      /// Getter for the maximal distance from the origin
      double getMaximalCylindricalR() const
      { return getGlobalCircle().maximalCylindricalR(); }

      /// Getter for the minimal distance from the origin - same as absolute value of the impact parameter
      double getMinimalCylindricalR() const
      { return getGlobalCircle().minimalCylindricalR(); }

      /// Getter for the signed impact parameter of the trajectory
      double getGlobalImpact() const
      { return getLocalCircle()->distance(-m_localOrigin.xy()); }

    public:
      /// Gets the charge sign of the trajectory.
      ESign getChargeSign() const;

      /// Reverses the trajectory in place.
      void reverse()
      {
        m_localHelix.reverse();
        m_flightTime = -m_flightTime;
      }

      /// Returns the reverse trajectory as a copy.
      CDCTrajectory3D reversed() const
      {
        return CDCTrajectory3D(getLocalOrigin(), getLocalHelix().reversed(), -getFlightTime());
      }

      /**
       *  Calculate the travel distance from the start position of the trajectory.
       *  Returns the travel distance on the trajectory from the start point to \n
       *  the given point. This is subjected to a discontinuity at the far point \n
       *  of the circle. Hence the value return is in the range from -pi*radius to pi*radius \n
       *  If you have a heavily curling track you have care about the feasibility of this \n
       *  calculation.
       */
      double calcArcLength2D(const Vector3D& point) const
      { return getLocalHelix()->circleXY().arcLengthBetween(Vector2D(0.0, 0.0), (point - getLocalOrigin()).xy()); }

      /*
       *  Get unit momentum vector at a specific postion.
       *  @return the unit travel direction at the closest approach to the position
       */
      //Vector3D getUnitMom3D(const Vector3D& point) const
      //{ return getLocalCircle().tangential(point - getLocalOrigin().xy()); }

      /// Get the estimation for the absolute value of the transvers momentum
      double getAbsMom3D(const double bZ) const;

      /// Get the estimation for the absolute value of the transvers momentum
      double getAbsMom3D() const;

      /// Get the momentum at the start point of the trajectory
      Vector3D getMom3DAtSupport(const double bZ) const
      { return  getUnitMom3DAtSupport() *= getAbsMom3D(bZ);  }

      /// Get the momentum at the start point of the trajectory
      Vector3D getMom3DAtSupport() const
      { return  getUnitMom3DAtSupport() *= getAbsMom3D();  }

      /// Get the unit momentum at the start point of the trajectory
      Vector3D getUnitMom3DAtSupport() const
      { return  getLocalHelix()->tangential();  }


      /// Get the support point of the trajectory in global coordinates
      Vector3D getSupport() const
      { return getLocalHelix()->perigee() + getLocalOrigin(); }

      /// Clears all information from this trajectoy
      void clear()
      {
        m_localOrigin.set(0.0, 0.0, 0.0);
        m_localHelix.invalidate();
        m_flightTime = NAN;
      }

    public:
      /// Getter for the slope of z over the transverse travel distance s.
      double getTanLambda() const
      { return getLocalHelix()->tanLambda(); }

      /// Shifts the tanLambda and z0 by the given amount. Method is specific to the corrections in the fusion fit.
      void shiftTanLambdaIntercept(const double tanLambdaShift, const double zShift)
      {
        // Temporary workaround - to be removed soonish
        const_cast<Helix&>(m_localHelix.helix()).shiftTanLambdaZ0(tanLambdaShift, zShift);
      }

      /**
       *  Adjusts the z0 to the one that lies n periods forward
       *  @returns The two dimensional arc length needed to travel from the old to the new support point.
       */
      double shiftPeriod(int nPeriods);

      /// Getter for the curvature as seen from the xy projection.
      double getCurvatureXY() const
      { return getLocalHelix()->curvatureXY(); }

      /// Getter for an individual element of the covariance matrix of the local helix parameters.
      double getLocalCovariance(EHelixParameter iRow, EHelixParameter iCol) const
      { return getLocalHelix().covariance(iRow, iCol); }

      /// Getter for an individual diagonal element of the covariance matrix of the local helix parameters.
      double getLocalVariance(EHelixParameter i) const
      { return getLocalHelix().variance(i); }


      /// Getter for the circle in global coordinates.
      PerigeeCircle getGlobalCircle() const
      {
        // Down cast since we do not necessarily wont the covariance matrix transformed as well
        PerigeeCircle result(getLocalHelix()->circleXY());
        result.passiveMoveBy(-getLocalOrigin().xy());
        return result;
      }

      /// Getter for the circle in local coordinates
      UncertainPerigeeCircle getLocalCircle() const
      { return getLocalHelix().uncertainCircleXY(); }

      /// Getter for the sz line starting from the local origin
      UncertainSZLine getLocalSZLine() const
      { return getLocalHelix().uncertainSZLine(); }

      ///  Getter for p-value
      double getPValue() const
      { return TMath::Prob(getChi2(), getNDF()); }

      /// Getter for the chi2 value of the fit
      double getChi2() const
      { return getLocalHelix().chi2(); }

      /// Setter for the chi square value of the helix fit
      void setChi2(const double chi2)
      { return m_localHelix.setChi2(chi2); }

      /// Getter for the number of degrees of freedom of the helix fit.
      size_t getNDF() const
      { return getLocalHelix().ndf(); }

      /// Setter for the number of degrees of freedom of the helix fit.
      void setNDF(const size_t& ndf)
      { return m_localHelix.setNDF(ndf); }

      /// Getter for the two dimensional trajectory
      CDCTrajectory2D getTrajectory2D() const
      {
        return CDCTrajectory2D(getLocalOrigin().xy(),
                               getLocalHelix().uncertainCircleXY(),
                               getFlightTime());
      }

      /// Getter for the sz trajectory
      CDCTrajectorySZ getTrajectorySZ() const
      {
        UncertainSZLine globalSZLine = getLocalHelix().uncertainSZLine();
        globalSZLine.passiveMoveBy(Vector2D(0, -getLocalOrigin().z()));
        return CDCTrajectorySZ(globalSZLine);
      }

      /// Getter for the helix in local coordinates.
      const UncertainHelix& getLocalHelix() const
      { return m_localHelix; }

      /// Setter for the helix that describes the trajectory in local coordinates.
      void setLocalHelix(const UncertainHelix& localHelix)
      { m_localHelix = localHelix; }

      /// Getter for the origin of the local coordinate system
      const Vector3D& getLocalOrigin() const
      { return m_localOrigin; }

      /**
       *  Setter for the origin of the local coordinate system.
       *  This sets the origin point the local helix representation is subjected.
       *  The local helix is changed such that the set of points in global space is not changed.
       */
      double setLocalOrigin(const Vector3D& localOrigin);

      /// Getter for the time when the particle reached the support point position.
      double getFlightTime() const
      { return m_flightTime; }

      /// Setter for the time when the particle reached the support point position.
      void setFlightTime(double flightTime)
      { m_flightTime = flightTime; }

      /// Output helper for debugging
      friend std::ostream& operator<<(std::ostream& output, const CDCTrajectory3D& trajectory3D)
      {
        return output << "Local origin : " << trajectory3D.getLocalOrigin() <<  ", "
               << "local helix : " << trajectory3D.getLocalHelix();
      }

    private:
      /// Memory for local coordinate origin of the circle representing the trajectory in global coordinates
      Vector3D m_localOrigin;

      /// Memory for the generalized circle describing the trajectory in coordinates from the local origin
      UncertainHelix m_localHelix;

      /// Memory for the estimation of the time at which the particle arrived at the support point.
      double m_flightTime = NAN;

    }; //class

  } // namespace TrackFindingCDC
} // namespace Belle2
