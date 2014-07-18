/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef CDCAXIALSTEREOSEGMENTPAIR_H
#define CDCAXIALSTEREOSEGMENTPAIR_H

#include <tracking/cdcLocalTracking/mockroot/MockRoot.h>
#include <tracking/cdcLocalTracking/typedefs/BasicTypes.h>
#include <tracking/cdcLocalTracking/algorithms/AutomatonCell.h>

#include <tracking/cdcLocalTracking/eventdata/entities/CDCEntities.h>
#include <tracking/cdcLocalTracking/eventdata/segments/CDCSegments.h>

namespace Belle2 {
  namespace CDCLocalTracking {

    /// Class representing a pair of one reconstructed axial segement and one stereo segment in adjacent superlayer.
    class CDCAxialStereoSegmentPair : public UsedTObject {
    public:

      /// Default constructor - for ROOT compatability.
      CDCAxialStereoSegmentPair();

      /// Constructor from two segments
      CDCAxialStereoSegmentPair(const CDCRecoSegment2D* startSegment,
                                const CDCRecoSegment2D* endSegment);

      /// Constructor from two segments and an assoziated trajectory
      CDCAxialStereoSegmentPair(const CDCRecoSegment2D* startSegment,
                                const CDCRecoSegment2D* endSegment,
                                const CDCTrajectory2D& trajectory2D);

      /// Empty destructor
      ~CDCAxialStereoSegmentPair();

      /// Equality comparision based on the pointers to the stored segments.
      bool operator==(CDCAxialStereoSegmentPair const& rhs) const
      { return getStartSegment() == rhs.getStartSegment() and getEndSegment() == rhs.getEndSegment(); }


      /// Total ordering sheme comparing the segment pointers.
      bool operator<(CDCAxialStereoSegmentPair const& rhs) const {
        return (getStartSegment() < rhs.getStartSegment()  or
                (getStartSegment() == rhs.getStartSegment() and getEndSegment() < rhs.getEndSegment()));
      }

      /// Define reconstructed segments and axial stereo segment pairs as coaligned on the start segment
      friend bool operator<(const CDCAxialStereoSegmentPair& axialStereoSegmentPair,
                            const CDCRecoSegment2D* segment)
      { return axialStereoSegmentPair.getStartSegment() < segment; }

      /// Define reconstructed segments and axial stereo segment pairs as coaligned on the start segment
      friend bool operator<(const CDCRecoSegment2D* segment,
                            const CDCAxialStereoSegmentPair& axialStereoSegmentPair)
      { return segment < axialStereoSegmentPair.getStartSegment(); }


      /// Checks if both stored segments are not nullptr. Returns true if check is succeded.
      bool checkSegmentsNonNullptr() const
      { return getStartSegment() != nullptr and getEndSegment() != nullptr; }


      /// Checks if the two segments are of different axial type.
      bool checkSegmentsStereoTypes() const
      { return getStartStereoType() xor getEndStereoType(); }

      /// Checks the references to the contained three segment for nullptrs and exactly one of them is axial and one is stereo
      bool checkSegments() const
      { return checkSegmentsNonNullptr() and checkSegmentsStereoTypes(); }



      /// Getter for the stereo type of the first segment.
      StereoType getStartStereoType() const
      { return getStartSegment() == nullptr ? INVALID_STEREOTYPE : getStartSegment()->getStereoType(); }


      /// Getter for the stereo type of the second segment.
      StereoType getEndStereoType() const
      { return getEndSegment() == nullptr ? INVALID_STEREOTYPE : getEndSegment()->getStereoType(); }



      /// Getter for the superlayer id of the start segment.
      ILayerType getStartISuperLayer() const
      { return getStartSegment() == nullptr ? INVALID_ISUPERLAYER : getStartSegment()->getISuperLayer(); }

      /// Getter for the superlayer id of the end segment.
      ILayerType getEndISuperLayer() const
      { return getEndSegment() == nullptr ? INVALID_ISUPERLAYER : getEndSegment()->getISuperLayer(); }



      /// Getter for the start segment.
      const CDCRecoSegment2D* getStartSegment() const
      { return m_startSegment; }

      /// Setter for the start segment.
      void setStartSegment(const CDCRecoSegment2D* startSegment)
      { setSegments(startSegment, getEndSegment()); }



      /// Getter for the end segment.
      const CDCRecoSegment2D* getEndSegment() const
      { return m_endSegment; }

      /// Setter for the end segment.
      void setEndSegment(const CDCRecoSegment2D* endSegment)
      { setSegments(getStartSegment(), endSegment); }



      /// Getter for the stereo segment
      const CDCAxialRecoSegment2D* getStereoSegment() const
      { return getStartStereoType() ? getStartSegment() : getEndSegment(); }

      /// Getter for the axial segment
      const CDCAxialRecoSegment2D* getAxialSegment() const
      { return getStartStereoType() ? getEndSegment() : getStartSegment(); }


      /// Setter for both segments simultaniously
      void setSegments(const CDCRecoSegment2D* startSegment, const CDCRecoSegment2D* endSegment) {
        m_startSegment = startSegment;
        m_endSegment = endSegment;
        if (not checkSegmentsNonNullptr()) B2WARNING("CDCAxialStereoSegmentPair: one segment set to nullptr");
        if (not checkSegmentsStereoTypes()) B2WARNING("CDCAxialStereoSegmentPair: segments set with same stereo type");
      }


      /// Checks if the last entity in the vector lies greater or lower travel distance than the last entity.
      /** Returns:
       *  * FORWARD if the last entity lies behind the first.
       *  * BACKWARD if the last entity lies before the first.
       */
      ForwardBackwardInfo isCoaligned(const CDCTrajectory2D& trajectory2D) const {
        ForwardBackwardInfo startIsCoaligned = getStartSegment()->isCoaligned(trajectory2D);
        ForwardBackwardInfo endIsCoaligned = getEndSegment()->isCoaligned(trajectory2D);

        if (startIsCoaligned == FORWARD and endIsCoaligned == FORWARD) return FORWARD;
        else if (startIsCoaligned == BACKWARD and endIsCoaligned == BACKWARD) return BACKWARD;
        else return INVALID_INFO;
      }



      /// Getter for the two dimensional trajectory.
      CDCTrajectory2D& getTrajectory2D() const
      { return m_trajectory2D; }

      /// Setter for the two dimensional trajectory.
      void setTrajectory2D(const CDCTrajectory2D& trajectory2D) const
      { m_trajectory2D =  trajectory2D; }

      /// Getter for the sz trajectory.
      CDCTrajectorySZ& getTrajectorySZ() const
      { return m_trajectorySZ; }

      /// Setter for the sz trajectory.
      void setTrajectorySZ(const CDCTrajectorySZ& trajectorySZ) const
      { m_trajectorySZ = trajectorySZ; }

      /// Invalides the currently stored trajectory information.
      void clearTrajectories() const {
        getTrajectory2D().clear();
        getTrajectorySZ().clear();
      }



      /// Sets the do not use flag of the segment triple's automaton cell and of the three contained segments
      void setDoNotUse() const {
        getAutomatonCell().setDoNotUseFlag();
        forwardDoNotUse();
      }

      /// Sets the do not use flag of the three contained segments
      void forwardDoNotUse() const {
        getStartSegment()->getAutomatonCell().setDoNotUseFlag();
        getEndSegment()->getAutomatonCell().setDoNotUseFlag();
      }

      /// If one of the contained segments is marked as do not use this segment triple is set be not usable as well
      void receiveDoNotUse() const {
        if (getStartSegment()->getAutomatonCell().hasDoNotUseFlag() or
            getEndSegment()->getAutomatonCell().hasDoNotUseFlag()) {
          getAutomatonCell().setDoNotUseFlag();
        }
      }

      /// Getter for the automaton cell.
      AutomatonCell& getAutomatonCell()
      { return m_automatonCell; }

      /// Constant getter for the automaton cell.
      const AutomatonCell& getAutomatonCell() const
      { return m_automatonCell; }



    private:
      const CDCRecoSegment2D* m_startSegment; ///< Reference to the start segment
      const CDCRecoSegment2D* m_endSegment; ///< Reference to the end segment

      mutable CDCTrajectory2D m_trajectory2D; ///< Reference to the common two dimensional trajectory
      mutable CDCTrajectorySZ m_trajectorySZ; ///< Reference to the common sz trajectory
      mutable AutomatonCell m_automatonCell; ///< Automaton cell assoziated with the pair of segments



      /** ROOT Macro to make CDCAxialStereoSegmentPair a ROOT class.*/
      ClassDefInCDCLocalTracking(CDCAxialStereoSegmentPair, 1);

    }; //end class CDCAxialStereoSegmentPair

  } // end namespace CDCLocalTracking
} // end namespace Belle2
#endif // CDCAXIALSTEREOSEGMENTPAIR_H
