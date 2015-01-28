/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "../include/EventDataPlotter.h"

#include <tracking/trackFindingCDC/display/SVGPrimitivePlotter.h>

#include <framework/logging/Logger.h>
#include <cmath>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

namespace {

  /// Returns the time of flight of the related CDCSimHit. NAN if not found.
  float getFlightTime(const CDCHit* ptrHit)
  {
    float tof = NAN;
    if (ptrHit) {
      const CDCHit& hit = *ptrHit;
      const CDCSimHit* ptrSimHit = hit.getRelated<CDCSimHit>();
      if (ptrSimHit) {
        const CDCSimHit& simHit = *ptrSimHit;
        tof = simHit.getFlightTime();
      }
    }
    return tof;
  }

  /// Updates the fill color to the stroke color unless it has already been set otherwise.
  void updateFillToStroke(EventDataPlotter::AttributeMap& attributeMap)
  {
    if (not attributeMap.count("fill") or attributeMap["fill"] != "") {
      if (attributeMap.count("stroke")) {
        attributeMap["fill"] = attributeMap["stroke"];
      }
    }
  }

}


EventDataPlotter::EventDataPlotter(bool animate) :
  m_ptrPrimitivePlotter(new SVGPrimitivePlotter()),
  m_animate(animate)
{
}

EventDataPlotter::EventDataPlotter(PrimitivePlotter* ptrPrimitivePlotter, bool animate) :
  m_ptrPrimitivePlotter(ptrPrimitivePlotter),
  m_animate(animate)
{
  if (not m_ptrPrimitivePlotter) {
    B2WARNING("EventDataPlotter initialized with nullptr. Using default backend SVGPrimitivePlotter.");
    m_ptrPrimitivePlotter = new SVGPrimitivePlotter();
  }
}

EventDataPlotter::EventDataPlotter(const EventDataPlotter& eventDataPlotter) :
  m_ptrPrimitivePlotter(eventDataPlotter.m_ptrPrimitivePlotter ? eventDataPlotter.m_ptrPrimitivePlotter->clone() : nullptr),
  m_animate(eventDataPlotter.m_animate)
{
}

EventDataPlotter::~EventDataPlotter()
{
  delete m_ptrPrimitivePlotter;
  m_ptrPrimitivePlotter = nullptr;
}

EventDataPlotter* EventDataPlotter::clone()
{
  return new EventDataPlotter(*this);
}

const std::string EventDataPlotter::save(const std::string& fileName)
{
  if (m_ptrPrimitivePlotter) {
    PrimitivePlotter& primitivePlotter = *m_ptrPrimitivePlotter;
    return primitivePlotter.save(fileName);
  } else {
    return "";
  }
}



void EventDataPlotter::clear()
{
  if (m_ptrPrimitivePlotter) {
    PrimitivePlotter& primitivePlotter = *m_ptrPrimitivePlotter;
    return primitivePlotter.clear();
  }
}


BoundingBox EventDataPlotter::getBoundingBox() const
{
  if (m_ptrPrimitivePlotter) {
    PrimitivePlotter& primitivePlotter = *m_ptrPrimitivePlotter;
    return primitivePlotter.getBoundingBox();
  } else {
    return BoundingBox(0, 0, 0, 0);
  }
}


void EventDataPlotter::setBoundingBox(const BoundingBox& boundingBox)
{
  if (m_ptrPrimitivePlotter) {
    PrimitivePlotter& primitivePlotter = *m_ptrPrimitivePlotter;
    return primitivePlotter.setBoundingBox(boundingBox);
  }
}


float EventDataPlotter::getCanvasWidth() const
{
  if (m_ptrPrimitivePlotter) {
    PrimitivePlotter& primitivePlotter = *m_ptrPrimitivePlotter;
    return primitivePlotter.getCanvasWidth();
  } else {
    return NAN;
  }
}


float EventDataPlotter::getCanvasHeight() const
{
  if (m_ptrPrimitivePlotter) {
    PrimitivePlotter& primitivePlotter = *m_ptrPrimitivePlotter;
    return primitivePlotter.getCanvasHeight();
  } else {
    return NAN;
  }
}

void EventDataPlotter::setCanvasWidth(const float& width)
{
  if (m_ptrPrimitivePlotter) {
    PrimitivePlotter& primitivePlotter = *m_ptrPrimitivePlotter;
    primitivePlotter.setCanvasWidth(width);
  }
}

void EventDataPlotter::setCanvasHeight(const float& height)
{
  if (m_ptrPrimitivePlotter) {
    PrimitivePlotter& primitivePlotter = *m_ptrPrimitivePlotter;
    primitivePlotter.setCanvasHeight(height);
  }

}


/// --------------------- Draw Circle2D ------------------------
void EventDataPlotter::draw(const Circle2D& circle,
                            AttributeMap attributeMap)
{
  if (not m_ptrPrimitivePlotter) return;
  PrimitivePlotter& primitivePlotter = *m_ptrPrimitivePlotter;

  float radius = circle.radius();

  AttributeMap defaultAttributeMap {
    {"stroke", "black"},
    {"stroke-width", std::to_string(radius)},
  };

  // Overwrite with the given values.
  attributeMap.insert(defaultAttributeMap.begin(), defaultAttributeMap.end());
  updateFillToStroke(attributeMap);

  const Vector2D& pos = circle.center();

  float x = pos.x();
  float y = pos.y();

  primitivePlotter.drawCircle(x, y, radius, attributeMap);
}


/// --------------------- Draw CDCWire ------------------------
void EventDataPlotter::draw(const CDCWire& wire, AttributeMap attributeMap)
{
  if (not m_ptrPrimitivePlotter) return;
  PrimitivePlotter& primitivePlotter = *m_ptrPrimitivePlotter;

  const float wireRadius = 0.1;
  const Vector2D& refPos = wire.getRefPos2D();

  draw(Circle2D(refPos, wireRadius), attributeMap);
}

/// --------------------- Draw CDCWireSuperLayer ------------------------
void EventDataPlotter::draw(const CDCWireSuperLayer& wireSuperLayer, AttributeMap attributeMap)
{
  if (not m_ptrPrimitivePlotter) return;
  PrimitivePlotter& primitivePlotter = *m_ptrPrimitivePlotter;

  AttributeMap defaultAttributeMap {
    {"stroke", "black"},
    {"stroke-width", "0.2"},
    {"fill", "none"},
  };

  // Add attributes if not present
  attributeMap.insert(defaultAttributeMap.begin(), defaultAttributeMap.end());

  float centerX = 0.0;
  float centerY = 0.0;

  float innerR = wireSuperLayer.getInnerPolarR();
  primitivePlotter.drawCircle(centerX, centerY, innerR, attributeMap);

  float outerR = wireSuperLayer.getOuterPolarR();
  primitivePlotter.drawCircle(centerX, centerY, outerR, attributeMap);
}


/// --------------------- Draw CDCSimHit ------------------------
void EventDataPlotter::draw(const CDCSimHit& simHit, AttributeMap attributeMap)
{
  if (not m_ptrPrimitivePlotter) return;
  PrimitivePlotter& primitivePlotter = *m_ptrPrimitivePlotter;

  // In case the event should be animated
  // uncover the group of elements at the time of flight of the CDCSimHit.
  if (m_animate) {
    float tof = simHit.getFlightTime();
    AttributeMap groupAttributeMap {
      {"_showAt", getAnimationTimeFromNanoSeconds(tof)}
    };
    primitivePlotter.startGroup(groupAttributeMap);

  } else {
    primitivePlotter.startGroup();
  }

  AttributeMap defaultAttributeMap {
    {"stroke", "yellow"},
    { "stroke-width", "0.02"}
  };

  // Add attributes if not present
  attributeMap.insert(defaultAttributeMap.begin(), defaultAttributeMap.end());
  updateFillToStroke(attributeMap);

  // Draw hit position as a small circle
  TVector3 position = simHit.getPosTrack();
  float x = position.X();
  float y = position.Y();
  float radius = 0.015;

  primitivePlotter.drawCircle(x, y, radius, attributeMap);

  // Draw momentum as an arrow proportional to the transverse component of the momentum
  const float momentumToArrowLength = 1.5;

  TVector3 momentum = simHit.getMomentum();
  float endX = x + momentum.X() * momentumToArrowLength;
  float endY = y + momentum.Y() * momentumToArrowLength;

  primitivePlotter.drawArrow(x, y, endX, endY, attributeMap);

  primitivePlotter.endGroup();

}

/// --------------------- Draw CDCWireHit ------------------------
void EventDataPlotter::draw(const CDCWireHit& wireHit, AttributeMap attributeMap)
{
  if (not m_ptrPrimitivePlotter) return;
  PrimitivePlotter& primitivePlotter = *m_ptrPrimitivePlotter;

  if (m_animate) {
    // Try to get the time of flight from the related CDCSimHit.
    const CDCHit* ptrHit = wireHit.getHit();
    float tof = getFlightTime(ptrHit);

    if (not std::isnan(tof)) {
      AttributeMap groupAttributeMap {
        {"_showAt", getAnimationTimeFromNanoSeconds(tof)}
      };
      primitivePlotter.startGroup(groupAttributeMap);

    } else {
      primitivePlotter.startGroup();

    }

  } else {
    primitivePlotter.startGroup();

  }

  const CDCWire& wire = wireHit.getWire();
  draw(wire, attributeMap);

  AttributeMap defaultAttributeMap {
    {"stroke", "black"},
    {"stroke-width", "0.02"},
    {"fill", "none"}
  };

  // Add attributes if not present
  attributeMap.insert(defaultAttributeMap.begin(), defaultAttributeMap.end());

  const Vector2D& refPos = wireHit.getRefPos2D();

  float x = refPos.x();
  float y = refPos.y();
  float radius = wireHit.getRefDriftLength();
  primitivePlotter.drawCircle(x, y, radius, attributeMap);

  primitivePlotter.endGroup();
}


/// --------------------- Draw CDCHit ------------------------
void EventDataPlotter::draw(const CDCHit& hit, AttributeMap attributeMap)
{
  CDCWireHit wireHit(&hit);
  draw(wireHit, attributeMap);
}
