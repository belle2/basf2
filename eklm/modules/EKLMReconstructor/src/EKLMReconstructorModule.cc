/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Timofey Uglov, Kirill Chilikin                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* Belle2 headers. */
#include <eklm/modules/EKLMReconstructor/EKLMReconstructorModule.h>
#include <framework/gearbox/Const.h>

using namespace Belle2;

REG_MODULE(EKLMReconstructor)

static bool comparePlane(EKLMDigit* d1, EKLMDigit* d2)
{
  int s1, s2;
  const EKLM::GeometryData& geoDat = EKLM::GeometryData::Instance();
  s1 = geoDat.planeNumber(d1->getEndcap(), d1->getLayer(), d1->getSector(),
                          d1->getPlane());
  s2 = geoDat.planeNumber(d2->getEndcap(), d2->getLayer(), d2->getSector(),
                          d2->getPlane());
  return s1 < s2;
}

static bool samePlane(EKLMDigit* d1, EKLMDigit* d2)
{
  return ((d1->getEndcap() == d2->getEndcap()) &&
          (d1->getLayer() == d2->getLayer()) &&
          (d1->getSector() == d2->getSector()) &&
          (d1->getPlane() == d2->getPlane()));
}

static bool sameSector(EKLMDigit* d1, EKLMDigit* d2)
{
  return ((d1->getEndcap() == d2->getEndcap()) &&
          (d1->getLayer() == d2->getLayer()) &&
          (d1->getSector() == d2->getSector()));
}

EKLMReconstructorModule::EKLMReconstructorModule() : Module(),
  m_DefaultTimeCalibrationData(0)
{
  setDescription("EKLM reconstruction module.");
  setPropertyFlags(c_ParallelProcessingCertified);
  m_TransformData = NULL;
  m_GeoDat = NULL;
  m_TimeCalibrationData = NULL;
}

EKLMReconstructorModule::~EKLMReconstructorModule()
{
}

void EKLMReconstructorModule::initialize()
{
  m_Hit2ds.registerInDataStore();
  m_AlignmentHits.registerInDataStore();
  m_Digits.isRequired();
  m_Hit2ds.registerRelationTo(m_Digits);
  m_AlignmentHits.registerRelationTo(m_Hit2ds);
  m_TransformData =
    new EKLM::TransformData(true, EKLM::TransformData::c_Alignment);
  m_GeoDat = &(EKLM::GeometryData::Instance());
  if (m_GeoDat->getNPlanes() != 2)
    B2FATAL("It is not possible to run EKLM reconstruction with 1 plane.");
  m_nStrip = m_GeoDat->getMaximalStripGlobalNumber();
  m_TimeCalibrationData = new const EKLMTimeCalibrationData*[m_nStrip];
}

void EKLMReconstructorModule::beginRun()
{
  int i;
  if (!m_RecPar.isValid())
    B2FATAL("EKLM digitization parameters are not available.");
  if (!m_TimeCalibration.isValid())
    B2FATAL("EKLM time calibration data is not available.");
  if (m_TimeCalibration.hasChanged()) {
    for (i = 0; i < m_nStrip; i++) {
      m_TimeCalibrationData[i] =
        m_TimeCalibration->getTimeCalibrationData(i + 1);
      if (m_TimeCalibrationData[i] == NULL) {
        B2FATAL("EKLM time calibration data is missing for strip "
                << i + 1 << ".");
        m_TimeCalibrationData[i] = &m_DefaultTimeCalibrationData;
      }
    }
  }
}

bool EKLMReconstructorModule::fastHit(HepGeom::Point3D<double>& pos,
                                      double time)
{
  return time < pos.mag() / Const::speedOfLight -
         2.0 * m_RecPar->getTimeResolution();
}

double EKLMReconstructorModule::getTime(EKLMDigit* d, double dist)
{
  int strip;
  strip = m_GeoDat->stripNumber(d->getEndcap(), d->getLayer(), d->getSector(),
                                d->getPlane(), d->getStrip()) - 1;
  return d->getTime() -
         (dist / m_TimeCalibration->getEffectiveLightSpeed() +
          m_TimeCalibration->getAmplitudeTimeConstant() / sqrt(d->getNPE()) +
          m_TimeCalibrationData[strip]->getTimeShift());
}

void EKLMReconstructorModule::event()
{
  int i, n;
  double d1, d2, t, t1, t2, sd;
  std::vector<EKLMDigit*> digitVector;
  std::vector<EKLMDigit*>::iterator it, it2, it3, it4, it5;
  n = m_Digits.getEntries();
  for (i = 0; i < n; i++)
    if (m_Digits[i]->isGood())
      digitVector.push_back(m_Digits[i]);
  /* Sort by plane. Note that numbers of planes from one sector differ by 1. */
  sort(digitVector.begin(), digitVector.end(), comparePlane);
  it = digitVector.begin();
  while (it != digitVector.end()) {
    it2 = it;
    while (1) {
      ++it2;
      if (it2 == digitVector.end())
        break;
      if (!samePlane(*it, *it2))
        break;
    }
    it3 = it2;
    --it3;
    while (1) {
      ++it3;
      if (it3 == digitVector.end())
        break;
      if (!sameSector(*it, *it3))
        break;
    }
    /* Now it .. it2 - first plane hits, it2 .. it3 - second plane hits. */
    for (it4 = it; it4 != it2; ++it4) {
      for (it5 = it2; it5 != it3; ++it5) {
        HepGeom::Point3D<double> crossPoint(0, 0, 0);
        if (!m_TransformData->intersection(*it4, *it5, &crossPoint, &d1, &d2,
                                           &sd))
          continue;
        t1 = getTime(*it4, d1) + 0.5 * sd / Const::speedOfLight;
        t2 = getTime(*it5, d2) - 0.5 * sd / Const::speedOfLight;
        t = (t1 + t2) / 2;
        EKLMHit2d* hit2d = m_Hit2ds.appendNew(*it4);
        hit2d->setEDep((*it4)->getEDep() + (*it5)->getEDep());
        hit2d->setPosition(crossPoint.x(), crossPoint.y(), crossPoint.z());
        hit2d->setChiSq((t1 - t2) * (t1 - t2) /
                        m_RecPar->getTimeResolution() /
                        m_RecPar->getTimeResolution());
        hit2d->setTime(t);
        hit2d->setMCTime(((*it4)->getMCTime() + (*it5)->getMCTime()) / 2);
        hit2d->addRelationTo(*it4);
        hit2d->addRelationTo(*it5);
        for (i = 0; i < 2; i++) {
          EKLMAlignmentHit* alignmentHit = m_AlignmentHits.appendNew(i);
          alignmentHit->addRelationTo(hit2d);
        }
      }
    }
    it = it3;
  }
}

void EKLMReconstructorModule::endRun()
{
}

void EKLMReconstructorModule::terminate()
{
  delete m_TransformData;
  delete[] m_TimeCalibrationData;
}

