/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/facet/FeasibleRLFacetFilter.h>

#include <tracking/trackFindingCDC/eventdata/hits/CDCFacet.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCRLWireHitTriple.h>

#include <tracking/trackFindingCDC/numerics/Modulo.h>

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

#include <framework/core/ModuleParamList.templateDetails.h>

using namespace Belle2;
using namespace TrackFindingCDC;

FeasibleRLFacetFilter::FeasibleRLFacetFilter(bool hardRLCut)
  : m_param_hardRLCut(hardRLCut)
{
}

void FeasibleRLFacetFilter::exposeParameters(ModuleParamList* moduleParamList,
                                             const std::string& prefix)
{
  Super::exposeParameters(moduleParamList, prefix);
  moduleParamList->addParameter(prefixed(prefix, "hardRLCut"),
                                m_param_hardRLCut,
                                "Switch to disallow the boarderline possible hit and "
                                "right left passage information.",
                                m_param_hardRLCut);
}

Weight FeasibleRLFacetFilter::operator()(const CDCFacet& facet)
{
  if (this->isFeasible(facet)) {
    return 3;
  } else {
    return NAN;
  }
}

bool FeasibleRLFacetFilter::isFeasible(const CDCRLWireHitTriple& rlWireHitTriple) const
{
  const CDCRLWireHitTriple::Shape shape = rlWireHitTriple.getShape();
  const short oClockDelta = shape.getOClockDelta();
  const short absOClockDelta = std::abs(oClockDelta);
  const short cellExtend = shape.getCellExtend();

  if (cellExtend + absOClockDelta > 6) {
    // funny formula, but basically checks the triple to be a progressing forward and not turning back in itself.
    return false;
  }

  const ERightLeft startRLInfo = rlWireHitTriple.getStartRLInfo();
  const ERightLeft middleRLInfo = rlWireHitTriple.getMiddleRLInfo();
  const ERightLeft endRLInfo = rlWireHitTriple.getEndRLInfo();

  const short stableTwist = -sign(shape.getOClockDelta()) * middleRLInfo;
  const bool startToMiddleIsCrossing = startRLInfo != middleRLInfo;
  const bool middleToEndIsCrossing = middleRLInfo != endRLInfo;

  const bool bothAreCrossing = startToMiddleIsCrossing and middleToEndIsCrossing;

  const bool startToMiddleIsLong = shape.getStartToMiddleCellDistance() > shape.getMiddleToEndCellDistance();
  const bool shortArmIsCrossing = startToMiddleIsLong ? middleToEndIsCrossing : startToMiddleIsCrossing;

  const bool onlyOneShortArm = isOdd(cellExtend);
  const bool shortArmsAreCrossing = bothAreCrossing or (onlyOneShortArm and shortArmIsCrossing);
  const bool noneAreCrossing = not startToMiddleIsCrossing and not middleToEndIsCrossing;

  const bool orthoHard = stableTwist > 0 and shortArmsAreCrossing;
  const bool ortho = stableTwist > 0 and not noneAreCrossing;
  const bool meta = stableTwist > 0 or noneAreCrossing;
  const bool para = not bothAreCrossing;

  // Redundant check saves a bit of computation time.
  if (oClockDelta == 0) {
    return para;
  }

  switch (cellExtend) {
    case 2:
      switch (absOClockDelta) {
        case 0:
          return para;
          break;

        case 2:
          return ortho or (not m_param_hardRLCut and meta);
          break;

        case 4:
          return orthoHard or (not m_param_hardRLCut and ortho);
          break;
      }
      break;

    case 3:
      switch (absOClockDelta) {
        case 0:
          return para;
          break;

        case 1:
          return meta;
          break;

        case 2:
          return orthoHard or (not m_param_hardRLCut and ortho);
          break;

        case 3:
          return orthoHard;
          break;
      }
      break;

    case 4:
      switch (absOClockDelta) {
        case 0:
          return para;
          break;

        case 1:
          return ortho or (not m_param_hardRLCut and meta);
          break;

        case 2:
          return orthoHard;
          break;
      }
      break;
  }
  return false;
}
