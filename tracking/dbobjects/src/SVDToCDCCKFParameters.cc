/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <tracking/dbobjects/SVDToCDCCKFParameters.h>

using namespace Belle2;

ClassImp(SVDToCDCCKFParameters);

/** FLOAT PARAMETERS */

void SVDToCDCCKFParameters::setMaximalDeltaPhi(float phi) { m_maximalDeltaPhi = phi; }
float SVDToCDCCKFParameters::getMaximalDeltaPhi() const { return m_maximalDeltaPhi; }

void SVDToCDCCKFParameters::setMinimalPtRequirement(float pt) { m_minimalPtRequirement = pt; }
float SVDToCDCCKFParameters::getMinimalPtRequirement() const { return m_minimalPtRequirement; }


/** INTEGER PARAMETERS */

void SVDToCDCCKFParameters::setMaximalLayerJump(int layer) { m_maximalLayerJump = layer; }
int SVDToCDCCKFParameters::getMaximalLayerJump() const { return m_maximalLayerJump; }

void SVDToCDCCKFParameters::setMaximalLayerJumpBackwardSeed(int layer) { m_maximalLayerJumpBackwardSeed = layer; }
int SVDToCDCCKFParameters::getMaximalLayerJumpBackwardSeed() const { return m_maximalLayerJumpBackwardSeed; }

void SVDToCDCCKFParameters::setPathMaximalCandidatesInFlight(unsigned int max) { m_pathMaximalCandidatesInFlight = max; }
unsigned int SVDToCDCCKFParameters::getPathMaximalCandidatesInFlight() const { return m_pathMaximalCandidatesInFlight; }

void SVDToCDCCKFParameters::setStateMaximalHitCandidates(unsigned int max) { m_stateMaximalHitCandidates = max; }
unsigned int SVDToCDCCKFParameters::getStateMaximalHitCandidates() const { return m_stateMaximalHitCandidates; }
