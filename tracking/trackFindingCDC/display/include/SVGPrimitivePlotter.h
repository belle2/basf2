/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef SVGPRIMITIVEPLOTTER_H_
#define SVGPRIMITIVEPLOTTER_H_

#include <tracking/trackFindingCDC/display/PrimitivePlotter.h>


#include <sstream>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// A concrete plotter that can draw primitive objects to standalone SVG files.
    class SVGPrimitivePlotter : public PrimitivePlotter {

    private:
      static int s_defaultNIndentationSpaces;
      static int s_addtionalNIndentationSpaces;

    public:
      /// Default constructor for ROOT compatibility.
      SVGPrimitivePlotter();

      /// Constructor taking additional attributes that go into the toplevel svg tag.
      /** @param svgAttributes    A map of attributes that goes into the toplevel svg tag. */
      SVGPrimitivePlotter(const AttributeMap& svgAttributes);

      /// Copy constructor
      SVGPrimitivePlotter(const SVGPrimitivePlotter& plotter);

      /// Make destructor virtual to handle polymorphic deconstruction.
      virtual ~SVGPrimitivePlotter();

    public:
      /// Returns a newly created plotter instance containing all information of this.
      /** The new object is created on the heap. The ownership is to the caller who has the responsibility to destroy it.
       */
      virtual SVGPrimitivePlotter* clone() const;

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


    private:
      /** Increases the current indention by one.*/
      void indent();

      /** Decreases the current indention by one.*/
      void dedent();

      /** Writes the xml header that indicates that this document will be a SVG document to the given output stream.
       *
       *  @param[out] outputStream       The output stream to which the result is written.
       */
      void writeSVGHeader(std::ostream& outputStream);

      /** Writes a preamble of <defs> that define an arrow cap which can be referenced by lines */
      void writeSVGDefs(std::ostream& outputStream);

      /** Writes an opening xml tag to the given output stream taking attributes from two sources.
       *
       *  @param[out] outputStream           The output stream to which the result is written.
       *  @param[in]  tagName                Name of the xml tag to be written
       *  @param[in]  geometryAttributeMap   A first map of attribute keys to attribute values that shall be attached to the tag.
       *                                     Usually this gives geometrical information but gets essentially treated the same way as the second attribute map
       *  @param[in]  styleAttributeMap      A second map of attribute keys to attribute values that shall be attached to the tag.
       *                                     Usually this gives styling information but gets essentially treated the same way as the first attribute map
       */
      void writeOpeningTag(std::ostream& outputStream,
                           const std::string& tagName,
                           const AttributeMap& geometryAttributeMap = AttributeMap(),
                           const AttributeMap& styleAttributeMap = AttributeMap());

      /** Writes a stand alone xml tag to the given output stream taking attributes from two sources.
       *
       *  @param[out] outputStream           The output stream to which the result is written.
       *  @param[in]  tagName                Name of the xml tag to be written
       *  @param[in]  geometryAttributeMap   A first map of attribute keys to attribute values that shall be attached to the tag.
       *                                     Usually this gives geometrical information but gets essentially treated the same way as the second attribute map
       *  @param[in]  styleAttributeMap      A second map of attribute keys to attribute values that shall be attached to the tag.
       *                                     Usually this gives styling information but gets essentially treated the same way as the first attribute map
       */
      void writeStandAloneTag(std::ostream& outputStream,
                              const std::string& tagName,
                              const AttributeMap& geometryAttributeMap = AttributeMap(),
                              const AttributeMap& styleAttributeMap = AttributeMap());

      /** Writes part that belongs between the <, > brakets. Used by writeOpeningTag and writeClosing.
       *
       *  Used by writeOpeningTag and writeClosing.
       *  Factored to avoid code dublication.
       *
       *  @param[out] outputStream           The output stream to which the result is written.
       *  @param[in]  tagName                Name of the xml tag to be written
       *  @param[in]  geometryAttributeMap   A first map of attribute keys to attribute values that shall be attached to the tag.
       *                                     Usually this gives geometrical information but gets essentially treated the same way as the second attribute map
       *  @param[in]  styleAttributeMap      A second map of attribute keys to attribute values that shall be attached to the tag.
       *                                     Usually this gives styling information but gets essentially treated the same way as the first attribute map
       */
      void writeTagIntern(std::ostream& outputStream,
                          const std::string& tagName,
                          const AttributeMap& geometryAttributeMap = AttributeMap(),
                          const AttributeMap& styleAttributeMap = AttributeMap());

      /** Writes attribute specification that belongs between the <, > brakets after the tag name.
       *
       *  @param[out] outputStream           The output stream to which the result is written.
       *  @param[in]  attributeMap           A map of attribute keys to attribute values that shall be attached to the tag.
       */
      void writeAttributes(std::ostream& outputStream,
                           const AttributeMap& attributeMap);

      /** Writes a closing xml tag to the given output stream.
       *
       *  @param[out] outputStream           The output stream to which the result is written.
       *  @param[in]  tagName                Name of the xml tag to be written*/
      void writeClosingTag(std::ostream& outputStream,
                           const std::string& tagName);

    public:

    private:
      /// Memory for the plotted elements. This contains only the fragment that is inbetween the svg tags and comes after the global svg definitions.
      std::stringstream m_svgContentStream;

      /// Memory for the number of spaces that shall be prepended to each line.
      int m_nIndentationSpaces;

      /// Memory for additional attributes to the toplevel svg element.
      AttributeMap m_svgAttributes;

    }; //class

  } // namespace TrackFindingCDC
} // namespace Belle2
#endif // SVGPRIMITIVEPLOTTER_H_
