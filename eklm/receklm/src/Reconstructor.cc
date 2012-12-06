/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Timofey Uglov, Kirill Chilikin                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* Belle2 headers. */
#include <eklm/receklm/Reconstructor.h>
#include <eklm/geoeklm/StripData.h>
#include <framework/gearbox/GearDir.h>
#include <framework/gearbox/Unit.h>

using namespace Belle2;

EKLM::Reconstructor::Reconstructor(struct TransformData* transf)
{
  GearDir Digitizer = GearDir("/Detector/DetectorComponent[@name=\"EKLM\"]"
                              "/Content/Digitizer");
  m_firstPhotonlightSpeed = Digitizer.getDouble("FirstPhotonSpeed");
  m_sigmaT = Digitizer.getDouble("TimeResolution");
  m_transf = transf;
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

  //    EKLMSectorHit *newSectorHit;
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
        new(sectorHitsArray->AddrAt(sectorHitsArray.getEntries()))
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
        TVector3 crossPoint(0, 0, 0);
        // drop entries with non-intersected strips
        double chisq = 0;
        double time = 0;
        if (!(doesIntersect(*itX, *itY, crossPoint, chisq, time)))
          continue;

        EKLMHit2d* hit2d =
          new(m_hit2dArray->AddrAt(m_hit2dArray.getEntries()))
        EKLMHit2d(*itX, *itY);
        hit2d->setCrossPoint(crossPoint);
        hit2d->setChiSq(chisq);
        hit2d->setTime(time);
        m_hit2dVector.push_back(hit2d);
        //hit2d->Print();
      }
    }
  }
}

bool EKLM::Reconstructor::doesIntersect(EKLMDigit* hit1, EKLMDigit* hit2,
                                        TVector3& crossPoint,
                                        double& chisq, double& time)
{
  /* Units are mm. */
  double max1 = EKLM::stripLen[hit1->getStrip() - 1];
  HepGeom::Point3D<double> p1(max1 / 2, 0., 0.);
  // sipm coordinates in a global frame
  HepGeom::Transform3D* tr1 = EKLM::getStripTransform(m_transf, hit1);
  HepGeom::Point3D<double> pt1 = (*tr1) * p1;

  double max2 = EKLM::stripLen[hit2->getStrip() - 1];
  HepGeom::Point3D<double> p2(max2 / 2, 0., 0.);
  HepGeom::Transform3D* tr2 = EKLM::getStripTransform(m_transf, hit2);
  // sipm coordinates in a global frame
  HepGeom::Point3D<double> pt2 = (*tr2) * p2;

  /* Units are cm. */
  crossPoint.SetZ((pt1.z() + pt2.z()) / 2 * Unit::mm);

  double t1 = 0;
  double t2 = 0;

  if (CheckStripOrientationX(hit1)) { // strip is X-oriented
    {
      if (fabs(pt1.x() - pt2.x()) <= max1 &&
          fabs(pt1.y() - pt2.y()) <= max2 &&
          fabs(pt2.x()) <= fabs(pt1.x()) &&
          fabs(pt1.y()) <= fabs(pt2.y())) {   // there is a crossing
        t1 = hit1->getTime() - fabs(pt1.x() - pt2.x()) * Unit::mm /
             m_firstPhotonlightSpeed;
        t2 = hit2->getTime() - fabs(pt2.y() - pt1.y()) * Unit::mm /
             m_firstPhotonlightSpeed;

        crossPoint.SetX(pt2.x()*Unit::mm);
        crossPoint.SetY(pt1.y()*Unit::mm);
        time = (t1 + t2) / 2;
        chisq = (t1 - t2) * (t1 - t2) / m_sigmaT / m_sigmaT;
        return true;
      }
    }
  } else { // Y-oriented strip
    {
      if (fabs(pt1.x() - pt2.x()) <= max2 &&
          fabs(pt1.y() - pt2.y()) <= max1 &&
          fabs(pt1.x()) <= fabs(pt2.x()) &&
          fabs(pt2.y()) <= fabs(pt1.y())) { // there is a crossing
        t1 = hit1->getTime() - fabs(pt1.y() - pt2.y()) * Unit::mm /
             m_firstPhotonlightSpeed;
        t2 = hit2->getTime() - fabs(pt1.x() - pt2.x()) * Unit::mm /
             m_firstPhotonlightSpeed;

        crossPoint.SetX(pt1.x()*Unit::mm);
        crossPoint.SetY(pt2.y()*Unit::mm);

        time = (t1 + t2) / 2;
        chisq = (t1 - t2) * (t1 - t2) / m_sigmaT / m_sigmaT;
        return true;
      }
    }
  }
  crossPoint = TVector3(0., 0., 0.);
  chisq = 0;
  return false; // no crossing found
}

bool EKLM::Reconstructor::CheckStripOrientationX(EKLMDigit* hit)
{
  HepGeom::Transform3D tinv =
    HepGeom::Transform3D(*(EKLM::getStripTransform(m_transf, hit))).inverse();
  return (fabs(sin(tinv.getRotation().phiX())) < 0.01);
}

