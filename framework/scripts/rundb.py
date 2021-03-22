#!/usr/bin/env python3
# -*- coding: utf-8 -*-

'''
rundb - Helper classes for retrieving information from the RunDB
----------------------------------------------------------------

This modules contains classes useful to deal with the RunDB:

* `RunDB`, a simple API class to just get run information from the RunDB
'''

import requests
import getpass


class RunDB:
    """
    Simple API class to just get run information from the RunDB.

    Parameters:
        apikey (str): RunDB API key (see `here <https://questions.belle2.org/question/9847/obtaining-api-key-for-rundb/>`_
                      how to get one)
        username (str): DESY username
    """

    #: URL of where the RunDB is hosted
    URL = "https://rundb.belle2.org"

    def __init__(self, apikey=None, username=None):
        """Create an object and setup authentication."""
        #: session object for connection to the RunDB
        self._session = requests.Session()
        if apikey is None:
            # If no specific username use the local system username
            if username is None:
                username = getpass.getuser()
            # If we don't have an api key use desy username/password
            self._session.auth = (username, getpass.getpass(f"DESY password ({username}): "))
        else:
            # Otherwise use the api key
            self._session.headers.update({'Authorization': f'Bearer {apikey}'})
        # And request json output ...
        self._session.headers.update({'Content-Type': 'application/json'})

    def _pagination(self, request):
        """Deal with API pagination of an initial request to the API.

        It will return all the objects from all pages lazily requesting new pages
        as objects are consumed. Will work for all list requests to the server.
        """
        while True:
            # check the return value and raise exception on error
            request.raise_for_status()
            # and otherwise get the json
            result = request.json()
            # and return the objects one by one by yielding objects from the list
            # of results
            yield from result['results']
            # check if there's a next page, if not done
            if result['next'] is None:
                break
            # otherwise continue with the next page
            # yees, global variable ...
            request = self._session.get(result['next'])

    def get_run_info(self, **search_params):
        """Return the run information from the run registry.

        All arguments are forwarded to the run registry ``/run/`` method
        documented at the following `link <https://rundb.belle2.org/rest/v1/swagger/>`_.
        Please check there for up to date documentation, at the time of
        this writing the supported arguments are:

          * min_experiment (int)
          * min_run (int)
          * max_experiment (int)
          * max_run (int)
          * min_date (iso8601 date string, e.g. 2020-05-06)
          * max_date (iso8601 date string, e.g. 2020-05-06)
          * all_detectors_running (bool)
          * expand (bool): If true return full run objects, not just a summary
            links to the run objects

        If ``expand=False`` you can request the full objects for each run by calling
        `get_details` with the returned run summary object as argument.
        ``expand=False`` is much faster if no further details are needed but
        getting the details in a separate step for many many runs will be slow
        so depending on how many runs are selected one or the other may be
        faster.
        """
        req = self._session.get(f'{self.URL}/rest/v1/runs/', params=search_params)
        return self._pagination(req)

    def get_details(self, run_summary):
        """
        Return details for a run summary object returned from `get_run_info`
        if ``expand`` was not set to True

        Parameters:
            run_summary: a run summary object returned from `get_run_info`
        """
        # Get the url object
        req = self._session.get(run_summary['url'])
        # Raise an exception in case of any error
        req.raise_for_status()
        # And return the json object
        return req.json()
