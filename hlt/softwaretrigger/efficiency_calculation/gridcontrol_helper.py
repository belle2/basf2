import subprocess
import csv
import os
import multiprocessing
from shutil import copyfile, rmtree


DEFAULT_GRIDCONTROL_CONTENT = """
[global]
task = UserTask
workdir create = True
workdir space = 0
backend = {backend}
[jobs]
wall time = 1:00
in flight = {jobs_in_flight}
[constants]
BASF2_COMPILE_OPTION = {compile_option}
BASF2_TOOLS_LOCATION = {tools_location}
BASF2_RELEASE_LOCATION = {release_location}
BASF2_STEERING_FILE = {steering_file}
[parameters]
repeat = 1
parameters = <pfs>
pfs type = csv
pfs source = {parameter_file}
[UserTask]
executable = .basf2_wrapper.sh
memory = 2100
input files = {steering_file_abs_path}
[wms]
queue = s
"""


def write_gridcontrol_file(steering_file, parameters, local_execution):
    release_location = os.getenv("BELLE2_LOCAL_DIR")
    tools_location = os.getenv("BELLE2_TOOLS")
    compile_option = os.getenv("BELLE2_OPTION")

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

    if local_execution:
        # only run as much jobs as physical CPU (account for hyperthreading multiplier here)
        jobs_in_flight = int(multiprocessing.cpu_count() / 2)
        backend = "host"
    else:
        jobs_in_flight = 50000
        backend = "lsf"

    with open(gridcontrol_file, "w") as f:
        f.write(DEFAULT_GRIDCONTROL_CONTENT.format(
            compile_option=compile_option,
            tools_location=tools_location,
            release_location=release_location,
            steering_file=steering_file,
            parameter_file=parameter_file,
            steering_file_abs_path=steering_file_abs_path,
            jobs_in_flight=jobs_in_flight,
            backend=backend
        ))

    return gridcontrol_file


def call_gridcontrol(gridcontrol_file, retries):
    if gridcontrol_file:
        subprocess.check_call(["gridcontrol", f"-m {retries}", "-Gc", gridcontrol_file])
