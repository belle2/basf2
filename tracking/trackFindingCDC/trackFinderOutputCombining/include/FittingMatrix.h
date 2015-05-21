/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <string>
#include <vector>

#include <framework/logging/Logger.h>
#include <framework/datastore/StoreArray.h>
#include <Eigen/Dense>

namespace Belle2 {

  namespace TrackFindingCDC {
    class CDCRecoSegment2D;
    class CDCTrack;

    class FittingMatrix {

    public:
      typedef unsigned int SegmentCounter;
      typedef unsigned int TrackCounter;

      FittingMatrix() : m_param_maximum_theta(0), m_param_maximum_distance_z(0),
        m_param_minimal_theta_difference(0),
        m_param_minimal_z_difference(0), m_param_minimal_chi2(0),
        m_param_minimal_chi2_stereo(0) {}

      enum SegmentStatus {
        ABOVE_TRACK, BENEATH_TRACK, IN_TRACK, MIX_WITH_TRACK, NAN_IN_CALCULATION
      };

      /**
       * Set the given cell to zero in all matrices.
       */
      void resetEntry(SegmentCounter segmentCounter, TrackCounter trackCounter)
      {
        m_fittingMatrix(segmentCounter, trackCounter) = 0;
        m_zMatrix(segmentCounter, trackCounter) = 0;
        m_zDistMatrix(segmentCounter, trackCounter) = 0;
      }

      /**
       * Set the given segment row to zero in all matrices.
       */
      void resetSegment(SegmentCounter counterSegment)
      {
        for (TrackCounter counterTracks = 0; counterTracks < m_fittingMatrix.cols(); counterTracks++) {
          resetEntry(counterSegment, counterTracks);
        }
      }

      /**
       * Calculate all matrices with the fitting procedure. Must be called before accessing the matrices.
       */
      void calculateMatrices(const std::vector<CDCRecoSegment2D>& recoSegments,
                             const std::vector<CDCTrack>& resultTrackCands);

      /**
       * Check if the given cell is good to work with. Checks for low theta-values and low z-distances and high chi2.
       */
      inline bool isGoodEntry(SegmentCounter counterSegments, TrackCounter counterTracks)
      {
        return (m_fittingMatrix(counterSegments, counterTracks) > m_param_minimal_chi2_stereo and
                std::abs(m_zMatrix(counterSegments, counterTracks)) < m_param_maximum_theta and
                std::abs(m_zDistMatrix(counterSegments, counterTracks)) < m_param_maximum_distance_z);
      }


      /**
       * Checks if a given cell has parameters in the range of the given reference parameters.
       */
      inline bool segmentHasTheSameParameters(SegmentCounter segmentCounter, SegmentCounter referenceSegment, TrackCounter trackCounter)
      {
        double zReference = getZ(referenceSegment, trackCounter);
        double zDistReference = getDistZ(referenceSegment, trackCounter);

        return isGoodEntry(segmentCounter, trackCounter)
               and std::abs(m_zMatrix(segmentCounter, trackCounter) - zReference)
               < m_param_minimal_theta_difference / 2
               and std::abs(m_zDistMatrix(segmentCounter, trackCounter) - zDistReference)
               < m_param_minimal_z_difference;
      }

      /**
       * Add a given segment to a track candidate and reset all entries of this segment.
       */
      void addSegmentToResultTrack(FittingMatrix::SegmentCounter counterSegment, FittingMatrix::TrackCounter counterTrack,
                                   const std::vector<CDCRecoSegment2D>& recoSegments, std::vector<CDCTrack>& resultTrackCands);

      bool isSegmentUsed(SegmentCounter segmentCounter)
      {
        return m_segmentIsUsed[segmentCounter];
      }

      /**
       * Debug print
       */
      void print()
      {
        B2DEBUG(100, "Fitting Matrix:\n" << m_fittingMatrix)
        B2DEBUG(100, "z Matrix:\n" << m_zMatrix)
        B2DEBUG(100, "z Dist Matrix:\n" << m_zDistMatrix)
      }

      /**
       * Return the chi2 of a given cell.
       */
      double getChi2(SegmentCounter segmentCounter, TrackCounter trackCounter) const
      {
        return m_fittingMatrix(segmentCounter, trackCounter);
      }

      /**
       * Return the theta of a given cell (only for stereo).
       */
      double getZ(SegmentCounter segmentCounter, TrackCounter trackCounter) const
      {
        return m_zMatrix(segmentCounter, trackCounter);
      }

      /**
       * Return the z distance of a given cell (only for stereo).
       */
      double getDistZ(SegmentCounter segmentCounter, TrackCounter trackCounter) const
      {
        return m_zDistMatrix(segmentCounter, trackCounter);
      }

      /**
       * Parameter
       */
      double& getParamMaximumDistanceZ()
      {
        return m_param_maximum_distance_z;
      }

      /**
       * Parameter
       */
      double& getParamMaximumTheta()
      {
        return m_param_maximum_theta;
      }

      /**
       * Parameter
       */
      double& getParamMinimalChi2()
      {
        return m_param_minimal_chi2;
      }

      /**
       * Parameter
       */
      double& getParamMinimalChi2Stereo()
      {
        return m_param_minimal_chi2_stereo;
      }

      /**
       * Parameter
       */
      double& getParamMinimalThetaDifference()
      {
        return m_param_minimal_theta_difference;
      }

      /**
       * Parameter
       */
      double& getParamMinimalZDifference()
      {
        return m_param_minimal_z_difference;
      }

      /**
       * Fill the hits into a given track candidate.
       */
      static void fillHitsInto(const CDCRecoSegment2D& recoSegment, CDCTrack& bestTrackCand);

      static SegmentStatus calculateSegmentStatus(const CDCRecoSegment2D& segment,
                                                  const CDCTrack& resultTrackCand);

    private:

      Eigen::MatrixXf
      m_fittingMatrix;  /**< The matrix with the chi2. One column corresponds to one track candiate and one row to a reco segement. */
      Eigen::MatrixXf m_zMatrix;        /**< The matrix with the theta values */
      Eigen::MatrixXf m_zDistMatrix;    /**< The matrix with the z distances */

      std::vector<bool> m_segmentIsUsed;            /**< An array with status flasg if the segment is used or not */

      double m_param_maximum_theta; /** Parameter */
      double m_param_maximum_distance_z; /** Parameter */
      double m_param_minimal_theta_difference; /** Parameter */
      double m_param_minimal_z_difference; /** Parameter */
      double m_param_minimal_chi2; /** Parameter */
      double m_param_minimal_chi2_stereo; /** Parameter */
    };
  }
}
