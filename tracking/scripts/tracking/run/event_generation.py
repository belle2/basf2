#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os

import basf2
import simulation
import beamparameters

from . import utilities
from .minimal import MinimalRun
import tracking.adjustments as adjustments

import argparse
import logging


def get_logger():
    return logging.getLogger(__name__)


# Standard run for generating or reading event #
################################################

class ReadOrGenerateEventsRun(MinimalRun):
    description = "Simulate events using various generator and detector setups from command line."
    # Declarative section which can be redefined in a subclass
    generator_module = None
    detector_setup = "Default"
    bkg_files = []
    components = None
    disable_deltas = False
    simulate_only = False

    def create_argument_parser(self, **kwds):
        argument_parser = super().create_argument_parser(**kwds)

        setup_argument_group = argument_parser.add_argument_group("Detector setup arguments")
        setup_argument_group.add_argument(
            '-d',
            '--detector',
            dest='detector_setup',
            default=argparse.SUPPRESS,
            metavar='DETECTOR_SETUP_NAME',
            choices=utilities.NonstrictChoices(detector_setups_by_short_name.keys()),
            help=('Name of the detector setup to be used')
        )

        setup_argument_group.add_argument(
            '-c',
            '--component',
            dest='components',
            nargs='+',
            default=argparse.SUPPRESS,
            metavar='COMPONENTS',
            action='store',
            help=('Overrides the components of the detector setup')
        )

        generator_argument_group = argument_parser.add_argument_group("Generator arguments")
        generator_argument_group.add_argument(
            '-g',
            '--generator',
            dest='generator_module',
            default=argparse.SUPPRESS,
            metavar='GENERATOR_NAME',
            choices=utilities.NonstrictChoices(valid_generator_short_names),
            help='Name module or short name of the generator to be used.',
        )

        simulation_argument_group = argument_parser.add_argument_group("Generator arguments")
        simulation_argument_group.add_argument(
            '-b',
            '--bkg-file',
            dest='bkg_files',
            default=self.bkg_files,
            action='append',
            metavar='BACKGROUND_DIRECTORY',
            help='Path to folder of files or to a file containing the background to be used. ' +
                 'Can be given multiple times.',
        )

        simulation_argument_group.add_argument(
            '--disable-deltas',
            action='store_true',
            help='Disable the generation of delta rays in the simulation'
        )

        return argument_parser

    def create_path(self):
        path = super().create_path()

        # Gearbox & Geometry must always be registered
        path.add_module("Gearbox")
        path.add_module("Geometry")
        if self.detector_setup:
            detector_setup = self.detector_setup
            detector_setup_function = detector_setups_by_short_name[detector_setup]
            components = detector_setup_function(path)

        if self.components:
            adjustments.adjust_module(path, "Geometry", components=self.components)
            components = self.components

        # Only generate events if no input file has been provided
        if self.root_input_file is None:
            # Check if generator means a decay file
            if isinstance(self.generator_module, str) and utilities.find_file(self.generator_module):
                dec_file_path = utilities.find_file(self.generator_module)
                add_evtgen_generator(path, dec_file_path)
            else:
                # All other possibilities
                utilities.extend_path(path,
                                      self.generator_module,
                                      generators_by_short_name,
                                      allow_function_import=True)

        # Only simulate if generator is setup
        if self.root_input_file is None:
            bkg_file_paths = get_bkg_file_paths(self.bkg_files)

            simulation.add_simulation(path,
                                      components=components,
                                      bkgfiles=bkg_file_paths)

            if self.disable_deltas:
                adjustments.disable_deltas(path)

            # Catch if no generator is added, no background should be simulated and events
            # are not read from a file.
            if not bkg_file_paths and self.generator_module is None:
                raise RuntimeError('Need at least one of root_input_file,'
                                   ' generator_module or bkg_files specified.')

        return path


class StandardEventGenerationRun(ReadOrGenerateEventsRun):
    generator_module = "EvtGenInput"


# Default settings and shorthand names for generator with specific settings #
#############################################################################

# PDG code of an electorn
electron_pdg_code = 11

# PDG code of a muon
muon_pdg_code = 13

# PDG code of a tau
tau_pdg_code = 15


def add_single_gun_generator(path):
    """Add ParticleGun with a single muon"""
    path.add_module("ParticleGun",
                    pdgCodes=[muon_pdg_code, -muon_pdg_code],
                    nTracks=1,
                    varyNTracks=False,
                    momentumGeneration='inversePt',
                    momentumParams=[0.6, 1.4],
                    thetaGeneration='uniform',
                    thetaParams=[17., 150.])


def add_simple_gun_generator(path):
    """Add ParticleGun firing 10 muons at medium energy"""
    path.add_module("ParticleGun",
                    pdgCodes=[muon_pdg_code, -muon_pdg_code],
                    nTracks=10,
                    varyNTracks=False,
                    momentumGeneration='inversePt',
                    momentumParams=[0.6, 1.4],
                    thetaGeneration='uniform')


def add_low_gun_generator(path):
    """Add ParticleGun firing 10 muons at low energy"""
    path.add_module("ParticleGun",
                    pdgCodes=[muon_pdg_code, -muon_pdg_code],
                    nTracks=10,
                    varyNTracks=False,
                    momentumGeneration='inversePt',
                    momentumParams=[0.4, 0.8],
                    thetaGeneration='uniform')


def add_gun_generator(path):
    """Add ParticleGun firing 10 muons with wide energy range"""
    path.add_module("ParticleGun",
                    pdgCodes=[muon_pdg_code, -muon_pdg_code],
                    nTracks=10,
                    varyNTracks=False,
                    momentumGeneration='inversePt',
                    thetaGeneration='uniform',
                    thetaParams=[17., 150.])


def add_forward_gun_generator(path):
    """Add ParticleGun with one muon in rather forward direction"""
    path.add_module("ParticleGun",
                    pdgCodes=[muon_pdg_code, -muon_pdg_code],
                    nTracks=1,
                    varyNTracks=False,
                    momentumGeneration='inversePt',
                    thetaGeneration='uniform',
                    thetaParams=[30., 31.])


def add_evtgen_generator(path, dec_file_path=None):
    """Add Y4S generator"""
    beamparameters.add_beamparameters(path, "Y4S")
    if dec_file_path:
        path.add_module("EvtGenInput", userDECFile=dec_file_path)
    else:
        path.add_module("EvtGenInput")


def add_cosmics_generator(path):
    """Add simple cosmics generator"""
    path.add_module("Cosmics")


def add_cosmics_tb_generator(path):
    """Add simple cosmics generator resembling the test beam setup"""
    path.add_module("Cosmics",
                    ipRequirement=1,
                    ipdr=5,
                    ipdz=15,
                    )

    # Use point trigger
    tof_mode = 1
    cosmics_selector = path.add_module('CDCCosmicSelector',
                                       xOfCounter=0.0,
                                       yOfCounter=0.0,
                                       zOfCounter=0.0,
                                       TOF=tof_mode,
                                       cryGenerator=False,
                                       )

    cosmics_selector.if_false(basf2.create_path())


def add_sector_tb_generator(path, sector=1):
    phiBounds = (240 + 60.0 * sector % 360.0, 300 + 60.0 * sector % 360.0)
    path.add_module("ParticleGun",
                    pdgCodes=[muon_pdg_code, -muon_pdg_code],
                    nTracks=1,
                    varyNTracks=False,
                    momentumGeneration='uniform',
                    momentumParams=[2.0, 4.0],
                    phiGeneration='uniform',
                    phiParams=phiBounds,
                    thetaGeneration='uniform',
                    thetaParams=[70., 110.])


def add_cry_tb_generator(path):
    """Add cry generator resembling the test beam setup"""
    from ROOT import Belle2
    path.add_module('CRYInput',
                    # cosmic data input
                    CosmicDataDir=Belle2.FileSystem.findFile('data/generators/modules/cryinput/'),
                    SetupFile=Belle2.FileSystem.findFile('data/tracking/muon_cry.setup'),
                    # acceptance half-lengths - at least one particle has to enter that box to use that event
                    acceptLength=0.5,
                    acceptWidth=0.5,
                    acceptHeight=0.5,
                    maxTrials=10000,

                    # keep half-lengths - all particles that do not enter the box are removed (keep box >= accept box)
                    keepLength=0.5,
                    keepWidth=0.5,
                    keepHeight=0.5,

                    # minimal kinetic energy - all particles below that energy are ignored
                    kineticEnergyThreshold=0.01,
                    )

    # Use plane trigger
    tof_mode = 2
    cosmics_selector = path.add_module('CDCCosmicSelector',
                                       lOfCounter=30.,
                                       wOfCounter=10.,
                                       xOfCounter=0.0,
                                       yOfCounter=0.0,
                                       zOfCounter=0.0,
                                       TOF=tof_mode,
                                       cryGenerator=True,
                                       )

    cosmics_selector.if_false(basf2.create_path())


def add_no_generator(path):
    """Add no generator for e.g. background only studies"""
    # Nothing to do here since the background files are included in the add_simulation
    pass

# Generator module names hashed by shorthand menomics. Includes
# None as a special value for background only simulation
generators_by_short_name = {
    'single_gun': add_single_gun_generator,
    'simple_gun': add_simple_gun_generator,
    'low_gun': add_low_gun_generator,
    'forward_gun': add_forward_gun_generator,
    'gun': add_gun_generator,
    'generic': add_evtgen_generator,
    "EvtGenInput": add_evtgen_generator,  # <- requires beam parameters
    'cosmics': add_cosmics_generator,
    'cosmics_tb': add_cosmics_tb_generator,
    'cry_tb': add_cry_tb_generator,
    'sector_tb': add_sector_tb_generator,
    'bkg': add_no_generator,
    'none': add_no_generator,
}

# Names of module names and short names of the generators usable in this script.
valid_generator_short_names = list(generators_by_short_name.keys())


# Memorandum of geometry setups #
# ############################# #
def setup_default_detector(path):
    pass


def setup_tracking_detector(path):
    components = ["BeamPipe", "PXD", "SVD", "CDC", "MagneticField"]
    override = [
        ("/DetectorComponent[@name='MagneticField']//Component[@type='3d'][2]/ExcludeRadiusMax",
         "4.20", "m", )  # Remove the second compontent which is the magnetic field outside the tracking volumn.
    ]

    adjustments.adjust_module(path, "Gearbox", override=override)
    adjustments.adjust_module(path, "Geometry", components=components)
    return components


def setup_tracking_detector_constant_b(path):
    components = ["BeamPipe", "PXD", "SVD", "CDC", "MagneticFieldConstant4LimitedRCDC"]
    adjustments.adjust_module(path, "Geometry", components=components)
    return components


def setup_cdc_cr_test(path):
    components = ["CDC"]
    override = [
        # Reset the top volume to accomodate cosmics that can hit all parts of the detector
        ("/Global/length", "8.", "m"),
        ("/Global/width", "8.", "m"),
        ("/Global/height", "1.5", "m"),

        # Adjustments of the CDC setup
        ("/DetectorComponent[@name='CDC']//t0FileName", "t0.dat", ""),
        ("/DetectorComponent[@name='CDC']//xtFileName", "xt_noB_v1.dat", ""),
        ("/DetectorComponent[@name='CDC']//GlobalPhiRotation", "1.875", "deg"),
        # ("/DetectorComponent[@name='CDC']//bwFileName", "badwire_CDCTOP.dat", ""),
    ]

    adjustments.adjust_module(path, "Gearbox", override=override)
    adjustments.adjust_module(path, "Geometry", components=components)
    return components


def setup_cdc_top_test(path):
    components = ["CDC"]
    override = [
        # Reset the top volume: must be larger than the generated surface and higher than the detector
        # It is the users responsibility to ensure a full angular coverage
        ("/Global/length", "8.", "m"),
        ("/Global/width", "8.", "m"),
        ("/Global/height", "1.5", "m"),

        # Adjustments of the CDC setup
        ("/DetectorComponent[@name='CDC']//t0FileName", "t0.dat", ""),
        ("/DetectorComponent[@name='CDC']//xtFileName", "xt_noB_v1.dat", ""),
        # ("/DetectorComponent[@name='CDC']//bwFileName", "badwire_CDCTOP.dat", ""),
        ("/DetectorComponent[@name='CDC']//GlobalPhiRotation", "1.875", "deg"),
        ("/DetectorComponent[@name='MagneticField']//Component/Z", "0", ""),
    ]

    adjustments.adjust_module(path, "Gearbox",
                              override=override,
                              fileName="geometry/CDCcosmicTests.xml"  # <- does something mysterious to the reconstruction...
                              )

    adjustments.adjust_module(path, "Geometry", components=components)
    return components

detector_setups_by_short_name = {
    "Default": setup_default_detector,
    'TrackingDetector': setup_tracking_detector,
    'TrackingDetectorConstB': setup_tracking_detector_constant_b,
    'CDCCRTest': setup_cdc_cr_test,
    'CDCTOPTest': setup_cdc_top_test,
}


# Heuristic to find background files #
# ################################## #

def is_bkg_file(bkg_file_path):
    """Test if a file path points to a file containing background mixins.

    Returns
    -------
    bool

    Note
    ----
    Simple test only checks if file exists and ends with ".root"
    """
    return os.path.isfile(bkg_file_path) and bkg_file_path.endswith('.root')


def get_bkg_file_paths(bkg_dir_or_file_paths):
    """Unpacks the content of a single or a list of directories and/or files filtering for
    files containing background mixins.

    Parameters
    ----------
    bkg_dir_or_file_paths : string or iterable of strings
        Single file or single directory in which background files are located or
        a list of files and/or directories.

    Returns
    -------
    list(string)
        A list of paths to individual background files.
    """

    if isinstance(bkg_dir_or_file_paths, str):
        bkg_dir_or_file_path = bkg_dir_or_file_paths
        bkg_dir_or_file_paths = [bkg_dir_or_file_path]

    result = []
    for bkg_dir_or_file_path in bkg_dir_or_file_paths:
        if is_bkg_file(bkg_dir_or_file_path):
            bkg_file_path = bkg_dir_or_file_path
            result.append(bkg_file_path)
        elif os.path.isdir(bkg_dir_or_file_path):

            bkg_dir_path = bkg_dir_or_file_path
            bkg_dir_contents = os.listdir(bkg_dir_path)
            for dir_or_file_name in bkg_dir_contents:
                dir_or_file_path = os.path.join(bkg_dir_path, dir_or_file_name)
                if is_bkg_file(dir_or_file_path):
                    bkg_file_path = dir_or_file_path
                    result.append(bkg_file_path)

    return result
