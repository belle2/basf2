/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: dschneider, Oliver Frost                                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/display/EventDataPlotter.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    template <class T>
    class Styling;

    /// Helper class to generated the svg image from the various tracking objects.
    class CDCSVGPlotter {

    public:
      /**
       * Constructor.
       */
      CDCSVGPlotter(bool animate = false, bool forwardFade = false);

      /**
       * Make a copy of the current status of the plotter.
       */
      CDCSVGPlotter* clone();

      /**
       * Draws the wires.
       */
      void drawWires(const CDCWireTopology& cdcWireTopology);

      /**
       * Draws the interaction point.
       */
      void drawInteractionPoint();

      /**
       * Draws the individual super layer boundaries.
       * @param stroke The color to be used for drawing.
       */
      void drawSuperLayerBoundaries(const std::string& stroke);

      /**
       * Draws the outer CDCWall.
       * @param stroke The color to be used for drawing.
       */
      void drawOuterCDCWall(const std::string& stroke);

      /**
       * Draws the inner CDCWall.
       * @param stroke The color to be used for drawing.
       */
      void drawInnerCDCWall(const std::string& stroke);

      /**
       * Draws CDCHits.
       * @param storeArrayName The name of the StoreArray containing the CDCHits.
       * @param stroke Either the color to be used for drawing or the name of the method, which maps the CDCHit and its id to a color.
       * @param strokeWidth Either the stroke-width to be used for drawing or the name of the method, which maps the CDCHit and its id to a stroke-width.
       */
      void drawHits(const std::string& storeArrayName,
                    const std::string& stroke,
                    const std::string& strokeWidth);

      /**
       * Draws CDCSimHits.
       * @param storeArrayName The name of the StoreArray containing the CDCSimHits.
       * @param stroke Either the color to be used for drawing or the name of the method, which maps the CDCSimHit and its id to a color.
       * @param strokeWidth Either the stroke-width to be used for drawing or the name of the method, which maps the CDCSimHit and its id to a stroke-width.
       */
      void drawSimHits(const std::string& storeArrayName,
                       const std::string& stroke,
                       const std::string& strokeWidth);

      /**
       * Draws CDCWireHitClusters.
       * @param storeObjName The name of the StoreVector containing the CDCWireHitClusters.
       * @param stroke Either the color to be used for drawing or the name of the method, which maps the CDCHit and its id to a color.
       * @param strokeWidth Either the stroke-width to be used for drawing or the name of the method, which maps the CDCHit and its id to a stroke-width.
       */
      void drawClusters(const std::string& storeObjName,
                        const std::string& stroke,
                        const std::string& strokeWidth);

      /**
       * Draws CDCSegments.
       * @param storeObjName The name of the StoreVector containing the CDCSegments.
       * @param stroke Either the color to be used for drawing or the name of the method, which maps the CDCHit and its id to a color.
       * @param strokeWidth Either the stroke-width to be used for drawing or the name of the method, which maps the CDCHit and its id to a stroke-width.
       */
      void drawSegments(const std::string& storeObjName,
                        const std::string& stroke,
                        const std::string& strokeWidth);

      /**
       * Draws SegmentTrajectories.
       * @param storeObjName The name of the StoreVector containing the CDCSegments.
       * @param stroke Either the color to be used for drawing or the name of the method, which maps the CDCHit and its id to a color.
       * @param strokeWidth Either the stroke-width to be used for drawing or the name of the method, which maps the CDCHit and its id to a stroke-width.
       */
      void drawSegmentTrajectories(const std::string& storeObjName,
                                   const std::string& stroke,
                                   const std::string& strokeWidth);

      /**
       * Draw the axial to stereo segment pairs.
       * @param storeObjName The name of the StoreVector containing the CDCSegmentPairs.
       * @param stroke Either the color to be used for drawing or the name of the method, which maps the CDCSegmentPair and its id to a color.
       * @param strokeWidth Either the stroke-width to be used for drawing or the name of the method, which maps the CDCSegmentPair and its id to a stroke-width.
       */
      void drawSegmentPairs(const std::string& storeObjName,
                            const std::string& stroke,
                            const std::string& strokeWidth);

      /**
       * Draw the axial to axial segment pairs.
       * @param storeObjName The name of the StoreVector containing the CDCAxialSegmentPairs.
       * @param stroke Either the color to be used for drawing or the name of the method, which maps the CDCAxialSegmentPair and its id to a color.
       * @param strokeWidth Either the stroke-width to be used for drawing or the name of the method, which maps the CDCAxialSegmentPair and its id to a stroke-width.
       */
      void drawAxialSegmentPairs(const std::string& storeObjName,
                                 const std::string& stroke,
                                 const std::string& strokeWidth);

      /**
       * Draw the axial, stereo, axial segment triples.
       * @param storeObjName The name of the StoreVector containing the CDCSegmentTriples.
       * @param stroke Either the color to be used for drawing or the name of the method, which maps the CDCSegmentTriple and its id to a color.
       * @param strokeWidth Either the stroke-width to be used for drawing or the name of the method, which maps the CDCSegmentTriple and its id to a stroke-width.
       */
      void drawSegmentTriples(const std::string& storeObjName,
                              const std::string& stroke,
                              const std::string& strokeWidth);

      /**
       * Draw the trajectories of the axial, stereo, axial segment triples.
       * @param storeObjName The name of the StoreVector containing the CDCSegmentTriples.
       * @param stroke Either the color to be used for drawing or the name of the method, which maps the CDCSegmentTriple and its id to a color.
       * @param strokeWidth Either the stroke-width to be used for drawing or the name of the method, which maps the CDCSegmentTriple and its id to a stroke-width.
       */
      void drawSegmentTripleTrajectories(const std::string& storeObjName,
                                         const std::string& stroke,
                                         const std::string& strokeWidth);

      /**
       * Draws CDCTracks.
       * @param storeObjName The name of the StoreVector containing the CDCTracks.
       * @param stroke Either the color to be used for drawing or the name of the method, which maps the CDCSegmentTriple and its id to a color.
       * @param strokeWidth Either the stroke-width to be used for drawing or the name of the method, which maps the CDCSegmentTriple and its id to a stroke-width.
       */
      void drawTracks(const std::string& storeObjName,
                      const std::string& stroke,
                      const std::string& strokeWidth);

      /**
       * Draws trajectories of the tracks.
       * @param storeObjName The name of the StoreVector containing the CDCTracks.
       * @param stroke Either the color to be used for drawing or the name of the method, which maps the CDCHit and its id to a color.
       * @param strokeWidth Either the stroke-width to be used for drawing or the name of the method, which maps the CDCHit and its id to a stroke-width.
       */
      void drawTrackTrajectories(const std::string& storeObjName,
                                 const std::string& stroke,
                                 const std::string& strokeWidth);

      /**
       * Draw RecoTracks.
       * @param storeArrayName The name of the StoreArray containing the RecoTracks.
       * @param stroke Either the color to be used for drawing or the name of the method, which maps the CDCHit and its id to a color.
       * @param strokeWidth Either the stroke-width to be used for drawing or the name of the method, which maps the CDCHit and its id to a stroke-width.
       */
      void drawRecoTracks(const std::string& storeArrayName,
                          const std::string& stroke,
                          const std::string& strokeWidth);

      /**
       * Draw RecoTracks trajectories.
       * @param storeArrayName The name of the StoreArray containing the RecoTracks.
       * @param stroke Either the color to be used for drawing or the name of the method, which maps the CDCHit and its id to a color.
       * @param strokeWidth Either the stroke-width to be used for drawing or the name of the method, which maps the CDCHit and its id to a stroke-width.
       */
      void drawRecoTrackTrajectories(const std::string& storeArrayName,
                                     const std::string& stroke,
                                     const std::string& strokeWidth);

      /**
       * Draw MCParticles.
       * @param storeArrayName The name of the StoreArray containing the RecoTracks.
       * @param stroke Either the color to be used for drawing or the name of the method, which maps the CDCHit and its id to a color.
       * @param strokeWidth Either the stroke-width to be used for drawing or the name of the method, which maps the CDCHit and its id to a stroke-width.
       */
      void drawMCParticleTrajectories(const std::string& storeArrayName,
                                      const std::string& stroke,
                                      const std::string& strokeWidth);

    public:
      /* *** Section with some more complex operations *** */

      /**
       * Draw the CDCRLWireHits in the CDCSegments colored by the match of the right left passage informations.
       * It respects segments reconstructed backwards to have the opposite rl informations
       */
      void drawWrongRLHitsInSegments(const std::string& segmentsStoreObjName);

      /**
       * Draw the CDCRLWireHits in the CDCTracks colored by the match of the right left passage informations.
       * It respects tracks reconstructed backwards to have the opposite rl informations
       */
      void drawWrongRLHitsInTracks(const std::string& tracksStoreObjName);

    private:
      /**
       * Draw the CDCRLWireHits in the ACDCHitCollection colored by the match of the right left passage informations.
       * It respects hit collections reconstructed backwards to have the opposite rl informations.
       */
      template<class ACDCHitCollection>
      void drawWrongRLHits(const std::string& storeObjName);

    public:
      /**
       * Draw the CDCSimHits connected in the order of their getFlightTime for each Monte Carlo particle.
       * @param simHitStoreArrayName  The name of the StoreArray containing the CDCSimHits.
       * @param stroke                Color of the lines to be drawn.
       * @param strokeWidth           Width of the lines to be drawn.
       */
      void drawSimHitsConnectByToF(const std::string& simHitStoreArrayName,
                                   const std::string& stroke,
                                   const std::string& strokeWidth);

      /**
       * Draws the Monte Carlo true CDCSegmentPairs.
       *
       * This routine creates the true segment pairs from the stored segments to get
       * an overview which cases the segment triple reconstruction should cover.
       *
       * @param segmentsStoreObjName  The name of the StoreVector containing the CDCSegment2Ds,
       *                              from which the pair should be constructed.
       * @param stroke                Color of the lines to be drawn.
       * @param strokeWidth           Width of the lines to be drawn.
       */
      void drawMCSegmentPairs(const std::string& segmentsStoreObjName,
                              const std::string& stroke,
                              const std::string& strokeWidth);

      /**
       * Draws the Monte Carlo true CDCAxialSegmentPairs.
       *
       * This routine creates the true axial segment pairs from the stored segments to get
       * an overview which cases the segment triple reconstruction should cover.
       *
       * @param segmentsStoreObjName  The name of the StoreVector containing the CDCSegment2Ds,
       *                              from which the pair should be constructed.
       * @param stroke                Color of the lines to be drawn.
       * @param strokeWidth           Width of the lines to be drawn.
       */
      void drawMCAxialSegmentPairs(const std::string& segmentsStoreObjName,
                                   const std::string& stroke,
                                   const std::string& strokeWidth);

      /**
       * Draws the Monte Carlo true CDCSegmentTriples.
       *
       * This routine creates the true segment triples from the stored segments to get
       * an overview which cases the segment triple reconstruction should cover.
       *
       * @param segmentsStoreObjName  The name of the StoreVector containing the CDCSegment2Ds,
       *                              from which the pair should be constructed.
       * @param stroke                Color of the lines to be drawn.
       * @param strokeWidth           Width of the lines to be drawn.
       */
      void drawMCSegmentTriples(const std::string& segmentsStoreObjName,
                                const std::string& stroke,
                                const std::string& strokeWidth);

    public:
      /**
       * Save the current dom object representation to disk.
       * @param fileName The Name of the saved file. Defaults to "display.svg".
       */
      std::string saveFile(const std::string& fileName = "display.svg");

    private:
      /**
       * Function Template for drawing the elements of a given StoreArray.
       *
       * Needs to know the type of the objects in the StoreArray and the right styling for these objects.
       *
       * @tparam a_drawTrajectories Switch to draw the fitted trajectory from the object instead.
       * @param  storeArrayName   The name of the StoreArray containing the objects.
       * @param  styling          Functional object to construct attributes for each object to be drawn
       */
      template<class AItem, bool a_drawTrajectories = false>
      void drawStoreArray(const std::string& storeArrayName, Styling<AItem>& styling);

      /**
       * Function Template for drawing the elements of a given StoreVector.
       *
       * Needs to know the type of the objects in the store objects and the right styling for these objects.
       *
       * @tparam a_drawTrajectories Switch to draw the fitted trajectory from the object instead.
       * @param storeObjName     name of the StoreObject containing the objects.
       * @param styling          Functional object to construct attributes for each object to be drawn
       */
      template<class AItem, bool a_drawTrajectories = false>
      void drawStoreVector(const std::string& storeObjName, Styling<AItem>& styling);

      /**
       * Draw every element of an iterable object.
       * @tparam a_drawTrajectory Switch to draw the fitted trajectory from the object instead.
       * @param  items            An iterable object (StoreArray, std::vector...).
       * @param  styling          Styling object from which drawing attributes are constructed
       */
      template <bool a_drawTrajectory = false, class AIterable, class AStyling>
      void drawIterable(const AIterable& items, AStyling& styling);

      /// Draws the object or its trajectory with the given attributes
      template <bool a_drawTrajectory = false, class AObject>
      void draw(const AObject& object, const AttributeMap& attributeMap);

    private:
      /// The plotter instance to translate event data objects to svg expressions.
      EventDataPlotter m_eventdataPlotter;
    };
  }
}
