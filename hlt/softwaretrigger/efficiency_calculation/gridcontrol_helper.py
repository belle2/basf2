import subprocess
import csv
import os
from shutil import copyfile, rmtree


DEFAULT_GRIDCONTROL_CONTENT = """
[global]
task = UserTask
workdir create = True
workdir space = 0
backend = lsf
[jobs]
wall time = 5:00
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
"""


def write_gridcontrol_file(steering_file, parameters):
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

    with open(gridcontrol_file, "w") as f:
        f.write(DEFAULT_GRIDCONTROL_CONTENT.format(
            compile_option=compile_option,
            tools_location=tools_location,
            release_location=release_location,
            steering_file=steering_file,
            parameter_file=parameter_file,
            steering_file_abs_path=steering_file_abs_path
        ))

    return gridcontrol_file


def call_gridcontrol(gridcontrol_file, retries):
    if gridcontrol_file:
        subprocess.check_call(["gridcontrol", f"-m {retries}", "-Gc", gridcontrol_file])
