/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once
#ifndef EVENTDATAPLOTTER_H_
#define EVENTDATAPLOTTER_H_

#include <tracking/trackFindingCDC/eventdata/CDCEventData.h>
#include <tracking/trackFindingCDC/topology/CDCWireTopology.h>

#include <cdc/dataobjects/CDCSimHit.h>
#include <cdc/dataobjects/CDCHit.h>

#include <tracking/trackFindingCDC/display/PrimitivePlotter.h>
#include <tracking/trackFindingCDC/display/BoundingBox.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// A class that can plot event related data types.
    class EventDataPlotter {


    public:
      typedef PrimitivePlotter::AttributeMap AttributeMap;

    public:
      /// Default constructor for ROOT compatibility. Uses an SVGPrimitivePlotter as backend.
      EventDataPlotter(bool animate = false);

      /** Constructor taking the specifc PrimitivePlotter instance as backend.
       *
       *  Note that the EventDataPlotter takes ownership of the PrimitivePlotter and destroys it on its on own deconstruction.
       */
      EventDataPlotter(PrimitivePlotter* primitivePlotter, bool animate = false);

      /// Copy constructor
      EventDataPlotter(const EventDataPlotter& EventDataPlotter);

      /// Make destructor virtual to handle polymorphic deconstruction.
      virtual ~EventDataPlotter();

      /// Returns a newly created plotter instance containing all information of this.
      /** The new object is created on the heap. The ownership is to the caller who has the responsibility to destroy it.
       */
      virtual EventDataPlotter* clone();

    public:

      /** Saves the current plot stead to a file.
       *
       *  Deriving instances may should implement the approriate thing here and
       *  may return a modified string indicating the file name to which the plot as been written.
       *  It is allowed to append or change the file extension if the concret implementation PrimitivePlotter decides to do so.
       *
       *  @param fileName       fileName where the plot shall be saved
       *  @return               Potentially modifed file name where the file has actually been written to.
       */
      const std::string save(const std::string& fileName);

      /** Clears all drawed elements from the plotter.
       *
       */
      void clear();

    public:
      /// Getter for the current bounding box
      BoundingBox getBoundingBox() const;

      /// Setter for the bounding box of all drawed objects.
      void setBoundingBox(const BoundingBox& boundingBox);

      /// Getter for the canvas width in pixels.
      float getCanvasWidth() const;

      /// Getter for the canvas height in pixels.
      float getCanvasHeight() const;

      /** Setter for the canvas width in pixels.
       *  The canvas height denotes the size of the image being produced.
       *  The coordinates space that is visible in the picture is a seperate concept
       *  which is stored in the bounding box (getBoundingBox()). */
      void setCanvasWidth(const float& width);

      /** Setter for the canvas height in pixels
       *  The canvas height denotes the size of the image being produced.
       *  The coordinates space that is visible in the picture is a seperate concept
       *  which is stored in the bounding box (getBoundingBox()). */
      void setCanvasHeight(const float& height);

    public:
      /// Converts a time given in nanoseconds to a time sting of the from "%fs".
      std::string getAnimationTimeFromNanoSeconds(const float& nanoseconds)
      { return std::to_string(nanoseconds) + "s"; }

    private:
      /// Start a group in the underlying plotter with an animation uncovering the elements at the time of flight of the CDCSimHit.
      void startAnimationGroup(const Belle2::CDCSimHit& simHit);

      /// Start a group in the underlying plotter with an animation uncovering the elements at the time of flight of the related CDCSimHit.
      void startAnimationGroup(const Belle2::CDCHit* ptrHit);

    public:
      /// Marks the position of the interaction point with a filled circle.
      void drawInteractionPoint();

      /// Draw the inner wall of the CDC.
      void drawInnerCDCWall(const AttributeMap& attributeMap = AttributeMap());

      /// Draw the outer wall of the CDC.
      void drawOuterCDCWall(const AttributeMap& attributeMap = AttributeMap());

    public:
      // Drawing methods for the variuous event data objects.

      /// Draws a filled circle.
      void draw(const Belle2::TrackFindingCDC::Circle2D& circle,
                AttributeMap attributeMap = AttributeMap());

      /// Draws the CDCWire as a small circle at the reference position.
      void draw(const Belle2::TrackFindingCDC::CDCWire& wire,
                const AttributeMap& attributeMap = AttributeMap());

      /// Draws the inner and the outer bound of the super layer.
      void draw(const Belle2::TrackFindingCDC::CDCWireSuperLayer& wireSuperLayer,
                const AttributeMap& attributeMap = AttributeMap());

      /// Draws the all wires in the CDC
      void draw(const Belle2::TrackFindingCDC::CDCWireTopology& wireTopology,
                AttributeMap attributeMap = AttributeMap());

      /// Draws the CDCSimHit as a momentum arrow starting at the track position with a length proportional to its momentum.
      void draw(const Belle2::CDCSimHit& simHit,
                const AttributeMap& attributeMap = AttributeMap());

      /// Draws the CDCHit as the wire position and its drift circle at the wire reference position.
      void draw(const Belle2::CDCHit& hit,
                const AttributeMap& attributeMap = AttributeMap());

      /// Draws the CDCWireHit as the wire position and its drift circle at the wire reference position.
      void draw(const Belle2::TrackFindingCDC::CDCWireHit& wireHit,
                const AttributeMap& attributeMap = AttributeMap());

      /// Draws the CDCRecoHit2D as a drift circle at the two dimensional reference wire position and a point at the reconstructed position"""
      void draw(const Belle2::TrackFindingCDC::CDCRecoHit2D& recoHit2D,
                const AttributeMap& attributeMap = AttributeMap());

      /// Draws the CDCRecoHit3D as a drift circle at the two dimensional reference wire position and a point at the reconstructed position"""
      void draw(const Belle2::TrackFindingCDC::CDCRecoHit3D& recoHit3D,
                const AttributeMap& attributeMap = AttributeMap())
      {
        draw(recoHit3D.getRecoHit2D(), attributeMap);
      }

      /// Draws the CDCRecoHit3D as a drift circle at the two dimensional reference wire position and a point at the reconstructed position"""
      void draw(const Belle2::TrackFindingCDC::CDCTangent& tangent,
                const AttributeMap& attributeMap = AttributeMap());

      /// Draws the CDCTrajectory from the start point until it first exits the CDC.
      void draw(const CDCTrajectory2D& trajectory2D, AttributeMap attributeMap = AttributeMap());

      /// Draws all CDCWireHits of the cluster
      void draw(const CDCWireHitCluster& wireHitCluster, const AttributeMap& attributeMap = AttributeMap())
      {
        drawRange(wireHitCluster, attributeMap);
      }

      /// Draws all CDCRecoHits2D of the segment
      void draw(const CDCRecoSegment2D& recoSegment2D, const AttributeMap& attributeMap = AttributeMap())
      {
        drawRange(recoSegment2D, attributeMap);
      }

      /// Draws all CDCRecoHits3D of the segment
      void draw(const CDCRecoSegment3D& recoSegment3D, const AttributeMap& attributeMap = AttributeMap())
      {
        drawRange(recoSegment3D, attributeMap);
      }

      /// Draws the pair of segments as an arrow connecting the centers of them.
      void draw(const CDCAxialSegmentPair& axialSegmentPair, const AttributeMap& attributeMap = AttributeMap());

      /// Draws the pair of segments as an arrow connecting the centers of them.
      void draw(const CDCSegmentPair& segmentPair, const AttributeMap& attributeMap = AttributeMap());

      /// Draws the triple of segments as two arrows connecting the centers from start to the middle segment and from the middle to the end segment.
      void draw(const CDCSegmentTriple& segmentTriple, const AttributeMap& attributeMap = AttributeMap());

      /// Draws all CDCRecoHits3D of the segment
      void draw(const CDCTrack& track, const AttributeMap& attributeMap = AttributeMap())
      {
        drawRange(track, attributeMap);
      }

      /// Draws the hit content of the Genfit track candidate.
      void draw(const genfit::TrackCand& gfTrackCand, const AttributeMap& attributeMap = AttributeMap());

      /// Allow the drawing of pointers checking if the pointer is nonzero.
      template<class T>
      void draw(const T* ptr, const AttributeMap& attributeMap = AttributeMap())
      {
        if (ptr) draw(*ptr, attributeMap);
      }

      /// Draw store array
      template<class T>
      void draw(const StoreArray<T>& storeArray, const AttributeMap& attributeMap = AttributeMap())
      {
        if (storeArray) {
          drawRange(storeArray, attributeMap);
        }
      }

      /// Draws a range iterable collection of drawable elements
      template<class Range>
      void drawRange(const Range& range, const AttributeMap& attributeMap = AttributeMap())
      {
        if (not m_ptrPrimitivePlotter) return;
        PrimitivePlotter& primitivePlotter = *m_ptrPrimitivePlotter;
        primitivePlotter.startGroup(attributeMap);
        for (const auto& element : range) {
          draw(element);
        }
        primitivePlotter.endGroup();
      }

    private:
      /// Reference to the primitivePlotter instance used as backend for the draw commands.
      PrimitivePlotter* m_ptrPrimitivePlotter;

      /// Memory for the flag if the event data should be animated. If animation is supported is backend dependent.
      bool m_animate;

    }; //class

  } // namespace TrackFindingCDC
} // namespace Belle2
#endif // EVENTDATAPLOTTER_H_
