#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Script that tests the various `caf.backends` and some options that are available.
This cannot be run as a normal unit test because not all backends are available on the
DESY cloud servers. So the only think we can do is test them individually on machines that
do have the backends installed.
"""

from caf import cli
from caf.backends import Job, monitor_jobs, Local
from pathlib import Path
import basf2
from basf2 import find_file

# Prevent ROOT inserting its own help text and arguments
import ROOT
ROOT.PyConfig.IgnoreCommandLineOptions = True


test_script = Path(
    find_file("calibration/examples/job_submission/test_script.sh"))
test_basf2_script = Path(
    find_file("calibration/examples/job_submission/basic_basf2.py"))
test_data = Path(find_file("calibration/examples/job_submission/test_data"))


def test_path_exists(path):
    if not path.exists():
        raise FileNotFoundError(
            f"The expected file {test_script.as_posix()} does not exist.")


def create_jobs(args):
    test_path_exists(test_data)
    test_path_exists(test_script)
    test_path_exists(test_basf2_script)
    output_dir = Path("test_backends_jobs")
    j1 = Job(name="test_job1")
    # Working directory that will be sent to the backend and used as the
    # current working directory
    j1.working_dir = Path(output_dir, j1.name, "working_dir").absolute()
    # Output directory, where the stdout and stderr will be sent.
    j1.output_dir = Path(output_dir, j1.name, "output_dir").absolute()
    # The command we want to run
    j1.cmd = ["bash", test_script.name]
    j1.args = ["first_arg_example", "\"Do quotes work?\""]
    # We add the script we want to run to the input sandbox so that it is available locally in the working directory.
    # We could instead simply change the 'cmd' above to always call the same file, but I prefer having the
    # script copied into the working directory.
    j1.input_sandbox_files.append(test_script.absolute())

    j2 = Job(name="test_job2")
    # Working directory that will be sent to the backend and used as the
    # current working directory
    j2.working_dir = Path(output_dir, j2.name).absolute()
    # Output directory, where the stdout and stderr will be sent. We have set
    # this to be the same as the working dir
    j2.output_dir = Path(output_dir, j2.name).absolute()
    # The command we want to run
    j2.cmd = ["basf2", test_basf2_script.name]
    # Setup basf2 in the job the same way as our current basf2 environment
    j2.append_current_basf2_setup_cmds()
    j2.input_sandbox_files.append(test_basf2_script.absolute())
    j2.input_files = sorted(p.as_posix() for p in Path(test_data).glob("*.root"))
    if args.files_per_subjob:
        j2.max_files_per_subjob = args.files_per_subjob
    elif args.max_subjobs:
        j2.max_subjobs = args.max_subjobs
    return [j1, j2]


def get_argparser():
    """Setup the argparser for this script"""
    import argparse
    parser = argparse.ArgumentParser(
        formatter_class=argparse.RawTextHelpFormatter)

    subparsers = cli.add_backends_subparsers(parser)
    for subparser in subparsers:
        cli.add_basf2_options(subparser)
        cli.add_monitor_options(subparser)
        cli.add_job_options(subparser)
    return parser


def main():
    parser = get_argparser()
    args = parser.parse_args()
    basf2.set_log_level(basf2.LogLevel.names[args.log_level])
    if args.debug_level:
        basf2.set_log_level(basf2.LogLevel.DEBUG)  # Override
        basf2.set_debug_level(args.debug_level)
    jobs = create_jobs(args)
    backend = args.func(args)
    backend.submit(jobs)
    monitor_jobs(args, jobs)
    if isinstance(backend, Local):
        backend.join()


if __name__ == "__main__":
    import sys
    sys.exit(main())
