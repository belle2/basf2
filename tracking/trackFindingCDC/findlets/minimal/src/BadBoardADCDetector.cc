/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/trackFindingCDC/findlets/minimal/BadBoardADCDetector.h>

#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>
#include <cdc/dataobjects/CDCHit.h>

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>
#include <framework/core/ModuleParamList.templateDetails.h>
#include <cdc/geometry/CDCGeometryPar.h>

#include <map>

using namespace Belle2;
using namespace TrackFindingCDC;

BadBoardADCDetector::BadBoardADCDetector()
{
}

std::string BadBoardADCDetector::getDescription()
{
  return "Detect boards with bad ADC values and mark hits accordingly";
}

void BadBoardADCDetector::exposeParameters(ModuleParamList* moduleParamList,
                                           const std::string& prefix)
{
  moduleParamList->addParameter(prefixed(prefix, "badADCaverageMin"),
                                m_badADCaverageMin,
                                "Minimal value of average ADC to consider board bad",
                                m_badADCaverageMin);
  moduleParamList->addParameter(prefixed(prefix, "badTOTaverageMin"),
                                m_badTOTaverageMin,
                                "Minimal value of average TOT to consider board bad",
                                m_badTOTaverageMin);
}

void BadBoardADCDetector::apply(std::vector<CDCWireHit>& wireHits)
{
  CDC::CDCGeometryPar& geometryPar = CDC::CDCGeometryPar::Instance();
  // first loop: average ADC per board
  std::map <int, double> BoardADC;
  std::map <int, double> BoardTOT;
  std::map <int, int> BoardCount;
  for (auto& wireHit : wireHits) {
    auto board = geometryPar.getBoardID(wireHit.getWireID());
    BoardCount[board] += 1;
    BoardADC[board] += (*wireHit.getHit()).getADCCount();
    BoardTOT[board] += (*wireHit.getHit()).getTOT();
  };
  // now compute the averages:
  for (auto& pair  : BoardADC) {
    int board = pair.first;
    BoardADC[board] /= BoardCount[board];
    BoardTOT[board] /= BoardCount[board];
  }

  // second loop, set flag if board is problematic:
  for (auto& wireHit : wireHits) {
    auto board = geometryPar.getBoardID(wireHit.getWireID());
    if (BoardADC[board] > m_badADCaverageMin)
      wireHit->setBoardWithBadADCFlag();
    if (BoardTOT[board] > m_badTOTaverageMin)
      wireHit->setBoardWithBadTOTFlag();
  }
}
