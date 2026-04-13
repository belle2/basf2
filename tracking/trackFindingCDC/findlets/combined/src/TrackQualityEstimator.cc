/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/trackFindingCDC/findlets/combined/TrackQualityEstimator.h>

#include <tracking/trackingUtilities/eventdata/tracks/CDCTrack.h>

#include <tracking/trackingUtilities/filters/base/ChooseableFilter.icc.h>

#include <tracking/trackingUtilities/utilities/StringManipulation.h>
#include <tracking/trackingUtilities/utilities/Algorithms.h>

#include <framework/core/ModuleParamList.templateDetails.h>

#include <tracking/trackingUtilities/eventdata/hits/CDCRecoHit3D.h>
#include <cdc/dataobjects/CDCHit.h>
#include <cdc/geometry/CDCGeometryPar.h>
#include <cdc/topology/CDCWire.h>
#include <tracking/trackingUtilities/rootification/StoreWrappedObjPtr.h>


using namespace Belle2;
using namespace TrackFindingCDC;
using namespace TrackingUtilities;

template class TrackingUtilities::ChooseableFilter<TrackQualityFilterFactory>;

TrackQualityEstimator::TrackQualityEstimator(const std::string& defaultFilterName)
  : m_trackQualityFilter(defaultFilterName)
{
  this->addProcessingSignalListener(&m_mcCloneLookUpFiller);
  this->addProcessingSignalListener(&m_trackQualityFilter);
}

void TrackQualityEstimator::initialize()
{
  Super::initialize();
  // cache output of needsTruthInformation in member variable
  m_needsTruthInformation = m_trackQualityFilter.needsTruthInformation();
}

std::string TrackQualityEstimator::getDescription()
{
  return "Set the quality indicator for CDC tracks and, if desired, delete tracks with a too low quality value.";
}

void TrackQualityEstimator::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  m_trackQualityFilter.exposeParameters(moduleParamList, prefix);
  m_mcCloneLookUpFiller.exposeParameters(moduleParamList, prefix);

  moduleParamList->addParameter(prefixed(prefix, "deleteTracks"),
                                m_param_deleteTracks,
                                "Delete tracks below cut instead of just assigning quality indicator.",
                                m_param_deleteTracks);

  moduleParamList->addParameter(prefixed(prefix, "resetTakenFlag"),
                                m_param_resetTakenFlag,
                                "Reset taken flag for deleted tracks so that hits can be used by subsequent TFs.",
                                m_param_resetTakenFlag);
}

void TrackQualityEstimator::apply(std::vector<CDCTrack>& tracks)
{

  if (m_needsTruthInformation) { m_mcCloneLookUpFiller.apply(tracks); }
  for (CDCTrack& track : tracks) {
    const double qualityIndicator = m_trackQualityFilter(track);
    track.setQualityIndicator(qualityIndicator);

    if (std::isnan(qualityIndicator)) {
      // debug track would be deleted
      auto list = getListOfBadBoardCands(track, 5);

      // debug: dummy vector with bad boards. Simulation did use 62 and 63 as bad boards
      //std::vector<unsigned int> badBoardList = {62, 63};

      Belle2::TrackingUtilities::StoreWrappedObjPtr< std::vector<unsigned int> > storeVector("TestBadBoardsVector");
      std::vector<unsigned int>& badBoardList = *storeVector;

      std::cout << "storevector " << std::endl;
      for (auto i : badBoardList) std::cout << i << std::endl;

      std::cout << "list of layer jumps" << std::endl;
      for (auto& li : list) {
        std::cout << li.first << " " << li.second << std::endl;
        if (std::find(badBoardList.begin(), badBoardList.end(), li.first) != badBoardList.end() ||
            std::find(badBoardList.begin(), badBoardList.end(), li.second) != badBoardList.end()) {

          std::cout << "kept" << std::endl;
          track.setQualityIndicator(123); // value doesnt matter as all NaN are rejected
        } else { std::cout << "rejected" << std::endl;}
      }



    }
  }

  if (m_param_deleteTracks) { // delete track with QI below cut threshold
    auto reject = [this](const CDCTrack & track) {
      const double qualityIndicator = track.getQualityIndicator();
      if (m_param_resetTakenFlag && std::isnan(qualityIndicator)) {
        track.forwardTakenFlag(false);
      }
      return std::isnan(qualityIndicator);
    };
    erase_remove_if(tracks, reject);
  }
}


std::vector< std::pair<unsigned short, unsigned short> >
TrackQualityEstimator::getListOfBadBoardCands(const TrackingUtilities::CDCTrack& aCDCTrack, int minJump)
{

  CDC::CDCGeometryPar& geometryPar = CDC::CDCGeometryPar::Instance();

  std::cout << "new track" << std::endl;
  std::vector< std::pair<unsigned short, unsigned short> > list;

  double arcLengthShift = 4.5; //unit cm, correcponding to around 2.25 drift cells (if passing pendicular)

  auto trajectory = aCDCTrack.getStartTrajectory3D();

  Helix localHelix = trajectory.getLocalHelix().helix();
  auto localOrigin = trajectory.getLocalOrigin();


  // detect layer jumps jumping at least minJump layers
  // Assumes hits are ordered! Need check if that is always true, or sorting without check
  const CDCRecoHit3D* prevHitPtr = nullptr;
  for (const CDCRecoHit3D& thisHit : aCDCTrack) {
    if (not prevHitPtr) {
      prevHitPtr = &thisHit;
      continue;
    }

    Belle2::CDC::ILayer iclayerPrev = ((const CDCHit*)*prevHitPtr)->getICLayer(); // signed short
    Belle2::CDC::ILayer iclayerThis = ((const CDCHit*)thisHit)->getICLayer();

    if (abs(iclayerPrev - iclayerThis) >= minJump) {
      double sPrevious = prevHitPtr->getArcLength2D();
      double sCurrent = thisHit.getArcLength2D();

      // may want to filter for too high arclength??? observed up to 45cm arclength differences
      Vector3D pos3DPrev = localHelix.atArcLength2D(sPrevious + arcLengthShift) + localOrigin;
      Vector3D pos3DThis = localHelix.atArcLength2D(sCurrent - arcLengthShift) + localOrigin;

      // direction in case of backcurling tracks
      int dir = (iclayerThis - iclayerPrev) < 0 ? -1 : 1;
      // jump 3 layers to get the onto next board
      unsigned int newlayerThis = iclayerThis - dir * 3;
      unsigned int newlayerPrev = iclayerPrev + dir * 3;

      // find the wire number
      Belle2::CDC::IWire iWireThis = geometryPar.cellId(newlayerThis, pos3DThis);
      Belle2::CDC::IWire iWirePrev = geometryPar.cellId(newlayerPrev, pos3DPrev);

      // get the board number
      auto boardThis = geometryPar.getBoardID(WireID(newlayerThis, iWireThis));
      auto boardPrev = geometryPar.getBoardID(WireID(newlayerPrev, iWirePrev));

      if (boardThis != boardPrev) list.push_back({boardThis, boardPrev});

      /*
      std::cout << "bad boards ? " << boardThis << " " << boardPrev << std::endl;
      std::cout << "arc length "  << sPrevious << " b " << sCurrent << std::endl;
      std::cout << "point a " << prevHitPtr->getRecoPos3D() << " point b " << thisHit.getRecoPos3D() << std::endl;
      std::cout << "extrapolated a " << (localHelix.atArcLength2D(sPrevious) + localOrigin) << " b " << (localHelix.atArcLength2D(sCurrent) + localOrigin)<< std::endl;
      std::cout << "new hit" << std::endl;
      std::cout << thisHit.getWire().getISuperLayer() << std::endl;
      std::cout << thisHit.getWire().getILayer() << std::endl;
      std::cout << thisHit.getWire().getICLayer() << std::endl;
      std::cout << thisHit.isAxial() << std::endl;
      std::cout << "old hit" << std::endl;
      std::cout << prevHitPtr->getWire().getISuperLayer() << std::endl;
      std::cout << prevHitPtr->getWire().getILayer() << std::endl;
      std::cout << prevHitPtr->getWire().getICLayer() << std::endl;
      std::cout << prevHitPtr->isAxial() << std::endl;

      int boardnumber =  geometryPar.getBoardID( WireID( ((const CDCHit*)thisHit)->getID()) );
      std::cout << "Board number :  " << boardnumber  << " old board " << geometryPar.getBoardID( WireID( ((const CDCHit*)*prevHitPtr)->getID() ) ) <<  std::endl;
      */

    }

    prevHitPtr = &thisHit;
  }

  return list;

}
