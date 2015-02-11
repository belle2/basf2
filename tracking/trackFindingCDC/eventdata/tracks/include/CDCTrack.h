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

#include <tracking/trackFindingCDC/rootification/SwitchableRootificationBase.h>
#include <tracking/trackFindingCDC/typedefs/BasicTypes.h>

#include <tracking/trackFindingCDC/eventdata/entities/CDCRecoHit3D.h>
#include <tracking/trackFindingCDC/eventdata/collections/CDCRecoHit3DVector.h>

#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectories.h>

#include <tracking/trackFindingCDC/eventdata/segments/CDCSegments.h>

#include "genfit/TrackCand.h"

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Class representing a sequence of three dimensional reconstructed hits
    class CDCTrack : public CDCRecoHit3DVector {
    public:

      /// Default constructor for ROOT compatibility.
      CDCTrack() {;}

      /// Constructor from a two dimensional segment filling the thrid dimension with 0 values.
      explicit CDCTrack(const CDCRecoSegment2D& segment);

      /// Empty destructor
      ~CDCTrack() {;}

      /// Copies the hit and trajectory content of this track to the Genfit track candidate
      bool fillInto(genfit::TrackCand& trackCand) const;

      /// Getter for the first reconstructed hit in the track.
      const Belle2::TrackFindingCDC::CDCRecoHit3D& getStartRecoHit3D() const
      { return front(); }

      /// Getter for the last reconstructed hit in the track.
      const Belle2::TrackFindingCDC::CDCRecoHit3D& getEndRecoHit3D() const
      { return back(); }



      /// Getter for the superlayer id the track starts from.
      ILayerType getStartISuperLayer() const
      { return getStartRecoHit3D().getISuperLayer(); }

      /// Getter for the superlayer id the track ends in.
      ILayerType getEndISuperLayer() const
      { return getEndRecoHit3D().getISuperLayer(); }



      /// Getter for the position of the first reconstructed hit.
      const Vector3D& getStartRecoPos3D() const
      { return getStartRecoHit3D().getRecoPos3D(); }

      /// Getter for the position of the last reconstructed hit.
      const Vector3D& getEndRecoPos3D() const
      { return getEndRecoHit3D().getRecoPos3D(); }



      /// Getter for the start fitted position of track.
      Vector3D getStartFitPos3D() const
      { return getStartTrajectory3D().getSupport(); }

      /// Getter for the end fitted position of track.
      Vector3D getEndFitPos3D() const
      { return getEndTrajectory3D().getSupport(); }

      /// Getter for the momentum at the start position.
      Vector3D getStartFitMom3D() const
      { return getStartTrajectory3D().getUnitMom3DAtSupport(); }

      /// Getter for the momentum at the end position.
      Vector3D getEndFitMom3D() const
      { return getEndTrajectory3D().getUnitMom3DAtSupport(); }



      /// Getter for the charge sign.
      SignType getStartChargeSign() const
      { return getStartTrajectory3D().getChargeSign(); }

      /// Getter for the charge sign.
      SignType getEndChargeSign() const
      { return getEndTrajectory3D().getChargeSign(); }



      /// Setter for the two dimensional trajectory. The trajectory should start at the start of the track and follow its direction.
      void setStartTrajectory3D(const CDCTrajectory3D& startTrajectory3D)
      { m_startTrajectory3D = startTrajectory3D; }

      /// Setter for the three dimensional trajectory. The trajectory should start at the END of the track and *follow* its direction.
      void setEndTrajectory3D(const CDCTrajectory3D& endTrajectory3D)
      { m_endTrajectory3D = endTrajectory3D; }



      /// Getter for the two dimensional trajectory. The trajectory should start at the start of the track and follow its direction.
      const CDCTrajectory3D& getStartTrajectory3D() const
      { return m_startTrajectory3D; }

      /// Getter for the three dimensional trajectory. The trajectory should start at the END of the track and follow its direction.
      const CDCTrajectory3D& getEndTrajectory3D() const
      { return m_endTrajectory3D; }



      /// Reverse the track inplace
      void reverse();

      /// Return a reversed copy of the track.
      CDCTrack reversed() const;

    private:
      /// Memory for the three dimensional trajectory at the start of the track
      CDCTrajectory3D m_startTrajectory3D;

      /// Memory for the three dimensional trajectory at the end of the track
      CDCTrajectory3D m_endTrajectory3D;

      /// ROOT Macro to make CDCTrack a ROOT class.
      TRACKFINDINGCDC_SwitchableClassDef(CDCTrack, 1);

    }; //class

  } // namespace TrackFindingCDC
} // namespace Belle2
#endif // CDCTRACK_H
