/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Timofey Uglov                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <eklm/receklm/EKLMRecon.h>
#include <eklm/geoeklm/G4PVPlacementGT.h>
#include "G4TransportationManager.hh"
#include "G4Navigator.hh"
#include "G4Box.hh"
#include <framework/gearbox/GearDir.h>
#include <framework/gearbox/Unit.h>

using namespace std;
namespace Belle2 {


  EKLMRecon::EKLMRecon()
  {
    GearDir Digitizer = GearDir("/Detector/DetectorComponent[@name=\"EKLM\"]/Content/Digitizer");
    m_firstPhotonlightSpeed = Digitizer.getDouble("FirstPhotonSpeed");
    m_sigmaT = Digitizer.getDouble("TimeResolution");
  }

  void EKLMRecon::readStripHits()
  {
    StoreArray<EKLMStripHit> stripHitsArray;
    for (int i = 0; i < stripHitsArray.getEntries(); i++)
      if (stripHitsArray[i]->isGood())
        m_StripHitVector.push_back(stripHitsArray[i]);
  }


  void EKLMRecon::createSectorHits()
  {

    //    EKLMSectorHit *newSectorHit;
    StoreArray<EKLMSectorHit> sectorHitsArray;

    for (vector<EKLMStripHit*>::iterator stripIter =
           m_StripHitVector.begin(); stripIter != m_StripHitVector.end();
         ++stripIter) {
      bool sectorNotFound = true;
      for (vector<EKLMSectorHit*>::iterator sectorIter =
             m_SectorHitVector.begin(); sectorIter != m_SectorHitVector.end();
           sectorIter++) {
        // since every hit could be added only once
        if (addStripHitToSector(*sectorIter, *stripIter)) {
          sectorNotFound = false;
          break;
        }
      }
      if (sectorNotFound) {
        EKLMSectorHit* newSectorHit = new(sectorHitsArray->AddrAt(sectorHitsArray.getEntries()))
        EKLMSectorHit((*stripIter)->getEndcap(),
                      (*stripIter)->getLayer(),
                      (*stripIter)->getSector());

        addStripHitToSector(newSectorHit, *stripIter);
        m_SectorHitVector.push_back(newSectorHit);
      }
    }

  }

  bool EKLMRecon::addStripHitToSector(EKLMSectorHit* sectorHit , EKLMStripHit* stripHit)
  {
    if (stripHit->getEndcap() == sectorHit->getEndcap() &&
        stripHit->getLayer() == sectorHit->getLayer() &&
        stripHit->getSector() == sectorHit->getSector()) {
      sectorHit->getStripHitVector()->push_back(stripHit);
      return true;
    }
    return false;


  }

  void EKLMRecon::create2dHits()
  {
    // loop over sectors

    for (vector<EKLMSectorHit*>::iterator sectorIter =
           m_SectorHitVector.begin(); sectorIter != m_SectorHitVector.end();
         sectorIter++) {

      vector<EKLMStripHit*>::iterator itX = ((*sectorIter)->getStripHitVector())->begin();

      for (vector<EKLMStripHit*>::iterator itX = (*sectorIter)->getStripHitVector()->begin();
           itX != (*sectorIter)->getStripHitVector()->end(); ++itX) {
        // only X strips
        if (!CheckStripOrientationX((*itX)->getVolume()))
          continue;
        for (vector<EKLMStripHit*>::iterator itY = (*sectorIter)->getStripHitVector()->begin();
             itY != (*sectorIter)->getStripHitVector()->end(); ++itY) {
          // only Y strips
          if (CheckStripOrientationX((*itY)->getVolume()))
            continue;
          TVector3 crossPoint(0, 0, 0);
          // drop entries with non-intersected strips
          double chisq = 0;
          double time = 0;
          if (!(doesIntersect(*itX, *itY, crossPoint, chisq, time)))
            continue;

          EKLMHit2d* hit2d = new(m_hit2dArray->AddrAt(m_hit2dArray.getEntries()))EKLMHit2d(*itX, *itY);
          hit2d->setCrossPoint(crossPoint);
          hit2d->setChiSq(chisq);
          hit2d->setTime(time);
          m_hit2dVector.push_back(hit2d);
          //hit2d->Print();
        }
      }
    }

  }


  bool EKLMRecon::doesIntersect(const EKLMStripHit* hit1, const EKLMStripHit* hit2,
                                TVector3& crossPoint, double& chisq, double& time)
  {
    // general remark
    // G4 native units are mm while we use cm
    // Thus we need to convert everythins to cm
    // However i do not like to get deep into trasformations etc, so I convert sizes only before filling
    // output values


    // actually, it is not too safe to use GetConstituentSolid(0).
    // here we rely on the fact that the first child is a real strip G4box
    // see similar problem in EKLMFiberAndElectronics::lightPropagationDistance function
    G4Box* box1 = (G4Box*)(hit1->getVolume()->GetLogicalVolume()->GetSolid()->GetConstituentSolid(0));
    double max1 = 2.0 * box1->GetXHalfLength();
    HepGeom::Point3D<double> p1(0.5 * max1, 0., 0.);
    HepGeom::Point3D<double> sipm1(0., 0., 0.);
    // strip centre coordinates in a global frame
    HepGeom::Point3D<double> pt1 = ((G4PVPlacementGT*)(hit1->getVolume()))-> getTransform() * p1;
    // sipm coordinates in a global frame
    HepGeom::Point3D<double> sipmt1 = ((G4PVPlacementGT*)(hit1->getVolume()))-> getTransform() *  sipm1;

    // actually, it is not too safe to use GetConstituentSolid(0).
    // here we rely on the fact that the first child is a real strip G4box
    // similar problem is in EKLMFiberAndElectronics::lightPropagationDistance function
    G4Box* box2 = (G4Box*)(hit2->getVolume()->GetLogicalVolume()->GetSolid()->GetConstituentSolid(0));
    double max2 = 2.0 * box2->GetXHalfLength();
    HepGeom::Point3D<double> p2(0.5 * max2, 0., 0.);
    HepGeom::Point3D<double> sipm2(0., 0., 0.);
    // strip centre coordinates in a global frame
    HepGeom::Point3D<double> pt2 = ((G4PVPlacementGT*)(hit2->getVolume()))->getTransform() * p2;
    // sipm coordinates in a global frame
    HepGeom::Point3D<double> sipmt2 = ((G4PVPlacementGT*)(hit2->getVolume()))-> getTransform() *  sipm2;

    crossPoint.SetZ((pt1.z() + pt2.z()) / 2 * Unit::mm);

    double t1 = 0;
    double t2 = 0;

    if (CheckStripOrientationX(hit1->getVolume())) { // strip is X-oriented
      {
        if (fabs(pt1.x() - pt2.x()) <= max1 && fabs(pt1.y() - pt2.y()) <= max2 &&
            fabs(pt2.x()) <= fabs(pt1.x()) && fabs(pt1.y()) <= fabs(pt2.y())) {   // there is a crossing
          t1 = hit1->getTime() - fabs(sipmt1.x() - pt2.x()) * Unit::mm / m_firstPhotonlightSpeed;
          t2 = hit2->getTime() - fabs(sipmt2.y() - pt1.y()) * Unit::mm / m_firstPhotonlightSpeed;
          crossPoint.SetX(pt2.x()*Unit::mm);
          crossPoint.SetY(pt1.y()*Unit::mm);
          time = (t1 + t2) / 2;
          chisq = (t1 - t2) * (t1 - t2) / m_sigmaT / m_sigmaT;
          return true;
        }
      }
    } else { // Y-oriented strip
      {
        if (fabs(pt1.x() - pt2.x()) <= max2 && fabs(pt1.y() - pt2.y()) <= max1 &&
            fabs(pt1.x()) <= fabs(pt2.x()) && fabs(pt2.y()) <= fabs(pt1.y())) { // there is a crossing
          t1 = hit1->getTime() - fabs(sipmt1.y() - pt2.y()) * Unit::mm / m_firstPhotonlightSpeed;
          t2 = hit2->getTime() - fabs(sipmt2.x() - pt1.x()) * Unit::mm / m_firstPhotonlightSpeed;
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


  bool EKLMRecon::CheckStripOrientationX(const G4VPhysicalVolume* strip)
  {
    G4Transform3D t = ((G4PVPlacementGT*)strip)->getTransform().inverse();
    return (fabs(sin(t.getRotation().phiX())) < 0.01);
  }


}//namespace
