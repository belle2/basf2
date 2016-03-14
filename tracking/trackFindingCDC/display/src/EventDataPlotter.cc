/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackFindingCDC/display/EventDataPlotter.h>

#include <tracking/trackFindingCDC/display/SVGPrimitivePlotter.h>


#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>
#include <genfit/AbsFitterInfo.h>

#include <genfit/AbsMeasurement.h>
#include <genfit/AbsTrackRep.h>
#include <genfit/GblFitterInfo.h>

#include <genfit/KalmanFitterInfo.h>
#include <genfit/MeasuredStateOnPlane.h>
#include <genfit/Track.h>
#include <genfit/TrackCand.h>
#include <cdc/dataobjects/CDCRecoHit.h>

#include <cmath>
#include <exception>

using genfit::AbsFitterInfo;
using genfit::AbsTrackRep;
using genfit::GblFitterInfo;
using genfit::KalmanFitterInfo;
using genfit::MeasuredStateOnPlane;

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

EventDataPlotter::EventDataPlotter(bool animate) :
  m_ptrPrimitivePlotter(new SVGPrimitivePlotter(AttributeMap {
  {"stroke", "orange"},
  {"stroke-width", "0.55"},
  {"fill", "none"}
})),
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


void EventDataPlotter::startAnimationGroup(const Belle2::CDCSimHit& simHit)
{
  // In case the event should be animated
  // uncover the group of elements at the time of flight of the CDCSimHit.
  if (not m_ptrPrimitivePlotter) return;
  PrimitivePlotter& primitivePlotter = *m_ptrPrimitivePlotter;

  if (m_animate) {
    float tof = simHit.getFlightTime();
    AttributeMap groupAttributeMap {
      {"_showAt", getAnimationTimeFromNanoSeconds(tof)}
    };
    primitivePlotter.startGroup(groupAttributeMap);

  } else {
    primitivePlotter.startGroup();
  }
}

void EventDataPlotter::startAnimationGroup(const Belle2::CDCHit* ptrHit)
{
  if (not m_ptrPrimitivePlotter) return;
  PrimitivePlotter& primitivePlotter = *m_ptrPrimitivePlotter;

  if (m_animate) {
    if (ptrHit) {
      const CDCHit& hit = *ptrHit;
      const CDCSimHit* ptrSimHit = hit.getRelated<CDCSimHit>();
      if (ptrSimHit) {
        const CDCSimHit& simHit = *ptrSimHit;
        startAnimationGroup(simHit);
        return;
      }
    }
  }
  primitivePlotter.startGroup();
}

void EventDataPlotter::drawInteractionPoint()
{
  Vector2D center(0.0, 0.0);
  float radius = 1.0;

  const Circle2D interactionPoint(center, radius);

  AttributeMap attributeMap {
    { "fill" , "black"},
    {"stroke-width" , "0"}
  };

  draw(interactionPoint, attributeMap);
}

void EventDataPlotter::drawInnerCDCWall(const AttributeMap& attributeMap)
{
  if (not m_ptrPrimitivePlotter) return;
  PrimitivePlotter& primitivePlotter = *m_ptrPrimitivePlotter;

  const CDCWireTopology& wireTopology = CDCWireTopology::getInstance();

  const CDCWireSuperLayer& wireSuperLayer = wireTopology.getWireSuperLayers().front();

  float centerX = 0.0;
  float centerY = 0.0;
  float innerR = wireSuperLayer.getInnerCylindricalR();

  primitivePlotter.drawCircle(centerX, centerY, innerR, attributeMap);
}

void EventDataPlotter::drawOuterCDCWall(const AttributeMap& attributeMap)
{
  if (not m_ptrPrimitivePlotter) return;
  PrimitivePlotter& primitivePlotter = *m_ptrPrimitivePlotter;

  const CDCWireTopology& wireTopology = CDCWireTopology::getInstance();

  const CDCWireSuperLayer& wireSuperLayer = wireTopology.getWireSuperLayers().back();

  float centerX = 0.0;
  float centerY = 0.0;
  float outerR = wireSuperLayer.getOuterCylindricalR();

  primitivePlotter.drawCircle(centerX, centerY, outerR, attributeMap);
}

void EventDataPlotter::drawLine(const float& startX, const float& startY, const float& endX, const float& endY,
                                const AttributeMap& attributeMap)
{
  if (not m_ptrPrimitivePlotter) return;
  PrimitivePlotter& primitivePlotter = *m_ptrPrimitivePlotter;

  primitivePlotter.drawLine(startX, startY, endX, endY, attributeMap);
}
/// --------------------- Draw Circle2D ------------------------
void EventDataPlotter::draw(const Circle2D& circle,
                            AttributeMap attributeMap)
{
  if (not m_ptrPrimitivePlotter) return;
  PrimitivePlotter& primitivePlotter = *m_ptrPrimitivePlotter;

  float radius = circle.radius();

  if (not attributeMap.count("fill") or attributeMap["fill"] != "") {
    if (attributeMap.count("stroke")) {
      attributeMap["fill"] = attributeMap["stroke"];
      attributeMap.erase("stroke");
    }
  }

  const Vector2D& pos = circle.center();

  float x = pos.x();
  float y = pos.y();

  primitivePlotter.drawCircle(x, y, radius, attributeMap);
}

/// --------------------- Draw CDCWire ------------------------
void EventDataPlotter::draw(const CDCWire& wire, const AttributeMap& attributeMap)
{
  const float wireRadius = 0.25;
  const Vector2D& refPos = wire.getRefPos2D();

  draw(Circle2D(refPos, wireRadius), attributeMap);
}

/// --------------------- Draw CDCWireSuperLayer ------------------------
void EventDataPlotter::draw(const CDCWireSuperLayer& wireSuperLayer, const AttributeMap& attributeMap)
{
  if (not m_ptrPrimitivePlotter) return;
  PrimitivePlotter& primitivePlotter = *m_ptrPrimitivePlotter;

  primitivePlotter.startGroup(attributeMap);
  for (const CDCWireLayer& wireLayer : wireSuperLayer) {
    for (const CDCWire& wire : wireLayer) {
      draw(wire);
    }
  }
  primitivePlotter.endGroup();

}

/// --------------------- Draw CDCWireTopology------------------------
void EventDataPlotter::draw(const CDCWireTopology& wireTopology, AttributeMap attributeMap)
{
  for (const CDCWireSuperLayer& wireSuperLayer : wireTopology.getWireSuperLayers()) {
    AttributeMap defaultSuperLayerAttributeMap {
      {"fill" , wireSuperLayer.isAxial() ? "black" : "gray"},
      {"stroke" , "none"}
    };

    AttributeMap superLayerAttributeMap(attributeMap);

    // Insert the values as defaults. Does not overwrite attributes with the same name.
    superLayerAttributeMap.insert(defaultSuperLayerAttributeMap.begin(), defaultSuperLayerAttributeMap.end());
    draw(wireSuperLayer, superLayerAttributeMap);
  }
}


/// --------------------- Draw CDCSimHit ------------------------
void EventDataPlotter::draw(const CDCSimHit& simHit, const AttributeMap& attributeMap)
{
  if (not m_ptrPrimitivePlotter) return;
  PrimitivePlotter& primitivePlotter = *m_ptrPrimitivePlotter;

  startAnimationGroup(simHit);

  // Draw hit position as a small circle
  TVector3 position = simHit.getPosTrack();
  float x = position.X();
  float y = position.Y();
  float radius = 0.2;

  primitivePlotter.drawCircle(x, y, radius, attributeMap);

  // Draw momentum as an arrow proportional to the transverse component of the momentum
  const float momentumToArrowLength = 1.5;

  TVector3 momentum = simHit.getMomentum();
  float endX = x + momentum.X() * momentumToArrowLength;
  float endY = y + momentum.Y() * momentumToArrowLength;

  primitivePlotter.drawArrow(x, y, endX, endY, attributeMap);

  primitivePlotter.endGroup();

}

/// --------------------- Draw CDCHit ------------------------
void EventDataPlotter::draw(const CDCHit& hit, const AttributeMap& attributeMap)
{
  CDCWireHit wireHit(&hit);
  draw(wireHit, attributeMap);
}

/// --------------------- Draw CDCWireHit ------------------------
void EventDataPlotter::draw(const CDCWireHit& wireHit, const AttributeMap& attributeMap)
{
  if (not m_ptrPrimitivePlotter) return;
  PrimitivePlotter& primitivePlotter = *m_ptrPrimitivePlotter;

  startAnimationGroup(wireHit.getHit());

  const Vector2D& refPos = wireHit.getRefPos2D();

  float x = refPos.x();
  float y = refPos.y();
  float radius = wireHit.getRefDriftLength();

  if (fabs(radius) < 100) {
    primitivePlotter.drawCircle(x, y, radius, attributeMap);
  }

  primitivePlotter.endGroup();
}

/// --------------------- Draw CDCRecoHit2D ------------------------
void EventDataPlotter::draw(const CDCRecoHit2D& recoHit2D, const AttributeMap& attributeMap)
{
  if (not m_ptrPrimitivePlotter) return;
  PrimitivePlotter& primitivePlotter = *m_ptrPrimitivePlotter;

  const CDCWireHit& wireHit = recoHit2D.getWireHit();

  startAnimationGroup(wireHit.getHit());

  const Vector2D& refPos2D = wireHit.getRefPos2D();
  const Vector2D& recoPos2D = recoHit2D.getRecoPos2D();

  float x = refPos2D.x();
  float y = refPos2D.y();
  float radius = wireHit.getRefDriftLength();
  primitivePlotter.drawCircle(x, y, radius, attributeMap);

  float supportPointRadius = 0.2;
  Circle2D supportPoint(recoPos2D, supportPointRadius);
  draw(supportPoint, attributeMap);

  primitivePlotter.endGroup();
}


/// --------------------- Draw CDCRecoHit2D ------------------------
void EventDataPlotter::draw(const CDCTangent& tangent, const AttributeMap& attributeMap)
{
  if (not m_ptrPrimitivePlotter) return;
  PrimitivePlotter& primitivePlotter = *m_ptrPrimitivePlotter;

  const Vector2D fromPos = tangent.getFromRecoPos2D();
  const float fromX =  fromPos.x();
  const float fromY =  fromPos.y();

  const Vector2D toPos = tangent.getToRecoPos2D();
  const float toX =  toPos.x();
  const float toY =  toPos.y();

  primitivePlotter.drawLine(fromX, fromY, toX, toY, attributeMap);

  float touchPointRadius = 0.015;
  const Circle2D fromTouchPoint(fromPos, touchPointRadius);
  draw(fromTouchPoint, attributeMap);

  const Circle2D toTouchPoint(toPos, touchPointRadius);
  draw(toTouchPoint, attributeMap);
}

/// --------------------- Draw CDCTrajectory2D ------------------------
void EventDataPlotter::draw(const CDCTrajectory2D& trajectory2D, AttributeMap attributeMap)
{
  if (not m_ptrPrimitivePlotter) return;
  PrimitivePlotter& primitivePlotter = *m_ptrPrimitivePlotter;

  AttributeMap defaultAttributeMap {};

  // Make the default color charge dependent
  int charge = trajectory2D.getChargeSign();
  if (charge > 0) {
    defaultAttributeMap["stroke"] = "red";
  } else if (charge < 0) {
    defaultAttributeMap["stroke"] = "blue";
  } else {
    defaultAttributeMap["stroke"] = "green";
  }

  // Add attributes if not present
  attributeMap.insert(defaultAttributeMap.begin(), defaultAttributeMap.end());

  if (trajectory2D.getLocalCircle().isCircle()) {

    const float radius = trajectory2D.getLocalCircle().absRadius();

    const Vector2D trajectoryExit = trajectory2D.getExit();
    const float& endX = trajectoryExit.x();
    const float& endY = trajectoryExit.y();

    if (trajectoryExit.hasNAN()) {
      // No exit point out of the cdc could be detected.
      // Draw full circle

      const Vector2D center = trajectory2D.getGlobalCircle().center();
      const float radius = trajectory2D.getLocalCircle().absRadius();
      const float& centerX = center.x();
      const float& centerY = center.y();

      primitivePlotter.drawCircle(centerX, centerY, radius);


    } else {

      const Vector2D start = trajectory2D.getSupport();
      const float& startX = start.x();
      const float& startY = start.y();

      const int curvature = -charge;
      const bool sweepFlag = curvature > 0;

      // check if exit point is on the close or
      // on the far side of the circle
      const bool longArc = (trajectory2D.calcPerpS(trajectoryExit) > 0) ? false : true;
      primitivePlotter.drawCircleArc(startX,
                                     startY,
                                     endX,
                                     endY,
                                     radius,
                                     longArc,
                                     sweepFlag,
                                     attributeMap);
    }
  } else {
    // trajectory is a straight line
    const Vector2D start = trajectory2D.getSupport();
    const float& startX = start.x();
    const float& startY = start.y();

    const Vector2D trajectoryExit = trajectory2D.getExit();
    const float endX = trajectoryExit.x();
    const float endY = trajectoryExit.y();

    if (trajectoryExit.hasNAN()) {
      B2WARNING("Could not compute point off exit in a straight line case.");
    } else {
      primitivePlotter.drawLine(startX, startY, endX, endY, attributeMap);
    }
  }

}

void EventDataPlotter::draw(const CDCAxialSegmentPair& axialSegmentPair,
                            const AttributeMap& attributeMap)
{
  if (not m_ptrPrimitivePlotter) return;
  PrimitivePlotter& primitivePlotter = *m_ptrPrimitivePlotter;

  const CDCRecoSegment2D* ptrFromSegment = axialSegmentPair.getStart();
  const CDCRecoSegment2D* ptrToSegment = axialSegmentPair.getEnd();

  if (not ptrFromSegment or not ptrToSegment) return;

  const CDCRecoSegment2D& fromSegment = *ptrFromSegment;
  const CDCRecoSegment2D& toSegment = *ptrToSegment;

  const Vector2D& fromPos = fromSegment.back().getWire().getRefPos2D();
  const Vector2D& toPos = toSegment.front().getWire().getRefPos2D();

  if (fromPos.hasNAN()) {
    B2WARNING("Center of mass of first segment in a pair contains NAN values.");
    return;
  }

  if (toPos.hasNAN()) {
    B2WARNING("Center of mass of second segment in a pair contains NAN values.");
    return;
  }

  const float fromX = fromPos.x();
  const float fromY = fromPos.y();

  const float toX = toPos.x();
  const float toY = toPos.y();

  primitivePlotter.drawArrow(fromX, fromY, toX, toY, attributeMap);

}

void EventDataPlotter::draw(const CDCSegmentPair& segmentPair,
                            const AttributeMap& attributeMap)
{
  if (not m_ptrPrimitivePlotter) return;
  PrimitivePlotter& primitivePlotter = *m_ptrPrimitivePlotter;

  const CDCRecoSegment2D* ptrFromSegment = segmentPair.getStartSegment();
  const CDCRecoSegment2D* ptrToSegment = segmentPair.getEndSegment();

  if (not ptrFromSegment or not ptrToSegment) return;

  const CDCRecoSegment2D& fromSegment = *ptrFromSegment;
  const CDCRecoSegment2D& toSegment = *ptrToSegment;

  const Vector2D& fromPos = fromSegment.back().getWire().getRefPos2D();
  const Vector2D& toPos = toSegment.front().getWire().getRefPos2D();

  if (fromPos.hasNAN()) {
    B2WARNING("Center of mass of first segment in a pair contains NAN values.");
    return;
  }

  if (toPos.hasNAN()) {
    B2WARNING("Center of mass of second segment in a pair contains NAN values.");
    return;
  }

  const float fromX = fromPos.x();
  const float fromY = fromPos.y();

  const float toX = toPos.x();
  const float toY = toPos.y();

  primitivePlotter.drawArrow(fromX, fromY, toX, toY, attributeMap);
}

void EventDataPlotter::draw(const CDCSegmentTriple& segmentTriple,
                            const AttributeMap& attributeMap)
{
  if (not m_ptrPrimitivePlotter) return;
  PrimitivePlotter& primitivePlotter = *m_ptrPrimitivePlotter;

  const CDCRecoSegment2D* ptrStartSegment = segmentTriple.getStart();
  const CDCRecoSegment2D* ptrMiddleSegment = segmentTriple.getMiddle();
  const CDCRecoSegment2D* ptrEndSegment = segmentTriple.getEnd();

  if (not ptrStartSegment or not ptrMiddleSegment or not ptrEndSegment) return;

  const CDCRecoSegment2D& startSegment = *ptrStartSegment;
  const CDCRecoSegment2D& middleSegment = *ptrMiddleSegment;
  const CDCRecoSegment2D& endSegment = *ptrEndSegment;

  const Vector2D& startBackPos2D = startSegment.back().getRefPos2D();
  const Vector2D& middleFrontPos2D = middleSegment.front().getRefPos2D();
  const Vector2D& middleBackPos2D = middleSegment.back().getRefPos2D();
  const Vector2D& endFrontPos2D = endSegment.front().getRefPos2D();

  if (startBackPos2D.hasNAN()) {
    B2WARNING("Back position of start segment in a triple contains NAN values.");
    return;
  }

  if (middleFrontPos2D.hasNAN()) {
    B2WARNING("Front position of middle segment in a triple contains NAN values.");
    return;
  }

  if (middleBackPos2D.hasNAN()) {
    B2WARNING("Back position of middle segment in a triple contains NAN values.");
    return;
  }

  if (endFrontPos2D.hasNAN()) {
    B2WARNING("Front position of end segment in a triple contains NAN values.");
    return;
  }

  const float startBackX = startBackPos2D.x();
  const float startBackY = startBackPos2D.y();

  const float middleFrontX = middleFrontPos2D.x();
  const float middleFrontY = middleFrontPos2D.y();

  primitivePlotter.drawArrow(startBackX, startBackY, middleFrontX, middleFrontY, attributeMap);

  const float middleBackX = middleBackPos2D.x();
  const float middleBackY = middleBackPos2D.y();

  const float endFrontX = endFrontPos2D.x();
  const float endFrontY = endFrontPos2D.y();

  primitivePlotter.drawArrow(middleBackX, middleBackY, endFrontX, endFrontY, attributeMap);
}

void EventDataPlotter::draw(const genfit::TrackCand& gfTrackCand, const AttributeMap& attributeMap)
{
  StoreArray<CDCHit> storedHits;
  auto hitIDs = gfTrackCand.getHitIDs(Const::CDC);

  if (not m_ptrPrimitivePlotter) return;
  PrimitivePlotter& primitivePlotter = *m_ptrPrimitivePlotter;

  primitivePlotter.startGroup(attributeMap);

  for (int hitID : hitIDs) {
    const CDCHit* ptrHit = storedHits[hitID];
    if (ptrHit) {
      const CDCHit& hit = *ptrHit;
      draw(hit);
    }
  }

  primitivePlotter.endGroup();

}

void EventDataPlotter::draw(const genfit::Track& gfTrack, const AttributeMap& attributeMap)
{
  StoreArray<CDCHit> storedHits;
  std::vector<genfit::TrackPoint*> gfTrackPoints = gfTrack.getPoints();
  AbsTrackRep* absTrackRep = gfTrack.getCardinalRep();

  std::vector<std::vector<CDCRecoHit*>> recoHits;

  const MeasuredStateOnPlane* fittedState(NULL);

  int iTrackPoint = 0;
  for (genfit::TrackPoint* gfTrackPoint : gfTrackPoints) {
    recoHits.push_back({});

    std::vector< genfit::AbsMeasurement* > absMeasurements = gfTrackPoint->getRawMeasurements();

    for (auto absMeasurement : absMeasurements) {
      recoHits[iTrackPoint].push_back(dynamic_cast<CDCRecoHit*>(absMeasurement));
    }

    // get the fitter infos ------------------------------------------------------------------
    if (! gfTrackPoint->hasFitterInfo(absTrackRep)) {
      B2ERROR("trackPoint has no fitterInfo for absTrackRep");
      continue;
    }

    KalmanFitterInfo* kalmanFitterInfo = gfTrackPoint->getKalmanFitterInfo(absTrackRep);

    if (!kalmanFitterInfo) {
      B2ERROR("Can only display KalmanFitterInfo or GblFitterInfo");
      continue;
    }

    if (kalmanFitterInfo && ! gfTrackPoint->hasRawMeasurements()) {
      B2ERROR("trackPoint has no raw measurements");
      continue;
    }

    if (kalmanFitterInfo && ! kalmanFitterInfo->hasPredictionsAndUpdates()) {
      B2ERROR("KalmanFitterInfo does not have all predictions and updates");
      continue;
    }

    try {
      if (kalmanFitterInfo)
        fittedState = &(kalmanFitterInfo->getFittedState(true));
    } catch (std::exception& e) {
      B2ERROR(e.what() << " - can not get fitted state");
      continue;
    }

    genfit::MeasurementOnPlane* mop = kalmanFitterInfo->getMeasurementOnPlane(0);
    const TVectorD& hit_coords = mop->getState();
    double driftLenght = std::fabs(hit_coords(0));

    B2Vector3D pointingVector;
    B2Vector3D trackDir;

    // 0 = index of AbsMeasurement
    recoHits[iTrackPoint][0]->getFlyByDistanceVector(pointingVector, trackDir, absTrackRep, true);

    TVector3 track_pos = absTrackRep->getPos(*fittedState);

    PrimitivePlotter& primitivePlotter = *m_ptrPrimitivePlotter;

    float x = track_pos.x() - pointingVector.X();
    float y = track_pos.y() - pointingVector.Y();
    float radius = driftLenght;
    primitivePlotter.startGroup();
    primitivePlotter.drawCircle(x, y, radius, attributeMap);

    primitivePlotter.endGroup();
    ++iTrackPoint;
  }// for (genfit::TrackPoint* gfTrackPoint : gfTrackPoints)
}
