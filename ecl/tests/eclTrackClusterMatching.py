import basf2 as b2

from ROOT import Belle2

import simulation
import reconstruction

b2.set_random_seed(42)

main = b2.create_path()


class CheckTrackClusterMatching(b2.Module):
    """
    Module which checks relations/variables have been properly set for track-cluster matching
    Note that this does not check the performance of the track-cluster matching
    """

    def event(self):
        """
        Load tracks from StoreArray for each event, and perform tests of module
        """

        tracks = Belle2.PyStoreArray("Tracks")

        for track in tracks:
            cluster = track.getRelatedTo("ECLClusters")
            if (cluster):
                # TEST 1: if cluster is related to track, flag should be set
                assert cluster.isTrack(), "Cluster does not have track set despite relation existing"
                # TEST 2: correct track-cluster matching method used (Enter Crystal or Angular Distance)
                fit_result = track.getTrackFitResultWithClosestMass(211)
                pt = fit_result.getTransverseMomentum()
                theta = fit_result.getMomentum().Theta()
                if (pt < 0.3 and theta > 0.5480334) or (pt > 0.3 and theta > 0.5480334 and theta < 0.561996):
                    assert track.getRelatedTo(
                        "ECLClusters", "EnterCrystal") is not None, "Relation set with EnterCrystal method does not exist"
                else:
                    assert track.getRelatedTo(
                        "ECLClusters", "AngularDistance") is not None, "Relation set with AngularDistance method does not exist"


main.add_module('EventInfoSetter', evtNumList=[1000])

main.add_module('ParticleGun',
                pdgCodes=[11],
                nTracks=5,
                momentumGeneration='uniform',
                momentumParams=[0.1, 1])

# specific case - high pt, towards gap
main.add_module('ParticleGun',
                pdgCodes=[11],
                nTracks=1,
                momentumGeneration='fixed',
                momentumParams=1,
                thetaGeneration='uniform',
                thetaParams=[31.4, 32.2])

simulation.add_simulation(main)

reconstruction.add_reconstruction(main)

main.add_module(CheckTrackClusterMatching())

b2.process(main)
