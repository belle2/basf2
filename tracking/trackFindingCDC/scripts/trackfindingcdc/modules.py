import basf2

import ROOT

import logging

from ROOT import Belle2


class CDCHitUniqueAssumer(basf2.Module):

    """
    A small helper module to look for double assigned hits.
    Prints a summary after execution
    """

    def initialize(self):
        self.number_of_doubled_hits = 0
        self.number_of_total_hits = 0
        self.number_of_hits_with_wrong_flags = 0

    def event(self):
        track_store_vector = Belle2.PyStoreObj('CDCTrackVector')

        if track_store_vector:
            # Wrapper around std::vector like
            wrapped_vector = track_store_vector.obj()
            tracks = wrapped_vector.get()

            for track in tracks:
                # Unset all taken flags
                for recoHit in list(track.items()):
                    if not recoHit.getWireHit().getAutomatonCell().hasTakenFlag():
                        self.number_of_hits_with_wrong_flags += 1

            for track in tracks:
                # Now check that we only have every wire hit once
                for recoHit in list(track.items()):
                    self.number_of_total_hits += 1
                    if recoHit.getWireHit().getAutomatonCell().hasAssignedFlag():
                        self.number_of_doubled_hits += 1
                    recoHit.getWireHit().getAutomatonCell().setAssignedFlag()

                for innerTrack in tracks:
                    for recoHit in list(innerTrack.items()):
                        recoHit.getWireHit().getAutomatonCell().unsetAssignedFlag()

    def terminate(self):
        print("Number of doubled hits:", self.number_of_doubled_hits)
        print("Number of hits with wrong taken flag:", self.number_of_hits_with_wrong_flags)
        print("Number of total hits:", self.number_of_total_hits)


class HitCleaner(basf2.Module):
    #: A small hit cleaner module to set the track information according to mc information.
    #: This is surely not for later usage but for testing the genfitter module

    def __init__(self):
        super(HitCleaner, self).__init__()

        self.number_of_tracks = 0
        self.number_of_deleted_hits = 0
        self.number_of_hits = 0

    def initialize(self):
        self.cdc_hit_look_up = Belle2.TrackFindingCDC.CDCMCHitLookUp()
        self.mc_matcher_lookup = Belle2.TrackMatchLookUp("MCTrackCands", "TrackCands")

    def event(self):
        tracks = Belle2.PyStoreArray("TrackCands")
        cdc_hits = Belle2.PyStoreArray("CDCHits")
        mc_particles = Belle2.PyStoreArray("MCParticles")

        cdc_hit_lookup = self.cdc_hit_look_up
        cdc_hit_lookup.fill()

        mc_matcher_lookup = self.mc_matcher_lookup

        self.number_of_tracks += tracks.getEntries()

        for track in tracks:
            # Store all Hit IDs and reset the track
            hitIDs = track.getHitIDs(Belle2.Const.CDC)
            hits = [cdc_hits[i] for i in hitIDs]
            good_hits = []

            relation_track_particle = [0] * mc_particles.getEntries()

            # Now only add those hits which do belong to the track (with MC Info)
            for i in hitIDs:
                current_mc_track = cdc_hit_lookup.getMCTrackId(cdc_hits[i])
                if 0 <= current_mc_track:
                    relation_track_particle[current_mc_track] += 1

            should_belong_to_track = np.argmax(relation_track_particle)

            deleted_hits = sum(relation_track_particle) - relation_track_particle[should_belong_to_track]

            plane_IDs_of_good_hits = []
            for i, hitID in enumerate(hitIDs):
                current_mc_track = cdc_hit_lookup.getMCTrackId(cdc_hits[hitID])
                if current_mc_track == should_belong_to_track:
                    good_hits.append(hitID)

            self.number_of_deleted_hits += deleted_hits
            self.number_of_hits += len(hitIDs)

            # Set the position and momentum
            mc_track = mc_matcher_lookup.getMatchedMCTrackCand(track)

            if mc_track:
                mc_trajectory = Belle2.TrackFindingCDC.CDCTrajectory3D(Belle2.TrackFindingCDC.Vector3D(mc_track.getPosSeed()),
                                                                       Belle2.TrackFindingCDC.Vector3D(mc_track.getMomSeed()),
                                                                       mc_track.getChargeSeed())
                startingPosition = Belle2.TrackFindingCDC.Vector3D(track.getPosSeed().X(), track.getPosSeed().Y(), 0)
                sStartingPosition = mc_trajectory.calcArcLength2D(startingPosition)
                zStartingPosition = mc_trajectory.getTrajectorySZ().mapSToZ(sStartingPosition)
                mc_trajectory.setLocalOrigin(Belle2.TrackFindingCDC.Vector3D(startingPosition.xy(), zStartingPosition))

                pos = ROOT.TVector3(mc_trajectory.getSupport().x(), mc_trajectory.getSupport().y(), mc_trajectory.getSupport().z())
                mom = ROOT.TVector3(
                    mc_trajectory.getMom3DAtSupport().x(),
                    mc_trajectory.getMom3DAtSupport().y(),
                    mc_trajectory.getMom3DAtSupport().z())

                # track.setPosMomSeedAndPdgCode(pos, mom , int(mc_track.getChargeSeed() * 211))
                track.setPdgCode(int(track.getChargeSeed() * 211))

            else:
                # track.reset()
                track.setPdgCode(int(track.getChargeSeed() * 211))

    def terminate(self):
        print(("Number of tracks in total: %d" % self.number_of_tracks))
        print(("Number of hits in total: %d" % self.number_of_hits))
        print(("Number of deleted hits: %d" % self.number_of_deleted_hits))

        print(("Number of deleted hits per track: %f" % (1.0 * self.number_of_deleted_hits / self.number_of_tracks)))
        print(("Ratio of deleted hits: %f" % (100.0 * self.number_of_deleted_hits / self.number_of_hits)))
