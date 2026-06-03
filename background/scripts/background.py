#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import glob
import os

from basf2 import Module, Path, B2ERROR, B2INFO
from basf2.utils import pretty_print_table
from rawdata import add_unpackers
from gdltrigger import filter_trigger_abort_gaps


BGO_OBJECTS = (
    'EventLevelTriggerTimeInfo',
    'PXDDigits',
    'SVDShaperDigits',
    'CDCHits',
    'TOPDigits',
    'TOPInjectionVeto'
    'ARICHDigits',
    'ECLWaveforms',
    'KLMDigits',
    'TRGECLBGTCHits',
    'TRGSummary'
)


class SelectTRGTypes(Module):
    '''Select events according to given trigger types.'''

    def __init__(self, trg_types=None):
        '''Constructor.'''
        super().__init__()
        #: The trigger types
        self.trg_types = trg_types

    def initialize(self, trg_types=None):
        '''Initialize the module.'''
        from ROOT import Belle2
        #: The trigger summary object
        self.trg_summary = Belle2.PyStoreObj('TRGSummary')
        self.trg_summary.isRequired()

    def event(self):
        '''Event processing.'''
        self.return_value(0)

        if not self.trg_summary.isValid():
            # This should never happen: let's report without crashing the processing
            B2ERROR('TRGSummary is not available: the event is discarded.')
            return

        for trg_type in self.trg_types:
            if self.trg_summary.getTimType() == trg_type:
                self.return_value(1)
                return


def get_trigger_types_for_bgo():
    '''Get the default trigger types to be used for the Beam Background Overlay (BGO) production.'''
    from ROOT import Belle2

    trg_types = [
        Belle2.TRGSummary.TTYP_DPHY,  # 5 -> delayed physics for background
        Belle2.TRGSummary.TTYP_RAND,  # 7 -> random trigger events
    ]
    return trg_types


def get_background_files(folder=None, output_file_info=True):
    """ Loads the location of the background files from the environmant variable
    BELLE2_BACKGROUND_DIR which is set on the validation server and ensures that background
    files exist and returns the list of background files which
    can be directly used with add_simulation() :

    >>> add_simulation(main, bkgfiles=background.get_background_files())

    Will fail with an assert if no background folder set or if no background file was
    found in the set folder.

    Parameters:
        folder (str): A specific folder to search for background files can be given as an optional parameter
        output_file_info (str): If true, a list of the found background files and there size will be printed
                                This is useful to understand later which background campaign has been used
                                to simulate events.
    """

    env_name = 'BELLE2_BACKGROUND_DIR'
    bg = None

    if folder is None:
        if env_name not in os.environ:
            raise RuntimeError("Environment variable {} for background files not set. Terminanting this script.".format(env_name))
        folder = os.environ[env_name]

    bg = glob.glob(folder + '/*.root')

    if len(bg) == 0:
        raise RuntimeError("No background files found in folder {} . Terminating this script.".format(folder))

    B2INFO("Background files loaded from folder {}".format(folder))

    # sort for easier comparison
    bg = sorted(bg)

    if output_file_info:
        bg_sizes = [os.path.getsize(f) for f in bg]
        # reformat to work with pretty_print_table
        table_rows = [list(entry) for entry in zip(bg, bg_sizes)]
        table_rows.insert(0, ["- Background file name -", "- file size -"])

        pretty_print_table(table_rows, [0, 0])

    return bg


def add_output(path, bgType, realTime, sampleType, phase=3, fileName='output.root', excludeBranches=None):
    '''
    A function to be used for output of BG simulation.
    @param path path name
    @param bgType background type, to get available types: basf2 -m BeamBkgTagSetter
    @param realTime equivalent time of superKEKB running in [ns]
    @param sampleType 'study' (for BG studies) or 'usual', 'PXD', 'ECL' (for BG mixer)
    @param specify the Phase, 1 for Phase 1, 2 for Phase 2, and 3 for Physics Run or Phase 3
    @param fileName optional file name, can be overridden by basf2 -o
    '''
    if excludeBranches is None:
        excludeBranches = []

    if sampleType == 'study':
        madeFor = ''
        branches = []
    elif sampleType == 'usual' and phase == 3:
        madeFor = ''
        branches = [
            'PXDSimHits',
            'SVDSimHits',
            'CDCSimHits',
            'TOPSimHits',
            'ARICHSimHits',
            'ECLHits',
            'KLMSimHits',
        ]
    elif sampleType == 'usual' and phase == 2:
        madeFor = ''
        branches = [
            'PXDSimHits',
            'SVDSimHits',
            'CDCSimHits',
            'TOPSimHits',
            'ARICHSimHits',
            'ECLHits',
            'KLMSimHits',
            'CLAWSSimHits',
            'FANGSSimHits',
            'PlumeSimHits',
            'BeamabortSimHits',
            'PindiodeSimHits',
            'QcsmonitorSimHits',
            'He3tubeSimHits',
            'MicrotpcSimHits',
        ]
    elif sampleType == 'usual' and phase == 1:
        madeFor = ''
        branches = [
            'ClawSimHits',
            'BeamabortSimHits',
            'PindiodeSimHits',
            'QcsmonitorSimHits',
            'He3tubeSimHits',
            'MicrotpcSimHits',
            'BgoSimHits',
            'CsiSimHits',
        ]
    elif sampleType == 'ECL':
        madeFor = 'ECL'
        branches = ['ECLHits']
    elif sampleType == 'PXD':
        madeFor = 'PXD'
        branches = ['PXDSimHits']
    else:
        madeFor = ''
        branches = []
        B2ERROR('add_output - invalid value of argument sampleType: %s'
                % sampleType)

    # Set background tag in SimHits and add BackgroundMetaData into persistent tree
    tagSetter = path.add_module('BeamBkgTagSetter', backgroundType=bgType, realTime=realTime,
                                specialFor=madeFor, Phase=phase)

    # Write out only non-empty events when producing samples for BG mixer
    if sampleType != 'study':
        emptyPath = Path()
        tagSetter.if_false(emptyPath)

    # Output to file. We don't need a TTreeIndex for background files and memory
    # consumption can be improved by setting a lower autoFlushSize so that
    # fewer and or smaller amounts of data have to be read for each GetEntry()
    path.add_module('RootOutput', outputFileName=fileName, branchNames=branches, excludeBranchNames=excludeBranches,
                    buildIndex=False, autoFlushSize=-500000)


def add_bgo_modules(path, additionalBranches=None):
    """
    This function adds to the path all the necessary modules to produce Beam Background Overlay (BGO) files
    starting from raw data.
    This function already adds the ``RootOutput`` module with all the branch names correctly set.

    Arguments:
        path (Path): Path to add module to
        additionalBranches (list): Additional objects/arrays of event durability to save
    """

    empty = Path()

    # Gearbox
    path.add_module('Gearbox')

    # Geometry
    path.add_module('Geometry')

    # Unpack TRGSummary
    path.add_module('TRGGDLUnpacker')
    path.add_module('TRGGDLSummary')

    # Show progress of processing
    path.add_module('Progress')

    # Select specific triggered events
    selector = path.add_module(SelectTRGTypes(trg_types=get_trigger_types_for_bgo()))
    selector.if_false(empty)

    # Filter away the events falling in the trigger abort gaps
    filter_trigger_abort_gaps(path)

    # Unpack detector data
    add_unpackers(path, components=['PXD', 'SVD', 'CDC', 'ECL', 'TOP', 'ARICH', 'KLM'])

    # Convert ECLDsps to ECLWaveforms
    compress = path.add_module('ECLCompressBGOverlay', CompressionAlgorithm=3)
    compress.if_false(empty)

    # Shift the time of KLMDigits
    path.add_module('KLMDigitTimeShifter')

    # ECL trigger unpacker and BGOverlay dataobject
    path.add_module('TRGECLUnpacker')
    path.add_module('TRGECLBGTCHit')

    # Output
    branches = list(BGO_OBJECTS)
    if additionalBranches:
        branches += additionalBranches
    path.add_module('RootOutput', branchNames=branches)
