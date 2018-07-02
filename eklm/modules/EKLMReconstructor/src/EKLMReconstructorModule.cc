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
#include <eklm/dataobjects/ElementNumbersSingleton.h>
#include <eklm/modules/EKLMReconstructor/EKLMReconstructorModule.h>
#include <framework/gearbox/Const.h>

using namespace Belle2;

REG_MODULE(EKLMReconstructor)

static bool compareSector(EKLMDigit* d1, EKLMDigit* d2)
{
  int s1, s2;
  static const EKLM::ElementNumbersSingleton& elementNumbers =
    EKLM::ElementNumbersSingleton::Instance();
  s1 = elementNumbers.sectorNumber(d1->getEndcap(), d1->getLayer(),
                                   d1->getSector());
  s2 = elementNumbers.sectorNumber(d2->getEndcap(), d2->getLayer(),
                                   d2->getSector());
  return s1 < s2;
}

static bool comparePlane(EKLMDigit* d1, EKLMDigit* d2)
{
  return d1->getPlane() < d2->getPlane();
}

static bool compareStrip(EKLMDigit* d1, EKLMDigit* d2)
{
  return d1->getStrip() < d2->getStrip();
}

static bool compareTime(EKLMDigit* d1, EKLMDigit* d2)
{
  return d1->getTime() < d2->getTime();
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
  addParam("CheckSegmentIntersection", m_CheckSegmentIntersection,
           "Check if segments intersect.", true);
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
  std::vector<EKLMDigit*>::iterator it1, it2, it3, it4, it5, it6, it7, it8, it9;
  n = m_Digits.getEntries();
  for (i = 0; i < n; i++) {
    if (m_Digits[i]->isGood())
      digitVector.push_back(m_Digits[i]);
  }
  /* Sort by sector. */
  sort(digitVector.begin(), digitVector.end(), compareSector);
  it1 = digitVector.begin();
  while (it1 != digitVector.end()) {
    it2 = it1;
    while (1) {
      ++it2;
      if (it2 == digitVector.end())
        break;
      if (!sameSector(*it1, *it2))
        break;
    }
    /* Now it1 .. it2 - hits in a sector. Sort by plane. */
    sort(it1, it2, comparePlane);
    /* If all hits are form the second plane, then continue. */
    if ((*it1)->getPlane() != 1) {
      it1 = it2;
      continue;
    }
    it3 = it1;
    while (1) {
      ++it3;
      if (it3 == it2)
        break;
      if ((*it3)->getPlane() != (*it1)->getPlane())
        break;
    }
    /*
     * Now it1 .. it3 - hits from the first plane, it3 .. it2 - hits from the
     * second plane. If there are no hits from the second plane, then continue.
     */
    if (it3 == it2) {
      it1 = it2;
      continue;
    }
    /* Sort by strip. */
    sort(it1, it3, compareStrip);
    sort(it3, it2, compareStrip);
    it4 = it1;
    while (it4 != it2) {
      it5 = it4;
      while (1) {
        ++it5;
        if (it5 == it2)
          break;
        /* This loop is for both planes so it is necessary to compare planes. */
        if ((*it5)->getStrip() != (*it4)->getStrip() ||
            (*it5)->getPlane() != (*it4)->getPlane())
          break;
      }
      /* Now it4 .. it5 - hits from the same strip. Sort by time. */
      sort(it4, it5, compareTime);
      it4 = it5;
    }
    /* Strip loop. */
    it4 = it1;
    while (it4 != it3) {
      it5 = it4;
      while (1) {
        ++it5;
        if (it5 == it3)
          break;
        if ((*it5)->getStrip() != (*it4)->getStrip())
          break;
      }
      it6 = it3;
      while (it6 != it2) {
        it7 = it6;
        while (1) {
          ++it7;
          if (it7 == it2)
            break;
          if ((*it7)->getStrip() != (*it6)->getStrip())
            break;
        }
        /*
         * Now it4 .. it5 - hits from a single fisrt-plane strip amd
         * it6 .. it7 - hits from a single second-plane strip.
         * If strips do not intersect, then continue.
         */
        HepGeom::Point3D<double> crossPoint(0, 0, 0);
        if (!m_TransformData->intersection(*it4, *it6, &crossPoint,
                                           &d1, &d2, &sd,
                                           m_CheckSegmentIntersection)) {
          it6 = it7;
          continue;
        }
        for (it8 = it4; it8 != it5; ++it8) {
          for (it9 = it6; it9 != it7; ++it9) {
            t1 = getTime(*it8, d1) + 0.5 * sd / Const::speedOfLight;
            t2 = getTime(*it9, d2) - 0.5 * sd / Const::speedOfLight;
            t = (t1 + t2) / 2;
            EKLMHit2d* hit2d = m_Hit2ds.appendNew(*it8);
            hit2d->setEDep((*it8)->getEDep() + (*it9)->getEDep());
            hit2d->setPosition(crossPoint.x(), crossPoint.y(), crossPoint.z());
            hit2d->setChiSq((t1 - t2) * (t1 - t2) /
                            m_RecPar->getTimeResolution() /
                            m_RecPar->getTimeResolution());
            hit2d->setTime(t);
            hit2d->setMCTime(((*it8)->getMCTime() + (*it9)->getMCTime()) / 2);
            hit2d->addRelationTo(*it8);
            hit2d->addRelationTo(*it9);
            for (i = 0; i < 2; i++) {
              EKLMAlignmentHit* alignmentHit = m_AlignmentHits.appendNew(i);
              alignmentHit->addRelationTo(hit2d);
            }
            /* Exit the loop. Equivalent to selection of the earliest hit. */
            break;
          }
          /* Exit the loop. Equivalent to selection of the earliest hit. */
          break;
        }
        it6 = it7;
      }
      it4 = it5;
    }
    it1 = it2;
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

