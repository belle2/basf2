from caf.framework import Calibration, CAF, Collection, LocalDatabase, CentralDatabase
from caf import backends
from caf import strategies


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

    m.param(argk)

    return m


def make_collection(name, path, **argk):
    """
    Handy function to make a collection configuration
    to be passed in 'collections' argument of the create(...) function

    Parameters
    ----------
    name : str
      Collection name
    path : basf2.Path
      pre-collector path
    argk : dict
      Dictionary of collector parameters specific for collection

    Returns
    -------
    tuple(str, basf2.Path, dict)

    """
    return (name, path, argk)


def create(name,
           dbobjects,
           collections,
           files,
           tags=None,
           timedep=None,
           commands=None,
           constraints=None,
           fixed=None,
           params=None,
           init_event=None,
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

    collections : list(tuple(str, basf2.Path, dict(...)))
        List of collection definitions.
        - str : collection name has to math entry in 'files' dictionary.
        - basf2.Path : is the reprocessing path
        - dict(...) : additional dictionary of parameters passed to the collector.
          This has to contain the input data sample (tracks / particles / primaryVertices ...) configuration for the collector.
          Optionally additional arguments can be specified for the collector specific for this collection.
          (Default arguments for all collections can be set using the 'params' parameter)
          Use make_collection(str, basf2.Path, **argk) for more convenient creation of custom collections.
          For standard collections to use, see alignment.collections

    files : dict( str -> list(str) )
        Dictionary of lists of input file paths, key is collection name, value is list of input files
    tags : list(str)
        List of input global tags. Can include absolute file paths to local databases (added to the chain)

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
        WARNING: the 'init_event' event parameter is used to initiate the generation of the constraints if 'timedep' is not set.
    fixed : list(int)
        List of fixed parameters (use alignment.parameters to get them)

    params : common parameters to set for collectors of all collections.
    init_event : tuple(int, int, int)
        Initial event (event, run, exp) to intitiate constraints if timedep is not configured.
    min_entries : int
        Minimum entries to require by the algorithm. Returns NotEnoughData if less entries is collected.

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

    for cmd in commands:
        set_command(cmd)

    algo = create_algorithm(dbobjects, min_entries=min_entries)

    print("- Commands:")
    for cmd_name, cmd in _commands.items():
        print("   ", cmd)
        algo.steering().command(cmd)

    print("- Constraints:")
    consts = constraints
    if len(consts):
        algo.steering().command('FortranFiles')
        for const in consts:
            print("   ", const.filename)
            algo.steering().command(const.filename)

    def gen_constraints(algorithm, iteration):
        from alignment.constraints import generate_constraints
        if len(consts):
            generate_constraints(consts, timedep, tags, init_event)

    algo.steering().command('Parameters')

    def fix(labels):
        for label in labels:
            algo.steering().command('{} 0.0 -1.'.format(str(label)))

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

    dbchain = make_database_chain(tags)

    #  Note the default collector and path are ignored
    #  The user is supposed to add at least one own collection
    calibration = Calibration(name,
                              collector=None,
                              algorithms=algo,
                              input_files=None,
                              pre_collector_path=None,
                              database_chain=dbchain if tags is not None else None,
                              output_patterns=None,
                              max_files_per_collector_job=1,
                              backend_args=None
                              )
    print("- Overriden common collector parameters:")
    for parname, parval in params.items():
        print("    ", parname, ":", parval)

    print("- Collections:")
    for colname, path, args in collections:
        filelist = []
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
                                max_files_per_collector_job=1,
                                database_chain=dbchain if tags is not None else None)

        calibration.add_collection(colname, collection)

    calibration.strategies = strategies.SingleIOV
    calibration.max_iterations = 1

    calibration.pre_algorithms = gen_constraints

    print("----------------------------")

    return calibration


def main():
    import os

    import basf2
    from ROOT import Belle2

    import rawdata as raw
    import reconstruction as reco
    import modularAnalysis as ana
    import vertex as vtx

    import alignment.parameters
    import alignment.constraints

    def diMuonCollection(name="diMuons", add_unpackers=True):
        path = basf2.create_path()

        path.add_module('Progress')
        # Remove all non-raw data to run the full reco again
        path.add_module('RootInput')  # , branchNames=input_branches, entrySequences=['0:5000'])
        path.add_module('Gearbox')
        path.add_module('Geometry')

        if add_unpackers:
            raw.add_unpackers(path)

        reco.add_reconstruction(path, pruneTracks=False)

        tmp = basf2.create_path()
        for m in path.modules():
            if m.name() == "PXDPostErrorChecker":
                m.param('CriticalErrorMask', 0)
            if m.name() in ["PXDUnpacker", "CDCHitBasedT0Extraction", "TFCDC_WireHitPreparer"]:
                m.set_log_level(basf2.LogLevel.ERROR)
            if m.name() == "SVDSpacePointCreator":
                m.param("MinClusterTime", -999)
            tmp.add_module(m)
        path = tmp
        path.add_module('DAFRecoFitter')

        ana.fillParticleList('mu+:mu_dimuon', 'abs(formula(z0)) < 0.5 and abs(d0) < 0.5 and nTracks == 2', writeOut=True, path=path)
        ana.reconstructDecay('Z0:mumu -> mu-:mu_dimuon mu+:mu_dimuon', '', writeOut=True, path=path)
        vtx.raveFit('Z0:mumu', 0.001, daughtersUpdate=True, path=path)

        return make_collection(name, path=path, primaryVertices=['Z0:mumu'])

    cal = create(
      name='alignment',
      dbobjects=['VXDAlignment', 'BeamSpot'],
      collections=[
        diMuonCollection(name="dimuon_skim", add_unpackers=False),
        make_collection(name="cosmics", path=basf2.create_path(), tracks=['RecoTracks'], minPValue=0.)
        ],
      constraints=[
        alignment.constraints.VXDHierarchyConstraints(),
        ],
      fixed=alignment.parameters.vxd_sensors() + alignment.parameters.vxd_ladders(),
      commands=[
        'method diagonalization 3 0.1',
        'scaleerrors 1. 1.',
        ('printcounts', None)

        ],

      tags=[tag for tag in basf2.conditions.default_globaltags],

      files={'dimuon_skim': [os.path.abspath(f) for f in Belle2.Environment.Instance().getInputFilesOverride()]},

      timedep=[([], [(0, 0, 0)])],
      params=dict(minPValue=0.001, externalIterations=0))

    cal_fw = CAF()
    cal_fw.add_calibration(cal)
    cal_fw.backend = backends.Local(1)
    cal_fw.run()

if __name__ == '__main__':
    main()
