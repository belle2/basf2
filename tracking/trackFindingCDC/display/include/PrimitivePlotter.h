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

    /// A two dimensional rectangle that keeps track of the extend of a drawing
    class PrimitivePlotter {

    public:
      /// A map type for attributes names to values for additional drawing information
      typedef std::map<std::string, std::string> AttributeMap;

      /// Default constructor for ROOT compatibility. Cell weight defaults to 1
      PrimitivePlotter();

      /// Make destructor virtual to handle polymorphic deconstruction.
      virtual ~PrimitivePlotter();

    public:
      /// Returns a newly created plotter instance containing all information of this.
      /** The new object is created on the heap. The ownership is to the caller who has the responsibility to destroy it.
       */
      virtual PrimitivePlotter* clone();

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

      /** Saves the current plot stead to a file.
       *
       *  Deriving instances may should implement the approriate thing here and
       *  may return a modified string indicating the file name to which the plot as been written.
       *  It is allows to append or change the file extension if the concret implementation decides to do so.
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

    private:
      /// Bounding box of the currently drawn objects.
      BoundingBox m_boundingBox;

    }; //class

  } // namespace TrackFindingCDC
} // namespace Belle2
#endif // PRIMITIVEPLOTTER_H_
