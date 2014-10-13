#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
import sys

import argparse

import basf2
import simulation
import cdclocaltracking.tools as tools
from cdclocaltracking.tools import is_iterable_collection


def main():
    """Function triggered when executing the Python module.
    Generates and simulates events and writes them to file output using arguments from the command line.
    
    Example
    -------
    Execute 

    >>> python -m cdclocaltracking.event_generation mc.root 
    
    to generate 100 events with 10 midrange momentum muons per event (default parameters).

    >>> python -m cdclocaltracking.event_generation -n 1000 -g generic mc.root 

    To find out the command line arguments execute

    >>> python -m cdclocaltracking.event_generation 
    """

    argument_parser = create_argument_parser(allow_file_input=False)
    argument_parser.add_argument('root_output_file',
                                 help='Output file to which the simulated events shall be written.'
                                 )

    arguments = argument_parser.parse_args()

    root_output_file_path = arguments.root_output_file

    main_path = create_path_from_parsed_arguments(arguments)

    root_output_module = basf2.register_module('RootOutput')
    root_output_params = {'outputFileName': root_output_file_path}
    root_output_module.param(root_output_params)
    main_path.add_module(root_output_module)

    basf2.process(main_path)
    print basf2.statistics


#### Helper functions to setup a command line tool executing basf2 that includes simulation of events or loading of presimulated events.

def create_argument_parser(allow_file_input=True, **kwds):
    """Creates an argparse.ArgumentParser prepopulated with arguments necessary to run event generation and simulation or to load events from a file.

    Parameters
    ----------
    allow_file_input : bool
        Indicates if an option for a root input file to be used instead of a generation and simulation shall be added to the argparse.ArgumentParser
    kwds : keywords
        Keywords are forwarded to the creation of an argparse.ArgumentParser object. 
        Most notable keywords are
            description - the detailed description of the purpose of the main programm.
            epilog - text displayed after the detailed argument summary.

    Returns
    -------
    argparse.ArgumentParser
        populated with commandline options to steer event generation and simulation or loading for file.

    See also
    --------
    argparse.ArgumentParser() : https://argparse.googlecode.com/svn/trunk/doc/ArgumentParser.html
    main() : this python module can be executed to save simulated events to a file.


    Examples
    --------
    >>> argument_parser = event_generation.create_argument_parser(description="My analysis script, that runs on various kinds of event inputs.")
    >>> main_path = event_generation.create_path_from_argument_parser(argument_parser)
    >>> # additional modules
    >>> basf2.process(main_path)
    
    or 
    
    >>> argument_parser = event_generation.create_argument_parser(description="My analysis script, that runs on various kinds of event inputs.")
    >>> # add additonal arguments
    >>> arguments = argument_parser.parse_args()
    >>> main_path = event_generation.create_path_from_parsed_arguments(arguments)
    >>> # additional modules
    >>> basf2.process(main_path)
    """

    argument_parser = tools.DefaultHelpArgumentParser(**kwds)
    add_arguments(argument_parser, allow_file_input=allow_file_input)
    return argument_parser


def add_arguments(argument_parser, allow_file_input=True):
    """Addes arguments relevant for event generation and simulation or for loading events from file to an argument parser."""

    if allow_file_input:
        argument_parser.add_argument('-i', '--input', default=None,
                                     dest='root_input_file',
                                     help='File path to the ROOT file from which the simulated events shall be loaded.'
                                     )

    argument_parser.add_argument(
        '-g',
        '--generator',
        default='gun',
        dest='generator',
        metavar='GENERATOR_NAME',
        choices=valid_generator_names,
        help='Name module or short name of the generator to be used.',
        )

    argument_parser.add_argument(
        '-n',
        '--n-events',
        default=100,
        type=int,
        dest='n_events',
        help='Number of events to be generated',
        )

    argument_parser.add_argument('-b', '--bkg-dir', default=[],
                                 metavar='BACKGROUND_DIRECTORY',
                                 help='Path to folder containing the background files to be used'
                                 )


def create_path_from_command_line(**kwds):
    """Creates an BASF2 execution path with information gathered from command line arguments."""

    argument_parser = event_generation.create_argument_parser(**kwds)
    path = event_generation.create_path_from_argument_parser(argument_parser)
    return path


def create_path_from_argument_parser(argument_parser):
    """Creates an BASF2 execution path with information gathered from an argparse.ArgumentParser object that has the appropriate arguments specified.
    
    See also
    -------
    create_argument_parser : for source of the appropriate argument parser object and usage example.
    """

    arguments = argument_parser.parse_args()
    return create_event_generation_path_from_parsed_arguments(arguments)


def create_path_from_parsed_arguments(arguments):
    """Creates an BASF2 execution path with information gathered from, a Namespace object as returned from argparse.ArgumentParser.parse_args().
    The argparse.ArgumentParser object should have been created by a call to create_argument_parser() or otherwise populated with arguments by add_arguments()

    Parameters
    ----------
    arguments : argparse.Namespace
        Arguments parsed from the command line by a call to argparse.ArgumentParser.parse_args()
        The argparse.ArgumentParser object should have been created by a call to create_argument_parser() or otherwise populated with arguments by add_arguments()

        

    See also
    --------
    create_argument_parser() : to create an ArgumentParser object prepopulated with some standard arguments.
    """

    root_input_file_path = getattr(arguments, 'root_input_file', None)
    if root_input_file_path:
        return create_load_events_path(root_input_file_path)
    else:
        generator_name = arguments.generator
        n_events = arguments.n_events
        bkg_dir_path = arguments.bkg_dir
        return create_simulate_events_path(generator_name, n_events,
                bkg_dir_path)


def create_load_events_path(root_input_file_path):
    """Creates an BASF2 execution path to load events from a file.

    Parameters
    ----------
    root_input_file_path : string
        Name of the ROOT input file that contains simulated events to be loaded and processed in the path

    Returns
    -------
    basf2 module path
        BASF2 module execution path populated with modules nessecary for loading events for further processing
    """

    path = basf2.create_path()
    add_load_events(path, root_input_file_path)
    progress_module = basf2.register_module('Progress')
    return path


def create_simulate_events_path(
    generator_name='gun',
    n_events=100,
    bkg_dir_or_file_paths=[],
    generator_params={},
    ):
    """Creates an BASF2 execution path to generate and simulate events.

    Returns
    -------
    basf2 module path
        BASF2 module execution path populated with modules nessecary for generating and simulating events

    See also
    --------
    add_simulate_events : for parameter description
    """

    path = basf2.create_path()
    add_simulate_events(path, generator_name, n_events, bkg_dir_or_file_paths,
                        generator_params)
    progress_module = basf2.register_module('Progress')
    path.add_module(progress_module)
    return path


#### Methodes and logic to compose a BASF2 module execution path to generate and simulate events or load them from an input file.

## PDG code of an electorn
electron_pdg_code = 11

## PDG code of a muon
muon_pdg_code = 13

## PDG code of a tau
tau_pdg_code = 15

## Default parameters of the generator modules hashed by their respective module name
default_generator_params_by_generator_module_name = {'ParticleGun': {
    'pdgCodes': [muon_pdg_code, -muon_pdg_code],
    'nTracks': 10,
    'varyNTracks': False,
    'momentumGeneration': 'uniform',
    'momentumParams': [0.6, 1.4],
    'thetaGeneration': 'uniform',
    'thetaParams': [17., 150.],
    }, 'EvtGenInput': {}}

## Generator module names hashed by shorthand menomics. Includes None as a special value for background only simulation
generator_module_names_by_short_name = {'gun': 'ParticleGun',
        'generic': 'EvtGenInput', 'bkg': None}  # Background only generator

## Names of module names and short names of the generators usable in this script.
valid_generator_names = list(generator_module_names_by_short_name.keys()) \
    + list(generator_module_names_by_short_name.values())
valid_generator_names = [name for name in valid_generator_names if name]


def get_tracking_components():
    """Getter the lsit for the geometry components relevant for tracking"""

    return ['MagneticFieldConstant4LimitedRCDC', 'BeamPipe', 'PXD', 'SVD',
            'CDC']


def is_root_input_file(file_path):
    """Test if a file path points to a file containing presimulated events. 

    Returns
    -------
    bool


    Note
    ----
    Simple test only checks if file exists and ends with ".root"
    """

    return os.path.isfile(file_path) and file_path.endswith('.root')


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

    if not is_iterable_collection(bkg_dir_or_file_paths):
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
                dir_or_file_path = os.join(bkg_dir_path, dir_or_file_name)
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
        raise ValueError('%s does not refere to a valid module name or short hand'
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

    generator_module_name = get_generator_module_name(generator_name)
    default_params = \
        default_generator_params_by_generator_module_name.get(generator_module_name,
            {})
    params = dict(default_params)
    params.update(additional_params)
    return params


def add_simulate_events(
    path,
    generator_name='gun',
    n_events=100,
    bkg_dir_or_file_paths='',
    generator_params={},
    ):
    """Addes modules generation and simulation of events to the basf2 module path.

    Parameters
    ----------
    path : basf2 module path
        BASF2 module execution path the be populated with modules nessecary for event generation and simulation.
    generator_name : string, optional
        Module name or short name of a generator module or the file name of the root input to be loaded.
        Defaults to "gun" which means particle gun is used to simulate.
    n_events : int, optional
        Number of events to be simulated.
        Only used if events are not loaded .
        Defaults to 100.
    bkg_dir_or_file_paths : string or iterable of strings, optional
        Single file or single directory in which background files are located or a list of files and/or directories. 
        Only used if events are not loaded.
        Defaults to [] which means on background is mixied in.
   generator_params : dict, optional
        Additional parameters to the generator module to overwrite the defaults.
        Defaults to {} meaning only the default parameters are used.
    
    Raises
    ------
    ValueError
        If generator_name does not refer to a valid module name or short name for a generator

    """

    event_info_setter_module = basf2.register_module('EventInfoSetter')
    event_info_setter_params = {'evtNumList': [n_events], 'runList': [1],
                                'expList': [1]}
    event_info_setter_module.param(event_info_setter_params)
    path.add_module(event_info_setter_module)

    add_generator(path, generator_name, generator_params)

    components = get_tracking_components()
    bkg_file_paths = get_bkg_file_paths(bkg_dir_or_file_paths)
    simulation.add_simulation(path, components=components,
                              bkgfiles=bkg_file_paths)


def add_generator(path, generator_name='gun', generator_params={}):
    """Addes generator module setup with the given additional parameters

    See also
    --------
    add_simulate_events : for parameter description    

    Raises
    ------
    ValueError
        If generator_name does not refer to a valid module name or short name for a generator.
    """

    generator_module_name = get_generator_module_name(generator_name)
    # Allow None as special value for background only simulation
    if generator_module_name is not None:
        generator_module = basf2.register_module(generator_module_name)

        generator_params = \
            update_default_generator_params(generator_module_name,
                generator_params)
        generator_module.param(generator_params)

        path.add_module(generator_module)


def add_load_events(path, root_input_file_path):
    """Addes modules to load events from the given file to the module path. 
    Also sets up the Gearbox and the Geometry limited to the tracking detectors.

    Parameters
    ----------
    path : basf2 module path
        BASF2 module executation path to be populated with modules
    root_input_file_path : string
        Path to file from which the presimulated events shall be loaded.   

    Raises
    ------
    ValueError
        If root_input_file_path does not point to a valid ROOT input file.
    
    See also
    --------
    add_simulate_or_load_events : for parameter description
    """

    if not is_root_input_file(root_input_file_path):
        raise ValueError('%s is not a path to a valid ROOT input file'
                         % root_input_file_path)

    # Load file
    root_input_module = basf2.register_module('RootInput')
    root_input_params = {'inputFileName': root_input_file_path}
    root_input_module.param(root_input_params)
    path.add_module(root_input_module)

    # Load gearbox
    gearbox_module = basf2.register_module('Gearbox')
    path.add_module(gearbox_module)

    # Load geometry
    geometry_module = basf2.register_module('Geometry')
    components = get_tracking_components()
    geometry_params = {'components': components}
    geometry_module.param(geometry_params)
    path.add_module(geometry_module)


if __name__ == '__main__':
    main()
