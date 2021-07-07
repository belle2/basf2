##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################
# std
from typing import Dict, Any, List, Tuple
from glob import glob
import json
import functools
import time
import datetime
from multiprocessing import Process, Queue
import os.path
import argparse
import logging
import sys
import queue
import webbrowser

# 3rd
import cherrypy

# ours
import json_objects
from validationplots import create_plots
import validationfunctions
import validationpath

g_plottingProcesses: Dict[str, Tuple[Process, Queue, Dict[str, Any]]] = ({})


def get_revision_label_from_json_filename(json_filename: str) -> str:
    """
    Gets the label of a revision from the path to the revision.json file
    for example results/r121/revision.json
    will result in the label r121
    This is useful if the results folder has been moved by the user
    """
    folder_part = os.path.split(json_filename)[0]
    last_folder = os.path.basename(folder_part)

    return last_folder


def get_json_object_list(results_folder: str, json_file_name: str) -> List[str]:
    """
    Searches one folder's sub-folder for json files of a
    specific name and returns a combined list of the
    json file's content
    """

    search_string = results_folder + "/*/" + json_file_name

    found_revs = glob(search_string)
    found_rev_labels = []

    for r_file in found_revs:
        # try loading json file
        with open(r_file) as json_file:
            data = json.load(json_file)  # noqa

            # always use the folder name as label
            found_rev_labels.append(
                get_revision_label_from_json_filename(r_file)
            )

    return found_rev_labels


def deliver_json(file_name: str):
    """
    Simply load & parse a json file and return the
    python objects
    """

    with open(file_name) as json_file:
        data = json.load(json_file)
    return data


def create_revision_key(revision_names: List[str]) -> str:
    """
    Create a string key out of a revision list, which is handed to tho browser
    in form of a progress key
    """
    return functools.reduce(lambda x, y: x + "-" + y, revision_names, "")


def check_plotting_status(progress_key: str):
    """
    Check the plotting status via the supplied progress_key
    """

    if progress_key not in g_plottingProcesses:
        return None

    process, qu, last_status = g_plottingProcesses[progress_key]

    # read latest message
    try:
        # read as much entries from the queue as possible
        while not qu.empty():
            msg = qu.get_nowait()
            last_status = msg

            # update the last status
            g_plottingProcesses[progress_key] = (process, qu, last_status)
    except queue.Empty:
        pass

    return last_status


# todo: remove this, once we're certain that the bug was fixed!
def warn_wrong_directory():
    if not os.getcwd().endswith("html"):
        print(
            f"ERROR: Expected to be in HTML directory, but my current "
            f"working directory is {os.getcwd()}; abspath: {os.getcwd()}."
        )


# todo: limit the number of running plotting requests and terminate hanging ones
def start_plotting_request(
    revision_names: List[str], results_folder: str
) -> str:
    """
    Start a new comparison between the supplied revisions

    Returns:
        revision key
    """

    rev_key = create_revision_key(revision_names)

    # still running a plotting for this combination ?
    if rev_key in g_plottingProcesses:
        logging.info(f"Plotting request for {rev_key} still running")
        return rev_key

    # create queue to stream progress, only one directional from parent to
    # child
    qu = Queue()

    # start a new process for creating the plots
    p = Process(
        target=create_plots,
        args=(
            revision_names,
            False,
            qu,
            # go one folder up, because this function
            # expects the work dir, which contains
            # the results folder
            os.path.dirname(results_folder),
        ),
    )
    p.start()
    g_plottingProcesses[rev_key] = (p, qu, None)

    logging.info(f"Started process for plotting request {rev_key}")

    return rev_key


class ValidationRoot:

    """
    Root Validation class to handle non-static HTTP requests into the
    validation server. The two main functions are to hand out compiled json
    objects of revisions and comparisons and to start and monitor the
    creation of comparison plots.

    """

    def __init__(self, working_folder):
        """
        class initializer, which takes the path to the folders containing the
        validation run results and plots (aka comparison)
        """

        #: html folder that contains plots etc.
        self.working_folder = working_folder

        #: Date when this object was instantiated
        self.last_restart = datetime.datetime.now()

        #: Git version
        self.version = validationfunctions.get_compact_git_hash(
            os.environ["BELLE2_LOCAL_DIR"]
        )

    @cherrypy.expose
    @cherrypy.tools.json_in()
    @cherrypy.tools.json_out()
    def create_comparison(self):
        """
        Triggers the start of a now comparison between the revisions supplied
        in revision_list
        """
        rev_list = cherrypy.request.json["revision_list"]
        logging.debug("Creating plots for revisions: " + str(rev_list))
        progress_key = start_plotting_request(
            rev_list, validationpath.get_results_folder(self.working_folder),
        )
        return {"progress_key": progress_key}

    @cherrypy.expose
    def index(self):
        """
        forward to the static landing page if
        the default url is used (like http://localhost:8080/)
        """
        raise cherrypy.HTTPRedirect("/static/validation.html")

    @cherrypy.expose
    def plots(self, *args):
        """
        Serve file from the html/plot directory.
        :param args: For the request /plots/a/b/c, these will be the strings
            "a", "b", "c"
        """

        warn_wrong_directory()

        if len(args) < 3:
            raise cherrypy.HTTPError(404)

        tag_folder = os.path.relpath(
            validationpath.get_html_plots_tag_comparison_folder(
                self.working_folder, args[:-2]
            ),
            validationpath.get_html_folder(self.working_folder),
        )
        path = os.path.join(tag_folder, *args[-2:])
        return cherrypy.lib.static.serve_file(path)

    @cherrypy.expose
    @cherrypy.tools.json_in()
    @cherrypy.tools.json_out()
    def check_comparison_status(self):
        """
        Checks on the status of a comparison creation
        """
        progress_key = cherrypy.request.json["input"]
        logging.debug("Checking status for plot creation: " + str(progress_key))
        status = check_plotting_status(progress_key)
        return status

    @cherrypy.expose
    @cherrypy.tools.json_out()
    def revisions(self, revision_label=None):
        """
        Return a combined json object with all revisions and
        mark the newest one with the field most_recent=true
        """

        # get list of available revision
        rev_list = get_json_object_list(
            validationpath.get_results_folder(self.working_folder),
            validationpath.file_name_results_json,
        )

        # always add the reference revision
        combined_list = []
        reference_revision = json.loads(
            json_objects.dumps(json_objects.Revision(label="reference"))
        )

        # load and combine
        for r in rev_list:
            full_path = os.path.join(
                validationpath.get_results_folder(self.working_folder),
                r,
                validationpath.file_name_results_json,
            )

            # update label, if dir has been moved
            lbl_folder = get_revision_label_from_json_filename(full_path)
            j = deliver_json(full_path)
            j["label"] = lbl_folder
            combined_list.append(j)

        # Sorting

        # Order by categories (nightly, build, etc.) first, then by date
        # A pure chronological order doesn't make sense, because we do not
        # have a linear history ((pre)releases branch off) and for the builds
        # the date corresponds to the build date, not to the date of the
        # actual commit.
        def sort_key(label: str):
            if "-" not in label:
                logging.warning(
                    f"Misformatted label encountered: '{label}' "
                    f"(doesn't seem to include date?)"
                )
                return label
            category, datetag = label.split("-", maxsplit=1)
            print(category, datetag)
            # Will later reverse order to bring items in the same category
            # in reverse chronological order, so the following list will have
            # the items in reverse order as well:
            order = ["release", "prerelease", "build", "nightly"]
            try:
                index = order.index(category)
            except ValueError:
                index = 9
                logging.warning(
                    f"Misformatted label encountered: '{label}' (doesn't seem "
                    f"to belong to any known category?)"
                )
            return f"{index}-{datetag}"

        combined_list.sort(key=lambda rev: sort_key(rev["label"]), reverse=True)

        # reference always on top
        combined_list = [reference_revision] + combined_list

        # Set the most recent one ...
        newest_date = None
        newest_rev = None
        for r in combined_list:
            rdate_str = r["creation_date"]
            if isinstance(rdate_str, str):
                if len(rdate_str) > 0:
                    try:
                        rdate = time.strptime(rdate_str, "%Y-%m-%d %H:%M")
                    except ValueError:
                        # some old validation results might still contain
                        # seconds and therefore cannot properly be converted
                        rdate = None

                    if rdate is None:
                        continue

                    if newest_date is None:
                        newest_date = rdate
                        newest_rev = r
                    if rdate > newest_date:
                        newest_date = rdate
                        newest_rev = r

        for c in combined_list:
            if c["most_recent"] is not None:
                c["most_recent"] = False

        # if there are no revisions at all, this might also be just None
        if newest_rev:
            newest_rev["most_recent"] = True

        # topmost item must be dictionary for the ractive.os template to match
        return {"revisions": combined_list}

    @cherrypy.expose
    @cherrypy.tools.json_out()
    def comparisons(self, comparison_label=None):
        """
        return the json file of the comparison results of one specific
        comparison
        """

        warn_wrong_directory()

        # todo: Make this independent of our working directory!
        path = os.path.join(
            os.path.relpath(
                validationpath.get_html_plots_tag_comparison_folder(
                    self.working_folder, comparison_label.split(",")
                ),
                validationpath.get_html_folder(self.working_folder),
            ),
            "comparison.json",
        )

        # check if this comparison actually exists
        if not os.path.isfile(path):
            raise cherrypy.HTTPError(
                404, f"Json Comparison file {path} does not exist"
            )

        return deliver_json(path)

    @cherrypy.expose
    @cherrypy.tools.json_out()
    def system_info(self):
        """
        Returns:
            JSON file containing git versions and time of last restart
        """

        warn_wrong_directory()

        # note: for some reason %Z doesn't work like this, so we use
        # time.tzname for the time zone.
        return {
            "last_restart": self.last_restart.strftime("%-d %b %H:%M ")
            + time.tzname[1],
            "version_restart": self.version,
            "version_current": validationfunctions.get_compact_git_hash(
                os.environ["BELLE2_LOCAL_DIR"]
            ),
        }


def setup_gzip_compression(path, cherry_config):
    """
    enable GZip compression for all text-based content the
    web-server will deliver
    """

    cherry_config[path].update(
        {
            "tools.gzip.on": True,
            "tools.gzip.mime_types": [
                "text/html",
                "text/plain",
                "text/css",
                "application/javascript",
                "application/json",
            ],
        }
    )


def get_argument_parser():
    """Prepare a parser for all the known command line arguments"""

    # Set up the command line parser
    parser = argparse.ArgumentParser()

    # Define the accepted command line flags and read them in
    parser.add_argument(
        "-ip",
        "--ip",
        help="The IP address on which the"
        "server starts. Default is '127.0.0.1'.",
        type=str,
        default="127.0.0.1",
    )
    parser.add_argument(
        "-p",
        "--port",
        help="The port number on which"
        " the server starts. Default is '8000'.",
        type=str,
        default=8000,
    )
    parser.add_argument(
        "-v",
        "--view",
        help="Open validation website" " in the system's default browser.",
        action="store_true",
    )
    parser.add_argument(
        "--production",
        help="Run in production environment: "
        "no log/error output via website and no auto-reload",
        action="store_true",
    )

    return parser


def parse_cmd_line_arguments():
    """!
    Sets up a parser for command line arguments,
    parses them and returns the arguments.
    @return: An object containing the parsed command line arguments.
    Arguments are accessed like they are attributes of the object,
    i.e. [name_of_object].[desired_argument]
    """
    parser = get_argument_parser()
    # Return the parsed arguments!
    return parser.parse_args()


def run_server(
    ip="127.0.0.1",
    port=8000,
    parse_command_line=False,
    open_site=False,
    dry_run=False,
):

    # Setup options for logging
    logging.basicConfig(
        level=logging.DEBUG,
        format="%(asctime)s %(levelname)-8s %(message)s",
        datefmt="%H:%M:%S",
    )

    basepath = validationpath.get_basepath()
    cwd_folder = os.getcwd()

    # Only execute the program if a basf2 release is set up!
    if (
        os.environ.get("BELLE2_RELEASE_DIR", None) is None
        and os.environ.get("BELLE2_LOCAL_DIR", None) is None
    ):
        sys.exit("Error: No basf2 release set up!")

    cherry_config = dict()
    # just empty, will be filled below
    cherry_config["/"] = {}
    # will ensure also the json requests are gzipped
    setup_gzip_compression("/", cherry_config)

    # check if static files are provided via central release
    static_folder_list = ["validation", "html_static"]
    static_folder = None

    if basepath["central"] is not None:
        static_folder_central = os.path.join(
            basepath["central"], *static_folder_list
        )
        if os.path.isdir(static_folder_central):
            static_folder = static_folder_central

    # check if there is also a collection of static files in the local release
    # this overwrites the usage of the central release
    if basepath["local"] is not None:
        static_folder_local = os.path.join(
            basepath["local"], *static_folder_list
        )
        if os.path.isdir(static_folder_local):
            static_folder = static_folder_local

    if static_folder is None:
        sys.exit(
            "Either BELLE2_RELEASE_DIR or BELLE2_LOCAL_DIR has to bet "
            "to provide static HTML content. Did you run b2setup ?"
        )

    # join the paths of the various result folders
    results_folder = validationpath.get_results_folder(cwd_folder)
    comparison_folder = validationpath.get_html_plots_folder(cwd_folder)

    logging.info(f"Serving static content from {static_folder}")
    logging.info(f"Serving result content and plots from {cwd_folder}")

    # check if the results folder exists and has at least one folder
    if not os.path.isdir(results_folder):
        sys.exit(
            "Result folder {} does not exist, run validate_basf2 first "
            "to create validation output".format(results_folder)
        )

    results_count = sum(
        [
            os.path.isdir(os.path.join(results_folder, f))
            for f in os.listdir(results_folder)
        ]
    )
    if results_count == 0:
        sys.exit(
            f"Result folder {results_folder} contains no folders, run "
            f"validate_basf2 first to create validation output"
        )

    # Go to the html directory
    if not os.path.exists("html"):
        os.mkdir("html")
    os.chdir("html")

    if not os.path.exists("plots"):
        os.mkdir("plots")

    # export js, css and html templates
    cherry_config["/static"] = {
        "tools.staticdir.on": True,
        # only serve js, css, html and png files
        "tools.staticdir.match": r"^.*\.(js|css|html|png|js.map)$",
        "tools.staticdir.dir": static_folder,
    }
    setup_gzip_compression("/static", cherry_config)

    # export generated plots
    cherry_config["/plots"] = {
        "tools.staticdir.on": True,
        # only serve json and png files
        "tools.staticdir.match": r"^.*\.(png|json|pdf)$",
        "tools.staticdir.dir": comparison_folder,
    }
    setup_gzip_compression("/plots", cherry_config)

    # export generated results and raw root files
    cherry_config["/results"] = {
        "tools.staticdir.on": True,
        "tools.staticdir.dir": results_folder,
        # only serve root files
        "tools.staticdir.match": r"^.*\.(log|root)$",
        # server the log files as plain text files, and make sure to use
        # utf-8 encoding. Firefox might decide different, if the files
        # are located on a .jp domain and use Shift_JIS
        "tools.staticdir.content_types": {
            "log": "text/plain; charset=utf-8",
            "root": "application/octet-stream",
        },
    }

    setup_gzip_compression("/results", cherry_config)

    # Define the server address and port
    # only if we got some specific
    production_env = False
    if parse_command_line:
        # Parse command line arguments
        cmd_arguments = parse_cmd_line_arguments()

        ip = cmd_arguments.ip
        port = int(cmd_arguments.port)
        open_site = cmd_arguments.view
        production_env = cmd_arguments.production

    cherrypy.config.update(
        {"server.socket_host": ip, "server.socket_port": port, }
    )
    if production_env:
        cherrypy.config.update({"environment": "production"})

    logging.info(f"Server: Starting HTTP server on {ip}:{port}")

    if open_site:
        webbrowser.open("http://" + ip + ":" + str(port))

    if not dry_run:
        cherrypy.quickstart(
            ValidationRoot(working_folder=cwd_folder), "/", cherry_config
        )


if __name__ == "__main__":
    run_server()
