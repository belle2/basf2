import basf2
from basf2 import B2INFO

import argparse

from caf.backends import Batch, HTCondor, LSF, Local, PBS


def command_local(args, backend_args=None):
    """
    Runs the jobs using the Local backend i.e. local multiprocessing.

    Parameters:
        args : Command line arguments that may contain backend args options. These take priority.
        backend_args (dict): Backend arguments that will be applied. Specific backend args set by the args positional argument
            will take priority over these values.
    """
    B2INFO(f"Requested use of Local backend")
    backend = Local(max_processes=args.max_processes, backend_args=backend_args)
    return backend


def command_lsf(args, backend_args=None):
    """
    Runs the jobs using the LSF backend

    Parameters:
        args : Command line arguments that may contain backend args options. These take priority.
        backend_args (dict): Backend arguments that will be applied. Specific backend args set by the args positional argument
            will take priority over these values.
    """
    B2INFO(f"Requested use of LSF backend")
    command_line_backend_args = {"queue": args.queue}
    # If any backend_args are None then they shouldn't overwrite
    command_line_backend_args = {key: value for key, value in command_line_backend_args.items() if value is not None}
    if backend_args is None:
        backend_args = {}
    backend_args = {**backend_args, **command_line_backend_args}
    backend = LSF(backend_args=backend_args)  # Sets the backend defaults (can be overriden at the Job level)
    backend.global_job_limit = args.global_job_limit
    backend.sleep_between_submission_checks = args.submission_check_heartbeat
    return backend


def command_pbs(args, backend_args=None):
    """
    Runs the jobs using the PBS backend

    Parameters:
        args : Command line arguments that may contain backend args options. These take priority.
        backend_args (dict): Backend arguments that will be applied. Specific backend args set by the args positional argument
            will take priority over these values.
    """
    B2INFO(f"Requested use of PBS backend")
    command_line_backend_args = {"queue": args.queue}
    command_line_backend_args = {key: value for key, value in command_line_backend_args.items() if value is not None}
    if backend_args is None:
        backend_args = {}
    backend_args = {**backend_args, **command_line_backend_args}
    backend = PBS(backend_args=backend_args)  # Sets the backend defaults (can be overriden at the Job level)
    backend.global_job_limit = args.global_job_limit
    backend.sleep_between_submission_checks = args.submission_check_heartbeat
    return backend


def command_condor(args, backend_args=None):
    """
    Runs the jobs using the HTCondor backend

    Parameters:
        args : Command line arguments that may contain backend args options. These take priority.
        backend_args (dict): Backend arguments that will be applied. Specific backend args set by the args positional argument
            will take priority over these values.
    """
    B2INFO(f"Requested use of HTCondor backend")
    command_line_backend_args = {
                                 "universe": args.universe,
                                 "getenv": args.getenv,
                                 "path_prefix": args.path_prefix
                                }
    command_line_backend_args = {key: value for key, value in command_line_backend_args.items() if value is not None}
    if backend_args is None:
        backend_args = {}
    backend_args = {**backend_args, **command_line_backend_args}
    backend = HTCondor(backend_args=backend_args)  # Sets the backend defaults (can be overriden at the Job level)
    backend.global_job_limit = args.global_job_limit
    backend.sleep_between_submission_checks = args.submission_check_heartbeat
    return backend


def add_basf2_options(parser, default_log_level="INFO"):
    choices = list(basf2.LogLevel.names.keys())
    choices.remove("default")
    parser.add_argument("--log-level", dest="log_level", choices=choices,
                        metavar="", default=default_log_level,
                        help=(f"Set the basf2 LogLevel. (default: {default_log_level}"))
    parser.add_argument("--debug-level", dest="debug_level",
                        type=int, metavar="",
                        help="Set the DEBUG level value, overrides log-level to be DEBUG.")


def add_monitor_options(parser, default_heartbeat=10):
    """
    Adds parser options for the monitoring of CAF jobs.
    """
    parser.add_argument("--heartbeat", dest="heartbeat",
                        type=int, metavar="", default=default_heartbeat,
                        help=("Sets the sleep interval (seconds) between attempts to check the readiness of jobs."
                              f" (default: {default_heartbeat})")
                        )


def add_job_options(parser):
    """
    Adds some overall options to a parser, useful for CAF type jobs that can be used with any backend.
    """
    group = parser.add_mutually_exclusive_group()
    group.add_argument("--max-files-per-subjob", dest="files_per_subjob",
                       type=int, metavar="",
                       help="Sets the number of input files that wil be used per subjob.")
    group.add_argument("--max-subjobs", dest="max_subjobs",
                       type=int, metavar="",
                       help=("Sets the maximum number of subjobs that will be submitted. "
                             "Input files will be split as evenly as possible between the subjobs."))


def add_backends_subparsers(parser, default_max_processes=4,
                            default_global_job_limit=Batch.default_global_job_limit,
                            default_submission_check_heartbeat=Batch.default_sleep_between_submission_checks,
                            local_func=command_local, lsf_func=command_lsf,
                            pbs_func=command_pbs, condor_func=command_condor):
    """
    Adds a subparser for each CAF backend type that is supported.
    Provides arguments specific to each Backend type.
    """
    # Add each backend as a subparser with their own options
    subparsers = parser.add_subparsers(help="Choose the CAF backend to use when submitting jobs.")
    local_parser = subparsers.add_parser("Local",
                                         help="Use the local backend.",
                                         description="Runs the jobs using the Local backend i.e. local multiprocessing.",
                                         formatter_class=argparse.RawDescriptionHelpFormatter)
    local_parser.set_defaults(func=local_func)
    local_parser.add_argument("--max-processes", dest="max_processes",
                              type=int, metavar="", default=default_max_processes,
                              help=("Set the multiprocessing Pool size (max concurrent processes)."
                                    f" (default: {default_max_processes})"))

    ###
    lsf_parser = subparsers.add_parser("LSF",
                                       help="Use the LSF backend.",
                                       description="Runs the jobs using the LSF backend.",
                                       formatter_class=argparse.RawDescriptionHelpFormatter)
    lsf_parser.set_defaults(func=lsf_func)

    lsf_parser.add_argument("--queue", dest="queue", metavar="",
                            help=("The batch queue to use."
                                  " (e.g. s)"))

    lsf_parser.add_argument("--global-job-limit", dest="global_job_limit", metavar="", default=default_global_job_limit, type=int,
                            help=("The number of batch jobs that can be active for the user before the backend class will stop "
                                  "submitting. This is not a completely hard limit, the actual max reached depends on other "
                                  "submitting processes and the number submitted before re-checking."
                                  f" (default: {default_global_job_limit})"))

    lsf_parser.add_argument("--submission-check-heartbeat", dest="submission_check_heartbeat", metavar="", type=int,
                            default=default_submission_check_heartbeat,
                            help=("The time (seconds) between checking if there are fewer batch jobs than the global limit. "
                                  "Generally not needed to change, but it certainly shouldn't be set lower than 30 seconds."
                                  f" (default: {default_submission_check_heartbeat})"))

    ###
    pbs_parser = subparsers.add_parser("PBS",
                                       help="Use the PBS backend.",
                                       description="Runs the jobs using the PBS backend.",
                                       formatter_class=argparse.RawDescriptionHelpFormatter)
    pbs_parser.set_defaults(func=pbs_func)

    pbs_parser.add_argument("--queue", dest="queue", metavar="",
                            help=("The batch queue to use."
                                  " e.g. short"))

    pbs_parser.add_argument("--global-job-limit", dest="global_job_limit", metavar="", default=default_global_job_limit, type=int,
                            help=("The number of batch jobs that can be active for the user before the backend class will stop "
                                  "submitting. This is not a completely hard limit, the actual max reached depends on other "
                                  "submitting processes and the number submitted before re-checking."
                                  f" (default: {default_global_job_limit})"))

    pbs_parser.add_argument("--submission-check-heartbeat", dest="submission_check_heartbeat", metavar="", type=int,
                            default=default_submission_check_heartbeat,
                            help=("The time (seconds) between checking if there are fewer batch jobs than the global limit. "
                                  "Generally not needed to change, but it certainly shouldn't be set lower than 30 seconds."
                                  f" (default: {default_submission_check_heartbeat})"))

    ###
    condor_parser = subparsers.add_parser("HTCondor",
                                          help="Use the HTCondor backend.",
                                          description="Runs the jobs using the HTCondor backend.",
                                          formatter_class=argparse.RawDescriptionHelpFormatter)
    condor_parser.set_defaults(func=condor_func)

    condor_parser.add_argument("--getenv", dest="getenv", metavar="",
                               help=("Should jobs inherit the submitting environment (doesn't always work as expected)."
                                     f" e.g. false"))

    condor_parser.add_argument("--universe", dest="universe", metavar="",
                               help=("Jobs should be submitted using this univese."
                                     " e.g. vanilla"))

    condor_parser.add_argument("--path-prefix", dest="path_prefix", metavar="", default="",
                               help=("The string that should be pre-appended to file path given to backend"
                                     " e.g. root://dcbldoor.sdcc.bnl.gov:1096"))

    condor_parser.add_argument(
        "--global-job-limit",
        dest="global_job_limit",
        metavar="",
        default=default_global_job_limit,
        type=int,
        help=(
            "The number of batch jobs that can be active for the user before the backend class will stop "
            "submitting. This is not a completely hard limit, the actual max reached depends on other "
            "submitting processes and the number submitted before re-checking."
            f" (default: {default_global_job_limit})"))

    condor_parser.add_argument("--submission-check-heartbeat", dest="submission_check_heartbeat", metavar="", type=int,
                               default=default_submission_check_heartbeat,
                               help=("The time (seconds) between checking if there are fewer batch jobs than the global limit. "
                                     "Generally not needed to change, but it certainly shouldn't be set lower than 30 seconds."
                                     f" (default: {default_submission_check_heartbeat})"))

    return [local_parser, lsf_parser, pbs_parser, condor_parser]
