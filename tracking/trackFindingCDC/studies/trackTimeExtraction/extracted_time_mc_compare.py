# Analysis script to perform a time extraction and analyse the results. Can use multiple different generators
# and different mc drifts, combined with different finders or extraction algorithms.

import basf2
import os

import simulation
import tracking

from time_extraction_helper_modules import *


def main(mc_drift, generator_module, track_finder, turns):
    input_file_name = "input_{drift}_{generator}.root".format(drift=mc_drift, generator=generator_module)

    path = basf2.create_path()

    # Create Event information. We need more events when the mc drift is not fixed to have enough statistics.
    if mc_drift == -999:
        path.add_module('EventInfoSetter',
                        evtNumList=[10000],
                        runList=[1],
                        )
    else:
        path.add_module('EventInfoSetter',
                        evtNumList=[1000],
                        runList=[1],
                        )

    # Show progress of processing
    path.add_module('Progress')

    # Load gearbox parameters
    path.add_module('Gearbox')

    # Create geometry
    path.add_module('Geometry')

    if not os.path.exists(input_file_name):
        if generator_module == "evtgen":
            path.add_module("EvtGenInput")

        elif generator_module == "particle_gun":
            muon_pdg_code = 13
            path.add_module("ParticleGun",
                            momentumParams=[2.0, 3.0],
                            pdgCodes=[muon_pdg_code, -muon_pdg_code],
                            nTracks=1)
        elif generator_module == "particle_gun_variable_pt":
            muon_pdg_code = 13
            path.add_module("ParticleGun",
                            momentumParams=[0.3, 3.0],
                            pdgCodes=[muon_pdg_code, -muon_pdg_code],
                            nTracks=1)
        else:
            raise ValueError("Unknown generator module key " + generator_module)

        path.add_module("FullSim")
        path.add_module(RandomizeTrackTimeModule(mc_drift))
        simulation.add_simulation(path)

        path.add_module("RootOutput", outputFileName=input_file_name)
    else:
        path.add_module("RootInput", inputFileName=input_file_name)

    if track_finder == "reconstruction":
        tracking.add_cdc_track_finding(path)
    elif track_finder == "mc":
        tracking.add_mc_track_finding(path, components=["CDC"])
        path.add_module(MCRecoTracksResetModule())

    else:
        raise ValueError("No presetting for track finder {track_finder}".format(track_finder=track_finder))

    # Correct the time of the reco tracks to include the time of flight.
    path.add_module("IPTrackTimeEstimator", useFittedInformation=False)

    # Do one fitting step needed for deciding if the tracks should be used in the extraction.
    path.add_module("SetupGenfitExtrapolation")
    path.add_module("DAFRecoFitter")

    # Select the tracks for the time extraction.
    path.add_module("SelectionForTrackTimeExtraction")

    # Extract the time: either with the TrackTimeExtraction or the FullGridTrackTimeExtraction module.
    if turns > 0:
        path.add_module("TrackTimeExtraction", maximalIterations=turns)
    elif turns == 0:
        path.add_module("FullGridTrackTimeExtraction")

    # Add some analysis modules.
    add_harvester(path, mc_drift, generator_module, track_finder, turns)

    # Do the processing.
    basf2.print_path(path)
    basf2.process(path)

    # show call statistics.
    print(basf2.statistics)


if __name__ == "__main__":
    # Make the parameters accessible form the outside.
    import argparse
    parser = argparse.ArgumentParser()
    parser.add_argument("mc_drift", help="The drift to apply to the event. -999 means randomize the drift " +
                                         "between -50 and 50.")
    parser.add_argument("generator_module", help="Which generator module to use. Choose between evtgen, " +
                                                 "particle_gun and particle_gun_variable_pt")
    parser.add_argument("track_finder", help="Which track finder algorithm to use. Choose between " +
                                             "reconstruction or mc.")
    parser.add_argument("turns", help="How many turns the time extraction module should take. For more than one turn, " +
                                      "the TrackTimeExtractionModule is used, for 0 turns the FullGridTrackTimeExtraction.")

    args = parser.parse_args()

    main(int(args.mc_drift), args.generator_module, args.track_finder, int(args.turns))
