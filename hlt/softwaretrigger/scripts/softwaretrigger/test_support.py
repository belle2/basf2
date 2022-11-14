##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import os
import random
import shutil
import subprocess
import sys
from glob import glob

import b2test_utils
import basf2
import generators
from simulation import add_simulation
from rawdata import add_packers
from softwaretrigger import constants
from softwaretrigger.constants import DEFAULT_EXPRESSRECO_COMPONENTS, RAWDATA_OBJECTS, DEFAULT_HLT_COMPONENTS
from ROOT import Belle2


class CheckForCorrectHLTResults(basf2.Module):
    """Test module for assuring correct data store content"""

    def event(self):
        """reimplementation of Module::event()."""
        sft_trigger = Belle2.PyStoreObj("SoftwareTriggerResult")

        if not sft_trigger.isValid():
            basf2.B2FATAL("SoftwareTriggerResult object not created")
        elif len(sft_trigger.getResults()) == 0:
            basf2.B2FATAL("SoftwareTriggerResult exists but has no entries")

        if not Belle2.PyStoreArray("ROIs").isValid():
            basf2.B2FATAL("ROIs are not present")


def get_file_name(base_path, run_type, location, passthrough, simulate_events_of_doom_buster):
    mode = ""
    if passthrough:
        mode += "_passthrough"
    if simulate_events_of_doom_buster:
        mode += "_eodb"
    return os.path.join(base_path, f"{location.name}_{run_type.name}{mode}.root")


def generate_input_file(run_type, location, output_file_name, exp_number, passthrough,
                        simulate_events_of_doom_buster):
    """
    Generate an input file for usage in the test.
    Simulate uubar for "beam" and two muons for "cosmic" setting.

    Only raw data will be stored to the given output file.
    :param run_type: Whether to simulate cosmic or beam
    :param location: Whether to simulate expressreco (with ROIs) or hlt (no PXD)
    :param output_file_name: where to store the result file
    :param exp_number: which experiment number to simulate
    :param passthrough: if true don't generate a trigger result in the input file
    :param simulate_events_of_doom_buster: if true, simulate the effect of the
      EventsOfDoomBuster module by inflating the number of CDC hits
    """
    if os.path.exists(output_file_name):
        return 1

    basf2.set_random_seed(12345)

    path = basf2.Path()
    path.add_module('EventInfoSetter', evtNumList=[4], expList=[exp_number])

    if run_type == constants.RunTypes.beam:
        generators.add_continuum_generator(path, finalstate="uubar")
    elif run_type == constants.RunTypes.cosmic:
        # add something which looks a tiny bit like a cosmic generator. We
        # cannot use the normal cosmic generator as that needs a bigger
        # simulation top volume than the default geometry from the database.
        path.add_module("ParticleGun", pdgCodes=[-13, 13], momentumParams=[10, 200])

    add_simulation(path, usePXDDataReduction=(location == constants.Location.expressreco))

    # inflate the number of CDC hits in order to later simulate the effect of the
    # EventsOfDoomBuster module
    if simulate_events_of_doom_buster:

        class InflateCDCHits(basf2.Module):
            """Artificially inflate the number of CDC hits."""

            def initialize(self):
                """Initialize."""
                self.cdc_hits = Belle2.PyStoreArray("CDCHits")
                self.cdc_hits.isRequired()
                eodb_parameters = Belle2.PyDBObj("EventsOfDoomParameters")
                if not eodb_parameters.isValid():
                    basf2.B2FATAL("EventsOfDoomParameters is not valid")
                self.cdc_hits_threshold = eodb_parameters.getNCDCHitsMax() + 1

            def event(self):
                """Event"""
                if self.cdc_hits.isValid():
                    # Let's simply append a (default) CDC hit multiple times
                    for i in range(self.cdc_hits_threshold):
                        self.cdc_hits.appendNew()

        path.add_module(InflateCDCHits())

    if location == constants.Location.hlt:
        components = DEFAULT_HLT_COMPONENTS
    elif location == constants.Location.expressreco:
        components = DEFAULT_EXPRESSRECO_COMPONENTS
    else:
        basf2.B2FATAL(f"Location {location.name} for test is not supported")

    components.append("TRG")

    add_packers(path, components=components)

    # express reco expects to have an HLT results, so lets add a fake one
    if location == constants.Location.expressreco and not passthrough:
        class FakeHLTResult(basf2.Module):
            def initialize(self):
                self.results = Belle2.PyStoreObj(Belle2.SoftwareTriggerResult.Class(), "SoftwareTriggerResult")
                self.results.registerInDataStore()

                self.EventMetaData = Belle2.PyStoreObj("EventMetaData")

            def event(self):
                self.results.create()
                # First event: Add all the results that are used on express reco just to test all paths
                if (self.EventMetaData.obj().getEvent() == 1):
                    self.results.addResult("software_trigger_cut&all&total_result", 1)
                    self.results.addResult("software_trigger_cut&skim&accept_mumutight", 1)
                    self.results.addResult("software_trigger_cut&skim&accept_dstar_1", 1)
                    self.results.addResult("software_trigger_cut&filter&L1_trigger", 1)
                # Second event: No skim lines to replicate a HLT discared event with filter ON
                elif (self.EventMetaData.obj().getEvent() == 2):
                    self.results.addResult("software_trigger_cut&all&total_result", 1)
                    self.results.addResult("software_trigger_cut&filter&L1_trigger", 1)
                # Third event: Does not pass through L1 passthrough
                elif (self.EventMetaData.obj().getEvent() == 3):
                    self.results.addResult("software_trigger_cut&all&total_result", 1)
                    self.results.addResult("software_trigger_cut&skim&accept_mumutight", 1)
                    self.results.addResult("software_trigger_cut&skim&accept_dstar_1", 1)
                    self.results.addResult("software_trigger_cut&filter&L1_trigger", 0)
                # Fourth event: HLT discarded but passes HLT skims (possible in HLT filter OFF mode)
                elif (self.EventMetaData.obj().getEvent() == 4):
                    self.results.addResult("software_trigger_cut&all&total_result", 0)
                    self.results.addResult("software_trigger_cut&skim&accept_mumutight", 1)
                    self.results.addResult("software_trigger_cut&skim&accept_dstar_1", 1)
                    self.results.addResult("software_trigger_cut&filter&L1_trigger", 0)

        path.add_module(FakeHLTResult())

    # remove everything but HLT input raw objects
    branch_names = RAWDATA_OBJECTS + ["EventMetaData", "TRGSummary"]
    if not passthrough:
        branch_names += ["SoftwareTriggerResult"]
    if location == constants.Location.hlt:
        branch_names.remove("RawPXDs")
        branch_names.remove("ROIs")

    # There is no packer for the following objects :(
    branch_names.remove("RawTRGs")

    path.add_module("RootOutput", outputFileName=output_file_name, branchNames=branch_names)

    basf2.process(path)

    return 0


def test_script(script_location, input_file_name, temp_dir):
    """
    Test a script with the given file path using the given input file.
    Raises an exception if the execution fails or if the needed output is not in
    the output file.
    The results are stored in the temporary directory location.

    :param script_location: the script to test
    :param input_file_name: the file path of the input file
    :param temp_dir: where to store and run
    """
    input_buffer = "UNUSED"   # unused
    output_buffer = "UNUSED"  # unused
    histo_port = 6666         # unused

    random_seed = "".join(random.choices("abcdef", k=4))

    histos_file_name = f"{random_seed}_histos.root"
    output_file_name = os.path.join(temp_dir, f"{random_seed}_output.root")
    # TODO: should we use the default global tag here?
    globaltags = list(basf2.conditions.default_globaltags)
    num_processes = 1

    # Because the script name is hard-coded in the run control GUI,
    # we must jump into the script directory
    cwd = os.getcwd()
    os.chdir(os.path.dirname(script_location))
    cmd1 = [sys.executable, script_location, "--central-db-tag"] + globaltags + [
        "--input-file", os.path.abspath(input_file_name),
        "--histo-output-file", f"./{histos_file_name}",
        "--output-file", os.path.abspath(output_file_name),
        "--number-processes", str(num_processes),
        input_buffer, output_buffer, str(histo_port)
    ]
    subprocess.check_call(cmd1)

    # Move the output file with DQM histograms under the expected location:
    # for reasons we don't want to know, they are saved under the current directory
    # even if a valid and existing working directory is specified
    if os.path.exists(histos_file_name):
        final_histos_file_name = os.path.join(temp_dir, histos_file_name)
        shutil.copy(histos_file_name, os.path.join(temp_dir, final_histos_file_name))
        os.unlink(histos_file_name)

    # Go back to the original directory for safety
    os.chdir(cwd)

    if "expressreco" not in script_location and "beam_reco" in script_location:
        # Check the integrity of HLT result
        test_path = basf2.Path()
        test_path.add_module("RootInput", inputFileName=output_file_name)
        test_path.add_module(CheckForCorrectHLTResults())
        assert(b2test_utils.safe_process(test_path) == 0)
        # Check the size of DQM histograms
        cmd2 = ["hlt-check-dqm-size", final_histos_file_name]
        subprocess.check_call(cmd2)


def test_folder(location, run_type, exp_number, phase, passthrough=False,
                simulate_events_of_doom_buster=False):
    """
    Run all hlt operation scripts in a given folder
    and test the outputs of the files.

    Will call the test_script function on all files in the folder given
    by the location after having created a suitable input file with the given
    experiment number.

    :param location: hlt or expressreco, depending on which operation files to run
                     and which input to simulate
    :param run_type: cosmic or beam, depending on which operation files to run
                     and which input to simulate
    :param exp_number: which experiment number to simulate
    :param phase:    where to look for the operation files (will search in the folder
                     hlt/operation/{phase}/global/{location}/evp_scripts/)
    :param passthrough: only relevant for express reco: If true don't create a
                     software trigger result in the input file to test running
                     express reco if hlt is in passthrough mode
    :param simulate_events_of_doom_buster: if true, simulate the effect of the
                     EventsOfDoomBuster module by inflating the number of CDC hits
    """

    # The test is already run in a clean, temporary directory
    temp_dir = os.getcwd()
    prepare_path = os.environ["BELLE2_PREPARE_PATH"]
    input_file_name = get_file_name(
        prepare_path, run_type, location, passthrough, simulate_events_of_doom_buster)
    # generate_input_file(run_type=run_type, location=location,
    #                    output_file_name=output_file_name, exp_number=exp_number,
    #                    passthrough=passthrough,
    #                    simulate_events_of_doom_buster=simulate_events_of_doom_buster)

    script_dir = basf2.find_file(f"hlt/operation/{phase}/global/{location.name}/evp_scripts/")
    run_at_least_one = False
    for script_location in glob(os.path.join(script_dir, f"run_{run_type.name}*.py")):
        run_at_least_one = True
        test_script(script_location, input_file_name=input_file_name, temp_dir=temp_dir)

    assert run_at_least_one
