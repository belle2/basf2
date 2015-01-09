#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os

import basf2
import simulation

import tracking.utilities

import logging


def get_logger():
    return logging.getLogger(__name__)


# Default settings and shorthand names for generator with specific settings #
#############################################################################

## PDG code of an electorn
electron_pdg_code = 11

## PDG code of a muon
muon_pdg_code = 13

## PDG code of a tau
tau_pdg_code = 15

## Generator module names hashed by shorthand menomics. Includes None as a special value for background only simulation
generator_module_names_by_short_name = {
    'gun': 'ParticleGun',
    'simple_gun': 'ParticleGun',
    'generic': 'EvtGenInput',
    'cosmics': 'Cosmics',
    'bkg': None,
    }

## Names of module names and short names of the generators usable in this script.
valid_generator_names = list(generator_module_names_by_short_name.keys()) \
    + list(generator_module_names_by_short_name.values())

# Strip of the None value
valid_generator_names.remove(None)

## Default parameters of the generator modules hashed by their respective module name
default_generator_params_by_generator_name = {
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
    'cosmics': {},
    'ParticleGun': {},
    'EvtGenInput': {},
    }


# Standard run for generating or reading event #
################################################

# This also provides a commandline interface to specify some parameters.

class ReadOrGenerateEventsRun(object):

    # Declarative section which can be redefined in a subclass
    n_events = 10000
    generator_module = 'EvtGenInput'
    bkg_files = []
    root_input_file = None
    components = ['PXD', 'SVD', 'CDC', 'BeamPipe',
                  'MagneticFieldConstant4LimitedRCDC']

    # Allow override from instances only in these field names to prevent some spelling errors
    # __slots__ = [
    #     '_path',
    #     'n_events',
    #     'generator_module',
    #     'bkg_files',
    #     'root_input_file',
    #     'components',
    #     ]

    def __init__(self):
        super(ReadOrGenerateEventsRun, self).__init__()
        self._path = None

    @property
    def name(self):
        return self.__class__.__name__

    def create_argument_parser(self, **kwds):
        # Argument parser that gives a help full message on error,
        # which includes the options that are valid.
        argument_parser = tracking.utilities.DefaultHelpArgumentParser(**kwds)

        argument_parser.add_argument('-i', '--input',
                                     default=self.root_input_file,
                                     dest='root_input_file',
                                     help='File path to the ROOT file from which the simulated events shall be loaded.'
                                     )

        argument_parser.add_argument(
            '-g',
            '--generator',
            dest='generator_module',
            default=self.generator_module,
            metavar='GENERATOR_NAME',
            choices=valid_generator_names,
            help='Name module or short name of the generator to be used.',
            )

        argument_parser.add_argument(
            '-n',
            '--n-events',
            dest='n_events',
            default=self.n_events,
            type=int,
            help='Number of events to be generated',
            )

        argument_parser.add_argument(
            '-b',
            '--bkg-file',
            dest='bkg_files',
            default=self.bkg_files,
            action='append',
            metavar='BACKGROUND_DIRECTORY',
            help='Path to folder of files or to a file containing the background to be used. Can be given multiple times.'
                ,
            )

        return argument_parser

    def configure(self, arguments):
        # Simply translate the arguments that have
        # the same name as valid instance arguments
        for (key, value) in vars(arguments).items():
            if hasattr(self, key):
                setattr(self, key, value)

    def configure_from_commandline(self):
        argument_parser = self.create_argument_parser()
        arguments = argument_parser.parse_args()
        self.configure(arguments)

    def create_path(self):
        # Compose basf2 module path #
        #############################
        main_path = basf2.create_path()

        # Master module
        eventInfoSetterModule = basf2.register_module('EventInfoSetter')
        eventInfoSetterModule.param({'evtNumList': [self.n_events],
                                    'runList': [1], 'expList': [1]})
        main_path.add_module(eventInfoSetterModule)

        # Progress module
        progressModule = basf2.register_module('Progress')
        main_path.add_module(progressModule)

        # use Generator if no root input file is specified
        components = self.components
        if self.root_input_file is None:
            generatorModule = get_generator_module(self.generator_module)
            if generatorModule is not None:
                # Allow for Background only execution
                main_path.add_module(generatorModule)

            bkg_file_paths = get_bkg_file_paths(self.bkg_files)
            simulation.add_simulation(main_path, components=components,
                                      bkgfiles=bkg_file_paths)

            # Catch if no generator is added, no background should be simulated and events are not read from a file.
            if not bkg_file_paths and generatorModule is None:
                raise RuntimeError('Need at least one of root_input_file, generator_module or bkg_files specified.'
                                   )
        else:

            rootInputModule = basf2.register_module('RootInput')
            rootInputModule.param({'inputFileName': self.root_input_file})
            main_path.add_module(rootInputModule)

            # gearbox & geometry needs to be registered any way
            gearbox = basf2.register_module('Gearbox')
            main_path.add_module(gearbox)
            geometry = basf2.register_module('Geometry')
            geometry.param('components', components)
            main_path.add_module(geometry)

        return main_path

    @property
    def path(self):
        if self._path is None:
            self._path = self.create_path()
        return self._path

    def add_module(self, module=None):
        path = self.path
        module = self.get_basf2_module(module)
        path.add_module(module)

    def execute(self):
        # Create path and run #
        #######################
        main_path = self.path

        # Run basf2 module path #
        #########################
        print 'Start processing'
        basf2.process(main_path)
        print basf2.statistics

    def configure_and_execute_from_commandline(self):
        self.configure_from_commandline()
        self.execute()

    @staticmethod
    def get_basf2_module(module_or_module_name):
        if isinstance(module_or_module_name, str):
            module_name = module_or_module_name
            module = basf2.register_module(module_name)
            return module
        elif isinstance(module_or_module_name, basf2.Module):
            module = module_or_module_name
            return module
        else:
            message_template = \
                '%s of type %s is neither a module nor the name of module. Expected str or basf2.Module instance.'
            raise ValueError(message_template % (module_or_module_name,
                             type(module_or_module_name)))


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
    """Unpacks the content of a single or a list of directories and/or files filtering for files containing background mixins.
    
    Parameters
    ----------
    bkg_dir_or_file_paths : string or iterable of strings
        Single file or single directory in which background files are located or a list of files and/or directories. 

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

    return generator_name in generator_module_names_by_short_name.values() \
        or generator_name in generator_module_names_by_short_name.keys()


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
    """Takes to default parameters of the generator module and returns a copy updated with the explicitly given additional parameters.
    
    Parameters
    ----------
    generator_name : string 
        Name or short name of a generator module
    additional_params : dict
        Parameters that shall overwrite the defaults

    Returns
    -------
    dict
        Parameters updated with the additional parameters from the defaults. Usable with module.param()
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


def main():
    readOrGenerateEventsRun = ReadOrGenerateEventsRun()

    argument_parser = readOrGenerateEventsRun.create_argument_parser()

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
    logging.basicConfig()
    main()
