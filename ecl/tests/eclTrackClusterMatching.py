import basf2 as b2

from ROOT import Belle2

import simulation
import reconstruction

b2.set_random_seed(42)

main = b2.create_path()


class CheckTrackClusterMatching(b2.Module):
    """
    (DRAFT) Module which checks the track-cluster matching
    """

    def event(self):

        tracks = Belle2.PyStoreArray('Tracks')

        for track in tracks:

            # check that tracks are related to only one cluster
            n_relations = track.getRelationsTo('ECLClusters').size()
            assert n_relations == 1, f"Given track has {n_relations} relations to ECLClusters"

            # check that cluster has track flag set
            cluster = track.getRelated("ECLClusters")
            assert cluster.isTrack(), "Cluster does not have track set despite relation existing"

            # TODO: add additional tests for the correct MC-matching method?

            # TODO: add checks to see if track-cluster match is correct?


main.add_module('EventInfoSetter', evtNumList=[10])

# preliminary test - high pt and in barrel region
main.add_module('ParticleGun',
                pdgCodes=[11],
                nTracks=1,
                momentumGeneration='fixed',
                momentumParams=1,
                thetaGeneration='uniform',
                thetaParams=[35.0, 125.0])

simulation.add_simulation(main)

reconstruction.add_reconstruction(main)

main.add_module(CheckTrackClusterMatching())

b2.process(main)
