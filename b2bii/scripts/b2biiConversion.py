#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
from modularAnalysis import analysis_main
import os
import re
import requests
import http


def convertBelleMdstToBelleIIMdst(inputBelleMDSTFile, applyHadronBJSkim=True, path=analysis_main):
    """
    Loads Belle MDST file and converts in each event the Belle MDST dataobjects to Belle II MDST
    data objects and loads them to the StoreArray.
    """

    input = register_module('B2BIIMdstInput')
    input.param('inputFileNames', parse_process_url(inputBelleMDSTFile))
    # input.logging.set_log_level(LogLevel.DEBUG)
    # input.logging.set_info(LogLevel.DEBUG, LogInfo.LEVEL | LogInfo.MESSAGE)
    analysis_main.add_module(input)

    # Fix MSDT Module
    fix = register_module('B2BIIFixMdst')
    # fix.logging.set_log_level(LogLevel.DEBUG)
    # fix.logging.set_info(LogLevel.DEBUG, LogInfo.LEVEL | LogInfo.MESSAGE)
    analysis_main.add_module(fix)

    if(applyHadronBJSkim):
        emptypath = create_path()
        fix.if_value('<=0', emptypath)  # discard 'bad events' marked by fixmdst

    # Convert MDST Module
    convert = register_module('B2BIIConvertMdst')
    # convert.logging.set_log_level(LogLevel.DEBUG)
    # convert.logging.set_info(LogLevel.DEBUG, LogInfo.LEVEL | LogInfo.MESSAGE)
    analysis_main.add_module(convert)


def parse_process_url(url):
    """
    Handle process_url style requests.
    If url is a list just return it without looking at it. If the url starts
    with http(s):// return the list of files provided by that url, otherwise
    check if a file with the name given as url exists and return this as a list
    with one item. If not such file exists return an empty list

    >>> parse_process_url(["foo", "bar"])
    ['foo', 'bar']
    >>> parse_process_url("/dev/null")
    ['/dev/null']
    >>> parse_process_url("missing_file.mdst")
    []
    >>> parse_process_url("http://bweb3.cc.kek.jp/mdst.php?ex=55&rs=1&re=5&skm=HadronBorJ&dt=on_resonance&bl=caseB") \
        # doctest: +NORMALIZE_WHITESPACE
    ['/group/belle/bdata_b/dstprod/dat/e000055/HadronBJ/0127/on_resonance/00/HadronBJ-e000055r000003-b20090127_0910.mdst',
     '/group/belle/bdata_b/dstprod/dat/e000055/HadronBJ/0127/on_resonance/00/HadronBJ-e000055r000004-b20090127_0910.mdst',
     '/group/belle/bdata_b/dstprod/dat/e000055/HadronBJ/0127/on_resonance/00/HadronBJ-e000055r000005-b20090127_0910.mdst']

    Some files are missing and thus marked with "### LOST ###", make sure we don't pick them up

    >>> parse_process_url("http://bweb3.cc.kek.jp/mdst.php?ex=65&rs=1&re=30&skm=ypipi")
    []

    If the URL cannot be found an error is raised and an empty list is returned.
    Also for other http errors or non existing urls

    >>> parse_process_url("http://httpbin.org/status/404")
    []
    >>> parse_process_url("http://httpbin.org/status/500")
    []
    >>> parse_process_url("http://nosuchurl")
    []
    """

    if isinstance(url, list) or isinstance(url, tuple):
        return url

    if not url.startswith("http://") and not url.startswith("https://"):
        if os.path.exists(url):
            return [url]
        else:
            B2ERROR("Could not parse url '{0}': no such file or directory".format(url))
            return []

    # regular expression to find process_event lines in html response
    process_event = re.compile(br"^\s*process_event\s+(.*)\s+0$", re.MULTILINE)
    # get the url
    try:
        request = requests.get(url)
        request.raise_for_status()
        return [e.decode("ASCII") for e in process_event.findall(request.content)]
    except (requests.ConnectionError, requests.HTTPError) as e:
        B2ERROR("Failed to connect to '{url}': {message}".format(url=url, message=str(e)))

    return []
