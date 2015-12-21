/*
 * svgdrawing.cc
 *
 *  Created on: Feb 25, 2015
 *      Author: dschneider
 */

#include <tracking/trackFindingCDC/rootification/StoreWrappedObjPtr.h>
#include <tracking/trackFindingCDC/display/CDCSVGPlotter.h>

#include <cdc/dataobjects/CDCRecoHit.h>
#include <cdc/dataobjects/CDCRecoHit.h>
#include <genfit/AbsTrackRep.h>
#include <genfit/KalmanFitterInfo.h>
#include <genfit/MeasuredStateOnPlane.h>
#include <genfit/Track.h>
#include <tracking/trackFindingCDC/display/Colorizer.h>
#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectory2D.h>
#include <tracking/trackFindingCDC/geometry/Vector2D.h>
#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCRecoSegment2D.h>
#include <tracking/trackFindingCDC/filters/axialSegmentPair/MCAxialSegmentPairFilter.h>
#include <genfit/TrackCand.h>

#include <tracking/trackFindingCDC/mclookup/CDCMCSegmentLookUp.h>
#include <tracking/trackFindingCDC/eventdata/tracks/CDCSegmentTriple.h>
#include <cmath>

using genfit::AbsTrackRep;
using genfit::KalmanFitterInfo;
using genfit::MeasuredStateOnPlane;

using namespace Belle2;
using namespace TrackFindingCDC;

CDCSVGPlotter::CDCSVGPlotter(bool animate)
{
  int top = -112;
  int left = -112;
  int right = 112;
  int bottom = 112;

  TrackFindingCDC::BoundingBox default_bound(left, bottom, right, top);
  int default_width = 1120;
  int default_height = 1120;

  //Switch to indicating if an animated SVG should be generated
  m_animate = animate;
  EventDataPlotter m_eventdataPlotter(m_animate);

  m_eventdataPlotter.setBoundingBox(default_bound);
  m_eventdataPlotter.setCanvasHeight(default_height);
  m_eventdataPlotter.setCanvasWidth(default_width);
}

template<class AIterable, class AColorizer>
void CDCSVGPlotter::drawIterable(const AIterable& iterable, AColorizer& colorizer)
{
  unsigned int c_Item = 0;
  std::map<std::string, std::string>obj_styling;
  if (colorizer.isStrokeSet()) {
    if (!colorizer.isStrokeWidthSet()) {
      for (const auto& item : iterable) {
        obj_styling["stroke"] = colorizer.mapStroke(c_Item, item);
        draw(item, obj_styling);
        ++c_Item;
      }
    } else
      for (const auto& item : iterable) {
        obj_styling["stroke"] = colorizer.mapStroke(c_Item, item);
        obj_styling["stroke-width"] = colorizer.mapStrokeWidth(c_Item, item);

        draw(item, obj_styling);
        c_Item++;
      }
  } else
    for (const auto& item : iterable) {
      draw(item, obj_styling);
      ++c_Item;
    }
}

CDCSVGPlotter* CDCSVGPlotter::clone()
{
  return (new CDCSVGPlotter(*this));
}

void CDCSVGPlotter::drawOuterCDCWall(const std::string& stroke)
{
  std::map<std::string, std::string> attributeMap;
  attributeMap.insert(std::pair<std::string, std::string>("stroke", stroke));
  m_eventdataPlotter.drawOuterCDCWall(attributeMap);
}

void CDCSVGPlotter::drawInnerCDCWall(const std::string& stroke)
{
  std::map<std::string, std::string> attributeMap;
  attributeMap.insert(std::pair<std::string, std::string>("stroke", stroke));
  m_eventdataPlotter.drawInnerCDCWall(attributeMap);
}

void CDCSVGPlotter::drawInteractionPoint()
{
  m_eventdataPlotter.drawInteractionPoint();
}

void CDCSVGPlotter::drawCDCRecoSegments(const std::string& storeObjName, const std::string& stroke, const std::string&)
{
  StoreWrappedObjPtr<std::vector<CDCRecoSegment2D>> storedRecoSegments(storeObjName);
  drawStoreVector<CDCSegmentColorizer>(storedRecoSegments, storeObjName, stroke);
}

//used by drawCDCSimHitsConnectByToF as comparison tool
class comp {
public:
  bool operator()(const CDCSimHit* x, const CDCSimHit* y) const
  {
    return (x->getFlightTime() < y->getFlightTime());
  }
};

void CDCSVGPlotter::drawCDCSimHitsConnectByToF(const std::string& storeArrayName,
                                               const std::string& stroke,
                                               const std::string& strokeWidth)
{
  B2INFO("Drawing simulated hits connected by tof");
  StoreArray<CDCHit> cdcHitStoreArray(storeArrayName);
  if (cdcHitStoreArray) {
    std::vector<CDCSimHit*> simHitsRelatedToCDCHit;
    for (const CDCHit& cdcHit : cdcHitStoreArray) {
      simHitsRelatedToCDCHit.push_back(cdcHit.getRelated<CDCSimHit>("CDCSimHits"));
    }

    //    group them by their mcparticle id
    std::map<int, std::multiset<CDCSimHit*, comp>> simHitsByMcParticleId;
    for (CDCSimHit * const& simHit : simHitsRelatedToCDCHit) {
      MCParticle* mcParticle = simHit->getRelated<MCParticle>("MCParticles");
      if (mcParticle != nullptr) {
        int mcTrackId = mcParticle->getArrayIndex();
        simHitsByMcParticleId[mcTrackId].insert(simHit);
      }
    }

    std::map<std::string, std::string> stylingMap = {{"stroke", stroke}, {"stroke-width", strokeWidth}};

    for (const auto& simHitsByMcParticleIdIterator : simHitsByMcParticleId) {
      for (std::multiset<CDCSimHit*>::const_iterator it = (simHitsByMcParticleIdIterator.second).begin();
           it != --(simHitsByMcParticleIdIterator.second).end();) {
        CDCSimHit* fromSimHit = *it;
        ++it;
        CDCSimHit* toSimHit = *it;

        CDCHit* fromHit = fromSimHit->getRelated<CDCHit>(storeArrayName);
        CDCHit* toHit = toSimHit->getRelated<CDCHit>(storeArrayName);

        if (fromHit != nullptr || toHit != nullptr) {
          CDCWireHit fromWireHit(fromHit);
          CDCWireHit toWireHit(toHit);

          CDCRLTaggedWireHit fromRLWireHit(&fromWireHit, ERightLeft::c_Unknown);
          CDCRLTaggedWireHit toRLWireHit(&toWireHit, ERightLeft::c_Unknown);

          Vector3D fromDisplacement(fromSimHit->getPosTrack() - fromSimHit->getPosWire());
          Vector3D toDisplacement(toSimHit->getPosTrack() - toSimHit->getPosWire());

          CDCRecoHit2D fromRecoHit2D(fromRLWireHit, fromDisplacement.xy());
          CDCRecoHit2D toRecoHit2D(toRLWireHit, toDisplacement.xy());

          draw(fromRecoHit2D, stylingMap);
          draw(toRecoHit2D, stylingMap);

          const Vector2D fromPos = fromRecoHit2D.getRecoPos2D();
          const float fromX =  fromPos.x();
          const float fromY =  fromPos.y();

          const Vector2D toPos = toRecoHit2D.getRecoPos2D();
          const float toX =  toPos.x();
          const float toY =  toPos.y();

          m_eventdataPlotter.drawLine(fromX, fromY, toX, toY, stylingMap);
        }
      }
    }
  } else
    B2WARNING("StoreArray " << storeArrayName << " not present");
}

void CDCSVGPlotter::drawCDCSimHits(const std::string& storeArrayName, const std::string&, const std::string& strokeWidth)
{
  B2INFO("Drawing simulated hits");
  StoreArray<CDCHit> storeArray(storeArrayName);
  if (storeArray) {
    std::vector<CDCSimHit> simHitsRelatedToHits;
    for (const CDCHit& hit : storeArray) {
      simHitsRelatedToHits.push_back(*hit.getRelated<CDCSimHit>("CDCSimHits"));
    }
    B2INFO("#CDCSimHits: " << storeArray.getEntries());
    InputValueColorizer<CDCSimHit> colorizer;
    colorizer.setStroke("orange");
    colorizer.setStrokeWidth(strokeWidth);
    drawIterable(simHitsRelatedToHits, colorizer);
  } else
    B2WARNING("StoreArray " << storeArrayName << " not present");
}

void CDCSVGPlotter::drawCDCTracks(const std::string& storeObjName, const std::string&, const std::string&)
{
  StoreWrappedObjPtr<std::vector<CDCTrack>> storedCDCTracks(storeObjName);
  drawStoreVector<ListColorsColorizer<CDCTrack>>(storedCDCTracks, storeObjName);
}

void CDCSVGPlotter::drawGFTrackCandTrajectories(const std::string& storeArrayName, const std::string&, const std::string&)
{
  B2INFO("Drawing trajectories of the exported Genfit tracks");
  StoreArray<genfit::TrackCand> storeArray(storeArrayName);
  if (storeArray) {
    B2INFO("#Genfit Tracks: " << storeArray.getEntries());
    ListColorsColorizer<CDCTrajectory2D> colorizer;
    std::vector<CDCTrajectory2D> trajectories;
    for (const genfit::TrackCand& gfTrackCand : storeArray) {
      TVector3 tPosition = gfTrackCand.getPosSeed();
      TVector3 tMomentum = gfTrackCand.getMomSeed();
      double charge = gfTrackCand.getChargeSeed();

      Vector2D momentum(tMomentum.x(), tMomentum.Y());
      Vector2D position(tPosition.x(), tPosition.Y());

      CDCTrajectory2D trajectory(position, momentum, charge);
      trajectories.push_back(trajectory);
    }
    drawIterable(trajectories, colorizer);
  } else B2WARNING("No Genfit tracks present");
}

void CDCSVGPlotter::drawGFTracks(const std::string& storeArrayName, const std::string&, const std::string&)
{
  StoreArray<genfit::Track> storeArray(storeArrayName);
  drawStoreArray<ListColorsColorizer<genfit::Track>>(storeArray, storeArrayName);
}

void CDCSVGPlotter::drawGFTrackTrajectories(const std::string& storeArrayName, const std::string&, const std::string&)
{
  B2INFO("Drawing trajectories of the CDCTracks");
  StoreArray<genfit::Track> cdcTracks(storeArrayName);
  if (cdcTracks) {
    int iGFTrack = 0;
    std::map<std::string, std::string> attributeMap;
    ListColorsColorizer<genfit::Track> colorizer;
    for (const genfit::Track& gfTrack : cdcTracks) {
      attributeMap["stroke"] = colorizer.mapStroke(iGFTrack, gfTrack);
      ++iGFTrack;

      StoreArray<CDCHit> storedHits;
      std::vector<genfit::TrackPoint*> gfTrackPoints = gfTrack.getPoints();
      AbsTrackRep* absTrackRep = gfTrack.getCardinalRep();

      std::vector<std::vector<CDCRecoHit*>> recoHits;

      const MeasuredStateOnPlane* fittedState(NULL);

      float x = nanf("");
      float y = nanf("");

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

        TVector3 track_pos = absTrackRep->getPos(*fittedState);

        if (!isnan(x))
          m_eventdataPlotter.drawLine(x, y, track_pos.x(), track_pos.y(), attributeMap);
        x = track_pos.x();
        y = track_pos.y();
        ++iTrackPoint;
      }// for (genfit::TrackPoint* gfTrackPoint : gfTrackPoints)

    }
  } else {
    B2WARNING(storeArrayName << " not present in the DataStore");
    B2INFO("Current content of the DataStore:");
    B2INFO("StoreArrays:");
    for (auto n : DataStore::Instance().getListOfArrays(TObject::Class(), DataStore::EDurability(0)))
      B2INFO(n);
    B2INFO("");
    B2INFO("StoreObjPtr:");
    for (auto n : DataStore::Instance().getListOfObjects(TObject::Class(), DataStore::EDurability(0)))
      B2INFO(n);
  }
}

void CDCSVGPlotter::drawGFTrackCands(const std::string& storeArrayName, const std::string&, const std::string&)
{
  StoreArray<genfit::TrackCand> storeArray(storeArrayName);
  drawStoreArray<ListColorsColorizer<genfit::TrackCand>>(storeArray, storeArrayName);
}

void CDCSVGPlotter::drawCDCHits(const std::string& storeArrayName, const std::string& stroke, const std::string& strokeWidth)
{
  StoreArray<CDCHit> storeArray(storeArrayName);
  drawStoreArray<CDCHitColorizer>(storeArray, storeArrayName, stroke, strokeWidth);
}

void CDCSVGPlotter::drawMCAxialAxialPairs(const std::string& storeObjName, const std::string& stroke, const std::string&)
{
  B2INFO("Draw axial to axial segment pairs");

  StoreWrappedObjPtr<std::vector<CDCRecoSegment2D>> storedRecoSegments(storeObjName);
  if (storedRecoSegments) {
    std::vector<CDCRecoSegment2D>& recoSegments = *storedRecoSegments;

    B2INFO("#Segments: " << recoSegments.size());

    std::vector<CDCAxialRecoSegment2D> axialSegments;
    for (const CDCAxialRecoSegment2D& segment : recoSegments) {
      if (segment.getStereoKind() == EStereoKind::c_Axial) axialSegments.push_back(segment);
    }

    std::vector<CDCAxialSegmentPair> axialAxialSegmentPairs;
    for (const CDCAxialRecoSegment2D& startSegment : axialSegments) {
      for (const CDCAxialRecoSegment2D& endSegment : axialSegments) {
        axialAxialSegmentPairs.push_back(CDCAxialSegmentPair(&startSegment, &endSegment));
      }
    }

//    B2INFO("#unchecked pairs" << axialAxialSegmentPairs.size());

    std::vector<CDCAxialSegmentPair> goodAxialAxialSegmentPairs;
    MCAxialSegmentPairFilter mcAxialAxialSegmentPairFilter;

    for (const CDCAxialSegmentPair& pair : axialAxialSegmentPairs) {
      if (not isNotACell(mcAxialAxialSegmentPairFilter(pair))) goodAxialAxialSegmentPairs.push_back(pair);
    }

    B2WARNING("draw_mcaxialaxialpairs does not work in cpp yet (isgoodpair-check needed)");
    B2INFO("#Pairs: " << goodAxialAxialSegmentPairs.size());

    InputValueColorizer<CDCAxialSegmentPair> colorizer;
    colorizer.setStroke(stroke);
    drawIterable(goodAxialAxialSegmentPairs, colorizer);
  } else
    B2INFO(storeObjName << "does not exist in current DataStore");
}

void CDCSVGPlotter::drawAxialStereoPairs(const std::string& storeObjName, const std::string& stroke, const std::string&)
{
  StoreWrappedObjPtr<std::vector<CDCSegmentPair>> storedCDCAxialStereoSegmentPairs(storeObjName);
  drawStoreVector<InputValueColorizer<CDCSegmentPair>>(storedCDCAxialStereoSegmentPairs, storeObjName, stroke);
}

void CDCSVGPlotter::drawMCSegmentTriples(const std::string& storeObjName, const std::string& stroke, const std::string&)
{
  B2INFO("Draw axial to axial segment pairs");
  StoreWrappedObjPtr<std::vector<CDCRecoSegment2D>> storedRecoSegments(storeObjName);
  if (storedRecoSegments) {
    std::vector<CDCRecoSegment2D>& recoSegments = *storedRecoSegments;
    B2INFO("#Segment " << recoSegments.size());

    std::vector<CDCAxialRecoSegment2D> axialSegments;
    std::vector<CDCStereoRecoSegment2D> stereoSegments;
    for (const CDCRecoSegment2D& segment : recoSegments) {
      if (segment.getStereoKind() == EStereoKind::c_Axial) axialSegments.push_back(segment);
      else stereoSegments.push_back(segment);
    }

    MCAxialSegmentPairFilter mcAxialAxialSegmentPairFilter;
    CDCMCSegmentLookUp mcSegmentLookup;

    std::vector<CDCSegmentTriple> segmentTriples;
    for (const CDCAxialRecoSegment2D& startSegment : axialSegments) {
      for (const CDCStereoRecoSegment2D& middleSegment : stereoSegments) {
        for (const CDCAxialRecoSegment2D& endSegment : axialSegments) {
          segmentTriples.push_back(CDCSegmentTriple(&startSegment, &middleSegment, &endSegment));
        }
      }
    }
    //isgoodtriple
    B2WARNING("draw_mcaxialaxialpairs does not work in cpp yet (isgoodtriple-check needed)");
    return;

    std::vector<CDCSegmentTriple> goodSegmentTriples;
    for (const CDCSegmentTriple& triple : segmentTriples) {
      //needs check for isgoodtriple
      if (true) goodSegmentTriples.push_back(triple);
    }
    B2INFO("#Triples: " << goodSegmentTriples.size());

    InputValueColorizer<CDCAxialSegmentPair> colorizer;
    colorizer.setStroke(stroke);
    drawIterable(goodSegmentTriples, colorizer);
  } else
    B2INFO(storeObjName << "does not exist in current DataStore");
}

void CDCSVGPlotter::drawAxialStereoSegmentPairs(const std::string& storeObjName, const std::string&, const std::string&)
{
  StoreWrappedObjPtr<std::vector<CDCSegmentPair>> storedCDCAxialStereoSegmentPairs(storeObjName);
  drawStoreVector<ListColorsColorizer<CDCSegmentPair>>(storedCDCAxialStereoSegmentPairs, storeObjName);
}

void CDCSVGPlotter::drawCDCWireHitClusters(const std::string& storeObjName, const std::string& stroke,
                                           const std::string&)
{
  StoreWrappedObjPtr<std::vector<CDCWireHitCluster>> storedCDCWireHitClusters(storeObjName);
  drawStoreVector<ListColorsColorizer<CDCWireHitCluster>>(storedCDCWireHitClusters, storeObjName, stroke);
}

void CDCSVGPlotter::drawSegmentTrajectories(const std::string& storeObjName, const std::string&,
                                            const std::string&)
{
  B2INFO("Drawing the fits to the selected RecoHit2DSegments");

  StoreWrappedObjPtr<std::vector<CDCRecoSegment2D>> storedRecoSegments(storeObjName);
  if (storedRecoSegments) {
    std::vector<CDCRecoSegment2D>& recoSegments = *storedRecoSegments;

    B2INFO("#2D Trajectories " << recoSegments.size() << "from segments");

    std::vector<CDCTrajectory2D> iterTrajectories;

    for (auto segment : recoSegments) {
      iterTrajectories.push_back(segment.getTrajectory2D());
    }

    InputValueColorizer<CDCTrajectory2D> colorizer;
    drawIterable(iterTrajectories, colorizer);
  } else
    B2INFO(storeObjName << "does not exist in current DataStore");
}

template<class AObject>
void CDCSVGPlotter::draw(const AObject& obj, const std::map<std::string, std::string>& obj_attributes)
{
  m_eventdataPlotter.draw(obj, obj_attributes);
}

void CDCSVGPlotter::drawWires(const CDCWireTopology& cdcWireTopology)
{
  std::map<std::string, std::string> placeholder;
  m_eventdataPlotter.draw(cdcWireTopology, placeholder);
}

const std::string CDCSVGPlotter::saveSVGFile(const std::string& svgFileName)
{
  TrackFindingCDC::BoundingBox boundingBox = m_eventdataPlotter.getBoundingBox();

  float height = boundingBox.getHeight();
  float width = boundingBox.getWidth();

  int totalPoints = 1120 * 1120;
  float svgHeight = roundf(sqrt(totalPoints * height / width));
  float svgWidth = roundf(sqrt(totalPoints * width / height));

  m_eventdataPlotter.setCanvasHeight(svgHeight);
  m_eventdataPlotter.setCanvasWidth(svgWidth);

  return (m_eventdataPlotter.save(svgFileName));
}

template<class AColorizer, class AObject>
void CDCSVGPlotter::drawStoreArray(const StoreArray<AObject>& storeArray, const std::string& storeArrayName,
                                   const std::string& stroke,
                                   const std::string& strokeWidth)
{
  B2INFO("Drawing StoreArray: " << storeArrayName);
  if (storeArray) {
    B2INFO("with " << storeArray.getEntries() << " entries");
    AColorizer colorizer;
    B2INFO("Attributes are");
    if (stroke != "")
      colorizer.setStroke(stroke);
    if (colorizer.isStrokeSet())
      B2INFO("stroke: " << colorizer.printStrokeAttribute());
    if (strokeWidth != "")
      colorizer.setStrokeWidth(strokeWidth);
    if (colorizer.isStrokeWidthSet())
      B2INFO("stroke-width: " << colorizer.printStrokeWidthAttribute());
    drawIterable(storeArray, colorizer);
  } else {
    B2WARNING(storeArrayName << " not present in the DataStore");
    B2INFO("Current content of the DataStore:");
    B2INFO("StoreArrays:");
    for (auto n : DataStore::Instance().getListOfArrays(TObject::Class(), DataStore::EDurability(0)))
      B2INFO(n);
    B2INFO("");
    B2INFO("StoreObjPtr:");
    for (auto n : DataStore::Instance().getListOfObjects(TObject::Class(), DataStore::EDurability(0)))
      B2INFO(n);
  }
}

template<class AColorizer, class AItem>
void CDCSVGPlotter::drawStoreVector(const StoreWrappedObjPtr<std::vector<AItem>>& storeVector, const std::string& storeObjName,
                                    const std::string& stroke,
                                    const std::string& strokeWidth)
{
  B2INFO("Drawing vector from DataStore: " << storeObjName);
  if (storeVector) {
    std::vector<AItem>& vector = *storeVector;
    B2INFO("with " << vector.size() << " entries");
    AColorizer colorizer;
    B2INFO("Attributes are");
    if (stroke != "")
      colorizer.setStroke(stroke);
    if (colorizer.isStrokeSet())
      B2INFO("stroke: " << colorizer.printStrokeAttribute());
    if (strokeWidth != "")
      colorizer.setStrokeWidth(strokeWidth);
    if (colorizer.isStrokeWidthSet())
      B2INFO("stroke-width: " << colorizer.printStrokeWidthAttribute());
    drawIterable(vector, colorizer);
  } else {
    B2WARNING(storeObjName << " not present in the DataStore");
    B2INFO("Current content of the DataStore:");
    B2INFO("StoreArrays:");
    for (auto n : DataStore::Instance().getListOfArrays(TObject::Class(), DataStore::EDurability(0)))
      B2INFO(n);
    B2INFO("");
    B2INFO("StoreObjPtr:");
    for (auto n : DataStore::Instance().getListOfObjects(TObject::Class(), DataStore::EDurability(0)))
      B2INFO(n);
  }
}

