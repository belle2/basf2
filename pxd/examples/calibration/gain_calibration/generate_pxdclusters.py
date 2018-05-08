#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
from ROOT import Belle2

import shape_utils


class PrintSimplePXDClusterShapesModule(Module):
    """
    Collector module for cluster shape calibration
    """

    def __init__(self, outdir='tmp', pixelkind=4):
        """constructor"""
        # call constructor of base class, required if you implement __init__ yourself!
        super().__init__()
        # and do whatever else is necessary like declaring member variables
        self.outdir = outdir
        self.pixelkind = pixelkind
        self.track_counter = 0
        self.file_counter = 0

        self.file_header = 'sensorID\t'            # SensorID of cluster
        self.file_header += 'pixelkind\t'          # kind of hit pixel cells
        self.file_header += 'cluster\t'            # cluster digits string
        self.file_header += 'clu_uID\t'            # cluster u cell number
        self.file_header += 'clu_vID\t'            # cluster v cell number
        self.file_header += '\n'

    def initialize(self):
        """Open empty text file for writing cluster shape training data and put header."""
        self.file = open(self.outdir + '/PXDClusters_index_{:d}.txt'.format(self.file_counter), 'w')
        self.file.write(self.file_header)

    def terminate(self):
        """Close the output file."""
        self.file.close()

    def write_cluster(self, cluster):
        """ Write cluster to output file."""
        sensor_info = Belle2.VXD.GeoCache.get(cluster.getSensorID())
        clusterstring, minu, minv, reject, kinds = self.process_cluster(cluster, sensor_info)

        # Filter all cluster track pairs failing selection
        if reject:
            return

        clu_uID = sensor_info.getUCellID(cluster.getU())
        clu_vID = sensor_info.getVCellID(cluster.getV())
        kind = kinds.pop()

        # Collect all relevant training data in list
        data = []
        data.append(str(int(cluster.getSensorID())))
        data.append(str(kind))
        data.append(clusterstring)
        data.append(str(clu_uID))
        data.append(str(clu_vID))

        self.track_counter += 1
        # Make sure files do not get too big
        if self.track_counter % 50000 == 0:
            self.file.close()
            self.file_counter += 1
            self.file = open(self.outdir + '/PXDClusters_index_{:d}.txt'.format(self.file_counter), 'w')
            self.file.write(self.file_header)

        # Dump training data into a text file
        s = '\t'.join(data) + '\n'
        self.file.write(s)

    def process_cluster(self, cluster, sensor_info):
        """ Returns cluster string from list of related digits"""

        # get sorted list of digits caused by truehits
        digits = cluster.getRelationsTo("PXDDigits")

        # compute the shape string
        minu = min([digit.getUCellID() for digit in digits])
        minv = min([digit.getVCellID() for digit in digits])
        clusterstring = str(len(digits))
        reject = False
        kinds = set()

        for i, digit in enumerate(digits):
            charge = int(digit.getCharge())
            clusterstring += 'D' + str(digit.getVCellID()) + '.' + str(digit.getUCellID()) + '.' + str(charge)
            kind = shape_utils.getPixelKind(sensor_info.getVPitch(sensor_info.getVCellPosition(digit.getVCellID())))
            kinds.add(kind)

            # Cluster has wrong pixel kind
            if (not kind == self.pixelkind) and (not self.pixelkind == 4):
                reject = True
            # Cluster at sensor edge
            if digit.getVCellID() <= 0 or digit.getVCellID() >= 767:
                reject = True
            # Cluster at sensor edge
            if digit.getUCellID() <= 0 or digit.getUCellID() >= 249:
                reject = True

        # Check cluster has not digits with different pixel kind
        if not len(kinds) == 1:
            reject = True

        return clusterstring, minu, minv, reject, kinds

    def event(self):
        """ Dump cluster to file"""

        clusters = Belle2.PyStoreArray("PXDClusters")

        # iterate over all truehits and look for the clusters
        for cls in clusters:
            self.write_cluster(cls)


class GeneratePXDClusterConfig():

    def __init__(self, outdir='tmp'):
        # Manage configuration of variabels contained here
        self.variables = {
            'NoiseFraction': 0.0000000,
            'PedestalMean': 155.0,
            'PedestalRMS': 45.0,
            'ChargeThreshold': 5,
            'SourceDrainBorder': 6.3,
            'ClearBorder': 4.2,
            'Gq': 0.77,
            'ElectronicNoise': 170,
            'ADCUnit': 130.0,
            'nEvents': 5000,
            'Magnet-Off': True,
            'PixelKind': 0,
            'Momenta': [1.0],
            'ThetaParams': [0, 180],
            'PhiParams': [0, 180],
            'Outdir': outdir,
            'NoiseSN': 3,
            'SeedSN': 5,
            'ClusterSN': 8,
            'pdgCodes': [-211],  # [211, -211, 11, -11]
            'thetaGeneration': 'uniformCos',
            'phiGeneration': 'uniform',
        }

        # Variables for fitting
        self.fit_variables = []

    def addFitVariable(self, name):
        if name in self.variables:
            self.fit_variables.append(name)

    def getFitVariableValues(self):
        return tuple([float(self.variables[name]) for name in self.fit_variables])

    def setFitVariableValues(self, x):
        for index in range(len(x)):
            name = self.fit_variables[index]
            self.variables[name] = x[index]

    def getOutdir(self):
        return self.variables['Outdir']

    def getPixelkind(self):
        return self.variables['PixelKind']

    def getParameters(self):
        parameters = {}
        parameters['EventInfoSetter'] = {"evtNumList": [self.variables['nEvents']], }
        parameters['Gearbox'] = {
            "overrideMultiple": [
                ("//ChargeThreshold",
                 "{:d}".format(
                     self.variables['ChargeThreshold']),
                    "ADU"),
                ("//NoiseFraction",
                 "{:f}".format(
                     self.variables['NoiseFraction']),
                 ""),
                ("//SourceBorderSmallPixel",
                 "{:f}".format(
                     self.variables['SourceDrainBorder']),
                    "um"),
                ("//ClearBorderSmallPixel",
                 "{:f}".format(
                     self.variables['ClearBorder']),
                    "um"),
                ("//DrainBorderSmallPixel",
                 "{:f}".format(
                     self.variables['SourceDrainBorder']),
                    "um"),
                ("//SourceBorderLargePixel",
                 "{:f}".format(
                     self.variables['SourceDrainBorder']),
                    "um"),
                ("//ClearBorderLargePixel",
                 "{:f}".format(
                     self.variables['ClearBorder']),
                    "um"),
                ("//DrainBorderLargePixel",
                 "{:f}".format(
                     self.variables['SourceDrainBorder']),
                    "um"),
            ]}
        if self.variables['Magnet-Off']:
            parameters['Geometry'] = {"components": ['PXD'], }
        else:
            parameters['Geometry'] = {"components": ['MagneticField', 'PXD'], }

        # For setting of vertex close to surface of for PXD kinds of pixels set:
        # 55 um pitch: sensor 1.3.2 , vertex: x: [-0.2050,-0.2], y: [1.35], z: [0.7,0.7055]
        # 60 um pitch: sensor 1.3.2 , vertex: x: [-0.2050,-0.2], y: [1.35], z: [-1.5,-1.5060]
        # 70 um pitch: sensor 2.4.2 , vertex: x: [-0.2050,-0.2], y: [2.18], z: [0.9,0.9070]
        # 85 um pitch: sensor 2.4.2 , vertex: x: [-0.2050,-0.2], y: [2.18], z: [-2.0,-2.0085]

        parameters['ParticleGun'] = {"nTracks": 1,
                                     "pdgCodes": self.variables['pdgCodes'],
                                     "momentumGeneration": 'discrete',
                                     "momentumParams": self.variables['Momenta'] + [1] * len(self.variables['Momenta']),
                                     "thetaGeneration": self.variables['thetaGeneration'],
                                     "thetaParams": self.variables['ThetaParams'],
                                     "phiGeneration": self.variables['phiGeneration'],
                                     "phiParams": self.variables['PhiParams'],
                                     "xVertexGeneration": 'uniform',
                                     "xVertexParams": [-0.2050, -0.2],
                                     "yVertexGeneration": 'fixed',
                                     "yVertexParams": [1.35],
                                     "zVertexGeneration": 'uniform',
                                     "zVertexParams": [0.7, 0.7055],
                                     "independentVertices": False,
                                     }
        if self.variables['PixelKind'] == 0:
            pass
        elif self.variables['PixelKind'] == 1:
            parameters['ParticleGun'].update({'zVertexParams': [-1.5060, -1.5]})
        elif self.variables['PixelKind'] == 2:
            parameters['ParticleGun'].update({'yVertexParams': [2.18], 'zVertexParams': [0.9, 0.9070]})
        elif self.variables['PixelKind'] == 3:
            parameters['ParticleGun'].update({'yVertexParams': [2.18], 'zVertexParams': [-2.0085, -2.0]})

        parameters['PXDDigitizer'] = {"Gq": self.variables['Gq'],
                                      "ElectronicNoise": self.variables['ElectronicNoise'],
                                      "ADCUnit": self.variables["ADCUnit"],
                                      "PedestalMean": self.variables["PedestalMean"],
                                      "PedestalRMS": self.variables["PedestalRMS"],
                                      }
        parameters['PXDClusterizer'] = {"NoiseSN": self.variables["NoiseSN"],
                                        "SeedSN": self.variables["SeedSN"],
                                        "ClusterSN": self.variables["ClusterSN"],
                                        }
        return parameters


def add_generate_pxdclusters_phase2(path, config):

    parameters = config.getParameters()
    # Now let's add modules to simulate our events.
    eventinfosetter = path.add_module("EventInfoSetter")
    eventinfosetter.param(parameters['EventInfoSetter'])
    particlegun = path.add_module("ParticleGun")
    particlegun.param(parameters['ParticleGun'])
    # path.add_module("EvtGenInput")
    gearbox = path.add_module("Gearbox")
    gearbox.param(parameters['Gearbox'])
    # gearbox.param('fileName', 'geometry/Beast2_phase2.xml')
    geometry = path.add_module("Geometry")
    geometry.param(parameters['Geometry'])
    path.add_module("FullSim")
    pxddigi = path.add_module("PXDDigitizer")
    pxddigi.param(parameters['PXDDigitizer'])
    pxdclu = path.add_module("PXDClusterizer")
    pxdclu.param(parameters['PXDClusterizer'])
    path.add_module(PrintSimplePXDClusterShapesModule(outdir=config.getOutdir(), pixelkind=config.getPixelkind()))
    path.add_module("Progress")
