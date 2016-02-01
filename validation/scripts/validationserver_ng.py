import cherrypy
from glob import glob
import json
import json_objects
import os.path
import time
import logging
import queue
from multiprocessing import Process, Queue
from validationplots import create_plots
import functools
from cherrypy.lib.static import serve_file


g_plottingProcesses = {}


def get_json_object_list(results_folder, json_file_name):
    """
    Searches one folder's sub-folder for json files of a
    specific name and returns a combined list of the
    json file's content
    """

    search_string = results_folder + "/*/" + json_file_name

    print("search string " + search_string)
    found_revs = glob(search_string)
    found_rev_labels = []

    for r_file in found_revs:
        # try loading json file
        with open(r_file) as json_file:
            data = json.load(json_file)

            if "label" in data:
                found_rev_labels.append(data["label"])

    return found_rev_labels


def get_revision_list(results_folder):
    """
    Searches one folder's sub-folder for json files of a
    revision and returns a combined list of the
    json file's content
    """
    search_string = results_folder + "/*/revision.json"

    print("search string " + search_string)
    found_revs = glob(search_string)
    found_rev_labels = []

    for r_file in found_revs:
        # try loading json file
        with open(r_file) as json_file:
            data = json.load(json_file)

            if "label" in data:
                found_rev_labels.append(data["label"])

    return found_rev_labels


def deliver_json(file_name):
    """
    Simply load & parse a json file and return the
    python objects
    """

    with open(file_name) as json_file:
        data = json.load(json_file)
    return data


def create_revsion_key(revision_names):
    """
    Create a string key out of a revision list, which is handed to tho browser
    in form of a progress key
    """
    return functools.reduce(lambda x, y: x + "-" + y, revision_names, "")


def check_plotting_status(progress_key):
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


# todo: limit the number of running plotting requests and terminate hanging ones
def start_plotting_request(revision_names):
    """
    Start a new comparison between the supplied revisions
    """

    rev_key = create_revsion_key(revision_names)

    # still running a plotting for this combination ?
    if rev_key in g_plottingProcesses:
        logging.info("Plotting request for {} still running".format(rev_key))
        return rev_key

    # create queue to stream progress, only one directional from parent to child
    qu = Queue()

    # start a new process for creating the plots
    p = Process(target=create_plots, args=(revision_names, False, qu))
    p.start()
    g_plottingProcesses[rev_key] = (p, qu, None)

    logging.info("Started process for plotting request {}".format(rev_key))

    return rev_key


class ValidationRoot(object):

    """
    Root Validation class to handle non-static HTTP requests into the validation server.
    The two main functions are to hand out compiled json objects of revisions and comparisons
    and to start and monitor the creation of comparison plots.
    """

    def __init__(self, results_folder, comparison_folder):
        """
        class initializer, which takes the path to the folders containing the
        validation run results and plots (aka comparison)
        """

        #: folder where the results of one revision run are located
        self.results_folder = results_folder

        #: folder where the comparison plots and json result files are located
        self.comparison_folder = comparison_folder

    @cherrypy.expose
    @cherrypy.tools.json_in()
    @cherrypy.tools.json_out()
    def create_comparison(self):
        """
        Triggers the start of a now comparison between the revisions supplied
        in revision_list
        """
        rev_list = cherrypy.request.json["revision_list"]
        logging.debug('Creating plots for revisions: ' + str(rev_list))
        progress_key = start_plotting_request(rev_list)
        return {"progress_key": progress_key}

    @cherrypy.expose
    @cherrypy.tools.json_in()
    @cherrypy.tools.json_out()
    def create_comparison_status(self):
        """
        Checks on the status of a comparison creation
        """
        progress_key = cherrypy.request.json["input"]
        logging.debug('Checking status for plot creation: ' + str(progress_key))
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
        rev_list = get_json_object_list(self.results_folder, "revision.json")

        combined_list = []

        # always add the reference revision
        combined_list = [json.loads(json_objects.dumps(json_objects.Revision("reference", None, "black")))]

        # load and combine
        for r in rev_list:
            full_path = os.path.join(self.results_folder, r, "revision.json")
            j = deliver_json(full_path)
            combined_list.append(j)

        # Set the most recent one ...
        newest_date = None
        newest_rev = None
        for r in combined_list:
            rdate_str = r["creation_date"]
            if isinstance(rdate_str, str):
                if len(rdate_str) > 0:
                    rdate = time.strptime(rdate_str, "%Y-%m-%d %H:%M:%S")
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
        return the json file of the comparison results of one specific comparison
        """

        # todo: ensure this file is not outside of the webserver
        full_path = os.path.join(self.comparison_folder, comparison_label, "comparison.json")
        return deliver_json(full_path)


def setup_gzip_compression(path, cherry_config):
    """
    enable GZip compression for all text-based content the
    web-server will deliver
    """

    cherry_config[path].update({'tools.gzip.on': True,
                                'tools.gzip.mime_types': ['text/html',
                                                          'text/plain',
                                                          'text/css',
                                                          'application/javascript',
                                                          'application/json']})


def run_server():

    # Setup options for logging
    logging.basicConfig(level=logging.DEBUG,
                        format='%(asctime)s %(levelname)-8s %(message)s',
                        datefmt='%H:%M:%S')

    # Only execute the program if a basf2 release is set up!
    if os.environ.get('BELLE2_RELEASE', None) is None:
        sys.exit('Error: No basf2 release set up!')

    # Make sure the output of validate_basf2.py is there
    if not os.path.isdir('html/results'):
        sys.exit('Error: No html/results '
                 'dir found! Run validate_basf2.py first.')

    # Go to the html directory
    os.chdir('html')

    cherry_config = dict()
    # just empty, will be filled below
    cherry_config["/"] = {}
    # will ensure also the json requests are gzipped
    setup_gzip_compression("/", cherry_config)

    var_b2_local_dir = "BELLE2_LOCAL_DIR"

    if var_b2_local_dir not in os.environ:
        print("{} hast to be set, exiting".format(var_b2_local_dir))
        return

    # get local directory of the belle 2 release
    local_dir = os.environ[var_b2_local_dir]

    # join the paths of the various result folders
    results_folder = os.path.join(local_dir, "results")
    comparison_folder = os.path.join(local_dir, "html", "plots")
    static_folder = os.path.join(local_dir, "validation", "html_ng")

    # export js, css and html templates
    cherry_config["/static"] = {
        'tools.staticdir.on': True,
        # only serve js, css, html and png files
        'tools.staticdir.match': "^.*\.(js|css|html|png)$",
        'tools.staticdir.dir': static_folder}
    setup_gzip_compression("/static", cherry_config)

    # export generated plots
    cherry_config["/plots"] = {
        'tools.staticdir.on': True,
        # only serve json and png files
        'tools.staticdir.match': "^.*\.(png|json|pdf)$",
        'tools.staticdir.dir': comparison_folder}
    setup_gzip_compression("/plots", cherry_config)

    # export generated results and raw root files
    cherry_config["/results"] = {
        'tools.staticdir.on': True,
        'tools.staticdir.dir': results_folder,
        # only serve root files
        'tools.staticdir.match': "^.*\.(log|root)$",
        # server the log files as plain text files
        'tools.staticdir.content_types': {'log': 'text/plain'}}
    setup_gzip_compression("/results", cherry_config)

    cherrypy.quickstart(ValidationRoot(results_folder=results_folder, comparison_folder=comparison_folder), '/', cherry_config)


if __name__ == '__main__':
    run_server()
