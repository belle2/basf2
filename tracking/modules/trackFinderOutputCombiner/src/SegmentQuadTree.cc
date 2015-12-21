#include <tracking/modules/trackFinderOutputCombiner/SegmentQuadTree.h>

#include <tracking/trackFindingCDC/rootification/StoreWrappedObjPtr.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCRecoSegment2D.h>
#include <tracking/trackFindingCDC/legendre/quadtree/QuadTreeProcessorImplementation.h>
#include <tracking/trackFindingCDC/fitting/CDCObservations2D.h>
#include <tracking/trackFindingCDC/fitting/CDCRiemannFitter.h>

#include <TFile.h>
#include <TH2F.h>

using namespace Belle2;
using namespace TrackFindingCDC;

REG_MODULE(SegmentQuadTree);

void SegmentQuadTreeModule::addHitToTrack(CDCTrack& track, const CDCRecoHit2D& hit, const CDCTrajectory3D& trajectory3D)
{
  const CDCRLTaggedWireHit& rlWireHit = hit.getRLWireHit();
  Vector3D recoPos3D(hit.getRecoPos2D(), 0.0);
  double perpS = trajectory3D.calcArcLength2D(recoPos3D);
  track.push_back(CDCRecoHit3D(rlWireHit, recoPos3D, perpS));
}

SegmentQuadTreeModule::SegmentQuadTreeModule() : TrackFinderCDCFromSegmentsModule()
{
  addParam("Level", m_param_level, "Maximum Level for QuadTreeSearch.", static_cast<unsigned int>(10));
  addParam("MinimumItems", m_param_minimumItems, "Minimum number of hits in one QuadTreeCell.", static_cast<unsigned int>(10));
  addParam("DebugOutput", m_param_debugOutput, "Create debug output or not.", false);
}

void SegmentQuadTreeModule::initialize()
{
  TrackFinderCDCFromSegmentsModule::initialize();
}

void SegmentQuadTreeModule::generate(std::vector<CDCRecoSegment2D>& segments, std::vector<CDCTrack>& tracks)
{
  quadTreeSearch(segments, tracks);
}

void SegmentQuadTreeModule::quadTreeSearch(std::vector<CDCRecoSegment2D>& recoSegments, std::vector<CDCTrack>& tracks)
{
  typedef TrackFindingCDC::QuadTreeProcessorSegments Processor; /**< The used quad tree processor */

  std::vector<Processor::ReturnList> foundTracks;

  Processor::ChildRanges ranges(Processor::rangeX(0, m_nbinsTheta),
                                Processor::rangeY(m_rMin, m_rMax));

  Processor::ReturnList hits_set;

  for (Processor::ItemType::TypeData& reco : recoSegments) {
    if (reco.getStereoKind() == EStereoKind::c_Axial) {
      hits_set.push_back(&reco);
    }
  }

  Processor::CandidateProcessorLambda lmdCandidateProcessing = [&](const Processor::ReturnList & items,
  Processor::QuadTree * node) -> void {
    B2DEBUG(90, "Found track with " << items.size() << " on level " << static_cast<unsigned int>(node->getLevel()));
    foundTracks.push_back(items);
  };

  Processor qtProcessor(m_param_level, ranges, Vector2D(0, 0), m_param_debugOutput);
  qtProcessor.provideItemsSet(hits_set);
  qtProcessor.fillGivenTree(lmdCandidateProcessing, m_param_minimumItems);

  B2DEBUG(90, "Found " << foundTracks.size() << " tracks");

  for (const Processor::ReturnList& track : foundTracks) {
    tracks.emplace_back();
    CDCTrack& foundTrack = tracks.back();
    CDCObservations2D observations;
    CDCTrajectory2D trajectory;

    for (Processor::ItemType::TypeData* item : track) {
      observations.appendRange(*item);
    }

    const CDCRiemannFitter& fitter = CDCRiemannFitter::getFitter();
    fitter.update(trajectory, observations);

    const CDCRecoSegment2D* ptrFirstSegment = track.front();
    const CDCRecoHit2D& firstRecoHit2D = ptrFirstSegment->front();
    trajectory.setLocalOrigin(firstRecoHit2D.getRecoPos2D());

    CDCTrajectory3D trajectory3D(trajectory, CDCTrajectorySZ::basicAssumption());

    foundTrack.setStartTrajectory3D(trajectory3D);

    for (Processor::ItemType::TypeData* recoSegment : track) {
      for (const CDCRecoHit2D& recoHit : *recoSegment) {
        addHitToTrack(foundTrack, recoHit, trajectory3D);
      }
    }
  }

  if (m_param_debugOutput) {
    // Debug output
    const auto& debugMap = qtProcessor.getDebugInformation();

    TFile file("output.root", "RECREATE");
    TH2F histUnused("histUnused", "QuadTreeContent - unused Items", std::pow(2, m_param_level), 0, m_nbinsTheta, std::pow(2,
                    m_param_level), m_rMin, m_rMax);
    TH2F histUsed("histUsed", "QuadTreeContent - used Items", std::pow(2, m_param_level), 0, m_nbinsTheta, std::pow(2, m_param_level),
                  m_rMin, m_rMax);

    for (const auto& debug : debugMap) {
      const auto& positionInformation = debug.first;
      const auto& quadItemsVector = debug.second;

      unsigned int usedItems = 0;

      for (const auto& quadItem : quadItemsVector) {
        if (quadItem->isUsed()) {
          usedItems++;
        }
      }

      histUnused.Fill(positionInformation.first, positionInformation.second, quadItemsVector.size() - usedItems);
      histUsed.Fill(positionInformation.first, positionInformation.second, usedItems);
    }

    histUnused.Write();
    histUsed.Write();
    file.Clone();
  }
}
