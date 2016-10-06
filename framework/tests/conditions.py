#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Script to make sure the ConditionsService interface is behaving as expected.

We do this by creating a local http server which acts as a mock database for
getting the payload information and payloads and then we run through different scenarios:
    - unknown host
    - connection refused
    - url not found (404)
    - corrupt payload information
    - incomplete payload information
    - correct payload information
    - payload file missing
    - payload file checksum mismatch
"""

from basf2 import *
from ROOT import Belle2
from http.server import HTTPServer, BaseHTTPRequestHandler
from urllib.parse import urlparse, parse_qs
from contextlib import contextmanager
import multiprocessing
import tempfile
import shutil


class SimpleConditionsDB(BaseHTTPRequestHandler):
    """Simple ConditionsDB server which handles just the two things needed to
    test the interface: get a list of payloads for the current run and download
    a payloadfile. It will return different payloads for the experiments to
    check for different error conditions"""

    #: xml string containing information for one payload
    example_payload = """
        <currentPayloadIovs><currentPayloadIov>
        <payload payloadId="1">
            <checksum>{checksum}</checksum>
            <revision>{revision}</revision>
            <payloadUrl>dbstore_BeamParameters_rev_{revision}.root</payloadUrl>
            <basf2Module><name>BeamParameters</name><basf2Package><name>dbstore</name></basf2Package></basf2Module>
        </payload><payloadIov>
            <initialRunId><name>{{exp}}</name><experiment><name>{{run}}</name></experiment></initialRunId>
            <finalRunId><name>{{exp}}</name><experiment><name>{{run}}</name></experiment></finalRunId>
        </payloadIov>
        </currentPayloadIov></currentPayloadIovs>"""

    #: map payload information to be returned for different experiments
    payloads = {
        # let's start with empty information
        "0": "<foo/>",
        # or one child but the wrong one
        "1": "<currentPayloadIovs><foo/></currentPayloadIovs>",
        # or let's have some invalid XML
        "2": "<foo><ba",
        # let's provide one correct payload
        "3": example_payload.format(checksum="2447fbcf76419fbbc7c6d015ef507769", revision="1"),
        # same payload but checksum mismatch
        "4": example_payload.format(checksum="[wrong checksum]", revision="1"),
        # non existing payload file
        "5": example_payload.format(checksum="missing", revision="2"),
        # duplicate payload, or in this case triple
        "6": example_payload.format(checksum="2447fbcf76419fbbc7c6d015ef507769", revision="2")[:-21] +
             example_payload.format(checksum="2447fbcf76419fbbc7c6d015ef507769", revision="1")[20:-21] +
             example_payload.format(checksum="2447fbcf76419fbbc7c6d015ef507769", revision="3")[20:],
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
        if url.path == "/iovPayloads/":
            exp = params["expName"][0]
            run = params["runName"][0]
            if exp in self.payloads:
                return self.reply(self.payloads[exp].format(exp=exp, run=run))
        else:
            # check if a fallback payload file exists in the data/framework directory
            filename = os.path.basename(url.path)
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
    # listen on port 0 means we want to listen on any free port
    httpd = HTTPServer(("127.0.0.1", 0), SimpleConditionsDB)
    # so see which port we actually got
    port = httpd.socket.getsockname()[1]
    # and send to parent
    pipe.send(port)
    # now start listening
    httpd.serve_forever()


@contextmanager
def clean_working_directory():
    """Context manager to create a temporary directory and directly us it as
    current working directory"""
    dirname = os.getcwd()
    try:
        with tempfile.TemporaryDirectory() as tempdir:
            os.chdir(tempdir)
            yield tempdir
    finally:
        os.chdir(dirname)


def dbprocess(host, path):
    """Process a given path in a child process so that FATAL will not abort this
    script but just the child and configure to use a central database at the given host"""
    # reset the database so that there is no chain
    reset_database()
    # now run the path in a child process inside of a clean working directory
    with clean_working_directory() as tempdir:
        os.chdir(tempdir)
        use_central_database("localtest", host, host, "", LogLevel.WARNING)
        child = multiprocessing.Process(target=process, args=(path,))
        child.start()
        child.join()


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
port = conn[0].recv()
# and remember host for database access
host = "http://localhost:%d/" % port

# create a simple processing path with just event info setter an a module which
# prints the beamparameters from the database
main = create_path()
evtinfo = main.add_module("EventInfoSetter")
main.add_module("PrintBeamParameters")

# run trough a set of experiments, each time we want to process two runs to make
# sure that it works correctly for more than one run
for exp in range(len(SimpleConditionsDB.payloads) + 1):
    evtinfo.param({"expList": [exp, exp], "runList": [0, 1], "evtNumList": [1, 1]})
    dbprocess(host, main)

# try to open localhost on port 0, this should always be refused
dbprocess("http://localhost:0", main)

# and once more with a non existing host name to check for routing errors
dbprocess("http://nosuchurl/", main)

# and once more with a non exsiting protocol
dbprocess("nosuchproto://nosuchurl/", main)

# and once more with a totally bogus url
dbprocess("h͌̉e̳̞̞͆ͨ̏͋̕ ͍͚̱̰̀͡c͟o͛҉̟̰̫͔̟̪̠m̴̀ͯ̿͌ͨ̃͆e̡̦̦͖̳͉̗ͨͬ̑͌̃ͅt̰̝͈͚͍̳͇͌h̭̜̙̦̣̓̌̃̓̀̉͜!̱̞̻̈̿̒̀͢!̋̽̍̈͐ͫ͏̠̹̺̜̬͍ͅ", main)
