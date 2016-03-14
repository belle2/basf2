#!/usr/bin/env python
# -*- encoding: utf-8 -*-

import BaseHTTPServer
import logging as log
import os
import sys
import time
import subprocess
import mimetypes
from urlparse import parse_qs
from cgi import parse_header, parse_multipart
from save import create_image_matrix, merge_multiple_plots

try:
    import simplejson as json
except ImportError:
    import json

from validationplots import create_plots


class Handler(BaseHTTPServer.BaseHTTPRequestHandler):
    """!
    Defines how the BaseHTTPServer handles HTTP Requests (GET or POST)
    """

    def logRequest(s):
        """!
        Writes the message in 's' to the log, which is by default printed to
        stdout

        @param s: The string containing the log message
        @return: None
        """
        log.debug("%s: %s/%s" % (s.client_address, s.command, s.path))

    def do_GET(self):
        """!
        How to deal with a GET-request
        """
        path = self.path

        # Usually 'Content-type' is empty, except for AJAX-requests
        if 'Content-type' in self.headers:
            print self.headers['Content-type']
            contentType = self.headers['Content-type']
        else:
            contentType = 'text/html'

        log.debug("GET: Requesting %s" % path)

        # Either we are dealing with a file request, or an AJAX-request
        if contentType.startswith('text/html'):
            (respCode, respContent, respContentType) = self.do_GET_FILE()
        elif contentType.startswith('application/json'):
            (respCode, respContent, respContentType) = self.do_GET_JSON()

        # Send results back to browser
        self.send_response(respCode)
        self.send_header('Content-type', respContentType)
        self.send_header('Content-length', len(respContent))
        self.end_headers()
        self.wfile.write(respContent)

        print

    def parse_POST(self):
        ctype, pdict = parse_header(self.headers['content-type'])
        if ctype == 'multipart/form-data':
            postvars = parse_multipart(self.rfile, pdict)
        elif ctype == 'application/x-www-form-urlencoded':
            length = int(self.headers['content-length'])
            postvars = parse_qs(self.rfile.read(length), keep_blank_values=1)
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
            log.debug("POST: Content-length %d" % contentLength)
            log.debug("POST: Content: %s" % content)
            data = json.loads(content)

            if 'input' not in data:
                log.debug("POST: Missing input parameter: %s" % content)
                self.send_response(404)
                self.send_header('Content-type', 'application/json')
                self.end_headers()
                self.wfile.write(json.dumps({"error": "Missing input parameter"}))
                return

            (respCode, respContent, respContType) = self.do_GET_JSON(data['input'])

            self.send_response(respCode)
            self.send_header('Content-type', respContType)
            self.send_header('Content-length', len(respContent))
            self.end_headers()

            self.wfile.write(respContent)

            return

        elif (contentType.startswith('multipart/form-data') or
              contentType.startswith('application/x-www-form-urlencoded')) and "matrix_save" in self.path:
            postvars = self.parse_POST()
            images = []
            pakidz = set()
            for element in postvars['matrix']:
                pakidz.add(element.split("__")[0])
                images.append(element.split("__")[1])
            if len(pakidz) > 1 or len(postvars['size']) > 1 or len(postvars['package']) > 1:
                sys.exit("Error: Two or more packages selected in /save_matrix")
            if "pdf" in postvars["type"]:
                try:
                    if merge_multiple_plots("".join(postvars['package']), images):
                        log.debug("POST: Requested plots successfully merged and the pdf saved.")
                        self.send_response(301)
                        self.send_header('Location', './plots/merged_plots.pdf')
                        self.end_headers()
                    else:
                        log.debug("POST: Something went wrong, pdf could not be saved.")
                        sys.exit()
                except ImportError:
                    log.debug("ImportError - the module 'PyPDF2' is missing. \
                              Please install it by running commands 'source activate' and 'pip install PyPDF2'")
                    self.send_response(301)
                    self.send_header('Location', './import_error_pdf.html')
                    self.end_headers()
            else:
                try:
                    if create_image_matrix(images, "".join(postvars['package']), int(max(postvars['size']))):
                        log.debug("POST: Requested plot matrix successfully created and saved.")
                        self.send_response(301)
                        self.send_header('Location', './plots/matrix.png')
                        self.end_headers()
                    else:
                        log.debug("POST: Plot matrix could not be created.")
                        self.wfile.write("Error: Requested plot matrix could not be created.")
                except ImportError:
                    log.debug("ImportError - the module 'pillow' is missing. \
                              Please install it by running commands 'source activate' and 'pip install pillow'")
                    self.send_response(301)
                    self.send_header('Location', './import_error.html')
                    self.end_headers()
            return

        else:
            self.send_response(404)
            self.send_header('Content-type', 'application/json')
            self.end_headers()
            self.wfile.write(json.dumps({"error": "ContentType not supported"}))
            return

    def do_GET_FILE(self):
        """!
        Deals with GET-requests that request a file from the server (i.e.
        serves this file, if it exists)

        @return: Tuple of the returncode of request, the file itself,
            and the file type
        """

        docRoot = './'
        docPath = "%s%s" % (docRoot, self.path)
        errorPath = "%s/404.html" % docRoot

        file_extension = os.path.splitext(self.path)[1]

        try:
            file_type = mimetypes.types_map[file_extension]
        except:
            file_type = 'text/html'

        if self.path.startswith('../'):
            log.debug('GET: "%s" trying to escape from the sandbox' % self.path)
            docFile = open(errorPath, 'r')
            code = 404
        elif docPath.endswith('/') and os.path.exists("%sindex.html" % docPath):
            log.debug('GET: Serving %sindex.html for %s' % (docPath, self.path))
            docFile = open("%sindex.html" % docPath, 'r')
            code = 200
        elif os.path.exists(docPath):
            log.debug('GET: Serving %s' % self.path)
            docFile = open(docPath, 'r')
            code = 200
        else:
            log.debug('GET: No document found for %s' % self.path)
            docFile = open(errorPath, 'r')
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

        # Used to check if a web server is running
        if '/ajax/pingserver' in self.path:
            return (200, json.dumps({}), 'application/json')

        # Used to get a list of all log files
        if '/ajax/listlogs' in self.path:
            # Get a list of all folders in results and sort by mod. date
            folders = ['./results/' + __ for __ in os.listdir('./results')
                       if os.path.isdir('./results/' + __)]
            newest = sorted(folders, key=os.path.getctime)[-1]

            loglist = {}
            for dir in os.listdir(newest):
                if os.path.isdir(newest + '/' + dir):
                    for file in os.listdir(newest + '/' + dir):
                        if file.endswith('.log'):
                            if dir in loglist.keys():
                                loglist[dir].append(file)
                            else:
                                loglist[dir] = [file]
            print json.dumps([newest, loglist])
            return (200, json.dumps([newest, loglist]), 'application/json')

        # Used to generate new plots
        if '/ajax/makeplots' in self.path:
            log.debug('Creating plots for revisions ' + ', '.join(data))
            create_plots(revisions=data)
            return (200, json.dumps({}), 'application/json')


###############################################################################
#   Webserver starts here
###############################################################################


def run(ip='localhost', port=8000):
    """!
    Runs a BaseHTTPServer on the given address
    """
    server_address = (ip, port)
    httpd = BaseHTTPServer.HTTPServer(server_address, Handler, 'root')
    log.info("Server: Waiting for requests... \n")
    httpd.serve_forever()

# Only execute if the file is not imported
if __name__ == '__main__':

    # Only execute the program if a basf2 release is set up!
    if os.environ.get('BELLE2_RELEASE', None) is None:
        sys.exit('Error: No basf2 release set up!')

    # Make sure the output of validate_basf2.py is there
    if not os.path.isdir('html/results'):
        sys.exit('Error: No html/results dir found! Run validate_bash2.py first.')

    # Go to the html directory
    os.chdir('html')

    # Setup options for logging
    log.basicConfig(level=log.DEBUG,
                    format='%(asctime)s %(levelname)-8s %(message)s',
                    datefmt='%H:%M:%S')

    # Define the server address
    ip = 'localhost'
    port = 8000

    # Start the server!
    try:
        log.info("Server: Starting HTTP server on %s:%d" % (ip, port))
        run(ip=ip)
        log.info("Server: Terminating")
    # Terminate upon KeyboardInterrupt
    except KeyboardInterrupt, k:
        print "Server: Terminated by user!"
