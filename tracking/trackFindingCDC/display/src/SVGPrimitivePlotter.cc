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

#include <fstream>

using namespace Belle2;
using namespace TrackFindingCDC;

const int SVGPrimitivePlotter::s_defaultNIndentationSpaces = 2;
const int SVGPrimitivePlotter::s_addtionalNIndentationSpaces = 2;

SVGPrimitivePlotter::SVGPrimitivePlotter()
  : PrimitivePlotter()
  , m_svgContentStream()
  , m_nIndentationSpaces(s_defaultNIndentationSpaces)
  , m_svgAttributes()
{
}

SVGPrimitivePlotter::SVGPrimitivePlotter(const AttributeMap& svgAttributes)
  : PrimitivePlotter()
  , m_svgContentStream()
  , m_nIndentationSpaces(s_defaultNIndentationSpaces)
  , m_svgAttributes(svgAttributes)
{
}

SVGPrimitivePlotter::SVGPrimitivePlotter(const SVGPrimitivePlotter& plotter)
  : PrimitivePlotter(plotter)
  , m_svgContentStream(plotter.m_svgContentStream.str(), std::ostringstream::ate)
  , m_nIndentationSpaces(plotter.m_nIndentationSpaces)
  , m_svgAttributes(plotter.m_svgAttributes)
{
}

std::unique_ptr<PrimitivePlotter> SVGPrimitivePlotter::clone() const
{
  return std::make_unique<SVGPrimitivePlotter>(*this);
}

void SVGPrimitivePlotter::drawLine(float startX,
                                   float startY,
                                   float endX,
                                   float endY,
                                   const AttributeMap& attributeMap)
{
  PrimitivePlotter::drawLine(startX, startY, endX, endY, attributeMap);

  AttributeMap geometryAttributeMap {
    // cppcheck-suppress ignoredReturnValue
    {"x1", std::to_string(startX)},
    // cppcheck-suppress ignoredReturnValue
    {"x2", std::to_string(endX)},
    // cppcheck-suppress ignoredReturnValue
    {"y1", std::to_string(startY)},
    // cppcheck-suppress ignoredReturnValue
    {"y2", std::to_string(endY)}
  };

  writeStandAloneTag(m_svgContentStream, "line", geometryAttributeMap, attributeMap);

}


void SVGPrimitivePlotter::drawArrow(float startX,
                                    float startY,
                                    float endX,
                                    float endY,
                                    const AttributeMap& attributeMap)
{
  PrimitivePlotter::drawArrow(startX, startY, endX, endY, attributeMap);

  AttributeMap geometryAttributeMap {
    // cppcheck-suppress ignoredReturnValue
    {"x1", std::to_string(startX)},
    // cppcheck-suppress ignoredReturnValue
    {"x2", std::to_string(endX)},
    // cppcheck-suppress ignoredReturnValue
    {"y1", std::to_string(startY)},
    // cppcheck-suppress ignoredReturnValue
    {"y2", std::to_string(endY)},
    {"marker-end" , "url(#endArrow)"}
  };

  writeStandAloneTag(m_svgContentStream, "line", geometryAttributeMap, attributeMap);
}


void SVGPrimitivePlotter::drawCircle(float centerX,
                                     float centerY,
                                     float radius,
                                     const AttributeMap& attributeMap)
{
  PrimitivePlotter::drawCircle(centerX, centerY, radius, attributeMap);

  AttributeMap geometryAttributeMap {
    // cppcheck-suppress ignoredReturnValue
    {"cx", std::to_string(centerX)},
    // cppcheck-suppress ignoredReturnValue
    {"cy", std::to_string(centerY)},
    // cppcheck-suppress ignoredReturnValue
    {"r", std::to_string(std::fabs(radius))}
  };

  writeStandAloneTag(m_svgContentStream, "circle", geometryAttributeMap, attributeMap);

}


void SVGPrimitivePlotter::drawCircleArc(float startX,
                                        float startY,
                                        float endX,
                                        float endY,
                                        float radius,
                                        bool longArc,
                                        bool sweepFlag,
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

  std::ostringstream pathStream;

  pathStream << "M" << ' ';
  pathStream << std::to_string(startX) << ' ';
  pathStream << std::to_string(startY) << ' ';
  pathStream << "A" << ' ';
  pathStream << std::to_string(radiusX) << ' ';
  pathStream << std::to_string(radiusY) << ' ';
  pathStream << std::to_string(rotationAngle) << ' ';
  pathStream << std::to_string(longArc) << ' ';
  pathStream << std::to_string(sweepFlag) << ' ';
  pathStream << std::to_string(endX) << ' ';
  pathStream << std::to_string(endY);

  AttributeMap geometryAttributeMap{{"d", pathStream.str()}};

  writeStandAloneTag(m_svgContentStream, "path", geometryAttributeMap, attributeMap);
}

void SVGPrimitivePlotter::drawCurve(const std::vector<std::array<float, 2>>& points,
                                    const std::vector<std::array<float, 2>>& tangents,
                                    const AttributeMap& attributeMap)
{
  // Magic number for circle approximation with splines
  static const double k = 4.0 / 3.0 * (std::sqrt(2.0) - 1.0);

  B2ASSERT("Expect number of points and tangents to be the same", points.size() == tangents.size());
  if (points.size() < 2) return;

  PrimitivePlotter::drawCurve(points, tangents, attributeMap);

  std::ostringstream pathStream;

  // Move to point
  float startX = std::get<0>(points[0]);
  float startY = std::get<1>(points[0]);

  pathStream << "M";
  pathStream << ' ' << std::to_string(startX);
  pathStream << ' ' << std::to_string(startY);

  for (size_t iCurrent = 0, iNext = 1; iNext < points.size(); iCurrent = iNext, ++iNext) {

    float currentTX = std::get<0>(tangents[iCurrent]);
    float currentTY = std::get<1>(tangents[iCurrent]);

    float nextTX = std::get<0>(tangents[iNext]);
    float nextTY = std::get<1>(tangents[iNext]);

    float currentT = std::hypot(currentTX, currentTY);
    float nextT = std::hypot(nextTX, nextTY);

    currentTX /= currentT;
    currentTY /= currentT;
    nextTX /= nextT;
    nextTY /= nextT;

    float currentX = std::get<0>(points[iCurrent]);
    float currentY = std::get<1>(points[iCurrent]);

    float nextX = std::get<0>(points[iNext]);
    float nextY = std::get<1>(points[iNext]);

    // Circle arc angle
    float alpha = std::atan2(currentTX * nextTY - currentTY * nextTX,
                             currentTX * nextTX + currentTY * nextTY);

    float distance = std::hypot(currentX - nextX, currentY - nextY);
    float controlLength = k * distance / 2 / std::cos(alpha / 2);

    float currentControlX = currentX + currentTX * controlLength;
    float currentControlY = currentY + currentTY * controlLength;

    float nextControlX = nextX - nextTX * controlLength;
    float nextControlY = nextY - nextTY * controlLength;

    pathStream << ' ' << "C";

    pathStream << ' ' << std::to_string(currentControlX);
    pathStream << ' ' << std::to_string(currentControlY);

    pathStream << ' ' << std::to_string(nextControlX);
    pathStream << ' ' << std::to_string(nextControlY);

    pathStream << ' ' << std::to_string(nextX);
    pathStream << ' ' << std::to_string(nextY);
  }

  AttributeMap geometryAttributeMap{{"d", pathStream.str()}};

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
    B2WARNING("Mismatching calls to startGroup and endGroup detected. "
              << "Proceeding to write the illforamed result.");
  }

  int savedNIndentationSpaces = m_nIndentationSpaces;
  m_nIndentationSpaces = 0;

  std::ofstream outputFileStream;
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
  std::ostringstream viewBoxStringStream;

  viewBoxStringStream << getBoundingBox().getLeft();
  viewBoxStringStream << " ";
  viewBoxStringStream << getBoundingBox().getBottom();
  viewBoxStringStream << " ";
  viewBoxStringStream << getBoundingBox().getWidth();
  viewBoxStringStream << " ";
  viewBoxStringStream << getBoundingBox().getHeight();

  AttributeMap variableAttributeMap{
    // cppcheck-suppress ignoredReturnValue
    {"height", std::to_string(getCanvasHeight())},
    // cppcheck-suppress ignoredReturnValue
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
  m_nIndentationSpaces = savedNIndentationSpaces;

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
  outputStream << std::endl;

  indent();

  if (geometryAttributeMap.count("_showAt") + styleAttributeMap.count("_showAt")) {
    const std::string showAt = geometryAttributeMap.count("_showAt")
                               ? geometryAttributeMap.at("_showAt")
                               : styleAttributeMap.at("_showAt");

    AttributeMap setAttributeMap{
      {"attributeName", "visibility"},
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
    outputStream << std::endl;
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
  for (const std::pair<std::string, std::string>& attribute : attributeMap) {

    const std::string& key = attribute.first;
    const std::string& value = attribute.second;

    // Skip special attribute
    if ('_' == key[0]) {
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

void SVGPrimitivePlotter::writeClosingTag(std::ostream& outputStream, const std::string& tagName)
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
  outputStream << std::endl;
}
