#!/usr/bin/env python3

# Import timeit module and start a timer. Allows to get the runtime of the
# program at any given point
import timeit

g_start_time = timeit.default_timer()  # noqa

# std
import argparse
import glob
import os
import subprocess
import sys
import time
from typing import Dict, Optional, List, Union
import logging

# 3rd party
import ROOT

# ours
import validationpath

###############################################################################
#                           Function definitions                              #
###############################################################################


def get_timezone() -> str:
    """
    Returns the correct timezone as short string
    """
    tz_tuple = time.tzname

    # in some timezones, there is a daylight saving times entry in the
    # second item of the tuple
    if time.daylight != 0:
        return tz_tuple[1]
    else:
        return tz_tuple[0]


def get_compact_git_hash(repo_folder: str) -> Optional[str]:
    """
    Returns the compact git hash from a folder inside of a git repository
    """
    try:
        cmd_output = (
            subprocess.check_output(
                ["git", "show", "--oneline", "-s"], cwd=repo_folder
            )
            .decode()
            .rstrip()
        )
        # the first word in this string will be the hash
        cmd_output = cmd_output.split(" ")
        if len(cmd_output) > 1:
            return cmd_output[0]
        else:
            # something went wrong
            return
    except subprocess.CalledProcessError:
        return


def basf2_command_builder(
    steering_file: str, parameters: List[str], use_multi_processing=False
) -> List[str]:
    """
    This utility function takes the steering file name and other basf2
    parameters and returns a list which can be executed via the OS shell for
    example to subprocess.Popen(params ...) If use_multi_processing is True,
    the script will be executed in multi-processing mode with only 1
    parallel process in order to test if the code also performs as expected
    in multi-processing mode
    """
    cmd_params = ["basf2"]
    if use_multi_processing:
        cmd_params += ["-p1"]
    cmd_params += [steering_file]
    cmd_params += parameters

    return cmd_params


def available_revisions(work_folder: str) -> List[str]:
    """
    Loops over the results folder and looks for revisions. It then returns an
    ordered list, with the most recent revision being the first element in the
    list and the oldest revision being the last element.
    The 'age' of a revision is determined by the 'Last-modified'-timestamp of
    the corresponding folder.
    :return: A list of all revisions available for plotting
    """

    # Get all folders in ./results/ sorted descending by the date they were
    # created (i.e. newest folder first)
    search_folder = validationpath.get_results_folder(work_folder)
    subfolders = [p for p in os.scandir(search_folder) if p.is_dir()]
    revisions = [
        p.name for p in sorted(subfolders, key=lambda p: p.stat().st_mtime)
    ]
    return revisions


def get_start_time() -> float:
    """!
    The function returns the value g_start_time which contain the start time
    of the validation and is set just a few lines above.

    @return: Time since the validation has been started
    """
    return g_start_time


def get_validation_folders(
    location: str, basepaths: Dict[str, str], log: logging.Logger
) -> Dict[str, str]:
    """!
    Collects the validation folders for all packages from the stated release
    directory (either local or central). Returns a dict with the following
    form:
    {'name of package':'absolute path to validation folder of package'}

    @param location: The location where we want to search for validation
        folders (either 'local' or 'central')
    """

    # Make sure we only look in existing locations:
    if location not in ["local", "central"]:
        return {}
    if basepaths[location] is None:
        return {}

    # Write to log what we are collecting
    log.debug(f"Collecting {location} folders")

    # Reserve some memory for our results
    results = {}

    # Now start collecting the folders.
    # First, collect the general validation folders, because it needs special
    # treatment (does not belong to any other package but may include
    # steering files):
    if os.path.isdir(basepaths[location] + "/validation"):
        results["validation"] = basepaths[location] + "/validation"

    # get the special folder containing the validation tests
    if os.path.isdir(basepaths[location] + "/validation/validation-test"):
        results["validation-test"] = (
            basepaths[location] + "/validation/validation-test"
        )

    # Now get a list of all folders with name 'validation' which are
    # subfolders of a folder (=package) in the release directory
    package_dirs = glob.glob(
        os.path.join(basepaths[location], "*", "validation")
    )

    # Now loop over all these folders, find the name of the package they belong
    # to and append them to our results dictionary
    for package_dir in package_dirs:
        package_name = os.path.basename(os.path.dirname(package_dir))
        results[package_name] = package_dir

    # Return our results
    return results


def get_argument_parser(
    modes: Optional[List[str]] = None,
) -> argparse.ArgumentParser:

    if not modes:
        modes = ["local"]

    # Set up the command line parser
    parser = argparse.ArgumentParser()

    # Define the accepted command line flags and read them in
    parser.add_argument(
        "-d",
        "--dry",
        help="Perform a dry run, i.e. run the validation module without "
        "actually executing the steering files (for debugging purposes).",
        action="store_true",
    )
    parser.add_argument(
        "-m",
        "--mode",
        help="The mode which will be used for running the validation. "
        "Possible values: " + ", ".join(modes) + ". Default is 'local'",
        choices=modes,
        type=str,
        default="local",
    )
    parser.add_argument(
        "-i",
        "--intervals",
        help="Comma seperated list of intervals for which to execute the "
        "validation scripts. Default is 'nightly'",
        type=str,
        default="nightly",
    )
    parser.add_argument(
        "-o",
        "--options",
        help="One or more strings that will be passed to basf2 as arguments. "
        "Example: '-n 100'. Quotes are necessary!",
        type=str,
        nargs="+",
    )
    parser.add_argument(
        "-p",
        "--parallel",
        help="The maximum number of parallel processes to run the "
        "validation. Only used for local execution. Default is number "
        "of CPU cores.",
        type=int,
        default=None,
    )
    parser.add_argument(
        "-pkg",
        "--packages",
        help="The name(s) of one or multiple packages. Validation will be "
        "run only on these packages! E.g. -pkg analysis arich",
        type=str,
        nargs="+",
    )
    parser.add_argument(
        "-s",
        "--select",
        help="The file name(s) of one or more space separated validation "
        "scripts that should be executed exclusively. All dependent "
        "scripts will also be executed. E.g. -s ECL2D.C",
        type=str,
        nargs="+",
    )
    parser.add_argument(
        "-si",
        "--select-ignore-dependencies",
        help="The file name of one or more space separated validation "
        "scripts that should be executed exclusively. This will ignore "
        "all dependencies. This is useful if you modified a script that "
        "produces plots based on the output of its dependencies.",
        type=str,
        nargs="+",
    )
    parser.add_argument(
        "--send-mails",
        help="Send email to the contact persons who have failed comparison "
        "plots. Mail is sent from b2soft@mail.desy.de via "
        "/usr/sbin/sendmail.",
        action="store_true",
    )
    parser.add_argument(
        "--send-mails-mode",
        help="How to send mails: Full report, incremental report (new/changed "
        "warnings/failures only) or automatic (default; follow hard coded "
        "rule, e.g. full reports every Monday).",
        choices=["full", "incremental", "automatic"],
        default="automatic",
    )
    parser.add_argument(
        "-q", "--quiet", help="Suppress the progress bar", action="store_true"
    )
    parser.add_argument(
        "-t",
        "--tag",
        help="The name that will be used for the current revision in the "
        "results folder. Default is 'current'.",
        type=str,
        default="current",
    )
    parser.add_argument(
        "--test",
        help="Execute validation in testing mode where only the validation "
        "scripts contained in the validation package are executed. "
        "During regular validation, these scripts are ignored.",
        action="store_true",
    )
    parser.add_argument(
        "--use-cache",
        help="If validation scripts are marked as cacheable and their output "
        "files already exist, don't execute these scripts again",
        action="store_true",
    )
    parser.add_argument(
        "--view",
        help="Once the validation is finished, start the local web server and "
        "display the validation results in the system's default browser.",
        action="store_true",
    )
    parser.add_argument(
        "--max-run-time",
        help="By default, running scripts (that is, steering files executed by"
        "the validation framework) are terminated after a "
        "certain time. Use this flag to change this setting by supplying "
        "the maximal run time in minutes. Value <=0 disables the run "
        "time upper limit entirely.",
        type=int,
        default=None,
    )

    return parser


def parse_cmd_line_arguments(
    modes: Optional[List[str]] = None,
) -> argparse.Namespace:
    """!
    Sets up a parser for command line arguments, parses them and returns the
    arguments.
    @return: An object containing the parsed command line arguments.
        Arguments are accessed like they are attributes of the object,
        i.e. [name_of_object].[desired_argument]
    """

    if not modes:
        modes = ["local"]

    # Return the parsed arguments!
    return get_argument_parser(modes).parse_args()


def scripts_in_dir(dirpath: str, log: logging.Logger, ext="*") -> List[str]:
    """!
    Returns all the files in the given dir (and its subdirs) that have
    the extension 'ext', if an extension is given (default: all extensions)

    @param dirpath: The directory in which we are looking for files
    @param log: logging.Logger object
    @param ext: The extension of the files, which we are looking for.
        '*' is the wildcard-operator (=all extensions are accepted)
    @return: A sorted list of all files with the specified extension in the
        given directory.
    """

    # Write to log what we are collecting
    log.debug(f"Collecting *{ext} files from {dirpath}")

    # Some space where we store our results before returning them
    results = []

    # A list of all folder names that will be ignored (e.g. folders that are
    # important for SCons
    blacklist = [
        "tools",
        "scripts",
        "examples",
        validationpath.folder_name_html_static,
    ]

    # Loop over the given directory and its subdirectories and find all files
    for root, dirs, files in os.walk(dirpath):

        # Skip a directory if it is blacklisted
        if os.path.basename(root) in blacklist:
            continue

        # Loop over all files
        for current_file in files:
            # If the file has the requested extension, append its full paths to
            # the results
            if current_file.endswith(ext):
                results.append(os.path.join(root, current_file))

    # Return our sorted results
    return sorted(results)


def strip_ext(path: str) -> str:
    """
    Takes a path and returns only the name of the file, without the
    extension on the file name
    """
    return os.path.splitext(os.path.split(path)[1])[0]


def get_style(index: Optional[int], overall_item_count=1):
    """
    Takes an index and returns the corresponding line attributes,
    i.e. LineColor, LineWidth and LineStyle.
    """

    # Define the colors for the plot
    colors = [
        ROOT.kRed,
        ROOT.kOrange,
        ROOT.kPink + 9,
        ROOT.kOrange - 8,
        ROOT.kGreen + 2,
        ROOT.kCyan + 2,
        ROOT.kBlue + 1,
        ROOT.kRed + 2,
        ROOT.kOrange + 3,
        ROOT.kYellow + 2,
        ROOT.kSpring,
    ]

    # Define the linestyles for the plot
    linestyles = {
        "dashed": 2,  # Dashed: - - - - -
        "solid": 1,  # Solid: ----------
        "dashdot": 10,
    }  # Dash-dot: -?-?-?-
    ls_index = {0: "dashed", 1: "solid", 2: "dashdot"}

    # Define the linewidth for the plots
    linewidth = 2

    # make sure the index is set
    if not index:
        index = 0

    # Get the color for the (index)th revisions
    color = colors[index % len(colors)]

    # Figure out the linestyle
    # If there is only one revision, make it solid!
    # It cannot overlap with any other line
    if overall_item_count == 1:
        linestyle = linestyles["solid"]
    # Otherwise make sure the newest revision (which is drawn on top) gets a
    # dashed linestyle
    else:
        linestyle = linestyles[ls_index[index % len(ls_index)]]

    return ROOT.TAttLine(color, linestyle, linewidth)


def index_from_revision(revision: str, work_folder: str) -> Optional[int]:
    """
    Takes the name of a revision and returns the corresponding index. Indices
    are used to ensure that the color and style of a revision in a plot are
    always the same, regardless of the displayed revisions.
    Example: release-X is always red, and no other release get drawn in red if
    release-X is not selected for display.
    :param revision: A string containing the name of a revision
    :param work_folder: The work folder containing the results and plots
    :return: The index of the requested revision, or None, if no index could
        be found for 'revision'
    """

    revisions = available_revisions(work_folder) + ["reference"]

    if revision in revisions:
        return revisions.index(revision)
    else:
        return None


def get_log_file_paths(logger: logging.Logger) -> List[str]:
    """
    Returns list of paths that the FileHandlers of logger write to.
    :param logger: logging.logger object.
    :return: List of paths
    """
    ret = []
    for handler in logger.handlers:
        try:
            ret.append(handler.baseFilename)
        except AttributeError:
            pass
    return ret


def get_terminal_width() -> int:
    """
    Returns width of terminal in characters, or 80 if unknown.

    Copied from basf2 utils. However, we only compile the validation package
    on b2master, so copy this here.
    """
    from shutil import get_terminal_size

    return get_terminal_size(fallback=(80, 24)).columns


def congratulator(
    success: Optional[Union[int, float]] = None,
    failure: Optional[Union[int, float]] = None,
    total: Optional[Union[int, float]] = None,
    just_comment=False,
    rate_name="Success rate",
) -> str:
    """ Keeping the morale up by commenting on success rates.

    Args:
        success: Number of successes
        failure: Number of failures
        total: success + failures (out of success, failure and total, exactly
            2 have to be spefified. If you want to use your own figure of
            merit, just set total = 1. and set success to a number between 0.0
            (infernal) to 1.0 (stellar))
        just_comment: Do not add calculated percentage to return string.
        rate_name: How to refer to the calculated success rate.

    Returns:
        Comment on your success rate (str).
    """

    n_nones = [success, failure, total].count(None)

    if n_nones == 0 and total != success + failure:
        print(
            "ERROR (congratulator): Specify 2 of the arguments 'success',"
            "'failure', 'total'.",
            file=sys.stderr,
        )
        return ""
    elif n_nones >= 2:
        print(
            "ERROR (congratulator): Specify 2 of the arguments 'success',"
            "'failure', 'total'.",
            file=sys.stderr,
        )
        return ""
    else:
        if total is None:
            total = success + failure
        if failure is None:
            failure = total - success
        if success is None:
            success = total - failure

    # Beware of zero division errors.
    if total == 0:
        return "That wasn't really exciting, was it?"

    success_rate = 100 * success / total

    comments = {
        00.0: "You're grounded!",
        10.0: "Infernal...",
        20.0: "That's terrible!",
        40.0: "You can do better than that.",
        50.0: "That still requires some work.",
        75.0: "Three quarters! Almost there!",
        80.0: "Way to go ;)",
        90.0: "Gold medal!",
        95.0: "Legendary!",
        99.0: "Nobel price!",
        99.9: "Godlike!",
    }

    for value in sorted(comments.keys(), reverse=True):
        if success_rate >= value:
            comment = comments[value]
            break
    else:
        # below minimum?
        comment = comments[0]

    if just_comment:
        return comment
    else:
        return "{} {}%. {}".format(rate_name, int(success_rate), comment)


def terminal_title_line(title="", subtitle="", level=0) -> str:
    """ Print a title line in the terminal.

    Args:
        title (str): The title. If no title is given, only a separating line
            is printed.
        subtitle (str): Subtitle.
        level (int): The lower, the more dominantly the line will be styled.
    """
    linewidth = get_terminal_width()

    # using the markdown title underlining chars for lack of better
    # alternatives
    char_dict = {0: "=", 1: "-", 2: "~"}

    for key in sorted(char_dict.keys(), reverse=True):
        if level >= key:
            char = char_dict[key]
            break
    else:
        # below minimum, shouldn't happen but anyway
        char = char_dict[0]

    line = char * linewidth
    if not title:
        return line

    # guess we could make a bit more effort with indenting/handling long titles
    # capitalization etc., but for now:
    ret = line + "\n"
    ret += title.capitalize() + "\n"
    if subtitle:
        ret += subtitle + "\n"
    ret += line
    return ret
