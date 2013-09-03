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
#include <eklm/reconstruction/Reconstructor.h>
#include <framework/gearbox/Const.h>
#include <framework/gearbox/GearDir.h>
#include <framework/gearbox/Unit.h>

using namespace Belle2;

EKLM::Reconstructor::Reconstructor(GeometryData* geoDat)
{
  GearDir Digitizer = GearDir("/Detector/DetectorComponent[@name=\"EKLM\"]"
                              "/Content/Digitizer");
  m_firstPhotonlightSpeed = Digitizer.getDouble("FirstPhotonSpeed");
  m_sigmaT = Digitizer.getDouble("TimeResolution");
  m_geoDat = geoDat;
}

bool EKLM::Reconstructor::fastHit(HepGeom::Point3D<double>& pos, double time)
{
  return time < pos.mag() / Const::speedOfLight - 2.0 * m_sigmaT;
}

void EKLM::Reconstructor::readStripHits()
{
  StoreArray<EKLMDigit> stripHitsArray;
  for (int i = 0; i < stripHitsArray.getEntries(); i++)
    if (stripHitsArray[i]->isGood())
      m_StripHitVector.push_back(stripHitsArray[i]);
}

void EKLM::Reconstructor::createSectorHits()
{
  for (std::vector<EKLMDigit*>::iterator stripIter =
         m_StripHitVector.begin(); stripIter != m_StripHitVector.end();
       ++stripIter) {
    bool sectorNotFound = true;
    for (std::vector<EKLMSectorHit*>::iterator sectorIter =
           m_SectorHitVector.begin(); sectorIter != m_SectorHitVector.end();
         sectorIter++) {
      // since every hit could be added only once
      if ((*sectorIter)->addHit(*stripIter) == 0) {
        sectorNotFound = false;
        break;
      }
    }
    if (sectorNotFound) {
      EKLMSectorHit* newSectorHit =
        new EKLMSectorHit((*stripIter)->getEndcap(),
                          (*stripIter)->getLayer(),
                          (*stripIter)->getSector());

      newSectorHit->addHit(*stripIter);
      m_SectorHitVector.push_back(newSectorHit);
    }
  }

}

void EKLM::Reconstructor::create2dHits()
{
  std::vector<EKLMSectorHit*>::iterator it;
  int n1, n2;
  int i1, i2;
  EKLMDigit* hit1, *hit2;
  for (it = m_SectorHitVector.begin(); it != m_SectorHitVector.end(); it++) {
    n1 = (*it)->getHitNumber(1);
    n2 = (*it)->getHitNumber(2);
    for (i1 = 0; i1 < n1; i1++) {
      hit1 = (*it)->getHit(1, i1);
      for (i2 = 0; i2 < n2; i2++) {
        hit2 = (*it)->getHit(2, i2);
        HepGeom::Point3D<double> crossPoint(0, 0, 0);
        double chisq = 0;
        double time = 0;
        if (!(doesIntersect(hit1, hit2, &crossPoint, chisq, time)))
          continue;
        if (fastHit(crossPoint, time))
          continue;
        EKLMHit2d* hit2d =
          new(m_hit2dArray.nextFreeAddress()) EKLMHit2d(hit1, hit2);
        hit2d->setEDep(hit1->getEDep() + hit2->getEDep());
        hit2d->setGlobalPosition(crossPoint);
        hit2d->setChiSq(chisq);
        hit2d->setTime(time);
        hit2d->setMCTime((hit1->getMCTime() + hit2->getMCTime()) / 2);
      }
    }
  }
}

bool EKLM::Reconstructor::doesIntersect(EKLMDigit* hit1, EKLMDigit* hit2,
                                        HepGeom::Point3D<double>* crossPoint,
                                        double& chisq, double& time)
{
  bool is;
  double d1, d2;
  double t1, t2;
  double sd;
  is = m_geoDat->intersection(hit1, hit2, crossPoint, &d1, &d2, &sd);
  if (is == false)
    return false;
  t1 = hit1->getTime() - d1 / m_firstPhotonlightSpeed +
       0.5 * sd / Const::speedOfLight;
  t2 = hit2->getTime() - d2 / m_firstPhotonlightSpeed -
       0.5 * sd / Const::speedOfLight;
  time = (t1 + t2) / 2;
  chisq = (t1 - t2) * (t1 - t2) / m_sigmaT / m_sigmaT;
  return true;
}

