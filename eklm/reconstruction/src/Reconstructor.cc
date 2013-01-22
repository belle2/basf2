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

void EKLM::Reconstructor::readStripHits()
{
  StoreArray<EKLMDigit> stripHitsArray;
  for (int i = 0; i < stripHitsArray.getEntries(); i++)
    if (stripHitsArray[i]->isGood())
      m_StripHitVector.push_back(stripHitsArray[i]);
}

void EKLM::Reconstructor::createSectorHits()
{
  StoreArray<EKLMSectorHit> sectorHitsArray;

  for (std::vector<EKLMDigit*>::iterator stripIter =
         m_StripHitVector.begin(); stripIter != m_StripHitVector.end();
       ++stripIter) {
    bool sectorNotFound = true;
    for (std::vector<EKLMSectorHit*>::iterator sectorIter =
           m_SectorHitVector.begin(); sectorIter != m_SectorHitVector.end();
         sectorIter++) {
      // since every hit could be added only once
      if (addStripHitToSector(*sectorIter, *stripIter)) {
        sectorNotFound = false;
        break;
      }
    }
    if (sectorNotFound) {
      EKLMSectorHit* newSectorHit =
        new(sectorHitsArray.nextFreeAddress())
      EKLMSectorHit((*stripIter)->getEndcap(),
                    (*stripIter)->getLayer(),
                    (*stripIter)->getSector());

      addStripHitToSector(newSectorHit, *stripIter);
      m_SectorHitVector.push_back(newSectorHit);
    }
  }

}

bool EKLM::Reconstructor::addStripHitToSector(EKLMSectorHit* sectorHit,
                                              EKLMDigit* stripHit)
{
  if (stripHit->getEndcap() == sectorHit->getEndcap() &&
      stripHit->getLayer() == sectorHit->getLayer() &&
      stripHit->getSector() == sectorHit->getSector()) {
    sectorHit->getStripHitVector()->push_back(stripHit);
    return true;
  }
  return false;


}

void EKLM::Reconstructor::create2dHits()
{
  // loop over sectors

  for (std::vector<EKLMSectorHit*>::iterator sectorIter =
         m_SectorHitVector.begin(); sectorIter != m_SectorHitVector.end();
       sectorIter++) {

    for (std::vector<EKLMDigit*>::iterator itX =
           (*sectorIter)->getStripHitVector()->begin();
         itX != (*sectorIter)->getStripHitVector()->end(); ++itX) {
      // only X strips
      if (!CheckStripOrientationX(*itX))
        continue;
      for (std::vector<EKLMDigit*>::iterator itY =
             (*sectorIter)->getStripHitVector()->begin();
           itY != (*sectorIter)->getStripHitVector()->end(); ++itY) {
        // only Y strips
        if (CheckStripOrientationX(*itY))
          continue;
        HepGeom::Point3D<double> crossPoint(0, 0, 0);
        // drop entries with non-intersected strips
        double chisq = 0;
        double time = 0;
        if (!(doesIntersect(*itX, *itY, &crossPoint, chisq, time)))
          continue;

        EKLMHit2d* hit2d =
          new(m_hit2dArray.nextFreeAddress()) EKLMHit2d(*itX, *itY);
        hit2d->setCrossPoint(&crossPoint);
        hit2d->setChiSq(chisq);
        hit2d->setTime(time);
        m_hit2dVector.push_back(hit2d);
        //hit2d->Print();
      }
    }
  }
}

bool EKLM::Reconstructor::doesIntersect(EKLMDigit* hit1, EKLMDigit* hit2,
                                        HepGeom::Point3D<double> *crossPoint,
                                        double& chisq, double& time)
{
  bool is;
  double d1, d2;
  double t1, t2;
  is = m_geoDat->intersection(hit1, hit2, crossPoint, &d1, &d2);
  if (is == false)
    return false;
  t1 = hit1->getTime() - d1 * Unit::mm / m_firstPhotonlightSpeed;
  t2 = hit2->getTime() - d2 * Unit::mm / m_firstPhotonlightSpeed;
  time = (t1 + t2) / 2;
  chisq = (t1 - t2) * (t1 - t2) / m_sigmaT / m_sigmaT;
  return true;
}

bool EKLM::Reconstructor::CheckStripOrientationX(EKLMDigit* hit)
{
  HepGeom::Transform3D tinv =
    HepGeom::Transform3D(*(EKLM::getStripTransform(&m_geoDat->transf, hit))).inverse();
  return (fabs(sin(tinv.getRotation().phiX())) < 0.01);
}

