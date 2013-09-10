/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/dataobjects/MuidLikelihood.h>
#include <tracking/dataobjects/Muid.h>

#include <cmath>

using namespace std;
using namespace Belle2;

ClassImp(MuidLikelihood)

MuidLikelihood::MuidLikelihood(const Muid* muid) : RelationsObject()
{
  m_logL_mu   = (muid->getMuonPDFValue() > 0.0 ? log(muid->getMuonPDFValue()) : -120.0);
  m_logL_pi   = (muid->getPionPDFValue() > 0.0 ? log(muid->getPionPDFValue()) : -120.0);
  m_logL_K    = (muid->getKaonPDFValue() > 0.0 ? log(muid->getKaonPDFValue()) : -120.0);
  m_logL_miss = (muid->getMissPDFValue() > 0.0 ? log(muid->getMissPDFValue()) : -120.0);
  m_logL_junk = (muid->getJunkPDFValue() > 0.0 ? log(muid->getJunkPDFValue()) : -120.0);
}

