/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Guofu Cao                                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "cdc/modules/cdcDigitizer/CDCSignal.h"
#include "cdc/modules/cdcDigitizer/CDCColours.h"

#include <framework/logging/Logger.h>

#include <cstdlib>
#include <iomanip>

using namespace Belle2;
/*
void CDCSignal::updateMCParticles(MCParticle * mcPart, float weight)
{
  //-------------------------------
  // Update MC truth information.
  //-------------------------------
  MCPartVec::iterator iterMCPartVec     = m_MCPartVec.begin();
  MCWeightVec::iterator iterMCWeightVec = m_MCWeightVec.begin();

  bool found = false;

  // Does this particle exist in vector?
  while ( (!found) && (iterMCPartVec != m_MCPartVec.end()) && (iterMCWeightVec != m_MCWeightVec.end()) ) {

    // Yes - update its weight
    if (*iterMCPartVec == mcPart) {

       *iterMCWeightVec += weight;

       found = true;
    }

    iterMCPartVec++;
    iterMCWeightVec++;
  }

  // No - create it
  if (!found) {

    m_MCPartVec.push_back(mcPart);
    m_MCWeightVec.push_back(weight);

  }

}

void CDCSignal::updateMCParticles(MCPartVec mcPartVec, MCWeightVec mcWeightVec)
{
  //------------------------------
  // Update MC truth information.
  //------------------------------

  // Check if sizes the same
  if (mcPartVec.size() != mcWeightVec.size()) {

     B2ERROR(   "CDCSignal::updateMCParticles - MC particle vector size: " << mcPartVec.size()
           << " is different from MC weight vector size: "            << mcWeightVec.size()
           << ", exit.");
     exit(0);
  }

  MCPartVec::iterator iterMCPartVec     = mcPartVec.begin();
  MCWeightVec::iterator iterMCWeightVec = mcWeightVec.begin();

  for ( iterMCPartVec = mcPartVec.begin(); iterMCPartVec != mcPartVec.end(); iterMCPartVec++, iterMCWeightVec++ ) {

    MCParticle * mcPart   = *iterMCPartVec;
    float        mcWeight = *iterMCWeightVec;

    MCPartVec::iterator iterMCPartVec2      = m_MCPartVec.begin();
    MCWeightVec::iterator iterMCWeightVec2  = m_MCWeightVec.begin();

    bool found = false;

    // Does this particle exist in vector?
    while ( (!found) && (iterMCPartVec2 != m_MCPartVec.end()) && (iterMCWeightVec2 != m_MCWeightVec.end()) ) {

      // Yes - update its weight
      if (*iterMCPartVec2 == mcPart) {

         *iterMCWeightVec2 += mcWeight;

         found = true;
      }

      iterMCPartVec2++;
      iterMCWeightVec2++;

    }

    // No - create it
    if (!found) {

      m_MCPartVec.push_back(mcPart);
      m_MCWeightVec.push_back(mcWeight);

    }

  } // For
}

float CDCSignal::getMCWeightSum()
{
  //---------------------------------------
  // Get MC truth information - total sum.
  //---------------------------------------
  float sum = 0;

  MCWeightVec::iterator iterMCWeightVec;

  for ( iterMCWeightVec = m_MCWeightVec.begin(); iterMCWeightVec != m_MCWeightVec.end(); iterMCWeightVec++) {

     sum += *iterMCWeightVec;
  }

  return sum;
}
*/
