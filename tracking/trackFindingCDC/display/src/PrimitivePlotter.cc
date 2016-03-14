/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "../include/PrimitivePlotter.h"

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;


PrimitivePlotter::PrimitivePlotter() : m_boundingBox()
{
}


PrimitivePlotter::~PrimitivePlotter()
{
}


PrimitivePlotter* PrimitivePlotter::clone()
{
  PrimitivePlotter* cloned = new PrimitivePlotter(*this);
  return cloned;
}



void PrimitivePlotter::drawLine(const float& startX,
                                const float& startY,
                                const float& endX,
                                const float& endY,
                                const AttributeMap& /* attributeMap */)
{
  m_boundingBox &= BoundingBox(startX, startY, endX, endY);
}


void PrimitivePlotter::drawArrow(const float& startX,
                                 const float& startY,
                                 const float& endX,
                                 const float& endY,
                                 const AttributeMap& /* attributeMap */)
{
  m_boundingBox &= BoundingBox(startX, startY, endX, endY);
}


void PrimitivePlotter::drawCircle(const float& centerX,
                                  const float& centerY,
                                  const float& radius,
                                  const AttributeMap& /* attributeMap */)
{
  const float left = centerX - radius;
  const float bottom = centerY - radius;
  const float right = centerX + radius;
  const float top = centerY + radius;

  m_boundingBox &= BoundingBox(left, bottom, right, top);
}


void PrimitivePlotter::drawCircleArc(const float& startX,
                                     const float& startY,
                                     const float& endX,
                                     const float& endY,
                                     const float& /* radius */,
                                     const bool& /* longArc */,
                                     const bool& /* sweepFlag */,
                                     const AttributeMap& /* attributeMap */)
{
  // The actual extend of the circle arc is more complicating.
  // Please fill in the soultion if you have one.
  m_boundingBox &= BoundingBox(startX, startY, endX, endY);
}


void PrimitivePlotter::startGroup(const AttributeMap& /* attributeMap */)
{
}


void PrimitivePlotter::endGroup()
{
}


const std::string PrimitivePlotter::save(const std::string& /* fileName */)
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

