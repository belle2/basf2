/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* Own header. */
#include <klm/dbobjects/KLMAlignmentData.h>

/* Belle 2 headers. */
#include <framework/logging/Logger.h>

using namespace Belle2;

KLMAlignmentData::KLMAlignmentData() :
  m_DeltaU(0),
  m_DeltaV(0),
  m_DeltaW(0),
  m_DeltaAlpha(0),
  m_DeltaBeta(0),
  m_DeltaGamma(0)
{
}

KLMAlignmentData::KLMAlignmentData(
  float deltaU, float deltaV, float deltaW,
  float deltaAlpha, float deltaBeta, float deltaGamma) :
  m_DeltaU(deltaU),
  m_DeltaV(deltaV),
  m_DeltaW(deltaW),
  m_DeltaAlpha(deltaAlpha),
  m_DeltaBeta(deltaBeta),
  m_DeltaGamma(deltaGamma)
{
}

KLMAlignmentData::~KLMAlignmentData()
{
}

float KLMAlignmentData::getParameter(enum ParameterNumbers number) const
{
  switch (number) {
    case c_DeltaU:
      return getDeltaU();
    case c_DeltaV:
      return getDeltaV();
    case c_DeltaW:
      return getDeltaW();
    case c_DeltaAlpha:
      return getDeltaAlpha();
    case c_DeltaBeta:
      return getDeltaBeta();
    case c_DeltaGamma:
      return getDeltaGamma();
  }
  B2FATAL("Attempt to get KLM alignment parameter with incorrect number " <<
          number);
}

void KLMAlignmentData::setParameter(enum ParameterNumbers number, float value)
{
  switch (number) {
    case c_DeltaU:
      setDeltaU(value);
      return;
    case c_DeltaV:
      setDeltaV(value);
      return;
    case c_DeltaW:
      setDeltaW(value);
      return;
    case c_DeltaAlpha:
      setDeltaAlpha(value);
      return;
    case c_DeltaBeta:
      setDeltaBeta(value);
      return;
    case c_DeltaGamma:
      setDeltaGamma(value);
      return;
  }
  B2FATAL("Attempt to set KLM alignment parameter with incorrect number " <<
          number);
}
