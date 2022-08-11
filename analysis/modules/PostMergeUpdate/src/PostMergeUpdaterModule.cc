/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <analysis/modules/PostMergeUpdate/PostMergeUpdaterModule.h>
#include <mdst/dbobjects/BeamSpot.h>
#include <iostream>
#include <framework/logging/Logger.h>
#include <analysis/utility/RotationTools.h>
#include <analysis/utility/PCmsLabTransform.h>
#include <TDatabasePDG.h>

namespace Belle2 {
  //-----------------------------------------------------------------
  //                 Register module
  //-----------------------------------------------------------------

  REG_MODULE(PostMergeUpdater)

  // Implementation

  PostMergeUpdaterModule::PostMergeUpdaterModule() : Module()
  {
    setDescription("Synchronize parts of the evnets post merge/embedding.");
    addParam("Mixing", m_mixing, "Mixing (true) or embedding (false) corrections", false);
    addParam("isCharged", m_isCharged, "Charged (true) or neutral (false) B mesons", true);
  }

  void PostMergeUpdaterModule::initialize()
  {
    m_mergedArrayIndices.isRequired("MergedArrayIndices");
    m_eventExtraInfo.isRequired("EventExtraInfo_indepPath"); // indepPath suffix is hardwired for no
    m_eventExtraInfo_orig.isRequired("EventExtraInfo"); // original extra info
    m_trackFits.isOptional();
    m_tracks.isOptional();
    m_eclclusters.isOptional();
  }

  TRotation PostMergeUpdaterModule::tag_vertex_rotation()
  {
    // Unit rotation:
    TRotation rot;

    if (
      m_eventExtraInfo_orig->hasExtraInfo("PX")
      and m_eventExtraInfo_orig->hasExtraInfo("PY")
      and m_eventExtraInfo_orig->hasExtraInfo("PZ")
      and m_eventExtraInfo->hasExtraInfo("PX")
      and m_eventExtraInfo->hasExtraInfo("PY")
      and m_eventExtraInfo->hasExtraInfo("PZ")
    ) {
      // We will rotate the second tag (simulated) B in the direction of the reconstructed:
      B2Vector3D tag3v = B2Vector3D(m_eventExtraInfo_orig->getExtraInfo("PX"),
                                    m_eventExtraInfo_orig->getExtraInfo("PY"),
                                    m_eventExtraInfo_orig->getExtraInfo("PZ")
                                   );

      B2Vector3D sec3v = B2Vector3D(m_eventExtraInfo->getExtraInfo("PX"),
                                    m_eventExtraInfo->getExtraInfo("PY"),
                                    m_eventExtraInfo->getExtraInfo("PZ")
                                   );

      // std::cout << " VV1: " << tag3v.X() << " " << tag3v.Y() << " "<<tag3v.Z();
      // std::cout << " VV2: " << sec3v.X() << " " << sec3v.Y() << " "<<sec3v.Z();

      if (m_mixing) {
        // We want in fact get opposite direction, secondary ROE should point in the direction of primary TAG
        PCmsLabTransform T;

        const int iPDG =  m_isCharged ? 521 : 511 ;
        const double mB = TDatabasePDG::Instance()->GetParticle(iPDG)->Mass();
        double E = sqrt(mB * mB + sec3v.Mag2());
        ROOT::Math::PxPyPzEVector sec4v(sec3v.X(), sec3v.Y(), sec3v.Z(), E);

        // std::cout << " mb " <<  mB;

        ROOT::Math::PxPyPzEVector secCMS = T.labToCms(sec4v);
        // relection:
        ROOT::Math::PxPyPzEVector secRoeCMS(-secCMS.X(), -secCMS.Y(), -secCMS.Z(), secCMS.E());
        ROOT::Math::PxPyPzEVector sec4roe = T.cmsToLab(secRoeCMS);

        // update sec3v direction:
        sec3v.SetXYZ(sec4roe.X(), sec4roe.Y(), sec4roe.Z());
      }
      // std::cout << " VV2roe: " << sec3v.X() << " " << sec3v.Y() << " "<<sec3v.Z();


      B2Vector3D cros = tag3v.Unit().Cross(sec3v.Unit());
      double  dot = tag3v.Unit().Dot(sec3v.Unit());

      // Rotation to make secondary B point as tag B
      rot.Rotate(-acos(dot), cros);

      // Test that rotation does what expected:

      B2Vector3D test = rot * sec3v;

      // std::cout << " VV2rot: " << test.X() << " " << test.Y() << " "<<test.Z() << "\n";


      double smallValue = 1e-12;
      if ((abs(sin(test.Phi() - tag3v.Phi())) > smallValue) or (abs(test.Theta() - tag3v.Theta()) > smallValue)) {
        B2ERROR("Loss of accuracy during rotation" << LogVar("Delta phi", abs(sin(test.Phi() - tag3v.Phi())))
                << LogVar("Delta Theta", abs(test.Theta() - tag3v.Theta())));
      }
    } else {
      B2ERROR("No vertex info, can not update tracks");
    }
    return rot;
  }

  void PostMergeUpdaterModule::event()
  {
    if (m_mixing) {
      // get beam vertex for P2:
      if (m_eventExtraInfo->hasExtraInfo("IPX")
          and m_eventExtraInfo->hasExtraInfo("IPY")
          and m_eventExtraInfo->hasExtraInfo("IPZ")) {
        double xv2 = m_eventExtraInfo->getExtraInfo("IPX");
        double yv2 = m_eventExtraInfo->getExtraInfo("IPY");
        double zv2 = m_eventExtraInfo->getExtraInfo("IPZ");
        const B2Vector3D origSpot(xv2, yv2, zv2);
        // Now vertex from DB:
        static DBObjPtr<Belle2::BeamSpot> beamSpotDB;
        const B2Vector3D beamSpot = beamSpotDB->getIPPosition();

        /*
        std::cout << " VV1: " << origSpot.X() << " " << origSpot.Y() << " "<<origSpot.Z() << "\n";
        std::cout << " VV2: " << beamSpot.X() << " " << beamSpot.Y() << " "<<beamSpot.Z() << "\n";
        */

        const TRotation rot = tag_vertex_rotation();
        const double bz = BFieldManager::getField(beamSpot).Z() / Unit::T;

        // Loop over track fit results from the attached part of the event:
        // Loop over new tracks and corresponding track fit results
        for (int idxTr =  m_mergedArrayIndices->getExtraInfo("Tracks"); idxTr < m_tracks.getEntries(); idxTr++) {
          for (short int i : m_tracks[idxTr]->getValidIndices()) {
            auto t_idx = m_tracks[idxTr]->m_trackFitIndices[i];

            short charge = m_trackFits[t_idx]->getChargeSign();
            auto  helixO = m_trackFits[t_idx]->getHelix();
            // Move track such that IP becomes centered at the beamSpot
            helixO.passiveMoveBy(origSpot - beamSpot);

            // Also Rotate:
            B2Vector3D position = helixO.getPerigee();
            B2Vector3D momentum = rot * helixO.getMomentum(bz);

            // New helix
            Helix helix(position, momentum, charge, bz);

            /*
            std::cout << " Orig " <<  m_trackFits[t_idx]->m_tau[TrackFitResult::iD0] <<
              "," <<  m_trackFits[t_idx]->m_tau[TrackFitResult::iPhi0] <<
              "," <<  m_trackFits[t_idx]->m_tau[TrackFitResult::iOmega] <<
              "," <<  m_trackFits[t_idx]->m_tau[TrackFitResult::iZ0] <<
              "," <<  m_trackFits[t_idx]->m_tau[TrackFitResult::iTanLambda] << " " << helixO.getMomentum(bz).Mag()
                << "\n";
            */
            // Store back in the mdst:
            m_trackFits[t_idx]->m_tau[TrackFitResult::iD0]        = helix.getD0();
            m_trackFits[t_idx]->m_tau[TrackFitResult::iPhi0]      = helix.getPhi0();
            m_trackFits[t_idx]->m_tau[TrackFitResult::iOmega]     = helix.getOmega();
            m_trackFits[t_idx]->m_tau[TrackFitResult::iZ0]        = helix.getZ0();
            m_trackFits[t_idx]->m_tau[TrackFitResult::iTanLambda] = helix.getTanLambda();

            /*
            std::cout << " Upd " <<  m_trackFits[t_idx]->m_tau[TrackFitResult::iD0] <<
              "," <<  m_trackFits[t_idx]->m_tau[TrackFitResult::iPhi0] <<
              "," <<  m_trackFits[t_idx]->m_tau[TrackFitResult::iOmega] <<
              "," <<  m_trackFits[t_idx]->m_tau[TrackFitResult::iZ0] <<
              "," <<  m_trackFits[t_idx]->m_tau[TrackFitResult::iTanLambda] << " " << helix.getMomentum(bz).Mag()
                << "\n";
            */
          }
        }
        // also ECL clusters:
        cluster_rotation(rot);

      } else {
        B2ERROR("No vertex info, can not update tracks");
      }

    } else {
      if (
        m_eventExtraInfo_orig->hasExtraInfo("X")
        and m_eventExtraInfo_orig->hasExtraInfo("Y")
        and m_eventExtraInfo_orig->hasExtraInfo("Z")
        and m_eventExtraInfo->hasExtraInfo("X")
        and m_eventExtraInfo->hasExtraInfo("Y")
        and m_eventExtraInfo->hasExtraInfo("Z")
      ) {

        B2Vector3D vertexTag(m_eventExtraInfo_orig->getExtraInfo("X"),
                             m_eventExtraInfo_orig->getExtraInfo("Y"),
                             m_eventExtraInfo_orig->getExtraInfo("Z"));

        B2Vector3D vertexEmb(m_eventExtraInfo->getExtraInfo("X"),
                             m_eventExtraInfo->getExtraInfo("Y"),
                             m_eventExtraInfo->getExtraInfo("Z"));


        const double bz = BFieldManager::getField(vertexTag).Z() / Unit::T;
        const TRotation rot = tag_vertex_rotation();

        // Loop over new tracks and corresponding track fit results
        for (int idxTr =  m_mergedArrayIndices->getExtraInfo("Tracks"); idxTr < m_tracks.getEntries(); idxTr++) {
          for (short int i : m_tracks[idxTr]->getValidIndices()) {
            auto t_idx = m_tracks[idxTr]->m_trackFitIndices[i];

            // get d0, z0 vs original vertex, add them back, to account for smearing:
            auto helix = m_trackFits[t_idx]->getHelix();
            helix.passiveMoveBy(vertexEmb);

            B2Vector3D mom    = m_trackFits[t_idx]->getMomentum();
            short    charge = m_trackFits[t_idx]->getChargeSign();

            // Rotate (no boost for now)
            B2Vector3D momTag = rot * mom;
            // New helix
            Helix h(vertexTag, momTag, charge, bz);

            m_trackFits[t_idx]->m_tau[TrackFitResult::iD0]          = h.getD0() + helix.getD0(); // some smear
            m_trackFits[t_idx]->m_tau[TrackFitResult::iPhi0]        = h.getPhi0();
            m_trackFits[t_idx]->m_tau[TrackFitResult::iZ0]          = h.getZ0() + helix.getZ0();
            m_trackFits[t_idx]->m_tau[TrackFitResult::iTanLambda]   = h.getTanLambda();
            m_trackFits[t_idx]->m_tau[TrackFitResult::iOmega]       = h.getOmega();

          }
        }
        cluster_rotation(rot);
      }

      else {
        B2ERROR("No track parameters info, can not update tracks");
      }
    }

  }


  void PostMergeUpdaterModule::cluster_rotation(const TRotation& rot)
  {
    // Loop over added clusters:
    for (int idxCl =  m_mergedArrayIndices->getExtraInfo("ECLClusters"); idxCl < m_eclclusters.getEntries(); idxCl++) {
      B2Vector3D pos =   m_eclclusters[idxCl]->getClusterPosition();
      B2Vector3D newPos = rot * pos;
      // Keep same R, update theta/phi:
      m_eclclusters[idxCl]->setTheta(newPos.Theta());
      m_eclclusters[idxCl]->setPhi(newPos.Phi());
    }
  }
}
