#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import basf2

from caf.framework import Calibration, CentralDatabase, Collection, LocalDatabase
from caf import strategies

import os

import alignment.parameters as parameters  # noqa
import alignment.constraints  # noqa

import alignment.collections  # noqa
from alignment.collections import make_collection  # noqa


def limit_file_events(calibration, collection_limits):
    for colname, max_events in collection_limits.items():
        basf2.set_module_parameters(
            calibration.collections[colname].pre_collector_path,
            'RootInput',
            entrySequences=[f'0:{max_events}'])


def collect(calibration, collection, input_files, output_file='CollectorOutput.root', basf2_args=None, bsub=False):
    """
    Standalone collection for calibration (without CAF)
    (experimental)

    Pickles the reprocessing path and collector of given collection and runs it in separate process
    to collect calibration data in output_file from input_files.

    Parameters
    ----------
    calibration : caf.framework.Calibration
      The configured Millepede calibration (see create(...)) to use for collection
    collection : str
      Collection name which should be collected (which re-processing path and collector to use)
    input_files : list(str)
      List of input files for this collection job
    output_file : str
      Name of output collector file with data/histograms (produced by HistoManager)
    basf2_args : dict
      Additional arguments to pass to basf2 job
    """
    if basf2_args is None:
        basf2_args = []

    import pickle
    import subprocess

    main = calibration.collections[collection].pre_collector_path

    tmp = basf2.Path()
    for m in main.modules():
        if m.name() == 'RootInput':
            m.param('inputFileNames', input_files)
            tmp.add_module('HistoManager', histoFileName=output_file)
        tmp.add_module(m)
    main = tmp
    main.add_module(calibration.collections[collection].collector)

    path_file_name = calibration.name + '.' + collection + '.' + output_file + '.path'
    with open(path_file_name, 'bw') as serialized_path_file:
        pickle.dump(basf2.pickle_path.serialize_path(main), serialized_path_file)

    if bsub:
        subprocess.call(["bsub", "-o", output_file + ".txt", "basf2", "--execute-path", path_file_name] + basf2_args)
    else:
        subprocess.call(["basf2", "--execute-path", path_file_name] + basf2_args)

    return os.path.abspath(output_file)


def calibrate(calibration, input_files=None, iteration=0):
    """
    Execute the algorithm from configured Millepede calibration over collected
    files in input_files. The pre_algorithm function is run before the algorithm.

    Parameters
    ----------
    calibration : caf.framework.Calibration
      Configured Millepede calibration (see create(...))
    input_files : list(str)
      List of input collected files
    iteration : int
      Iteration number to pass to pre_algorithm function
    """
    if input_files is None:
        input_files = ['CollectorOutput.root']
    """
    Execute algorithm of the Millepede calibration over
    """
    for algo in calibration.algorithms:
        algo.algorithm.setInputFileNames(input_files)
        algo.pre_algorithm(algo.algorithm, iteration)
        algo.algorithm.execute()
        algo.algorithm.commit()


def create_algorithm(dbobjects, min_entries=10, ignore_undetermined=True):
    """
    Create Belle2.MillepedeAlgorithm

    Parameters
    ----------
    dbobjects : list(str)
      List of DB objects to calibrate - has to match collector settings
    min_entries : int
      Minimum number of collected entries for calibration. Algorithm will return
      NotEnoughData is less entries collected.
    ignore_undetermined : bool
      Whether undetermined parameters should be ignored or the calibration should fail if any
    """
    import ROOT
    from ROOT.Belle2 import MillepedeAlgorithm
    algorithm = MillepedeAlgorithm()

    std_components = ROOT.vector('string')()
    for component in dbobjects:
        std_components.push_back(component)
    algorithm.setComponents(std_components)

    algorithm.ignoreUndeterminedParams(ignore_undetermined)
    algorithm.setMinEntries(min_entries)

    algorithm.invertSign(True)

    return algorithm


def create_commands():
    """
    Create default list of commands for Pede
    """
    cmds = []
    cmds.append('method inversion 3 0.1')
    cmds.append('skipemptycons')

    import multiprocessing
    ncpus = multiprocessing.cpu_count()

    cmds.append(f'threads {ncpus} {ncpus}')
    cmds.append('printcounts 2')
    cmds.append('closeandreopen')

    cmds.append('hugecut 50.')
    cmds.append('chiscut 30. 6.')
    cmds.append('outlierdownweighting 3')
    cmds.append('dwfractioncut 0.1')

    return cmds


def create_collector(dbobjects, **argk):
    """
    Create MillepedeCollector module with default configuration

    Parameters
    ----------
    dbobject : list(str)
      List of database objects to be calibrated (global derivatives of others
      will be disabled) - has to match algorithm settings
    argk : dict
      Dictionary of additional module parameters (can override defaults)

    Returns
    -------
    MillepedeCollectorModule (configured)
    """
    import basf2
    m = basf2.register_module('MillepedeCollector')

    m.param('granularity', 'all')
    # Let's enable this always - will be in effect only if BeamSpot is in dbobjects
    m.param('calibrateVertex', True)
    # Not yet implemeted -> alwas OFF for now
    m.param('calibrateKinematics', False)
    m.param('minUsedCDCHitFraction', 0.8)
    m.param('minPValue', 0.0)
    m.param('externalIterations', 0)
    m.param('tracks', [])
    m.param('fitTrackT0', True)
    m.param('components', dbobjects)
    m.param('useGblTree', False)
    m.param('absFilePaths', True)

    # By default, hierarchy is disabled, because you need some constraints
    # - Adding VXDHierarchyConstraints changes the hierarchy type to the correct one
    m.param('hierarchyType', 0)

    m.param(argk)

    return m


def create(name,
           dbobjects,
           collections,
           files=None,
           tags=None,
           timedep=None,
           commands=None,
           constraints=None,
           fixed=None,
           params=None,
           min_entries=0
           ):
    """
    Create the Millepede Calibration, fully configured in one call


    Parameters
    ----------
    name : str
        Calibration name
    dbobjects : list(str)
        List of database objects to calibrate, e.g. ['BeamSpot', 'VXDAlignment']
        Note that by default all parameters of the db object are free (exceptions
        depend on some constraint and collector configuration) and you might need to fix
        the unwanted ones (typically higher order sensor deformations) using the 'fixed' parameter.

    collections : list(namedtuple('MillepedeCollection', ['name', 'files', 'path', 'params']))
        List of collection definitions.
        - name : str
            Collection name has to math entry in 'files' dictionary.
        - files : list(str) | None
            Optional list of files. Can (should if not set here) be overriden by 'files' parameter
        - path : basf2.Path
            The reprocessing path
        - dict(...) : additional dictionary of parameters passed to the collector.
          This has to contain the input data sample (tracks / particles / primaryVertices ...) configuration for the collector.
          Optionally additional arguments can be specified for the collector specific for this collection.
          (Default arguments for all collections can be set using the 'params' parameter)
          Use make_collection(str, path=basf2.Path, **argk) for more convenient creation of custom collections.
          For standard collections to use, see alignment.collections

    files : dict( str -> list(str) )
        Dictionary of lists of input file paths, key is collection name, value is list of input files.
        NOTE: This overrides possible list of files assigned during creation of collections (if set)
    tags : list(str)
        List of input global tags. Can include absolute file paths to local databases (added to the chain).

    timedep : list(tuple(list(int), list(tuple(int, int, int))))
        Time-depence configuration.
        Each list item is 2-tuple with list of parameter numbers (use alignment.parameters to get them) and
        the (event, run, exp) numbers at which values of these parameters can change.
        Use with caution. Namely the first event of the lowest run in input data has to be included in (some of the)
        event lists.

    commands : list(str | tuple(str, None))
        List of commands for Millepede. Default commands can be overriden be specifing different values for them.
        A command can be erased completely from the default commands if instead a ('command_name', None) is passed.
    constraints : list(alignment.Constraints)
        List of constraints from alignment.constraints to be used.
        Constraints are generated by the pre-algorithm function by CAF.
    fixed : list(int)
        List of fixed parameters (use alignment.parameters to get them)

    params : dict
        Dictionary of common parameters to set for collectors of all collections.
    min_entries : int
        Minimum entries to required by the algorithm. Returns NotEnoughData if less entries is collected.

    Returns
    ----------
    caf.framework.Calibration object, fully configured and ready to run.
    You might want to set/override some options to custom values, like 'max_iterations' etc.
    """

    print("----------------------------")
    print(" Calibration: ", name, "")
    print("----------------------------")

    print("- DB Objects:")
    for objname in dbobjects:
        print("   ", objname)

    cmds = create_commands()

    _commands = dict()

    def set_command(command):
        spec = ''
        if isinstance(command, tuple):
            if not len(command) == 2:
                raise AttributeError("Commands has to be strings or tuple ('command name', None) to remove the command")
            spec = None
            command = command[0]
        words = command.split(" ")
        cmd_name = words[0]

        if spec == '':
            spec = command

        if spec is not None:
            _commands[cmd_name] = spec
        elif cmd_name in _commands:
            del _commands[cmd_name]

    for cmd in cmds:
        set_command(cmd)

    if commands is None:
        commands = []

    for cmd in commands:
        set_command(cmd)

    algo = create_algorithm(dbobjects, min_entries=min_entries)

    print("- Commands:")
    for cmd_name, cmd in _commands.items():
        print("   ", cmd)
        algo.steering().command(cmd)

    if constraints is None:
        constraints = []

    print("- Constraints:")
    consts = constraints
    if len(consts):
        algo.steering().command('FortranFiles')
        for const in consts:
            print("   ", const.filename)
            algo.steering().command(const.filename)

    if timedep is None:
        timedep = []

    def gen_constraints(algorithm, iteration):
        import basf2
        from alignment.constraints import generate_constraints

        data_iov = algorithm.getRunRangeFromAllData().getIntervalOfValidity()
        init_event = (0, data_iov.getRunLow(), data_iov.getExperimentLow())

        # This function runs with DB chain set up
        # TODO: we ignore the local DBs from CAF etc., that is, the constraints
        # are always build only from last valid central GT. As constraints are only
        # linearizations, small alignment changes have numerical neglible effects on
        # constraint coefficients. But we can do even better -> should be not difficult
        # but needs much more testing.
        # NOTE: rversed (highest priority last) order expected here
        constraint_tags = [tag for tag in reversed(basf2.conditions.globaltags)]

        if len(consts):
            generate_constraints(consts, timedep, constraint_tags, init_event)

    algo.steering().command('Parameters')

    def fix(labels):
        for label in labels:
            algo.steering().command('{} 0.0 -1.'.format(str(label)))

    if fixed is None:
        fixed = []

    print("- Fixed parameters:", len(fixed))
    fix(fixed)

    algo.setTimedepConfig(timedep)

    print("- Tags:")

    def make_database_chain(tags):
        import os
        chain = []
        for tag in tags:
            if os.path.exists(tag):
                print("   Local:", os.path.abspath(tag))
                chain.append(LocalDatabase(os.path.abspath(tag)))
            else:
                print("   Global:", tag)
                chain.append(CentralDatabase(tag))
        return chain

    dbchain = make_database_chain(tags) if tags is not None else None

    #  Note the default collector and path are ignored
    #  The user is supposed to add at least one own collection
    calibration = Calibration(name,
                              collector=None,
                              algorithms=algo,
                              input_files=None,
                              pre_collector_path=None,
                              database_chain=dbchain,
                              output_patterns=None,
                              backend_args=None
                              )

    if params is None:
        params = dict()

    print("- Overriden common collector parameters:")
    for parname, parval in params.items():
        print("    ", parname, ":", parval)

    if files is None:
        files = dict()

    print("- Collections:")
    for col in collections:
        colname = col.name
        colfiles = col.files
        path = col.path
        args = col.params

        filelist = colfiles
        if colname in files:
            if colname in files:
                filelist = files[colname]

        print(f"  - {colname} ({len(filelist)} files)")

        collector = create_collector(dbobjects)
        if params is not None:
            collector.param(params)

        for const in consts:
            const.configure_collector(collector)

        collector.param('timedepConfig', timedep)

        for argname, argval in args.items():
            print("    ", argname, " : ", argval)

        collector.param(args)

        collection = Collection(collector=collector,
                                input_files=filelist,
                                pre_collector_path=path,
                                database_chain=dbchain)

        calibration.add_collection(colname, collection)

    calibration.strategies = strategies.SingleIOV
    calibration.max_iterations = 1

    calibration.pre_algorithms = gen_constraints

    print("----------------------------")

    return calibration
