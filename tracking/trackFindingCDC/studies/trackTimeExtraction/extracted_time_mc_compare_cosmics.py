# Analysis script to perform a time extraction and analyse the results. Can use multiple different generators
# and different mc drifts, combined with different finders or extraction algorithms.
import os

import basf2
from ROOT import Belle2

import simulation
import tracking

from time_extraction_helper_modules import MCRecoTracksResetModule, RandomizeTrackTimeModule


def main(mc_drift, generator_module, track_finder, turns, selection_mode, selected_tracks):
    basf2.set_nprocesses(4)

    output_file_name = "input_{drift}_{generator}"

    if selection_mode != "all_tracks":
        output_file_name += "_{selection_mode}_{selected_tracks}.root"
    else:
        output_file_name += "_all_tracks.root"

    output_file_name = output_file_name.format(drift=mc_drift,
                                               generator=generator_module,
                                               selection_mode=selection_mode,
                                               selected_tracks=selected_tracks)

    if os.path.exists(output_file_name):
        print("Output file already present.")
        exit()

    path = basf2.create_path()

    # Create Event information. We need more events when the mc drift is not fixed to have enough statistics.
    if mc_drift == -999:
        path.add_module('EventInfoSetter',
                        evtNumList=[10000],
                        runList=[1],
                        )
    else:
        path.add_module('EventInfoSetter',
                        evtNumList=[5000],
                        runList=[2],
                        )

    # Load gearbox parameters
    path.add_module('Gearbox',
                    override=[
                        ("/Global/length", "8.", "m"),
                        ("/Global/width", "8.", "m"),
                        ("/Global/height", "1.5", "m")
                    ])

    # Create geometry
    path.add_module('Geometry', components=['CDC'])

    if generator_module == "evtgen":
        path.add_module("EvtGenInput")

    elif generator_module == "particle_gun":
        muon_pdg_code = 13
        path.add_module("ParticleGun",
                        momentumParams=[1.0, 1.0],
                        pdgCodes=[muon_pdg_code, -muon_pdg_code],
                        thetaParams=[90., 90.],
                        phiGeneration='uniform',
                        phiParams=[-270, -270],
                        nTracks=1)

    elif generator_module == "particle_gun_variable_pt":
        muon_pdg_code = 13
        path.add_module("ParticleGun",
                        momentumParams=[0.3, 3.0],
                        pdgCodes=[muon_pdg_code, -muon_pdg_code],
                        nTracks=1)

    elif generator_module == "cosmics":
        # Register the CRY module
        path.add_module('CRYInput', CosmicDataDir=Belle2.FileSystem.findFile('data/generators/modules/cryinput/'),
                        # user input file
                        SetupFile='cry.setup',
                        # acceptance half-lengths - at least one particle has to enter that box to use that event
                        acceptLength=0.6,
                        acceptWidth=0.2,
                        acceptHeight=0.2,
                        maxTrials=10000,
                        # keep half-lengths - all particles that do not enter the box are removed
                        # (keep box >= accept box)
                        keepLength=0.6,
                        keepWidth=0.2,
                        keepHeight=0.2,
                        # minimal kinetic energy - all particles below that energy are ignored
                        kineticEnergyThreshold=0.01)

    else:
        raise ValueError("Unknown generator module key " + generator_module)

    # to prevent delta rays, not realistic !
    path.add_module('FullSim', ProductionCut=1000000.)
    path.add_module(RandomizeTrackTimeModule(mc_drift))

    simulation.add_simulation(path, components=["CDC"])

    if track_finder == "reconstruction":
        tracking.add_cdc_cr_track_finding(path)

    elif track_finder == "mc":
        tracking.add_mc_track_finding(path, components=["CDC"])
        path.add_module(MCRecoTracksResetModule())

    else:
        raise ValueError("No presetting for track finder {track_finder}".format(track_finder=track_finder))

    # Do one fitting step needed for deciding if the tracks should be used in the extraction.
    path.add_module("SetupGenfitExtrapolation")
    path.add_module("PlaneTriggerTrackTimeEstimator",
                    pdgCodeToUseForEstimation=13,
                    triggerPlanePosition=[0, 0, 0],
                    triggerPlaneDirection=[0, 1, 0],
                    useFittedInformation=False,
                    useReadoutPosition=False)

    path.add_module("DAFRecoFitter",
                    pdgCodesToUseForFitting=13,
                    )

    # Select the tracks for the time extraction.
    if selection_mode != "all_tracks":
        path.add_module("SelectionForTrackTimeExtraction", selectionCriteria=selection_mode,
                        maximalNumberOfTracks=selected_tracks)

    # Extract the time: either with the TrackTimeExtraction or the FullGridTrackTimeExtraction module.
    if turns > 0:
        path.add_module("TrackTimeExtraction", maximalIterations=turns)
    elif turns == 0:
        if selection_mode != "all_tracks":
            path.add_module(
                "FullGridTrackTimeExtraction",
                recoTracksStoreArrayName="__SelectedRecoTracks",
                minimalT0Shift=-70,
                maximalT0Shift=70,
                numberOfGrids=6)
        else:
            path.add_module("FullGridTrackTimeExtraction", recoTracksStoreArrayName="RecoTracks",
                            maximalT0Shift=70, minimalT0Shift=-70, numberOfGrids=6)

    path.add_module("DAFRecoFitter",
                    pdgCodesToUseForFitting=13)

    path.add_module('TrackCreator')
    path.add_module('CDCCosmicAnalysis',
                    Output='output.root')
    path.add_module("RootOutput", outputFileName=output_file_name)

    # Show progress of processing
    path.add_module('Progress')

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
                                                 "particle_gun, particle_gun_variable_pt and cosmics")
    parser.add_argument("track_finder", help="Which track finder algorithm to use. Choose between " +
                                             "reconstruction or mc.")
    parser.add_argument("turns",
                        help="How many turns the time extraction module should take. For more than one turn, " +
                             "the TrackTimeExtractionModule is used, for 0 turns the FullGridTrackTimeExtraction.")

    parser.add_argument("selection_mode",
                        help="Which selection criteria to use")

    parser.add_argument("selected_tracks",
                        help="How many selected tracks should be used.")

    args = parser.parse_args()

    main(int(args.mc_drift), args.generator_module, args.track_finder, int(args.turns),
         args.selection_mode, int(args.selected_tracks))
