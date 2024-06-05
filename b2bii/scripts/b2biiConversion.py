#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import basf2 as b2
import b2bii
from modularAnalysis import setAnalysisConfigParams
import os
import re
import requests
from ctypes import cdll


def setupBelleDatabaseServer():
    """
    Sets the Belle DB server to the one recommended in /sw/belle/local/var/belle_postgres_server.

    If the user does not have the access rights to the above file, the can01 server is set by default.
    """
    belleDBServerFile = '/sw/belle/local/var/belle_postgres_server'
    belleDBServer = 'can01'

    try:
        with open(belleDBServerFile) as f:
            belleDBServer = (f.read()).strip()
    except OSError:
        pass

    os.environ['PGUSER'] = 'g0db'
    os.environ['BELLE_POSTGRES_SERVER'] = belleDBServer


def convertBelleMdstToBelleIIMdst(inputBelleMDSTFile, applySkim=True,
                                  saveResultExtraInfo=False,
                                  useBelleDBServer=None,
                                  convertBeamParameters=True,
                                  generatorLevelReconstruction=False,
                                  generatorLevelMCMatching=False,
                                  path=None, entrySequences=None,
                                  matchType2E9oE25Threshold=-1.1,
                                  enableNisKsFinder=True,
                                  HadronA=True, HadronB=True,
                                  enableRecTrg=False, enableEvtcls=True,
                                  SmearTrack=2, enableLocalDB=True,
                                  convertNbar=False):
    """
    Loads Belle MDST file and converts in each event the Belle MDST dataobjects to Belle II MDST
    data objects and loads them to the StoreArray.

    Args:
        inputBelleMDSTFile (str): Name of the file(s) to be loaded.
        applySkim (bool): Apply skim conditions in B2BIIFixMdst.
        saveResultExtraInfo (bool): Save B2BIIFixMdst module return value as EventExtraInfo.
        useBelleDBServer (str): None to use the recommended BelleDB server.
        convertBeamParameters (bool): Convert beam parameters or use information stored in Belle II database.
        generatorLevelReconstruction (bool): Enables to bypass skims and corrections applied in B2BIIFixMdst.
        generatorLevelMCMatching (bool): Enables to switch MCTruth matching to generator-level particles.
            This is recommended for analyses with gammas in the final state.
        path (basf2.Path): Path to add modules in.
        entrySequences (list(str)): The number sequences (e.g. 23:42,101) defining
            the entries which are processed for each inputFileName.
        matchType2E9oE25Threshold (float): Clusters with a E9/E25 value above this threshold are classified as neutral
            even if tracks are matched to their connected region (matchType == 2 in basf).
        enableNisKsFinder (bool): Enables to convert nisKsFinder information.
        HadronA (bool): Enables to switch on HadronA skim in B2BIIFixMdst module.
        HadronB (bool): Enables to switch on HadronB skim in B2BIIFixMdst module.
        enableRecTrg (bool): Enables to convert RecTrg_summary3 table.
        enableEvtcls (bool): Enables to convert Evtcls and Evtcls_hadronic tables.
        SmearTrack (float): Smear the MC tracks to match real data.
            Apart from the recommended default value of 2 it can also be set to 1.
            Details about the difference between those two options can be found
            `here <https://belle.kek.jp/secured/wiki/doku.php?id=physics:charm:tracksmearing>`_.
            Set to 0 to skip smearing (automatically set to 0 internally for real data).
        enableLocalDB (bool): Enables to use local payloads.
        convertNbar (bool): Enables conversion of anti-n0:mdst.
    """

    # If we are on KEKCC make sure we load the correct NeuroBayes library
    try:
        cdll.LoadLibrary(
            '/sw/belle/local/neurobayes/lib/libNeuroBayesCore_shared.so')
    except BaseException:
        pass

    if useBelleDBServer is None:
        setupBelleDatabaseServer()
    else:
        os.environ['BELLE_POSTGRES_SERVER'] = useBelleDBServer

    b2.B2INFO('Belle DB server is set to: ' + os.environ['BELLE_POSTGRES_SERVER'])

    setAnalysisConfigParams({'mcMatchingVersion': 'Belle'}, path)

    b2bii.setB2BII()

    if enableLocalDB is True:
        b2.conditions.metadata_providers = ["/sw/belle/b2bii/database/conditions/b2bii.sqlite"]
        b2.conditions.payload_locations = ["/sw/belle/b2bii/database/conditions/"]
    else:
        b2.B2WARNING(
            "B2BII is accessing the payloads from the conditions database.\n"
            "The recommended procedure is to use the offline database and it is significantly\n"
            "faster than using the global database.\n"
            "If you need the payloads which are not included in the current offline database,\n"
            "please contact b2bii librarian.")

    input = b2.register_module('B2BIIMdstInput')
    if inputBelleMDSTFile is not None:
        input.param('inputFileNames', parse_process_url(inputBelleMDSTFile))
    if entrySequences is not None:
        input.param('entrySequences', entrySequences)
    # input.logging.set_log_level(LogLevel.DEBUG)
    # input.logging.set_info(LogLevel.DEBUG, LogInfo.LEVEL | LogInfo.MESSAGE)
    path.add_module(input)

    # we need magnetic field which is different than default.
    # shamelessly copied from analysis/scripts/modularAnalysis.py:inputMdst
    from ROOT import Belle2  # reduced scope of potentially-misbehaving import
    from ROOT.Math import XYZVector
    field = Belle2.MagneticField()
    field.addComponent(
        Belle2.MagneticFieldComponentConstant(
            XYZVector(0, 0, 1.5 * Belle2.Unit.T)))
    Belle2.DBStore.Instance().addConstantOverride("MagneticField", field, False)

    if (not generatorLevelReconstruction):
        # Fix MSDT Module
        fix = b2.register_module('B2BIIFixMdst')
        # fix.logging.set_log_level(LogLevel.DEBUG)
        # fix.logging.set_info(LogLevel.DEBUG, LogInfo.LEVEL | LogInfo.MESSAGE)
        fix.param('SaveResultExtraInfo', saveResultExtraInfo)
        # Hadron skim settings
        fix.param('HadronA', HadronA)
        fix.param('HadronB', HadronB)
        fix.param('Smear_trk', SmearTrack)
        if (HadronA is not True and HadronB is True):
            b2.B2WARNING(
                'The Hadron A skim is turned off.'
                'However, its requirements are still applied since the HadronB(J) skim, which includes them, is turned on.')
        path.add_module(fix)

        if(applySkim):
            emptypath = b2.create_path()
            # discard 'bad events' marked by fixmdst
            fix.if_value('<=0', emptypath)
        else:
            b2.B2INFO('applySkim is set to be False.'
                      'No bad events marked by fixmdst will be discarded.'
                      'Corrections will still be applied.')
    else:
        b2.B2INFO('Perform generator level reconstruction, no corrections or skims in fix_mdst will be applied.')
    # Convert MDST Module
    convert = b2.register_module('B2BIIConvertMdst')
    convert.param('convertBeamParameters', convertBeamParameters)
    if (generatorLevelMCMatching):
        convert.param('mcMatchingMode', 'GeneratorLevel')
    convert.param("matchType2E9oE25Threshold", matchType2E9oE25Threshold)
    convert.param("nisKsInfo", enableNisKsFinder)
    convert.param("RecTrg", enableRecTrg)
    convert.param("convertEvtcls", enableEvtcls)
    convert.param("convertNbar", convertNbar)
    # convert.logging.set_log_level(LogLevel.DEBUG)
    # convert.logging.set_info(LogLevel.DEBUG, LogInfo.LEVEL | LogInfo.MESSAGE)
    path.add_module(convert)
    if convertNbar:
        b2.conditions.append_globaltag('BellePID')
        path.add_module('BelleNbarMVA', particleList='anti-n0:mdst', identifier='nbarMVA')


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
            b2.B2ERROR(
                f"Could not parse url '{url}': no such file or directory")
            return []

    # regular expression to find process_event lines in html response
    process_event = re.compile(br"^\s*process_event\s+(.*)\s+0$", re.MULTILINE)
    # get the url
    try:
        request = requests.get(url)
        request.raise_for_status()
        return [e.decode("ASCII")
                for e in process_event.findall(request.content)]
    except (requests.ConnectionError, requests.HTTPError) as e:
        b2.B2ERROR(
            f"Failed to connect to '{url}': {str(e)}")

    return []
