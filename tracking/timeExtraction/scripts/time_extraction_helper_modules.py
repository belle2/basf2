# This file summarizes some helper functions to analyse the track time extraction. They are used by the
# study script extracted_time_mc_compare.py
import basf2
import numpy as np

from ROOT import Belle2
import ROOT

from tracking.harvest.harvesting import harvest
from tracking.harvest.refiners import save_tree, save_pull_analysis


class RandomizeTrackTimeModule(basf2.Module):
    """
    Main module for the analysis: take the CDCSimHits as well as the MCParticles
    and add the mc_drift value to their internal event time.
    If the mc_drift is -999, set the drift for each event to a random value between
    -50 and 50.

    Please note that this module needs to run between the FullSim and the CDCDigitizer module.
    Also, setting the MCParticles time has no influence on the event at all, nut is just for later
    reference and to compare the reconstructed/extracted time with something.
    """

    def __init__(self, mc_drift):
        """
        Create a new RandomizeTrackTimeModule instance.
        :param mc_drift: The drift in ns to which the CDCSimHits and MCParticles should be shifted.
        """
        self.mc_drift = mc_drift
        basf2.Module.__init__(self)

        self.set_property_flags(basf2.ModulePropFlags.PARALLELPROCESSINGCERTIFIED)

    def event(self):
        """
        Shift the MCParticles (just for reference) and the CDCSimHits.
        :return:
        """
        cdc_sim_hits = Belle2.PyStoreArray("CDCSimHits")
        mc_particles = Belle2.PyStoreArray("MCParticles")

        if self.mc_drift == -999:
            mc_drift = np.random.randint(-50, 50)
        else:
            mc_drift = self.mc_drift

        for cdc_sim_hit in cdc_sim_hits:
            cdc_sim_hit.setFlightTime(cdc_sim_hit.getFlightTime() + float(mc_drift))

        # JUST FOR LATER REFERENCE!
        for mc_particle in mc_particles:
            mc_particle.setProductionTime(float(mc_drift))


class MCRecoTracksResetModule(basf2.Module):
    """
    If we use the TrackFinderMCTruth for testing, it will use the track time stored in the MCParticles,
    which makes the time extraction too simple. Instead, we have to reset the track time to 0 and set the
     track start position to the first hit, which will allow us to use the IPTrackTimeEstimator later.
    """
    def __init__(self):
        basf2.Module.__init__(self)

        self.set_property_flags(basf2.ModulePropFlags.PARALLELPROCESSINGCERTIFIED)

    def event(self):
        """
        Reset the track position of all reco tracks to start at their first hit and set the time seed to 0.
        """
        reco_tracks = Belle2.PyStoreArray("RecoTracks")

        for reco_track in reco_tracks:
            # Set the time seed to 0
            reco_track.setTimeSeed(0)

            # Define the position and momentum at the first CDC hit:
            first_cdc_hit = reco_track.getSortedCDCHitList()[0]
            first_cdc_wire = Belle2.TrackFindingCDC.CDCWire.getInstance(first_cdc_hit)
            first_hit_position = first_cdc_wire.getRefPos2D()

            helix = Belle2.Helix(reco_track.getPositionSeed(), reco_track.getMomentumSeed(),
                                 reco_track.getChargeSeed(), 1.5)

            arc_length_at_first_hit = helix.getArcLength2DAtXY(first_hit_position.x(), first_hit_position.y())
            momentum_at_first_hit = helix.getMomentumAtArcLength2D(arc_length_at_first_hit, 1.5)

            position_t_vector = helix.getPositionAtArcLength2D(arc_length_at_first_hit)
            momentum_t_vector = ROOT.TVector3(momentum_at_first_hit.x(), momentum_at_first_hit.y(),
                                              momentum_at_first_hit.z())
            reco_track.setPositionAndMomentum(position_t_vector, momentum_t_vector)


def add_harvester(path, mc_drift, generator_module, track_finder, turns):
    """
    Add four harvester to the path. The harvester are better described in their corresponding functions.
    Use the parameters to control the name of the root output files.
    :param path: To which path to add the modules.
    :param mc_drift: Which drift was applied to the mc particles.
    :param generator_module: Which generator was used.
    :param track_finder: Which track finding algorithm was used.
    :param turns: How many turns the track extraction had.
    """
    @save_pull_analysis(truth_name="mc_production_time",
                        estimate_name="reco_track_time",
                        part_name="track_time")
    @save_tree(name="tree")
    @harvest(foreach="EventT0", show_results=False,
             output_file_name="harvest_track_time_{mc_drift}_{generator}_{track_finder}_{turns}.root".format(
                 mc_drift=mc_drift, generator=generator_module, track_finder=track_finder, turns=turns))
    def harvest_track_time(event_t0):
        """
        Write out the extracted time from the EventT0 and compare it to the MCParticle time.
        """
        mc_particles = Belle2.PyStoreArray("MCParticles")

        if event_t0 is not None:
            time_seed = event_t0.getEventT0()
        else:
            time_seed = -999

        if mc_particles[0] is not None:
            mc_time = mc_particles[0].getProductionTime()
        else:
            mc_time = -999

        yield dict(mc_production_time=mc_time,
                   reco_track_time=time_seed)

    @save_tree(name="tree")
    @harvest(
        foreach="EventMetaData",
        show_results=False,
        output_file_name="harvest_extracted_times_{mc_drift}_{generator}_{track_finder}_{turns}.root".format(
            mc_drift=mc_drift, generator=generator_module, track_finder=track_finder, turns=turns))
    def harvest_extracted_times(event_meta_data):
        """
        Do one time extraction step and write out the extracted derivatives and some information on the track.
        """
        for reco_track in Belle2.PyStoreArray("__SelectedRecoTracks"):
            pair_of_results = Belle2.TimeExtractionUtils.getChi2Derivatives(reco_track)

            yield {
                "dchi2da": pair_of_results.first,
                "d2chi2da2": pair_of_results.second,
                "pt": reco_track.getMomentumSeed().Pt(),
                "charge": reco_track.getChargeSeed(),
                "number_of_hits": reco_track.getNumberOfCDCHits(),
                "event_number": event_meta_data.getEvent()
            }

    @save_tree(name="tree")
    @harvest(foreach="__SelectedRecoTracks",
             show_results=False,
             output_file_name="harvest_full_chi2_grid_{mc_drift}_{generator}_{track_finder}_{turns}.root".format(
                 mc_drift=mc_drift, generator=generator_module, track_finder=track_finder, turns=turns))
    def harvest_full_chi2_grid(reco_track):
        """
        Do a grid search on the range -30 to 30 and extract the time.

        This means go to 30 points between -30 and 30, set the global event time, do a fit, extract the time
        and calculate the chi2. All this information together with an event number will be stored in the ROOT file.
        """
        track_fitter = Belle2.TrackFitter()

        initial_value = reco_track.getTimeSeed()

        reco_track.setTimeSeed(reco_track.getTimeSeed() - 30)
        reco_track.deleteFittedInformation()

        delta_t0 = 2

        for i in range(30):
            reco_track.setTimeSeed(reco_track.getTimeSeed() + delta_t0)
            reco_track.deleteFittedInformation()

            track_fitter.fit(reco_track)

            extractedDerivatePair = Belle2.TimeExtractionUtils.getChi2Derivatives(reco_track)
            extractedTime = extractedDerivatePair.first / extractedDerivatePair.second
            try:
                chi2 = reco_track.getTrackFitStatus().getChi2() / reco_track.getTrackFitStatus().getNdf()
            except ZeroDivisionError:
                chi2 = 999

            number_of_fitted_hits = np.mean([weight for x in reco_track.getHitPointsWithMeasurement()
                                             for weight in x.getKalmanFitterInfo().getWeights()])
            reco_hit_information = reco_track.getRelationsTo("RecoHitInformations")
            number_of_reco_hit_information = len(reco_hit_information)
            number_of_fitted_reco_hit_information = sum(x.getFlag() != 1 for x in reco_hit_information)

            yield dict(t0_estimate=reco_track.getTimeSeed(),
                       chi2=chi2,
                       extracted_time=extractedTime,
                       number_of_fitted_hits=number_of_fitted_hits,
                       number_of_reco_hit_information=number_of_reco_hit_information,
                       number_of_fitted_reco_hit_information=number_of_fitted_reco_hit_information,
                       )

        reco_track.setTimeSeed(initial_value)

    @save_tree(name="tree")
    @harvest(foreach="EventMetaData",
             show_results=False,
             output_file_name="harvest_chi2_path_{mc_drift}_{generator}_{track_finder}_{turns}.root".format(
                 mc_drift=mc_drift, generator=generator_module, track_finder=track_finder, turns=turns))
    def harvest_chi2_path(event_meta_data):
        """
        Harvesting module build after the FullGridTrackTimeExtractionModule. Does the same as this module,
        but writes out information in every iteration step together with an event number.
        """
        track_fitter = Belle2.TrackFitter()

        t0_min = -30
        t0_max = 30

        def extract_until_finished(reco_track, start_value, event_number, number_of_tries, try_list):
            current_try = 0

            initial_value = reco_track.getTimeSeed()

            reco_track.setTimeSeed(initial_value + start_value)
            reco_track.deleteFittedInformation()

            track_fitter.fit(reco_track)

            if not reco_track.wasFitSuccessful():
                return

            chi2 = Belle2.TimeExtractionUtils.extractReducedChi2(reco_track)
            derivatives_pair = Belle2.TimeExtractionUtils.getChi2Derivatives(reco_track)

            number_of_fitted_hits = len(reco_track.getHitPointsWithMeasurement())

            reco_track_time = reco_track.getTimeSeed()
            try_list.append(dict(chi2=chi2, extracted_time=reco_track_time, event_number=event_number, finished=False,
                                 first_deriv=derivatives_pair.first, number_of_fitted_hits=number_of_fitted_hits,
                                 second_deriv=derivatives_pair.second, delta_t0=np.NaN, delta_chi2=np.NaN))
            current_try += 1

            for i in range(number_of_tries):
                extracted_time = Belle2.TimeExtractionUtils.extractTime(reco_track)
                derivatives_pair = Belle2.TimeExtractionUtils.getChi2Derivatives(reco_track)
                reco_track.setTimeSeed(reco_track.getTimeSeed() + extracted_time)
                reco_track.deleteFittedInformation()

                track_fitter.fit(reco_track)

                if not reco_track.wasFitSuccessful():
                    break

                chi2 = Belle2.TimeExtractionUtils.extractReducedChi2(reco_track)

                reco_track_time = reco_track.getTimeSeed()

                if abs(reco_track_time) > 30 or chi2 > 10:
                    break

                finished = derivatives_pair.second > 2.7122 and chi2 < 1.739

                number_of_fitted_hits = len(reco_track.getHitPointsWithMeasurement())

                try_list.append(dict(chi2=chi2, extracted_time=reco_track_time,
                                     event_number=event_number,
                                     number_of_fitted_hits=number_of_fitted_hits,
                                     first_deriv=derivatives_pair.first,
                                     second_deriv=derivatives_pair.second,
                                     finished=finished, delta_t0=np.NaN, delta_chi2=np.NaN))
                current_try += 1

            reco_track.setTimeSeed(initial_value)
            reco_track.deleteFittedInformation()

            if current_try > 2 and abs(try_list[-1]["extracted_time"] - try_list[-2]["extracted_time"]) < 2:
                try_list[-1]["delta_t0"] = try_list[-1]["extracted_time"] - try_list[-2]["extracted_time"]
                try_list[-1]["delta_chi2"] = try_list[-1]["chi2"] - try_list[-2]["chi2"]

        event_number = event_meta_data.getEvent()

        for reco_track in Belle2.PyStoreArray("__SelectedRecoTracks"):
            try_list = []

            extract_until_finished(reco_track, t0_min + 0.2 * (t0_max - t0_min), event_number, 2, try_list)
            extract_until_finished(reco_track, t0_min + 0.4 * (t0_max - t0_min), event_number, 2, try_list)
            extract_until_finished(reco_track, t0_min + 0.6 * (t0_max - t0_min), event_number, 2, try_list)
            extract_until_finished(reco_track, t0_min + 0.8 * (t0_max - t0_min), event_number, 2, try_list)

            for try_out in try_list:
                yield try_out

            finished_ones = list(filter(lambda x: x["finished"], try_list))
            if finished_ones:
                yield min(finished_ones, key=lambda i: i["chi2"])
            else:
                best_guesses = sorted(try_list, key=lambda i: i["chi2"])
                for best_guess in best_guesses:
                    extract_until_finished(reco_track, best_guess["extracted_time"], event_number, 2, try_list)
                    if try_list[-1]["finished"]:
                        yield try_list[-1]
                        return

    path.add_module(harvest_track_time)
    # path.add_module(harvest_extracted_times)
    # path.add_module(harvest_full_chi2_grid)
    # path.add_module(harvest_chi2_path)
