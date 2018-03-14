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

#include <tracking/trackFindingCDC/fitting/EFitVariance.h>

#include <vector>

namespace Belle2 {
  namespace TrackFindingCDC {
    class Vector2D;
    class CDCRecoHit3D;
    class CDCSegment3D;
    class CDCTrack;

    /// Class serving as a storage of observed sz positions to present to the sz line fitters.
    class CDCSZObservations {

    public:
      /**
       *  Constructor taking in prefered variance quanity be taken from the various hit objects if present.
       *
       *  @param fitVariance Variance information to be used
       *                     Currently only c_Unit and c_Proper are supported.
       *  @param onlyStereo  Switch to only use the information of stereo hits.
       */
      explicit CDCSZObservations(EFitVariance fitVariance = EFitVariance::c_Proper,
                                 bool onlyStereo = false)
        : m_fitVariance(fitVariance)
        , m_onlyStereo(onlyStereo)
      {
      }

    public:
      /// Returns the number of observations stored
      std::size_t size() const
      {
        return m_szObservations.size() / 3;
      }

      /// Return the pointer to the number buffer
      double* data()
      {
        return m_szObservations.data();
      }

      /// Returns true if there are no observations stored.
      bool empty() const
      {
        return m_szObservations.empty();
      }

      /// Removes all observations stored
      void clear()
      {
        m_szObservations.clear();
      }

      /// Reserves enough space for nObservations
      void reserve(std::size_t nObservations)
      {
        m_szObservations.reserve(nObservations * 3);
      }

      /// Getter for the arc length value of the observation at the given index.
      double getS(int iObservation) const
      {
        return m_szObservations[iObservation * 3];
      }

      /// Getter for the z value of the observation at the given index.
      double getZ(int iObservation) const
      {
        return m_szObservations[iObservation * 3 + 1];
      }

      /// Getter for the weight / inverse variance of the observation at the given index.
      double getWeight(int iObservation) const
      {
        return m_szObservations[iObservation * 3 + 2];
      }

      /**
       *  Appends the observed position.
       *  @note Observations are skipped, if one of the given variables is NAN.
       *  @param s            S coordinate of the center of the observed position.
       *  @param z            Z coordinate of the center of the observed position.
       *  @param weight       The relative weight of the observation.
       *                      In order to generate a unit less chi^2 measure the weight should be
       *                      chosen as the inverse variance of the drift length. Defaults to 1.
       *  @return             Number of observations added. One if the observation was added.
       *                      Zero if one of the given variables is NAN.
       */
      std::size_t fill(double s, double z, double weight = 1.0);

      /// Appends the observed position
      std::size_t append(const CDCRecoHit3D& recoHit3D);

      /**
       *  Appends all reconstructed hits from the three dimensional track.
       *  @return  Number of added hits
       */
      std::size_t appendRange(const std::vector<CDCRecoHit3D>& recoHit3Ds);

      /**
       *  Appends all reconstructed hits from the three dimensional segment.
       *  @return  Number of added hits
       */
      std::size_t appendRange(const CDCSegment3D& segment3D);

      /**
       *  Appends all reconstructed hits from the three dimensional track.
       *  @return  Number of added hits
       */
      std::size_t appendRange(const CDCTrack& track);

      /// Extracts the observation center that is at the index in the middle.
      Vector2D getCentralPoint() const;

      /// Moves all observations passively such that the given vector becomes to origin of the new coordinate system
      void passiveMoveBy(const Vector2D& origin);

      /// Picks one observation as a reference point and transform all observations to that new origin
      Vector2D centralize();

    public:
      /// Setter for the indicator that the drift variance should be used.
      void setFitVariance(EFitVariance fitVariance)
      {
        m_fitVariance = fitVariance;
      }

    private:
      /**
       *  Memory for the individual observations.
       *  Arrangement of values is s, z, weight, s, z, .....
       */
      std::vector<double> m_szObservations;

      /**
       *  Indicator which variance information should preferably be extracted from
       *  hits in calls to append.
       *  Meaning of the constants detailed in EFitVariance.
       */
      EFitVariance m_fitVariance;

      /// Switch to only use information from stereo hits.
      bool m_onlyStereo;
    };
  }
}
