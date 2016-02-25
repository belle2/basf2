/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Timofey Uglov, Kirill Chilikin                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* External headers. */
#include <TFile.h>
#include <TTree.h>

/* Belle2 headers. */
#include <eklm/geometry/GeometryData.h>
#include <eklm/reconstruction/Reconstructor.h>
#include <framework/gearbox/Const.h>
#include <framework/gearbox/GearDir.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>
#include <framework/utilities/FileSystem.h>

using namespace Belle2;

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

EKLM::Reconstructor::Reconstructor(TransformData* transformData)
{
  int i, n;
  float p0, p1;
  TFile* f;
  TTree* t;
  setDefDigitizationParams(&m_digPar);
  m_TransformData = transformData;
  m_GeoDat = &(EKLM::GeometryData::Instance());
  n = m_GeoDat->getNStripsDifferentLength();
  m_TimeParams = new struct TimeParams[n];
  f = new TFile(FileSystem::findFile(
                  "/data/eklm/TimeCalibration.root").c_str());
  if (f->IsZombie())
    B2FATAL("Cannot open time calibration data file.");
  t = (TTree*)f->Get("t_calibration");
  t->SetBranchAddress("p0", &p0);
  t->SetBranchAddress("p1", &p1);
  for (i = 0; i < n; i++) {
    t->GetEntry(i);
    m_TimeParams[i].p0 = p0;
    m_TimeParams[i].p1 = p1;
  }
  delete t;
  delete f;
}

EKLM::Reconstructor::~Reconstructor()
{
  delete[] m_TimeParams;
}

bool EKLM::Reconstructor::fastHit(HepGeom::Point3D<double>& pos, double time)
{
  return time < pos.mag() / Const::speedOfLight - 2.0 * m_digPar.timeResolution;
}

double EKLM::Reconstructor::getTime(EKLMDigit* d, double dist)
{
  int n;
  n = EKLM::GeometryData::Instance().getStripLengthIndex(d->getStrip() - 1);
  return d->getTime() - (dist * m_TimeParams[n].p0 + m_TimeParams[n].p1);
}

void EKLM::Reconstructor::create2dHits()
{
  int i, n;
  double d1, d2, t, t1, t2, sd;
  StoreArray<EKLMDigit> digits;
  StoreArray<EKLMHit2d> hit2ds;
  std::vector<EKLMDigit*> digitVector;
  std::vector<EKLMDigit*>::iterator it, it2, it3, it4, it5;
  n = digits.getEntries();
  for (i = 0; i < n; i++)
    if (digits[i]->isGood())
      digitVector.push_back(digits[i]);
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
        if (fastHit(crossPoint, t))
          continue;
        EKLMHit2d* hit2d = hit2ds.appendNew(*it4);
        hit2d->setEDep((*it4)->getEDep() + (*it5)->getEDep());
        hit2d->setPosition(crossPoint.x(), crossPoint.y(), crossPoint.z());
        hit2d->setChiSq((t1 - t2) * (t1 - t2) /
                        m_digPar.timeResolution / m_digPar.timeResolution);
        hit2d->setTime(t);
        hit2d->setMCTime(((*it4)->getMCTime() + (*it5)->getMCTime()) / 2);
        hit2d->addRelationTo(*it4);
        hit2d->addRelationTo(*it5);
        /* cppcheck-suppress memleak */
      }
    }
    it = it3;
  }
}

