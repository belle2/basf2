#!/usr/bin/env python
# -*- encoding: utf-8 -*-

import BaseHTTPServer
import logging as log
import os
import sys
import subprocess
import mimetypes

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

    def do_POST(self):
        """!
        How to deal with a POST-request
        """

        path = self.path
        contentType = self.headers['Content-type']

        log.debug("POST: Path %s" % path)

        if not contentType.startswith('application/json'):
            self.send_response(404)
            self.send_header('Content-type', 'application/json')
            self.end_headers()
            self.wfile.write(json.dumps({"error": "ContentType not supported"}))
            return

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
        if self.path.startswith('/ajax/pingserver'):
            return (200, json.dumps({}), 'application/json')

        # Used to get a list of all log files
        if self.path.startswith('/ajax/listlogs'):
            loglist = {}
            for dir in os.listdir('./results/current'):
                if os.path.isdir('./results/current/' + dir):
                    for file in os.listdir('./results/current/' + dir):
                        if file.endswith('.log'):
                            print './results/current/' + dir
                            if dir in loglist.keys():
                                loglist[dir].append(file)
                            else:
                                loglist[dir] = [file]
            print loglist
            return (200, json.dumps(loglist), 'application/json')

        # Used to generate new plots
        if self.path.startswith('/ajax/makeplots'):
            log.debug('Creating plots for revisions ' + ', '.join(data))
            create_plots(revisions=data)
            return (200, json.dumps({}), 'application/json')


###############################################################################
### Webserver starts here
###############################################################################


def run(ip='', port=8000):
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
