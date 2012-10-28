/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/dataobjects/ParticleCode.h>


using namespace Belle2;

const unsigned int ParticleCode::s_chargedStables[NParticle] = {11, 13, 211, 321, 2212};

ParticleCode::ParticleCode(unsigned int pdgCode)
{
  m_iCode = NParticle + 1;
  for (int ii = 0; ii < NParticle; ii++) {
    if (s_chargedStables[ii] == pdgCode) {
      m_iCode = ii;
    }
  }
  if (m_iCode == NParticle + 1) {
    B2ERROR("pdgCode used as input isn't corresponding to known particle.");
  }
}
