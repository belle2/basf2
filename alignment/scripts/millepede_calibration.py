from basf2 import *

set_log_level(LogLevel.INFO)

import os
import sys
import multiprocessing

import ROOT
from ROOT import Belle2
from ROOT.Belle2 import MillepedeAlgorithm

from caf.framework import Calibration, CAF, Collection, LocalDatabase, CentralDatabase
from caf import backends
from caf import strategies

import rawdata as raw
import reconstruction as reco
import modularAnalysis as ana

import copy

import alignment.parameters
import alignment.constraints
import alignment.defaults

from contextlib import contextmanager

input_branches = [
    'EventMetaData',
    'RawTRGs',
    'RawFTSWs',
    'RawPXDs',
    'RawSVDs',
    'RawCDCs',
    'RawTOPs',
    'RawARICHs',
    'RawECLs',
    'RawKLMs']

#  Default "creators" for algo, collector and main paths.
#  Override using e.g. (see alignment/defaults.py):
#
#  >>> def my_path(**argk):
#  >>>   path = create_path()
#  >>>   ...
#  >>>   return path
#  >>>
#  >>> import millepede_calibration as mpc
#  >>> mpc.create_stdreco_path = my_path
#  >>>
create_algorithm = alignment.defaults.create_algorithm
create_collector = alignment.defaults.create_collector
create_stdreco_path = alignment.defaults.create_stdreco_path
create_cosmics_path = alignment.defaults.create_cosmics_path


def copy_module(module):
    m = register_module(module.type())
    for par in module.available_params():
        m.param(par.name, copy.deepcopy(par.values))
    return m


def mkdict(**argk):
    """
    Simple helper to make dictionary faster without quoting strings:
    {'key1':val1, 'key2':val2} == mkdict(key1=val1, key2=val2)
    """
    return argk


class CollectionConfig():
    """
    Simple class holding configuration for a collection
    (pre-collector path + collector module)
    """

    def __init__(self, path, collector):
        self.path = path
        self.collector = collector


class Config():
    """
    The main configuration object for Millepede calibration

    The user should construct this object using

    >>> config = create_configuration(...)

    and finish the configuration of collections afterwards using class
    methods of the Config object

    Create new (physics) collection and add some data samples

    with config.reprocess_physics():
      config.collect_particles('mu+:good')
      config.collect_particles('pi+:good')

    """

    def __init__(self):
        # : List of names of DB objects to calibrate (BeamSpot, VXDAlignment, ...)
        self.db_components = []
        # : List of fixed parameter labels (time indep.)
        self.fixed = []
        # : List of alignment.constraints.Constraints objects to generate constraint files
        self.constraints = []
        # : List of (additional) commands to pass to Pede
        self.commands = dict()
        # : The pre-configured algorithm
        self.algo = Belle2.MillepedeAlgorithm()
        # : The pre-configured defualt path
        self.path = Path()
        # : Reconstruction components (PXD, SVD, ...) to be passed to automatically generated paths
        self.reco_components = None
        # : The pre-configured default collector
        self.collector = register_module('MillepedeCollector')
        # : Additional default parameters to pass to the collector
        self.params = dict()
        # : The list of created data collections
        self.collections = dict()

        self._collections_context = list()

    def set_command(self, command, spec=''):
        words = command.split(" ")
        cmd_name = words[0]

        if spec == '':
            spec = command

        if spec is not None:
            self.commands[cmd_name] = spec
        elif cmd_name in self.commands:
            del self.commands[cmd_name]

    def reprocess_collection(self, name, path=None, collector=None, **argk):
        return self._ctrlctx(self._reprocess_collection(name, path=path, collector=collector, **argk))

    def reprocess_physics(self, path=None, collector=None, collection_name='physics', **argk):
        if path is None:
            path = create_stdreco_path(components=self.reco_components)

        return self._ctrlctx(self._reprocess_collection(collection_name, path=path, collector=collector, **argk))

    def reprocess_cosmics(self, path=None, collector=None, collection_name='cosmics', **argk):
        if path is None:
            path = create_cosmics_path(components=self.reco_components)

        return self._ctrlctx(self._reprocess_collection(collection_name, path=path, collector=collector, **argk))

    def reprocess_B0cosmics(self, path=None, collector=None, collection_name='B0cosmics', **argk):
        if path is None:
            path = create_cosmics_path(components=self.reco_components)

        return self._ctrlctx(self._reprocess_collection(collection_name, path=path, collector=collector, **argk))

    def add_path(self, path, collection=None):
        if path is not None:
            self._collection_from_context(collection).path.add_path(path)

    def collect(self, sample_type, sample_name, path=None, collection=None):
        self._append_param_list(sample_type, sample_name, collection)
        self.add_path(path)

    def collect_tracks(self, arrayname='RecoTracks', path=None, collection=None):
        self.collect('tracks', arrayname, path, collection)

    def collect_particles(self, listname, path=None, collection=None):
        self.collect('particles', listname, path, collection)

    def collect_vertex_decays(self, listname, path=None, collection=None, ip_constraint=False):
        if ip_constraint:
            self.collect('primaryVertices', listname, path, collection)
        else:
            self.collect('vertices', listname, path, collection)

    def collect_2body_decays(
            self,
            listname,
            path=None,
            collection=None,
            ipvertex_constraint=False,
            ipmass_constraint=False,
            ipboost_constraint=False,
            mass=None,
            width=None):
        if ipboost_constraint and not (ipvertex_constraint and ipmass_constraint):
            B2WARNING(
                "When boost_constraint=True, also ip and mass constraints are set True as a full kinematic constraint"
                "based B-Factory kinematics will be used (you can set custom inv. mass and width to be used instead of those"
                "from BeamParameters).")
            ipvertex_constraint = ipmass_constraint = True

        if width is not None and mass is None:
            raise AttributeError("When custom width is set, you have to provide custom mass for the mother particle, too.")

        if mass is not None:
            if width is None:
                raise AttributeError("When custom mass is set, you have to provide custom width for the mother particle, too.")
            else:
                self._set_custom_masswidth(listname, mass, width, collection)

        if ipboost_constraint and ipvertex_constraint and ipmass_constraint:
            #  = all constraints ON -> full kinematic constraint
            self.collect('primaryTwoBodyDecays', listname, path, collection)
        elif ipmass_constraint and ipvertex_constraint:
            self.collect('primaryMassVertexTwoBodyDecays', listname, path, collection)
        elif ipmass_constraint:
            self.collect('primaryMassTwoBodyDecays', listname, path, collection)
        elif ipvertex_constraint:
            raise AttributeError("Cannot enable only IP vertex constraint on two-body-decays. Use vertex decays with ip_constraint")
        else:
            #  Only invariant mass constraint derived from mother particle or set to custom value
            self.collect('twoBodyDecays', listname, path, collection)

    #  "Private" methods: ----------------------------------------------------------------------------------

    def _reprocess_collection(self, name, path=None, collector=None, **argk):
        """
        Create a new (or replace) collection configuration.
        @param path Path with pre-collector path
        @param collector MillepedeCollector module to override all defaults
        @param argk additional parameters to be passed to collector
        """
        #  Copy our default path if none provided, needs register_module to make
        #  actuall deep copy
        if path is None:
            path = create_path()
            for m in self.path.modules():
                path.add_module(copy_module(m))

        #  Copy the default collector with params from create_configuration(params=..., collector=...)
        if collector is None:
            collector = copy_module(self.collector)
        #  Override with ad-hoc parameters for this collection
        collector.param(argk)

        self.collections[name] = CollectionConfig(path, collector)

        #  Entering context... exit in _ctrlctx or never
        #  (but may be preceded by new context) if not used in 'with' construct
        self._collections_context.append(name)

        return name

    @contextmanager
    def _ctrlctx(self, name):
        #  Execute 'with' block with collection=collections[name] context.
        #  Does not yield anything. Accessing the collection configurations
        #  directly is non-standard operation (a standard user should know how
        #  to do it if ever needed)
        yield

        #  Exiting context
        self._collections_context.pop()

    def _collection_from_context(self, collection_name=None):
        if not len(self._collections_context) and collection_name is None:
            collection_name = 'default'
        if collection_name is None:
            collection_name = self._collections_context[-1]

        if collection_name not in self.collections:
            B2WARNING("Creating default collection config")
            self.collections[collection_name] = CollectionConfig(self.path, self.collector)

        return self.collections[collection_name]

    def _append_param_list(self, param, value, collection=None):
        collector = self._collection_from_context(collection).collector
        for par in collector.available_params():
            if par.name == param:
                val = copy.deepcopy(par.values)
                val.append(value)
                collector.param(param, val)
                break

    def _append_param_dict(self, param, key, value, collection=None):
        collector = self._collection_from_context(collection).collector
        for par in collector.available_params():
            if par.name == param:
                val = copy.deepcopy(par.values)
                val[key] = value
                collector.param(param, val)
                break

    def _set_custom_masswidth(self, listname, mass, width, collection=None):
        if mass is None or width is None:
            raise AttributeError('Please set both, mass and width for custom configuration.')
        self._append_param_dict('customMassConfig', listname, (mass, width), collection)

    def dump(self):
        import inspect
        attrs = self.__dict__
        for name, val in attrs.items():
            if name == 'path':
                #  print_path(val)
                pass
            elif name == 'collections':
                for colname, col in val.items():
                    p = create_path()
                    p.add_path(col.path)
                    p.add_module(col.collector)
                    print_path(p)
                    #  print_params(col.collector)
            elif name == 'fixed':
                print(name, ':', str([str(i) for i in val[:10]] + ['...']))
            else:
                print(name, '=', str(val))


def create_configuration(db_components,
                         constraints=None, fixed=None, commands=None, params=None,
                         ignore_undetermined=True, min_entries=0,
                         reco_components=None,
                         granularity='all', min_pval=0,
                         algo=None, path=None, collector=None):
    c = Config()
    c.db_components = db_components
    c.fixed = fixed
    c.constraints = constraints

    cmds = []
    cmds.append('method inversion 3 0.1')
    cmds.append('skipemptycons')
    cmds.append('threads 40 40')
    cmds.append('matiter 1')
    cmds.append('scaleerrors 1. 1.')
    cmds.append('entries 2 2')
    cmds.append('printcounts 2')
    #  cmds.append('histprint')
    cmds.append('monitorresiduals')
    cmds.append('closeandreopen')
    cmds.append('hugecut 50.')
    cmds.append('chiscut 30. 6.')
    cmds.append('outlierdownweighting 3')
    cmds.append('dwfractioncut 0.1')
    cmds.append('presigmas 1.')

    for cmd in cmds:
        c.set_command(cmd)

    for cmd in commands:
        c.set_command(cmd)

    if algo is None:
        algo = create_algorithm(db_components, ignore_undetermined=ignore_undetermined, min_entries=min_entries)

    c.reco_components = reco_components
    if path is None:
        path = create_stdreco_path(reco_components)

    if collector is None:
        collector = create_collector(db_components, granularity=granularity, minPValue=min_pval)

    if params is not None:
        collector.param(params)

    c.algo = algo
    c.path = path
    c.collector = collector
    c.params = params

    c.collections = dict()

    return c


def create_calibration(cfg, name='MillepedeCalibration', tags=None, files=None, timedep=None):
    algo = Belle2.MillepedeAlgorithm(cfg.algo)

    for cmd_name, cmd in cfg.commands.items():
        algorithm.steering().command(cmd)

    consts = cfg.constraints
    if len(consts):
        algo.steering().command('FortranFiles')
        for const in consts:
            algo.steering().command(const.filename)

    def gen_constraints(algorithm, iteration):
        from alignment.constraints import generate_constraints
        if len(consts):
            generate_constraints(consts, timedep, tags)

    algo.steering().command('Parameters')

    #  def fix(label_sets):
    #     for labels in label_sets:
    #         for label in labels:
    #             cfg.algo.steering().command('{} 0.0 -1.'.format(str(label)))

    def fix(labels):
        for label in labels:
            algo.steering().command('{} 0.0 -1.'.format(str(label)))

    fix(cfg.fixed)

    algo.setTimedepConfig(timedep)

    #  Note the default collector and path are ignored
    #  The user is supposed to add at least one own collection
    calibration = Calibration(name,
                              collector=None,
                              algorithms=algo,
                              input_files=None,
                              pre_collector_path=None,
                              database_chain=[CentralDatabase(tag) for tag in tags],
                              output_patterns=None,
                              max_files_per_collector_job=1,
                              backend_args=None
                              )

    for colname, col in cfg.collections.items():
        collector = copy_module(col.collector)

        for const in consts:
            const.configure_collector(collector)

        collector.param('timedepConfig', timedep)

        filelist = []
        if colname in files:
            filelist = files[colname]

        collection = Collection(collector=collector,
                                input_files=filelist,
                                pre_collector_path=col.path,
                                max_files_per_collector_job=1,
                                database_chain=[CentralDatabase(tag) for tag in tags])

        calibration.add_collection(colname, collection)

    calibration.strategies = strategies.SingleIOV
    calibration.max_iterations = 1

    calibration.pre_algorithms = gen_constraints

    return calibration


if __name__ == '__main__':
    cfg = create_configuration(
      db_components=['VXDAlignment'],
      constraints=[alignment.constraints.VXDHierarchyConstraints(), alignment.constraints.CDCWireConstraints()],
      fixed=alignment.parameters.vxd_sensors() + alignment.parameters.vxd_ladders(),
      commands=['method diagonalization 3 0.1', 'scaleerrors 1. 1.'],
      params=dict(minPValue=0.001, externalIterations=0))

    cfg.reprocess_physics(tracks=['RecoTracks'])
    cfg.reprocess_cosmics(tracks=['RecoTracks'], minPValue=0.0001)
    cfg.reprocess_B0cosmics(tracks=['RecoTracks'])

    with cfg.reprocess_physics(collection_name='dimuon_skim'):

        import modularAnalysis as ana
        path = create_path()
        ana.fillParticleList('mu+:mu_dimuon', 'abs(formula(z0)) < 0.5 and abs(d0) < 0.5 and nTracks == 2', writeOut=True, path=path)
        ana.reconstructDecay('Z0:mumu -> mu-:mu_dimuon mu+:mu_dimuon', '', writeOut=True, path=path)
        ana.reconstructDecay(
            'Z0:mumu_4mom -> mu-:mu_dimuon mu+:mu_dimuon',
            'InvM > 10.5296 and InvM < 10.6296',
            writeOut=True,
            path=path)

        #  WARNING/TODO/NOTE/WTF? Now this even makes all GBL composite trajectory fits to fail! on MC!
        #  What has changed in vertex fitters?? (Rave in particular)
        #  However without any vertex fit, it works fine (albeit on ideal MC - have to check the data)
        #  ana.vertexRaveDaughtersUpdate('Z0:mumu', 0.001, path=path)
        #  ana.vertexRaveDaughtersUpdate('Z0:mumu_4mom', 0.001, path=path)
        ana.vertexRave('Z0:mumu', 0.001, path=path)
        ana.vertexRave('Z0:mumu_4mom', 0.001, path=path)

        cfg.add_path(path)

        cfg.collect_tracks('RecoTracks')

        cfg.collect_particles('mu+:mu_dimuon')
        cfg.collect_particles('pi+:good')

        cfg.collect_vertex_decays('Z0:mumu', ip_constraint=False)
        cfg.collect_vertex_decays('Z0:mumu', ip_constraint=True)

        #  Note this sets custom mass and width for all lists of the same name (probably a feature not a bug).
        #  You should have all lists distinct anyway, this is just for testing and I am lazy :-)
        cfg.collect_2body_decays(
            'Z0:mumu_4mom',
            ipvertex_constraint=True,
            ipmass_constraint=True,
            ipboost_constraint=True,
            mass=10.579,
            width=0.05)
        #  All other constraint are versions of mass and/or ip constraint (boost constraint only in full constraint)
        cfg.collect_2body_decays('Z0:mumu_4mom', ipvertex_constraint=True, ipmass_constraint=True, ipboost_constraint=False)
        cfg.collect_2body_decays('Z0:mumu_4mom', ipvertex_constraint=False, ipmass_constraint=True, ipboost_constraint=False)

        cfg.collect_2body_decays('Z0:mumu_4mom', ipvertex_constraint=False, ipmass_constraint=False, ipboost_constraint=False)

    cfg.dump()

    cal = create_calibration(
      cfg,
      name='MyCalib',
      tags=[tag for tag in conditions.default_globaltags],
      files={'dimuon_skim': [os.path.abspath(f) for f in Belle2.Environment.Instance().getInputFilesOverride()]},
      timedep=[([], [(0, 0, 0)])])

    # import pprint
    # pp = pprint.PrettyPrinter(indent=4)
    #  for key, item in cfg.__dict__.items():
    #  try:
    #  if len(item) > 10:
    # item = str(item[:10]) + ' ... '
    #  except:
    #  pass
    # pp.pprint((key, item))

    cal_fw = CAF()
    cal_fw.add_calibration(cal)
    cal_fw.backend = backends.LSF()

    #  Try to guess if we are at KEKCC and change the backend to Local if not
    if multiprocessing.cpu_count() < 10:
        cal_fw.backend = backends.Local(1)

    cal_fw.run()
