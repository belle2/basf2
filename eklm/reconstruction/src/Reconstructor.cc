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
  setDefDigitizationParams(&m_digPar);
  m_geoDat = geoDat;
}

bool EKLM::Reconstructor::fastHit(HepGeom::Point3D<double>& pos, double time)
{
  return time < pos.mag() / Const::speedOfLight - 2.0 * m_digPar.timeResolution;
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
  double d1, d2;
  double t, t1, t2;
  double sd;
  EKLMDigit* hit1, *hit2;
  for (it = m_SectorHitVector.begin(); it != m_SectorHitVector.end(); it++) {
    n1 = (*it)->getHitNumber(1);
    n2 = (*it)->getHitNumber(2);
    for (i1 = 0; i1 < n1; i1++) {
      hit1 = (*it)->getHit(1, i1);
      for (i2 = 0; i2 < n2; i2++) {
        hit2 = (*it)->getHit(2, i2);
        HepGeom::Point3D<double> crossPoint(0, 0, 0);
        if (!m_geoDat->intersection(hit1, hit2, &crossPoint, &d1, &d2, &sd))
          continue;
        t1 = hit1->getTime() - d1 / m_digPar.fiberLightSpeed +
             0.5 * sd / Const::speedOfLight;
        t2 = hit2->getTime() - d2 / m_digPar.fiberLightSpeed -
             0.5 * sd / Const::speedOfLight;
        t = (t1 + t2) / 2;
        if (fastHit(crossPoint, t))
          continue;
        EKLMHit2d* hit2d = m_hit2dArray.appendNew(hit1, hit2);
        hit2d->setEDep(hit1->getEDep() + hit2->getEDep());
        hit2d->setGlobalPosition(crossPoint);
        hit2d->setChiSq((t1 - t2) * (t1 - t2) /
                        m_digPar.timeResolution / m_digPar.timeResolution);
        hit2d->setTime(t);
        hit2d->setMCTime((hit1->getMCTime() + hit2->getMCTime()) / 2);
      }
    }
    delete *it;
  }
}

