/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef PRIMITIVEPLOTTER_H_
#define PRIMITIVEPLOTTER_H_

#include <tracking/trackFindingCDC/display/BoundingBox.h>

#include <algorithm>
#include <cmath>
#include <utility>
#include <map>


namespace Belle2 {
  namespace TrackFindingCDC {

    /// A map type for attributes names to values for additional drawing information
    typedef std::map<std::string, std::string> AttributeMap;

    /// A base class for plots of primitive objects.
    class PrimitivePlotter {

    public:
      /// A map type for attributes names to values for additional drawing information
      typedef Belle2::TrackFindingCDC::AttributeMap AttributeMap;

    public:
      /// The default width of the canvas to be plotted into
      static float s_defaultCanvasWidth;

      /// The default height of the canvas to be plotted into
      static float s_defaultCanvasHeight;

    public:

      /// Default constructor for ROOT compatibility.
      PrimitivePlotter();

      /// Make destructor virtual to handle polymorphic deconstruction.
      virtual ~PrimitivePlotter();

    public:
      /// Returns a newly created plotter instance containing all information of this.
      /** The new object is created on the heap. The ownership is to the caller who has the responsibility to destroy it.
       */
      virtual PrimitivePlotter* clone() const;

    public:
      /** Adds a line to the plot
       *
       *  Base implementation only updates the bounding box.
       *
       *  @param startX        x coordinate where the line starts.
       *  @param startY        y coordinate where the line starts.
       *  @param endX          x coordinate where the line ends.
       *  @param endY          y coordinate where the line ends.
       *  @param attributeMap  A map of sting keys and values that describe the drawing properties of the line.
       */
      virtual void drawLine(const float& startX,
                            const float& startY,
                            const float& endX,
                            const float& endY,
                            const AttributeMap& attributeMap = AttributeMap());

      /** Adds an arrow to the plot
       *
       *  Base implementation only updates the bounding box.
       *
       *  @param startX        x coordinate where the arrow starts.
       *  @param startY        y coordinate where the arrow starts.
       *  @param endX          x coordinate where the arrow ends.
       *  @param endY          y coordinate where the arrow ends.
       *  @param attributeMap  A map of sting keys and values that describe the drawing properties of the line.
       */
      virtual void drawArrow(const float& startX,
                             const float& startY,
                             const float& endX,
                             const float& endY,
                             const AttributeMap& attributeMap = AttributeMap());


      /** Adds a circle to the plot
       *
       *  Base implementation only updates the bounding box.
       *
       *  @param centerX        x coordinate of the circle center.
       *  @param centerY        y coordinate of the circle center.
       *  @param radius         radius of the circle
       *  @param attributeMap  A map of sting keys and values that describe the drawing properties of the line.
       */
      virtual void drawCircle(const float& centerX,
                              const float& centerY,
                              const float& radius,
                              const AttributeMap& attributeMap = AttributeMap());

      /** Adds a circle arc to the plot
       *
       *  Base implementation only updates the bounding box.
       *
       *  @param startX        x coordinate where the line starts.
       *  @param startY        y coordinate where the line starts.
       *  @param endX          x coordinate where the line end.
       *  @param endY          y coordinate where the line end.
       *  @param radius        Radius of the circle
       *  @param longArc       Boolean indicator if the long arc or the short arc is traversed from one point to the other.
       *  @param sweepFlag     Boolean indicator related to the curvature perceived one the circle arc. False means negative curvature. True mean positive curvature.
       *  @param attributeMap  A map of sting keys and values that describe the drawing properties of the line.
       */
      virtual void drawCircleArc(const float& startX,
                                 const float& startY,
                                 const float& endX,
                                 const float& endY,
                                 const float& radius,
                                 const bool& longArc,
                                 const bool& sweepFlag,
                                 const AttributeMap& attributeMap = AttributeMap());

      /** Indicates the start of a group of drawn elements. Meaning depends on the actual implementation.
       *
       *  Does nothing in the base implementation
       *
       */
      virtual void startGroup(const AttributeMap& attributeMap = AttributeMap());

      /** Indicates the end of a group of drawn elements. Meaning depends on the actual implementation.
       *
       *  Does nothing in the base implementation
       *
       */
      virtual void endGroup();

      /** Saves the current plot state to a file.
       *
       *  Deriving instances may should implement the approriate thing here and
       *  may return a modified string indicating the file name to which the plot as been written.
       *  It is allowed to append or change the file extension if the concret implementation decides to do so.
       *
       *  @param fileName       fileName where the plot shall be saved
       *  @return               Potentially modifed file name where the file has actually been written to.
       */
      virtual const std::string save(const std::string& fileName);

      /** Clears all drawed elements from the plotter.
       *
       *  Base implementation only updates the bounding box.
       */
      virtual void clear();

      /// Clears the current bounding box. Only draw calls following draw calls contribute to the bounding box
      void clearBoundingBox();

      /// Getter for the bounding box of all drawed objects.
      const BoundingBox& getBoundingBox() const
      { return m_boundingBox; }

      /// Setter for the bounding box of all drawed objects.
      void setBoundingBox(const BoundingBox& boundingBox)
      { m_boundingBox = boundingBox; }

    public:
      /// Getter for the canvas width in pixels.
      const float& getCanvasWidth()
      { return m_canvasWidth; }

      /// Getter for the canvas height in pixels.
      const float& getCanvasHeight()
      { return m_canvasHeight; }

      /** Setter for the canvas width in pixels.
       *  The canvas height denotes the size of the image being produced.
       *  The coordinates space that is visible in the picture is a seperate concept
       *  which is stored in the bounding box (getBoundingBox()). */
      void setCanvasWidth(const float& width)
      { m_canvasWidth = width; }

      /** Setter for the canvas height in pixels
       *  The canvas height denotes the size of the image being produced.
       *  The coordinates space that is visible in the picture is a seperate concept
       *  which is stored in the bounding box (getBoundingBox()). */
      void setCanvasHeight(const float& height)
      { m_canvasHeight = height; }

    private:
      /// Bounding box of the currently drawn objects.
      BoundingBox m_boundingBox;

      /// Memory for the width of the SVG drawing in pixels
      float m_canvasWidth;

      /// Memory for the height of the SVG drawing in pixels
      float m_canvasHeight;

    }; //class

  } // namespace TrackFindingCDC
} // namespace Belle2
#endif // PRIMITIVEPLOTTER_H_
