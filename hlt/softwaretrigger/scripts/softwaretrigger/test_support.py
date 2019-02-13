import subprocess
import os
import sys
import argparse
import tempfile
from glob import glob

import basf2
import generators
from simulation import add_simulation
from rawdata import add_packers
from softwaretrigger.path_functions import get_store_only_rawdata_path, DEFAULT_HLT_COMPONENTS
from softwaretrigger.path_functions import DEFAULT_EXPRESSRECO_COMPONENTS, RAWDATA_OBJECTS
from ROOT import Belle2
from L1trigger import add_tsim
find_file = Belle2.FileSystem.findFile

import ROOT
import basf2


class CheckForCorrectHLTResults(basf2.Module):
    """test"""

    def event(self):
        """reimplementation of Module::event()."""
        sft_trigger = ROOT.Belle2.PyStoreObj("SoftwareTriggerResult")

        if not sft_trigger.isValid():
            basf2.B2FATAL("SoftwareTriggerResult object not created")
        elif len(sft_trigger.getResults()) == 0:
            basf2.B2FATAL("SoftwareTriggerResult exists but has no entries")

        if not ROOT.Belle2.PyStoreObj("ROIs").isValid():
            basf2.B2FATAL("ROIs are not present")


def generate_input_file(run_type, location, output_file_name, exp_number):
    if os.path.exists(output_file_name):
        return

    basf2.set_random_seed(12345)

    path = basf2.Path()
    path.add_module('EventInfoSetter', evtNumList=[1], expList=[exp_number])

    if run_type == "beam":
        generators.add_continuum_generator(path, finalstate="uubar")
    elif run_type == "cosmic":
        # add something which looks a tiny bit like a cosmic generator. We
        # cannot use the normal cosmic generator as that needs a bigger
        # simulation top volume than the default geometry from the database.
        path.add_module("ParticleGun", pdgCodes=[-13, 13], momentumParams=[10, 200])

    add_simulation(path, usePXDDataReduction=False)
    add_tsim(path)

    if location == "hlt":
        components = DEFAULT_HLT_COMPONENTS
    elif location == "expressreco":
        components = DEFAULT_EXPRESSRECO_COMPONENTS
    else:
        basf2.B2FATAL("Location {} for test is not supported".format(location))

    components.append("TRG")

    add_packers(path, components=components)

    # remove everything but HLT input raw objects
    branch_names = RAWDATA_OBJECTS + ["EventMetaData", "TRGSummary"]
    if location == "hlt":
        branch_names.remove("RawPXDs")
        branch_names.remove("ROIs")
    branch_names.remove("RawTRGs")
    branch_names.remove("RawFTSWs")
    path.add_module("RootOutput", outputFileName=output_file_name, branchNames=branch_names)

    basf2.process(path)


def test_script(script_location, input_file_name, temp_dir):
    input_buffer = "UNUSED"   # unused
    output_buffer = "UNUSED"  # unused
    histo_port = 6666         # unused

    histos_file_name = os.path.join(temp_dir, "histos.root")
    output_file_name = os.path.join(temp_dir, "output.root")
    # TODO: should we use the default global tag here?
    central_database = basf2.get_default_global_tags()
    num_processes = 1

    cmd = [sys.executable, script_location,
           "--central-db-tag", central_database,
           "--input-file", input_file_name,
           "--histo-output-file", histos_file_name,
           "--output-file", output_file_name,
           input_buffer, output_buffer, str(histo_port), str(num_processes)]

    subprocess.check_call(cmd)

    test_path = basf2.Path()
    test_path.add_module("RootInput", inputFileName=output_file_name)
    test_path.add_module(CheckForCorrectHLTResults())

    if "beam_reco" in script_location:
        basf2.process(test_path)


def test_folder(location, run_type, exp_number):
    temp_dir = tempfile.mkdtemp()
    output_file_name = os.path.join(temp_dir, f"{location}_{run_type}.root")
    generate_input_file(run_type=run_type, location=location,
                        output_file_name=output_file_name, exp_number=exp_number)

    script_dir = find_file(f"hlt/operation/phase3/global/{location}/evp_scripts/")
    for script_location in glob(os.path.join(script_dir, f"{run_type}_*.py")):
        test_script(script_location, input_file_name=output_file_name, temp_dir=temp_dir)
