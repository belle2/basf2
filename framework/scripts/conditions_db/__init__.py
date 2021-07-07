#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
conditions_db
-------------

Python interface to the ConditionsDB
"""

import os
from basf2 import B2FATAL, B2ERROR, B2INFO, B2WARNING
import requests
from requests.auth import HTTPBasicAuth, HTTPDigestAuth
from requests.packages.urllib3.fields import RequestField
from requests.packages.urllib3.filepost import encode_multipart_formdata
import json
import urllib
from versioning import upload_global_tag, jira_global_tag_v2
from collections import defaultdict
from concurrent.futures import ThreadPoolExecutor, wait as futures_wait
import hashlib
import itertools


def encode_name(name):
    """Escape name to be used in an url"""
    return urllib.parse.quote(name, safe="")


def file_checksum(filename):
    """Calculate md5 hash of file"""
    md5hash = hashlib.md5()
    with open(filename, "rb") as data:
        md5hash.update(data.read())
    return md5hash.hexdigest()


def chunks(container, chunk_size):
    """Cut a container in chunks of max. chunk_size"""
    it = iter(container)
    while True:
        chunk = tuple(itertools.islice(it, chunk_size))
        if not chunk:
            return
        yield chunk


class PayloadInformation:
    """Small container class to help compare payload information for efficient
    comparison between globaltags"""

    @classmethod
    def from_json(cls, payload, iov=None):
        """Set all internal members from the json information of the payload and the iov.

        Arguments:
            payload (dict): json information of the payload as returned by REST api
            iov (dict): json information of the iov as returned by REST api
        """
        if iov is None:
            iov = {"payloadIovId": None, "expStart": None, "runStart": None, "expEnd": None, "runEnd": None}

        return cls(
            payload['payloadId'],
            payload['basf2Module']['name'],
            payload['revision'],
            payload['checksum'],
            payload['payloadUrl'],
            payload['baseUrl'],
            iov['payloadIovId'],
            (iov["expStart"], iov["runStart"], iov["expEnd"], iov["runEnd"]),
        )

    def __init__(self, payload_id, name, revision, checksum, payload_url, base_url, iov_id=None, iov=None):
        """
        Create a new object from the given information
        """
        #: name of the payload
        self.name = name
        #: checksum of the payload
        self.checksum = checksum
        #: interval of validity
        self.iov = iov
        #: revision, not used for comparisons
        self.revision = revision
        #: payload id in CDB, not used for comparisons
        self.payload_id = payload_id
        #: iov id in CDB, not used for comparisons
        self.iov_id = iov_id
        #: base url
        self.base_url = base_url
        #: payload url
        self.payload_url = payload_url

    @property
    def url(self):
        """Return the full url to the payload on the server"""
        return urllib.parse.urljoin(self.base_url + '/', self.payload_url)

    def __hash__(self):
        """Make object hashable"""
        return hash((self.name, self.checksum, self.iov))

    def __eq__(self, other):
        """Check if two payloads are equal"""
        return (self.name, self.checksum, self.iov) == (other.name, other.checksum, other.iov)

    def __lt__(self, other):
        """Sort payloads by name, iov, revision"""
        return (self.name.lower(), self.iov, self.revision) < (other.name.lower(), other.iov, other.revision)

    def readable_iov(self):
        """return a human readable name for the IoV"""
        if self.iov is None:
            return "none"

        if self.iov == (0, 0, -1, -1):
            return "always"

        e1, r1, e2, r2 = self.iov
        if e1 == e2:
            if r1 == 0 and r2 == -1:
                return f"exp {e1}"
            elif r2 == -1:
                return f"exp {e1}, runs {r1}+"
            elif r1 == r2:
                return f"exp {e1}, run {r1}"
            else:
                return f"exp {e1}, runs {r1} - {r2}"
        else:
            if e2 == -1 and r1 == 0:
                return f"exp {e1} - forever"
            elif e2 == -1:
                return f"exp {e1}, run {r1} - forever"
            elif r1 == 0 and r2 == -1:
                return f"exp {e1}-{e2}, all runs"
            elif r2 == -1:
                return f"exp {e1}, run {r1} - exp {e2}, all runs"
            else:
                return f"exp {e1}, run {r1} - exp {e2}, run {r2}"


class ConditionsDB:
    """Class to interface conditions db REST interface"""

    #: base url to the conditions db to be used if no custom url is given
    BASE_URLS = ["http://belle2db.sdcc.bnl.gov/b2s/rest/"]

    class RequestError(RuntimeError):
        """Class to be thrown by request() if there is any error"""
        pass

    @staticmethod
    def get_base_urls(given_url):
        """Resolve the list of server urls. If a url is given just return it.
        Otherwise return servers listed in BELLE2_CONDB_SERVERLIST or the
        builtin defaults

        Arguments:
            given_url (str): Explicit base_url. If this is not None it will be
               returned as is in a list of length 1

        Returns:
            a list of urls to try for database connectivity
        """

        base_url_list = ConditionsDB.BASE_URLS[:]
        base_url_env = os.environ.get("BELLE2_CONDB_SERVERLIST", None)
        if given_url is not None:
            base_url_list = [given_url]
        elif base_url_env is not None:
            base_url_list = base_url_env.split()
            B2INFO("Getting Conditions Database servers from Environment:")
            for i, url in enumerate(base_url_list, 1):
                B2INFO(f"  {i}. {url}")
        # try to escalate to https for all given urls
        full_list = []
        for url in base_url_list:
            if url.startswith("http://"):
                full_list.append("https" + url[4:])
            # but keep the http in case of connection problems
            full_list.append(url)
        return full_list

    def __init__(self, base_url=None, max_connections=10, retries=3):
        """
        Create a new instance of the interface

        Args:
            base_url (string): base url of the rest interface
            max_connections (int): number of connections to keep open, mostly useful for threaded applications
            retries (int): number of retries in case of connection problems
        """

        #: session object to get keep-alive support and connection pooling
        self._session = requests.Session()
        # and set the connection options we want to have
        adapter = requests.adapters.HTTPAdapter(
            pool_connections=max_connections, pool_maxsize=max_connections,
            max_retries=retries, pool_block=True
        )
        self._session.mount("http://", adapter)
        self._session.mount("https://", adapter)
        # and also set the proxy settings
        if "BELLE2_CONDB_PROXY" in os.environ:
            self._session.proxies = {
                "http": os.environ.get("BELLE2_CONDB_PROXY"),
                "https": os.environ.get("BELLE2_CONDB_PROXY"),
            }
        # test the given url or try the known defaults
        base_url_list = ConditionsDB.get_base_urls(base_url)

        for url in base_url_list:
            #: base url to be prepended to all requests
            self._base_url = url.rstrip("/") + "/"
            try:
                req = self._session.request("HEAD", self._base_url + "v2/globalTags")
                req.raise_for_status()
            except requests.RequestException as e:
                B2WARNING(f"Problem connecting to {url}:\n     {e}\n Trying next server ...")
            else:
                break
        else:
            B2FATAL("No working database servers configured, giving up")

        # We have a working server so change the api to return json instead of
        # xml, much easier in python, also request non-cached replies. We do
        # this now because for the server check above we're fine with cached
        # results.
        self._session.headers.update({"Accept": "application/json", "Cache-Control": "no-cache"})

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
            req = self._session.request(method, self._base_url + "v2/" + url.lstrip("/"), *args, **argk)
        except requests.exceptions.ConnectionError as e:
            B2FATAL("Could not access '" + self._base_url + url.lstrip("/") + "': " + str(e))

        if req.status_code >= 300:
            # Apparently something is not good. Let's try to decode the json
            # reply containing reason and message
            try:
                response = req.json()
                message = response.get("message", "")
                colon = ": " if message.strip() else ""
                error = "Request {method} {url} returned {code} {reason}{colon}{message}".format(
                    method=method, url=url,
                    code=response["code"],
                    reason=response["reason"],
                    message=message,
                    colon=colon,
                )
            except json.JSONDecodeError:
                # seems the reply was not even json
                error = "Request {method} {url} returned non JSON response {code}: {content}".format(
                    method=method, url=url,
                    code=req.status_code,
                    content=req.content
                )

            if message is not None:
                raise ConditionsDB.RequestError(f"{message} failed: {error}")
            else:
                raise ConditionsDB.RequestError(error)

        if method != "HEAD" and req.status_code != requests.codes.no_content:
            try:
                req.json()
            except json.JSONDecodeError as e:
                B2INFO(f"Invalid response: {req.content}")
                raise ConditionsDB.RequestError("{method} {url} returned invalid JSON response {}"
                                                .format(e, method=method, url=url))
        return req

    def get_globalTags(self):
        """Get a list of all globaltags. Returns a dictionary with the globaltag
        names and the corresponding ids in the database"""

        try:
            req = self.request("GET", "/globalTags")
        except ConditionsDB.RequestError as e:
            B2ERROR(f"Could not get the list of globaltags: {e}")
            return None

        result = {}
        for tag in req.json():
            result[tag["name"]] = tag

        return result

    def has_globalTag(self, name):
        """Check whether the globaltag with the given name exists."""

        try:
            self.request("GET", "/globalTag/{globalTagName}".format(globalTagName=encode_name(name)))
        except ConditionsDB.RequestError:
            return False

        return True

    def get_globalTagInfo(self, name):
        """Get the id of the globaltag with the given name. Returns either the
        id or None if the tag was not found"""

        try:
            req = self.request("GET", "/globalTag/{globalTagName}".format(globalTagName=encode_name(name)))
        except ConditionsDB.RequestError as e:
            B2ERROR(f"Cannot find globaltag '{name}': {e}")
            return None

        return req.json()

    def get_globalTagType(self, name):
        """
        Get the dictionary describing the given globaltag type (currently
        one of DEV or RELEASE). Returns None if tag type was not found.
        """
        try:
            req = self.request("GET", "/globalTagType")
        except ConditionsDB.RequestError as e:
            B2ERROR(f"Could not get list of valid globaltag types: {e}")
            return None

        types = {e["name"]: e for e in req.json()}

        if name in types:
            return types[name]

        B2ERROR("Unknown globaltag type: '{}', please use one of {}".format(name, ", ".join(types)))
        return None

    def create_globalTag(self, name, description, user):
        """
        Create a new globaltag
        """
        info = {"name": name, "description": description, "modifiedBy": user, "isDefault": False}
        try:
            req = self.request("POST", "/globalTag/DEV", f"Creating globaltag {name}", json=info)
        except ConditionsDB.RequestError as e:
            B2ERROR(f"Could not create globaltag {name}: {e}")
            return None

        return req.json()

    def get_all_iovs(self, globalTag, exp=None, run=None, message=None):
        """
        Return list of all payloads in the given globaltag where each element is
        a `PayloadInformation` instance

        Parameters:
            gobalTag (str): name of the globaltag
            exp (int): if given limit the list of payloads to the ones valid for
                the given exp,run combination
            run (int): if given limit the list of payloads to the ones valid for
                the given exp,run combination
            message (str): additional message to show when downloading the
                payload information. Will be directly appended to
                "Obtaining lists of iovs for globaltag {globalTag}"

        Warning:
            Both, exp and run, need to be given at the same time. Just supplying
            an experiment or a run number will not work
        """
        globalTag = encode_name(globalTag)
        if message is None:
            message = ""
        if exp is not None:
            msg = f"Obtaining list of iovs for globaltag {globalTag}, exp={exp}, run={run}{message}"
            req = self.request("GET", "/iovPayloads", msg, params={'gtName': globalTag, 'expNumber': exp, 'runNumber': run})
        else:
            msg = f"Obtaining list of iovs for globaltag {globalTag}{message}"
            req = self.request("GET", f"/globalTag/{globalTag}/globalTagPayloads", msg)
        all_iovs = []
        for item in req.json():
            payload = item["payload" if 'payload' in item else "payloadId"]
            if "payloadIov" in item:
                iovs = [item['payloadIov']]
            else:
                iovs = item['payloadIovs']

            for iov in iovs:
                all_iovs.append(PayloadInformation.from_json(payload, iov))

        all_iovs.sort()
        return all_iovs

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
            B2ERROR(f"Cannot get list of payloads: {e}")
            return {}

        result = {}
        for payload in req.json():
            module = payload["basf2Module"]["name"]
            checksum = payload["checksum"]
            result[(module, checksum)] = payload["payloadId"]

        return result

    def check_payloads(self, payloads, information="payloadId"):
        """
        Check for the existence of payloads in the database.

        Arguments:
            payloads (list((str,str))): A list of payloads to check for. Each
               payload needs to be a tuple of the name of the payload and the
               md5 checksum of the payload file.
            information (str): The information to be extracted from the
               payload dictionary

        Returns:
            A dictionary with the payload identifiers (name, checksum) as keys
            and the requested information as values for all payloads which are already
            present in the database.
        """

        search_query = [{"name": e[0], "checksum": e[1]} for e in payloads]
        try:
            req = self.request("POST", "/checkPayloads", json=search_query)
        except ConditionsDB.RequestError as e:
            B2ERROR(f"Cannot check for existing payloads: {e}")
            return {}

        result = {}
        for payload in req.json():
            module = payload["basf2Module"]["name"]
            checksum = payload["checksum"]
            result[(module, checksum)] = payload[information]

        return result

    def get_revisions(self, entries):
        """
        Get the revision numbers of payloads in the database.

        Arguments:
            entries (list): A list of payload entries.
               Each entry must have the attributes module and checksum.

        Returns:
            True if successful.
        """

        result = self.check_payloads([(entry.module, entry.checksum) for entry in entries], "revision")
        if not result:
            return False

        for entry in entries:
            entry.revision = result.get((entry.module, entry.checksum), 0)

        return True

    def create_payload(self, module, filename, checksum=None):
        """
        Create a new payload

        Args:
            module (str): name of the module
            filename (str): name of the file
            checksum (str): md5 hexdigest of the file. Will be calculated automatically if not given
        """
        if checksum is None:
            checksum = file_checksum(filename)

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
            B2ERROR(f"Could not create Payload: {e}")
            return None

        return req.json()["payloadId"]

    def create_iov(self, globalTagId, payloadId, firstExp, firstRun, finalExp, finalRun):
        """
        Create an iov.

        Args:
            globalTagId (int): id of the globaltag, obtain with get_globalTagId()
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
            B2ERROR(f"Could not create IOV: {e}")
            return None

        return req.json()["payloadIovId"]

    def get_iovs(self, globalTagName, payloadName=None):
        """Return existing iovs for a given tag name. It returns a dictionary
        which maps (payloadId, first runId, final runId) to iovId

        Parameters:
          globalTagName(str): Global tag name.
          payloadName(str):   Payload name (if None, selection by name is
                              not performed.
        """

        try:
            req = self.request("GET", "/globalTag/{globalTagName}/globalTagPayloads"
                               .format(globalTagName=encode_name(globalTagName)))
        except ConditionsDB.RequestError:
            # there could be just no iovs so no error
            return {}

        result = {}
        for payload in req.json():
            payloadId = payload["payloadId"]["payloadId"]
            if payloadName is not None:
                if payload["payloadId"]["basf2Module"]["name"] != payloadName:
                    continue
            for iov in payload["payloadIovs"]:
                iovId = iov["payloadIovId"]
                firstExp, firstRun = iov["expStart"], iov["runStart"]
                finalExp, finalRun = iov["expEnd"], iov["runEnd"]
                result[(payloadId, firstExp, firstRun, finalExp, finalRun)] = iovId

        return result

    def upload(self, filename, global_tag, normalize=False, ignore_existing=False, nprocess=1, uploaded_entries=None):
        """
        Upload a testing payload storage to the conditions database.

        Parameters:
          filename (str): filename of the testing payload storage file that should be uploaded
          global_tage (str): name of the globaltag to which the data should be uploaded
          normalize (bool/str): if True the payload root files will be normalized to have the same checksum for the same content,
                                if normalize is a string in addition the file name in the root file metadata will be set to it
          ignore_existing (bool): if True do not upload payloads that already exist
          nprocess (int): maximal number of parallel uploads
          uploaded_entries (list): the list of successfully uploaded entries

        Returns:
          True if the upload was successful
        """

        # first create a list of payloads
        from conditions_db.testing_payloads import parse_testing_payloads_file
        B2INFO(f"Reading payload list from {filename}")
        entries = parse_testing_payloads_file(filename)
        if entries is None:
            B2ERROR(f"Problems with testing payload storage file {filename}, exiting")
            return False

        if not entries:
            B2INFO(f"No payloads found in {filename}, exiting")
            return True

        B2INFO(f"Found {len(entries)} iovs to upload")

        # time to get the id for the globaltag
        tagId = self.get_globalTagInfo(global_tag)
        if tagId is None:
            return False
        tagId = tagId["globalTagId"]

        # now we could have more than one payload with the same iov so let's go over
        # it again and remove duplicates but keep the last one for each
        entries = sorted(set(reversed(entries)))

        if normalize:
            name = normalize if normalize is not True else None
            for e in entries:
                e.normalize(name=name)

        # so let's have a list of all payloads (name, checksum) as some payloads
        # might have multiple iovs. Each payload gets a list of all of those
        payloads = defaultdict(list)
        for e in entries:
            payloads[(e.module, e.checksum)].append(e)

        existing_payloads = {}
        existing_iovs = {}

        def upload_payload(item):
            """Upload a payload file if necessary but first check list of existing payloads"""
            key, entries = item
            if key in existing_payloads:
                B2INFO(f"{key[0]} (md5:{key[1]}) already existing in database, skipping.")
                payload_id = existing_payloads[key]
            else:
                entry = entries[0]
                payload_id = self.create_payload(entry.module, entry.filename, entry.checksum)
                if payload_id is None:
                    return False

                B2INFO(f"Created new payload {payload_id} for {entry.module} (md5:{entry.checksum})")

            for entry in entries:
                entry.payload = payload_id

            return True

        def create_iov(entry):
            """Create an iov if necessary but first check the list of existing iovs"""
            if entry.payload is None:
                return None

            iov_key = (entry.payload,) + entry.iov_tuple
            if iov_key in existing_iovs:
                entry.iov = existing_iovs[iov_key]
                B2INFO(f"IoV {entry.iov_tuple} for {entry.module} (md5:{entry.checksum}) already existing in database, skipping.")
            else:
                entry.payloadIovId = self.create_iov(tagId, entry.payload, *entry.iov_tuple)
                if entry.payloadIovId is None:
                    return False

                B2INFO(f"Created IoV {entry.iov_tuple} for {entry.module} (md5:{entry.checksum})")

            return entry

        # multithreading for the win ...
        with ThreadPoolExecutor(max_workers=nprocess) as pool:
            # if we want to check for existing payloads/iovs we schedule the download of
            # the full payload list. And write a message as each completes
            if not ignore_existing:
                B2INFO("Downloading information about existing payloads and iovs...")
                futures = []
                existing_iovs = {}
                existing_payloads = {}

                def create_future(iter, func, callback=None):
                    fn = pool.submit(iter, func)
                    if callback is not None:
                        fn.add_done_callback(callback)
                    futures.append(fn)

                def update_iovs(iovs):
                    existing_iovs.update(iovs.result())
                    B2INFO(f"Found {len(existing_iovs)} existing iovs in {global_tag}")

                def update_payloads(payloads):
                    existing_payloads.update(payloads.result())
                    B2INFO(f"Found {len(existing_payloads)} existing payloads")

                create_future(self.get_iovs, global_tag, update_iovs)
                # checking existing payloads should not be done with too many at once
                for chunk in chunks(payloads.keys(), 1000):
                    create_future(self.check_payloads, chunk, update_payloads)

                futures_wait(futures)

            # upload payloads
            failed_payloads = sum(0 if result else 1 for result in pool.map(upload_payload, payloads.items()))
            if failed_payloads > 0:
                B2ERROR(f"{failed_payloads} payloads could not be uploaded")

            # create IoVs
            failed_iovs = 0
            for entry in pool.map(create_iov, entries):
                if entry:
                    if uploaded_entries is not None:
                        uploaded_entries.append(entry)
                else:
                    failed_iovs += 1
            if failed_iovs > 0:
                B2ERROR(f"{failed_iovs} IoVs could not be created")

        # update revision numbers
        if uploaded_entries is not None:
            self.get_revisions(uploaded_entries)

        return failed_payloads + failed_iovs == 0

    def staging_request(self, filename, normalize, data, password):
        """
        Upload a testing payload storage to a staging globaltag and create or update a jira issue

        Parameters:
          filename (str): filename of the testing payload storage file that should be uploaded
          normalize (bool/str): if True the payload root files will be
            normalized to have the same checksum for the same content, if
            normalize is a string in addition the file name in the root file
            metadata will be set to it
          data (dict): a dictionary with the information provided by the user:

            * task: category of globaltag, either master, online, prompt, data, mc, or analysis
            * tag: the globaltage name
            * request: type of request, either Update, New, or Modification. The latter two imply task == master because
              if new payload classes are introduced or payload classes are modified then they will first be included in
              the master globaltag. Here a synchronization of code and payload changes has to be managed.
              If new or modified payload classes should be included in other globaltags they must already be in a release.
            * pull-request: number of the pull request containing new or modified payload classes,
              only for request == New or Modified
            * backward-compatibility: description of what happens if the old payload is encountered by the updated code,
              only for request == Modified
            * forward-compatibility: description of what happens if a new payload is encountered by the existing code,
              only for request == Modified
            * release: the required release version
            * reason: the reason for the request
            * description: a detailed description for the globaltag manager
            * issue: identifier of an existing jira issue (optional)
            * user: name of the user
            * time: time stamp of the request

          password: the password for access to jira or the access token and secret for oauth access

        Returns:
          True if the upload and jira issue creation/upload was successful
        """

        # determine the staging globaltag name
        data['tag'] = upload_global_tag(data['task'])
        if data['tag'] is None:
            data['tag'] = f"staging_{data['task']}_{data['user']}_{data['time']}"

        # create the staging globaltag if it does not exists yet
        if not self.has_globalTag(data['tag']):
            if not self.create_globalTag(data['tag'], data['reason'], data['user']):
                return False

        # upload the payloads
        B2INFO(f"Uploading testing database {filename} to globaltag {data['tag']}")
        entries = []
        if not self.upload(filename, data['tag'], normalize, uploaded_entries=entries):
            return False

        # get the dictionary for the jira issue creation/update
        if data['issue']:
            issue = data['issue']
        else:
            issue = jira_global_tag_v2(data['task'])
        if issue is None:
            issue = {"components": [{"name": "globaltag"}]}

        # create jira issue text from provided information
        if type(issue) is tuple:
            description = issue[1].format(**data)
            issue = issue[0]
        else:
            description = f"""
|*Upload globaltag*      | {data['tag']} |
|*Request reason*        | {data['reason']} |
|*Required release*      | {data['release']} |
|*Type of request*       | {data['request']} |
"""
            if 'pull-request' in data.keys():
                description += f"|*Pull request* | \\#{data['pull-request']} |\n"
            if 'backward-compatibility' in data.keys():
                description += f"|*Backward compatibility* | \\#{data['backward-compatibility']} |\n"
            if 'forward-compatibility' in data.keys():
                description += f"|*Forward compatibility* | \\#{data['forward-compatibility']} |\n"
            description += '|*Details* |' + ''.join(data['details']) + ' |\n'
            if data['task'] == 'online':
                description += '|*Impact on data taking*|' + ''.join(data['data_taking']) + ' |\n'

        # add information about uploaded payloads/IoVs
        description += '\nPayloads\n||Name||Revision||IoV||\n'
        for entry in entries:
            description += f"|{entry.module} | {entry.revision} | ({entry.iov_str()}) |\n"

        # create a new issue
        if type(issue) is dict:
            issue["description"] = description
            if "summary" in issue.keys():
                issue["summary"] = issue["summary"].format(**data)
            else:
                issue["summary"] = f"Globaltag request for {data['task']} by {data['user']} at {data['time']}"
            if "project" not in issue.keys():
                issue["project"] = {"key": "BII"}
            if "issuetype" not in issue.keys():
                issue["issuetype"] = {"name": "Task"}
            if data["task"] == "master":
                issue["labels"] = ["TUPPR"]

            B2INFO(f"Creating jira issue for {data['task']} globaltag request")
            if isinstance(password, str):
                response = requests.post('https://agira.desy.de/rest/api/latest/issue', auth=(data['user'], password),
                                         json={'fields': issue})
            else:
                fields = {'issue': json.dumps(issue)}
                if 'user' in data.keys():
                    fields['user'] = data['user']
                if password:
                    fields['token'] = password[0]
                    fields['secret'] = password[1]
                response = requests.post('https://b2-master.belle2.org/cgi-bin/jira_issue.py', data=fields)
            if response.status_code in range(200, 210):
                B2INFO(f"Issue successfully created: https://agira.desy.de/browse/{response.json()['key']}")
            else:
                B2ERROR('The creation of the issue failed: ' + requests.status_codes._codes[response.status_code][0])
                return False

        # comment on an existing issue
        else:
            # Let's make sure all assignees of new issues are added as watchers
            # in that case, otherwise they might never find out
            new_issue_config = jira_global_tag_v2(data['task'])
            if isinstance(new_issue_config, dict) and "assignee" in new_issue_config:
                user = new_issue_config['assignee'].get('name', None)
                if user is not None and isinstance(password, str):
                    response = requests.post(f'https://agira.desy.de/rest/api/latest/issue/{issue}/watchers',
                                             auth=(data['user'], password), json=user)
                    if response.status_code in range(200, 210):
                        B2INFO(f"Added {user} as watcher to {issue}")
                    else:
                        B2WARNING(f"Could not add {user} as watcher to {issue}: {response.status_code}")

            B2INFO(f"Commenting on jira issue {issue} for {data['task']} globaltag request")
            if isinstance(password, str):
                response = requests.post('https://agira.desy.de/rest/api/latest/issue/%s/comment' % issue,
                                         auth=(data['user'], password), json={'body': description})
            else:
                fields = {'id': issue, 'user': user, 'comment': description}
                if password:
                    fields['token'] = password[0]
                    fields['secret'] = password[1]
                response = requests.post('https://b2-master.belle2.org/cgi-bin/jira_issue.py', data=fields)
            if response.status_code in range(200, 210):
                B2INFO(f"Issue successfully updated: https://agira.desy.de/browse/{issue}")
            else:
                B2ERROR('The commenting of the issue failed: ' + requests.status_codes._codes[response.status_code][0])
                return False

        return True


def require_database_for_test(timeout=60, base_url=None):
    """Make sure that the database is available and skip the test if not.

    This function should be called in test scripts if they are expected to fail
    if the database is down. It either returns when the database is ok or it
    will signal test_basf2 that the test should be skipped and exit
    """
    import sys
    if os.environ.get("BELLE2_CONDB_GLOBALTAG", None) == "":
        raise Exception("Access to the Database is disabled")
    base_url_list = ConditionsDB.get_base_urls(base_url)
    for url in base_url_list:
        try:
            req = requests.request("HEAD", url.rstrip('/') + "/v2/globalTags")
            req.raise_for_status()
        except requests.RequestException as e:
            B2WARNING(f"Problem connecting to {url}:\n     {e}\n Trying next server ...")
        else:
            break
    else:
        print("TEST SKIPPED: No working database servers configured, giving up", file=sys.stderr)
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
