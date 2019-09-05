'''
@author: Claus Kleinwort (DESY), Tadeas Bilka
'''
from basf2 import *
from ROOT import Belle2

import os
import pickle
import math


class Constraint():
    def __init__(self, comment="", value=0.):
        self.value = value
        self.comment = comment
        self.data = []

    def add(self, label, coeff):
        entry = (label, coeff)
        self.data.append(entry)

    def get_checksum(self):
        labels_only = [label for (label, coeff) in self.data]
        checksum = hash(str(labels_only))
        return checksum


class Constraints():
    def __init__(self, filename):
        self.filename = filename

    def generate(self):
        consts = []
        return consts

    def configure_collector(self, collector):
        pass


def generate_constraints(constraint_sets, timedep, global_tags):
    files = []
    for filename in [consts.filename for consts in constraint_sets]:
        files.append(os.path.abspath(filename))

    from alignment.constraints_generator import save_config
    ccfn = save_config(constraint_sets, timedep, global_tags)
    os.system('basf2 {} {}'.format(Belle2.FileSystem.findFile('alignment/scripts/alignment/constraints_generator.py'), ccfn))

    return files

# ----------------------------- Sub-detector specific constraint classes -----------------------------------------------


class VXDHierarchyConstraints(Constraints):
    def __init__(self, type=2, pxd=True, svd=True):
        #  TODO: cannot currently change the filename in collector, so fixed here
        super(VXDHierarchyConstraints, self).__init__("constraints.txt")
        self.type = type
        self.pxd = pxd
        self.svd = svd

    def configure_collector(self, collector):
        collector.param('hierarchyType', self.type)
        collector.param('enablePXDHierarchy', self.pxd)
        collector.param('enableSVDHierarchy', self.svd)

    def generate(self):
        print("Generating constraints for VXD (no-op, file created by collector) ...")
        return []


def cdc_layer_label(layer, param):
    wire = 511
    wireid = Belle2.WireID(layer, wire).getEWire()
    label = Belle2.GlobalLabel()
    label.construct(Belle2.CDCAlignment.getGlobalUniqueID(), wireid, param)
    return label.label()


class CDCLayerConstraints(Constraints):
    # Code from Claus Kleinwort
    cdc = [['A1', 8, 160, 16.80, 23.80, 0., -35.9, 67.9],
           ['U2', 6, 160, 25.70, 34.80, 0.068, -51.4, 98.6],
           ['A3', 6, 192, 36.52, 45.57, 0., -57.5, 132.9],
           ['V4', 6, 224, 47.69, 56.69, -0.060, -59.6, 144.7],
           ['A5', 6, 256, 58.41, 67.41, 0., -61.8, 146.8],
           ['U6', 6, 288, 69.53, 78.53, 0.064, -63.9, 148.9],
           ['A7', 6, 320, 80.25, 89.25, 0., -66.0, 151.0],
           ['V8', 6, 352, 91.37, 100.37, -0.072, -68.2, 153.2],
           ['A9', 6, 384, 102.00, 111.14, 0., -70.2, 155.3]]

    parameter = [(1, 'x-offset bwd'), (2, 'y-offset bwd'), (6, 'z-rotation bwd'),
                 (11, 'x-offset fwd-bwd'), (12, 'y-offset fwd-bwd'), (16, 'z-rotation fwd-bwd')]

    def __init__(self, filename='cdc-constraints.txt', rigid=True, z_offset=False, r_scale=False, z_scale=False):
        super(CDCLayerConstraints, self).__init__(filename)
        self.rigid = rigid
        self.z_offset = z_offset
        self.r_scale = r_scale
        self.z_scale = z_scale
        pass

    def generate(self):
        print("Generating constraints for CDC layers...")

        def cmp(a, b):
            return (a > b) - (a < b)

        consts = []

        for par in self.parameter:
            nTot = 0
            for sl in self.cdc:
                nlyr = sl[1]
                for lyr in range(nlyr):
                    nTot += nlyr
        nLayer = nTot

        if self.rigid:
            #  all layers
            for par in self.parameter:
                # f.write("Constraint  0. ! %s \n" % (par[1]))
                const = Constraint()

                nTot = 0
                for sl in self.cdc:
                    nlyr = sl[1]
                    for lyr in range(nlyr):
                        label = cdc_layer_label(nTot + lyr, par[0])
                        # f.write(" %10i  1.0\n" % (label))
                        const.add(label, 1.0)
                    nTot += nlyr

                #  f.write('\n')
                consts.append(const)

        if self.z_offset:
            #  stereo layers (Z offset)
            par = self.parameter[2]
            # f.write("Constraint  0. ! %s (Z offset)\n" % (par[1]))
            const = Constraint()

            nTot = 0
            for sl in self.cdc:
                nlyr = sl[1]
                rInner = sl[3]
                rOuter = sl[4]
                stereo = sl[5]
                if stereo != 0.:
                    dr = (rOuter - rInner) / (nlyr - 1)
                    rWire = rInner
                    for lyr in range(nlyr):
                        label = cdc_layer_label(nTot + lyr, par[0])
                        # f.write(" %10i  %f\n" % (label, stereo * rWire))
                        const.add(label, stereo * rWire)
                        rWire += dr
                nTot += nlyr

            #  f.write('\n')
            consts.append(const)

        if self.r_scale:
            #  all layers 2nd
            for par in self.parameter[:2]:
                # f.write("Constraint  0. ! %s (2nd, radial scale)\n" % (par[1]))
                const = Constraint()

                nTot = 0
                for sl in self.cdc:
                    nlyr = sl[1]
                    for lyr in range(nlyr):
                        label = cdc_layer_label(nTot + lyr, par[0])
                        der = cmp(2.*float(nTot + lyr) + 0.5, float(nLayer))
                        # f.write(" %10i  %3.1f\n" % (label, der))
                        const.add(label, der)
                    nTot += nlyr

                #  f.write('\n')
                consts.append(const)

        if self.z_scale:
            #  stereo layers (Z -scale)
            par = self.parameter[5]
            # f.write("Constraint  0. ! %s (Z scale)\n" % (par[1]))
            const = Constraint()
            nTot = 0
            for sl in self.cdc:
                nlyr = sl[1]
                stereo = sl[5]
                if stereo != 0.:
                    for lyr in range(nlyr):
                        label = cdc_layer_label(nTot + lyr, par[0])
                        # f.write(" %10i  %f\n" % (label, stereo))
                        const.add(label, stereo)
                nTot += nlyr

            #  f.write('\n')
            consts.append(const)

        return consts


class CDCTimeZerosConstraint(Constraints):
    wires_in_layer = [
        160, 160, 160, 160, 160, 160, 160, 160,
        160, 160, 160, 160, 160, 160,
        192, 192, 192, 192, 192, 192,
        224, 224, 224, 224, 224, 224,
        256, 256, 256, 256, 256, 256,
        288, 288, 288, 288, 288, 288,
        320, 320, 320, 320, 320, 320,
        352, 352, 352, 352, 352, 352,
        384, 384, 384, 384, 384, 384]

    def __init__(self, filename='cdc-T0-constraints.txt'):
        super(CDCTimeZerosConstraint, self).__init__(filename)
        pass

    def generate(self):
        print("Generating constraints for CDC T0's ...")
        consts = []
        const = Constraint()

        for layer in range(0, 56):
            for wire in range(0, self.wires_in_layer[layer]):
                label = Belle2.GlobalLabel()
                label.construct(Belle2.CDCTimeZeros.getGlobalUniqueID(), Belle2.WireID(layer, wire).getEWire(), 0)
                const.add(label.label(), 1.0)

        consts.append(const)
        return consts


class CDCWireConstraints(Constraints):
    wires_in_layer = [
        160, 160, 160, 160, 160, 160, 160, 160,
        160, 160, 160, 160, 160, 160,
        192, 192, 192, 192, 192, 192,
        224, 224, 224, 224, 224, 224,
        256, 256, 256, 256, 256, 256,
        288, 288, 288, 288, 288, 288,
        320, 320, 320, 320, 320, 320,
        352, 352, 352, 352, 352, 352,
        384, 384, 384, 384, 384, 384]

    def __init__(self, filename='cdc-wire-constraints.txt'):
        super(CDCWireConstraints, self).__init__(filename)
        pass

    def get_label(self, layer, wire, parameter):
        wireid = Belle2.WireID(layer, wire).getEWire()
        label = Belle2.GlobalLabel()
        label.construct(Belle2.CDCAlignment.getGlobalUniqueID(), wireid, parameter)
        return label.label()

    def generate(self):
        print("Generating constraints for CDC wires...")
        consts = []

        layers = [l for l in range(0, 56)]

        const1 = Constraint()
        for layer in layers:
            # sum of wire X (BWD) in layer
            for wire in range(0, self.wires_in_layer[layer]):
                const1.add(self.get_label(layer, wire, Belle2.CDCAlignment.wireBwdX), 1.)
        consts.append(const1)

        const2 = Constraint()
        for layer in layers:
            # sum of wire Y (BWD) in layer
            for wire in range(0, self.wires_in_layer[layer]):
                const2.add(self.get_label(layer, wire, Belle2.CDCAlignment.wireBwdY), 1.)
        consts.append(const2)

        const3 = Constraint()
        for layer in layers:
            # sum of wire rotations (BWD) in layer
            for wire in range(0, self.wires_in_layer[layer]):

                wirePhi = Belle2.TrackFindingCDC.CDCWire.getInstance(Belle2.WireID(layer, wire)).getBackwardPos3D().phi()

                const3.add(self.get_label(layer, wire, Belle2.CDCAlignment.wireBwdX), -math.sin(wirePhi))
                const3.add(self.get_label(layer, wire, Belle2.CDCAlignment.wireBwdY), +math.cos(wirePhi))
        consts.append(const3)

        const4 = Constraint()
        for layer in layers:
            # sum of wire X (FWD) in layer
            for wire in range(0, self.wires_in_layer[layer]):
                const4.add(self.get_label(layer, wire, Belle2.CDCAlignment.wireFwdX), 1.)
        consts.append(const4)

        const5 = Constraint()
        for layer in layers:
            # sum of wire Y (FWD) in layer
            for wire in range(0, self.wires_in_layer[layer]):
                const5.add(self.get_label(layer, wire, Belle2.CDCAlignment.wireFwdY), 1.)
        consts.append(const5)

        const6 = Constraint()
        for layer in layers:
            # sum of wire rotations (FWD) in layer
            for wire in range(0, self.wires_in_layer[layer]):

                wirePhi = Belle2.TrackFindingCDC.CDCWire.getInstance(Belle2.WireID(layer, wire)).getForwardPos3D().phi()

                const6.add(self.get_label(layer, wire, Belle2.CDCAlignment.wireFwdX), -math.sin(wirePhi))
                const6.add(self.get_label(layer, wire, Belle2.CDCAlignment.wireFwdY), +math.cos(wirePhi))
        consts.append(const6)

        return consts

# ------------ Main: Generate available constraint files with default config (no time-dependence, default global tags) ------

if __name__ == '__main__':
    consts6 = CDCLayerConstraints('cdc-layer-constraints-6D.txt')
    consts7 = CDCLayerConstraints('cdc-layer-constraints-7D.txt', rigid=True, z_offset=True, r_scale=False, z_scale=False)
    consts10 = CDCLayerConstraints('cdc-layer-constraints-10D.txt', rigid=True, z_offset=True, r_scale=True, z_scale=True)
    cdcT0 = CDCTimeZerosConstraint()
    cdcWires = CDCWireConstraints()

    # phase 2
    # timedep = [([], [(0, 0, 1002)])]
    # early phase 3
    # timedep = [([], [(0, 0, 1003)])]

    # final detector (phase 3)
    timedep = [([], [(0, 0, 0)])]

    import alignment.parameters as ap
    print(len(ap.cdc_wires()))

    files = generate_constraints(
      [
        consts6,
        consts7,
        cdcT0,
        cdcWires,
        VXDHierarchyConstraints(type=1, pxd=False),
        Constraints("my_file.txt")],

      timedep=timedep,
      global_tags=None)

    print(files)
