/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oksana Brovchenko                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/cdcHoughTracking/CDCHoughHit.h>

#include <cdc/geometry/CDCGeometryPar.h>

#include <cmath>
#include <TMath.h>

using namespace Belle2;
using namespace Belle2::Tracking;


//CDCHoughHit::CDCHoughHit

CDCHoughHit::CDCHoughHit(): m_storeIndex(0), m_superlayerId(0), m_isAxial(false),
  m_x(0), m_y(0), m_r(0), m_phi(0)
{
}

CDCHoughHit::CDCHoughHit(const CDCHit& hit, const unsigned int index): m_storeIndex(index),
  m_superlayerId(hit.getISuperLayer()),
  m_isAxial(m_superlayerId % 2 == 0)
{
  CDCGeometryPar* cdcgp = CDCGeometryPar::Instance();
  const unsigned int wire = hit.getIWire();
  unsigned int layer = hit.getILayer();

  if (m_superlayerId != 0) {
    layer += m_superlayerId * 6 + 2 ;
  }

  m_forwardPosition = cdcgp->wireForwardPosition(layer, wire);
  m_backwardPosition = cdcgp->wireBackwardPosition(layer, wire);
  m_x = (m_forwardPosition.x() + m_backwardPosition.x()) / 2;
  m_y = (m_forwardPosition.y() + m_backwardPosition.y()) / 2;
  m_r = sqrt(m_x * m_x + m_y * m_y);
  m_phi = Angle(atan2(m_y, m_x));
}

CDCHoughHit::~CDCHoughHit()
{
}


bool SortHoughHitsByRadius::operator()(const CDCHoughHit& lhs, const CDCHoughHit& rhs)
{
  return lhs.getR() < rhs.getR();
}
