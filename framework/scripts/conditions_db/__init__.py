#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
conditions_db
-------------

Python interface to the ConditionsDB
"""

from basf2 import *
import requests
from requests.packages.urllib3.fields import RequestField
from requests.packages.urllib3.filepost import encode_multipart_formdata
import json
import hashlib


def int_if_possible(value):
    """Convert a value to an integer if possible. Silently ignore errors and
    return the original value in this case"""
    try:
        return int(value)
    except ValueError:
        return value


def calculate_checksum(filename):
    """Calculate md5 hash of file"""
    md5hash = hashlib.md5()
    with open(filename, "rb") as data:
        md5hash.update(data.read())
    return md5hash.hexdigest()


class ConditionsDB:
    """Class to interface conditions db REST interface"""

    #: base url to the conditions db to be used if no custom url is given
    BASE_URL = "http://belle2db.hep.pnnl.gov/b2s/rest/v1/"

    def __init__(self, base_url=BASE_URL, max_connections=10, retries=3):
        """
        Create a new instance of the interface

        Args:
            base_url (string): base url of the rest interface
            max_connections (int): number of connections to keep open, mostly useful for threaded applications
            retries (int): number of retries in case of connection problems
        """
        #: base url to be prepended to all requests
        self._base_url = base_url.rstrip("/") + "/"
        #: session object to get keep-alive support and connection pooling
        self._session = requests.Session()
        # change the api to return json instead of xml, much easier in python
        self._session.headers.update({"Accept": "application/json"})
        # add a http adapter to honour our max_connections and retries settings
        self._session.mount(self._base_url, requests.adapters.HTTPAdapter(
            pool_connections=max_connections, pool_maxsize=max_connections,
            max_retries=retries, pool_block=True))

    def request(self, method, url, *args, **argk):
        """
        Request function, similar to requests.request but adding the base_url

        Args:
            method (str): GET, POST, etc.
            url (str): url for the request, base_url will be prepended

        All other arguments will be forwarded to requests.request.
        """
        try:
            req = self._session.request(method, self._base_url + url.lstrip("/"), *args, **argk)
        except requests.exceptions.ConnectionError as e:
            B2FATAL("Could not access '" + self._base_url + url.lstrip("/") + "': " + str(e))

        if not req.status_code == requests.codes.ok:
            # Apparently something is not good. Let's try to decode the json
            # reply containing reason and message
            try:
                response = req.json()
                B2WARNING("Request {method} {url} returned {code} {reason}: {message}".format(
                    method=method, url=url,
                    code=response["code"],
                    reason=response["reason"],
                    message=response["message"],
                ))
            except json.JSONDecodeError:
                # seems the reply was not even json, just print it but flag it
                # as error
                B2ERROR("Request {method} {url} returned non JSON response {code}: {content}".format(
                    method=method, url=url,
                    code=req.status_code,
                    content=req.content
                ))
        return req

    def get_globalTags(self):
        """Get a list of all global tags. Returns a dictionary with the global
        tag names and the corresponding ids in the database"""

        req = self.request("GET", "/globalTags")
        if req.status_code != requests.codes.ok:
            B2ERROR("Could not get the list of global tags")
            return None

        result = {}
        for tag in req.json():
            result[tag["name"]] = tag["globalTagId"]

        return result

    def get_globalTagId(self, name):
        """Get the id of the global tag with the given name. Returns either the id or None if the tag was not found"""

        req = self.request("GET", "/globalTag/{globalTagName}".format(globalTagName=name))
        if req.status_code != requests.codes.ok:
            B2ERROR("Cannot find global tag '%s'" % name)
            return None

        return req.json()["globalTagId"]

    def get_experiments(self):
        """Get list of experiments. Will return a dictionary with the
        experiments as keys and their ids as values."""

        req = self.request("GET", "/experiments")
        if req.status_code != requests.codes.ok:
            B2ERROR("Could not get the list of experiments")
            return {}

        result = {}
        for exp in req.json():
            result[int_if_possible(exp["name"])] = exp["experimentId"]

        return result

    def get_runs(self, experiment):
        """Return list of runs for a given experiment name. Returns None if no
        such experiments exists, otherwise a dictionary with run rame and run
        ids"""

        req = self.request("GET", "/experiment/{name}/runs".format(name=experiment))
        if req.status_code != requests.codes.ok:
            # there could just be no runs so just return empty
            return {}

        result = {}
        for exp in req.json():
            result[int_if_possible(exp["name"])] = exp["runId"]

        return result

    def get_runId(self, experiment, run):
        """Get the run id for a given experiment and run name. Returns None if
        no such experiment/run can be found.

        Warning: don't use this function to get the id's of many runs in an
        experiment. It's much better to call get_runs(experiment) and just look
        in the dictionary."""

        runs = self.get_runs(experiment)
        try:
            return runs[run]
        except (KeyError, TypeError):
            B2ERROR("Cannot get run id for experiment {0}, run {1}: no such run".format(experiment, run))
            return None

    def get_payloads(self, global_tag=None):
        """
        Get a list of all defined payloads (for the given global_tag or by default for all).
        Returns a dictionary which maps (package, module, checksum) to the payload id.
        """

        if global_tag:
            req = self.request("GET", "/globalTag/{global_tag}/payloads".format(global_tag=global_tag))
        else:
            req = self.request("GET", "/payloads")
        if req.status_code != requests.codes.ok:
            B2ERROR("Cannot get list of payloads")
            return {}

        result = {}
        for payload in req.json():
            package = payload["basf2Module"]["basf2Package"]["name"]
            module = payload["basf2Module"]["name"]
            checksum = payload["checksum"]
            result[(package, module, checksum)] = payload["payloadId"]

        return result

    def create_payload(self, package, module, filename, checksum=None):
        """
        Create a new payload

        Args:
            package (str): name of the package
            module (str): name of the module
            filename (str): name of the file
            checksum (str): md5 hexdigest of the file. Will be calculated automatically if not given
        """
        if checksum is None:
            checksum = calculate_checksum(filename)

        # this is the only complicated request as we have to provide a
        # multipart/mixed request which is not directly provided by the request
        # library.
        files = [
            (filename, open(filename, "rb").read(), "application/x-root"),
            ("json", json.dumps({"checksum": checksum, "isDefault": False}), "application/json"),
        ]
        # ok we have the two "files" we want to send, create multipart/mixed
        # body
        fields = []
        for name, contents, mimetype in files:
            rf = RequestField(name=name, data=contents)
            rf.make_multipart(content_type=mimetype)
            fields.append(rf)

        post_body, content_type = encode_multipart_formdata(fields)
        content_type = ''.join(('multipart/mixed',) + content_type.partition(';')[1:])
        headers = {'Content-Type': content_type}

        # now make the request. Note to self: if multipart/form-data would be
        # accepted this would be so much nicer here. but it works.
        req = self.request("POST", "/package/{packageName}/module/{moduleName}/payload".format(
            packageName=package, moduleName=module
        ), data=post_body, headers=headers)

        if req.status_code != requests.codes.ok:
            B2ERROR("Could not create Payload")
            return None

        return req.json()["payloadId"]

    def create_iov(self, globalTagId, payloadId, firstRunId, lastRunId):
        """
        Create an iov.

        Args:
            globalTagId (int): id of the global tag, obtain with get_globalTagId()
            payloadId (int): id of the payload, obtain from create_payload() or get_payloads()
            firstRunId (int): id of the first run for which this iov is valid. Optionally a tuple of experiment/run names
            lastRunid (int): id of the last run for which this iov is valid. Optionally a tuple of experiment/run names

        Returns:
            payloadIovId of the created iov, None if creation was not successful
        """
        # check if the run is a tuple of exp/run names. If so try to get the id
        if isinstance(firstRunId, tuple):
            firstRunId = self.get_runId(*firstRunId)
            if firstRunId is None:
                return None

        # and again
        if isinstance(lastRunId, tuple):
            lastRunId = self.get_runId(*lastRunId)
            if lastRunId is None:
                return None

        # try to create the iov
        req = self.request("POST", "/globalTagPayload/{globalTagId},{payloadId}/payloadIov/{initialRunId},{finalRunId}".format(
            globalTagId=globalTagId, payloadId=payloadId, initialRunId=firstRunId, finalRunId=lastRunId
        ))
        if req.status_code != requests.codes.ok:
            B2ERROR("Could not create IOV")
            return None

        return req.json()["payloadIovId"]

    def get_iovs(self, globalTagName):
        """Return existing iovs for a given tag name. It returns a dictionary
        which maps (payloadId, first runId, final runId) to iovId"""

        req = self.request("GET", "/globalTag/{globalTagName}/globalTagPayloads".format(globalTagName=globalTagName))
        if req.status_code != requests.codes.ok:
            # there could be just no iovs so no error
            return {}

        result = {}
        for payload in req.json():
            payloadId = payload["payloadId"]["payloadId"]
            for iov in payload["payloadIovs"]:
                iovId = iov["payloadIovId"]
                firstRunId = iov["initialRunId"]["runId"]
                finalRunId = iov["finalRunId"]["runId"]
                result[(payloadId, firstRunId, finalRunId)] = iovId

        return result


def enable_debugging():
    """Enable verbose output of python-requests to be able to debug http connections"""
    # These two lines enable debugging at httplib level (requests->urllib3->http.client)
    # You will see the REQUEST, including HEADERS and DATA, and RESPONSE with HEADERS but without DATA.
    # The only thing missing will be the response.body which is not logged.
    import http.client as http_client
    import logging
    http_client.HTTPConnection.debuglevel = 1
    # You must initialize logging, otherwise you'll not see debug output.
    logging.basicConfig()
    logging.getLogger().setLevel(logging.DEBUG)
    requests_log = logging.getLogger("requests.packages.urllib3")
    requests_log.setLevel(logging.DEBUG)
    requests_log.propagate = True
