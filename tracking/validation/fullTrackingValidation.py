#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
  <contact>software-tracking@belle2.org</contact>
  <input>EvtGenSimNoBkg.root</input>
  <output>FullTrackingValidation.root</output>
  <description>This script validates the full track finding chain in Y(4S) runs.</description>
</header>
"""

import basf2
basf2.set_random_seed(1337)

import logging
import tracking

from tracking.validation.run import TrackingValidationRun

test_cuts = [0.0, 0.7, 0.75]  # 0.25, 0.5, 0.6, 0.7, 0.75, 0.8, 0.85, 0.9, 0.95]
processes = ['BBBAR']
with open("fullVal_cut_iterator.txt", "r") as cut_file:
    iterator = int(cut_file.read())
process_it, cut_it = divmod(iterator, 3)
cut = test_cuts[cut_it]
process = processes[process_it]
print("The cut value of the CDC QI is:", cut)
print("Test the process:", process)
print(str(int(cut*100)).zfill(3))
N_EVENTS = 2000
# elif process =='BBBAR':
#    N_EVENTS=1000
VALIDATION_OUTPUT_FILE = 'ftv_reco_qi_N' + str(N_EVENTS) + '_' + process + \
                                               '_novxd_cdcQi' + str(int(cut*100)).zfill(3) + '_exp0.root'

print(VALIDATION_OUTPUT_FILE)
# cut = 0.0
# VALIDATION_OUTPUT_FILE = 'ftv_cdc_qi_N1000_cdcQi' + str(int(cut*100)).zfill(3) + '.root'
# N_EVENTS = 1000
ACTIVE = True


class Full(TrackingValidationRun):
    """Validate the full track-finding chain"""
    #: number of events to generate
    n_events = N_EVENTS
    #: Generator to be used in the simulation (-so)
    generator_module = 'generic'
    #: no background overlay
    # root_input_file = '../EvtGenSim_1k_exp1003_run2_new.root'
    root_input_file = 'generated_mc_N' + str(N_EVENTS) + '_' + process + '_test_exp0.root'
    #: use the complete track-reconstruction chain
    #: finder_module = staticmethod(tracking.add_tracking_reconstruction)

    def finder_module(self, path):
        tracking.add_tracking_reconstruction(path, add_cdcTrack_QI=True, add_vxdTrack_QI=False, add_recoTrack_QI=True)

        # Replace weightfile identifiers from defaults (CDB payloads) to new
        # weightfiles created by the b2luigi script
        cdc_qe_mva_filter_parameters = {
            # "identifier": 'trackfindingcdc_TrackQualityIndicator_nTrees350_nCuts6_nLevels5_shrin10_skimmedVar.weights.xml',
            # "identifier": 'cdc_mva_qe_nTrees350_nCuts6_nLevels5_shrin10.weights.xml',
            "cut": cut}
        basf2.set_module_parameters(
            path,
            name="TFCDC_TrackQualityEstimator",
            filterParameters=cdc_qe_mva_filter_parameters,
            deleteTracks=True,
            resetTakenFlag=True
            )
        # basf2.set_module_parameters(
        #    path,
        #    name="VXDQualityEstimatorMVA",
        #    WeightFileIdentifier='vxdtf2_mva_qe_nTrees350_nCuts6_nLevels5_shrin10.weights.xml'
        #    )
        basf2.set_module_parameters(
            path,
            name="TrackQualityEstimatorMVA",
            WeightFileIdentifier="recotrack_mva_qe_nTrees350_nCuts6_nLevels5_shrin10_deleteCDCQI"
            + str(int(cut*100)).zfill(3) + "_noVXD_useCDC.weights.xml",
            )

    #: Define the user parameters for the track-finding module
    tracking_coverage = {
        'WhichParticles': [],  # Include all particles, also secondaries
        'UsePXDHits': True,
        'UseSVDHits': True,
        'UseCDCHits': True,
        "UseReassignedHits": True,
        'UseNLoops': 1
    }
    #: tracks will be already fitted by
    #: add_tracking_reconstruction finder module set above
    fit_tracks = False
    #: But we need to tell the validation module to use the fit information
    use_fit_information = True
    #: do not create expert-level output histograms
    use_expert_folder = False
    #: Include pulls in the validation output
    pulls = True
    #: Include resolution information in the validation output
    resolution = True
    # save trees such that manual studies are possible
    extended = True
    saveFullTrees = True
    #: name of the output ROOT file
    output_file_name = VALIDATION_OUTPUT_FILE


def main():
    validation_run = Full()
    validation_run.configure_and_execute_from_commandline()


if __name__ == '__main__':
    logging.basicConfig(level=logging.INFO)
    if ACTIVE:
        main()
