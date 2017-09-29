#!/usr/bin/env python3
# -*- coding: utf-8 -*-


class PXDClusterShape_ParticleGunSetting():
    """
    A module return dictionaries for particle gun.
    Module is specific for ClusterShape correction module.
    Module set gan position for 4 pixel kinds.
    For setting of vertex close to surface of for PXD kinds of pixels set:
    55 um pitch: sensor 1.3.2 , vertex: x: [-0.2050,-0.2], y: [1.35], z: [0.7,0.7055]
    60 um pitch: sensor 1.3.2 , vertex: x: [-0.2050,-0.2], y: [1.35], z: [-1.5,-1.5060]
    70 um pitch: sensor 2.4.2 , vertex: x: [-0.2050,-0.2], y: [2.18], z: [0.9,0.9070]
    85 um pitch: sensor 2.4.2 , vertex: x: [-0.2050,-0.2], y: [2.18], z: [-2.0,-2.0085]
    """
    def __init__(self):
        """Basic common intialization of PXD Cluster Shape Particle Gun Setting."""
        CommonSets = {
            'nTracks': 1,
            'varyNTracks': False,
            'pdgCodes': [211, -211, 11, -11],
            'momentumGeneration': 'normal',
            'momentumParams': [2, 0.2],
            'phiGeneration': 'uniform',
            'phiParams': [0, 180],
            # particlegun.param('phiParams', [20, 160])
            'thetaGeneration': 'uniformCos',
            # particlegun.param('thetaGeneration', 'uniform')
            'thetaParams': [1, 179],
            'xVertexGeneration': 'uniform',
            'xVertexParams': [-0.2050, -0.2],
            'yVertexGeneration': 'fixed',
            'zVertexGeneration': 'uniform',
            'independentVertices': False
            }

        #: data_pixelType0 is preset for PXD pixel size 55 x 50 um
        self.data_pixelType0 = CommonSets.copy()
        self.data_pixelType0.update({
            'yVertexParams': [1.35],
            # 'yVertexParams': [2.18],
            'zVertexParams': [0.7, 0.7055]
            # 'zVertexParams': [-1.5060, -1.5],
            # 'zVertexParams': [0.9, 0.9070],
            # 'zVertexParams': [-2.0085, -2.0],
            })
        #: data_pixelType1 is preset for PXD pixel size 60 x 50 um
        self.data_pixelType1 = CommonSets.copy()
        self.data_pixelType1.update({
            'yVertexParams': [1.35],
            # 'yVertexParams': [2.18],
            # 'zVertexParams': [0.7, 0.7055],
            'zVertexParams': [-1.5060, -1.5]
            # 'zVertexParams': [0.9, 0.9070],
            # 'zVertexParams': [-2.0085, -2.0],
            })
        #: data_pixelType2 is preset for PXD pixel size 70 x 50 um
        self.data_pixelType2 = CommonSets.copy()
        self.data_pixelType2.update({
            # 'yVertexParams': [1.35],
            'yVertexParams': [2.18],
            # 'zVertexParams': [0.7, 0.7055],
            # 'zVertexParams': [-1.5060, -1.5],
            'zVertexParams': [0.9, 0.9070]
            # 'zVertexParams': [-2.0085, -2.0],
            })
        #: data_pixelType3 is preset for PXD pixel size 85 x 50 um
        self.data_pixelType3 = CommonSets.copy()
        self.data_pixelType3.update({
            # 'yVertexParams': [1.35],
            'yVertexParams': [2.18],
            # 'zVertexParams': [0.7, 0.7055],
            # 'zVertexParams': [-1.5060, -1.5],
            # 'zVertexParams': [0.9, 0.9070],
            'zVertexParams': [-2.0085, -2.0]
            })
        #: data_ParticleGunDefault is preset for particle gun in IP
        self.data_ParticleGunDefault = {
            }

    def SetParticleGun(self, PixelType):
        """Set parameters of PXD Cluster Shape Particle Gun Setting.."""
        if PixelType == 0:
            return self.data_pixelType0
        elif PixelType == 1:
            return self.data_pixelType1
        elif PixelType == 2:
            return self.data_pixelType2
        elif PixelType == 3:
            return self.data_pixelType3
        else:
            return self.data_ParticleGunDefault

    def SetLayerNo(self, PixelType):
        """Set Layer No of PXD Cluster Shape Particle Gun Setting."""
        if PixelType <= 1:
            return 1
        else:
            return 2

    def SetLadderNo(self, PixelType):
        """Set Ladder No of PXD Cluster Shape Particle Gun Setting."""
        if PixelType <= 1:
            return 3
        else:
            return 4

    def SetSensorNo(self, PixelType):
        """Set Sensor No of PXD Cluster Shape Particle Gun Setting."""
        return 2
