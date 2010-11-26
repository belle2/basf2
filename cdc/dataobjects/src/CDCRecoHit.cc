/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Guofu Cao, Martin Heck                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <cdc/dataobjects/CDCRecoHit.h>

using namespace std;
using namespace Belle2;

ClassImp(CDCRecoHit);

CDCRecoHit::CDCRecoHit()
    : GFRecoHitIfc<GFWireHitPolicy> (m_nParHitRep)
{
}

CDCRecoHit::CDCRecoHit(const CDCHit& cdcHit)
    : GFRecoHitIfc<GFWireHitPolicy> (m_nParHitRep)
{
}

GFAbsRecoHit* CDCRecoHit::clone()
{
}

TMatrixD CDCRecoHit::getHMatrix()
{
}
