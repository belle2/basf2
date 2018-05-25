#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
conditions_db
-------------

Python interface to the ConditionsDB
"""

import os
from basf2 import B2FATAL, B2ERROR, B2INFO
import requests
from requests.auth import HTTPBasicAuth, HTTPDigestAuth
from requests.packages.urllib3.fields import RequestField
from requests.packages.urllib3.filepost import encode_multipart_formdata
import json
import hashlib
import urllib


def calculate_checksum(filename):
    """Calculate md5 hash of file"""
    md5hash = hashlib.md5()
    with open(filename, "rb") as data:
        md5hash.update(data.read())
    return md5hash.hexdigest()


def encode_name(name):
    """Escape name to be used in an url"""
    return urllib.parse.quote(name, safe="")


class ConditionsDB:
    """Class to interface conditions db REST interface"""

    #: base url to the conditions db to be used if no custom url is given
    BASE_URL = "http://belle2db.hep.pnnl.gov/b2s/rest/v2/"

    class RequestError(RuntimeError):
        """Class to be thrown by request() if there is any error"""
        pass

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
        self._session.headers.update({"Accept": "application/json", "Cache-Control": "no-cache"})
        # add a http adapter to honour our max_connections and retries settings
        self._session.mount(self._base_url, requests.adapters.HTTPAdapter(
            pool_connections=max_connections, pool_maxsize=max_connections,
            max_retries=retries, pool_block=True))
        if "BELLE2_CONDB_PROXY" in os.environ:
            self._session.proxies = {
                "http": os.environ.get("BELLE2_CONDB_PROXY"),
                "https": os.environ.get("BELLE2_CONDB_PROXY"),
            }

    def set_authentication(self, user, password, basic=True):
        """
        Set authentication credentials when talking to the database

        Args:
            user (str): username
            password (str): password
            basic (bool): if True us HTTP Basic authentication, otherwise HTTP Digest
        """
        authtype = HTTPBasicAuth if basic else HTTPDigestAuth
        self._session.auth = authtype(user, password)

    def request(self, method, url, message=None, *args, **argk):
        """
        Request function, similar to requests.request but adding the base_url

        Args:
            method (str): GET, POST, etc.
            url (str): url for the request, base_url will be prepended
            message (str): message to show when starting the request and if it fails

        All other arguments will be forwarded to requests.request.
        """
        if message is not None:
            B2INFO(message)

        try:
            req = self._session.request(method, self._base_url + url.lstrip("/"), *args, **argk)
        except requests.exceptions.ConnectionError as e:
            B2FATAL("Could not access '" + self._base_url + url.lstrip("/") + "': " + str(e))

        if req.status_code >= 300:
            # Apparently something is not good. Let's try to decode the json
            # reply containing reason and message
            try:
                response = req.json()
                error = "Request {method} {url} returned {code} {reason}: {message}".format(
                    method=method, url=url,
                    code=response["code"],
                    reason=response["reason"],
                    message=response.get("message", ""),
                )
            except json.JSONDecodeError:
                # seems the reply was not even json
                error = "Request {method} {url} returned non JSON response {code}: {content}".format(
                    method=method, url=url,
                    code=req.status_code,
                    content=req.content
                )

            if message is not None:
                raise ConditionsDB.RequestError("{} failed: {}".format(message, error))
            else:
                raise ConditionsDB.RequestError(error)

        if method != "HEAD" and req.status_code != requests.codes.no_content:
            try:
                req.json()
            except json.JSONDecodeError as e:
                B2INFO("Invalid response: {}".format(req.content))
                raise ConditionsDB.RequestError("{method} {url} returned invalid JSON response {}"
                                                .format(e, method=method, url=url))
        return req

    def get_globalTags(self):
        """Get a list of all global tags. Returns a dictionary with the global
        tag names and the corresponding ids in the database"""

        try:
            req = self.request("GET", "/globalTags")
        except ConditionsDB.RequestError as e:
            B2ERROR("Could not get the list of global tags: {}".format(e))
            return None

        result = {}
        for tag in req.json():
            result[tag["name"]] = tag

        return result

    def get_globalTagInfo(self, name):
        """Get the id of the global tag with the given name. Returns either the
        id or None if the tag was not found"""

        try:
            req = self.request("GET", "/globalTag/{globalTagName}".format(globalTagName=encode_name(name)))
        except ConditionsDB.RequestError as e:
            B2ERROR("Cannot find global tag '{}': {}".format(name, e))
            return None

        return req.json()

    def get_globalTagType(self, name):
        """
        Get the dictionary describing the given global tag type (currently
        one of DEV or RELEASE). Returns None if tag type was not found.
        """
        try:
            req = self.request("GET", "/globalTagType")
        except ConditionsDB.RequestError as e:
            B2ERROR("Coult not get list of valid global tag types: {}".format(e))
            return None

        types = {e["name"]: e for e in req.json()}

        if name in types:
            return types[name]

        B2ERROR("Unknown global tag type: '{}', please use one of {}".format(name, ", ".join(types)))
        return None

    def get_payloads(self, global_tag=None):
        """
        Get a list of all defined payloads (for the given global_tag or by default for all).
        Returns a dictionary which maps (module, checksum) to the payload id.
        """

        try:
            if global_tag:
                req = self.request("GET", "/globalTag/{global_tag}/payloads"
                                   .format(global_tag=encode_name(global_tag)))
            else:
                req = self.request("GET", "/payloads")
        except ConditionsDB.RequestError as e:
            B2ERROR("Cannot get list of payloads: {}".format(e))
            return {}

        result = {}
        for payload in req.json():
            module = payload["basf2Module"]["name"]
            checksum = payload["checksum"]
            result[(module, checksum)] = payload["payloadId"]

        return result

    def check_payloads(self, payloads):
        """
        Check for the existence of payloads in the database.

        Arguments:
            payloads list((str,str)): A list of payloads to check for. Each
               payload needs to be a tuple of the name of the payload and the
               md5 checksum of the payload file.

        Returns:
            A dictionary with the payload identifiers (name, checksum) as keys
            and the payload ids as values for all payloads which are already
            present in the database.
        """

        search_query = [{"name": e[0], "checksum": e[1]} for e in payloads]
        try:
            req = self.request("POST", "/checkPayloads", json=search_query)
        except ConditionsDB.RequestError as e:
            B2ERROR("Cannot check for existing payloads: {}".format(e))
            return {}

        result = {}
        for payload in req.json():
            module = payload["basf2Module"]["name"]
            checksum = payload["checksum"]
            result[(module, checksum)] = payload["payloadId"]

        return result

    def create_payload(self, module, filename, checksum=None):
        """
        Create a new payload

        Args:
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
        try:
            req = self.request("POST", "/package/dbstore/module/{moduleName}/payload"
                               .format(moduleName=encode_name(module)),
                               data=post_body, headers=headers)
        except ConditionsDB.RequestError as e:
            B2ERROR("Could not create Payload: {}".format(e))
            return None

        return req.json()["payloadId"]

    def create_iov(self, globalTagId, payloadId, firstExp, firstRun, finalExp, finalRun):
        """
        Create an iov.

        Args:
            globalTagId (int): id of the global tag, obtain with get_globalTagId()
            payloadId (int): id of the payload, obtain from create_payload() or get_payloads()
            firstExp (int): first experiment for which this iov is valid
            firstRun (int): first run for which this iov is valid
            finalExp (int): final experiment for which this iov is valid
            finalRun (int): final run for which this iov is valid

        Returns:
            payloadIovId of the created iov, None if creation was not successful
        """
        try:
            # try to convert all arguments except self to integers to make sure they are
            # valid.
            local_variables = locals()
            variables = {e: int(local_variables[e]) for e in
                         ["globalTagId", "payloadId", "firstExp", "firstRun", "finalExp", "finalRun"]}
        except ValueError:
            B2ERROR("create_iov: All parameters need to be integers")
            return None

        # try to create the iov
        try:
            req = self.request("POST", "/globalTagPayload/{globalTagId},{payloadId}"
                               "/payloadIov/{firstExp},{firstRun},{finalExp},{finalRun}".format(**variables))
        except ConditionsDB.RequestError as e:
            B2ERROR("Could not create IOV: {}".format(e))
            return None

        return req.json()["payloadIovId"]

    def get_iovs(self, globalTagName):
        """Return existing iovs for a given tag name. It returns a dictionary
        which maps (payloadId, first runId, final runId) to iovId"""

        try:
            req = self.request("GET", "/globalTag/{globalTagName}/globalTagPayloads"
                               .format(globalTagName=encode_name(globalTagName)))
        except ConditionsDB.RequestError as e:
            # there could be just no iovs so no error
            return {}

        result = {}
        for payload in req.json():
            payloadId = payload["payloadId"]["payloadId"]
            for iov in payload["payloadIovs"]:
                iovId = iov["payloadIovId"]
                firstExp, firstRun = iov["expStart"], iov["runStart"]
                finalExp, finalRun = iov["expEnd"], iov["runEnd"]
                result[(payloadId, firstExp, firstRun, finalExp, finalRun)] = iovId

        return result


def require_database_for_test(timeout=60, base_url=ConditionsDB.BASE_URL):
    """Make sure that the database is available and skip the test if not.

    This function should be called in test scripts if they are expected to fail
    if the database is down. It either returns when the database is ok or it
    will signal test_basf2 that the test should be skipped and exit
    """
    import sys
    try:
        if os.environ.get("BELLE2_CONDB_GLOBALTAG", None) == "":
            raise Exception("Access to the Database is disabled")
        req = requests.request("HEAD", base_url + "globalTags", timeout=timeout)
        req.raise_for_status()
    except Exception as e:
        print("TEST SKIPPED: Database problem: %s" % e, file=sys.stderr)
        sys.exit(1)


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
