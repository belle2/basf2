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

using namespace Belle2;

AlignableEKLMRecoHit::AlignableEKLMRecoHit()
{
  m_AlignmentHit = NULL;
  m_Hit2d = NULL;
  m_GeoDat = &(EKLM::GeometryData::Instance());
}

AlignableEKLMRecoHit::AlignableEKLMRecoHit(
  const EKLMAlignmentHit* hit, const genfit::TrackCandHit* trackCandHit)
{
  RelationVector<EKLMHit2d> hit2ds = hit->getRelationsTo<EKLMHit2d>();
  if (hit2ds.size() != 1)
    B2FATAL("Incorrect number of related EKLMHit2ds.");
  m_AlignmentHit = hit;
  m_Hit2d = hit2ds[0];
  m_GeoDat = &(EKLM::GeometryData::Instance());
}

AlignableEKLMRecoHit::~AlignableEKLMRecoHit()
{
}

std::vector<int> AlignableEKLMRecoHit::labels()
{
  int digit, segment;
  std::vector<int> labels;
  EKLMSegmentID* segmentId;
  RelationVector<EKLMDigit> eklmDigits =
    m_Hit2d->getRelationsTo<EKLMDigit>();
  if (eklmDigits.size() != 2)
    B2FATAL("Incorrect number of related EKLMDigits.");
  digit = m_AlignmentHit->getDigitIdentifier();
  segment = (eklmDigits[digit]->getStrip() - 1) /
            m_GeoDat->getNStripsSegment() + 1;
  segmentId = new EKLMSegmentID(
    eklmDigits[digit]->getEndcap(), eklmDigits[digit]->getLayer(),
    eklmDigits[digit]->getSector(), eklmDigits[digit]->getPlane(), segment);
  labels.push_back(GlobalLabel(*segmentId, 1));
  labels.push_back(GlobalLabel(*segmentId, 2));
  delete segmentId;
  return labels;
}

TMatrixD AlignableEKLMRecoHit::derivatives(const genfit::StateOnPlane* sop)
{
  /* Matrix of global derivatives. */
  TMatrixD derGlobal(2, 2);
  derGlobal.Zero();
  return derGlobal;
}

