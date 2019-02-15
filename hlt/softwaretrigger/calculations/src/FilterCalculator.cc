/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun, Chris Hearty                                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <hlt/softwaretrigger/calculations/FilterCalculator.h>
#include <hlt/softwaretrigger/calculations/utilities.h>

using namespace Belle2;
using namespace SoftwareTrigger;

void FilterCalculator::requireStoreArrays()
{
  /*isMC = m_effSample>=0 and m_effSample<=98;
  TrackArray.isRequired();
  eclClusterArray.isRequired();
  if(isMC) {MCParticleArray.isRequired();}
  if(m_RequireL1) {TRGResults.isRequired();}*/
};

void FilterCalculator::doCalculation(SoftwareTriggerObject& calculationResult)
{


}
