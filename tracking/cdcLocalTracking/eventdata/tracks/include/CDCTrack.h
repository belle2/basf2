/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef CDCTRACK_H
#define CDCTRACK_H

#include <tracking/cdcLocalTracking/mockroot/MockRoot.h>
#include <tracking/cdcLocalTracking/typedefs/BasicTypes.h>

#include <tracking/cdcLocalTracking/eventdata/entities/CDCRecoHit3D.h>
#include <tracking/cdcLocalTracking/eventdata/collections/CDCRecoHit3DVector.h>

#include <tracking/cdcLocalTracking/eventdata/segments/CDCSegments.h>

namespace Belle2 {
  namespace CDCLocalTracking {

    /// Class representing a sequence of three dimensional reconstructed hits
    class CDCTrack : public CDCRecoHit3DVector {
    public:

      /// Default constructor for ROOT compatibility.
      CDCTrack() : m_fbInfo(UNKNOWN) {;}

      /// Empty destructor
      ~CDCTrack() {;}

      /// Getter for the first reconstructed hit in the track. Does not account for the forward backward info.
      const Belle2::CDCLocalTracking::CDCRecoHit3D& getStartRecoHit3D() const { return front(); }

      /// Getter for the last reconstructed hit in the track. Does not account for the forward backward info.
      const Belle2::CDCLocalTracking::CDCRecoHit3D& getEndRecoHit3D() const { return back(); }

      /// Getter for the superlayer id the track starts from. Does not account for the forward backward info.
      ILayerType getStartISuperLayer() const
      { return getStartRecoHit3D().getISuperLayer(); }

      /// Getter for the superlayer id the track ends in. Does not account for the forward backward info.
      ILayerType getEndISuperLayer() const
      { return getEndRecoHit3D().getISuperLayer(); }

      /// Getter for the position of the first reconstructed hit. Does not account for the forward backward info.
      const Vector3D& getStartRecoPos3D() const
      { return getStartRecoHit3D().getRecoPos3D(); }

      /// Getter for the position of the last reconstructed hit. Does not account for the forward backward info.
      const Vector3D& getEndRecoPos3D() const
      { return getEndRecoHit3D().getRecoPos3D(); }


      /// Getter for the start fitted position of track. Does not account for the forward backward info.
      Vector3D getStartFitPos3D() const {
        const FloatType z = getStartTrajectorySZ().getStartZ();
        return Vector3D(getStartTrajectory2D().getStartPos2D() , z);
      }

      /// Getter for the end fitted position of track. Does not account for the forward backward info.
      Vector3D getEndFitPos3D() const {
        const FloatType z = getEndTrajectorySZ().getStartZ();
        return Vector3D(getEndTrajectory2D().getStartPos2D() , z);
      }


      /// Getter for the momentum at the start position. Does not account for the forward backward info.
      Vector3D getStartFitMom3D() const {

        Vector3D startMom(getStartTrajectory2D().getStartMom2D(), 0.0);
        const FloatType pt = startMom.polarR();
        const FloatType pz = getStartTrajectorySZ().mapPtToPz(pt);
        startMom.setZ(pz);
        return startMom;

      }

      /// Getter for the momentum at the end position. Does not account for the forward backward info.
      Vector3D getEndFitMom3D() const {
        Vector3D startMom(getEndTrajectory2D().getStartMom2D(), 0.0);
        const FloatType pt = startMom.polarR();
        const FloatType pz = getEndTrajectorySZ().mapPtToPz(pt);
        startMom.setZ(pz);
        return startMom;
      }

      /// Getter for the charge sign. Does not account for the forward backward info.
      SignType getChargeSign() const
      { return getStartTrajectory2D().getChargeSign(); }

      /// Getter for an PID estimation. Does not account for the forward backward info.
      int getPID() const {
        // dummy pid - dont know if one can make this assumption better at this point
        // Muon id 13 has negativ charge, so the correctly charged (anti)muon is
        return getChargeSign() * (-13);
      }

      /// Getter for the two dimensional trajectory. Does not account for the forward backward info.
      const CDCTrajectory2D& getStartTrajectory2D() const
      { return m_startTrajectory2D; }

      /// Setter for the two dimensional trajectory.
      void setStartTrajectory2D(const CDCTrajectory2D& startTrajectory2D)
      { m_startTrajectory2D = startTrajectory2D; }

      /// Getter for the sz trajectory at the start of the track.
      const CDCTrajectorySZ& getStartTrajectorySZ() const
      { return m_startTrajectorySZ; }

      /// Setter for the sz trajectory at the start of the track.
      void setStartTrajectorySZ(const CDCTrajectorySZ& startTrajectorySZ)
      { m_startTrajectorySZ = startTrajectorySZ; }


      /// Getter for the two dimensional trajectory. The travel distance is set relative to the last reconstructed hit
      const CDCTrajectory2D& getEndTrajectory2D() const
      { return m_endTrajectory2D; }

      /// Setter for the two dimensional trajectory. The travel distance should be set relative to the last reconstructed hit
      void setEndTrajectory2D(const CDCTrajectory2D& endTrajectory2D)
      { m_endTrajectory2D = endTrajectory2D; }

      /// Getter for the sz trajectory at the end of the track. The travel distance is set relative to the last reconstructed hit
      const CDCTrajectorySZ& getEndTrajectorySZ() const
      { return m_endTrajectorySZ; }

      /// Setter for the sz trajectory at the end of the track. The travel distance should be set relative to the last reconstructed hit
      void setEndTrajectorySZ(const CDCTrajectorySZ& endTrajectorySZ)
      { m_endTrajectorySZ = endTrajectorySZ; }


      /// Getter for the forward backward indicator variable
      ForwardBackwardInfo getFBInfo() const
      { return m_fbInfo; }

      /// Setter for the forward backward indicator variable
      void setFBInfo(const ForwardBackwardInfo& fbInfo)
      {  m_fbInfo = fbInfo; }

    private:

      //CDCRecoHit3DVector m_hits; // now inherited

      CDCTrajectory2D m_startTrajectory2D; ///< Memory for the two dimensional trajectory at the start of the track
      CDCTrajectorySZ m_startTrajectorySZ; ///< Memory for the sz trajectory at the start of the track

      CDCTrajectory2D m_endTrajectory2D; ///< Memory for the two dimensional trajectory at the end of the track
      CDCTrajectorySZ m_endTrajectorySZ; ///< Memory for the sz trajectory at the end of the track

      ForwardBackwardInfo m_fbInfo; ///< Memory for the forward backward indicator

      /// ROOT Macro to make CDCTrack a ROOT class.
      ClassDefInCDCLocalTracking(CDCTrack, 1);

    }; //class

  } // namespace CDCLocalTracking
} // namespace Belle2
#endif // CDCTRACK_H
