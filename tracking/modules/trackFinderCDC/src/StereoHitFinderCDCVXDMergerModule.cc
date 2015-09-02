#include <tracking/modules/trackFinderCDC/StereoHitFinderCDCVXDMergerModule.h>
#include <genfit/Track.h>
#include <exception>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

//ROOT macro
REG_MODULE(StereoHitFinderCDCVXDMerger)

bool extrapolateStateWithoutError(genfit::MeasuredStateOnPlane& measuredState, double cylindricalR)
{
  try {
    measuredState.extrapolateToCylinder(cylindricalR);
    return true;
  } catch (...) {
    B2WARNING("VXDTrack extrapolation to cylinder failed!");
    return false;
  }
}

void StereoHitFinderCDCVXDMergerModule::generate(std::vector<Belle2::TrackFindingCDC::CDCTrack>& tracks)
{
  StoreArray<genfit::Track> vxdGFTracks(m_param_VXDGenfitTracksStoreArrayName);

  double innermostCylindricalR = 200;

  for (CDCTrack& track : tracks) {
    Vector2D innermostHitPosition = track.front().getRecoPos2D();
    // we set z = 0 to ensure we do not bias the finding.
    // TODO: Later we can do a first loop with the current approach and a second loop later without!

    if (innermostHitPosition.norm() < innermostCylindricalR) {
      innermostCylindricalR = innermostHitPosition.norm();
    }
  }

  // Calculate the extrapolated states of the vxd tracks on the innermost hit position
  std::map<const genfit::Track*, genfit::MeasuredStateOnPlane> mapVXDTrackToExtrapolatedStates;

  for (const genfit::Track& vxdTrack : vxdGFTracks) {
    std::pair<std::map<const genfit::Track*, genfit::MeasuredStateOnPlane>::iterator, bool> inserted =
      mapVXDTrackToExtrapolatedStates.insert(std::make_pair(&vxdTrack, vxdTrack.getFittedState()));
    genfit::MeasuredStateOnPlane& measuredState = inserted.first->second;
    extrapolateStateWithoutError(measuredState, innermostCylindricalR);
  }

  for (CDCTrack& track : tracks) {
    Vector2D innermostHitPosition = track.front().getRecoPos2D();
    const genfit::Track* bestTrackMatch = nullptr;
    genfit::MeasuredStateOnPlane bestMeasuredStateOnPlane = nullptr;
    double smallestDistance = m_param_minimalDistanceToMerge;

    for (const std::pair<const genfit::Track*, genfit::MeasuredStateOnPlane>& pair : mapVXDTrackToExtrapolatedStates) {
      // COPY IS INTENDED!
      genfit::MeasuredStateOnPlane measuredState = pair.second;
      if (extrapolateStateWithoutError(measuredState, innermostHitPosition.norm())) {
        Vector2D extrapolatedPosition{measuredState.getPos().XYvector()};
        double distance = (extrapolatedPosition - innermostHitPosition).norm();

        if (distance < smallestDistance) {
          bestTrackMatch = pair.first;
          bestMeasuredStateOnPlane = measuredState;
          smallestDistance = distance;
        }
      }
    }

    if (bestTrackMatch) {
      Vector3D vxdMom{bestMeasuredStateOnPlane.getMom()};
      Vector3D vxdPos{bestMeasuredStateOnPlane.getPos()};
      double angleBetweenTracks = std::abs(vxdMom.xy().angleWith(track.getStartFitMom3D().xy()));
      //B2INFO(angleBetweenTracks);

      if (angleBetweenTracks < m_param_maximumAllowedDeviationAngle) {
        CDCTrajectory2D trajectory2D = track.getStartTrajectory3D().getTrajectory2D();

        double slopeSZ = vxdMom.z() / trajectory2D.calcArcLength2DBetween(vxdPos.xy(), vxdPos.xy() + vxdMom.xy());
        CDCTrajectorySZ trajectorySZ(slopeSZ, vxdPos.z());

        CDCTrajectory3D trajectory3D(trajectory2D, trajectorySZ);
        track.setStartTrajectory3D(trajectory3D);
      }
    }
  }
}

/*

      pos.SetZ(0);
      vxdpos.SetZ(0);

      double merge_radius = 5.;
      double dist = TMath::Sqrt(square(pos - vxdpos)) - driftLength_hit_min;

      // Extrapolate tracks to same plane & Match Tracks
      if (dist < merge_radius) {

        if (min_dist > dist) {
          bestVXDTrackToMerge = VXDGFTracks[iVxdTrack];
          bestVXDTrackToMergeID = iVxdTrack;
          min_dist = dist;
          vxdmomBest = vxdmom;
          vxdposBest = vxdpos;
        }
      }
    }//end loop on VXD tracks

    if (bestVXDTrackToMerge) {
      if (bestVXDTrackToMergeID < 0)continue;
      vxdmomBest.SetZ(0);
      TVector3 candMom = cand->getMomentumEstimation();
      candMom.SetZ(0);
      double momDiff(0);
      momDiff = (vxdmomBest - candMom).Pt();
      B2INFO("momDiff = " << momDiff << "; momDiff/candMom = " << momDiff / candMom.Pt());
      if (momDiff > 1.) continue;


      std::string m_VXDGFTracksCandColName("VXDTracksCand");

      const genfit::TrackCand* vxdTrackCandRel = DataStore::getRelatedToObj<genfit::TrackCand>(bestVXDTrackToMerge,
                                                 m_VXDGFTracksCandColName);

      genfit::MeasuredStateOnPlane vxd_sop = bestVXDTrackToMerge->getFittedState(-1);
      vxd_sop.extrapolateToCylinder(m_CDC_wall_radius, position, momentum);
      vxdposBest = vxd_sop.getPos();
      vxdmomBest = vxd_sop.getMom();
      double thetaTrack = vxdmom.Theta();



      stereohitsProcesser.assignStereohitsByAngle(cand, thetaTrack, m_StereoHitList);


      vxdTrackCandRel = nullptr;
      delete vxdTrackCandRel;

    }

    bestVXDTrackToMerge = nullptr;

  }
}*/


