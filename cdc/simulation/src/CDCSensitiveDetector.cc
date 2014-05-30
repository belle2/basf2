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

#include <cdc/simulation/Helix.h>
#include <cdc/geometry/CDCGeometryPar.h>
#include <cdc/geometry/GeoCDCCreator.h>
#include <framework/logging/Logger.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>
#include <framework/gearbox/GearDir.h>
#include <framework/gearbox/Unit.h>
#include <cdc/dataobjects/CDCSimHit.h>
#include <cdc/dataobjects/CDCEBSimHit.h>

#include "G4Step.hh"
#include "G4SteppingManager.hh"
#include "G4SDManager.hh"
#include "G4TransportationManager.hh"
#include "G4FieldManager.hh"
#include "G4MagneticField.hh"

#include "CLHEP/Geometry/Vector3D.h"
#include "CLHEP/Geometry/Point3D.h"

#include "TVector3.h"

#ifndef ENABLE_BACKWARDS_COMPATIBILITY
typedef HepGeom::Point3D<double> HepPoint3D;
#endif
#ifndef ENABLE_BACKWARDS_COMPATIBILITY
typedef HepGeom::Vector3D<double> HepVector3D;
#endif



namespace Belle2 {
  using namespace CDC;

  CDCSensitiveDetector::CDCSensitiveDetector(G4String name, G4double thresholdEnergyDeposit, G4double thresholdKineticEnergy):
    SensitiveDetectorBase(name, Const::CDC), m_thresholdEnergyDeposit(thresholdEnergyDeposit),
    m_thresholdKineticEnergy(thresholdKineticEnergy), m_hitNumber(0), m_EBhitNumber(0)
  {
    StoreArray<MCParticle> mcParticles;
    StoreArray<CDCSimHit> cdcSimHits;
    StoreArray<CDCEBSimHit> cdcEBArray;
    RelationArray cdcSimHitRel(mcParticles, cdcSimHits);
    registerMCParticleRelation(cdcSimHitRel);
    cdcSimHits.registerAsPersistent();
    cdcEBArray.registerAsTransient();
    RelationArray::registerPersistent<MCParticle, CDCSimHit>();

    GearDir gd = GearDir("/Detector/DetectorComponent[@name=\"CDC\"]/Content");
    gd.append("/SensitiveDetector");
    m_thresholdEnergyDeposit =  Unit::convertValue(gd.getDouble("EnergyDepositionThreshold"), "eV");
    m_thresholdEnergyDeposit *= GeV;  //GeV to MeV
    m_thresholdKineticEnergy = 0.0; // Dummy to avoid a warning (tentative).
    //    B2INFO("Threshold energy " << m_thresholdEnergyDeposit);

    m_wireSag = gd.getBool("WireSag");
    B2INFO("Sense wire sag in CDCSensitiveDetector on(=1)/off(=0):" << m_wireSag);

    m_minTrackLength = gd.getDouble("MinTrackLength");
    B2INFO("MinTrackLength in CDCSensitiveDetector:" << m_minTrackLength);

  }

  void CDCSensitiveDetector::Initialize(G4HCofThisEvent*)
  {
    // Initialize
    m_nonUniformField = 0;
  }

  //-----------------------------------------------------
  // Method invoked for every step in sensitive detector
  //-----------------------------------------------------
  bool CDCSensitiveDetector::step(G4Step* aStep, G4TouchableHistory*)
  {
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

    // No save in MCParticle if track-length is short
    if (t.GetTrackLength() > m_minTrackLength) {
      Simulation::TrackInfo::getInfo(t).setIgnore(false);
    }

    const G4double charge = t.GetDefinition()->GetPDGCharge();

    //    const G4double tof = t.GetGlobalTime(); //tof at post step point
    //    if (isnan(tof)) {
    //      B2ERROR("SensitiveDetector: global time is nan");
    //      return false;
    //    }

    const G4int pid = t.GetDefinition()->GetPDGEncoding();
    const G4int trackID = t.GetTrackID();

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

    //--------------------------------------------------------------------------
    // check if in electronics board, if true, CDCEBSimHit will be created.
    //--------------------------------------------------------------------------
    G4LogicalVolume* aLogicalVolume = v.GetLogicalVolume();
    if ((aLogicalVolume->GetName()).find("Electronics") != std::string::npos) {
      double phi = (posIn + posOut).phi();
      saveEBSimHit(layerId, phi, trackID, pid, edep, momIn);
      return true;
    }

    // If neutral particles, ignore them.

    if (charge == 0.) return false;


    // Calculate cell ID
    CDCGeometryPar& cdcg = CDCGeometryPar::Instance();

    TVector3 tposIn(posIn.x() / cm, posIn.y() / cm, posIn.z() / cm);
    TVector3 tposOut(posOut.x() / cm, posOut.y() / cm, posOut.z() / cm);
    const unsigned idIn = cdcg.cellId(layerId, tposIn);
    const unsigned idOut = cdcg.cellId(layerId, tposOut);
#if defined(CDC_DEBUG)
    std::cout << "edep= " << edep << std::endl;
    std::cout << "idIn,idOut= " << idIn << " " << idOut << std::endl;
#endif

    // Calculate drift length
    std::vector<int> wires = WireId_in_hit_order(idIn, idOut, cdcg.nWiresInLayer(layerId));
    G4double sint(0.);
    const G4double s_in_layer = stepLength / cm;
    G4double xint[6] = {0};

    const G4ThreeVector momOut(out.GetMomentum().x(), out.GetMomentum().y(),
                               out.GetMomentum().z());
    const G4double speedIn  =  in.GetVelocity();
    const G4double speedOut = out.GetVelocity();
    const G4double speed    = 0.5 * (speedIn + speedOut);
    const G4double speedInCmPerNs = speed / cm;

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
      bool magneticField = true;
      if (Bfield[0] == 0. && Bfield[1] == 0. &&
          Bfield[2] == 0.) magneticField = false;
#if defined(CDC_DEBUG)
      std::cout << "Bfield= " << Bfield[0] << " " << Bfield[1] << " " << Bfield[2] << std::endl;
      std::cout << "magneticField= " << magneticField << std::endl;
#endif

      double distance = 0;
      G4ThreeVector posW(0, 0, 0);
      HepPoint3D onTrack;
      HepPoint3D pOnTrack;

      // Calculate forward/backward position of current wire
      const TVector3 tfw3v = cdcg.wireForwardPosition(layerId, wires[i]);
      const TVector3 tbw3v = cdcg.wireBackwardPosition(layerId, wires[i]);

      const HepPoint3D fwd(tfw3v.x(), tfw3v.y(), tfw3v.z());
      const HepPoint3D bck(tbw3v.x(), tbw3v.y(), tbw3v.z());

      if (magneticField) {
        // Cal. distance assuming helix track (still approximation)
        m_nonUniformField = 1;
        if (Bfield[0] == 0. && Bfield[1] == 0. &&
            Bfield[2] != 0.) m_nonUniformField = 0;

        const G4double B_kG[3] = {Bfield[0] / kilogauss,
                                  Bfield[1] / kilogauss,
                                  Bfield[2] / kilogauss
                                 };

        const HepPoint3D  x(pos[0] / cm, pos[1] / cm, pos[2] / cm);
        const HepVector3D p(momIn.x() / GeV, momIn.y() / GeV, momIn.z() / GeV);
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
            cdcg.getWirSagEffect(layerId, wires[i], q2[2], ywb_sag, ywf_sag);
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
          distance = ClosestApproach(bwp, fwp, posIn / cm, posOut / cm,
                                     hitPosition, wirePosition);
          if (m_wireSag) {
            G4double ywb_sag, ywf_sag;
            cdcg.getWirSagEffect(layerId, wires[i], wirePosition.z(), ywb_sag, ywf_sag);
            bwp.setY(ywb_sag);
            fwp.setY(ywf_sag);
            distance = ClosestApproach(bwp, fwp, posIn / cm, posOut / cm,
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
        distance = ClosestApproach(bwp, fwp, posIn / cm, posOut / cm,
                                   hitPosition, wirePosition);
        if (m_wireSag) {
          G4double ywb_sag, ywf_sag;
          cdcg.getWirSagEffect(layerId, wires[i], wirePosition.z(), ywb_sag, ywf_sag);
          bwp.setY(ywb_sag);
          fwp.setY(ywf_sag);
          distance = ClosestApproach(bwp, fwp, posIn / cm, posOut / cm,
                                     hitPosition, wirePosition);
        }

        onTrack.setX(hitPosition.x());
        onTrack.setY(hitPosition.y());
        onTrack.setZ(hitPosition.z());
        posW.setX(wirePosition.x());
        posW.setY(wirePosition.y());
        posW.setZ(wirePosition.z());
        //tentative setting
        pOnTrack.setX(0.5 * (momIn.x() + momOut.x()) / GeV);
        pOnTrack.setY(0.5 * (momIn.y() + momOut.y()) / GeV);
        pOnTrack.setZ(0.5 * (momIn.z() + momOut.z()) / GeV);
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
      distance *= cm;  onTrack *= cm;  posW *= cm;
      pOnTrack *= GeV;

      G4ThreeVector posTrack(onTrack.x(), onTrack.y(), onTrack.z());
      G4ThreeVector mom(pOnTrack.x(), pOnTrack.y(), pOnTrack.z());

      const TVector3 tPosW(posW.x(), posW.y(), posW.z());
      const TVector3 tPosTrack(posTrack.x(), posTrack.y(), posTrack.z());
      const TVector3 tMom(mom.x(), mom.y(), mom.z());
      G4int lr = cdcg.getOldLeftRight(tPosW, tPosTrack, tMom);
      G4int newLrRaw = cdcg.getNewLeftRightRaw(tPosW, tPosTrack, tMom);
      G4int newLr = newLrRaw; //tentative !

      if (nWires == 1) {

        //        saveSimHit(layerId, wires[i], trackID, pid, distance, tofBefore, edep, s_in_layer * cm, momIn, posW, posIn, posOut, posTrack, lr, newLrRaw, newLr, speed);
        saveSimHit(layerId, wires[i], trackID, pid, distance, tofBefore, edep, s_in_layer * cm, pOnTrack, posW, posIn, posOut, posTrack, lr, newLrRaw, newLr, speed);
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
        const G4double s2 = t.GetTrackLength() / cm;  //at post-step
        G4double s1 = (s2 - s_in_layer);  //at pre-step; varied later
        G4ThreeVector din = momIn;
        if (din.mag() != 0.) din /= momIn.mag();

        G4double  vent[6] = {posIn.x() / cm, posIn.y() / cm, posIn.z() / cm, din.x(), din.y(), din.z()};

        G4ThreeVector dot(momOut.x(), momOut.y(), momOut.z());
        if (dot.mag() != 0.) {
          dot /= dot.mag();
        } else {
          // Flight-direction is needed to set even when a particle stops
          dot = din;
        }

        G4double  vext[6] = {posOut.x() / cm, posOut.y() / cm, posOut.z() / cm, dot.x(), dot.y(), dot.z()};

        if (i > 0) {
          for (int j = 0; j < 6; ++j) vent[j] = xint[j];
          s1 = sint;
        }

        const G4int ic(3);  // cubic approximation of the track
        G4int    flag(0);
        G4double edep_in_cell(0.);
        G4double eLossInCell(0.);

        if (cel1 != cel2) {
#if defined(CDC_DEBUG)
          std::cout << "layerId,cel1,cel2= " << layerId << " " << cel1 << " " << cel2 << std::endl;
          std::cout << "vent= " << vent[0] << " " << vent[1] << " " << vent[2] << " " << vent[3] << " " << vent[4] << " " << vent[5] << std::endl;
          std::cout << "vext= " << vext[0] << " " << vext[1] << " " << vext[2] << " " << vext[3] << " " << vext[4] << " " << vext[5] << std::endl;
          std::cout << "s1,s2,ic= " << s1 << " " << s2 << " " << ic << std::endl;
#endif
          CellBound(layerId, cel1, cel2, vent, vext, s1, s2, ic, xint, sint, flag);
#if defined(CDC_DEBUG)
          std::cout << "flag,sint= " << flag << " " << sint << std::endl;
          std::cout << "xint= " << xint[0] << " " << xint[1] << " " << xint[2] << " " << xint[3] << " " << xint[4] << " " << xint[5] << std::endl;
#endif

          const G4double test = (sint - s1) / s_in_layer;
          if (test < 0. || test > 1.) {
            B2WARNING("CDCSensitiveDetector: Strange path length: " << "s1=" << " " << s1 << "sint=" << " " << sint << "s_in_layer=" << " " << s_in_layer);
          }
          edep_in_cell = edep * (sint - s1) / s_in_layer;

          const G4ThreeVector x_In(vent[0]*cm, vent[1]*cm, vent[2]*cm);
          const G4ThreeVector x_Out(xint[0]*cm, xint[1]*cm, xint[2]*cm);
          const G4ThreeVector p_In(momBefore * vent[3], momBefore * vent[4], momBefore * vent[5]);

          //          saveSimHit(layerId, wires[i], trackID, pid, distance, tofBefore, edep_in_cell, (sint - s1) * cm, p_In, posW, x_In, x_Out, posTrack, lr, newLrRaw, newLr, speed);
          saveSimHit(layerId, wires[i], trackID, pid, distance, tofBefore, edep_in_cell, (sint - s1) * cm, pOnTrack, posW, x_In, x_Out, posTrack, lr, newLrRaw, newLr, speed);
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
            B2WARNING("CDCSensitiveDetector: Kinetic Energy < 0.")
            momBefore = 0.;
          }

        } else {  //the particle exits

          const G4double test = (s2 - sint) / s_in_layer;
          if (test < 0. || test > 1.) {
            B2WARNING("CDCSensitiveDetector: Strange path length: " << "s1=" << " " << s1 << "sint=" << " " << sint << "s_in_layer=" << " " << s_in_layer);
          }
          edep_in_cell = edep * (s2 - sint) / s_in_layer;

          const G4ThreeVector x_In(vent[0]*cm, vent[1]*cm, vent[2]*cm);
          const G4ThreeVector p_In(momBefore * vent[3], momBefore * vent[4], momBefore * vent[5]);

          //          saveSimHit(layerId, wires[i], trackID, pid, distance, tofBefore, edep_in_cell, (s2 - sint) * cm, p_In, posW, x_In, posOut, posTrack, lr, newLrRaw, newLr, speed);
          saveSimHit(layerId, wires[i], trackID, pid, distance, tofBefore, edep_in_cell, (s2 - sint) * cm, pOnTrack, posW, x_In, posOut, posTrack, lr, newLrRaw, newLr, speed);
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


  //  void CDCSensitiveDetector::EndOfEvent(G4HCofThisEvent*)
  //  {
  //  }

  int
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
                                   const G4double speed)
  {

    // Discard the hit below Edep_th
    if (edep <= m_thresholdEnergyDeposit) return 0;

    //compute tof at the closest point; linear approx.
    const G4double CorrectTof = tof + (posTrack - posIn).mag() / speed;
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

    CDCSimHit* simHit =  cdcArray.appendNew();

    simHit->setWireID(layerId, wireId);
    simHit->setTrackId(trackID);
    simHit->setPDGCode(pid);
    simHit->setDriftLength(distance / cm);
    simHit->setFlightTime(CorrectTof / ns);
    simHit->setGlobalTime(CorrectTof / ns);
    simHit->setEnergyDep(edep / GeV);
    simHit->setStepLength(stepLength / cm);
    TVector3 momentum(mom.getX() / GeV, mom.getY() / GeV, mom.getZ() / GeV);
    simHit->setMomentum(momentum);
    TVector3 posWire(posW.getX() / cm, posW.getY() / cm, posW.getZ() / cm);
    simHit->setPosWire(posWire);
    TVector3 positionIn(posIn.getX() / cm, posIn.getY() / cm, posIn.getZ() / cm);
    simHit->setPosIn(positionIn);
    TVector3 positionOut(posOut.getX() / cm, posOut.getY() / cm, posOut.getZ() / cm);
    simHit->setPosOut(positionOut);
    TVector3 positionTrack(posTrack.getX() / cm, posTrack.getY() / cm, posTrack.getZ() / cm);
    simHit->setPosTrack(positionTrack);
    simHit->setPosFlag(lr);
    simHit->setLeftRightPassageRaw(newLrRaw);
    simHit->setLeftRightPassage(newLr);
#if defined(CDC_DEBUG)
    std::cout << "sensitived,oldlr,newlrRaw,newlr= " << lr << " " << newLrRaw << " " << newLr << std::endl;
#endif

    B2DEBUG(150, "HitNumber: " << m_hitNumber);
    cdcSimHitRel.add(trackID, m_hitNumber);
    return (m_hitNumber);
  }

  int
  CDCSensitiveDetector::saveEBSimHit(const G4int layerId,
                                     const G4double phi,
                                     const G4int trackID,
                                     const G4int pid,
                                     const G4double edep,
                                     const G4ThreeVector& mom)
  {
    //change Later
    StoreArray<CDCEBSimHit> cdcEBArray;


    m_EBhitNumber = cdcEBArray.getEntries();
    CDCEBSimHit* simEBHit = cdcEBArray.appendNew();

    simEBHit->setLayerId(layerId);
    simEBHit->setPhi(phi);
    simEBHit->setTrackId(trackID);
    simEBHit->setPDGCode(pid);
    simEBHit->setEnergyDep(edep / GeV);
    TVector3 momentum(mom.getX() / GeV, mom.getY() / GeV, mom.getZ() / GeV);
    simEBHit->setMomentum(momentum);

    B2DEBUG(150, "HitNumber: " << m_EBhitNumber);
    return (m_EBhitNumber);
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
                                  const G4int ic, G4double xint[6],
                                  G4double& sint, G4int& iflag)
  {
    //dead copy of gsim_cdc_cellbound.F in gsim-cdc for Belle (for tentaive use)
    //---------------------------------------------------------------------------
    // (Purpose)
    //    calculate intersection of track with cell boundary.
    //
    // (Relations)
    //    Calls       RotVec, GIPLAN
    //
    // (Arguments)
    //   input
    //     ic1        integer * 4   #cell(serial) of entrance.
    //     ic2        integer * 4   #cell(serial) of exit.
    //     venter(6)  real * 4      (x,y,z,px/p,py/p,pz/p) at entrance.
    //     vexit(6)   real * 4      (x,y,z,px/p,py/p,pz/p) at exit.
    //     s1         real * 4      track length at entrance.
    //     s2         real * 4      track length at exit.
    //     ic         integer * 4   mode for GIPLAN ( ic=3: cubic model )
    //   output
    //     xint(6)    real * 4      (x,y,z,px/p,py/p,pz/p) at intersection of
    //                              cell boundary.
    //     sint       real * 4      track length at intersection of cell
    //                              boundary.
    //     iflag      integer * 4   return code from GIPLAN.
    //---------------------------------------------------------------------------
    const G4double yshift = 0.5;

    //main
    G4int irTry = 0;
    CDCGeometryPar& p_cdc = CDCGeometryPar::Instance();
    G4double div   = p_cdc.nWiresInLayer(layerId);

    //Check if ic1 and ic2 are ok
    if (std::abs(ic1 - ic2) != 1) {
      if (ic1 == 1 && ic2 == div) {
      } else if (ic1 == div && ic2 == 1) {
      } else {
        B2ERROR("CDCSensitiveDetector: |ic1 - ic2| != 1 in CellBound; " << "ic1=" << ic1 << " " << "ic2=" << ic2);
      }
    }

    // Calculate forward/backward position of current wire
    const TVector3 fw0 = p_cdc.wireForwardPosition(layerId, 0);
    const TVector3 bw0 = p_cdc.wireBackwardPosition(layerId, 0);

    G4double slant;
    if (0 == p_cdc.nShifts(layerId)) {
      slant = 0.0;
    } else {
      double delfi = double(p_cdc.nShifts(layerId)) * CLHEP::pi / p_cdc.nWiresInLayer(layerId);
      double sinhdel = std::sin(delfi / 2.0);
      double z1 = fw0.z();
      double z2 = bw0.z();
      slant = std::atan2(2.0 * (p_cdc.senseWireR(layerId)) * sinhdel, z1 - z2);
#if defined(CDC_DEBUG)
      std::cout << "nwires = " << p_cdc.nWiresInLayer(layerId) << std::endl;
      std::cout << "nshift = " << p_cdc.nShifts(layerId) << std::endl;
      std::cout << "delfi  = " << delfi   << std::endl;
#endif
    }
#if defined(CDC_DEBUG)
    std::cout << "layerId,slant= " << layerId << " " << slant << std::endl;
    std::cout << "R      = " << p_cdc.senseWireR(layerId) << std::endl;
    std::cout << "z1,z2  = " << fw0.z() << " " << bw0.z() << std::endl;
    std::cout << "nshifts= " << p_cdc.nShifts(layerId) << std::endl;
#endif
    G4double xwb   = (p_cdc.wireBackwardPosition(layerId, ic1 - 1)).x();
    G4double ywb   = (p_cdc.wireBackwardPosition(layerId, ic1 - 1)).y();
    G4double zwb   = (p_cdc.wireBackwardPosition(layerId, ic1 - 1)).z();
    G4double xwf   = (p_cdc.wireForwardPosition(layerId, ic1 - 1)).x();
    G4double ywf   = (p_cdc.wireForwardPosition(layerId, ic1 - 1)).y();
    G4double zwf   = (p_cdc.wireForwardPosition(layerId, ic1 - 1)).z();

L100:
    //copy arrays
    G4double xx1[6], xx2[6];
    for (int i = 0; i < 6; ++i) {
      xx1[i] = venter[i];
      xx2[i] = vexit [i];
    }

    //calculate rotation angles for phi & theta
    G4int mode;
    G4double psi, cospsi, sinpsi, phi, theta;
    G4double xfwb, yfwb, zfwb, xfwf, yfwf, zfwf;
    G4double zw = 0.0;

    if (slant == 0.0) {
      mode  = 2;
      theta = 0.;
      psi    = double(ic2 - ic1) * CLHEP::pi / div;
      cospsi = cos(psi);
      sinpsi = sin(psi);
      xfwb   = cospsi * xwb - sinpsi * ywb;
      yfwb   = sinpsi * xwb + cospsi * ywb;
      phi = atan2(yfwb, xfwb);
      if (phi < 0.0) phi += 2.*CLHEP::pi;
    } else {
      mode = 0;
      theta = slant;
      psi    = double(ic2 - ic1) * CLHEP::pi / div;
      cospsi = cos(psi);
      sinpsi = sin(psi);
      xfwb   = cospsi * xwb - sinpsi * ywb;
      yfwb   = sinpsi * xwb + cospsi * ywb;
      xfwf   = cospsi * xwf - sinpsi * ywf;
      yfwf   = sinpsi * xwf + cospsi * ywf;
      zfwb   = zwb;
      zfwf   = zwf;

      G4double vx = xfwf - xfwb;
      G4double vy = yfwf - yfwb;
      G4double vz = zfwf - zfwb;
      G4double vv = sqrt(vx * vx + vy * vy + vz * vz);
      vx /= vv;
      vy /= vv;
      vz /= vv;

      if (irTry == 0) {
        zw   = 0.5 * (xx1[2] + xx2[2]);
        G4double beta = (zw - zfwb) / vz;
        G4double xfw  = xfwb + beta * vx;
        G4double yfw  = yfwb + beta * vy;
        phi = atan2(yfw, xfw);
        if (phi < 0.0) phi += 2.*M_PI;
      } else if (irTry == 1) {
        phi   = atan2(xx2[1], xx2[0]);
        if (phi < 0.0) phi += 2.*M_PI;
        zw = xx2[2];
      } else {
        phi   = atan2(xx1[1], xx1[0]);
        if (phi < 0.0) phi += 2.*M_PI;
        zw = xx1[2];
      }
      xx1[2] -= zw;
      xx2[2] -= zw;
    }

    //rotate to the plane parallele to the x-z plane.
    RotVec(xx1[0], xx1[1], xx1[2], phi, theta, mode);
    RotVec(xx1[3], xx1[4], xx1[5], phi, theta, mode);
    RotVec(xx2[0], xx2[1], xx2[2], phi, theta, mode);
    RotVec(xx2[3], xx2[4], xx2[5], phi, theta, mode);

    //check y position for xx1 & xx2
    G4double yadj = 0.0;
    if (xx1[1]*xx2[1] > 0.0) {
      if (mode == 0  && irTry <= 1) {
        irTry += 1;
        goto L100;
      }
      yadj = 0.5 * (xx1[1] + xx2[1]);

      //      //adjust yadj (cell-boundary), because GIPLAN requires yadj be btw xx2[1] and xx1[1]
      //      const G4double epsl = 1.e-3;
      //      if( xx1[1] < xx2[1] ) {
      //  if( 0. < xx1[1] ) yadj =
      //    xx1[1] + epsl*(xx2[1]-xx1[1])/std::abs(xx2[1]-xx1[1]);
      //  if( xx2[1] < 0. ) yadj =
      //    xx2[1] + epsl*(xx1[1]-xx2[1])/std::abs(xx1[1]-xx2[1]);
      //      }
      //      if( xx2[1] < xx1[1] ) {
      //  if( 0. < xx2[1] ) yadj =
      //    xx2[1] + epsl*(xx1[1]-xx2[1])/std::abs(xx1[1]-xx2[1]);
      //  if( xx1[1] < 0. ) yadj =
      //    xx1[1] + epsl*(xx2[1]-xx1[1])/std::abs(xx2[1]-xx1[1]);
      //      }
    }

    //get intersection using stupid GEANT tool.
    G4double yc = yshift + yadj;
    xx1[1] += yshift;
    xx2[1] += yshift;
    G4double pzint[4];
    GIPLAN(yc, xx1, xx2, s1, s2, ic, xint, sint, pzint, iflag);

    //reverse rotation
    if (iflag == 1) {
      xint[1] -= yshift;
      RotVec(xint[0], xint[1], xint[2], phi, theta, mode + 1);
      RotVec(xint[3], xint[4], xint[5], phi, theta, mode + 1);
      if (mode == 0) xint[2] += zw;
    } else {
      B2WARNING("CDCSensitiveDetector: No intersection " << iflag << " " << xx1[1] << " " << xx2[1] << " " << yc << " " << irTry << " " << ic1 << " " << ic2);
      //      B2WARNING("Retry with line approximation");
      GIPLAN(yc, xx1, xx2, s1, s2, 2, xint, sint, pzint, iflag);
      if (iflag == 1) {
        xint[1] -= yshift;
        RotVec(xint[0], xint[1], xint[2], phi, theta, mode + 1);
        RotVec(xint[3], xint[4], xint[5], phi, theta, mode + 1);
        if (mode == 0) xint[2] += zw;
        //  B2INFO(" ");
        //  B2INFO("mode= " << mode);
        //  B2INFO("phi= " << phi);
        //  B2INFO("theta= " << theta);
        //  B2INFO("yc= " << yc);
        //  B2INFO("xx2= " << xx2);
        //  B2INFO("s1= " << s1);
        //  B2INFO("s2= " << s2);
        //  B2INFO("ic= " << ic);
        //  B2INFO("xint= " << xint);
        //  B2INFO("sint= " << sint);
        //  B2INFO("venter= " << venter);
        //  B2INFO("vexit= " << vexit);
      } else {
        //B2INFO("Fail again");
      }
    }

  }

  void CDCSensitiveDetector::RotVec(G4double& x, G4double& y, G4double& z, const G4double phi, const G4double theta, const G4int mode)
  {
    //dead copy of UtilCDC_RotVec in com-cdc for Belle (for tentative use)
    //----------------------------------------------------------------------
    //  (Purpose)
    //
    //      Rotate coordinate axes in such a way that the stareo wire is
    //      parallel to Z-axis in X-Z plane in the new coordinate sytem
    //        Rotation is done assuming Z=0 is the center of CDC (so trans-
    //      lation must be done before this routine if CDC not centered to
    //      origin of coordinate system).
    //
    //      Rotation order;
    //        1) rotate in phi  : (Xw,Yw,0) --> (Xw',0,0), wire //to Y-Z       '
    //        2) rotate in theta: wire // to Z-axis
    //
    //  (Input)
    //       MODE    = 0    : twice rotation (phi --> theta)
    //               = 1    : twice reverse rotation (-theta --> -phi)
    //               = 2    : once rotation (phi only)
    //               = 3    : once reverse rotation (-phi only)
    //       X       = X compnent of a vector
    //       Y       = Y compnent of a vector
    //       Z       = Z compnent of a vector
    //       phi     = phi
    //       theta   = theta
    //  (Output)
    //       X       = X compnent of a vector after rotation
    //       Y       = Y compnent of a vector after rotation
    //       Z       = Z compnent of a vector after rotation
    //
    //  (Relation)
    //       None
    //-----------------------------------------------------------------------

    G4double cosst = 0., sinst = 0., xx, yy, zz;

    G4double phiw = phi;
    G4double cs = cos(phiw);
    G4double sn = sin(phiw);

    if (mode <= 1) {
      G4double ang = theta;
      cosst = cos(ang);
      sinst = sin(ang);
    }

    if (mode == 0) {
      xx = x * cs + y * sn;
      yy = y * cs - x * sn;
      zz = z;
      x = xx;
      y = yy * cosst - zz * sinst;
      z = zz * cosst + yy * sinst;

    }  else if (mode == 1) {
      xx = x;
      yy = y * cosst + z * sinst;
      zz = z * cosst - y * sinst;
      x = xx * cs - yy * sn;
      y = yy * cs + xx * sn;
      z = zz;

    } else if (mode == 2) {
      xx = x * cs + y * sn;
      y  = y * cs - x * sn;
      x  = xx;

    } else if (mode == 3) {
      xx = x * cs - y * sn;
      y  = y * cs + x * sn;
      x  = xx;
    }

  }

  void
  CDCSensitiveDetector::GIPLAN(const G4double yc, const G4double x1[6], const G4double x2[6],
                               const G4double s1, const G4double s2, const G4int ic,
                               G4double xint[6], G4double& sint, G4double pzint[4], G4int& iflag)
  {

    //dead copy of GIPLAN in Geant3 (for tentative use)
    //.    ******************************************************************
    //    *                                                                *
    //    *       Calculates intersection of track (X1,X2)                 *
    //    *       with plane parallel to (X-Z)                             *
    //    *        The track is approximated by a cubic in the             *
    //    *       track length.                                            *
    //    *       To improve stability, the coordinate system              *
    //    *       is shifted.                                              *
    //    *       input parameters                                         *
    //    *        YC    = Y COORDINATE OF PLANE                           *
    //    *        X1    = X,Y,Z,XP,YP,ZP OF 1ST POINT                     *
    //    *        X2    =                   2ND                           *
    //    *        S1(2) = S AT 1ST(2ND) POINT                             *
    //    *        IC    = 1 STRAIGHT LINE DEFINED BY X+XP                 *
    //    *        IC    = 2 STRAIGHT LINE DEFINED BY X1+X2                *
    //    *        IC    = 3 CUBIC MODEL                                   *
    //    *                                                                *
    //    *      output parameters                                         *
    //    *        XINT  = X,Y,Z,XP,YP,ZP AT INTERSECTION POINT            *
    //    *        SINT  = S AT INTERSECTION POINT                         *
    //    *        PZINT = PHI,Z,DPHI/DR,DZ/DR                             *
    //    *        IFLAG = 1 IF TRACK INTERSECTS PLANE                     *
    //    *              = 0 IF NOT                                        *
    //    *                                                                *
    //    *      Warning : the default accuracy is 10 microns. The value   *
    //    *      of EPSI must be changed for a better precision            *
    //    *                                                                *
    //    *    ==>Called by : <USER>, GUDIGI                               *
    //    *                                                                *
    //    *        Authors: R.BRUN/JJ.DUMONT from an original routine by   *
    //    *       H. BOERNER  KEK  OCTOBER 1982                            *
    //    *                                                                *
    //    *                                                                *
    //    ******************************************************************

    const G4int maxhit(100);
    const G4double epsi(0.001); //10um

    iflag = 1;
    G4double drctn = 1.;
    G4double s(0), dxds(0), dyds(0), dzds(0), bx(0), bz(0), trl2(0);
    G4double dx(0), dy(0), dz(0), ds(0), trlen(0);
    G4double shiftx(0), shifty(0), shiftz(0), shifts(0);
    G4double xshft(0), yshft(0), zshft(0), sshft(0), pabs1(0), pabs2(0);
    G4double dinter(0), term(0);
    G4double a[4] = {0}, b[4] = {0}, c[4] = {0};

    //Track crossing the plane from above or below ?

    if (x2[1] < x1[1])                        goto L5;
    if (yc < x1[1])                           goto L90;
    if (yc > x2[1])                           goto L90;
    if (ic == 2) goto L30;
    if (ic == 3) goto L7;

    s = s1;
    dxds = x1[3];
    dyds = x1[4];
    dzds = x1[5];
    bx = x1[0] - dxds * (x1[1] - yc) / dyds;
    bz = x1[2] - dzds * (x1[1] - yc) / dyds;
    trl2 = (bx - x1[0]) * (bx - x1[0])
           + (x1[1] - yc) * (x1[1] - yc)
           + (bz - x1[2]) * (bz - x1[2]);
    goto L40;

L5:
    if (yc < x2[1])                           goto L90;
    if (yc > x1[1])                           goto L90;
    if (ic == 2) goto L30;
    drctn = -1.;

    if (ic == 3) goto L7;
    s = s2;
    dxds = x2[3];
    dyds = x2[4];
    dzds = x2[5];
    bx = x2[0] - dxds * (x2[1] - yc) / dyds;
    bz = x2[2] - dzds * (x2[1] - yc) / dyds;
    trl2 = (bx - x2[0]) * (bx - x2[0])
           + (x2[1] - yc) * (x2[1] - yc)
           + (bz - x2[2]) * (bz - x2[2]);
    goto  L40;

L30:
    dx = x2[0] - x1[0];
    dy = x2[1] - x1[1];
    dz = x2[2] - x1[2];
    ds = sqrt(dx * dx + dy * dy + dz * dz);
    s = s1;
    dxds = dx / ds;
    dyds = dy / ds;
    dzds = dz / ds;
    bx = x1[0] - dx * (x1[1] - yc) / dy;
    bz = x1[2] - dz * (x1[1] - yc) / dy;
    trl2 = (bx - x1[0]) * (bx - x1[0])
           + (x1[1] - yc) * (x1[1] - yc)
           + (bz - x1[2]) * (bz - x1[2]);

L40:
    trlen = sqrt(trl2) * drctn + s;
    xint[0] = bx;
    xint[1] = yc;
    xint[2] = bz;
    sint = trlen;
    xint[3] = dxds;
    xint[4] = dyds;
    xint[5] = dzds;
    goto L200;

    //Shift coordinate system such that center of gravity=0

L7:
    if (yc <= 0.)                                goto L90;
    shiftx = (x1[0] + x2[0]) * 0.5;
    shifty = (x1[1] + x2[1]) * 0.5;
    shiftz = (x1[2] + x2[2]) * 0.5;
    shifts = (s1 + s2) * 0.5;

    //Only one value necessary since X1= -X2 etc...

    xshft = x1[0] - shiftx;
    yshft = x1[1] - shifty;
    zshft = x1[2] - shiftz;
    sshft = s1    - shifts;

    pabs1 = sqrt(x1[3] * x1[3] + x1[4] * x1[4] + x1[5] * x1[5]);
    pabs2 = sqrt(x2[3] * x2[3] + x2[4] * x2[4] + x2[5] * x2[5]);
    if (pabs1 == 0. || pabs2 == 0.)            goto L90;

    //Parametrize the track by a cubic through X1, X2

    GCUBS(sshft, xshft, x1[3] / pabs1, x2[3] / pabs2, a);
    GCUBS(sshft, yshft, x1[4] / pabs1, x2[4] / pabs2, b);
    GCUBS(sshft, zshft, x1[5] / pabs1, x2[5] / pabs2, c);

    //Iterate to find the track length corresponding to
    //the intersection of track and plane.
    //Start at S=0. middle of the shifted interval.

    dinter = std::abs(s2 - s1) * 0.5;
    s = 0.;

    for (int i = 1; i <= maxhit; ++i) {
      G4double y = shifty + b[0] + s * (b[1] + s * (b[2] + s * b[3]));
      G4double dr = (yc - y) * drctn;
      if (std::abs(dr) <  epsi)                     goto L20;
      dinter *= 0.5;
      if (dr <  0.) s -= dinter;
      if (dr >= 0.) s += dinter;
    }

    //Compute intersection in original coordinates

L20:
    xint[0] = shiftx + a[0] + s * (a[1] + s * (a[2] + s * a[3]));
    xint[1] = yc;
    xint[2] = shiftz + c[0] + s * (c[1] + s * (c[2] + s * c[3]));
    xint[3] = a[1] + s * (2. * a[2] + 3. * s * a[3]);
    xint[4] = b[1] + s * (2. * b[2] + 3. * s * b[3]);
    xint[5] = c[1] + s * (2. * c[2] + 3. * s * c[3]);

    //Compute PHIHIT,ZHIT and corresponding derivatives

    sint = s + shifts;
L200:
    term = 1. / (xint[3] * xint[0] + xint[4] * xint[1]);
    pzint[0] = atan2(xint[1], xint[0]);
    pzint[1] = xint[2];
    pzint[2] = (xint[0] * xint[4] - xint[1] * xint[3]) * term / yc;
    pzint[3] = term * xint[5] * yc;
    return;

L90:
    iflag = 0;
  }


  void CDCSensitiveDetector::GCUBS(const G4double x, const G4double y, const G4double d1, const G4double d2, G4double a[4])
  {
    //dead copy of GCUBS in Geant3 (for tentative use)
    //    ******************************************************************
    //    *                                                                *
    //    *       Calculates a cubic through P1,(-X,Y1),(X,Y2),P2          *
    //    *       where Y2=-Y1                                             *
    //    *        Y=A(1)+A(2)*X+A(3)*X**2+A(4)*X**3                       *
    //    *        The coordinate system is assumed to be the cms system   *
    //    *        of P1,P2.                                               *
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

    brot[0][0] = bz / bxz;
    brot[1][0] = 0.;
    brot[2][0] = -bx / bxz;
    brot[0][1] = -by * bx / bxz / bfield;
    brot[1][1] = bxz     / bfield;
    brot[2][1] = -by * bz / bxz / bfield;
    brot[0][2] = bx / bfield;
    brot[1][2] = by / bfield;
    brot[2][2] = bz / bfield;

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
      x = brot[0][0] * x0 + brot[1][0] * y0 + brot[2][0] * z0;
      y = brot[0][1] * x0 + brot[1][1] * y0 + brot[2][1] * z0;
      z = brot[0][2] * x0 + brot[1][2] * y0 + brot[2][2] * z0;
    } else if (mode == -1) {
      x = brot[0][0] * x0 + brot[0][1] * y0 + brot[0][2] * z0;
      y = brot[1][0] * x0 + brot[1][1] * y0 + brot[1][2] * z0;
      z = brot[2][0] * x0 + brot[2][1] * y0 + brot[2][2] * z0;
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
      x[0] = brot[0][0] * x0 + brot[1][0] * y0 + brot[2][0] * z0;
      x[1] = brot[0][1] * x0 + brot[1][1] * y0 + brot[2][1] * z0;
      x[2] = brot[0][2] * x0 + brot[1][2] * y0 + brot[2][2] * z0;
    } else if (mode == -1) {
      x[0] = brot[0][0] * x0 + brot[0][1] * y0 + brot[0][2] * z0;
      x[1] = brot[1][0] * x0 + brot[1][1] * y0 + brot[1][2] * z0;
      x[2] = brot[2][0] * x0 + brot[2][1] * y0 + brot[2][2] * z0;
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
    alpha  = 1.e4 / 2.99792458 / bfield;
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

  G4double CDCSensitiveDetector::ClosestApproach(const G4ThreeVector bwp, const G4ThreeVector fwp, const G4ThreeVector posIn, const G4ThreeVector posOut, G4ThreeVector& hitPosition, G4ThreeVector& wirePosition)//,G4double& transferT)
  {
    //----------------------------------------------------------
    /* For two lines r=r1+t1.v1 & r=r2+t2.v2
       the closest approach is d=|(r2-r1).(v1 x v2)|/|v1 x v2|
       the point where closest approach are
       t1=(v1 x v2).[(r2-r1) x v2]/[(v1 x v2).(v1 x v2)]
       t2=(v1 x v2).[(r2-r1) x v1]/[(v1 x v2).(v1 x v2)]
       if v1 x v2=0 means two lines are parallel
       d=|(r2-r1) x v1|/|v1|
    */

    G4double t1, t2, distance, dInOut, dHitIn, dHitOut;

    //--------------------------
    // Get wirepoint @ endplate
    //--------------------------
    /*    CDCGeometryPar& cdcgp = CDCGeometryPar::Instance();
    TVector3 tfwp = cdcgp.wireForwardPosition(layerId, cellId);
    G4ThreeVector fwp(tfwp.x(), tfwp.y(), tfwp.z());
    TVector3 tbwp = cdcgp.wireBackwardPosition(layerId, cellId);
    G4ThreeVector bwp(tbwp.x(), tbwp.y(), tbwp.z());
    */
    G4ThreeVector wireLine = fwp - bwp;
    G4ThreeVector hitLine = posOut - posIn;

    G4ThreeVector hitXwire = hitLine.cross(wireLine);
    G4ThreeVector wire2hit = fwp - posOut;

    //----------------------------------------------------------------
    // Hitposition is the position on hit line where closest approach
    // of two lines, but it may out the area from posIn to posOut
    //----------------------------------------------------------------
    if (hitXwire.mag() == 0) {
      distance = wireLine.cross(wire2hit).mag() / wireLine.mag();
      hitPosition = posIn;
      t2 = (posIn - fwp).dot(wireLine) / wireLine.mag2();
    } else {
      t1 = hitXwire.dot(wire2hit.cross(wireLine)) / hitXwire.mag2();
      hitPosition = posOut + t1 * hitLine;
      t2 = hitXwire.dot(wire2hit.cross(hitLine)) / hitXwire.mag2();

      dInOut = (posOut - posIn).mag();
      dHitIn = (hitPosition - posIn).mag();
      dHitOut = (hitPosition - posOut).mag();
      if (dHitIn <= dInOut && dHitOut <= dInOut) { //Between point in & out
        distance = fabs(wire2hit.dot(hitXwire) / hitXwire.mag());
      } else if (dHitOut > dHitIn) { // out posIn
        distance = wireLine.cross(posIn - fwp).mag() / wireLine.mag();
        hitPosition = posIn;
        t2 = (posIn - fwp).dot(wireLine) / wireLine.mag2();
      } else { // out posOut
        distance = wireLine.cross(posOut - fwp).mag() / wireLine.mag();
        hitPosition = posOut;
        t2 = (posOut - fwp).dot(wireLine) / wireLine.mag2();
      }
    }

    wirePosition = fwp + t2 * wireLine;

    return distance;

  }
} // namespace Belle2
