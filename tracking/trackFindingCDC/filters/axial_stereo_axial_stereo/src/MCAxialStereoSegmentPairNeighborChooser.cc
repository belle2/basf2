/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include "../include/MCSegmentPairNeighborChooser.h"

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

MCSegmentPairNeighborChooser::MCSegmentPairNeighborChooser(bool allowReverse) :
  m_mcSegmentPairFilter(allowReverse)
{
}


void MCSegmentPairNeighborChooser::clear()
{
  m_mcSegmentPairFilter.clear();
}



void MCSegmentPairNeighborChooser::initialize()
{
  m_mcSegmentPairFilter.initialize();
}



void MCSegmentPairNeighborChooser::terminate()
{
  m_mcSegmentPairFilter.terminate();
}



void MCSegmentPairNeighborChooser::setParameter(const std::string& key, const std::string& value)
{
  if (key == "symmetric") {
    if (value == "true") {
      m_mcSegmentPairFilter.setAllowReverse(true);
      B2INFO("Filter received parameter '" << key << "' " << value);
    } else if (value == "false") {
      m_mcSegmentPairFilter.setAllowReverse(false);
      B2INFO("Filter received parameter '" << key << "' " << value);
    } else {
      Super::setParameter(key, value);
    }
  } else {
    Super::setParameter(key, value);
  }
}

std::map<std::string, std::string> MCSegmentPairNeighborChooser::getParameterDescription()
{
  std::map<std::string, std::string> des = Super::getParameterDescription();
  des["symmetric"] =  "Accept the facet relation if the reverse axial stereo segment pair relation is correct "
                      "preserving the progagation reversal symmetry on this level of detail."
                      "Allowed values 'true', 'false'. Default is 'true'.";
  return des;
}

bool MCSegmentPairNeighborChooser::needsTruthInformation()
{
  return true;
}

NeighborWeight
MCSegmentPairNeighborChooser::operator()(const CDCSegmentPair& fromSegmentPair,
                                         const CDCSegmentPair& toSegmentPair)
{
  CellWeight mcFromPairWeight = m_mcSegmentPairFilter(fromSegmentPair);
  CellWeight mcToPairWeight = m_mcSegmentPairFilter(toSegmentPair);

  bool mcDecision = (not isNotACell(mcFromPairWeight)) and (not isNotACell(mcToPairWeight));
  return mcDecision ? -toSegmentPair.getStartSegment()->size() : NOT_A_NEIGHBOR;
}
