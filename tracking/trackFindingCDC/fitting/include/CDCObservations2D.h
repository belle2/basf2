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

#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectory3D.h>
#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectory2D.h>

#include <tracking/trackFindingCDC/fitting/EFitPos.h>
#include <tracking/trackFindingCDC/fitting/EFitVariance.h>

#include <Eigen/Dense>
#include <iterator>

namespace Belle2 {
  namespace TrackFindingCDC {

    class CDCWire;
    class CDCWireHit;
    class CDCRLWireHit;
    class CDCRLWireHitPair;
    class CDCRLWireHitTriple;
    class CDCFacet;
    class CDCRecoHit2D;
    class CDCRecoHit3D;
    class CDCWireHitSegment;
    class CDCRecoSegment2D;
    class CDCRecoSegment3D;
    class CDCAxialSegmentPair;
    class CDCTrack;

    /// Class serving as a storage of observed drift circles to present to the Riemann fitter
    class CDCObservations2D {

    public:
      /**
       *  Constructor taking the flag if the reconstructed positon of the hits should be used when they are available
       *  The default is to use the wire position and the drift length signed by the right left passage hypotheses.
       */
      explicit CDCObservations2D(EFitPos fitPos = EFitPos::c_RLDriftCircle,
                                 EFitVariance fitVariance = EFitVariance::c_Proper)
        : m_fitPos(fitPos)
        , m_fitVariance(fitVariance)
      {
      }

    public:
      /**
       *  Matrix type used to wrap the raw memory chunk of values
       *  generated from the various hit types for structured vectorized access.
       */
      using EigenObservationMatrix = Eigen::Map< Eigen::Matrix< double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor > >;

      /**
       *  Gets the pseudo variance.
       *  The pseudo drift length variance is a measure that tries to incorporate the drift length
       *  into the fit to drift circles where the right left passage ambiguity could not be resolved.
       *  In such situations only the position of an hit can be used, however if only the position
       *  that can be determined is the wire position the information of the drift length
       *  should at least weaken the gravity of that wire in relation to the fitted trajectory.
       *  The pseudo variance is therefore the square of the drift length itself (square for unit conformity)
       *  plus its reference drift length variance.
       */
      static double getPseudoDriftLengthVariance(double driftLength,
                                                 double driftLengthVariance)
      {
        return driftLength * driftLength + driftLengthVariance;
      }

      /// Calculate the pseudo variance from the drift length and its variance.
      static double getPseudoDriftLengthVariance(const CDCWireHit& wireHit);

      /// Returns the number of observations stored
      std::size_t size() const
      {
        return m_observations.size() / 4;
      }

      /// Returns true if there are no observations stored.
      bool empty() const
      {
        return m_observations.empty();
      }

      /// Removes all observations stored
      void clear()
      {
        m_observations.clear();
      }

      /// Reserves enough space for nObservations
      void reserve(std::size_t nObservations)
      {
        m_observations.reserve(nObservations * 4);
      }

      /// Getter for the x value of the observation at the given index.
      double getX(int iObservation) const
      {
        return m_observations[iObservation * 4];
      }

      /// Getter for the y value of the observation at the given index.
      double getY(int iObservation) const
      {
        return m_observations[iObservation * 4 + 1];
      }

      /// Getter for the signed drift radius of the observation at the given index.
      double getDriftLength(int iObservation) const
      {
        return m_observations[iObservation * 4 + 2];
      }

      /// Getter for the weight / inverse variance of the observation at the given index.
      double getWeight(int iObservation) const
      {
        return m_observations[iObservation * 4 + 3];
      }

      /**
       *  Appends the observed position.
       *  @note Observations are skipped, if one of the given variables is NAN.
       *  @param x            X coordinate of the center of the observed circle.
       *  @param y            Y coordinate of the center of the observed circle.
       *  @param signedRadius The radius of the observed circle signed with right left passage hypotheses.
       *                      Defaults to 0.
       *  @param weight       The relative weight of the observation.
       *                      In order to generate a unit less chi^2 measure the weight should be
       *                      chosen as the inverse variance of the drift length. Defaults to 1.
       *  @return             Number of observations added. One if the observation was added.
       *                      Zero if one of the given variables is NAN.
       */
      std::size_t fill(double x, double y, double signedRadius = 0.0, double weight = 1.0);

      /**
       *  Appends the observed position.
       *  @note Observations are skipped, if one of the given variables is NAN.
       *  @param pos2D        X, Y coordinate of the center of the observed circle.
       *  @param signedRadius The radius of the observed circle signed with right left passage hypotheses.
       *                      Defaults to 0.
       *  @param weight       The relative weight of the observation.
       *                      In order to generate a unit less chi^2 measure the weight should be
       *                      chosen as the inverse variance of the drift length. Defaults to 1.
       *  @return             Number of observations added. One if the observation was added.
       *                      Zero if one of the given variables is NAN.
       */
      std::size_t fill(const Vector2D& pos2D, double signedRadius = 0.0, double weight = 1.0);

      /**
       *  Appends the hit circle at wire reference position without a right left passage hypotheses.
       *  @note Observations are skipped, if one of the contained variables is NAN.
       *  @note The left right passage information is always set to ERightLeft::c_Right,
       *  since on specific assumption can be made from the wire hit alone.
       *  @param wireHit      Hit information to be appended as observation.
       *                      XY position, drift length and inverse variance are taken at the wire reference position.
       *  @param rlInfo       Right left passage information with which the drift length should be signed.
       *  @return             Number of observations added. One if the observation was added.
       *                      Zero if one of the given variables is NAN.
       */
      std::size_t append(const CDCWireHit& wireHit, ERightLeft rlInfo = ERightLeft::c_Unknown);

      /**
       *  Appends the position information of the given wire hit to the
       *  stored obseravations. Obtionally includes the drift length with
       *  the given right left orientation

       *  @param wireHit      Hit information to be appended as observation.
       *  @param rlInfo       Right left passage information with which the drift length should be signed.
       *  @return             Number of observations added. One if the observation was added.
       *                      Zero if one of the given variables is NAN.
       */
      std::size_t append(const CDCWireHit* wireHit, ERightLeft rlInfo = ERightLeft::c_Unknown);

      /**
       *  Appends the hit circle at wire reference position with a right left passage hypotheses.
       *  @note Observations are skipped, if one of the contained variables is NAN.
       *  @note The left right passage information is always set to
       *        the right left passage hypotheses of the give hit.
       *  @param rlWireHit    Hit information to be appended as observation.
       *                      XY position, signed drift length and inverse variance are taken at the wire reference position.
       *  @return             Number of observations added. One if the observation was added.
       *                      Zero if one of the given variables is NAN.
       */
      std::size_t append(const CDCRLWireHit& rlWireHit);

      /// Appends the two observed position
      std::size_t append(const CDCRLWireHitPair& rlWireHitPair);

      /// Appends the three observed position
      std::size_t append(const CDCRLWireHitTriple& rlWireHitTriple);

      /// Appends the three observed position
      std::size_t append(const CDCFacet& facet);

      /// Appends the hit using the reconstructed position if useRecoPos indicates it
      std::size_t append(const CDCRecoHit2D& recoHit2D);

      /// Appends the observed position
      std::size_t append(const CDCRecoHit3D& recoHit3D);

      /**
       *  Appends all reconstructed hits from the two dimensional segment.
       *  @return  Number of added hits
       */
      std::size_t appendRange(const CDCRecoSegment2D& recoSegment2D);

      /**
       *  Appends all reconstructed hits from the three dimensional segment.
       *  @return  Number of added hits
       */
      std::size_t appendRange(const CDCRecoSegment3D& recoSegment3D);

      /**
       *  Appends all reconstructed hits from the two axial segments,
       *  @return  Number of added hits
       */
      std::size_t appendRange(const CDCAxialSegmentPair& axialSegmentPair);

      /**
       *  Appends all the reference wire positions.
       *  @note For cross check to legendre finder.
       *  @return  Number of added hits
       */
      std::size_t appendRange(const std::vector<const CDCWire*>& wires);

      /**
       *  Appends all the wire hit reference positions with the pseudo variance.
       *  @note For cross check to legendre finder.
       *  @return  Number of added hits
       */
      std::size_t appendRange(const CDCWireHitSegment& wireHits);

      /// Append all hits from a generic range.
      template<class ARange>
      std::size_t appendRange(const ARange& range)
      {
        std::size_t nAppendedHits = 0;
        using std::begin;
        using std::end;
        for (const auto& item : range) {
          nAppendedHits += append(item);
        }
        return nAppendedHits;
      }

      /// Get the postion of the first observation.
      Vector2D getFrontPos2D() const
      {
        return empty() ? Vector2D() : Vector2D(getX(0), getY(0));
      }

      /// Get the postion of the first observation.
      Vector2D getBackPos2D() const
      {
        return empty() ? Vector2D() : Vector2D(getX(size() - 1), getY(size() - 1));
      }

      /**
       *  Calculate the total transvers travel distance traversed by these observations comparing
       *  the travel distance of first and last position.
       */
      double getTotalPerpS(const CDCTrajectory2D& trajectory2D) const
      {
        return trajectory2D.calcArcLength2DBetween(getFrontPos2D(), getBackPos2D());
      }

      /**
       *  Checks if the last position of these observations lies at greater travel distance than the
       *  first.
       */
      bool isForwardTrajectory(const CDCTrajectory2D& trajectory2D) const
      {
        return getTotalPerpS(trajectory2D) > 0.0;
      }

      /**
       *  Checks if the last observation in the vector lies at greater or lower travel distance than the last observation.
       *  @retval     EForwardBackward::c_Forward if the last observation lies behind the first.
       *  @retval     EForwardBackward::c_Backward if the last observation lies before the first.
       */
      EForwardBackward isCoaligned(const CDCTrajectory2D& trajectory2D) const
      {
        return static_cast<EForwardBackward>(sign(getTotalPerpS(trajectory2D)));
      }

      /// Extracts the observation center that is at the index in the middle.
      Vector2D getCentralPoint() const;

      /// Moves all observations passively such that the given vector becomes to origin of the new coordinate system
      void passiveMoveBy(const Vector2D& origin);

      /// Picks one observation as a reference point and transform all observations to that new origin
      Vector2D centralize();

      /// Returns the number of observations having a drift radius radius
      std::size_t getNObservationsWithDriftRadius() const;

      /**
       *  Returns the observations structured as an Eigen matrix
       *  This returns a reference to the stored observations.
       *  @note      Operations may alter the content of the underlying memory and
       *             render it useless for subceeding calculations.
       */
      EigenObservationMatrix getObservationMatrix();

      /**
       *  Constructs a symmetric matrix of weighted sums of x, y, r^2 and drift lengts as relevant for circle fits.
       *
       *  Cumulates weights, x positions, y positions, quadratic cylindrical radii and signed drift lengths and products thereof
       *  @returns symmetric matrix s with the following:
       *  * \f$ s_{00} = \sum w \f$
       *  * \f$ s_{01} = \sum x * w \f$
       *  * \f$ s_{02} = \sum y * w \f$
       *  * \f$ s_{03} = \sum (r^2 - l^2) * w \f$
       *  * \f$ s_{04} = \sum l * w \f$
       *
       *  * \f$ s_{11} = \sum x * x * w \f$
       *  * \f$ s_{12} = \sum x * y * w \f$
       *  * \f$ s_{13} = \sum x * (r^2 - l^2) * w \f$
       *  * \f$ s_{14} = \sum x * l * w \f$
       *
       *  * \f$ s_{22} = \sum y * y * w \f$
       *  * \f$ s_{23} = \sum y * (r^2 - l^2) * w \f$
       *  * \f$ s_{24} = \sum y * l * w \f$
       *
       *  * \f$ s_{33} = \sum (r^2 - l^2) * (r^2 - l^2) * w \f$
       *  * \f$ s_{34} = \sum (r^2 - l^2) * l * w \f$

       *  * \f$ s_{44} = \sum l * l * w \f$
       *  * + symmetric.
       */
      Eigen::Matrix<double, 5, 5> getWXYRLSumMatrix();

      /**
       *  Constructs a symmetric matrix of weighted sums of x, y and drift lengts as relevant for line fits.
       *
       *  Cumulates weights, x positions, y positions and signed drift legnths and products thereof
       *  @returns symmetric matrix s with the following:
       *  * \f$ s_{00} = \sum w \f$
       *  * \f$ s_{01} = \sum x * w \f$
       *  * \f$ s_{02} = \sum y * w \f$
       *  * \f$ s_{03} = \sum l * w \f$
       *
       *  * \f$ s_{11} = \sum x * x * w \f$
       *  * \f$ s_{12} = \sum x * y * w \f$
       *  * \f$ s_{13} = \sum x * l * w \f$
       *
       *  * \f$ s_{22} = \sum y * y * w \f$
       *  * \f$ s_{23} = \sum y * l * w \f$
       *
       *  * \f$ s_{33} = \sum (r^2 - l^2) * l * w \f$
       *  * + symmetric
       */
      Eigen::Matrix<double, 4, 4> getWXYLSumMatrix();

      /**
       *  Constructs a symmetric matrix of weighted sums of x, y, r^2 as relevant for circle fits.
       *
       *  Cumulates weights, x positions, y positions, quadratic cylindrical radii and products thereof
       *  @returns symmetric matrix s with the following:
       *  * \f$ s_{00} = \sum w \f$
       *  * \f$ s_{01} = \sum x * w \f$
       *  * \f$ s_{02} = \sum y * w \f$
       *  * \f$ s_{03} = \sum (r^2 - l^2) * w \f$
       *
       *  * \f$ s_{11} = \sum x * x * w \f$
       *  * \f$ s_{12} = \sum x * y * w \f$
       *  * \f$ s_{13} = \sum x * (r^2 - l^2) * w \f$
       *
       *  * \f$ s_{22} = \sum y * y * w \f$
       *  * \f$ s_{23} = \sum y * (r^2 - l^2) * w \f$
       *  * \f$ s_{24} = \sum y * l * w \f$
       *
       *  * \f$ s_{33} = \sum (r^2 - l^2) * (r^2 - l^2) * w \f$
       *  * + symmetric.
       */
      Eigen::Matrix<double, 4, 4> getWXYRSumMatrix();


      /**
       *  Constructs a symmetric matrix of weighted sums of x, y as relevant for line fits.
       *
       *  Cumulates weights, x positions, y positions and products thereof
       *  @returns symmetric matrix s with the following:
       *  * \f$ s_{00} = \sum w \f$
       *  * \f$ s_{01} = \sum x * w \f$
       *  * \f$ s_{02} = \sum y * w \f$
       *
       *  * \f$ s_{11} = \sum x * x * w \f$
       *  * \f$ s_{12} = \sum x * y * w \f$
       *
       *  * \f$ s_{22} = \sum y * y * w \f$
       *  * + symmetric.
       */
      Eigen::Matrix<double, 3, 3> getWXYSumMatrix();

    public:
      /// Getter for the indicator that the reconstructed position should be favoured.
      EFitPos getFitPos() const
      {
        return m_fitPos;
      }

      /// Setter for the indicator that the reconstructed position should be favoured.
      void setFitPos(EFitPos fitPos)
      {
        m_fitPos = fitPos;
      }

      /// Setter for the indicator that the drift variance should be used.
      void setFitVariance(EFitVariance fitVariance)
      {
        m_fitVariance = fitVariance;
      }

    private:
      /**
       *  Memory for the individual observations.
       *  Arrangement of values is x,y, drift raduis, weight, x, y, .....
       */
      std::vector<double> m_observations;

      /**
       *  Indicator which positional information should preferably be extracted
       *  from hits in calls to append.
       *  Meaning of the constants detailed in EFitPos.
       */
      EFitPos m_fitPos;

      /**
       *  Indicator which variance information should preferably be extracted from
       *  hits in calls to append.
       *  Meaning of the constants detailed in EFitVariance.
       */
      EFitVariance m_fitVariance;

    };

  }
}
