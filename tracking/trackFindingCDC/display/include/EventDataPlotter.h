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

#include <tracking/trackFindingCDC/display/PrimitivePlotter.h>

#include <framework/datastore/StoreArray.h>

#include <memory>

namespace Belle2 {
  class MCParticle;
  class RecoTrack;
  class CDCHit;
  class CDCSimHit;

  namespace TrackFindingCDC {

    class BoundingBox;

    class Circle2D;

    class CDCWireTopology;
    class CDCWireSuperLayer;
    class CDCWire;

    class CDCTrajectory2D;

    class CDCWireHit;
    class CDCTangent;
    class CDCRecoHit2D;
    class CDCRecoHit3D;

    class CDCWireHitCluster;
    class CDCSegment2D;
    class CDCSegment3D;

    class CDCSegmentPair;
    class CDCAxialSegmentPair;
    class CDCSegmentTriple;

    class CDCTrack;

    /// A class that can plot event related data types.
    class EventDataPlotter {


    public:
      /// Forward the Attributre map from the primitive plotter.
      using AttributeMap = PrimitivePlotter::AttributeMap;

    public:
      /// Default constructor for ROOT compatibility. Uses an SVGPrimitivePlotter as backend.
      explicit EventDataPlotter(bool animate = false, bool forwardFade = false);

      /**
       *  Constructor taking the specifc PrimitivePlotter instance as backend.
       *
       *  Note that the EventDataPlotter takes ownership of the PrimitivePlotter and destroys it on its on own deconstruction.
       */
      explicit EventDataPlotter(std::unique_ptr<PrimitivePlotter> ptrPrimitivePlotter,
                                bool animate = false,
                                bool forwardFade = false);

      /// Copy constructor
      EventDataPlotter(const EventDataPlotter& eventDataPlotter);

    public:
      /**
       *  Saves the current plot stead to a file.
       *
       *  Deriving instances may should implement the approriate thing here and
       *  may return a modified string indicating the file name to which the plot as been written.
       *  It is allowed to append or change the file extension if the concret implementation PrimitivePlotter decides to do so.
       *
       *  @param fileName       fileName where the plot shall be saved
       *  @return               Potentially modifed file name where the file has actually been written to.
       */
      const std::string save(const std::string& fileName);

      /// Clears all drawed elements from the plotter.
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

      /**
       *  Setter for the canvas width in pixels.
       *  The canvas height denotes the size of the image being produced.
       *  The coordinates space that is visible in the picture is a seperate concept
       *  which is stored in the bounding box (getBoundingBox()).
       */
      void setCanvasWidth(float width);

      /**
       *  Setter for the canvas height in pixels
       *  The canvas height denotes the size of the image being produced.
       *  The coordinates space that is visible in the picture is a seperate concept
       *  which is stored in the bounding box (getBoundingBox()).
       */
      void setCanvasHeight(float height);

      /**
       *  Indicates the start of a group of drawn elements.
       */
      void startGroup(const AttributeMap& attributeMap = AttributeMap());

      /**
       *  Indicates the end of a group of drawn elements.
       */
      void endGroup();

    public:
      /// Converts a time given in nanoseconds to a time sting of the from "%fs".
      std::string getAnimationTimeFromNanoSeconds(float nanoseconds)
      {
        return std::to_string(nanoseconds) + "s";
      }

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

      /// Draw the super layer bounds of the CDC.
      void drawSuperLayerBoundaries(const AttributeMap& attributeMap = AttributeMap());

      /// Draws a straight Line.
      void drawLine(float startX,
                    float startY,
                    float endX,
                    float endY,
                    const AttributeMap& attributeMap = AttributeMap());

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
                const AttributeMap& attributeMap = AttributeMap());

      /// Draws the CDCRecoHit3D as a drift circle at the two dimensional reference wire position and a point at the reconstructed position"""
      void draw(const Belle2::TrackFindingCDC::CDCTangent& tangent,
                const AttributeMap& attributeMap = AttributeMap());

      /// Draws the CDCTrajectory from the start point until it first exits the CDC.
      void draw(const CDCTrajectory2D& trajectory2D, AttributeMap attributeMap = AttributeMap());

      /// Draws all CDCWireHits of the cluster
      void draw(const CDCWireHitCluster& wireHitCluster, const AttributeMap& attributeMap = AttributeMap());

      /// Draws all CDCRecoHits2D of the segment
      void draw(const CDCSegment2D& segment2D, const AttributeMap& attributeMap = AttributeMap());

      /// Draws all CDCRecoHits3D of the segment
      void draw(const CDCSegment3D& segment3D, const AttributeMap& attributeMap = AttributeMap());

      /// Draws the pair of segments as an arrow connecting the centers of them.
      void draw(const CDCAxialSegmentPair& axialSegmentPair, const AttributeMap& attributeMap = AttributeMap());

      /// Draws the pair of segments as an arrow connecting the centers of them.
      void draw(const CDCSegmentPair& segmentPair, const AttributeMap& attributeMap = AttributeMap());

      /// Draws the triple of segments as two arrows connecting the centers from start to the middle segment and from the middle to the end segment.
      void draw(const CDCSegmentTriple& segmentTriple, const AttributeMap& attributeMap = AttributeMap());

      /// Draws all CDCRecoHits3D of the segment
      void draw(const CDCTrack& track, const AttributeMap& attributeMap = AttributeMap());

      /// Draws the hit content of the RecoTrack.
      void draw(const RecoTrack& recoTrack, const AttributeMap& attributeMap = AttributeMap());

      /// Draws the trajectory that is represented by the MC particle
      void drawTrajectory(const MCParticle& mcParticle, const AttributeMap& attributeMap = AttributeMap());

      /// Draws trajectory of the CDCSegment2D
      void drawTrajectory(const CDCSegment2D& segment, const AttributeMap& attributeMap = AttributeMap());

      /// Draws trajectory of the CDCSegmentTriple
      void drawTrajectory(const CDCSegmentTriple& segmentTriple, const AttributeMap& attributeMap = AttributeMap());

      /// Draws trajectory of the CDCTrack
      void drawTrajectory(const CDCTrack& track, const AttributeMap& attributeMap = AttributeMap());

      /// Draws curve along the fitted points of the RecoTrack
      void drawTrajectory(const RecoTrack& recoTrack, const AttributeMap& attributeMap = AttributeMap());

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
        if (not storeArray) return;
        drawRange(storeArray, attributeMap);
      }

      /// Draws a range iterable collection of drawable elements
      template<class ARange>
      void drawRange(const ARange& range, const AttributeMap& attributeMap = AttributeMap())
      {
        if (not m_ptrPrimitivePlotter) return;
        PrimitivePlotter& primitivePlotter = *m_ptrPrimitivePlotter;
        primitivePlotter.startGroup(attributeMap);
        for (const auto& element : range) {
          draw(element);
        }
        primitivePlotter.endGroup();
      }

      /// Draws a range iterable collection of drawable elements
      template<class ARange>
      void drawRangeWithFade(const ARange& range, const AttributeMap& attributeMap = AttributeMap())
      {
        if (not m_ptrPrimitivePlotter) return;
        PrimitivePlotter& primitivePlotter = *m_ptrPrimitivePlotter;
        primitivePlotter.startGroup(attributeMap);
        float opacity = 1;
        float endOpacity = 0.5;
        int n = std::distance(std::begin(range), std::end(range));
        if (n == 0) return;
        float opacityFactor = std::pow(endOpacity / opacity, 1.0 / (n - 1));
        for (const auto& element : range) {
          const auto opacity_str = std::to_string(opacity);
          AttributeMap elementAttributeMap{{"opacity", opacity_str}};
          draw(element, elementAttributeMap);
          opacity *= opacityFactor;
        }
        primitivePlotter.endGroup();
      }

    private:
      /// Reference to the primitivePlotter instance used as backend for the draw commands.
      std::unique_ptr<PrimitivePlotter> m_ptrPrimitivePlotter;

      /// Memory for the flag if the event data should be animated. If animation is supported is backend dependent.
      bool m_animate = false;

      /// Memory for the flag whether the orientation of tracks segments etc should be shown as diming opacity
      bool m_forwardFade = false;

    };
  }
}
