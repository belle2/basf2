/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/display/PrimitivePlotter.h>


#include <algorithm>
#include <utility>
#include <cmath>

using namespace Belle2;
using namespace TrackFindingCDC;

const float PrimitivePlotter::s_defaultCanvasWidth = 1120.0;
const float PrimitivePlotter::s_defaultCanvasHeight = 1120.0;

PrimitivePlotter::PrimitivePlotter() :
  m_boundingBox(),
  m_canvasWidth(s_defaultCanvasWidth),
  m_canvasHeight(s_defaultCanvasHeight)
{
}

PrimitivePlotter::~PrimitivePlotter() = default;

std::unique_ptr<PrimitivePlotter> PrimitivePlotter::clone() const
{
  return std::make_unique<PrimitivePlotter>(*this);
}



void PrimitivePlotter::drawLine(float startX,
                                float startY,
                                float endX,
                                float endY,
                                const AttributeMap& attributeMap  __attribute__((unused)))
{
  m_boundingBox &= BoundingBox(startX, startY, endX, endY);
}


void PrimitivePlotter::drawArrow(float startX,
                                 float startY,
                                 float endX,
                                 float endY,
                                 const AttributeMap& attributeMap  __attribute__((unused)))
{
  m_boundingBox &= BoundingBox(startX, startY, endX, endY);
}


void PrimitivePlotter::drawCircle(float centerX,
                                  float centerY,
                                  float radius,
                                  const AttributeMap& attributeMap  __attribute__((unused)))
{
  const float left = centerX - radius;
  const float bottom = centerY - radius;
  const float right = centerX + radius;
  const float top = centerY + radius;

  m_boundingBox &= BoundingBox(left, bottom, right, top);
}


void PrimitivePlotter::drawCircleArc(float startX,
                                     float startY,
                                     float endX,
                                     float endY,
                                     float radius  __attribute__((unused)),
                                     bool longArc  __attribute__((unused)),
                                     bool sweepFlag  __attribute__((unused)),
                                     const AttributeMap& attributeMap  __attribute__((unused)))
{
  // The actual extend of the circle arc is more complicating.
  // Please fill in the soultion if you have one.
  m_boundingBox &= BoundingBox(startX, startY, endX, endY);
}

void PrimitivePlotter::drawCurve(const std::vector<std::array<float, 2> >& points,
                                 const std::vector<std::array<float, 2> >& tangents  __attribute__((unused)),
                                 const AttributeMap& /*attributeMap */)
{
  for (size_t i = 0; i < points.size() - 1; ++i) {
    float startX = points[i][0];
    float startY = points[i][1];
    float endX = points[i + 1][0];
    float endY = points[i + 1][1];
    m_boundingBox &= BoundingBox(startX, startY, endX, endY);
  }
}



void PrimitivePlotter::startGroup(const AttributeMap& attributeMap  __attribute__((unused)))
{
}


void PrimitivePlotter::endGroup()
{
}


const std::string PrimitivePlotter::save(const std::string& fileName  __attribute__((unused)))
{
  return "";
}


void PrimitivePlotter::clear()
{
}

void PrimitivePlotter::clearBoundingBox()
{
  m_boundingBox.clear();
}

