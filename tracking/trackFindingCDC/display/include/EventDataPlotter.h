/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef EVENTDATAPLOTTER_H_
#define EVENTDATAPLOTTER_H_

#include <cdc/dataobjects/CDCSimHit.h>
#include <cdc/dataobjects/CDCHit.h>

#include <tracking/trackFindingCDC/display/PrimitivePlotter.h>
#include <tracking/trackFindingCDC/eventdata/CDCEventData.h>
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

    public:
      // Drawing methods for the variuous event data objects.

      /// Draws a circle at the position with the given radius
      void draw(const Belle2::TrackFindingCDC::Circle2D& circle,
                AttributeMap attributeMap = AttributeMap());

      /// Draws the CDCWire as a small circle at the reference position.
      void draw(const Belle2::TrackFindingCDC::CDCWire& wire,
                AttributeMap attributeMap = AttributeMap());

      /// Draws the inner and the outer bound of the super layer.
      void draw(const Belle2::TrackFindingCDC::CDCWireSuperLayer& wireSuperLayer,
                AttributeMap attributeMap = AttributeMap());

      /// Draws the CDCSimHit as a momentum arrow starting at the track position with a length proportional to its momentum.
      void draw(const Belle2::CDCSimHit& simHit,
                AttributeMap attributeMap = AttributeMap());

      /// Draws the CDCHit as the wire position and its drift circle at the wire reference position.
      void draw(const Belle2::TrackFindingCDC::CDCWireHit& wireHit,
                AttributeMap attributeMap);

      /// Draws the CDCHit as the wire position and its drift circle at the wire reference position.
      void draw(const Belle2::CDCHit& cdcHit,
                AttributeMap attributeMap = AttributeMap());

      /// Draws a range iterable collection of drawable elements
      template<class Range>
      void draw(const Range& range, AttributeMap attributeMap = AttributeMap()) {
        for (const auto & element : range) {
          draw(element, attributeMap);
        }
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
