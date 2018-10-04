#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Script to make sure the conditions database interface is behaving as expected.

We do this by creating a local http server which acts as a mock database for
getting the payload information and payloads and then we run through different scenarios:
    - unknown host
    - connection refused
    - url not found (404)
    - retry on (503)
    - corrupt payload information
    - incomplete payload information
    - correct payload information
    - payload file missing
    - payload file checksum mismatch
"""

import sys
from basf2 import *
from ROOT import Belle2
from http.server import HTTPServer, BaseHTTPRequestHandler
from urllib.parse import urlparse, parse_qs
from b2test_utils import clean_working_directory, safe_process, skip_test
import multiprocessing
import shutil


class SimpleConditionsDB(BaseHTTPRequestHandler):
    """Simple ConditionsDB server which handles just the two things needed to
    test the interface: get a list of payloads for the current run and download
    a payloadfile. It will return different payloads for the experiments to
    check for different error conditions"""

    #: json string containing information for one payload
    example_payload = """[{{
        "payload": {{
            "baseUrl": "%(baseurl)s",
            "payloadId":1,
            "checksum":"{checksum}",
            "revision":{revision},
            "payloadUrl":"dbstore_BeamParameters_rev_{revision}.root",
            "basf2Module": {{ "name":"BeamParameters", "basf2Package": {{ "name":"dbstore" }} }}
        }}, "payloadIov": {{
            "expStart": %(exp)s,
            "expEnd": %(exp)s,
            "runStart": %(run)s,
            "runEnd": %(run)s
        }}
        }}]"""

    #: map payload information to be returned for different experiments
    payloads = {
        # let's start with empty information
        "0": "[]",
        # or one child but the wrong one
        "1": '[{ "foo": { } }]',
        # or let's have some invalid XML
        "2": '[{ "foo',
        # let's provide one correct payload
        "3": example_payload.format(checksum="2447fbcf76419fbbc7c6d015ef507769", revision="1"),
        # same payload but checksum mismatch
        "4": example_payload.format(checksum="[wrong checksum]", revision="1"),
        # non existing payload file
        "5": example_payload.format(checksum="missing", revision="2"),
        # duplicate payload, or in this case triple
        "6": example_payload.format(checksum="2447fbcf76419fbbc7c6d015ef507769", revision="2")[:-1] + "," +
             example_payload.format(checksum="2447fbcf76419fbbc7c6d015ef507769", revision="1")[1:-1] + "," +
             example_payload.format(checksum="2447fbcf76419fbbc7c6d015ef507769", revision="3")[1:],
    }

    def reply(self, xml):
        """Return a given xml string"""
        self.send_response(200)
        self.end_headers()
        self.wfile.write(xml.encode())

    def log_message(self, format, *args):
        """Override default logging to remove timestamp"""
        print("MockConditionsDB:", format % args)

    def log_error(self, *args):
        """Disable error logs"""
        pass

    def do_GET(self):
        """Parse a get request"""
        url = urlparse(self.path)
        params = parse_qs(url.query)
        # return mock payload info
        if url.path.endswith("/iovPayloads/"):
            exp = params["expNumber"][0]
            run = params["runNumber"][0]

            if int(exp) > 100:
                self.send_error(int(exp))
                return

            if int(run) > 1:
                exp = None

            if exp in self.payloads:
                baseurl = "http://%s:%s" % self.server.socket.getsockname()
                return self.reply(self.payloads[exp] % dict(exp=exp, run=run, baseurl=baseurl))
        else:
            # check if a fallback payload file exists in the data/framework directory
            filename = os.path.basename(url.path)
            # replace rev_3 with rev_1
            filename = filename.replace("rev_3", "rev_1")
            basedir = Belle2.FileSystem.findFile("data/framework")
            path = os.path.join(basedir, filename)
            if os.path.isfile(path):
                # ok, file exists. let's serve it
                self.send_response(200)
                self.end_headers()
                with open(path, "rb") as f:
                    shutil.copyfileobj(f, self.wfile)
                return

        # fall back: just return file not found
        self.send_error(404)


def run_mockdb(pipe):
    """Startup the mock conditions db server and send the port we listen on back
    to the parent process"""
    # listen on port 0 means we want to listen on any free port which would be
    # nice. But since the new code prints the full url including port when there
    # is a problem we choose a fixed one and hope that it it's free ...
    try:
        httpd = HTTPServer(("127.0.0.1", 12701), SimpleConditionsDB)
    except OSError:
        pipe.send(None)
        skip_test("Socket 12701 is in use, cannot continue")
        return
    # so see which port we actually got
    port = httpd.socket.getsockname()[1]
    # and send to parent
    pipe.send(port)
    # now start listening
    httpd.serve_forever()


def run_redirect(pipe, redir_port):
    """Startup the redirection server: any request should be transparently forwarded
    to the other using 308 http replies"""

    class SimpleRedirectServer(BaseHTTPRequestHandler):
        def do_GET(self):
            self.send_response(308)
            self.send_header("Location", f"http://127.0.0.1:{redir_port}{self.path}")
            self.end_headers()

        def log_message(self, format, *args):
            """Override default logging to remove timestamp"""
            print("Redirect Server:", format % args)

        def log_error(self, *args):
            """Disable error logs"""
            pass

    try:
        httpd = HTTPServer(("127.0.0.1", 12702), SimpleRedirectServer)
        pipe.send(12702)
    except OSError:
        pipe.send(None)
        skip_test("Socket 12702 is in use, cannot continue")
        return

    # now start listening
    httpd.serve_forever()


def dbprocess(host, path, lastChangeCallback=lambda: None):
    """Process a given path in a child process so that FATAL will not abort this
    script but just the child and configure to use a central database at the given host"""
    # reset the database so that there is no chain
    reset_database()
    # now run the path in a child process inside of a clean working directory
    with clean_working_directory() as tempdir:
        use_central_database("localtest", host, host, "", LogLevel.WARNING)
        lastChangeCallback()
        safe_process(path)


# keep timeouts short for testing
set_central_database_networkparams(backoff_factor=1, connection_timeout=5, stalled_timeout=5)

# set the random seed to something fixed
set_random_seed("something important")
# simplify logging output to just the type and the message
for level in LogLevel.values.values():
    logging.set_info(level, LogInfo.LEVEL | LogInfo.MESSAGE)
# disable error summary, we don't need it for these short tests and it basically
# doubles the output
logging.enable_summary(False)
# and create a pipe so we can send the port we listen on from child to parent
conn = multiprocessing.Pipe(False)
# now start the mock conditions database as daemon so it gets killed at the end
# of the script
mock_conditionsdb = multiprocessing.Process(target=run_mockdb, args=(conn[1],))
mock_conditionsdb.daemon = True
mock_conditionsdb.start()
# mock db has started when we recieve the port number from the child, so wait for that
mock_port = conn[0].recv()
# if the port we got is None the server didn't start ... so bail
if mock_port is None:
    sys.exit(1)

# startup redirect server, same as conditionsdb_
redir_server = multiprocessing.Process(target=run_redirect, args=(conn[1], mock_port))
redir_server.daemon = True
redir_server.start()
redir_port = conn[0].recv()
if redir_port is None:
    sys.exit(1)

# and remember host for database access
mock_host = f"http://localhost:{mock_port}/"
redir_host = f"http://localhost:{redir_port}/"

# create a simple processing path with just event info setter an a module which
# prints the beamparameters from the database
main = create_path()
evtinfo = main.add_module("EventInfoSetter")
main.add_module("PrintBeamParameters")

# run trough a set of experiments, each time we want to process two runs to make
# sure that it works correctly for more than one run
for exp in range(len(SimpleConditionsDB.payloads) + 1):
    evtinfo.param({"expList": [exp, exp, exp], "runList": [0, 1, 2], "evtNumList": [1, 1, 1]})
    dbprocess(mock_host, main)
    # and again using redirection
    dbprocess(redir_host, main)

# check 503 retry
evtinfo.param({"expList": [503], "runList": [0], "evtNumList": [1]})
dbprocess(mock_host, main)
# check again with different amount of retries
set_central_database_networkparams(max_retries=0)
dbprocess(mock_host, main)

# the following ones fail, no need for 3 times
evtinfo.param({"expList": [0], "runList": [0], "evtNumList": [1]})

# try to open localhost on port 0, this should always be refused
dbprocess("http://localhost:0", main)

# and once more with a non existing host name to check for lookup errors
dbprocess("http://nosuchurl/", main)

# and once more with a non existing protocol
dbprocess("nosuchproto://nosuchurl/", main)

# and once more with a totally bogus url
dbprocess("h͌̉e̳̞̞͆ͨ̏͋̕ ͍͚̱̰̀͡c͟o͛҉̟̰̫͔̟̪̠m̴̀ͯ̿͌ͨ̃͆e̡̦̦͖̳͉̗ͨͬ̑͌̃ͅt̰̝͈͚͍̳͇͌h̭̜̙̦̣̓̌̃̓̀̉͜!̱̞̻̈̿̒̀͢!̋̽̍̈͐ͫ͏̠̹̺̜̬͍ͅ", main)

# try to have a list of servers
serverlist = ["http://localhost:0", "h͌̉e̳̞̞͆ͨ̏͋̕c͟o͛҉̟̰̫͔̟̪̠m̴̀ͯ̿͌ͨ̃͆e̡̦̦͖̳͉̗ͨͬ̑͌̃ͅt̰̝͈͚͍̳͇͌h̭̜̙̦̣̓̌̃̓̀̉͜!̱̞̻̈̿̒̀͢", mock_host]
os.environ["BELLE2_CONDB_SERVERLIST"] = " ".join(serverlist)
dbprocess("", main)

# ok, try again with the steering file settings instead of environment variable
del os.environ["BELLE2_CONDB_SERVERLIST"]
del serverlist[1]
dbprocess("", main, lastChangeCallback=lambda: set_central_serverlist(serverlist))

if "ssl" in sys.argv:
    # ok, test SSL connectivity ... for now we just want to accept anything. This
    # is disabled by default since I don't want to depend on badssl.com to be
    # available
    for hostname in ("expired", "wrong.host", "self-signed", "untrusted-root"):
        dbprocess(f"https://{hostname}.badssl.com/", main)
