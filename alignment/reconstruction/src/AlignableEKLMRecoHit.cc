/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* Belle2 headers. */
#include <alignment/GlobalLabel.h>
#include <alignment/reconstruction/AlignableEKLMRecoHit.h>
#include <eklm/dataobjects/EKLMDigit.h>
#include <eklm/dataobjects/EKLMHit2d.h>
#include <eklm/geometry/GeometryData.h>

using namespace Belle2;

AlignableEKLMRecoHit::AlignableEKLMRecoHit() : m_Segment()
{
}

AlignableEKLMRecoHit::AlignableEKLMRecoHit(
  const EKLMAlignmentHit* hit, const genfit::TrackCandHit* trackCandHit) :
  m_Segment()
{
  int digit, segment;
  const EKLM::GeometryData* geoDat = &(EKLM::GeometryData::Instance());
  RelationVector<EKLMHit2d> hit2ds = hit->getRelationsTo<EKLMHit2d>();
  if (hit2ds.size() != 1)
    B2FATAL("Incorrect number of related EKLMHit2ds.");
  RelationVector<EKLMDigit> eklmDigits = hit2ds[0]->getRelationsTo<EKLMDigit>();
  if (eklmDigits.size() != 2)
    B2FATAL("Incorrect number of related EKLMDigits.");
  digit = hit->getDigitIdentifier();
  segment = (eklmDigits[digit]->getStrip() - 1) / geoDat->getNStripsSegment()
            + 1;
  m_Segment.setEndcap(eklmDigits[digit]->getEndcap());
  m_Segment.setLayer(eklmDigits[digit]->getLayer());
  m_Segment.setSector(eklmDigits[digit]->getSector());
  m_Segment.setPlane(eklmDigits[digit]->getPlane());
  m_Segment.setSegment(segment);
}

AlignableEKLMRecoHit::~AlignableEKLMRecoHit()
{
}

std::vector<int> AlignableEKLMRecoHit::labels()
{
  std::vector<int> labels;
  labels.push_back(GlobalLabel(m_Segment, 1));
  labels.push_back(GlobalLabel(m_Segment, 2));
  return labels;
}

TMatrixD AlignableEKLMRecoHit::derivatives(const genfit::StateOnPlane* sop)
{
  /* Matrix of global derivatives. */
  TMatrixD derGlobal(2, 2);
  derGlobal.Zero();
  return derGlobal;
}

