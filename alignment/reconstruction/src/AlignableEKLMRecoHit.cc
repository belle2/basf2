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
#include <eklm/geometry/TransformDataGlobalDisplaced.h>

#include <alignment/Hierarchy.h>
#include <alignment/GlobalDerivatives.h>


using namespace Belle2;

AlignableEKLMRecoHit::AlignableEKLMRecoHit()
{
}

AlignableEKLMRecoHit::AlignableEKLMRecoHit(
  const EKLMAlignmentHit* hit, const genfit::TrackCandHit* trackCandHit) :
  genfit::PlanarMeasurement(2)
{
  (void)trackCandHit;
  int digit, endcap, layer, sector, plane, segment, strip;
  const HepGeom::Transform3D* t;
  CLHEP::HepRotation r;
  CLHEP::Hep3Vector origin;
  CLHEP::Hep3Vector u(1, 0, 0);
  CLHEP::Hep3Vector v(0, 1, 0);
  TVector3 origin2, u2, v2, globalPosition;
  TVector2 localPosition;
  const EKLM::GeometryData* geoDat = &(EKLM::GeometryData::Instance());
  const EKLM::TransformDataGlobalDisplaced* transformData =
    &(EKLM::TransformDataGlobalDisplaced::Instance());
  RelationVector<EKLMHit2d> hit2ds = hit->getRelationsTo<EKLMHit2d>();
  if (hit2ds.size() != 1)
    B2FATAL("Incorrect number of related EKLMHit2ds.");
  RelationVector<EKLMDigit> eklmDigits = hit2ds[0]->getRelationsTo<EKLMDigit>();
  if (eklmDigits.size() != 2)
    B2FATAL("Incorrect number of related EKLMDigits.");
  digit = hit->getDigitIdentifier();
  endcap = eklmDigits[digit]->getEndcap();
  layer = eklmDigits[digit]->getLayer();
  sector = eklmDigits[digit]->getSector();
  plane = eklmDigits[digit]->getPlane();
  segment = (eklmDigits[digit]->getStrip() - 1) / geoDat->getNStripsSegment()
            + 1;
  strip = (segment - 1) * geoDat->getNStripsSegment() + 1;
  m_Segment.setEndcap(endcap);
  m_Segment.setLayer(layer);
  m_Segment.setSector(sector);
  m_Segment.setPlane(plane);
  m_Segment.setSegment(segment);
  t = transformData->getStripTransform(endcap, layer, sector, plane, strip);
  origin = t->getTranslation();
  origin2.SetX(origin.x() / CLHEP::cm * Unit::cm);
  origin2.SetY(origin.y() / CLHEP::cm * Unit::cm);
  origin2.SetZ(origin.z() / CLHEP::cm * Unit::cm);
  r = t->getRotation();
  u = r * u;
  v = r * v;
  u2.SetX(u.x());
  u2.SetY(u.y());
  u2.SetZ(u.z());
  v2.SetX(v.x());
  v2.SetY(v.y());
  v2.SetZ(v.z());
  genfit::SharedPlanePtr detPlane(new genfit::DetPlane(origin2, u2, v2, 0));
  setPlane(detPlane, m_Segment.getSegmentGlobalNumber());
  globalPosition = hit2ds[0]->getPosition();
  /* Projection onto hit plane - only need to change Z. */
  globalPosition.SetZ(origin2.Z());
  localPosition = detPlane->LabToPlane(globalPosition);
  rawHitCoords_[0] = localPosition.X();
  rawHitCoords_[1] = localPosition.Y();
  rawHitCov_[0][0] = pow(geoDat->getStripGeometry()->getWidth(), 2) / 12;
  rawHitCov_[0][1] = 0;
  rawHitCov_[1][0] = 0;
  rawHitCov_[1][1] = rawHitCov_[0][0];
}

AlignableEKLMRecoHit::~AlignableEKLMRecoHit()
{
}

std::pair<std::vector<int>, TMatrixD> AlignableEKLMRecoHit::globalDerivatives(const genfit::StateOnPlane* sop)
{
  std::vector<int> labels;
  labels.push_back(GlobalLabel::construct<EKLMAlignment>(m_Segment.getSegmentGlobalNumber(), 1));
  labels.push_back(GlobalLabel::construct<EKLMAlignment>(m_Segment.getSegmentGlobalNumber(), 2));

  const double dalpha = 0;
  const double dy = 0;
  const double sinda = sin(dalpha);
  const double cosda = cos(dalpha);
  /* Local position. */
  TVector2 pos = sop->getPlane()->LabToPlane(sop->getPos());
  double u = pos.X();
  double v = pos.Y();
  /* Matrix of global derivatives. */
  TMatrixD derGlobal(2, 2);
  derGlobal(0, 0) = -sinda;
  derGlobal(0, 1) = -cosda;
  derGlobal(1, 0) = -u * sinda + (v - dy) * cosda;
  derGlobal(1, 1) = -u * cosda + (v + dy) * sinda;

  return alignment::GlobalDerivatives::passGlobals(make_pair(labels, derGlobal));
}


genfit::AbsMeasurement* AlignableEKLMRecoHit::clone() const
{
  return new AlignableEKLMRecoHit(*this);
}

