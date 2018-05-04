#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
from ROOT import Belle2


class PrintPXDClusterShapesModule(Module):
    """
    Module for printing training/test data for PXDClusterShape corrections to file.
    """

    def __init__(self, outdir='tmp', pixelkind=0, min_mom=0.02, max_mom=6.0, turchetta=False):
        """constructor"""
        # call constructor of base class, required if you implement __init__ yourself!
        super().__init__()
        # and do whatever else is necessary like declaring member variables
        self.outdir = outdir
        self.pixelkind = pixelkind
        self.min_mom = min_mom
        self.max_mom = max_mom
        self.turchetta = turchetta
        self.track_counter = 0
        self.file_counter = 0

        self.file_header = 'sensorID\t'            # SensorID of cluster
        self.file_header += 'pixelkind\t'          # kind of hit pixel cells
        self.file_header += 'cluster\t'            # cluster digits string
        self.file_header += 'trk_u[mm]\t'          # intersect of track on sensor, in cluster coordinates
        self.file_header += 'trk_v[mm]\t'          # intersect of track on sensor, in cluster coordinates
        self.file_header += 'trk_dudw\t'           # tangent of incidence angle, in cluster coordinates
        self.file_header += 'trk_dvdw\t'           # tangent of incidence angle, in cluster coordinates
        self.file_header += 'trk_mom[GeV]\t'       # momentum of track
        self.file_header += 'pdg\t'                # pdg code of track
        self.file_header += 'trk_cov_uu[mm2]\t'    # covariance of track intersection
        self.file_header += 'trk_cov_vv[mm2]\t'    # covariance of track intersection
        self.file_header += 'trk_cov_uv[mm2]'      # covariance of track intersection
        if self.turchetta:
            self.file_header += '\t'
            self.file_header += 'hit_off_u[mm]\t'    # cluster offset u, computed turchetta style
            self.file_header += 'hit_off_v[mm]\t'    # cluster offset v, computed turchetta style
            self.file_header += 'hit_cov_uu[mm2]\t'   # cluster covariance, computed turchetta style
            self.file_header += 'hit_cov_vv[mm2]\t'   # cluster covariance, computed turchetta style
            self.file_header += 'hit_cov_uv[mm2]'     # cluster covariance, computed turchetta style
        self.file_header += '\n'

    def initialize(self):
        """Open empty text file for writing cluster shape training data and put header."""
        self.file = open(self.outdir + '/PXDClusters_index_{:d}_kind_{:d}.txt'.format(self.file_counter, self.pixelkind), 'w')
        self.file.write(self.file_header)

    def terminate(self):
        """Close the output file."""
        self.file.close()

    def write_truehit(self, truehit, cluster):
        """ Write truehit-cluster pair to output file."""
        sensor_info = Belle2.VXD.GeoCache.get(truehit.getSensorID())
        clusterstring, minu, minv, reject, kinds = self.process_cluster(truehit, cluster, sensor_info)

        # Filter all cluster track pairs failing selection
        if reject:
            return

        trk_u = truehit.getU() - sensor_info.getUCellPosition(minu)
        trk_v = truehit.getV() - sensor_info.getVCellPosition(minv)
        kind = kinds.pop()
        mom = truehit.getMomentum()

        if self.min_mom > mom.Mag() or self.max_mom < mom.Mag():
            # Wrong momentum, ignore this one
            return

        # Default pdg code, in case branch is MCParticels missing
        pdg = 0
        for mcp in truehit.getRelationsFrom("MCParticles"):
            pdg = mcp.getPDG()
            # Only look at primary particles
            if not mcp.hasStatus(1):
                return

        # Collect all relevant training data in list
        data = []
        data.append(str(int(truehit.getSensorID())))
        data.append(str(kind))
        data.append(clusterstring)
        data.append(str(10 * trk_u))
        data.append(str(10 * trk_v))
        data.append(str(mom[0] / mom[2]))
        data.append(str(mom[1] / mom[2]))
        data.append(str(mom.Mag()))
        data.append(str(pdg))
        data.append(str(0.00000001))  # put 100nm x 100nm for variance of truehit
        data.append(str(0.00000001))  # put 100nm x 100nm for variance of truehit
        data.append(str(0.0))        # put zero for cross covariance

        if self.turchetta:
            # Add colums for printing the hit position estimated in the clusterizer
            hit_u = cluster.getU() - sensor_info.getUCellPosition(minu)
            hit_v = cluster.getV() - sensor_info.getVCellPosition(minv)
            hit_cov_uu = cluster.getUSigma() * cluster.getUSigma()
            hit_cov_vv = cluster.getVSigma() * cluster.getVSigma()
            hit_cov_uv = cluster.getRho() * cluster.getUSigma() * cluster.getVSigma()
            data.append(str(10 * hit_u))
            data.append(str(10 * hit_v))
            data.append(str(100 * hit_cov_uu))
            data.append(str(100 * hit_cov_vv))
            data.append(str(100 * hit_cov_uv))

        self.track_counter += 1
        # Make sure files do not get too big
        if self.track_counter % 50000 == 0:
            self.file.close()
            self.file_counter += 1
            self.file = open(self.outdir + '/PXDClusters_index_{:d}_kind_{:d}.txt'.format(self.file_counter, self.pixelkind), 'w')
            self.file.write(self.file_header)

        # Dump training data into a text file
        s = '\t'.join(data) + '\n'
        self.file.write(s)

    def process_cluster(self, truehit, cluster, sensor_info):
        """ Returns cluster string from list of related digits"""

        # get sorted list of digits caused by truehits
        # true_digits = truehit.getRelationsFrom("PXDDigits")
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
            kind = self.getPixelKind(sensor_info.getVPitch(sensor_info.getVCellPosition(digit.getVCellID())))
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

    def getPixelKind(self, VPitch):
        if abs(VPitch - 0.0055) < 0.0001:
            return 0
        elif abs(VPitch - 0.0060) < 0.0001:
            return 1
        elif abs(VPitch - 0.0070) < 0.0001:
            return 2
        elif abs(VPitch - 0.0085) < 0.0001:
            return 3
        else:
            return -1

    def event(self):
        """ Dump cluster shape data by iterating over all primary particles"""

        truehits = Belle2.PyStoreArray("PXDTrueHits")

        # iterate over all truehits and look for the clusters
        for i, truehit in enumerate(truehits):

            # meh, something strange with the momentum, ignore this one
            if truehit.getMomentum()[2] == 0:
                continue

            if isinstance(truehit, Belle2.PXDTrueHit):
                clusters = truehit.getRelationsFrom("PXDClusters")

                # now check if we find a cluster
                for j, cls in enumerate(clusters):
                    # we ignore all clusters where less then 100 electrons come from
                    # our truehit
                    if clusters.weight(j) < 100:
                        continue

                    self.write_truehit(truehit, cls)


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
            'Turchetta': False,
            'Momenta': [1.0],
            'ThetaParams': [0, 90],
            'PhiParams': [0, 180],
            'Outdir': outdir,
            'NoiseSN': 3.0,  # 3
            'SeedSN': 3.0,   # 5
            'ClusterSN': 3.0,  # 8
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

    def getMomenta(self):
        return self.variables['Momenta']

    def getTurchettaFlag(self):
        return self.variables['Turchetta']

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


def add_generate_pxdclusters(path, config, use_default_pxd=True, min_mom=0.02, max_mom=6.0):

    parameters = config.getParameters()

    # Now let's add modules to simulate our events.
    eventinfosetter = path.add_module("EventInfoSetter")
    eventinfosetter.param(parameters['EventInfoSetter'])
    gearbox = path.add_module("Gearbox")
    if not use_default_pxd:
        gearbox.param(parameters['Gearbox'])
    geometry = path.add_module("Geometry")
    geometry.param(parameters['Geometry'])
    particlegun = path.add_module("ParticleGun")
    particlegun.param(parameters['ParticleGun'])
    path.add_module("FullSim")
    pxddigi = path.add_module("PXDDigitizer")
    if not use_default_pxd:
        pxddigi.param(parameters['PXDDigitizer'])
    pxdclu = path.add_module("PXDClusterizer")
    if not use_default_pxd:
        pxdclu.param(parameters['PXDClusterizer'])

    path.add_module(PrintPXDClusterShapesModule(outdir=config.getOutdir(), pixelkind=config.getPixelkind(),
                                                turchetta=config.getTurchettaFlag(), min_mom=min_mom, max_mom=max_mom))

    path.add_module("Progress")


def add_generate_pxdclusters_bbbar(path, config, bkgfiles=None, bkgOverlay=False, use_default_pxd=True, min_mom=0.02, max_mom=6.0):

    parameters = config.getParameters()

    # Set number of events to generate
    eventinfosetter = path.add_module("EventInfoSetter")
    eventinfosetter.param(parameters['EventInfoSetter'])

    # Generate BBbar events
    path.add_module("EvtGenInput")

    # Background mixing or overlay input before process forking
    if bkgfiles:
        if bkgOverlay:
            bkginput = register_module('BGOverlayInput')
            bkginput.param('inputFileNames', bkgfiles)
            path.add_module(bkginput)

    # Geometry parameter database
    if 'Gearbox' not in path:
        gearbox = register_module('Gearbox')
        if not use_default_pxd:
            gearbox.param(parameters['Gearbox'])
        path.add_module(gearbox)

    # Detector geometry
    if 'Geometry' not in path:
        geometry = register_module('Geometry', useDB=False)
        # geometry.param(parameters['Geometry'])
        path.add_module(geometry)

    # Detector simulation
    if 'FullSim' not in path:
        g4sim = register_module('FullSim')
        path.add_module(g4sim)

    if 'PXDDigitizer' not in path:
        pxddigi = path.add_module("PXDDigitizer")
        if not use_default_pxd:
            pxddigi.param(parameters['PXDDigitizer'])

    # Background overlay executor - after all digitizers
    if bkgfiles is not None and bkgOverlay:
        pass
        # path.add_module('BGOverlayExecutor', PXDDigitsName='')
        # path.add_module("PXDDigitSorter", digits='')

    if 'PXDClusterizer' not in path:
        pxdclu = path.add_module("PXDClusterizer")
        if not use_default_pxd:
            pxdclu.param(parameters['PXDClusterizer'])

    path.add_module(PrintPXDClusterShapesModule(outdir=config.getOutdir(), pixelkind=config.getPixelkind(),
                                                turchetta=config.getTurchettaFlag(), min_mom=min_mom, max_mom=max_mom))

    path.add_module("Progress")
