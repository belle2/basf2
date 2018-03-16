#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *

# The backend provides the input data files to the job and you can get the list of
# files from this function
from caf.backends import get_input_data


def run_collectors():
    """
    Runs the CAF collector in a general way by passing serialised basf2 paths, input file
    locations, and local databases from previous calibrations.
    """
    import glob
    import os
    import sys
    import pickle
    import json

    # We pass in basf2 and CAF options via the config.json file
    with open('collector_config.json', 'r') as config_file:
        config = json.load(config_file)

    # Create the database chain to use the necessary central DB global tag and local DBs if they are requested
    # We don't check for existence of keys in the dictionary as we want it to fail if they don't exist.
    reset_database()
    use_database_chain(True)
    if config['global_tag']:
        B2INFO("Using Global Tag {}".format(config['global_tag']))
        use_central_database(config['global_tag'])
    for filename, directory in config['local_database_chain']:
        B2INFO("Adding Local Database {} to chain".format(filename))
        use_local_database(filename, directory, True, LogLevel.INFO)

    # create a path with all modules needed before calibration path is run.
    collector_path = create_path()

    # Grab all the pickled path files. Should at least be one for the collector. And optionally
    # one for the modules to run before the collector.
    pre_collector_path_name = 'pre_collector.path'
    pickle_paths = glob.glob('./*.path')

    # Remove the pre-collector path from the overall list
    pickle_paths = filter(lambda x: pre_collector_path_name not in x, pickle_paths)

    # Check for and add a path to run before the collector
    if os.path.exists(pre_collector_path_name):
        collector_path.add_path(get_path_from_file(pre_collector_path_name))
    else:
        B2INFO("Couldn't find any pickle files for pre-collector setup")

    # Check for any other paths to run (should only be collectors) and append them
    if pickle_paths:
        for pickle_path in pickle_paths:
            collector_path.add_path(get_path_from_file(pickle_path))
    else:
        B2FATAL("Couldn't find any pickle files for collector path!")

    # Grab the input data. Can't continue without some
    input_data = get_input_data()

    # Now we need to create a path that definitely has RootInput as a module.
    main = create_path()
    # Use this utility wrapper to check for RootInput and change params if necessary
    from caf.utils import PathExtras
    pe = PathExtras(collector_path)
    if 'RootInput' in pe:
        root_input_mod = collector_path.modules()[pe.index('RootInput')]
        root_input_mod.param('inputFileNames', input_data)
    else:
        main.add_module('RootInput', inputFileNames=input_data)
    if 'HistoManager' not in pe:
        main.add_module('HistoManager', histoFileName='CollectorOutput.root')

    main.add_path(collector_path)
    process(main)


if __name__ == '__main__':
    run_collectors()
