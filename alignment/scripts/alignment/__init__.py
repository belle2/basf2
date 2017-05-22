from basf2 import *
set_log_level(LogLevel.INFO)

import sys
import ROOT
from ROOT import Belle2
from caf.framework import Calibration


class MillepedeCalibration():
    """ The generic Millepede calibration collector+algorithm """

    def __init__(self, components, tracks=None, particles=None, vertices=None, primary_vertices=None, magnet=True):
        """
        components are the names of DB obejcts to calibrate (BeamParameters etc.)
        tracks are collections of RecoTracks of fitted charged tracks (usually cosmic rays, no associated particle)
        particles are names of ParticleLists with single charged particles
        vertices are names ParticleLists with at least two body decays fitted with vertex constraint
        primary_vertices are names of ParticleLists with at least two body decays fitted with beam+vertex constraint
        """

        if particles is None:
            particles = []

        if vertices is None:
            vertices = []

        if primary_vertices is None:
            primary_vertices = []

        if tracks is None:
            tracks = ['']

        self.components = components
        self.parameters = []

        self.algo = Belle2.MillepedeAlgorithm()
        std_components = ROOT.vector('string')()
        for component in components:
            std_components.push_back(component)
        self.algo.setComponents(std_components)

        self.algo.steering().command('method diagonalization 3 0.1')
        self.algo.steering().command('skipemptycons')
        self.algo.steering().command('entries 10')
        self.algo.steering().command('hugecut 50')
        self.algo.steering().command('chiscut 30. 6.')
        self.algo.steering().command('outlierdownweighting 3')
        self.algo.steering().command('dwfractioncut 0.1')

        calibrate_vertex = ((components == []) or ('BeamParameters' in components)) and primary_vertices is not None

        self.collector = register_module('MillepedeCollector',
                                         minPValue=0.,
                                         useGblTree=False,
                                         tracks=tracks,
                                         particles=particles,
                                         vertices=vertices,
                                         primaryVertices=primary_vertices,
                                         calibrateVertex=calibrate_vertex)
        self.collector.param('components', components)

        self.path = create_path()
        self.path.add_module('Progress')
        self.path.add_module('Gearbox')

        if magnet:
            self.path.add_module('Geometry')
        else:
            self.path.add_module('Geometry', excludedComponents=['MagneticField'])

        self.path.add_module('SetupGenfitExtrapolation', noiseBetheBloch=False, noiseCoulomb=False, noiseBrems=False)
        # self.path.add_module('DAFRecoFitter')

        self.has_collected_in_this_job = False

    def pre_algo(self, algorithm, iteration):
        pass

    def create(self, name, input_files):
        self.algo.steering().command('Parameters')
        for command in self.parameters:
            self.algo.steering().command(command)

        cal = Calibration(name, self.collector, self.algo, input_files)
        cal.output_patterns.append('Belle2FileCatalog.xml')
        cal.output_patterns.append('constraints.txt')
        cal.output_patterns.append('*.mille')
        # Adding in setup basf2 paths and functions for the collector and algorithm
        cal.pre_collector_path = self.path
        cal.pre_algorithms = self.pre_algo

        return cal

    def collect(self):
        main = create_path()
        main.add_module('RootInput')
        main.add_path(self.path)
        main.add_module(self.collector)
        main.add_module('RootOutput', branchNames=['EventMetaData'])
        process(main)
        self.has_collected_in_this_job = True
        print(statistics)

    def calibrate(self):
        self.algo.steering().command('Parameters')
        for command in self.parameters:
            self.algo.steering().command(command)

        if not self.has_collected_in_this_job:
            input = register_module('RootInput', inputFileName='RootOutput.root')
            input.initialize()

        self.algo.execute()
        self.algo.commit()

    def fixCDCLayerX(self, layer):
        self._fixGlobalParam(Belle2.CDCLayerAlignment.getGlobalUniqueID(), layer, Belle2.CDCLayerAlignment.layerX)

    def fixCDCLayerY(self, layer):
        self._fixGlobalParam(Belle2.CDCLayerAlignment.getGlobalUniqueID(), layer, Belle2.CDCLayerAlignment.layerY)

    def fixCDCLayerRot(self, layer):
        self._fixGlobalParam(Belle2.CDCLayerAlignment.getGlobalUniqueID(), layer, Belle2.CDCLayerAlignment.layerPhi)

    def fixCDCTimeWalk(self, board_id):
        self._fixGlobalParam(Belle2.CDCTimeWalks.getGlobalUniqueID(), board_id, 0)

    def fixCDCTimeZero(self, wire_id):
        self._fixGlobalParam(Belle2.CDCTimeZeros.getGlobalUniqueID(), wire_id, 0)

    def fixVXDid(self, layer, ladder, sensor, segment=0):
        for param in range(1, 7):
            self._fixGlobalParam(
                Belle2.VXDAlignment.getGlobalUniqueID(), Belle2.VxdID(
                    layer, ladder, sensor, segment).getID(), param)

    """
        {{"PXD.Ying"}, {Belle2::VxdID(1, 0, 0, 1)}},
        {{"PXD.Yang"}, {Belle2::VxdID(1, 0, 0, 2)}},
        {{"SVD.Pat"}, {Belle2::VxdID(3, 0, 0, 1)}},
        {{"SVD.Mat"}, {Belle2::VxdID(3, 0, 0, 2)}}
    """

    def fixPXDYing(self):
        self.fixVXDid(1, 0, 0, 1)

    def fixPXDYang(self):
        self.fixVXDid(1, 0, 0, 2)

    def fixSVDPat(self):
        self.fixVXDid(3, 0, 0, 1)

    def fixSVDMat(self):
        self.fixVXDid(3, 0, 0, 2)

    def _fixGlobalParam(self, uid, element, param):
        label = Belle2.GlobalLabel()
        label.construct(uid, element, param)
        self.parameters.append(str(label.label()) + ' 0.0 -1.')
