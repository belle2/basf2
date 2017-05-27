#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import os
import sys
from basf2 import *

import ROOT
from ROOT import Belle2

import math
import numpy


class GlobalDeformation:
    """
    Base class to calculate global deformation of the detector
    """

    def __init__(self, scale):
        """ Constructor, sets the scale of the transformation """
        self.scale = scale

    def __call__(self, xyz):
        """ Return TVector3 with difference in global position induced by the deformation,
        given the original x,y,z position as TVector3 or a list with length of 3.
        """

        rphiz = self._xyz_to_rphiz(xyz)
        d_rphiz = self._transform(rphiz[0], rphiz[1], rphiz[2])

        rphiz[0] += d_rphiz[0]
        rphiz[1] += d_rphiz[1]
        rphiz[2] += d_rphiz[2]

        new_xyz = self._rphiz_to_xyz(rphiz)

        d_xyz = ROOT.TVector3(new_xyz[0] - xyz[0], new_xyz[1] - xyz[1], new_xyz[2] - xyz[2])

        return d_xyz

    def _transform(self, r, phi, z):
        pass

    def _xyz_to_rphiz(self, xyz):
        import math
        x = xyz[0]
        y = xyz[1]
        z = xyz[2]
        r = math.sqrt(x * x + y * y)
        phi = math.atan2(y, x)
        rphiz = [r, phi, z]
        return rphiz

    def _rphiz_to_xyz(self, rphiz):
        import math
        z = rphiz[2]
        y = rphiz[0] * math.sin(rphiz[1])
        x = rphiz[0] * math.cos(rphiz[1])
        return [x, y, z]


class RadialExpansion(GlobalDeformation):

    def __init__(self, scale):
        super().__init__(scale)

    def _transform(self, r, phi, z):
        return [self.scale * r, 0., 0.]


class Curl(GlobalDeformation):

    def __init__(self, scale, scale2=0.):
        self.scale2 = scale2
        super().__init__(scale)

    def _transform(self, r, phi, z):
        return [0., self.scale * r + self.scale2, 0.]


class Telescope(GlobalDeformation):

    def __init__(self, scale):
        super().__init__(scale)

    def _transform(self, r, phi, z):
        return [0., 0., self.scale * r]


class Elliptical(GlobalDeformation):

    def __init__(self, scale):
        super().__init__(scale)

    def _transform(self, r, phi, z):
        import math
        return [self.scale * 1. / 2. * math.cos(2 * phi) * r, 0., 0.]


class Clamshell(GlobalDeformation):

    def __init__(self, scale):
        super().__init__(scale)

    def _transform(self, r, phi, z):
        import math
        import numpy

        return [0., self.scale * math.cos(phi), 0.]


class Skew(GlobalDeformation):

    def __init__(self, scale):
        super().__init__(scale)

    def _transform(self, r, phi, z):
        return [0., 0., self.scale * math.cos(phi) * r]


class Bowing(GlobalDeformation):

    def __init__(self, scale):
        super().__init__(scale)

    def _transform(self, r, phi, z):
        return [self.scale * abs(z), 0., 0.]


class Twist(GlobalDeformation):

    def __init__(self, scale):
        super().__init__(scale)

    def _transform(self, r, phi, z):
        return [0., self.scale * z, 0.]


class ZExpansion(GlobalDeformation):

    def __init__(self, scale):
        super().__init__(scale)

    def _transform(self, r, phi, z):
        return [0., 0., self.scale * z]


class CreateMisalignmentModule(Module):

    def __init__(self, global_deformations=None, random_misalignments=None, iov=Belle2.IntervalOfValidity(0, 0, -1, -1)):
        self.global_deformations = global_deformations
        self.random_misalignments = random_misalignments
        self.iov = iov
        super().__init__()

    def initialize(self):
        alignment = Belle2.VXDAlignment()
        txt = open('generated_misalignment.txt', 'w')
        txt.write('layer ladder sensor x_orig y_orig z_orig x y z u v w\n')

        for sensor in Belle2.VXD.GeoCache.getInstance().getListOfSensors():
            info = Belle2.VXD.GeoCache.getInstance().get(sensor)
            global_pos = info.pointToGlobal(ROOT.TVector3(0., 0., 0.))

            delta = ROOT.TVector3(0., 0., 0.)

            if self.global_deformations is not None:
                if not isinstance(self.global_deformations, list):
                    self.global_deformations = [self.global_deformations]

                for deformation in self.global_deformations:
                    delta += deformation(global_pos)

            if sensor == Belle2.VxdID(1, 1, 1):
                delta = ROOT.TVector3(0., 0., 0.)

            new_global_pos = global_pos + delta
            new_local_pos = info.pointToLocal(new_global_pos)

            if self.random_misalignments is not None:
                if not isinstance(self.random_misalignments, list):
                    self.random_misalignments = [self.random_misalignments]

                for random in self.random_misalignments:
                    pass

            for i in range(0, 3):
                if abs(new_local_pos[i]) < 1.e-14:
                    new_local_pos[i] = 0.

                alignment.set(sensor.getID(), i + 1, new_local_pos[i])

            # helper = GlobalDeformation(0.)

            # global_pos = helper._xyz_to_rphiz(global_pos)
            # new_global_pos = helper._xyz_to_rphiz(new_global_pos)

            txt.write(
                '{0} {1} {2} {3} {4} {5} {6} {7} {8} {9} {10} {11}\n'.format(
                    str(sensor.getLayerNumber()),
                    str(sensor.getLadderNumber()),
                    str(sensor.getSensorNumber()),
                    str(global_pos[0]),
                    str(global_pos[1]),
                    str(global_pos[2]),
                    str(new_global_pos[0]),
                    str(new_global_pos[1]),
                    str(new_global_pos[2]),
                    str(new_local_pos[0]),
                    str(new_local_pos[1]),
                    str(new_local_pos[2])
                ))

        txt.close()

        iov = Belle2.IntervalOfValidity(0, 0, -1, -1)
        Belle2.Database.Instance().storeData('VXDAlignment', alignment, self.iov)
        txt.close()
