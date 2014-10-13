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

#include <tracking/cdcLocalTracking/rootification/SwitchableRootificationBase.h>
#include <tracking/cdcLocalTracking/typedefs/BasicTypes.h>

#include <tracking/cdcLocalTracking/eventdata/entities/CDCRecoHit3D.h>
#include <tracking/cdcLocalTracking/eventdata/collections/CDCRecoHit3DVector.h>

#include <tracking/cdcLocalTracking/eventdata/trajectories/CDCTrajectories.h>

#include <tracking/cdcLocalTracking/eventdata/segments/CDCSegments.h>

#include "genfit/TrackCand.h"

namespace Belle2 {
  namespace CDCLocalTracking {

    /// Class representing a sequence of three dimensional reconstructed hits
    class CDCTrack : public CDCRecoHit3DVector {
    public:

      /// Default constructor for ROOT compatibility.
      CDCTrack() : m_fbInfo(UNKNOWN) {;}

      /// Empty destructor
      ~CDCTrack() {;}



      /// Copies the hit and trajectory content of this track to the Genfit track candidate
      void fillInto(genfit::TrackCand& trackCand) const;



      /// Getter for the first reconstructed hit in the track. Does not account for the forward backward info.
      const Belle2::CDCLocalTracking::CDCRecoHit3D& getStartRecoHit3D() const
      { return front(); }

      /// Getter for the last reconstructed hit in the track. Does not account for the forward backward info.
      const Belle2::CDCLocalTracking::CDCRecoHit3D& getEndRecoHit3D() const
      { return back(); }



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
      Vector3D getStartFitPos3D() const
      { return getStartTrajectory3D().getSupport(); }

      /// Getter for the end fitted position of track. Does not account for the forward backward info.
      Vector3D getEndFitPos3D() const
      { return getEndTrajectory3D().getSupport(); }

      /// Getter for the momentum at the start position. Does not account for the forward backward info.
      Vector3D getStartFitMom3D() const
      { return getStartTrajectory3D().getUnitMom3DAtSupport(); }

      /// Getter for the momentum at the end position. Does not account for the forward backward info.
      Vector3D getEndFitMom3D() const
      { return getEndTrajectory3D().getUnitMom3DAtSupport(); }



      /// Getter for the charge sign. Does not account for the forward backward info.
      SignType getStartChargeSign() const
      { return getStartTrajectory3D().getChargeSign(); }

      /// Getter for the charge sign. Does not account for the forward backward info.
      SignType getEndChargeSign() const
      { return getEndTrajectory3D().getChargeSign(); }



      /// Setter for the two dimensional trajectory. The trajectory should start at the start of the track and follow its direction.
      void setStartTrajectory3D(const CDCTrajectory2D& startTrajectory2D,
                                const CDCTrajectorySZ& startTrajectorySZ)
      { m_startTrajectory3D = CDCTrajectory3D(startTrajectory2D, startTrajectorySZ); }

      /// Setter for the three dimensional trajectory. The trajectory should start at the END of the track and follow it in the reverse direction.
      void setEndTrajectory3D(const CDCTrajectory2D& endTrajectory2D,
                              const CDCTrajectorySZ& endTrajectorySZ)
      { m_endTrajectory3D = CDCTrajectory3D(endTrajectory2D, endTrajectorySZ); }



      /// Setter for the two dimensional trajectory. The trajectory should start at the start of the track and follow its direction.
      void setStartTrajectory3D(const CDCTrajectory3D& startTrajectory3D)
      { m_startTrajectory3D = startTrajectory3D; }

      /// Setter for the three dimensional trajectory. The trajectory should start at the END of the track and follow it in the reverse direction.
      void setEndTrajectory3D(const CDCTrajectory3D& endTrajectory3D)
      { m_endTrajectory3D = endTrajectory3D; }



      /// Getter for the two dimensional trajectory. The trajectory should start at the start of the track and follow its direction.
      const CDCTrajectory3D& getStartTrajectory3D() const
      { return m_startTrajectory3D; }

      /// Getter for the three dimensional trajectory. The trajectory should start at the END of the track and follow it in the reverse direction.
      const CDCTrajectory3D& getEndTrajectory3D() const
      { return m_endTrajectory3D; }



      /// Getter for the forward backward indicator variable
      ForwardBackwardInfo getFBInfo() const
      { return m_fbInfo; }

      /// Setter for the forward backward indicator variable
      void setFBInfo(const ForwardBackwardInfo& fbInfo)
      {  m_fbInfo = fbInfo; }

    private:
      CDCTrajectory3D m_startTrajectory3D; ///< Memory for the three dimensional trajectory at the start of the track
      CDCTrajectory3D m_endTrajectory3D; ///< Memory for the three dimensional trajectory at the end of the track

      ForwardBackwardInfo m_fbInfo; ///< Memory for the forward backward indicator

      /// ROOT Macro to make CDCTrack a ROOT class.
      CDCLOCALTRACKING_SwitchableClassDef(CDCTrack, 1);

    }; //class

  } // namespace CDCLocalTracking
} // namespace Belle2
#endif // CDCTRACK_H
