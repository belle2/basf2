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
#include <eklm/geometry/EKLMObjectNumbers.h>
#include <eklm/reconstruction/Reconstructor.h>
#include <framework/gearbox/Const.h>
#include <framework/gearbox/GearDir.h>
#include <framework/gearbox/Unit.h>

using namespace Belle2;

static bool comparePlane(EKLMDigit* d1, EKLMDigit* d2)
{
  int s1, s2;
  s1 = EKLM::planeNumber(d1->getEndcap(), d1->getLayer(), d1->getSector(),
                         d1->getPlane());
  s2 = EKLM::planeNumber(d2->getEndcap(), d2->getLayer(), d2->getSector(),
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

EKLM::Reconstructor::Reconstructor(GeometryData* geoDat)
{
  setDefDigitizationParams(&m_digPar);
  m_geoDat = geoDat;
}

bool EKLM::Reconstructor::fastHit(HepGeom::Point3D<double>& pos, double time)
{
  return time < pos.mag() / Const::speedOfLight - 2.0 * m_digPar.timeResolution;
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
        if (!m_geoDat->intersection(*it4, *it5, &crossPoint, &d1, &d2, &sd))
          continue;
        t1 = (*it4)->getTime() - d1 / m_digPar.fiberLightSpeed +
             0.5 * sd / Const::speedOfLight;
        t2 = (*it5)->getTime() - d2 / m_digPar.fiberLightSpeed -
             0.5 * sd / Const::speedOfLight;
        t = (t1 + t2) / 2;
        if (fastHit(crossPoint, t))
          continue;
        EKLMHit2d* hit2d = hit2ds.appendNew(*it4);
        hit2d->setEDep((*it4)->getEDep() + (*it5)->getEDep());
        hit2d->setGlobalPosition(crossPoint);
        hit2d->setChiSq((t1 - t2) * (t1 - t2) /
                        m_digPar.timeResolution / m_digPar.timeResolution);
        hit2d->setTime(t);
        hit2d->setMCTime(((*it4)->getMCTime() + (*it5)->getMCTime()) / 2);
        hit2d->addRelationTo(*it4);
        hit2d->addRelationTo(*it5);
      }
    }
    it = it3;
  }
}

