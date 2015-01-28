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

#include <tracking/trackFindingCDC/display/PrimitivePlotter.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// A class that can plot event related data types.
    class EventDataPlotter {


    public:
      /// Default constructor for ROOT compatibility. Uses an SVGPrimitivePlotter as backend.
      EventDataPlotter();

      /** Constructor taking the specifc PrimitivePlotter instance as backend.
       *
       *  Note that the EventDataPlotter takes ownership of the PrimitivePlotter and destroys it on its on own deconstruction.
       */
      EventDataPlotter(PrimitivePlotter* primitivePlotter);

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
      /// Getter for the canvas width in pixels.
      float getCanvasWidth();

      /// Getter for the canvas height in pixels.
      float getCanvasHeight();

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

    private:
      /// Reference to the primitivePlotter instance used as backend for the draw commands.
      PrimitivePlotter* m_ptrPrimitivePlotter;

    }; //class

  } // namespace TrackFindingCDC
} // namespace Belle2
#endif // EVENTDATAPLOTTER_H_
