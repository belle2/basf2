/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <analysis/modules/PostMergeUpdate/PostMergeUpdaterModule.h>
#include <mdst/dbobjects/BeamSpot.h>
#include <framework/logging/Logger.h>
#include <analysis/utility/RotationTools.h>
#include <analysis/utility/PCmsLabTransform.h>
#include <TDatabasePDG.h>
#include <Math/Vector3D.h>

using namespace Belle2;

//-----------------------------------------------------------------
//                 Register module
//-----------------------------------------------------------------
REG_MODULE(PostMergeUpdater);

// Implementation
PostMergeUpdaterModule::PostMergeUpdaterModule() : Module()
{
  setDescription("Synchronize parts of the events post merge/embedding. Used in the signal embedding pipeline. Uses kinematic information for the tag / simulated decay stored in eventExtraInfo.");
  addParam("Mixing", m_mixing, "Mixing (true) or embedding (false) corrections", false);
  addParam("isCharged", m_isCharged, "Charged (true) or neutral (false) B mesons", true);
}

void PostMergeUpdaterModule::initialize()
{
  m_mergedArrayIndices.isRequired("MergedArrayIndices");
  m_eventExtraInfo.isRequired("EventExtraInfo_indepPath"); // indepPath suffix is hardwired for the second part of the event
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
    // For embedding, we will rotate the simulated B in the direction of the reconstructed tag
    B2Vector3D tag3v = B2Vector3D(m_eventExtraInfo_orig->getExtraInfo("PX"),
                                  m_eventExtraInfo_orig->getExtraInfo("PY"),
                                  m_eventExtraInfo_orig->getExtraInfo("PZ")
                                 );

    B2Vector3D sec3v = B2Vector3D(m_eventExtraInfo->getExtraInfo("PX"),
                                  m_eventExtraInfo->getExtraInfo("PY"),
                                  m_eventExtraInfo->getExtraInfo("PZ")
                                 );

    // For mixing, we want to get an opposite direction, the secondary ROE should point in the direction of the primary TAG
    if (m_mixing) {
      PCmsLabTransform T;

      const int iPDG =  m_isCharged ? 521 : 511 ;
      const double mB = TDatabasePDG::Instance()->GetParticle(iPDG)->Mass();
      double E = sqrt(mB * mB + sec3v.Mag2());
      ROOT::Math::PxPyPzEVector sec4v(sec3v.X(), sec3v.Y(), sec3v.Z(), E);
      ROOT::Math::PxPyPzEVector secCMS = T.labToCms(sec4v);

      // reflection:
      ROOT::Math::PxPyPzEVector secRoeCMS(-secCMS.X(), -secCMS.Y(), -secCMS.Z(), secCMS.E());
      ROOT::Math::PxPyPzEVector sec4roe = T.cmsToLab(secRoeCMS);

      // update sec3v direction:
      sec3v.SetXYZ(sec4roe.X(), sec4roe.Y(), sec4roe.Z());
    }


    B2Vector3D cros = tag3v.Unit().Cross(sec3v.Unit());
    double  dot = tag3v.Unit().Dot(sec3v.Unit());

    // Rotation to make secondary B point as tag B
    rot.Rotate(-acos(dot), cros);

    // Closure test that rotation does what expected:
    B2Vector3D test = rot * sec3v;
    double smallValue = 1e-12;
    if ((abs(sin(test.Phi() - tag3v.Phi())) > smallValue) or (abs(test.Theta() - tag3v.Theta()) > smallValue)) {
      B2ERROR("Loss of accuracy during rotation" << LogVar("Delta phi", abs(sin(test.Phi() - tag3v.Phi())))
              << LogVar("Delta Theta", abs(test.Theta() - tag3v.Theta())));
    }
  } else {
    B2ERROR("No momentum information provided for the tag/simulated particle list, can not update tracks");
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

      const TRotation rot = tag_vertex_rotation();
      const double bz = BFieldManager::getFieldInTesla(beamSpot).Z();

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
          B2Vector3D momentum = rot * B2Vector3D(helixO.getMomentum(bz));

          // New helix
          //            Helix helix(position, momentum, charge, bz);
          Helix helix(ROOT::Math::XYZVector(position.X(), position.Y(), position.Z()),
                      ROOT::Math::XYZVector(momentum.X(), momentum.Y(), momentum.Z()), charge, bz);

          // Store back in the mdst:
          m_trackFits[t_idx]->m_tau[TrackFitResult::iD0]        = helix.getD0();
          m_trackFits[t_idx]->m_tau[TrackFitResult::iPhi0]      = helix.getPhi0();
          m_trackFits[t_idx]->m_tau[TrackFitResult::iOmega]     = helix.getOmega();
          m_trackFits[t_idx]->m_tau[TrackFitResult::iZ0]        = helix.getZ0();
          m_trackFits[t_idx]->m_tau[TrackFitResult::iTanLambda] = helix.getTanLambda();

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

      const double bz = BFieldManager::getFieldInTesla(vertexTag).Z();
      const TRotation rot = tag_vertex_rotation();

      // Loop over new tracks and corresponding track fit results
      for (int idxTr =  m_mergedArrayIndices->getExtraInfo("Tracks"); idxTr < m_tracks.getEntries(); idxTr++) {
        for (short int i : m_tracks[idxTr]->getValidIndices()) {
          auto t_idx = m_tracks[idxTr]->m_trackFitIndices[i];

          // get d0, z0 vs original simulated-truth vertex. They include smearing due to reconstruction.
          auto helix = m_trackFits[t_idx]->getHelix();
          helix.passiveMoveBy(vertexEmb);

          B2Vector3D mom    = m_trackFits[t_idx]->getMomentum();
          short    charge = m_trackFits[t_idx]->getChargeSign();

          // Rotate (no boost for now)
          B2Vector3D momTag = rot * mom;

          // Define new helix "h", based on tag vertex position and rotated track momentum. Note that the vertex position is usually well
          // reconstructed and smearing in the track position parameters (d0,z0) for the helix h is not sufficient. In addition,
          // for embedding channels with multiple tracks we need an extra smearing for each track individually. To take into account correlations,
          // at least to first order, we used determined above (d0,z0) parameters from the track vs original simulated vertex.

          Helix h(vertexTag, momTag, charge, bz);

          m_trackFits[t_idx]->m_tau[TrackFitResult::iD0]          = h.getD0() + helix.getD0(); // include smearing simulated -> reconstructed
          m_trackFits[t_idx]->m_tau[TrackFitResult::iPhi0]        = h.getPhi0();
          m_trackFits[t_idx]->m_tau[TrackFitResult::iZ0]          = h.getZ0() + helix.getZ0(); // include smearing simulated -> reconstructed
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
