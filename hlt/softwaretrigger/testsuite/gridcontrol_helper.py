import subprocess
import csv
import os
import multiprocessing
from shutil import copyfile, rmtree, which

# GC_BELLE2_BACKGROUND_DIR is used, because it needs to set to
# BELLE2_BACKGROUND_DIR in the steering file itself, because
# BELLE2_BACKGROUND_DIR will be overwritten as soon as basf2
# is sourced on batch nodes.

GRIDCONTROL_GLOBAL_CONTENT = """
[global]
task = UserTask
workdir create = True
workdir space = 0
backend = {backend}
"""

GRIDCONTROL_JOB_CONTENT = """
[jobs]
wall time = {job_runtime}
in flight = {jobs_in_flight}
memory = 2100
[wms]
queue = {queue_name}
"""

GRIDCONTROL_COMMON_BASF2_CONTENT = """
[constants]
BASF2_COMPILE_OPTION = {compile_option}
BASF2_TOOLS_LOCATION = {tools_location}
BASF2_RELEASE_LOCATION = {release_location}
BASF2_STEERING_FILE = {steering_file}
"""

# configuration for genertion, reco and analysis of events
DEFAULT_GRIDCONTROL_CONTENT = GRIDCONTROL_GLOBAL_CONTENT \
    + GRIDCONTROL_JOB_CONTENT + GRIDCONTROL_COMMON_BASF2_CONTENT + """
GC_BELLE2_BACKGROUND_DIR = {background_dir}
[parameters]
repeat = 1
parameters = <pfs>
pfs type = csv
pfs source = {parameter_file}
[UserTask]
executable = .basf2_wrapper.sh
input files = {steering_file_abs_path}
"""

# configuration to test HLT processing on raw input events
TEST_PROCESSING_GRIDCONTROL_CONTENT = GRIDCONTROL_GLOBAL_CONTENT \
    + GRIDCONTROL_JOB_CONTENT + GRIDCONTROL_COMMON_BASF2_CONTENT + """
BASF2_LOCAL_DB_PATH = {local_db_path}
{use_gdb}
[UserTask]
executable = .basf2_wrapper_hlt.sh
files per job = 1 ; Number of files to process per job
dataset provider = file
; use wildcard to pickup all input files
dataset = :scan:{dataset_folder}
source recurse = True
filename filter = *.root
"""


def write_gridcontrol_hlt_test(working_folder, hlt_steering_file, dataset_folder, local_db_path, local_execution,
                               jobs_in_flight=100, use_gdb=False):
    if os.path.exists(working_folder):
        rmtree(working_folder)

    os.mkdir(working_folder)

    # this steering file is part of the release, so we don't need to copy it as input
    hlt_steering_file = os.path.abspath(hlt_steering_file)
    gridcontrol_file = os.path.join(working_folder, "hlt_raw_processing.conf")

    # rewrite the use_gdb flag to be used as env variable
    use_gdb = "BASF2_USE_GDB = 1" if use_gdb else ""

    specific_gc_settings = {"dataset_folder": dataset_folder,
                            "steering_file": hlt_steering_file,
                            "queue_name": "l",  # KEKCC long queue
                            "job_runtime": "8:00",  # this jobs might take some hours
                            "use_gdb": use_gdb,
                            "local_db_path": local_db_path}
    return write_gridcontrol_base(local_execution=local_execution, working_folder=working_folder,
                                  gridcontrol_filename=gridcontrol_file,
                                  specific_gc_settings=specific_gc_settings,
                                  jobs_in_flight=jobs_in_flight,
                                  gc_template=TEST_PROCESSING_GRIDCONTROL_CONTENT)


def write_gridcontrol_swtrigger(steering_file, parameters, local_execution):
    background_dir = os.getenv("BELLE2_BACKGROUND_DIR")

    basename = os.path.splitext(steering_file)[0]
    working_folder = basename + "/"

    if os.path.exists(working_folder):
        rmtree(working_folder)

    os.mkdir(working_folder)

    try:
        copyfile(steering_file, os.path.join(working_folder, steering_file))
    except FileExistsError:
        pass

    gridcontrol_file = os.path.join(working_folder, basename + ".conf")
    parameter_file = os.path.join(working_folder, basename + ".csv")

    if len(parameters) == 0:
        return

    if len(parameters[0].keys()) == 1:
        for parameter in parameters:
            parameter["__unused"] = ""

    with open(parameter_file, "w") as f:
        writer = csv.DictWriter(f, fieldnames=parameters[0].keys())
        writer.writeheader()

        writer.writerows(parameters)

    steering_file_abs_path = os.path.abspath(steering_file)

    specific_gc_settings = {"background_dir": background_dir,
                            "parameter_file": parameter_file,
                            "queue_name": "s",  # KEKCC short queue
                            "job_runtime": "1:00",
                            "steering_file": steering_file,
                            "steering_file_abs_path": steering_file_abs_path}
    return write_gridcontrol_base(local_execution=local_execution, working_folder=working_folder,
                                  gridcontrol_filename=gridcontrol_file,
                                  specific_gc_settings=specific_gc_settings)


def write_gridcontrol_base(local_execution, gridcontrol_filename, working_folder, specific_gc_settings,
                           gc_template=DEFAULT_GRIDCONTROL_CONTENT,
                           jobs_in_flight=100):
    release_location = os.getenv("BELLE2_LOCAL_DIR")
    tools_location = os.getenv("BELLE2_TOOLS")
    compile_option = os.getenv("BELLE2_OPTION")

    if local_execution:
        # only run as much jobs as physical CPU (account for hyperthreading multiplier here)
        jobs_in_flight = int(multiprocessing.cpu_count() / 2)
        backend = "host"
    else:
        jobs_in_flight = jobs_in_flight
        backend = "lsf"

    common_gc_settings = {
        "compile_option": compile_option,
        "tools_location": tools_location,
        "release_location": release_location,
        "jobs_in_flight": jobs_in_flight,
        "backend": backend}

    # merged dictionaries
    final_gc_settings = {**common_gc_settings, **specific_gc_settings}

    with open(gridcontrol_filename, "w") as f:
        f.write(gc_template.format(**final_gc_settings))

    return gridcontrol_filename


def call_gridcontrol(gridcontrol_file, retries):
    if gridcontrol_file:
        if which("gridcontrol"):
            executable_path = "gridcontrol"
        else:
            # for working on KEKCC
            executable_path = os.path.expanduser("~/.local/bin/gridcontrol")

        subprocess.check_call([executable_path, f"-m {retries}", "-Gc", gridcontrol_file])
