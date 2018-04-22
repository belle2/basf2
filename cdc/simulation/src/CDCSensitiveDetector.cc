/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Guofu Cao                                                *
 * 2012.03.05 SensitiveDetector -> CDCSensitiveDetector by M. Uchida      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <cdc/simulation/CDCSensitiveDetector.h>

#include <cdc/simulation/CDCSimControlPar.h>
#include <cdc/simulation/Helix.h>
#include <cdc/geometry/CDCGeometryPar.h>
#include <cdc/geometry/GeoCDCCreator.h>
#include <cdc/utilities/ClosestApproach.h>
#include <framework/logging/Logger.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>
#include <framework/gearbox/Unit.h>
#include <cdc/dataobjects/CDCSimHit.h>


#include "G4Step.hh"
#include "G4SteppingManager.hh"
#include "G4SDManager.hh"
#include "G4TransportationManager.hh"
#include "G4FieldManager.hh"
#include "G4MagneticField.hh"

#include "CLHEP/Geometry/Vector3D.h"
#include "CLHEP/Geometry/Point3D.h"

#include "TVector3.h"

//#include <iomanip>

#ifndef ENABLE_BACKWARDS_COMPATIBILITY
typedef HepGeom::Point3D<double> HepPoint3D;
#endif
#ifndef ENABLE_BACKWARDS_COMPATIBILITY
typedef HepGeom::Vector3D<double> HepVector3D;
#endif



namespace Belle2 {
  using namespace CDC;

  //N.B.#0: Do not put CDCGeometryPar::Instance() in the initializing list of the constructor,
  //because it is called before CDCGeometryPar(geom) is called in case of UseDB=true of Geometry module.
  //N.B.#1:  Do not call AddNewDetector(), because it'll cause a job crash currently.

  CDCSensitiveDetector::CDCSensitiveDetector(G4String name, G4double thresholdEnergyDeposit, G4double thresholdKineticEnergy):
    SensitiveDetectorBase(name, Const::CDC),
    //    m_cdcgp(CDCGeometryPar::Instance()),
    m_cdcgp(nullptr),
    m_thresholdEnergyDeposit(thresholdEnergyDeposit),
    m_thresholdKineticEnergy(thresholdKineticEnergy), m_hitNumber(0)
  {
    StoreArray<MCParticle> mcParticles;
    StoreArray<CDCSimHit> cdcSimHits;
    RelationArray cdcSimHitRel(mcParticles, cdcSimHits);
    registerMCParticleRelation(cdcSimHitRel);
    //    registerMCParticleRelation(cdcSimHitRel, RelationArray::c_doNothing);
    //    registerMCParticleRelation(cdcSimHitRel, RelationArray::c_negativeWeight);
    //    registerMCParticleRelation(cdcSimHitRel, RelationArray::c_deleteElement);
    //    registerMCParticleRelation(cdcSimHitRel, RelationArray::c_zeroWeight);
    cdcSimHits.registerInDataStore();
    mcParticles.registerRelationTo(cdcSimHits);

    CDCSimControlPar& cntlp = CDCSimControlPar::getInstance();

    m_thresholdEnergyDeposit = cntlp.getThresholdEnergyDeposit();
    m_thresholdEnergyDeposit *= CLHEP::GeV;  //GeV to MeV (=unit in G4)
    B2DEBUG(150, "CDCSensitiveDetector: Threshold energy (MeV): " << m_thresholdEnergyDeposit);
    m_thresholdKineticEnergy = 0.0; // Dummy to avoid a warning (tentative).

    m_wireSag = cntlp.getWireSag();
    //    m_wireSag = false;
    B2DEBUG(150, "CDCSensitiveDetector: Sense wire sag on(=1)/off(=0): " << m_wireSag);

    m_modifiedLeftRightFlag = cntlp.getModLeftRightFlag();
    B2DEBUG(150, "CDCSensitiveDetector: Set left/right flag modified for tracking (=1)/ not set (=0): " << m_modifiedLeftRightFlag);

    m_minTrackLength = cntlp.getMinTrackLength();
    m_minTrackLength *= CLHEP::cm;  //cm to mm (=unit in G4)
    B2DEBUG(150, "CDCSensitiveDetector: MinTrackLength (mm): " << m_minTrackLength);

    //For activating Initialize and EndOfEvent functions
    //but not work --> commented out  for a while...
    //    if (m_modifiedLeftRightFlag) {
    //      G4SDManager::GetSDMpointer()->AddNewDetector(this);
    //    }
  }

  void CDCSensitiveDetector::Initialize(G4HCofThisEvent*)
  {
    /*
    m_cdcgp = &CDCGeometryPar::Instance();

    m_thresholdEnergyDeposit = m_cdcgp->getThresholdEnergyDeposit();
    m_thresholdEnergyDeposit *= CLHEP::GeV;  //GeV to MeV (=unit in G4)
    B2INFO("CDCSensitiveDetector: Threshold energy (MeV): " << m_thresholdEnergyDeposit);
    m_modifiedLeftRightFlag = m_cdcgp->isModifiedLeftRightFlagOn();
    B2INFO("CDCSensitiveDetector: Set left/right flag modified for tracking (=1)/ not set (=0): " << m_modifiedLeftRightFlag);

    m_minTrackLength = m_cdcgp->getMinTrackLength();
    m_minTrackLength *= CLHEP::cm;  //cm to mm (=unit in G4)
    B2INFO("CDCSensitiveDetector: MinTrackLength (mm): " << m_minTrackLength);
    */

    // Initialize
    m_nonUniformField = 0;
    //    std::cout << "Initialize called" << std::endl;
    //    m_nPosHits = 0;
    //    m_nNegHits = 0;
  }

  //-----------------------------------------------------
  // Method invoked for every step in sensitive detector
  //-----------------------------------------------------
  bool CDCSensitiveDetector::step(G4Step* aStep, G4TouchableHistory*)
  {
    //    static bool firstCall = true;
    //    if (firstCall) {
    //      firstCall = false;
    m_cdcgp = &CDCGeometryPar::Instance();
    //      CDCSimControlPar & m_cntlp   = CDCSimControlPar::getInstance();

    //      //      m_thresholdEnergyDeposit = m_cdcgp->getThresholdEnergyDeposit();
    //      m_thresholdEnergyDeposit = m_cntlp.getThresholdEnergyDeposit();
    //      m_thresholdEnergyDeposit *= CLHEP::GeV;  //GeV to MeV (=unit in G4)
    //      B2INFO("CDCSensitiveDetector: Threshold energy (MeV): " << m_thresholdEnergyDeposit);

    //      //      m_modifiedLeftRightFlag = m_cdcgp->isModifiedLeftRightFlagOn();
    //      m_modifiedLeftRightFlag = m_cntlp.getModLeftRightFlag();
    //      B2INFO("CDCSensitiveDetector: Set left/right flag modified for tracking (=1)/ not set (=0): " << m_modifiedLeftRightFlag);

    //      //      m_minTrackLength = m_cdcgp->getMinTrackLength();
    //      m_minTrackLength = m_cntlp.getMinTrackLength();
    //      m_minTrackLength *= CLHEP::cm;  //cm to mm (=unit in G4)
    //      B2INFO("CDCSensitiveDetector: MinTrackLength (mm): " << m_minTrackLength);

    //      //      m_wireSag = m_cdcgp->isWireSagOn();
    //      m_wireSag = m_cntlp.getWireSag();

    m_nonUniformField = 0;
    //    }

#if defined(CDC_DEBUG)
    std::cout << " " << std::endl;
    std::cout << "********* step in ********" << std::endl;
#endif
    // Get deposited energy
    const G4double edep = aStep->GetTotalEnergyDeposit();


    // Discard the hit below Edep_th
    if (edep <= m_thresholdEnergyDeposit) return false;

    // Get step length
    const G4double stepLength = aStep->GetStepLength();
    if (stepLength == 0.) return false;

    // Get step information
    const G4Track& t = * aStep->GetTrack();

    G4double hitWeight = Simulation::TrackInfo::getInfo(t).getIgnore() ? -1 : 1;
    // save in MCParticle if track-length is enough long
    if (t.GetTrackLength() > m_minTrackLength) {
      Simulation::TrackInfo::getInfo(t).setIgnore(false);
      hitWeight = 1.;
      //     std::cout <<"setignore=false for track= "<< t.GetTrackID() << std::endl;
      //    } else {
      //      std::cout <<"setignore=true for track= "<< t.GetTrackID() << std::endl;
    }

    const G4double charge = t.GetDefinition()->GetPDGCharge();

    //    const G4double tof = t.GetGlobalTime(); //tof at post step point
    //    if (isnan(tof)) {
    //      B2ERROR("SensitiveDetector: global time is nan");
    //      return false;
    //    }

    const G4int pid = t.GetDefinition()->GetPDGEncoding();
    const G4int trackID = t.GetTrackID();
    //    std::cout << "pid,stepl,trackID,trackl,weight= " << pid <<" "<< stepLength <<" "<< trackID <<" "<< t.GetTrackLength() <<" "<< hitWeight << std::endl;

    const G4VPhysicalVolume& v = * t.GetVolume();
    const G4StepPoint& in = * aStep->GetPreStepPoint();
    const G4StepPoint& out = * aStep->GetPostStepPoint();
    const G4ThreeVector& posIn = in.GetPosition();
    const G4ThreeVector& posOut = out.GetPosition();
    const G4ThreeVector momIn(in.GetMomentum().x(), in.GetMomentum().y(),
                              in.GetMomentum().z());
#if defined(CDC_DEBUG)
    std::cout << "pid   = " << pid  << std::endl;
    std::cout << "mass  = " << t.GetDefinition()->GetPDGMass() << std::endl;
    std::cout << "posIn = " << posIn << std::endl;
    std::cout << "posOut= " << posOut << std::endl;
    std::cout << "tof at post-step  = " << out.GetGlobalTime() << std::endl;
    std::cout << "stepl = " << stepLength << std::endl;
#endif

    // Get layer ID
    const unsigned layerId = v.GetCopyNo();
    B2DEBUG(150, "LayerID in continuous counting method: " << layerId);

    // If neutral particles, ignore them.

    if (charge == 0.) return false;

    // Calculate cell ID
    TVector3 tposIn(posIn.x() / CLHEP::cm, posIn.y() / CLHEP::cm, posIn.z() / CLHEP::cm);
    TVector3 tposOut(posOut.x() / CLHEP::cm, posOut.y() / CLHEP::cm, posOut.z() / CLHEP::cm);
    const unsigned idIn = m_cdcgp->cellId(layerId, tposIn);
    const unsigned idOut = m_cdcgp->cellId(layerId, tposOut);
#if defined(CDC_DEBUG)
    std::cout << "edep= " << edep << std::endl;
    std::cout << "idIn,idOut= " << idIn << " " << idOut << std::endl;
#endif

    // Calculate drift length
    std::vector<int> wires = WireId_in_hit_order(idIn, idOut, m_cdcgp->nWiresInLayer(layerId));
    G4double sint(0.);
    const G4double s_in_layer = stepLength / CLHEP::cm;
    G4double xint[6] = {0};

    const G4ThreeVector momOut(out.GetMomentum().x(), out.GetMomentum().y(),
                               out.GetMomentum().z());
    const G4double speedIn  =  in.GetVelocity();
    const G4double speedOut = out.GetVelocity();
    const G4double speed    = 0.5 * (speedIn + speedOut);
    const G4double speedInCmPerNs = speed / CLHEP::cm;

    const unsigned int nWires = wires.size();
    G4double tofBefore = in.GetGlobalTime();
    G4double kinEnergyBefore = in.GetKineticEnergy();
    G4double momBefore = momIn.mag();
    const G4double eLoss = kinEnergyBefore - out.GetKineticEnergy(); //n.b. not always equal to edep
    const G4double mass = t.GetDefinition()->GetPDGMass();
#if defined(CDC_DEBUG)
    std::cout << "momBefore = " << momBefore << std::endl;
    std::cout << "momIn = " << momIn.x() << " " <<  momIn.y() << " " << momIn.z() << std::endl;
    std::cout << "momOut= " << momOut.x() << " " <<  momOut.y() << " " << momOut.z() << std::endl;
    std::cout << "speedIn,speedOut= " << speedIn << " " << speedOut << std::endl;
    std::cout << " speedInCmPerNs= " <<  speedInCmPerNs << std::endl;
    std::cout << "tofBefore= " <<  tofBefore << std::endl;
#endif

    const G4Field* field = G4TransportationManager::GetTransportationManager()->GetFieldManager()->GetDetectorField();

    for (unsigned i = 0; i < nWires; ++i) {
#if defined(CDC_DEBUG)
      std::cout << "============ i,wires[i]= " << i << " " << wires[i] << std::endl;
#endif

      const G4double pos[3] = {posIn.x(), posIn.y(), posIn.z()};
      G4double Bfield[3];
      field->GetFieldValue(pos, Bfield);
      m_magneticField = (Bfield[0] == 0. && Bfield[1] == 0. &&
                         Bfield[2] == 0.) ? false : true;
#if defined(CDC_DEBUG)
      std::cout << "Bfield= " << Bfield[0] << " " << Bfield[1] << " " << Bfield[2] << std::endl;
      std::cout << "magneticField= " << m_magneticField << std::endl;
#endif

      double distance = 0;
      G4ThreeVector posW(0, 0, 0);
      HepPoint3D onTrack;
      HepPoint3D pOnTrack;

      // Calculate forward/backward position of current wire
      const TVector3 tfw3v = m_cdcgp->wireForwardPosition(layerId, wires[i]);
      const TVector3 tbw3v = m_cdcgp->wireBackwardPosition(layerId, wires[i]);

      const HepPoint3D fwd(tfw3v.x(), tfw3v.y(), tfw3v.z());
      const HepPoint3D bck(tbw3v.x(), tbw3v.y(), tbw3v.z());

      if (m_magneticField) {
        // Cal. distance assuming helix track (still approximation)
        m_nonUniformField = 1;
        if (Bfield[0] == 0. && Bfield[1] == 0. &&
            Bfield[2] != 0.) m_nonUniformField = 0;

        const G4double B_kG[3] = {Bfield[0] / CLHEP::kilogauss,
                                  Bfield[1] / CLHEP::kilogauss,
                                  Bfield[2] / CLHEP::kilogauss
                                 };
#if defined(CDC_DEBUG)
        std::cout << "B_kG= " << B_kG[0] << " " << B_kG[1] << " " << B_kG[2] << std::endl;
        std::cout << "magneticField= " << m_magneticField << std::endl;
#endif

        const HepPoint3D  x(pos[0] / CLHEP::cm, pos[1] / CLHEP::cm, pos[2] / CLHEP::cm);
        const HepVector3D p(momIn.x() / CLHEP::GeV, momIn.y() / CLHEP::GeV, momIn.z() / CLHEP::GeV);
        Helix tmp(x, p, charge);
        tmp.bFieldZ(B_kG[2]);
        tmp.ignoreErrorMatrix();

        /*  // Calculate forward/backward position of current wire
          const TVector3 tfw3v = cdcg.wireForwardPosition(layerId, wires[i]);
          const TVector3 tbw3v = cdcg.wireBackwardPosition(layerId, wires[i]);

          const HepPoint3D fwd(tfw3v.x(), tfw3v.y(), tfw3v.z());
          const HepPoint3D bck(tbw3v.x(), tbw3v.y(), tbw3v.z());
        */

        const HepVector3D wire = fwd - bck;
        HepPoint3D tryp =
          (x.z() - bck.z()) / wire.z() * wire + bck;
        tmp.pivot(tryp);
        tryp = (tmp.x(0.).z() - bck.z()) / wire.z() * wire + bck;
        tmp.pivot(tryp);
        tryp = (tmp.x(0.).z() - bck.z()) / wire.z() * wire + bck;
        tmp.pivot(tryp);

        distance = std::abs(tmp.a()[0]);
        posW.setX(tryp.x());
        posW.setY(tryp.y());
        posW.setZ(tryp.z());

        //  HepPoint3D onTrack = tmp.x(0.);
        onTrack = tmp.x(0.);
        pOnTrack = tmp.momentum(0.);

        for_Rotat(B_kG);
        const G4double xwb(bck.x()), ywb(bck.y()), zwb(bck.z());
        const G4double xwf(fwd.x()), ywf(fwd.y()), zwf(fwd.z());
        const G4double xp(onTrack.x()), yp(onTrack.y()), zp(onTrack.z());
        const G4double px(pOnTrack.x()), py(pOnTrack.y()), pz(pOnTrack.z());
        G4double q2[3] = {0.}, q1[3] = {0.}, q3[3] = {0.};
        const G4int ntryMax(50);  //tentative; too large probably...
        G4double dist;
        G4int ntry(999);
        HELWIR(xwb, ywb, zwb, xwf, ywf, zwf,
               xp,   yp,   zp,   px,   py,   pz,
               B_kG, charge, ntryMax, dist, q2, q1, q3, ntry);

#if defined(CDC_DEBUG)
        std::cout << "ntry= " << ntry << std::endl;
        std::cout << "bf distance= " << distance << std::endl;
        std::cout << "onTrack    = " << onTrack  << std::endl;
        std::cout << "posW       = " << posW     << std::endl;
#endif
        if (ntry <= ntryMax) {
          if (m_wireSag) {
            G4double ywb_sag, ywf_sag;
            m_cdcgp->getWireSagEffect(CDCGeometryPar::c_Base, layerId, wires[i], q2[2], ywb_sag, ywf_sag);
            HELWIR(xwb, ywb_sag, zwb, xwf, ywf_sag, zwf,
                   xp,   yp,   zp,   px,   py,   pz,
                   B_kG, charge, ntryMax, dist, q2, q1, q3, ntry);
          }
          if (ntry <= ntryMax) {
            distance = dist;
            onTrack.setX(q1[0]);
            onTrack.setY(q1[1]);
            onTrack.setZ(q1[2]);
            posW.setX(q2[0]);
            posW.setY(q2[1]);
            posW.setZ(q2[2]);
            pOnTrack.setX(q3[0]);
            pOnTrack.setY(q3[1]);
            pOnTrack.setZ(q3[2]);
          }
#if defined(CDC_DEBUG)
          std::cout << " " << std::endl;
          std::cout << "helix distance= " << distance << std::endl;
          std::cout << "onTrack = " << onTrack  << std::endl;
          std::cout << "posW    = " << posW     << std::endl;
          std::cout << "pOnTrack= " << pOnTrack << std::endl;
          G4ThreeVector bwp(bck.x(), bck.y(), bck.z());
          G4ThreeVector fwp(fwd.x(), fwd.y(), fwd.z());
          G4ThreeVector hitPosition, wirePosition;
          distance = ClosestApproach(bwp, fwp, posIn / CLHEP::cm, posOut / CLHEP::cm,
                                     hitPosition, wirePosition);
          if (m_wireSag) {
            G4double ywb_sag, ywf_sag;
            m_cdcgp->getWireSagEffect(CDCGeometryPar::c_Base, layerId, wires[i], wirePosition.z(), ywb_sag, ywf_sag);
            bwp.setY(ywb_sag);
            fwp.setY(ywf_sag);
            distance = ClosestApproach(bwp, fwp, posIn / CLHEP::cm, posOut / CLHEP::cm,
                                       hitPosition, wirePosition);
          }
          std::cout << "line distance= " << distance << std::endl;
          std::cout << "onTrack= " << hitPosition.x() << " " << hitPosition.y() << " " << hitPosition.z() << std::endl;
          std::cout << "posW   = " << wirePosition.x() << " " << wirePosition.y() << " " << wirePosition.z() << std::endl;
#endif
        }
      } else {  //no magnetic field case
        // Cal. distance assuming a line track
        G4ThreeVector bwp(bck.x(), bck.y(), bck.z());
        G4ThreeVector fwp(fwd.x(), fwd.y(), fwd.z());
        G4ThreeVector hitPosition, wirePosition;
        distance = ClosestApproach(bwp, fwp, posIn / CLHEP::cm, posOut / CLHEP::cm,
                                   hitPosition, wirePosition);
        if (m_wireSag) {
          G4double ywb_sag, ywf_sag;
          m_cdcgp->getWireSagEffect(CDCGeometryPar::c_Base, layerId, wires[i], wirePosition.z(), ywb_sag, ywf_sag);
          bwp.setY(ywb_sag);
          fwp.setY(ywf_sag);
          distance = ClosestApproach(bwp, fwp, posIn / CLHEP::cm, posOut / CLHEP::cm,
                                     hitPosition, wirePosition);
        }

        onTrack.setX(hitPosition.x());
        onTrack.setY(hitPosition.y());
        onTrack.setZ(hitPosition.z());
        posW.setX(wirePosition.x());
        posW.setY(wirePosition.y());
        posW.setZ(wirePosition.z());
        //tentative setting
        pOnTrack.setX(0.5 * (momIn.x() + momOut.x()) / CLHEP::GeV);
        pOnTrack.setY(0.5 * (momIn.y() + momOut.y()) / CLHEP::GeV);
        pOnTrack.setZ(0.5 * (momIn.z() + momOut.z()) / CLHEP::GeV);
      }  //end of magneticfiled on or off

#if defined(CDC_DEBUG)
      std::cout << "af distance= " << distance << std::endl;
      std::cout << "onTrack    = " << onTrack  << std::endl;
      std::cout << "posW       = " << posW     << std::endl;
      std::cout << "pOnTrack   = " << pOnTrack << std::endl;
      if (distance > 2.4) {
        std::cout << "toolargedriftl" << std::endl;
      }
#endif
      distance *= CLHEP::cm;  onTrack *= CLHEP::cm;  posW *= CLHEP::cm;
      pOnTrack *= CLHEP::GeV;

      G4ThreeVector posTrack(onTrack.x(), onTrack.y(), onTrack.z());
      G4ThreeVector mom(pOnTrack.x(), pOnTrack.y(), pOnTrack.z());

      const TVector3 tPosW(posW.x(), posW.y(), posW.z());
      const TVector3 tPosTrack(posTrack.x(), posTrack.y(), posTrack.z());
      const TVector3 tMom(mom.x(), mom.y(), mom.z());
      G4int lr = m_cdcgp->getOldLeftRight(tPosW, tPosTrack, tMom);
      G4int newLrRaw = m_cdcgp->getNewLeftRightRaw(tPosW, tPosTrack, tMom);
      //      if(abs(pid) == 11) {
      //  std::cout <<"pid,lr,newLrRaw 4electron= " << pid <<" "<< lr <<" "<< newLrRaw << std::endl;
      //      }
      G4int newLr = newLrRaw; //to be modified in EndOfEvent

      if (nWires == 1) {

        //        saveSimHit(layerId, wires[i], trackID, pid, distance, tofBefore, edep, s_in_layer * cm, momIn, posW, posIn, posOut, posTrack, lr, newLrRaw, newLr, speed);
        saveSimHit(layerId, wires[i], trackID, pid, distance, tofBefore, edep, s_in_layer * CLHEP::cm, pOnTrack, posW, posIn, posOut,
                   posTrack, lr, newLrRaw, newLr, speed, hitWeight);
#if defined(CDC_DEBUG)
        std::cout << "saveSimHit" << std::endl;
        std::cout << "momIn    = " << momIn    << std::endl;
        std::cout << "pOnTrack = " << pOnTrack << std::endl;
#endif

      } else {

        G4int cel1 = wires[i] + 1;
        G4int cel2 = cel1;
        if (i + 1 <= nWires - 1) {
          cel2 = wires[i + 1] + 1;
        }
        const G4double s2 = t.GetTrackLength() / CLHEP::cm;  //at post-step
        G4double s1 = (s2 - s_in_layer);  //at pre-step; varied later
        G4ThreeVector din = momIn;
        if (din.mag() != 0.) din /= momIn.mag();

        G4double  vent[6] = {posIn.x() / CLHEP::cm, posIn.y() / CLHEP::cm, posIn.z() / CLHEP::cm, din.x(), din.y(), din.z()};

        G4ThreeVector dot(momOut.x(), momOut.y(), momOut.z());
        if (dot.mag() != 0.) {
          dot /= dot.mag();
        } else {
          // Flight-direction is needed to set even when a particle stops
          dot = din;
        }

        G4double  vext[6] = {posOut.x() / CLHEP::cm, posOut.y() / CLHEP::cm, posOut.z() / CLHEP::cm, dot.x(), dot.y(), dot.z()};

        if (i > 0) {
          for (int j = 0; j < 6; ++j) vent[j] = xint[j];
          s1 = sint;
        }

        //        const G4int ic(3);  // cubic approximation of the track
        G4int    flag(0);
        G4double edep_in_cell(0.);
        G4double eLossInCell(0.);

        if (cel1 != cel2) {
#if defined(CDC_DEBUG)
          std::cout << "layerId,cel1,cel2= " << layerId << " " << cel1 << " " << cel2 << std::endl;
          std::cout << "vent= " << vent[0] << " " << vent[1] << " " << vent[2] << " " << vent[3] << " " << vent[4] << " " << vent[5] <<
                    std::endl;
          std::cout << "vext= " << vext[0] << " " << vext[1] << " " << vext[2] << " " << vext[3] << " " << vext[4] << " " << vext[5] <<
                    std::endl;
          std::cout << "s1,s2,ic= " << s1 << " " << s2 << " " << ic << std::endl;
#endif
          CellBound(layerId, cel1, cel2, vent, vext, s1, s2, xint, sint, flag);
#if defined(CDC_DEBUG)
          std::cout << "flag,sint= " << flag << " " << sint << std::endl;
          std::cout << "xint= " << xint[0] << " " << xint[1] << " " << xint[2] << " " << xint[3] << " " << xint[4] << " " << xint[5] <<
                    std::endl;
#endif

          const G4double test = (sint - s1) / s_in_layer;
          if (test < 0. || test > 1.) {
            B2WARNING("CDCSensitiveDetector: Strange path length: " << "s1= " << s1 << " sint= " << sint << " s_in_layer= " << s_in_layer <<
                      " test= " << test);
          }
          edep_in_cell = edep * std::abs((sint - s1)) / s_in_layer;

          const G4ThreeVector x_In(vent[0]*CLHEP::cm, vent[1]*CLHEP::cm, vent[2]*CLHEP::cm);
          const G4ThreeVector x_Out(xint[0]*CLHEP::cm, xint[1]*CLHEP::cm, xint[2]*CLHEP::cm);
          const G4ThreeVector p_In(momBefore * vent[3], momBefore * vent[4], momBefore * vent[5]);

          //          saveSimHit(layerId, wires[i], trackID, pid, distance, tofBefore, edep_in_cell, (sint - s1) * cm, p_In, posW, x_In, x_Out, posTrack, lr, newLrRaw, newLr, speed);
          saveSimHit(layerId, wires[i], trackID, pid, distance, tofBefore, edep_in_cell, std::abs((sint - s1)) * CLHEP::cm, pOnTrack, posW,
                     x_In, x_Out,
                     posTrack, lr, newLrRaw, newLr, speed, hitWeight);
#if defined(CDC_DEBUG)
          std::cout << "saveSimHit" << std::endl;
          std::cout << "p_In    = " << p_In     << std::endl;
          std::cout << "pOnTrack= " << pOnTrack << std::endl;
#endif
          tofBefore += (sint - s1) / speedInCmPerNs;
          eLossInCell = eLoss * (sint - s1) / s_in_layer;
          kinEnergyBefore -= eLossInCell;
          if (kinEnergyBefore >= 0.) {
            momBefore = sqrt(kinEnergyBefore * (kinEnergyBefore + 2.*mass));
          } else {
            B2WARNING("CDCSensitiveDetector: Kinetic Energy < 0.");
            momBefore = 0.;
          }

        } else {  //the particle exits

          const G4double test = (s2 - sint) / s_in_layer;
          if (test < 0. || test > 1.) {
            B2WARNING("CDCSensitiveDetector: Strange path length: " << "s2= " << s2 << " sint= " << sint << " s_in_layer= " << s_in_layer <<
                      " test= " << test);
          }
          edep_in_cell = edep * std::abs((s2 - sint)) / s_in_layer;

          const G4ThreeVector x_In(vent[0]*CLHEP::cm, vent[1]*CLHEP::cm, vent[2]*CLHEP::cm);
          const G4ThreeVector p_In(momBefore * vent[3], momBefore * vent[4], momBefore * vent[5]);

          //          saveSimHit(layerId, wires[i], trackID, pid, distance, tofBefore, edep_in_cell, (s2 - sint) * cm, p_In, posW, x_In, posOut, posTrack, lr, newLrRaw, newLr, speed);
          saveSimHit(layerId, wires[i], trackID, pid, distance, tofBefore, edep_in_cell, std::abs((s2 - sint)) * CLHEP::cm, pOnTrack, posW,
                     x_In,
                     posOut, posTrack, lr, newLrRaw, newLr, speed, hitWeight);
#if defined(CDC_DEBUG)
          std::cout << "saveSimHit" << std::endl;
          std::cout << "p_In    = " << p_In     << std::endl;
          std::cout << "pOnTrack= " << pOnTrack << std::endl;
#endif
        }
      }
      //setSeenInDetectorFlag(aStep, MCParticle::c_SeenInCDC);

      ////Add relation between the MCParticle and the hit.
      ////The index of the MCParticle has to be set to the TrackID and will be
      ////replaced later by the correct MCParticle index automatically.
      //StoreArray<Relation> mcPartToSimHits(getRelationCollectionName());
      //StoreArray<MCParticle> mcPartArray(DEFAULT_MCPARTICLES);
      //if (saveIndex < 0) {B2FATAL("SimHit wasn't saved despite charge != 0");}
      //StoreArray<CDCSimHit> cdcArray(DEFAULT_CDCSIMHITS);

      //new(mcPartToSimHits->AddrAt(saveIndex)) Relation(mcPartArray, cdcArray, trackID, saveIndex);

    } //end of wire loop

    return true;
  }

  /*
  void CDCSensitiveDetector::BeginOfEvent(G4HCofThisEvent*)
  {
    std::cout <<"CDCSensitiveDetector::BeginOfEvent callded." << std::endl;
  }
  */

  void CDCSensitiveDetector::EndOfEvent(G4HCofThisEvent*)
  {
    setModifiedLeftRightFlag();
  }

  void
  CDCSensitiveDetector::saveSimHit(const G4int layerId,
                                   const G4int wireId,
                                   const G4int trackID,
                                   const G4int pid,
                                   const G4double distance,
                                   const G4double tof,
                                   const G4double edep,
                                   const G4double stepLength,
                                   const G4ThreeVector& mom,
                                   const G4ThreeVector& posW,
                                   const G4ThreeVector& posIn,
                                   const G4ThreeVector& posOut,
                                   const G4ThreeVector& posTrack,
                                   const G4int lr,
                                   const G4int newLrRaw,
                                   const G4int newLr,
                                   const G4double speed,
                                   const G4double hitWeight)
  {

    // Discard the hit below Edep_th
    //    if (edep <= m_thresholdEnergyDeposit) return 0;
    if (edep <= m_thresholdEnergyDeposit) return;

    //compute tof at the closest point; linear approx.
    const G4double sign = (posTrack - posIn).dot(mom) < 0. ? -1. : 1.;
    const G4double CorrectTof = tof + sign * (posTrack - posIn).mag() / speed;
    //    if (sign < 0.) std::cout <<"deltatof= "<< sign * (posTrack - posIn).mag() / speed << std::endl;
#if defined(CDC_DEBUG)
    std::cout << "posIn= " << posIn.x() << "  " << posIn.y() << "  " << posIn.z() << std::endl;
    std::cout << "posOut= " << posOut.x() << "  " << posOut.y() << "  " << posOut.z() << std::endl;
    std::cout << "posTrack= " << posTrack.x() << "  " << posTrack.y() << "  " << posTrack.z() << std::endl;
    std::cout << "posW= " << posW.x() << "  " << posW.y() << "  " << posW.z() << std::endl;
    std::cout << "tof       = " << tof        << std::endl;
    std::cout << "deltaTof  = " << (posTrack - posIn).mag() / speed << std::endl;
    std::cout << "CorrectTof= " << CorrectTof << std::endl;
    if (CorrectTof > 95) {
      std::cout << "toolargecorrecttof" << std::endl;
    }
#endif

    StoreArray<MCParticle> mcParticles;

    StoreArray<CDCSimHit> cdcArray;

    RelationArray cdcSimHitRel(mcParticles, cdcArray);

    m_hitNumber = cdcArray.getEntries();

    // cppcheck-suppress memleak
    CDCSimHit* simHit =  cdcArray.appendNew();

    simHit->setWireID(layerId, wireId);
    simHit->setTrackId(trackID);
    simHit->setPDGCode(pid);
    simHit->setDriftLength(distance / CLHEP::cm);
    simHit->setFlightTime(CorrectTof / CLHEP::ns);
    simHit->setGlobalTime(CorrectTof / CLHEP::ns);
    simHit->setEnergyDep(edep / CLHEP::GeV);
    simHit->setStepLength(stepLength / CLHEP::cm);
    TVector3 momentum(mom.getX() / CLHEP::GeV, mom.getY() / CLHEP::GeV, mom.getZ() / CLHEP::GeV);
    simHit->setMomentum(momentum);
    TVector3 posWire(posW.getX() / CLHEP::cm, posW.getY() / CLHEP::cm, posW.getZ() / CLHEP::cm);
    simHit->setPosWire(posWire);
    TVector3 positionIn(posIn.getX() / CLHEP::cm, posIn.getY() / CLHEP::cm, posIn.getZ() / CLHEP::cm);
    simHit->setPosIn(positionIn);
    TVector3 positionOut(posOut.getX() / CLHEP::cm, posOut.getY() / CLHEP::cm, posOut.getZ() / CLHEP::cm);
    simHit->setPosOut(positionOut);
    TVector3 positionTrack(posTrack.getX() / CLHEP::cm, posTrack.getY() / CLHEP::cm, posTrack.getZ() / CLHEP::cm);
    simHit->setPosTrack(positionTrack);
    simHit->setPosFlag(lr);
    simHit->setLeftRightPassageRaw(newLrRaw);
    simHit->setLeftRightPassage(newLr);
#if defined(CDC_DEBUG)
    std::cout << "sensitived,oldlr,newlrRaw,newlr= " << lr << " " << newLrRaw << " " << newLr << std::endl;
#endif

    B2DEBUG(150, "HitNumber: " << m_hitNumber);
    if (m_modifiedLeftRightFlag) {
      //N.B. Negative hitWeight is allowed intentionally here; all weights are to be reset to positive in EndOfEvent
      cdcSimHitRel.add(trackID, m_hitNumber, hitWeight);
    } else {
      cdcSimHitRel.add(trackID, m_hitNumber);
    }

    //    if (hitWeight > 0) m_nPosHits++;
    //    if (hitWeight < 0) m_nNegHits++;
    //    std::cout <<"trackID,HitNumber,weight,driftL,edep= "<< trackID <<" "<< m_hitNumber <<" "<< hitWeight <<" "<< distance <<" "<< edep << std::endl;
    //    return (m_hitNumber);
    // cppcheck-suppress memleak
  }


  /*
    void SensitiveDetector::AddbgOne(bool doit) {
    Belle::Datcdc_olhit_Manager& olhitmgr=Belle::Datcdc_olhit_Manager::get_manager();
    if(doit) {
    for( int i=0;i<olhitmgr.count();i++ ){
    Belle::Datcdc_olhit&  h = olhitmgr[i];
    }
    dout(Debugout::B2INFO,"SensitiveDetector")
    << "AddbgOne " << olhitmgr.size()
    << std::endl;
    }
    olhitmgr.remove();
    }
  */

  void
  CDCSensitiveDetector::CellBound(const G4int layerId,
                                  const G4int ic1,
                                  const G4int ic2,
                                  const G4double venter[6],
                                  const G4double vexit[6],
                                  const G4double s1, const G4double s2,
                                  G4double xint[6],
                                  G4double& sint, G4int& iflag)
  {
    //---------------------------------------------------------------------------
    // (Purpose)
    //    calculate an intersection of track with cell boundary.
    //
    // (Relations)
    //    Calls       GCUBS
    //
    // (Arguments)
    //   input
    //     ic1        serial cell# (start w/ one) of entrance.
    //     ic2        serial cell# (start w/ one) of exit.
    //     venter(6)  (x,y,z,px/p,py/p,pz/p) at entrance.
    //     vexit(6)   (x,y,z,px/p,py/p,pz/p) at exit.
    //     s1         track length at entrance.
    //     s2         track length at exit.
    //   output
    //     xint(6)    (x,y,z,px/p,py/p,pz/p) at intersection of cell boundary.
    //     sint       track length at intersection of cell boundary.
    //     iflag      return code.
    //
    // N.B.(TODO ?) CDC misalignment wrt Belle2 coordinate system is ignored
    // when calculating the cell-boundary assuming misalign. is small.
    //--------------------------------------------------------------------------

    G4double div   = m_cdcgp->nWiresInLayer(layerId);

    //Check if s1, s2, ic1 and ic2 are ok
    if (s1 >= s2) {
      B2ERROR("CDCSensitiveDetector: s1(=" << s1 << ") > s2(=" << s2 << ")");
    }
    if (std::abs(ic1 - ic2) != 1) {
      if (ic1 == 1 && ic2 == div) {
      } else if (ic1 == div && ic2 == 1) {
      } else {
        B2ERROR("CDCSensitiveDetector: |ic1 - ic2| != 1 in CellBound; " << "ic1=" << ic1 << " " << "ic2=" << ic2);
      }
    }

    //get wire positions for the entrance cell
    G4double xwb = (m_cdcgp->wireBackwardPosition(layerId, ic1 - 1)).x();
    G4double ywb = (m_cdcgp->wireBackwardPosition(layerId, ic1 - 1)).y();
    G4double zwb = (m_cdcgp->wireBackwardPosition(layerId, ic1 - 1)).z();
    G4double xwf = (m_cdcgp->wireForwardPosition(layerId,  ic1 - 1)).x();
    G4double ywf = (m_cdcgp->wireForwardPosition(layerId,  ic1 - 1)).y();
    G4double zwf = (m_cdcgp->wireForwardPosition(layerId,  ic1 - 1)).z();

    /*
    G4double pathl = sqrt((vexit[0] - venter[0]) * (vexit[0] - venter[0])
        + (vexit[1] - venter[1]) * (vexit[1] - venter[1])
        + (vexit[2] - venter[2]) * (vexit[2] - venter[2]));
    std::cout << "app pathl= " << pathl << std::endl;
    G4double dot = venter[3] * vexit[3] + venter[4] * vexit[4];
    dot /= sqrt(venter[3] * venter[3] + venter[4] * venter[4]);
    dot /= sqrt( vexit[3] *  vexit[3] +  vexit[4] *  vexit[4]);
    if (dot < 0.) std::cout <<"negativedot= " << dot << std::endl;
    */

    //copy arrays
    G4double xx1[6], xx2[6];
    for (int i = 0; i < 6; ++i) {
      xx1[i] = venter[i];
      xx2[i] = vexit [i];
    }

    //calculate the field wire position betw. cell#1 and #2
    G4double psi = double(ic2 - ic1) * CLHEP::pi / div;
    if (ic1 == 1 && ic2 == div) {
      psi = -CLHEP::pi / div;
    } else if (ic1 == div && ic2 == 1) {
      psi =  CLHEP::pi / div;
    }
    G4double cospsi = cos(psi);
    G4double sinpsi = sin(psi);

    G4double xfwb = cospsi * xwb - sinpsi * ywb;
    G4double yfwb = sinpsi * xwb + cospsi * ywb;
    G4double xfwf = cospsi * xwf - sinpsi * ywf;
    G4double yfwf = sinpsi * xwf + cospsi * ywf;
    G4double zfwb = zwb;
    G4double zfwf = zwf;

    //prepare quantities related to the cell-boundary
    G4double vx = xfwf - xfwb;
    G4double vy = yfwf - yfwb;
    G4double vz = zfwf - zfwb;
    G4double vv = sqrt(vx * vx + vy * vy + vz * vz);
    vx /= vv;  vy /= vv;  vz /= vv;

    //translate to make the cubic description easier
    G4double shiftx = (xx1[0] + xx2[0]) * 0.5;
    G4double shifty = (xx1[1] + xx2[1]) * 0.5;
    G4double shiftz = (xx1[2] + xx2[2]) * 0.5;
    G4double shifts = (s1     +     s2) * 0.5;
    G4double xshft = xx1[0] - shiftx;
    G4double yshft = xx1[1] - shifty;
    G4double zshft = xx1[2] - shiftz;
    G4double sshft = s1     - shifts;

    //approximate the trajectroy by cubic curves
    G4double pabs1 = sqrt(xx1[3] * xx1[3] + xx1[4] * xx1[4] + xx1[5] * xx1[5]);
    G4double pabs2 = sqrt(xx2[3] * xx2[3] + xx2[4] * xx2[4] + xx2[5] * xx2[5]);
    //      std::cout << "pabs1,2= " << pabs1 <<" "<< pabs2 << std::endl;

    G4double a[4] = {0.}, b[4] = {0.}, c[4] = {0.};

    if (m_magneticField) {
      GCUBS(sshft, xshft, xx1[3] / pabs1, xx2[3] / pabs2, a);
      GCUBS(sshft, yshft, xx1[4] / pabs1, xx2[4] / pabs2, b);
      GCUBS(sshft, zshft, xx1[5] / pabs1, xx2[5] / pabs2, c);
      //      std::cout <<"a= " << a[0] <<" "<< a[1] <<" "<< a[2] <<" "<< a[3] << std::endl;
      //      std::cout <<"b= " << b[0] <<" "<< b[1] <<" "<< b[2] <<" "<< b[3] << std::endl;
      //      std::cout <<"c= " << c[0] <<" "<< c[1] <<" "<< c[2] <<" "<< c[3] << std::endl;
    } else {
      //n.b. following is really better ?
      a[1] = xshft / sshft;
      b[1] = yshft / sshft;
      c[1] = zshft / sshft;
    }

    //calculate an int. point betw. the trajectory and the cell-boundary
    G4double stry(0.), xtry(0.), ytry(0.), ztry(0.);
    G4double beta(0.), xfw(0.), yfw(0.);
    G4double sphi(0.), cphi(0.), dphil(0.), dphih(0.);
    const G4int maxTrials = 100;
    const G4double eps = 5.e-4;
    G4double sl =  sshft;  // negative value
    G4double sh = -sshft;  // positive value
    G4int i = 0;

    //set initial value (dphil) for the 1st iteration
    stry = sl;
    xtry = shiftx + a[0] + stry * (a[1] + stry * (a[2] + stry * a[3]));
    ytry = shifty + b[0] + stry * (b[1] + stry * (b[2] + stry * b[3]));
    ztry = shiftz + c[0] + stry * (c[1] + stry * (c[2] + stry * c[3]));
    beta = (ztry - zfwb) / vz;
    xfw  = xfwb + beta * vx;
    yfw  = yfwb + beta * vy;
    sphi = (xtry * yfw - ytry * xfw);
    cphi = (xtry * xfw + ytry * yfw);
    dphil = atan2(sphi, cphi);  //n.b. no need to conv. to dphi...

    iflag = 1;

    while (((sh - sl) > eps) && (i < maxTrials)) {
      stry = 0.5 * (sl + sh);
      xtry = shiftx + a[0] + stry * (a[1] + stry * (a[2] + stry * a[3]));
      ytry = shifty + b[0] + stry * (b[1] + stry * (b[2] + stry * b[3]));
      ztry = shiftz + c[0] + stry * (c[1] + stry * (c[2] + stry * c[3]));
      beta = (ztry - zfwb) / vz;
      xfw  = xfwb + beta * vx;
      yfw  = yfwb + beta * vy;

      sphi  = (xtry * yfw - ytry * xfw);
      cphi  = (xtry * xfw + ytry * yfw);
      dphih = atan2(sphi, cphi);  //n.b. no need to conv. to dphi...

      if (dphil * dphih > 0.) {
        sl = stry;
      } else {
        sh = stry;
      }
      ++i;
    }

    //      std::cout << "itry= " << i << std::endl;
    if (i >= maxTrials - 1) {
      iflag = 0;
      B2WARNING("CDCSensitiveDetector: No intersection ?");

      /*  G4double ds = 1.e-4;
      G4int imax = (s2 - s1) / ds + 1;
      G4double rdphimin = DBL_MAX;

      for (i=0; i <= imax; ++i) {
          stry = sshft + i * ds;
          xtry = shiftx + a[0] + stry * (a[1] + stry * (a[2] + stry * a[3]));
          ytry = shifty + b[0] + stry * (b[1] + stry * (b[2] + stry * b[3]));
          ztry = shiftz + c[0] + stry * (c[1] + stry * (c[2] + stry * c[3]));
          beta = (ztry - zfwb) / vz;
          xfw  = xfwb + beta * vx;
          yfw  = yfwb + beta * vy;

          sphi = (xtry * yfw - ytry * xfw);
          cphi = (xtry * xfw + ytry * yfw);
          dphi  = atan2(sphi, cphi);
          rdphi = sqrt(xfw * xfw + yfw * yfw) * dphi;

          if ( std::abs(rdphi) < std::abs(rdphimin)) {
      rdphimin = rdphi;
      imin = i;
          }
      }
      */
    }
    //      sint = sshft + imin * ds;
    sint = stry;

    //      std::cout <<"i,dphil,dphih,sint= " << i <<" "<< dphil <<" "<< dphih <<" "<< sint << std::endl;
    //get the trajectory at the int. point
    xint[0] = a[0] + sint * (a[1] + sint * (a[2] + sint * a[3]));
    xint[1] = b[0] + sint * (b[1] + sint * (b[2] + sint * b[3]));
    xint[2] = c[0] + sint * (c[1] + sint * (c[2] + sint * c[3]));
    xint[3] = a[1] + sint * (2. * a[2] + 3. * sint * a[3]);
    xint[4] = b[1] + sint * (2. * b[2] + 3. * sint * b[3]);
    xint[5] = c[1] + sint * (2. * c[2] + 3. * sint * c[3]);

    //translate back to the lab. frame
    xint[0] += shiftx;
    xint[1] += shifty;
    xint[2] += shiftz;
    sint    += shifts;
    /*
      std::cout <<"s1,s2,sint= " << s1 <<" "<< s2 <<" "<< sint << std::endl;
      std::cout <<" xx1= " << xx1[0] <<" "<<  xx1[1] <<" "<<  xx1[2] << std::endl;
      std::cout <<" xx2= " << xx2[0] <<" "<<  xx2[1] <<" "<<  xx2[2] << std::endl;
      std::cout <<"xint= " << xint[0] <<" "<< xint[1] <<" "<< xint[2] << std::endl;
    */

    /*      if (((xx1[0] <= xint[0] && xint[0] <= xx2[0]) ||
      (xx2[0] <= xint[0] && xint[0] <= xx1[0])) &&
      ((xx1[1] <= xint[1] && xint[1] <= xx2[1]) ||
      (xx2[1] <= xint[1] && xint[1] <= xx1[1])) &&
      ((xx1[2] <= xint[2] && xint[2] <= xx2[2]) ||
      (xx2[2] <= xint[2] && xint[2] <= xx1[2])) &&
      (s1     <= sint    && sint    <= s2)) {
      } else {
      std::cout << "strangeinttersection" << std::endl;
      }
    */
    //re-normalize to one since abs=1 is not guearanteed in the cubic approx.
    G4double p = sqrt(xint[3] * xint[3] + xint[4] * xint[4] + xint[5] * xint[5]);
    xint[3] /= p;  xint[4] /= p;  xint[5] /= p;
    //      std::cout << "norm= " << p << std::endl;
    //    std::cout <<"s1,s2,sint= " << s1 <<" "<< s2 <<" "<< sint << std::endl;
    //    std::cout <<"xint= " << xint[0] <<" "<< xint[1] <<" "<< xint[2] << std::endl;
    //    std::cout <<"xint= " << xint[3] <<" "<< xint[4] <<" "<< xint[5] << std::endl;
  }

  void CDCSensitiveDetector::GCUBS(const G4double x, const G4double y, const G4double d1, const G4double d2, G4double a[4])
  {
    //Original: GCUBS in Geant3
    //    ******************************************************************
    //    *                                                                *
    //    *       Calculates a cubic through P1,(X,Y),(-X,-Y),P2           *
    //    *        Y=A(1)+A(2)*X+A(3)*X**2+A(4)*X**3                       *
    //    *        The coordinate system is assumed to be the cms system   *
    //    *        of P1,P2.                                               *
    //    *        d1(2): directional cosine at P1(2).                     *
    //    *                                                                *
    //    *    ==>Called by : GIPLAN,GICYL                                 *
    //    *       Author    H.Boerner  *********                           *
    //    *                                                                *
    //    ******************************************************************

    G4double fact(0);

    if (x == 0.) goto L10;

    fact = (d1 - d2) * 0.25;
    a[0] = - 1. * fact * x;
    a[2] = fact / x;
    a[1] = (6. * y - (d1 + d2) * x) / (4. * x);
    a[3]   = ((d1 + d2) * x - 2.*y) / (4.*x * x * x);
    return;

L10:
    a[0] = 0.;
    a[1] = 1.;
    a[2] = 0.;
    a[3] = 0.;
  }

  void
  CDCSensitiveDetector::for_Rotat(const G4double bfld[3])
  {
    //Calculates a rotation matrix in advance at a local position in lab.
    //The rotation is done about the coord. origin; lab.-frame to B-field
    //frame in which only Bz-comp. is non-zero.
    //~dead copy of gsim_cdc_for_rotat.F in gsim-cdc for Belle (for tentaive use)

    if (m_nonUniformField == 0) return;

    G4double bx, by, bz;
    bx = bfld[0];
    by = bfld[1];
    bz = bfld[2];

    //cal. rotation matrix
    G4double bxz, bfield;
    bxz    = bx * bx + bz * bz;
    bfield = bxz   + by * by;
    bxz    = sqrt(bxz);
    bfield = sqrt(bfield);

    m_brot[0][0] = bz / bxz;
    m_brot[1][0] = 0.;
    m_brot[2][0] = -bx / bxz;
    m_brot[0][1] = -by * bx / bxz / bfield;
    m_brot[1][1] = bxz     / bfield;
    m_brot[2][1] = -by * bz / bxz / bfield;
    m_brot[0][2] = bx / bfield;
    m_brot[1][2] = by / bfield;
    m_brot[2][2] = bz / bfield;

    return;

  }

  void
  CDCSensitiveDetector::Rotat(G4double& x, G4double& y, G4double& z,
                              const int mode)
  {
    //Translates (x,y,z) in lab. to (x,y,z) in B-field frame (mode=1), or reverse
    // translation (mode=-1).
    //~dead copy (for tentaive use) of gsim_cdc_rotat/irotat.F in gsim-cdc
    //for Belle

    if (m_nonUniformField == 0) return;

    G4double x0(x), y0(y), z0(z);

    if (mode  == 1) {
      x = m_brot[0][0] * x0 + m_brot[1][0] * y0 + m_brot[2][0] * z0;
      y = m_brot[0][1] * x0 + m_brot[1][1] * y0 + m_brot[2][1] * z0;
      z = m_brot[0][2] * x0 + m_brot[1][2] * y0 + m_brot[2][2] * z0;
    } else if (mode == -1) {
      x = m_brot[0][0] * x0 + m_brot[0][1] * y0 + m_brot[0][2] * z0;
      y = m_brot[1][0] * x0 + m_brot[1][1] * y0 + m_brot[1][2] * z0;
      z = m_brot[2][0] * x0 + m_brot[2][1] * y0 + m_brot[2][2] * z0;
    } else {
      //B2ERROR("SensitiveDetector " <<"invalid mode " << mode << "specifed");
    }
    return;

  }

  void
  CDCSensitiveDetector::Rotat(G4double x[3], const int mode)
  {
    //Translates (x,y,z) in lab. to (x,y,z) in B-field frame (mode=1), or reverse
    // translation (mode=-1).
    //~dead copy (for tentaive use) of gsim_cdc_rotat/irotat.F in gsim-cdc
    //for Belle

    if (m_nonUniformField == 0) return;

    G4double x0(x[0]), y0(x[1]), z0(x[2]);

    if (mode  == 1) {
      x[0] = m_brot[0][0] * x0 + m_brot[1][0] * y0 + m_brot[2][0] * z0;
      x[1] = m_brot[0][1] * x0 + m_brot[1][1] * y0 + m_brot[2][1] * z0;
      x[2] = m_brot[0][2] * x0 + m_brot[1][2] * y0 + m_brot[2][2] * z0;
    } else if (mode == -1) {
      x[0] = m_brot[0][0] * x0 + m_brot[0][1] * y0 + m_brot[0][2] * z0;
      x[1] = m_brot[1][0] * x0 + m_brot[1][1] * y0 + m_brot[1][2] * z0;
      x[2] = m_brot[2][0] * x0 + m_brot[2][1] * y0 + m_brot[2][2] * z0;
    } else {
      //B2ERROR("SensitiveDetector " <<"invalid mode " << mode << "specifed");
    }
    return;

  }

  void
  CDCSensitiveDetector::HELWIR(const G4double xwb4, const G4double ywb4,
                               const G4double zwb4,
                               const G4double xwf4, const G4double ywf4,
                               const G4double zwf4,
                               const G4double xp, const G4double yp,
                               const G4double zp,
                               const G4double px, const G4double py,
                               const G4double pz,
                               const G4double B_kG[3],
                               const G4double charge, const G4int ntryMax,
                               G4double& distance,
                               G4double q2[3], G4double q1[3],
                               G4double q3[3],
                               G4int& ntry)
  {
    //~dead copy of gsim_cdc_hit.F in gsim-cdc for Belle (for tentaive use)
    // ---------------------------------------------------------------------
    //     Purpose : Calculate closest points between helix and wire.
    //
    //     Input
    //         xwb4 : x of wire at backward endplate in lab.
    //         ywb4 : y of wire at backward endplate   "
    //         zwb4 : z of wire at backward endplate   "
    //         xwf4 : x of wire at forward  endplate   "
    //         ywf4 : y of wire at forward  endplate   "
    //         zwf4 : z of wire at forward  endplate   "
    //
    //     Output
    //         q2(1) : x of wire  at closest point in lab.
    //         q2(2) : y of wire  at closest point   "
    //         q2(3) : z of wire  at closest point   "
    //         q1(1) : x of helix at closest point   "
    //         q1(2) : y of helix at closest point   "
    //         q1(3) : z of helix at closest point   "
    //         ntry  :
    // ---------------------------------------------------------------------

    const G4int ndim = 3;
    const G4double delta = 1.e-5;


    G4double xwb, ywb, zwb, xwf, ywf, zwf;
    G4double xw, yw, zw, xh, yh, zh, pxh, pyh, pzh;
    G4double fi, fi_corr;

    G4double dr, fi0, cpa, dz, tanl;
    G4double x0, y0, z0;
    // "chrg" removed by M. U. June, 2nd, 2013
    //    G4double xc, yc, r, chrg;
    G4double xc, yc, r;
    G4double xwm, ywm;
    G4double sinfi0, cosfi0, sinfi0fi, cosfi0fi;

    G4double vx, vy, vz, vv, cx, cy, cz, tt[3][3];
    G4double tmp[3];

    G4double xx[3], dxx[3], ddxx[3], pp[3];
    G4double xxtdxx, dxxtdxx, xxtddxx;


    G4double fst = 0.0;
    G4double f, fderiv, deltafi, fact, eval;
    G4double dx1, dy1, dx2, dy2, crs, dot;

    G4int iflg;

    //set parameters
    xwb = xwb4;  ywb = ywb4;  zwb = zwb4;
    xwf = xwf4;  ywf = ywf4;  zwf = zwf4;

    G4double xxx(xp), yyy(yp), zzz(zp);
    G4double pxx(px), pyy(py), pzz(pz);

    //rotate z-axis to be parallel to B-field in case of non-uniform B
    Rotat(xwb, ywb, zwb, 1);
    Rotat(xwf, ywf, zwf, 1);
    Rotat(xxx, yyy, zzz, 1);
    Rotat(pxx, pyy, pzz, 1);

    G4double a[8] = {0.};
    G4double pt = sqrt(pxx * pxx + pyy * pyy);
    a[1] = atan2(-pxx, pyy);
    a[2] = charge / pt;
    a[4] = pzz   / pt;
    a[5] = xxx;  a[6] = yyy;  a[7] = zzz;

    //calculate unit direction vector of the sense wire
    vx = xwf - xwb;  vy = ywf - ywb;  vz = zwf - zwb;
    vv = sqrt(vx * vx + vy * vy + vz * vz);
    vx /= vv;  vy /= vv;  vz /= vv;

    //flag for distingushing between stereo and axial wire
    iflg = 0;
    if (vx == 0. &&  vy == 0.) iflg = 1;
    //  std::cout << "iflg= " << iflg << std::endl;
    //write(6,*) ' hlx2wir ', xwb, ywb, zwb, vx, vy, vz

    //calculate coefficients of f
    cx = xwb - vx * (vx * xwb + vy * ywb + vz * zwb);
    cy = ywb - vy * (vx * xwb + vy * ywb + vz * zwb);
    cz = zwb - vz * (vx * xwb + vy * ywb + vz * zwb);

    //calculate tensor for f
    tt[0][0] = vx * vx - 1.;  tt[1][0] = vx * vy;       tt[2][0] = vx * vz;
    tt[0][1] = vy * vx;       tt[1][1] = vy * vy - 1.;  tt[2][1] = vy * vz;
    tt[0][2] = vz * vx;       tt[1][2] = vz * vy;       tt[2][2] = vz * vz - 1.;

    //set helix parameters
    dr   = a[0];   fi0  = a[1];  cpa  = a[2];
    dz   = a[3];   tanl = a[4];
    x0   = a[5];   y0   = a[6];  z0   = a[7];

    //
    // set initial value for phi
    //

    xwm    = xxx;
    ywm    = yyy;
    //r(cm) = alpha/cpa = alpha * pt(GeV); bfield(kG)
    G4double bfield = sqrt(B_kG[0] * B_kG[0] +
                           B_kG[1] * B_kG[1] +
                           B_kG[2] * B_kG[2]);
    G4double alpha  = 1.e4 / 2.99792458 / bfield;
    r      = alpha / cpa;
    cosfi0 = cos(fi0);
    sinfi0 = sin(fi0);
    xc  = x0 + (dr + r) * cosfi0;
    yc  = y0 + (dr + r) * sinfi0;
    dx1 = x0 - xc;
    dy1 = y0 - yc;
    dx2 = xwm - xc;
    dy2 = ywm - yc;
    crs = dx1 * dy2 - dy1 * dx2;
    dot = dx1 * dx2 + dy1 * dy2;
    fi = atan2(crs, dot);

    //begin iterative procedure for newton 's method   '
    fact = 1.;
    ntry = 0;
line1:
    ntry += 1;
    cosfi0fi = cos(fi0 + fi);
    sinfi0fi = sin(fi0 + fi);

    //calculate spatial point Q(x,y,z) along the helix
    xx[0] = x0 + dr * cosfi0 + r * (cosfi0 - cosfi0fi);
    xx[1] = y0 + dr * sinfi0 + r * (sinfi0 - sinfi0fi);
    xx[2] = z0 + dz        - r * tanl * fi;
    pp[0] = -pt * sinfi0fi;
    pp[1] = pt * cosfi0fi;
    pp[2] = pt * tanl;

    if (iflg  == 1) {
      q2[0] = xwb;    q2[1] = ywb;    q2[2] = xx[2];
      q1[0] = xx[0];  q1[1] = xx[1];  q1[2] = xx[2];
      q3[0] = pp[0];  q3[1] = pp[1];  q3[2] = pp[2];
      //inverse rotation to lab. frame in case of non-uniform B
      Rotat(q1, -1);
      Rotat(q2, -1);
      Rotat(q3, -1);
      distance = sqrt((q2[0] - q1[0]) * (q2[0] - q1[0]) +
                      (q2[1] - q1[1]) * (q2[1] - q1[1]) +
                      (q2[2] - q1[2]) * (q2[2] - q1[2]));
      return;
    }

    //calculate direction vector (dx/dphi,dy/dphi,dz/dphi)
    //on a point along the helix.
    dxx[0] =   r * sinfi0fi;  dxx[1] = - r * cosfi0fi;  dxx[2] = - r * tanl;

    //   In order to derive the closest pont between straight line and helix,
    //   we can put following two conditions:
    //     (i)  A point H(xh,yh,zh) on the helix given should be on
    //          the plane which is perpendicular to the straight line.
    //     (ii) A line HW from W(xw,yw,zw) which is a point on the straight
    //          line to H(xh,yh,zh) should normal to the direction vector
    //          on the point H.
    //
    //   Thus, we can make a equation from above conditions.
    //     f(phi) = cx*(dx/dphi) + cy*(dy/dphi) + cz*(dz/dphi)
    //              + (x,y,z)*tt(i,j)*(dx/dphi,dy/dphi,dz/dphi)
    //            = 0,
    //     where
    //      cx      = xwb - vx*( vx*xwb + vy*ywb + vz*zwb )
    //      cy      = ywb - vy*( vx*xwb + vy*ywb + vz*zwb )
    //      cz      = zwb - vz*( vx*xwb + vy*ywb + vz*zwb )
    //
    //      tt(1,1) = vx*vx - 1  tt(1,2) = vx*vy      tt(1,3) = vx*vz
    //      tt(2,1) = vy*vx      tt(2,2) = vy*vy - 1  tt(2,3) = vy*vz
    //      tt(3,1) = vz*vx      tt(3,2) = vz*vy      tt(3,3) = vz*vz - 1
    //
    //     and the equation of straight line(stereo wire) is written by
    //     (x,y,z) = (xwb,ywb,zwb) + beta*(vx,vy,vz), beta is free parameter.

    //Now calculate f
    Mvopr(ndim, xx, tt, dxx, tmp, 1);
    xxtdxx = tmp[0];
    f = cx * dxx[0] + cy * dxx[1] + cz * dxx[2] + xxtdxx;
    if (std::abs(f) < delta) goto line100;

    //evaluate fitting result and prepare some factor to multiply to 1/derivative
    if (ntry > 1) {
      eval = (1.0 - 0.25 * fact) * std::abs(fst) - std::abs(f);
      if (eval <= 0.) fact *= 0.5;
    }

    //calculate derivative of f
    ddxx[0] = r * cosfi0fi;  ddxx[1] = r * sinfi0fi;  ddxx[2] = 0.;

    //Now we have derivative of f
    Mvopr(ndim, dxx, tt,  dxx, tmp, 1);
    dxxtdxx = tmp[0];
    Mvopr(ndim,  xx, tt, ddxx, tmp, 1);
    xxtddxx = tmp[0];
    fderiv = cx * ddxx[0] + cy * ddxx[1] + cz * ddxx[2] + dxxtdxx + xxtddxx;
    // Commented by M. U. June, 2nd, 2013
    //    fist    = fi;
    deltafi = f / fderiv;
    fi     -= fact * deltafi;
    fst     = f;

    if (ntry > ntryMax) {
      //B2DEBUG(" Exceed max. trials HelWir ");
      goto line100;
    }
    //write(6,*) ntry, fist, deltafi
    goto line1;

    //check if zh is btw zwb and zwf; if not, set zh=zwb or zh=zwf.
    //dead regions due to feed-throughs should be considered later.
line100:
    zh  = z0 + dz - r * tanl * fi;
    fi_corr = 0.;
    if (zh  < zwb) fi_corr = (zwb - zh) / (-r * tanl);
    if (zh  > zwf) fi_corr = (zwf - zh) / (-r * tanl);
    fi += fi_corr;

    cosfi0fi = cos(fi0 + fi);
    sinfi0fi = sin(fi0 + fi);

    xh  = x0 + dr * cosfi0 + r * (cosfi0 - cosfi0fi);
    yh  = y0 + dr * sinfi0 + r * (sinfi0 - sinfi0fi);
    zh  = z0 + dz        - r * tanl * fi;
    pxh = -pt * sinfi0fi;
    pyh = pt * cosfi0fi;
    pzh = pt * tanl;

    //write(6,*) 'fi_corr, zh, zwb, zwf=', fi_corr, zh, zwb, zwf
    //write(6,*) 'zh = ', z0, dz, r, tanl, fi

    zw = vx * vz * xh + vy * vz * yh + vz * vz * zh + zwb - vz * (vx * xwb + vy * ywb + vz * zwb);
    xw = xwb + vx * (zw - zwb) / vz;
    yw = ywb + vy * (zw - zwb) / vz;

    q2[0] = xw;  q2[1] = yw;  q2[2] = zw;
    q1[0] = xh;  q1[1] = yh;  q1[2] = zh;
    q3[0] = pxh; q3[1] = pyh; q3[2] = pzh;

    //inverse rotation to lab. frame in case of non-uniform B
    Rotat(q1, -1);
    Rotat(q2, -1);
    Rotat(q3, -1);
    distance = sqrt((q2[0] - q1[0]) * (q2[0] - q1[0]) +
                    (q2[1] - q1[1]) * (q2[1] - q1[1]) +
                    (q2[2] - q1[2]) * (q2[2] - q1[2]));
    return;

  }

  void
  CDCSensitiveDetector::Mvopr(const G4int ndim, const G4double b[3], const G4double m[3][3],
                              const G4double a[3], G4double c[3], const G4int mode)
  {
    //~dead copy of UtilCDC_mvopr in com-cdc for Belle (for tentative use)
    //-----------------------------------------------------------------------
    //     Input
    //         ndim             : dimension
    //         b(1-ndim)        : vector
    //         m(1-ndim,1-ndim) : matrix
    //         a(1-ndim)        : vector
    //         c(1-ndim)        : vector
    //         mode             : c = m * a     for mode=0
    //                            c = b * m * a for mode=1
    //     Output
    //         c(1-ndim)        : for mode 1, solution is put on c[0]
    //-----------------------------------------------------------------------

    if (ndim != 3) {
      //B2ERROR("invalid ndim " << ndim << " specified");
      return;
    }

    for (int i = 0; i < ndim; ++i)   c[i] = 0.;
    G4double tmp[3];
    for (int i = 0; i < ndim; ++i) tmp[i] = 0.;

    if (mode == 0) {
      for (int i = 0; i < ndim; ++i) {
        for (int j = 0; j < ndim; ++j) {
          c[i] += m[j][i] * a[j];
        }
      }
      return;
    } else if (mode == 1) {
      for (int i = 0; i < ndim; ++i) {
        for (int j = 0; j < ndim; ++j) {
          tmp[i] += m[j][i] * a[j];
        }
        c[0] += b[i] * tmp[i];
      }
    } else {
      //B2ERROR("Error, you specified invalid mode= " << mode);
    }

    return;

  }

  std::vector<int>
  CDCSensitiveDetector::WireId_in_hit_order(int id0, int id1, int nWires)
  {
    std::vector<int> list;
    int i0 = int(id0);
    int i1 = int(id1);
    if (abs(i0 - i1) * 2 < int(nWires)) {
      if (id0 < id1) {
        for (int i = id0; i <= id1; ++i)
          list.push_back(i);
      } else {
        for (int i = id0; i >= id1; i--) {
          list.push_back(i);
        }
      }
    } else {
      if (id0 < id1) {
        for (int i = id0; i >= 0; i--)
          list.push_back(i);
        for (int i = nWires - 1; i >= id1; i--)
          list.push_back(i);
      } else {
        for (int i = id0; i < nWires; ++i)
          list.push_back(i);
        for (int i = 0; i <= id1; ++i)
          list.push_back(i);
      }
    }

    return list;
  }

  G4double CDCSensitiveDetector::ClosestApproach(const G4ThreeVector bwp, const G4ThreeVector fwp, const G4ThreeVector posIn,
                                                 const G4ThreeVector posOut, G4ThreeVector& hitPosition, G4ThreeVector& wirePosition)//,G4double& transferT)
  {

    TVector3 tbwp(bwp.x(), bwp.y(), bwp.z());
    TVector3 tfwp(fwp.x(), fwp.y(), fwp.z());
    TVector3 tposIn(posIn.x(),  posIn.y(),  posIn.z());
    TVector3 tposOut(posOut.x(), posOut.y(), posOut.z());
    TVector3 thitPosition(0., 0., 0.);
    TVector3 twirePosition(0., 0., 0.);

    //    G4double distance = m_cdcgp.ClosestApproach(tbwp, tfwp, tposIn, tposOut, thitPosition, twirePosition);
    G4double distance = CDC::ClosestApproach(tbwp, tfwp, tposIn, tposOut, thitPosition, twirePosition);

    hitPosition.setX(thitPosition.x());
    hitPosition.setY(thitPosition.y());
    hitPosition.setZ(thitPosition.z());

    wirePosition.setX(twirePosition.x());
    wirePosition.setY(twirePosition.y());
    wirePosition.setZ(twirePosition.z());

    return distance;
  }


  //The following-to-end is for setting of left/right flag modified for tracking
  void CDCSensitiveDetector::setModifiedLeftRightFlag()
  {
    if (!m_modifiedLeftRightFlag) return;

    //    std::cout <<"#posHits,#negHits= " << m_nPosHits <<" "<< m_nNegHits << std::endl;

    // Get SimHit array and relation betw. MC and SimHit
    // N.B. MCParticle is incomplete at this stage; the relation betw it and
    // simHit is Okay.
    // MCParticle will be completed after all sub-detectors' EndOfEvent calls.
    StoreArray<CDCSimHit>  simHits;
    StoreArray<MCParticle> mcParticles;
    RelationArray mcPartToSimHits(mcParticles, simHits);
    int nRelationsMinusOne = mcPartToSimHits.getEntries() - 1;

    if (nRelationsMinusOne == -1) return;

    //    std::cout <<"#simHits= " << simHits.getEntries() << std::endl;
    //    std::cout <<"#mcParticles= " << mcParticles.getEntries() << std::endl;
    //    std::cout <<"#mcPartToSimHits= " << mcPartToSimHits.getEntries() << std::endl;

    //reset some of negative weights to positive; this is needed for the hits
    //created by secondary particles whose track-lengths get larger than the
    //threshold (set by the user) during G4 swimming (i.e. the weights are
    //first set to negative as far as the track-lengths are shorther than the
    //threshold; set to positive when the track-lengths exceed the threshold).

    size_t iRelation = 0;
    int trackIdOld = INT_MAX;
    //    std::cout << "INT_MAX= " << INT_MAX << std::endl;
    m_hitWithPosWeight.clear();
    m_hitWithNegWeight.clear();

    for (int it = nRelationsMinusOne; it >= 0; --it) {
      RelationElement& mcPartToSimHit = const_cast<RelationElement&>(mcPartToSimHits[it]);
      size_t nRelatedHits = mcPartToSimHit.getSize();
      if (nRelatedHits > 1) B2FATAL("CDCSensitiveDetector::EndOfEvent: MCParticle<-> CDCSimHit relation is not one-to-one !");

      unsigned short trackId = mcPartToSimHit.getFromIndex();
      RelationElement::weight_type weight = mcPartToSimHit.getWeight(iRelation);
      if (weight > 0.) {
        trackIdOld = trackId;
      } else if (weight <= 0. && trackId == trackIdOld) {
        //  RelationElement::index_type iSimHit = mcPartToSimHit.getToIndex(iRelation);
        weight *= -1.;
        mcPartToSimHit.setToIndex(mcPartToSimHit.getToIndex(iRelation), weight);
        trackIdOld = trackId;
        //  std::cout <<"trackId,,iSimHit,wgtafterreset= "<<  trackId <<" "<< iSimHit <<" "<< mcPartToSimHit.getWeight(iRelation) << std::endl;
      }

      CDCSimHit* sHit = simHits[mcPartToSimHit.getToIndex(iRelation)];

      if (weight > 0.) {
        m_hitWithPosWeight.insert(std::pair<unsigned short, CDCSimHit*>(sHit->getWireID().getISuperLayer(), sHit));
      } else {
        m_hitWithNegWeight.push_back(sHit);
      }
    }

    /*
    //    std::cout <<"m_hitWithPosWeight.size= " << m_hitWithPosWeight.size() << std::endl;
    for(int i=0; i<9; ++i) {
      //      if (m_hitWithPosWeight.find(i) != m_hitWithPosWeight.end()) {
      //  std::cout << i << " found" << std::endl;
      //      }
      m_posWeightMapItBegin.push_back(m_hitWithPosWeight.find(i));
      m_posWeightMapItEnd.push_back(m_hitWithPosWeight.find(i+1));
    }
    */

    //reassign L/R flag
    reAssignLeftRightInfo();

    //reset all weights positive; this is required for completing MCParticle object at the EndOfEvent action of FullSim
    // is this part really needed ??? check again !
    for (int it = 0; it <= nRelationsMinusOne; ++it) {
      RelationElement& mcPartToSimHit = const_cast<RelationElement&>(mcPartToSimHits[it]);
      RelationElement::weight_type weight = mcPartToSimHit.getWeight(iRelation);
      if (weight < 0.) {
        mcPartToSimHit.setToIndex(mcPartToSimHit.getToIndex(iRelation), -1.*weight);
      }
    }

  }


  void CDCSensitiveDetector::reAssignLeftRightInfo()
  {
    CDCSimHit* sHit = nullptr;
    WireID sWireId             = WireID();
    TVector3 sPos              = TVector3();

    CDCSimHit* pHit = nullptr;
    WireID pWireId = WireID();
    double minDistance2 = DBL_MAX;
    double    distance2 = DBL_MAX;
    //    unsigned short bestNeighb = 0;
    unsigned short neighb = 0;

    std::multimap<unsigned short, CDCSimHit*>::iterator pItBegin = m_hitWithPosWeight.begin();
    std::multimap<unsigned short, CDCSimHit*>::iterator pItEnd   = m_hitWithPosWeight.end();

    //    unsigned short sClayer     = 0;
    //    unsigned short sSuperLayer = 0;
    //    unsigned short sLayer      = 0;
    //    unsigned short sWire       = 0;
    //    CDCSimHit* fHit = nullptr;

    //Find a primary track close to the input 2'ndary hit in question
    for (std::vector<CDCSimHit*>::iterator nIt = m_hitWithNegWeight.begin(), nItEnd = m_hitWithNegWeight.end(); nIt != nItEnd; ++nIt) {

      sHit = *nIt;
      sPos    = sHit->getPosTrack();
      sWireId = sHit->getWireID();
      //      sClayer     = sWireId.getICLayer();
      //      sSuperLayer = sWireId.getISuperLayer();
      //      sLayer      = sWireId.getILayer();
      //      sWire       = sWireId.getIWire();
      //      fHit = sHit;
      unsigned short sClayer     = sWireId.getICLayer();
      unsigned short sSuperLayer = sWireId.getISuperLayer();
      unsigned short sLayer      = sWireId.getILayer();
      unsigned short sWire       = sWireId.getIWire();
      CDCSimHit*     fHit = sHit;

      pItBegin = m_hitWithPosWeight.find(sSuperLayer);
      pItEnd   = m_hitWithPosWeight.find(sSuperLayer + 1);
      /*
      if (sSuperLayer <= 8) {
      pItBegin = m_posWeightMapItBegin.at(sSuperLayer);
      pItEnd   = m_posWeightMapItEnd.at(sSuperLayer);
      } else {
      B2FATAL("CDCSensitiveDetector::EndOfEvent: invalid super-layer id ! " << sSuperLayer);
      }
      */

      minDistance2 = DBL_MAX;
      //      bestNeighb = 0;

      /*      for (std::multimap<unsigned short, CDCSimHit*>::iterator pIt = m_hitWithPosWeight.begin(); pIt != m_hitWithPosWeight.end(); ++pIt) {
        std::cout <<"superLyr#= " << pIt->first << std::endl;
      }
      */

      for (std::multimap<unsigned short, CDCSimHit*>::iterator pIt = pItBegin; pIt != pItEnd; ++pIt) {

        //scan hits in the same/neighboring cells
        pHit = pIt->second;
        pWireId = pHit->getWireID();
        //      neigh = areNeighbors(sWireId, pWireId);
        neighb = areNeighbors(sClayer, sSuperLayer, sLayer, sWire, pWireId);
        if (neighb != 0 || pWireId == sWireId) {
          distance2 = (pHit->getPosTrack() - sPos).Mag2();
          if (distance2 < minDistance2) {
            fHit = pHit;
            minDistance2 = distance2;
            //      bestNeighb = neighb;
          }
        }
      }

      //reassign LR using the momentum-direction of the primary particle found
      unsigned short lR = m_cdcgp->getNewLeftRightRaw(sHit->getPosWire(),
                                                      sHit->getPosTrack(),
                                                      fHit->getMomentum());
      //      unsigned short bflr = sHit->getLeftRightPassage();
      sHit->setLeftRightPassage(lR);
      //      std::cout <<"neighb, bfaf lrs, minDistance= " << bestNeighb <<" "<<" "<< bflr <<" "<< sHit->getLeftRightPassage() <<" "<< std::scientific << sqrt(minDistance2) << std::endl;
    }
  }


  unsigned short CDCSensitiveDetector::areNeighbors(const WireID& wireId, const WireID& otherWireId) const
  {
    //require within the same super-layer
    if (otherWireId.getISuperLayer() != wireId.getISuperLayer()) return 0;

    const signed short iWire       =      wireId.getIWire();
    const signed short iOtherWire  = otherWireId.getIWire();
    const signed short iCLayer     =      wireId.getICLayer();
    const signed short iOtherCLayer = otherWireId.getICLayer();

    //require nearby wire
    if (iWire == iOtherWire) {
    } else if (iWire == (iOtherWire + 1) % static_cast<signed short>(m_cdcgp->nWiresInLayer(iOtherCLayer))) {
    } else if ((iWire + 1) % static_cast<signed short>(m_cdcgp->nWiresInLayer(iCLayer)) == iOtherWire) {
    } else {
      return 0;
    }
    //  std::cout <<"iCLayer,iLayer,nShifts= " << iCLayer <<" "<< iLayer <<" "<< nShifts(iCLayer) << std::endl;

    signed short iLayerDifference = otherWireId.getILayer() - wireId.getILayer();
    if (abs(iLayerDifference) > 1) return 0;

    if (iLayerDifference == 0) {
      if (iWire == (iOtherWire + 1) % static_cast<signed short>(m_cdcgp->nWiresInLayer(iCLayer))) return CW_NEIGHBOR;
      else if ((iWire + 1) % static_cast<signed short>(m_cdcgp->nWiresInLayer(iCLayer)) == iOtherWire) return CCW_NEIGHBOR;
      else return 0;
    } else if (iLayerDifference == -1) {
      //    const CCWInfo deltaShift = otherLayer.getShift() - layer.getShift();
      const signed short deltaShift = m_cdcgp->getShiftInSuperLayer(otherWireId.getISuperLayer(), otherWireId.getILayer()) -
                                      m_cdcgp->getShiftInSuperLayer(wireId.getISuperLayer(), wireId.getILayer());
      //    std::cout <<"in deltaShift,iOtherWire,iWire= " << deltaShift <<" "<< iOtherWire <<" "<< iWire << std::endl;
      if (iWire == iOtherWire) {
        if (deltaShift ==  CW) return  CW_IN_NEIGHBOR;
        else if (deltaShift == CCW) return CCW_IN_NEIGHBOR;
        else return 0;
      } else if (iWire == (iOtherWire + 1) % static_cast<signed short>(m_cdcgp->nWiresInLayer(iOtherCLayer))) {
        if (deltaShift == CCW) return  CW_IN_NEIGHBOR;
        else return 0;
      } else if ((iWire + 1) % static_cast<signed short>(m_cdcgp->nWiresInLayer(iCLayer)) == iOtherWire) {
        if (deltaShift ==  CW) return CCW_IN_NEIGHBOR;
        else return 0;
      } else return 0;
    } else if (iLayerDifference == 1) {
      //    const CCWInfo deltaShift = otherLayer.getShift() - layer.getShift();
      const signed short deltaShift = m_cdcgp->getShiftInSuperLayer(otherWireId.getISuperLayer(), otherWireId.getILayer()) -
                                      m_cdcgp->getShiftInSuperLayer(wireId.getISuperLayer(), wireId.getILayer());
      //    std::cout <<"out deltaShift,iOtherWire,iWire= " << deltaShift <<" "<< iOtherWire <<" "<< iWire << std::endl;
      if (iWire == iOtherWire) {
        if (deltaShift ==  CW) return  CW_OUT_NEIGHBOR;
        else if (deltaShift == CCW) return CCW_OUT_NEIGHBOR;
        else return 0;
      } else if (iWire == (iOtherWire + 1) % static_cast<signed short>(m_cdcgp->nWiresInLayer(iOtherCLayer))) {
        if (deltaShift == CCW) return  CW_OUT_NEIGHBOR;
        else return 0;
      } else if ((iWire + 1) % static_cast<signed short>(m_cdcgp->nWiresInLayer(iCLayer)) == iOtherWire) {
        if (deltaShift ==  CW) return CCW_OUT_NEIGHBOR;
        else return 0;
      } else return 0;
    } else return 0;

  }

  unsigned short CDCSensitiveDetector::areNeighbors(unsigned short iCLayer, unsigned short iSuperLayer, unsigned short iLayer,
                                                    unsigned short iWire, const WireID& otherWireId) const
  {
    //require within the same super-layer
    if (otherWireId.getISuperLayer() != iSuperLayer) return 0;

    const signed short iOtherWire  = otherWireId.getIWire();
    const signed short iOtherCLayer = otherWireId.getICLayer();

    //require nearby wire
    if (iWire == iOtherWire) {
    } else if (iWire == (iOtherWire + 1) % static_cast<signed short>(m_cdcgp->nWiresInLayer(iOtherCLayer))) {
    } else if ((iWire + 1) % static_cast<signed short>(m_cdcgp->nWiresInLayer(iCLayer)) == iOtherWire) {
    } else {
      return 0;
    }

    //  std::cout <<"iCLayer,iLayer,nShifts= " << iCLayer <<" "<< iLayer <<" "<< nShifts(iCLayer) << std::endl;
    signed short iLayerDifference = otherWireId.getILayer() - iLayer;
    if (abs(iLayerDifference) > 1) return 0;

    if (iLayerDifference == 0) {
      if (iWire == (iOtherWire + 1) % static_cast<signed short>(m_cdcgp->nWiresInLayer(iCLayer))) return CW_NEIGHBOR;
      else if ((iWire + 1) % static_cast<signed short>(m_cdcgp->nWiresInLayer(iCLayer)) == iOtherWire) return CCW_NEIGHBOR;
      else return 0;
    } else if (iLayerDifference == -1) {
      //    const CCWInfo deltaShift = otherLayer.getShift() - layer.getShift();
      const signed short deltaShift = m_cdcgp->getShiftInSuperLayer(otherWireId.getISuperLayer(), otherWireId.getILayer()) -
                                      m_cdcgp->getShiftInSuperLayer(iSuperLayer, iLayer);
      //    std::cout <<"in deltaShift,iOtherWire,iWire= " << deltaShift <<" "<< iOtherWire <<" "<< iWire << std::endl;
      if (iWire == iOtherWire) {
        if (deltaShift ==  CW) return  CW_IN_NEIGHBOR;
        else if (deltaShift == CCW) return CCW_IN_NEIGHBOR;
        else return 0;
      } else if (iWire == (iOtherWire + 1) % static_cast<signed short>(m_cdcgp->nWiresInLayer(iOtherCLayer))) {
        if (deltaShift == CCW) return  CW_IN_NEIGHBOR;
        else return 0;
      } else if ((iWire + 1) % static_cast<signed short>(m_cdcgp->nWiresInLayer(iCLayer)) == iOtherWire) {
        if (deltaShift ==  CW) return CCW_IN_NEIGHBOR;
        else return 0;
      } else return 0;
    } else if (iLayerDifference == 1) {
      //    const CCWInfo deltaShift = otherLayer.getShift() - layer.getShift();
      const signed short deltaShift = m_cdcgp->getShiftInSuperLayer(otherWireId.getISuperLayer(), otherWireId.getILayer()) -
                                      m_cdcgp->getShiftInSuperLayer(iSuperLayer, iLayer);
      //    std::cout <<"out deltaShift,iOtherWire,iWire= " << deltaShift <<" "<< iOtherWire <<" "<< iWire << std::endl;
      if (iWire == iOtherWire) {
        if (deltaShift ==  CW) return  CW_OUT_NEIGHBOR;
        else if (deltaShift == CCW) return CCW_OUT_NEIGHBOR;
        else return 0;
      } else if (iWire == (iOtherWire + 1) % static_cast<signed short>(m_cdcgp->nWiresInLayer(iOtherCLayer))) {
        if (deltaShift == CCW) return  CW_OUT_NEIGHBOR;
        else return 0;
      } else if ((iWire + 1) % static_cast<signed short>(m_cdcgp->nWiresInLayer(iCLayer)) == iOtherWire) {
        if (deltaShift ==  CW) return CCW_OUT_NEIGHBOR;
        else return 0;
      } else return 0;
    } else return 0;

  }

} // namespace Belle2
