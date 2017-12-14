from basf2 import *
set_log_level(LogLevel.INFO)

import sys
import ROOT
import basf2
from ROOT import Belle2
from caf.framework import Calibration

from collections import OrderedDict

UVWABC = [1, 2, 3, 4, 5, 6]


def collect(calibration, basf2_args=[]):
    import pickle
    import subprocess

    main = create_path()
    main.add_module('RootInput', inputFileNames=calibration.input_files)
    main.add_module('HistoManager', histoFileName='RootOutput.root')
    main.add_path(calibration.pre_collector_path)
    main.add_module(calibration.collector)

    path_file_name = calibration.name + '.path'
    with open(path_file_name, 'bw') as serialized_path_file:
        pickle.dump(serialize_path(main), serialized_path_file)

    subprocess.call(["basf2", "--execute-path", path_file_name] + basf2_args)


def calibrate(calibration, input_file='RootOutput.root'):
    for algo in calibration.algorithms:
        algo.algorithm.setInputFileNames([input_file])
        algo.algorithm.execute()
        algo.algorithm.commit()


class MillepedeCalibration():
    """ The generic Millepede calibration collector+algorithm """

    def __init__(self,
                 components=None,
                 tracks=None,
                 particles=None,
                 vertices=None,
                 primary_vertices=None,
                 path=None):
        """
        components are the names of DB objects to calibrate (BeamParameters etc.)
        tracks are collections of RecoTracks of fitted charged tracks (usually cosmic rays, no associated particle)
        particles are names of ParticleLists with single charged particles
        vertices are names ParticleLists with at least two body decays fitted with vertex constraint
        primary_vertices are names of ParticleLists with at least two body decays fitted with beam+vertex constraint
        """

        if components is None:
            components = []

        # By default, at least the raw RecoTracks are fitted (let's specify its name explicitly here)
        if (tracks is None) and (particles is None) and (vertices is None) and (primary_vertices is None):
            tracks = ['RecoTracks']

        if particles is None:
            particles = []

        if vertices is None:
            vertices = []

        if primary_vertices is None:
            primary_vertices = []

        if path is None:
            path = create_path()
            path.add_module('Gearbox')
            path.add_module('Geometry')
            path.add_module('SetupGenfitExtrapolation', noiseBetheBloch=False, noiseCoulomb=False, noiseBrems=False)

        #: the algorithm
        self.algo = Belle2.MillepedeAlgorithm()
        #: pre-collector path
        self.path = path
        #: the collector module
        self.collector = register_module('MillepedeCollector')
        #: Parameter config at algo level (fixing)
        self.parameters = OrderedDict()
        #: Commands for pede steering
        self.commands = OrderedDict()

        self.set_components(components)

        self.collector.param('tracks', tracks)
        self.collector.param('particles', particles)
        self.collector.param('vertices', vertices)
        self.collector.param('primaryVertices', primary_vertices)
        self.collector.param('minPValue', 0.)
        self.collector.param('useGblTree', True)

        self.set_command('method diagonalization 3 0.1')
        self.set_command('skipemptycons')
        self.set_command('entries 10')
        self.set_command('hugecut 50')
        self.set_command('chiscut 30. 6.')
        self.set_command('outlierdownweighting 3')
        self.set_command('dwfractioncut 0.1')

    def set_components(self, components):
        """ Select db objects for calibration from list of their names """
        #: db objects for calibration
        self.components = components
        calibrate_vertex = ((components == []) or ('BeamParameters' in components))

        std_components = ROOT.vector('string')()
        for component in components:
            std_components.push_back(component)

        self.algo.setComponents(std_components)
        self.collector.param('components', components)
        self.collector.param('calibrateVertex', calibrate_vertex)

    def set_command(self, cmd_name, command=''):
        """ Set command for Millepede steering """
        if command is None:
            self.commands[cmd_name] = ''
            return

        if command == '':
            command = cmd_name
        else:
            command = cmd_name + ' ' + str(command)

        cmd_words = str(command).split(' ')
        self.commands[cmd_words[0]] = command

    def pre_algo(self, algorithm, iteration):
        """ Fcn to execute before algorithm... """
        pass

    def create(self, name, input_files):
        """ Create the CAF Calibration object """

        # We keep steering options for pede in python
        # as long as possible for the user before passing to algo
        for command in self.commands.values():
            self.algo.steering().command(command)
        # Add parameter fixing/setting
        self.algo.steering().command('Parameters')
        for command in self.parameters.values():
            self.algo.steering().command(command)

        # If you call this, you are going to use calibration framework most likely.
        # This NOW does not anymore move/copy files from collector to algorithm, so
        # we have to remeber where they are located at the time of creation
        if not self.get_param('useGblTree'):
            self.set_param(True, 'absFilePaths')

        cal = Calibration(name, self.collector, self.algo, input_files)
        cal.output_patterns.append('Belle2FileCatalog.xml')
        cal.output_patterns.append('constraints.txt')
        cal.output_patterns.append('*.mille')
        # Adding in setup basf2 paths and functions for the collector and algorithm
        cal.pre_collector_path = self.path
        cal.pre_algorithms = self.pre_algo

        return cal

    def get_param(self, param, module=None):
        """ Get parameter of the collector module or any module in path (given its name) """
        module = self.get_module(module)
        for mpi in module.available_params():
            if mpi.name == param:
                return mpi.values
        return None

    def set_param(self, value, param, module=None):
        """ Set parameter of the collector module or any module in path (given its name) """
        module = self.get_module(module)
        if module is not None:
            module.param(param, value)

    def get_module(self, module=None):
        """ Get collector module or any other module from path (given its name) """
        if module is None or module == self.collector.name() or module == self.collector:
            module = self.collector
        else:
            for mod in self.path.modules():
                if mod.name() == module:
                    module = mod
                    break
        if isinstance(module, basf2.Module):
            return module
        return None

    def __getstate__(self):
        """ serialization """

        state = self.__dict__.copy()
        state['path'] = serialize_path(self.path)
        state['collector'] = serialize_module(self.collector)
        return state

    def __setstate__(self, state):
        """ de-serialization """

        self.__dict__.update(state)
        self.collector = deserialize_module(state['collector'])
        self.path = deserialize_path(state['path'])

    def fixGlobalParam(self, uid, element, param, value=0.):
        """ Generic fcn to manipulate (fix to given value) parameters identified by db object id (uid),
            element and parameter number """
        label = Belle2.GlobalLabel()
        label.construct(uid, element, param)
        self.parameters[str(label.label())] = (str(label.label()) + ' ' + str(value) + ' -1.')

    # CDC helpers --------------------------------------------------------------------------------------------------

    def fixCDCLayerX(self, layer):
        """ fix CDC layer X-shift """
        self.fixGlobalParam(Belle2.CDCLayerAlignment.getGlobalUniqueID(), layer, Belle2.CDCLayerAlignment.layerX)

    def fixCDCLayerY(self, layer):
        """ fix CDC layer Y-shift """
        self.fixGlobalParam(Belle2.CDCLayerAlignment.getGlobalUniqueID(), layer, Belle2.CDCLayerAlignment.layerY)

    def fixCDCLayerRot(self, layer):
        """ fix CDC layer Phi rotation """
        self.fixGlobalParam(Belle2.CDCLayerAlignment.getGlobalUniqueID(), layer, Belle2.CDCLayerAlignment.layerPhi)

    def fixCDCTimeWalk(self, board_id):
        """ fix CDC time walk parameter for given board """
        self.fixGlobalParam(Belle2.CDCTimeWalks.getGlobalUniqueID(), board_id, 0)

    def fixCDCTimeZero(self, wire_id):
        """ fix CDC tie zero parameter for given wire """
        self.fixGlobalParam(Belle2.CDCTimeZeros.getGlobalUniqueID(), wire_id, 0)

    # VXD helpers --------------------------------------------------------------------------------------------------

    def fixVXDid(self, layer, ladder, sensor, segment=0, parameters=[1, 2, 3, 4, 5, 6]):
        """ Fix VXD element parameters by layer,ladder,sensor and segment number """
        for param in parameters:
            self.fixGlobalParam(Belle2.VXDAlignment.getGlobalUniqueID(), Belle2.VxdID(
                layer, ladder, sensor, segment).getID(), param)
        """
            {{"PXD.Ying"}, {Belle2::VxdID(1, 0, 0, 1)}},
            {{"PXD.Yang"}, {Belle2::VxdID(1, 0, 0, 2)}},
            {{"SVD.Pat"}, {Belle2::VxdID(3, 0, 0, 1)}},
            {{"SVD.Mat"}, {Belle2::VxdID(3, 0, 0, 2)}}
        """

    def fixPXDYing(self, parameters=UVWABC):
        """ fix PXD Ying half-shell """
        self.fixVXDid(1, 0, 0, 1, parameters)

    def fixPXDYang(self, parameters=UVWABC):
        """ fix PXD Yang half-shell """
        self.fixVXDid(1, 0, 0, 2, parameters)

    def fixSVDPat(self, parameters=UVWABC):
        """ fix SVD Pat half-shell """
        self.fixVXDid(3, 0, 0, 1, parameters)

    def fixSVDMat(self, parameters=UVWABC):
        """ fix SVD Mat half-shell """
        self.fixVXDid(3, 0, 0, 2, parameters)

    # EKLM helpers --------------------------------------------------------------------------------------------------

    def fixEKLMSector(self, endcap, layer, sector, parameters=[1, 2, 6]):
        """ Fix EKLM sector parameters """

        for ipar in parameters:
            self.fixGlobalParam(Belle2.EKLMAlignment.getGlobalUniqueID(),
                                Belle2.EKLMElementID(endcap, layer, sector).getGlobalNumber(),
                                ipar)

    def fixEKLMSegment(self, endcap, layer, sector, plane, segment, parameters=[1, 2, 6]):
        """ Fix EKLM segment parameters """
        for ipar in parameters:
            self.fixGlobalParam(Belle2.EKLMAlignment.getGlobalUniqueID(),
                                Belle2.EKLMElementID(endcap, layer, sector, plane, segment).getGlobalNumber(),
                                ipar)

    # BKLM helpers --------------------------------------------------------------------------------------------------

    def fixBKLMModule(self, sector, layer, forward, parameters=UVWABC):
        """ Fix a BKLM module """
        for ipar in parameters:
            self.fixGlobalParam(
                Belle2.BKLMAlignment.getGlobalUniqueID(),
                Belle2.BKLMElementID(
                    forward,
                    sector,
                    layer).getID(),
                ipar)

    def fixBKLM(self, sectors=range(1, 9), layers=range(1, 16), forbackwards=[0, 1]):
        """ Fix (all by default) BKLM modules """
        for sector in sectors:
            for layer in layers:
                for forward in forbackwards:
                    self.fixBKLMModule(forward, sector, layer)
