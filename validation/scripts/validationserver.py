#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import argparse
import http.server
import logging as log
import os
import sys
import traceback
import mimetypes
import webbrowser
from urllib.parse import parse_qs
from cgi import parse_header, parse_multipart
from save import create_image_matrix, merge_multiple_plots
from multiprocessing import Process, Queue
import queue
from urllib.parse import urlparse
import time
from functools import reduce

try:
    import simplejson as json
except ImportError:
    import json

from validationplots import create_plots


def get_error():
    """!
    In case of an error, return some information for debugging purposes.
    """

    respCode = 500
    respContent = '<h1>There was a problem</h1>'
    respContent += '<b>I don\'t really know what went wrong, but here is the '\
                   'Exception message and the Python call stack. Good luck! ' \
                   ';-) </b> <br> <hr> <br>'
    respContent += '<small><pre>'
    respContent += traceback.format_exc() + '</pre><br><hr><br>\n\n'
    respContent += '<pre>'
    respContent += '\n'.join(traceback.format_stack())
    respContent += '</pre>'
    respContentType = 'text/html'

    log.error('An error occurred. This is the traceback of the exception: \n' +
              traceback.format_exc())

    return respCode, respContent, respContentType


def parse_cmd_line_arguments():
    """!
    Sets up a parser for command line arguments,
    parses them and returns the arguments.
    @return: An object containing the parsed command line arguments.
    Arguments are accessed like they are attributes of the object,
    i.e. [name_of_object].[desired_argument]
    """

    # Set up the command line parser
    parser = argparse.ArgumentParser()

    # Define the accepted command line flags and read them in
    parser.add_argument("-ip", "--ip", help="The IP address on which the"
                        "server starts. Default is 'localhost'.",
                        type=str, default='localhost')
    parser.add_argument("-p", "--port", help="The port number on which"
                        " the server starts. Default is '8000'.",
                        type=str, default=8000)
    parser.add_argument("-v", "--view", help="Open validation website"
                        " in the system's default browser.",
                        action='store_true')
    # Return the parsed arguments!
    return parser.parse_args()


g_plottingProcesses = {}


def create_revsion_key(revision_names):
    return reduce(lambda x, y: x + "-" + y, revision_names, "")


def check_plotting_status(progress_key):
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


def start_plotting_request(revision_names):
    rev_key = create_revsion_key(revision_names)

    # still running a plotting for this combination ?
    if rev_key in g_plottingProcesses:
        return rev_key

    # create queue to stream progress, only one directional from parent to child
    qu = Queue()

    # start a new process for creating the plots
    p = Process(target=create_plots, args=(revision_names, False, qu))
    p.start()
    g_plottingProcesses[rev_key] = (p, qu, None)

    return rev_key


class Handler(http.server.BaseHTTPRequestHandler):

    """!
    Defines how the BaseHTTPServer handles HTTP Requests (GET or POST)
    @var path: The path of the request, e.g. the file that is being requested
    """

    def logRequest(self):
        """!
        Writes the message in 's' to the log, which is by default printed to
        stdout

        @param s: The string containing the log message
        @return: None
        """
        log.debug("%s: %s/%s" % (self.client_address, self.command, self.path))

    def do_GET(self):
        """!
        How to deal with a GET-request
        """

        # Usually 'Content-type' is empty, except for AJAX-requests
        if 'Content-type' in self.headers:
            contentType = self.headers['Content-type']
        else:
            contentType = 'text/html'

        log.debug("GET: Requesting %s" % self.path)

        # Set defaults
        respCode, respContent, respContentType = None, None, None

        # Either we are dealing with a file request, or an AJAX-request
        if contentType.startswith('text/html'):
            try:
                (respCode, respContent, respContentType) = self.do_GET_FILE()
            except Exception:
                (respCode, respContent, respContentType) = get_error()
        elif contentType.startswith('application/json'):
            try:
                (respCode, respContent, respContentType) = self.do_GET_JSON()
            except Exception:
                (respCode, respContent, respContentType) = get_error()

        # Send results back to browser
        self.send_response(respCode)
        self.send_header('Content-type', respContentType)
        self.send_header('Content-length', len(respContent))
        self.end_headers()
        if isinstance(respContent, str):
            respContent = respContent.encode()
        self.wfile.write(respContent)

    def parse_POST(self):
        """!
        A method that parses header sent to the server and returns
        POST method html form data.
        """

        ctype, pdict = parse_header(self.headers['content-type'])
        if ctype == 'multipart/form-data':
            postvars = parse_multipart(self.rfile, pdict)
        elif ctype == 'application/x-www-form-urlencoded':
            length = int(self.headers['content-length'])
            # explicit decode as utf-8 as python3 will provide only a byte-string
            content_str = self.rfile.read(length).decode('utf-8')
            postvars = parse_qs(content_str, keep_blank_values=1)
        else:
            postvars = {}
        return postvars

    def do_POST(self):
        """!
        How to deal with a POST-request
        """

        path = self.path
        contentType = self.headers['Content-type']

        log.debug("POST: Path %s" % path)

        if contentType.startswith('application/json'):
            contentLength = int(self.headers['Content-length'])
            content = self.rfile.read(contentLength)
            # explicit decode as utf-8 as python3 will provide only a byte-string
            content_decoded = content.decode('utf-8')
            log.debug("POST: Content-length %d" % contentLength)
            log.debug("POST: Content: %s" % content_decoded)
            data = json.loads(content_decoded)

            if 'input' not in data:
                log.debug("POST: Missing input parameter: %s" % content_decoded)
                self.send_response(404)
                self.send_header('Content-type', 'application/json')
                self.end_headers()
                self.wfile.write(json.dumps({"error": "Missing input parameter"}).encode())
                return

            try:
                (respCode,
                 respContent,
                 respContType) = self.do_GET_JSON(data['input'])
            except Exception:
                (respCode, respContent, respContType) = get_error()

            self.send_response(respCode)
            self.send_header('Content-type', respContType)
            self.send_header('Content-length', len(respContent))
            self.end_headers()

            if isinstance(respContent, str):
                respContent = respContent.encode()
            self.wfile.write(respContent)

            return

        elif ((contentType.startswith('multipart/form-data') or
               contentType.startswith('application/x-www-form-urlencoded')) and
              "matrix_save" in self.path):
            postvars = self.parse_POST()
            images = []
            pakidz = set()
            for element in postvars['matrix']:
                pakidz.add(element.split("__")[0])
                images.append(element.split("__")[1])
            if (len(pakidz) > 1 or
                    len(postvars['size']) > 1 or
                    len(postvars['package']) > 1):
                sys.exit("Error: Two or more packages"
                         " selected in /save_matrix")
            if "pdf" in postvars["type"]:
                try:
                    if merge_multiple_plots("".join(postvars['package']),
                                            images):
                        log.debug("POST: Requested plots "
                                  "successfully merged and the pdf saved.")
                        self.send_response(301)
                        self.send_header('Location',
                                         './plots/merged_plots.pdf')
                        self.end_headers()
                    else:
                        log.debug("POST: Something went wrong, pdf could not"
                                  " be saved.")
                        sys.exit()
                except ImportError:
                    log.debug("ImportError - the module 'PyPDF2' is missing."
                              "Please install it by running commands "
                              "'source activate' and 'pip install PyPDF2'")
                    self.send_response(301)
                    self.send_header('Location', './import_error_pdf.html')
                    self.end_headers()
            else:
                try:
                    if create_image_matrix(images,
                                           "".join(postvars['package']),
                                           int(max(postvars['size']))):
                        log.debug("POST: Requested plot matrix"
                                  " successfully created and saved.")
                        self.send_response(301)
                        self.send_header('Location', './plots/matrix.png')
                        self.end_headers()
                    else:
                        log.debug("POST: Plot matrix could not be created.")
                        self.wfile.write("Error: Requested plot matrix "
                                         "could not be created.".encode())
                except ImportError:
                    log.debug("ImportError - the module 'pillow' is missing. "
                              "Please install it by running commands"
                              " 'source activate' and 'pip install pillow'")
                    self.send_response(301)
                    self.send_header('Location', './import_error.html')
                    self.end_headers()
            return

        else:
            self.send_response(404)
            self.send_header('Content-type', 'application/json')
            self.end_headers()
            self.wfile.write(json.dumps({"error": "ContentType not supported"}).encode())
            return

    def do_GET_FILE(self):
        """!
        Deals with GET-requests that request a file from the server (i.e.
        serves this file, if it exists)

        @return: Tuple of the returncode of request, the file itself,
            and the file type
        """

        if '?' in self.path:
            self.path = self.path.split('?')[0]
            if self.path.endswith('/'):
                self.path = self.path[:-1]

        docRoot = './'
        docPath = "%s%s" % (docRoot, self.path)
        errorPath = "%s/404.html" % docRoot

        file_extension = os.path.splitext(self.path)[1]

        try:
            file_type = mimetypes.types_map[file_extension]
        except:
            file_type = 'text/html'

        if self.path.startswith('../'):
            log.debug('GET: "%s" trying to escape from the sandbox' % self
                      .path)
            docFile = open(errorPath, 'rb')
            code = 404
        elif docPath.endswith('/') and os.path.exists("%sindex.html" %
                                                      docPath):
            log.debug('GET: Serving %sindex.html for %s' % (docPath,
                                                            self.path))
            docFile = open("%sindex.html" % docPath, 'rb')
            code = 200
        elif os.path.exists(docPath):
            log.debug('GET: Serving %s' % self.path)
            docFile = open(docPath, 'rb')
            code = 200
        else:
            log.debug('GET: No document found for %s' % self.path)
            docFile = open(errorPath, 'rb')
            code = 404

        doc = docFile.read()
        docFile.close()

        return code, doc, file_type

    def do_GET_JSON(self, data=None):
        """!
        Deals with GET-requests that want some kind of data as an answer.
        The answer will be returned in JSON-format, hence the name.

        @param data: Information specifying the request, i.e. what kind of
            data we want as an answer
        @return: Tuple of returncode, the requested date, and the type
            (usually 'application/json')
        """

        # parse the ajax command name which was requested
        access_path = urlparse(self.path)
        path_items = access_path.path.split("/")
        if "ajax" not in path_items:
            return 404, json.dumps({}), 'application/json'

        # make sure there is at least one path item after the ajax one
        ajax_location = path_items.index("ajax")
        if (len(path_items) - 1) == ajax_location:
            return 404, json.dumps({}), 'application/json'

        ajax_command = path_items[ajax_location + 1]
        print("got ajax command" + ajax_command)

        # Used to check if a web server is running
        if 'pingserver' == ajax_command:
            return 200, json.dumps({}), 'application/json'

        # Used to get a list of all log files
        if 'listlogs' == ajax_command:

            # In this list we will store the HTML code that will be returned
            # to the requesting JavaScript application
            html = []

            # Get a list of all folders in results and sort by mod. date
            folders = ['./results/' + __ for __ in os.listdir('./results')
                       if os.path.isdir('./results/' + __)]

            # The folder containing the data from
            # the most recent validation run
            newest = sorted(folders, key=os.path.getctime)[-1]

            # Read in the failed scripts
            failedscripts_path = (newest +
                                  "/__general__/list_of_failed_scripts.log")
            if os.path.exists(failedscripts_path):
                # If a list of failed scripts exists, read it in
                with open(failedscripts_path, 'r') as f:
                    failed_scripts = f.readlines()
                    failed_scripts = [line.strip()[:-3] for line in
                                      failed_scripts]
            else:
                # Otherwise use an empty list
                failed_scripts = []

            # Read in the skipped scripts
            skippedscripts_path = (newest +
                                   "/__general__/list_of_skipped_scripts.log")
            if os.path.exists(skippedscripts_path):
                # If a list of skipped scripts exists, read it in
                with open(skippedscripts_path, 'r') as f:
                    skipped_scripts = f.readlines()
                    skipped_scripts = [line.strip()[:-3] for line in
                                       skipped_scripts]
            else:
                # Otherwise use an empty list
                skipped_scripts = []

            # Add some general information to the HTML
            html.append('<strong>General information:</strong><br>')
            html.append('Execution failed for '
                        '<strong style="color:red">{0}</strong> scripts!'
                        '<br>'.format(len(failed_scripts)))
            html.append('Execution skipped '
                        '<strong style="color:orange">{0}</strong>'
                        ' scripts!<br>'.format(len(skipped_scripts)))
            html.append('<br>')

            # A dict of all logfiles.
            # Structure: {'package':[list, of, logfiles, for, pkg]}
            logfiles = {}
            for _ in os.listdir(newest):
                if os.path.isdir(newest + '/' + _):
                    logfiles[_] = []
            for pkg in logfiles:
                logfiles[pkg] = [newest + '/' + pkg +
                                 '/' + _ for _ in os.listdir(newest + '/' +
                                                             pkg)
                                 if _.endswith('.log')]

            # Parse the dict into HTML
            for pkg in sorted(logfiles.keys()):
                html.append('<strong>{0}</strong><br>'.format(pkg))
                for logfile in sorted(logfiles[pkg]):

                    # Make the variable names more intuitively understandable
                    filename = os.path.basename(logfile)
                    filepath = logfile

                    if filename[:-7] in failed_scripts:
                        html.append('<span class="loglink" name="{0}" '
                                    'style="color: red">&bull;&nbsp;{1}'
                                    '</span></br>'.format(filepath, filename))
                    elif filename[:-7] in skipped_scripts:
                        html.append('<span class="loglink" name="{0}" '
                                    'style="color:orange">&bull;&nbsp;{1}'
                                    '</span></br>'.format(filepath, filename))

                    else:
                        # Add the entry to the html list
                        html.append('<span class="loglink" name="{0}">'
                                    '<span style="color:green">&bull;</span>'
                                    ' {1}</span></br>'
                                    .format(filepath, filename))
                html.append('<br>')

            return 200, json.dumps('\n'.join(html)), 'application/json'

        # Get the number of scripts that had issues
        if 'scriptcount' == ajax_command:

            # Get a list of all folders in results and sort by mod. date
            folders = ['./results/' + __ for __ in os.listdir('./results')
                       if os.path.isdir('./results/' + __)]

            # The folder containing the data from
            # the most recent validation run
            newest = sorted(folders, key=os.path.getctime)[-1]

            # Get the number of failed/skipped scripts
            failpth = newest + '/__general__/list_of_failed_scripts.log'
            skippth = newest + '/__general__/list_of_skipped_scripts.log'
            number_of_failed_scripts = sum(1 for line in open(failpth))
            number_of_skipped_scripts = sum(1 for line in open(skippth))

            # Turn that into the result we want to deliver to the website
            if number_of_failed_scripts + number_of_skipped_scripts == 0:
                result = ''
            else:
                result = '{0}+{1}'.format(number_of_failed_scripts,
                                          number_of_skipped_scripts)

            # Answer the AJAX request
            return 200, json.dumps(result), 'application/json'

        # Used to generate new plots
        if 'makeplots' == ajax_command:
            log.debug('Creating plots for revisions: ' + str(data))
            progress_key = start_plotting_request(data)

            return 200, json.dumps({"progress_key": progress_key}), 'application/json'

        # Used to check for the status of plot creating
        if 'makeplots-status' in self.path:
            log.debug('Checking status for plot creation: ' + str(data))
            status = check_plotting_status(data["progress_key"])
            return 200, json.dumps(status), 'application/json'


###############################################################################
#   Webserver starts here
###############################################################################


def run(ip='localhost', port=8000):
    """!
    Runs a BaseHTTPServer on the given address
    """
    server_address = (ip, port)
    httpd = http.server.HTTPServer(server_address, Handler, 'root')
    log.info("Server: Waiting for requests... \n")
    httpd.serve_forever()


def configure_and_run(ip='localhost', port=8000, parseCommandLine=False, openSite=False):
    """!
    Checks for correct basf2 setup, parses the command line and opens a http server
    on the indicated ip and port.
    """
    # Only execute the program if a basf2 release is set up!
    if os.environ.get('BELLE2_RELEASE', None) is None:
        sys.exit('Error: No basf2 release set up!')

    # Make sure the output of validate_basf2.py is there
    if not os.path.isdir('html/results'):
        sys.exit('Error: No html/results '
                 'dir found! Run validate_basf2.py first.')

    # Go to the html directory
    os.chdir('html')

    # Setup options for logging
    log.basicConfig(level=log.DEBUG,
                    format='%(asctime)s %(levelname)-8s %(message)s',
                    datefmt='%H:%M:%S')

    # Define the server address and port
    # only if we got some specific
    if parseCommandLine:
        # Parse command line arguments
        cmd_arguments = parse_cmd_line_arguments()

        ip = cmd_arguments.ip
        port = int(cmd_arguments.port)
        openSite = cmd_arguments.view

    # Start the server!
    try:
        log.info("Server: Starting HTTP server on {0}:{1}".format(ip, port))

        if openSite:
            webbrowser.open("http://" + ip + ":" + str(port))
        run(ip=ip, port=port)
        log.info("Server: Terminating")
    # Terminate upon KeyboardInterrupt
    except KeyboardInterrupt as k:
        print("Server: Terminated by user!")
