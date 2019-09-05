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
#include <alignment/GlobalDerivatives.h>
#include <alignment/GlobalLabel.h>
#include <alignment/reconstruction/AlignableEKLMRecoHit.h>
#include <klm/dataobjects/KLMElementNumbers.h>
#include <klm/eklm/dataobjects/EKLMDigit.h>
#include <klm/eklm/dataobjects/EKLMHit2d.h>
#include <klm/eklm/dataobjects/ElementNumbersSingleton.h>
#include <klm/eklm/dbobjects/EKLMAlignment.h>
#include <klm/eklm/geometry/GeometryData.h>
#include <klm/eklm/geometry/TransformDataGlobalAligned.h>

using namespace Belle2;
using namespace alignment;

AlignableEKLMRecoHit::AlignableEKLMRecoHit()
{
}

AlignableEKLMRecoHit::AlignableEKLMRecoHit(
  const EKLMAlignmentHit* hit, const genfit::TrackCandHit* trackCandHit) :
  genfit::PlanarMeasurement(1)
{
  (void)trackCandHit;
  int digit, plane, segment, strip;
  const HepGeom::Transform3D* t;
  CLHEP::HepRotation r;
  CLHEP::Hep3Vector origin;
  CLHEP::Hep3Vector u(1, 0, 0);
  CLHEP::Hep3Vector v(0, 1, 0);
  B2Vector3D origin2, u2, v2;
  const EKLM::GeometryData* geoDat = &(EKLM::GeometryData::Instance());
  const EKLM::TransformDataGlobalAligned* transformData =
    &(EKLM::TransformDataGlobalAligned::Instance());
  RelationVector<EKLMHit2d> hit2ds = hit->getRelationsTo<EKLMHit2d>();
  if (hit2ds.size() != 1)
    B2FATAL("Incorrect number of related EKLMHit2ds.");
  RelationVector<EKLMDigit> eklmDigits = hit2ds[0]->getRelationsTo<EKLMDigit>();
  if (eklmDigits.size() != 2)
    B2FATAL("Incorrect number of related EKLMDigits.");
  digit = hit->getDigitIdentifier();
  m_Section = eklmDigits[digit]->getSection();
  m_Layer = eklmDigits[digit]->getLayer();
  m_Sector = eklmDigits[digit]->getSector();
  plane = eklmDigits[digit]->getPlane();
  segment = (eklmDigits[digit]->getStrip() - 1) / geoDat->getNStripsSegment()
            + 1;
  strip = (segment - 1) * geoDat->getNStripsSegment() + 1;
  const KLMElementNumbers* elementNumbers = &(KLMElementNumbers::Instance());
  m_KLMModule = elementNumbers->moduleNumberEKLM(m_Section, m_Sector, m_Layer);
  const EKLM::ElementNumbersSingleton* eklmElementNumbers =
    &(EKLM::ElementNumbersSingleton::Instance());
  m_Segment = eklmElementNumbers->segmentNumber(
                m_Section, m_Layer, m_Sector, plane, segment);
  t = transformData->getStripTransform(
        m_Section, m_Layer, m_Sector, plane, strip);
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
  t = transformData->getSectorTransform(m_Section, m_Layer, m_Sector);
  r = t->getRotation().inverse();
  v = r * v;
  m_StripV.SetX(v.unit().x());
  m_StripV.SetY(v.unit().y());
  genfit::SharedPlanePtr detPlane(new genfit::DetPlane(origin2, u2, v2, 0));
  setPlane(detPlane, m_Segment);
  rawHitCoords_[0] = geoDat->getStripGeometry()->getWidth() *
                     ((eklmDigits[digit]->getStrip() - 1) %
                      geoDat->getNStripsSegment()) / CLHEP::cm * Unit::cm;
  rawHitCov_[0][0] = pow(geoDat->getStripGeometry()->getWidth() /
                         CLHEP::cm * Unit::cm, 2) / 12;
  setStripV();
}

AlignableEKLMRecoHit::~AlignableEKLMRecoHit()
{
}

std::pair<std::vector<int>, TMatrixD> AlignableEKLMRecoHit::globalDerivatives(const genfit::StateOnPlane* sop)
{
  std::vector<int> labGlobal;
  labGlobal.push_back(GlobalLabel::construct<EKLMAlignment>(m_KLMModule, KLMAlignmentData::c_DeltaU)); // dx
  labGlobal.push_back(GlobalLabel::construct<EKLMAlignment>(m_KLMModule, KLMAlignmentData::c_DeltaV)); // dy
  labGlobal.push_back(GlobalLabel::construct<EKLMAlignment>(m_KLMModule, KLMAlignmentData::c_DeltaGamma)); // drot

  /* Local parameters. */
  const double dalpha = 0;
  const double dxs = 0;
  const double dys = 0;
  //const double dy = 0;
  const double sinda = sin(dalpha);
  const double cosda = cos(dalpha);
  /* Local position in segment coordinates. */
  TVector2 pos = sop->getPlane()->LabToPlane(sop->getPos());
  //double u = pos.X();
  //double v = pos.Y();
  /* Local position in sector coordinates. */
  HepGeom::Point3D<double> globalPos;
  HepGeom::Transform3D t;
  const EKLM::TransformDataGlobalAligned* transformData =
    &(EKLM::TransformDataGlobalAligned::Instance());
  t = (*transformData->getSectorTransform(m_Section, m_Layer, m_Sector)).inverse();
  globalPos.setX(sop->getPos().X() / Unit::cm * CLHEP::cm);
  globalPos.setY(sop->getPos().Y() / Unit::cm * CLHEP::cm);
  globalPos.setZ(sop->getPos().Z() / Unit::cm * CLHEP::cm);
  globalPos = t * globalPos;
  double x = globalPos.x() / CLHEP::cm * Unit::cm;
  double y = globalPos.y() / CLHEP::cm * Unit::cm;
  /*
   * Matrix of global derivatives of local coordinate (second dimension is
   * added because of resizing in GblFitterInfo::constructGblPoint()) by
   * variations of the detector element position.
   * The sign is inverted to match the convention for other detectors.
   */
  TMatrixD derGlobal(2, 3);
  derGlobal(0, 0) = 0;
  derGlobal(0, 1) = 0;
  derGlobal(0, 2) = 0;
  derGlobal(1, 0) = cosda * m_StripV.X() - sinda * m_StripV.Y();
  derGlobal(1, 1) = sinda * m_StripV.X() + cosda * m_StripV.Y();
  derGlobal(1, 2) =
    -((x - dxs) * (-sinda * m_StripV.X() - cosda * m_StripV.Y()) +
      (y - dys) * (cosda * m_StripV.X() - sinda * m_StripV.Y()));
  //derGlobal(1, 3) = -cosda;
  //derGlobal(1, 4) = -u * cosda - (v - dy) * sinda;

  return alignment::GlobalDerivatives(labGlobal, derGlobal);

}

genfit::AbsMeasurement* AlignableEKLMRecoHit::clone() const
{
  return new AlignableEKLMRecoHit(*this);
}

