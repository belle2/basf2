/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include "../include/MCAxialStereoSegmentPairNeighborChooser.h"

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

MCAxialStereoSegmentPairNeighborChooser::MCAxialStereoSegmentPairNeighborChooser(bool allowReverse) :
  m_mcAxialStereoSegmentPairFilter(allowReverse)
{
}


void MCAxialStereoSegmentPairNeighborChooser::clear()
{
  m_mcAxialStereoSegmentPairFilter.clear();
}



void MCAxialStereoSegmentPairNeighborChooser::initialize()
{
  m_mcAxialStereoSegmentPairFilter.initialize();
}



void MCAxialStereoSegmentPairNeighborChooser::terminate()
{
  m_mcAxialStereoSegmentPairFilter.terminate();
}



void MCAxialStereoSegmentPairNeighborChooser::setParameter(const std::string& key, const std::string& value)
{
  if (key == "symmetric") {
    if (value == "true") {
      m_mcAxialStereoSegmentPairFilter.setAllowReverse(true);
      B2INFO("Filter received parameter '" << key << "' " << value);
    } else if (value == "false") {
      m_mcAxialStereoSegmentPairFilter.setAllowReverse(false);
      B2INFO("Filter received parameter '" << key << "' " << value);
    } else {
      Super::setParameter(key, value);
    }
  } else {
    Super::setParameter(key, value);
  }
}

std::map<std::string, std::string> MCAxialStereoSegmentPairNeighborChooser::getParameterDescription()
{
  std::map<std::string, std::string> des = Super::getParameterDescription();
  des["symmetric"] =  "Accept the facet relation if the reverse axial stereo segment pair relation is correct "
                      "preserving the progagation reversal symmetry on this level of detail."
                      "Allowed values 'true', 'false'. Default is 'true'.";
  return des;
}

bool MCAxialStereoSegmentPairNeighborChooser::needsTruthInformation()
{
  return true;
}

NeighborWeight
MCAxialStereoSegmentPairNeighborChooser::operator()(const CDCAxialStereoSegmentPair& fromAxialStereoSegmentPair,
                                                    const CDCAxialStereoSegmentPair& toAxialStereoSegmentPair)
{
  CellWeight mcFromPairWeight = m_mcAxialStereoSegmentPairFilter(fromAxialStereoSegmentPair);
  CellWeight mcToPairWeight = m_mcAxialStereoSegmentPairFilter(toAxialStereoSegmentPair);

  bool mcDecision = (not isNotACell(mcFromPairWeight)) and (not isNotACell(mcToPairWeight));
  return mcDecision ? -toAxialStereoSegmentPair.getStartSegment()->size() : NOT_A_NEIGHBOR;
}
