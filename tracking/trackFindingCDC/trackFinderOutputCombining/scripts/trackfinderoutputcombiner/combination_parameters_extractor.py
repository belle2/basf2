import basf2
import ROOT
from ROOT import Belle2

from ROOT import gSystem
gSystem.Load('libtracking')
gSystem.Load('libtracking_trackFindingCDC')

from trackfinderoutputcombiner.combinerValidation import CombinerTrackFinderRun
from trackfinderoutputcombiner.combination_checker import CombinationChecker
from tracking.validation.harvesting import HarvestingModule
from tracking.validation import refiners

try:
    import root_pandas
except ImportError:
    print "do a pip install git+https://github.com/ibab/root_pandas"
import pandas
import matplotlib.pyplot as plt
import seaborn as sb
import numpy as np


class CombinationParameterExtractorModule(HarvestingModule):

    def __init__(self, legendre_track_cands_store_array_name, local_track_cands_store_array_name, output_file_name):
        super(
            CombinationParameterExtractorModule,
            self).__init__(
            foreach=legendre_track_cands_store_array_name,
            output_file_name=output_file_name)
        self.foreach_other = local_track_cands_store_array_name

        # this is a total hack!
        self.crop_counter = 0

        # Fitters
        self.zFitter = Belle2.TrackFindingCDC.CDCSZFitter.getFitter()
        self.circleFitter = Belle2.TrackFindingCDC.CDCRiemannFitter.getFitter()

    # We need a doubleHarvester here
    def gather(self):
        registered_store_arrays = Belle2.PyStoreArray.list()
        registered_store_objs = Belle2.PyStoreObj.list()

        foreach = self.foreach
        foreach_other = self.foreach_other
        foreach_is_store_array = foreach in registered_store_arrays
        foreach_other_is_store_array = foreach_other in registered_store_arrays

        if foreach is not None and foreach_other is not None:
            if foreach_is_store_array and foreach_other_is_store_array:
                store_array = Belle2.PyStoreArray(self.foreach)
                store_array_other = Belle2.PyStoreArray(self.foreach_other)
                for crop in store_array:
                    for crop_other in store_array_other:
                        yield (crop, crop_other)
            else:
                raise KeyError(
                    "Name %s or %s does not refer to a valid store array on the data store" %
                    (self.foreach, self.foreach_other))
        else:
            yield None

    def prepare(self):
        self.combination_checker = CombinationChecker(track_cands_1_store_array_name=self.foreach,
                                                      track_cands_2_store_array_name=self.foreach_other,
                                                      mc_track_cands_store_array_name="MCTrackCands")

        store_wrapper = Belle2.PyStoreObj("RecoSegments")
        self.reco_segments = list(self.iter_store_obj(store_wrapper))

        self.crop_counter = 0

        self.cdcHits = Belle2.PyStoreArray("CDCHits")

        return HarvestingModule.prepare(self)

    def pick(self, track_cands_tupel):
        self.crop_counter += 1
        (legendre_track_cand, local_track_cand) = track_cands_tupel
        return self.combination_checker.is_matchable(legendre_track_cand, local_track_cand)

    def peel(self, track_cands_tupel):
        (legendre_track_cand, local_track_cand) = track_cands_tupel
        belong_together = self.combination_checker.belong_to_same_mc_track(legendre_track_cand, local_track_cand)

        # Minus 1 because pick increases the index before executing peel
        reco_segment_index = (self.crop_counter - 1) % len(self.reco_segments)
        reco_segment = self.reco_segments[reco_segment_index]

        # Calculate the different parameters for the segment
        trajectory = Belle2.TrackFindingCDC.CDCTrajectory3D(Belle2.TrackFindingCDC.Vector3D(legendre_track_cand.getPosSeed()),
                                                            Belle2.TrackFindingCDC.Vector3D(legendre_track_cand.getMomSeed()),
                                                            legendre_track_cand.getChargeSeed())

        # Calculate minimum and maximum angle:
        minimumAngle = np.NaN
        maximumAngle = np.NaN

        for cdcHitID in legendre_track_cand.getHitIDs(Belle2.Const.CDC):
            cdcHit = self.cdcHits[cdcHitID]
            cdcWireHit = Belle2.TrackFindingCDC.CDCWireHit(cdcHit)
            currentAngle = cdcWireHit.getPerpS(trajectory.getTrajectory2D())
            if currentAngle > 0:
                if minimumAngle is np.NaN or currentAngle < minimumAngle:
                    minimumAngle = currentAngle
                if maximumAngle is np.NaN or currentAngle > maximumAngle:
                    maximumAngle = currentAngle

        prob_for_good_chi2_segment = np.NaN
        prob_for_good_chi2_track = np.NaN
        z_dist_segment = np.NaN
        z_angle_segment = np.NaN
        z_angle_track = np.NaN
        z_dist_track = np.NaN
        stereo = False
        fit_failed = False

        # For the reconstructed segment there are only two possible cases:
        # (1) the segment is full axial. We fit the hits of the segment together with the axial hits of the track.
        # (2) the segment is full stereo. We pull the hits of the segment to the track trajectory and fit the z direction
        if reco_segment.getStereoType() == 0:  # Axial
            observationsCircle = Belle2.TrackFindingCDC.CDCObservations2D()

            stereo = False

            perps_list = list()

            # Add the hits from the segment to the circle fit
            for recoHit2D in reco_segment.items():
                currentAngle = recoHit2D.getPerpS(trajectory.getTrajectory2D())
                perps_list.append(currentAngle)
                # if currentAngle < 1.5 * maximumAngle and currentAngle > 0.5 * minimumAngle:
                observationsCircle.append(recoHit2D)

            # Add the hits from the TrackCand to the circle fit
            for cdcHitID in legendre_track_cand.getHitIDs(Belle2.Const.CDC):
                cdcHit = self.cdcHits[cdcHitID]
                cdcWireHit = Belle2.TrackFindingCDC.CDCWireHit(cdcHit)
                if cdcWireHit.getStereoType() == 0:  # Axial
                    observationsCircle.append(cdcWireHit)

            # Fit the circle trajectory
            trajectory2D = Belle2.TrackFindingCDC.CDCTrajectory2D()
            self.circleFitter.update(trajectory2D, observationsCircle)
            prob_for_good_chi2_segment = ROOT.TMath.Prob(trajectory2D.getChi2(), trajectory2D.getNDF())

        elif reco_segment.getStereoType() == 1 or reco_segment.getStereoType() == -1:  # Stereo
            observationsSZ = Belle2.TrackFindingCDC.CDCObservations2D()

            stereo = True

            perps_list = list()

            # Add the hits from the segment to the sz fit
            for recoHit2D in reco_segment.items():
                recoHit3D = Belle2.TrackFindingCDC.CDCRecoHit3D.reconstruct(recoHit2D, trajectory.getTrajectory2D())
                currentAngle = recoHit3D.getPerpS(trajectory.getTrajectory2D())
                perps_list.append(currentAngle)
                # if recoHit3D.isInCDC() and currentAngle < 1.5 * maximumAngle and currentAngle > 0.5 * minimumAngle:
                observationsSZ.append(recoHit3D.getPerpS(), recoHit3D.getRecoPos3D().z())

            if observationsSZ.size() > 3:
                # Fit the sz trajectory
                trajectorySZ = Belle2.TrackFindingCDC.CDCTrajectorySZ()
                self.zFitter.update(trajectorySZ, observationsSZ)
                z_angle_segment = ROOT.TMath.ATan(trajectorySZ.getSZSlope())
                z_dist_segment = trajectorySZ.getStartZ()
                prob_for_good_chi2_segment = ROOT.TMath.Prob(trajectorySZ.getChi2(), trajectorySZ.getNDF())
            else:
                fit_failed = True

        # Calculate the different parameters for the track
        observationsSZ = Belle2.TrackFindingCDC.CDCObservations2D()

        for cdcHitID in legendre_track_cand.getHitIDs(Belle2.Const.CDC):
            cdcHit = self.cdcHits[cdcHitID]

            if cdcHit.getISuperLayer() % 2 != 0:
                # we do not know the right-left information
                cdcWireHit = Belle2.TrackFindingCDC.CDCWireHit(cdcHit)
                cdcRLWireHit = Belle2.TrackFindingCDC.CDCRLWireHit(cdcWireHit)
                recoHit3D = Belle2.TrackFindingCDC.CDCRecoHit3D.reconstruct(cdcRLWireHit, trajectory.getTrajectory2D())
                if recoHit3D.isInCDC():
                    observationsSZ.append(recoHit3D.getPerpS(), recoHit3D.getRecoPos3D().z())

        if observationsSZ.size() > 3:
            trajectorySZ = Belle2.TrackFindingCDC.CDCTrajectorySZ()
            self.zFitter.update(trajectorySZ, observationsSZ)
            z_angle_track = ROOT.TMath.ATan(trajectorySZ.getSZSlope())
            prob_for_good_chi2_track = ROOT.TMath.Prob(trajectorySZ.getChi2(), trajectorySZ.getNDF())
            z_dist_track = trajectorySZ.getStartZ()

        return dict(belong_together=belong_together,
                    prob_for_good_chi2_segment=prob_for_good_chi2_segment,
                    z_angle_segment=z_angle_segment,
                    z_dist_segment=z_dist_segment,
                    stereo=stereo,
                    minimumAngle=minimumAngle,
                    maximumAngle=maximumAngle,
                    z_angle_track=z_angle_track,
                    diff_z_angle=z_angle_track - z_angle_segment,
                    prob_for_good_chi2_track=prob_for_good_chi2_track,
                    fit_failed=fit_failed)

    save_tree = refiners.save_tree(folder_name="tree")


class CombinationParameterExtractorRun(CombinerTrackFinderRun):

    use_segment_quality_check = False

    def create_path(self):
        main_path = super(CombinationParameterExtractorRun, self).create_path()

        mc_track_matcher_module_legendre = basf2.register_module('MCTrackMatcher')
        mc_track_matcher_module_legendre.param({
            'UseCDCHits': True,
            'UseSVDHits': False,
            'UsePXDHits': False,
            'RelateClonesToMCParticles': True,
            'MCGFTrackCandsColName': "MCTrackCands",
            'PRGFTrackCandsColName': self.legendre_track_cands_store_array_name
        })

        mc_track_matcher_module_local = basf2.register_module('MCTrackMatcher')
        mc_track_matcher_module_local.param({
            'UseCDCHits': True,
            'UseSVDHits': False,
            'UsePXDHits': False,
            'RelateClonesToMCParticles': True,
            'MCGFTrackCandsColName': "MCTrackCands",
            'PRGFTrackCandsColName': self.local_track_cands_store_array_name
        })

        main_path.add_module(mc_track_matcher_module_legendre)
        main_path.add_module(mc_track_matcher_module_local)

        main_path.add_module(
            CombinationParameterExtractorModule(
                legendre_track_cands_store_array_name=self.legendre_track_cands_store_array_name,
                local_track_cands_store_array_name=self.local_track_cands_store_array_name,
                output_file_name="combination_parameters_extractor.root"))

        return main_path


def main():
    run = CombinationParameterExtractorRun()
    run.configure_and_execute_from_commandline()
    print basf2.statistics


def plot():
    data = root_pandas.read_root("combination_parameters_extractor.root", tree_key="tree/CombinationParameterExtractorModule_tree")

    data = data.dropna()

    belong_together = data.loc[data.belong_together == 1]
    not_belong_together = data.loc[data.belong_together == 0]

    for col in data.columns:

        if col != "belong_together":
            normed = True
        else:
            normed = False

        plt.clf()
        plt.subplot(211)
        n, bins, p = plt.hist(
            [
                belong_together.loc[
                    belong_together.stereo == 1, col], not_belong_together.loc[
                    not_belong_together.stereo == 1, col]], histtype="bar", normed=normed, label=[
                "Signal", "Background"])
        plt.xlabel("Stereo")
        if col == "belong_together":
            print n
        plt.legend()

        if belong_together.loc[
                belong_together.stereo == 0,
                col].count() > 0 or not_belong_together.loc[
                not_belong_together.stereo == 0,
                col].count() > 0:
            plt.subplot(212)
            n, bins, p = plt.hist(
                [
                    belong_together.loc[
                        belong_together.stereo == 0, col], not_belong_together.loc[
                        not_belong_together.stereo == 0, col]], histtype="bar", normed=normed, label=[
                    "Signal", "Background"])
            plt.xlabel("Axial")
            if col == "belong_together":
                print n
            plt.legend()

        plt.savefig(str(col) + ".pdf")

if __name__ == "__main__":
    main()
    plot()
