/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackFindingCDC/display/SVGPrimitivePlotter.h>

#include <framework/logging/Logger.h>

#include <boost/algorithm/string/predicate.hpp>
#include <fstream>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

int SVGPrimitivePlotter::s_defaultNIndentationSpaces = 2;
int SVGPrimitivePlotter::s_addtionalNIndentationSpaces = 2;

SVGPrimitivePlotter::SVGPrimitivePlotter() :
  PrimitivePlotter(),
  m_svgContentStream(),
  m_nIndentationSpaces(s_defaultNIndentationSpaces),
  m_svgAttributes()
{
}

SVGPrimitivePlotter::SVGPrimitivePlotter(const AttributeMap& svgAttributes) :
  PrimitivePlotter(),
  m_svgContentStream(),
  m_nIndentationSpaces(s_defaultNIndentationSpaces),
  m_svgAttributes(svgAttributes)
{
}

SVGPrimitivePlotter::SVGPrimitivePlotter(const SVGPrimitivePlotter& plotter) :
  PrimitivePlotter(plotter),
  m_svgContentStream(plotter.m_svgContentStream.str()),
  m_nIndentationSpaces(plotter.m_nIndentationSpaces),
  m_svgAttributes(plotter.m_svgAttributes)
{
}

std::unique_ptr<PrimitivePlotter> SVGPrimitivePlotter::clone() const
{
  return std::unique_ptr<PrimitivePlotter>(new SVGPrimitivePlotter(*this));
}

void SVGPrimitivePlotter::drawLine(const float& startX,
                                   const float& startY,
                                   const float& endX,
                                   const float& endY,
                                   const AttributeMap& attributeMap)
{
  PrimitivePlotter::drawLine(startX, startY, endX, endY, attributeMap);

  AttributeMap geometryAttributeMap {
    {"x1", std::to_string(startX)},
    {"x2", std::to_string(endX)},
    {"y1", std::to_string(startY)},
    {"y2", std::to_string(endY)}
  };

  writeStandAloneTag(m_svgContentStream, "line", geometryAttributeMap, attributeMap);

}


void SVGPrimitivePlotter::drawArrow(const float& startX,
                                    const float& startY,
                                    const float& endX,
                                    const float& endY,
                                    const AttributeMap& attributeMap)
{
  PrimitivePlotter::drawArrow(startX, startY, endX, endY, attributeMap);

  AttributeMap geometryAttributeMap {
    {"x1", std::to_string(startX)},
    {"x2", std::to_string(endX)},
    {"y1", std::to_string(startY)},
    {"y2", std::to_string(endY)},
    {"marker-end" , "url(#endArrow)"}
  };

  writeStandAloneTag(m_svgContentStream, "line", geometryAttributeMap, attributeMap);
}


void SVGPrimitivePlotter::drawCircle(const float& centerX,
                                     const float& centerY,
                                     const float& radius,
                                     const AttributeMap& attributeMap)
{
  PrimitivePlotter::drawCircle(centerX, centerY, radius, attributeMap);

  AttributeMap geometryAttributeMap {
    {"cx", std::to_string(centerX)},
    {"cy", std::to_string(centerY)},
    {"r", std::to_string(std::fabs(radius))}
  };

  writeStandAloneTag(m_svgContentStream, "circle", geometryAttributeMap, attributeMap);

}


void SVGPrimitivePlotter::drawCircleArc(const float& startX,
                                        const float& startY,
                                        const float& endX,
                                        const float& endY,
                                        const float& radius,
                                        const bool& longArc,
                                        const bool& sweepFlag,
                                        const AttributeMap& attributeMap)
{
  PrimitivePlotter::drawCircleArc(startX,
                                  startY,
                                  endX,
                                  endY,
                                  radius,
                                  longArc,
                                  sweepFlag,
                                  attributeMap);

  // Compile the path for a ellipses arc
  // spelling out all the parts in case somebody want adopt
  // although only the simpler circle arc case is needed.

  const float radiusX = std::fabs(radius);
  const float radiusY = std::fabs(radius);
  const float rotationAngle = 0;

  std::stringstream pathStream;

  pathStream << "M" << ' ';
  pathStream << to_string(startX) << ' ';
  pathStream << to_string(startY) << ' ';
  pathStream << "A" << ' ';
  pathStream << to_string(radiusX) << ' ';
  pathStream << to_string(radiusY) << ' ';
  pathStream << to_string(rotationAngle) << ' ';
  pathStream << to_string(longArc) << ' ';
  pathStream << to_string(sweepFlag) << ' ';
  pathStream << to_string(endX) << ' ';
  pathStream << to_string(endY);

  AttributeMap geometryAttributeMap {
    {"d", pathStream.str()}
  };

  writeStandAloneTag(m_svgContentStream, "path", geometryAttributeMap, attributeMap);

}


void SVGPrimitivePlotter::startGroup(const AttributeMap& attributeMap)
{
  writeOpeningTag(m_svgContentStream, "g", attributeMap);
}


void SVGPrimitivePlotter::endGroup()
{
  writeClosingTag(m_svgContentStream, "g");
}


const std::string SVGPrimitivePlotter::save(const std::string& fileName)
{
  // Check indention
  if (m_nIndentationSpaces != s_defaultNIndentationSpaces) {
    B2WARNING("Mismatching calls to startGroup and endGroup detected. Proceeding to write the illforamed result.");
  }

  int savedNIndentationSpaces = m_nIndentationSpaces;
  m_nIndentationSpaces = 0;

  ofstream outputFileStream;
  outputFileStream.open(fileName);

  writeSVGHeader(outputFileStream);

  AttributeMap standardAttributeMap {
    {"baseProfile", "full"},
    {"ev", "http://www.w3.org/2001/xml-events"},
    {"version", "1.1"},
    {"xlink", "http://www.w3.org/1999/xlink"},
    {"xmlns", "http://www.w3.org/2000/svg"}
  };


  // Combine the viewbox specification from the bounding box
  // Format "{left} {bottom} {width} {height}"
  std::stringstream viewBoxStringStream;

  viewBoxStringStream << getBoundingBox().getLeft();
  viewBoxStringStream << " ";
  viewBoxStringStream << getBoundingBox().getBottom();
  viewBoxStringStream << " ";
  viewBoxStringStream << getBoundingBox().getWidth();
  viewBoxStringStream << " ";
  viewBoxStringStream << getBoundingBox().getHeight();

  AttributeMap variableAttributeMap {
    {"height", std::to_string(getCanvasHeight())},
    {"width", std::to_string(getCanvasWidth())},
    {"viewBox", viewBoxStringStream.str()},
  };

  variableAttributeMap.insert(m_svgAttributes.begin(), m_svgAttributes.end());

  writeOpeningTag(outputFileStream, "svg", standardAttributeMap, variableAttributeMap);

  writeSVGDefs(outputFileStream);

  // Copy the stream the output and rewind it to its original position.
  outputFileStream << m_svgContentStream.str();

  writeClosingTag(outputFileStream, "svg");

  outputFileStream.close();
  m_nIndentationSpaces = savedNIndentationSpaces ;

  return fileName;
}


void SVGPrimitivePlotter::clear()
{
  PrimitivePlotter::clear();
  m_svgContentStream.str("");
  m_nIndentationSpaces = s_defaultNIndentationSpaces;
}


void SVGPrimitivePlotter::indent()
{
  m_nIndentationSpaces += s_addtionalNIndentationSpaces;
}

void SVGPrimitivePlotter::dedent()
{
  m_nIndentationSpaces -= s_addtionalNIndentationSpaces;
}


void SVGPrimitivePlotter::writeSVGHeader(std::ostream& outputStream)
{
  outputStream << "<?xml version=\"1.0\" ?>" << std::endl;
  outputStream << "<!DOCTYPE svg PUBLIC '-//W3C//DTD SVG 1.1//EN' 'http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd'>" << std::endl;
}


void SVGPrimitivePlotter::writeSVGDefs(std::ostream& outputStream)
{
  writeOpeningTag(outputStream, "defs");

  AttributeMap markerAttributes {
    { "id", "endArrow"},
    { "viewBox", "0 0 10 10"},
    { "refX", "1"},
    { "refY", "5"},
    { "markerUnits", "strokeWidth"},
    { "markerWidth", "4"},
    { "markerHeight", "3"},
    {"orient", "auto"}
  };

  writeOpeningTag(outputStream, "marker", markerAttributes);


  AttributeMap polylineAttributes {
    {"points", "0,0 10,5 0,10 1,5"},
    {"fill", "black"}
  };

  writeStandAloneTag(outputStream, "polyline", polylineAttributes);

  writeClosingTag(outputStream, "marker");
  writeClosingTag(outputStream, "defs");

}

void SVGPrimitivePlotter::writeOpeningTag(std::ostream& outputStream,
                                          const std::string& tagName,
                                          const AttributeMap& geometryAttributeMap,
                                          const AttributeMap& styleAttributeMap)
{
  // Indentation
  outputStream << std::string(m_nIndentationSpaces, ' ');

  // Opening braket
  outputStream << '<';

  // Write contained part
  writeTagIntern(outputStream, tagName, geometryAttributeMap, styleAttributeMap);

  // Closing bracket
  outputStream << '>';

  // New line
  outputStream << endl;

  indent();

  if (geometryAttributeMap.count("_showAt") + styleAttributeMap.count("_showAt")) {
    const std::string showAt = geometryAttributeMap.count("_showAt") ? geometryAttributeMap.at("_showAt") :
                               styleAttributeMap.at("_showAt");

    AttributeMap setAttributeMap {
      {"attributeName", "visibility"} ,
      {"to", "hidden"},
      {"begin", "0s"},
      {"end", showAt}
    };

    writeStandAloneTag(outputStream, "set", setAttributeMap);
  }
}


void SVGPrimitivePlotter::writeStandAloneTag(std::ostream& outputStream,
                                             const std::string& tagName,
                                             const AttributeMap& geometryAttributeMap,
                                             const AttributeMap& styleAttributeMap)
{
  // Special handling for _showAt attribute, which introduces a contained <set> tag
  if (geometryAttributeMap.count("_showAt") + styleAttributeMap.count("_showAt")) {
    writeOpeningTag(outputStream, tagName, geometryAttributeMap, styleAttributeMap);
    writeClosingTag(outputStream, tagName);
  } else {

    // Indentation
    outputStream << std::string(m_nIndentationSpaces, ' ');

    // Opening braket
    outputStream << '<';

    // Write contained part
    writeTagIntern(outputStream, tagName, geometryAttributeMap, styleAttributeMap);

    // Closing bracket
    outputStream << '/' << '>';

    // New line
    outputStream << endl;
  }
}


void SVGPrimitivePlotter::writeTagIntern(std::ostream& outputStream,
                                         const std::string& tagName,
                                         const AttributeMap& geometryAttributeMap,
                                         const AttributeMap& styleAttributeMap)
{
  // Tag name
  outputStream << tagName;

  // First attribute map
  if (not geometryAttributeMap.empty()) {
    outputStream << ' ';
    writeAttributes(outputStream, geometryAttributeMap);
  }

  // Second attribute map
  if (not styleAttributeMap.empty()) {
    outputStream << ' ';
    writeAttributes(outputStream, styleAttributeMap);
  }
}

void SVGPrimitivePlotter::writeAttributes(std::ostream& outputStream,
                                          const AttributeMap& attributeMap)
{
  bool first = true;
  for (const pair<string, string>& attribute : attributeMap) {

    const string& key = attribute.first;
    const string& value = attribute.second;

    // Skip special attribute
    if (boost::starts_with(key, "_")) {
      continue;
    }

    // Introduce a space between every attribute
    if (first) {
      first = false;
    } else {
      outputStream << ' ';
    }

    outputStream << key;
    outputStream << '=' << '"';
    outputStream << value;
    outputStream << '"';
  }
}

void SVGPrimitivePlotter::writeClosingTag(std::ostream& outputStream,
                                          const std::string& tagName)
{
  dedent();

  // Indentation
  outputStream << std::string(m_nIndentationSpaces, ' ');

  // Opening braket
  outputStream << '<' << '/';

  // Tag name
  outputStream << tagName;

  // Closing bracket
  outputStream << '>';

  // New line
  outputStream << endl;
}
