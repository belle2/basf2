#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os

import basf2
import simulation
import beamparameters

import tracking.utilities as utilities

import logging

from tracking.run.minimal import MinimalRun


def get_logger():
    return logging.getLogger(__name__)


# Default settings and shorthand names for generator with specific settings #
#############################################################################

# PDG code of an electorn
electron_pdg_code = 11

# PDG code of a muon
muon_pdg_code = 13

# PDG code of a tau
tau_pdg_code = 15

# Generator module names hashed by shorthand menomics. Includes
# None as a special value for background only simulation
generator_module_names_by_short_name = {
    'gun': 'ParticleGun',
    'single_gun': 'ParticleGun',
    'simple_gun': 'ParticleGun',
    'forward_gun': 'ParticleGun',
    'generic': 'EvtGenInput',
    'cosmics': 'Cosmics',
    'bkg': None,
}

# Names of module names and short names of the generators usable in this script.
valid_generator_names = list(generator_module_names_by_short_name.keys()) \
    + list(generator_module_names_by_short_name.values())

# Strip of the None value
valid_generator_names.remove(None)

# Default parameters of the generator modules hashed by their respective module name
default_generator_params_by_generator_name = {
    'single_gun': {
        'pdgCodes': [muon_pdg_code, -muon_pdg_code],
        'nTracks': 1,
        'varyNTracks': False,
        'momentumGeneration': 'uniform',
        'momentumParams': [0.6, 1.4],
        'thetaGeneration': 'uniform',
        'thetaParams': [17., 150.],
    },
    'simple_gun': {
        'pdgCodes': [muon_pdg_code, -muon_pdg_code],
        'nTracks': 10,
        'varyNTracks': False,
        'momentumGeneration': 'uniform',
        'momentumParams': [0.6, 1.4],
        'thetaGeneration': 'uniform',
        'thetaParams': [17., 150.],
    },
    'gun': {
        'pdgCodes': [muon_pdg_code, -muon_pdg_code],
        'nTracks': 10,
        'varyNTracks': False,
        'momentumGeneration': 'uniform',
        'thetaGeneration': 'uniform',
        'thetaParams': [17., 150.],
    },
    'forward_gun': {
        'pdgCodes': [muon_pdg_code, -muon_pdg_code],
        'nTracks': 1,
        'varyNTracks': False,
        'momentumGeneration': 'uniform',
        'thetaGeneration': 'uniform',
        'thetaParams': [30., 31.],
    },
    'cosmics': {},
    'ParticleGun': {},
    'EvtGenInput': {},
}


# Standard run for generating or reading event #
################################################

# This also provides a commandline interface to specify some parameters.

class ReadOrGenerateEventsRun(MinimalRun):
    # Declarative section which can be redefined in a subclass
    generator_module = None
    bkg_files = []
    simulate_only = False
    components = ['PXD', 'SVD', 'CDC', 'BeamPipe',
                  'MagneticFieldConstant4LimitedRCDC']

    def create_argument_parser(self, **kwds):
        argument_parser = super(ReadOrGenerateEventsRun, self).create_argument_parser(**kwds)

        argument_parser.add_argument(
            '-g',
            '--generator',
            dest='generator_module',
            default=self.generator_module,
            metavar='GENERATOR_NAME',
            choices=utilities.NonstrictChoices(valid_generator_names),
            help='Name module or short name of the generator to be used.',
        )

        argument_parser.add_argument(
            '-b',
            '--bkg-file',
            dest='bkg_files',
            default=self.bkg_files,
            action='append',
            metavar='BACKGROUND_DIRECTORY',
            help='Path to folder of files or to a file containing the background to be used. ' +
                 'Can be given multiple times.',
        )

        argument_parser.add_argument(
            '-so',
            '--simulate-only',
            action='store_true',
            default=self.simulate_only,
            dest='simulate_only',
            help='Only generate and simulate the events, but do not run any tracking or validation code')

        return argument_parser

    def create_path(self):
        # Compose basf2 module path #
        #############################
        main_path = super(ReadOrGenerateEventsRun, self).create_path()

        # Only generate events if no input file has been provided
        if self.root_input_file is None:
            generator_module = get_generator_module(self.generator_module)

            if generator_module is not None:
                generator_module_name = self.get_basf2_module_name(generator_module)
                if generator_module_name == "EvtGenInput":
                    beamparameters.add_beamparameters(main_path, "Y4S")

                # Allow for Background only execution
                main_path.add_module(generator_module)

            random_barrier_module = basf2.register_module("RandomBarrier")
            main_path.add_module(random_barrier_module)

            bkg_file_paths = get_bkg_file_paths(self.bkg_files)
            components = self.components
            simulation.add_simulation(main_path,
                                      components=components,
                                      bkgfiles=bkg_file_paths)

            # Catch if no generator is added, no background should be simulated and events
            # are not read from a file.
            if not bkg_file_paths and generator_module is None:
                raise RuntimeError('Need at least one of root_input_file,'
                                   ' generator_module or bkg_files specified.')

        return main_path


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


def is_generator_name(generator_name):
    """Test, if the given name correspondes to a valid short or module name of a generator.

    Returns
    -------
    bool
        If the name is a module name or a valid short hand for a module
    """

    if generator_name in list(generator_module_names_by_short_name.values()) \
            or generator_name in list(generator_module_names_by_short_name.keys()):
        return True
    else:
        # Also except any module from the generator package
        # although there are false positives.
        generator_module = basf2.register_module(generator_name)
        return generator_module.package() == "generators"


def get_generator_module_name(generator_name):
    """Translates the proper basf2 module name from a short name.

    Returns
    -------
    The proper module name resolved from the short name.

    Raises
    ------
    ValueError
        If no valid module name can be found.
    """

    if is_generator_name(generator_name):
        return generator_module_names_by_short_name.get(generator_name,
                                                        generator_name)
    else:
        raise ValueError('%s does not refer to a valid module name or short hand'
                         % generator_name)


def update_default_generator_params(generator_name, additional_params):
    """Takes to default parameters of the generator module and
    returns a copy updated with the explicitly given additional parameters.

    Parameters
    ----------
    generator_name : string
        Name or short name of a generator module
    additional_params : dict
        Parameters that shall overwrite the defaults

    Returns
    -------
    dict
        Parameters updated with the additional parameters from the defaults.
        Usable with module.param()
    """

    return params


def get_generator_module(generator_module_or_generator_name):
    if isinstance(generator_module_or_generator_name, str):
        generator_name = generator_module_or_generator_name

        # Translate short hand name to basf module name
        generator_module_name = get_generator_module_name(generator_name)

        # Allow None as special value for background only simulation
        if generator_module_name is not None:
            generator_module = basf2.register_module(generator_module_name)
            generator_params = \
                default_generator_params_by_generator_name.get(generator_name,
                                                               {})
            get_logger().info('Setting up generator with parameters %s',
                              generator_params)
            generator_module.param(generator_params)
        else:

            generator_module = None
    else:

        generator_module = generator_module_or_generator_name

    return generator_module


class StandardEventGenerationRun(ReadOrGenerateEventsRun):
    generator_module = 'EvtGenInput'


def main():
    readOrGenerateEventsRun = StandardEventGenerationRun()

    argument_parser = \
        readOrGenerateEventsRun.create_argument_parser(allow_input=False)

    argument_parser.add_argument('root_output_file',
                                 help='Output file to which the simulated events shall be written.'
                                 )

    arguments = argument_parser.parse_args()

    # Configure the read or event generation from the command line
    readOrGenerateEventsRun.configure(arguments)

    # Add the output module
    root_output_file_path = arguments.root_output_file
    root_output_module = basf2.register_module('RootOutput')
    root_output_params = {'outputFileName': root_output_file_path}
    root_output_module.param(root_output_params)

    readOrGenerateEventsRun.add_module(root_output_module)

    # Execute the run
    readOrGenerateEventsRun.execute()


if __name__ == '__main__':
    logging.basicConfig(level=logging.INFO)
    main()
