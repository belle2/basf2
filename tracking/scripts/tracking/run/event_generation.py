#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os

import basf2
import simulation
import generators
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
    simulation_output = None

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

        simulation_argument_group = argument_parser.add_argument_group("Simulation arguments")
        simulation_argument_group.add_argument(
            '-b',
            '--bkg-file',
            dest='bkg_files',
            nargs='+',
            default=self.bkg_files,
            metavar='BACKGROUND_DIRECTORY',
            help='Path to folder of files or to a file containing the background to be used. ' +
                 'Can be given multiple times.',
        )

        simulation_argument_group.add_argument(
            '--disable-deltas',
            action='store_true',
            help='Disable the generation of delta rays in the simulation'
        )

        simulation_argument_group.add_argument(
            '-so',
            '--simulation-output',
            nargs='?',
            default=self.simulation_output,
            const=self.root_input_file,
            dest='simulation_output',
            help='Only generate and simulate the events and write them to the given output file. Skip rest of the path.'
        )

        return argument_parser

    def configure(self, arguments):
        super().configure(arguments)
        if self.simulation_output:
            get_logger().info("Requested to simulation run. Deactivate input file")
            self.root_input_file = None

    def execute(self):
        if not self.simulation_output:
            super().execute()
            return

        # Run only simulation
        path = ReadOrGenerateEventsRun.create_path(self)
        self.run(path)

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
        else:
            if not os.path.exists(self.root_input_file):
                raise RuntimeError("Could not find file " + str(self.root_input_file) + ". Generate it with -- -so?")

        # early write out if simulation output was requested
        if self.simulation_output:
            root_output_module = path.add_module('RootOutput',
                                                 outputFileName=self.simulation_output)

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

# PDG code of a pion
pion_pdg_code = 211

# PDG code of a kaon
kaon_pdg_code = 321

# PDG code of a protons
proton_pdg_code = 2212


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


def add_transverse_gun_generator(path):
    """Add ParticleGun to illuminate a region of the phase space with low efficiency"""
    path.add_module("ParticleGun",
                    pdgCodes=[muon_pdg_code, -muon_pdg_code],
                    nTracks=1,
                    varyNTracks=False,
                    momentumGeneration='inversePt',
                    momentumParams=[0.275, 0.276],
                    thetaGeneration='uniform',
                    thetaParams=[89., 91.])


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


def add_eloss_gun_generator(path):
    """Add ParticleGun particle gun for energy loss estimations"""
    path.add_module("ParticleGun",
                    pdgCodes=[
                        # muon_pdg_code,
                        # -muon_pdg_code,
                        # electron_pdg_code,
                        # -electron_pdg_code,
                        pion_pdg_code,
                        -pion_pdg_code,
                        # kaon_pdg_code,
                        # -kaon_pdg_code,
                        # proton_pdg_code,
                        # -proton_pdg_code,
                    ],
                    momentumParams=[0.3, 2],
                    nTracks=10,
                    varyNTracks=False,
                    thetaGeneration='uniform',
                    # thetaParams=[17., 150.],
                    thetaParams=[89., 91],
                    )


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
    generators.add_cosmics_generator(path)


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


def add_cosmics_tb_generator(path):
    """Add simple cosmics generator resembling the test beam setup"""
    path.add_module("Cosmics",
                    ipRequirement=1,
                    ipdr=5,
                    ipdz=15,
                    )

    # Use point trigger
    tof_mode = 1
    # Do not add time of propagation in the scintilator
    top_mode = False

    cosmics_selector = path.add_module('CDCCosmicSelector',
                                       xOfCounter=0.0,
                                       yOfCounter=0.0,
                                       zOfCounter=0.0,
                                       TOF=tof_mode,
                                       TOP=top_mode,
                                       cryGenerator=False,
                                       )

    cosmics_selector.if_false(basf2.create_path())


def add_cry_tb_generator(path):
    """Add cry generator resembling the test beam setup"""
    generators.add_cosmics_generator(path, accept_box=[0.7, 0.3, 0.3],
                                     keep_box=[0.7, 0.3, 0.3],
                                     pre_general_run_setup="normal")


def add_no_generator(path):
    """Add no generator for e.g. background only studies"""
    # Nothing to do here since the background files are included in the add_simulation
    pass

# Generator module names hashed by shorthand menomics. Includes
# None as a special value for background only simulation
generators_by_short_name = {
    'single_gun': add_single_gun_generator,
    'transverse_gun': add_transverse_gun_generator,
    'simple_gun': add_simple_gun_generator,
    'low_gun': add_low_gun_generator,
    'forward_gun': add_forward_gun_generator,
    'gun': add_gun_generator,
    'eloss_gun': add_eloss_gun_generator,
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
