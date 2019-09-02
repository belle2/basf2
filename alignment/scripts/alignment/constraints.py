'''
Created on 28 May 2018

@author: kleinwrt
'''
from basf2 import *
from ROOT import Belle2

import os


class ConstraintsGenerator(Module):
    def __init__(self, generator):
        """ init """
        super(ConstraintsGenerator, self).__init__()
        self.generator = generator
        self.consts = dict()

    def event(self):
        consts = self.generator.genenerate()
        print(consts)
        for const in consts:
            self.consts[const.get_checksum()] = const.data

    def terminate(self):
        if len(self.consts):
            with open(self.generator.filename, 'w') as f:
                for checksum, data in self.consts.items():
                    f.write('Constraint 0.\n')
                    for label, coef in data:
                        f.write('{} {}\n'.format(label, coef))
                print("Done: ", self.generator.filename)


def gen_constraints(constraint_sets, timedep_config=None, global_tags=None):
    if timedep_config is None:
        timedep_config = []

    if global_tags is None:
        global_tags = [tag for tag in conditions.default_globaltags]

    events = []
    for (labels, events_) in timedep_config:
        events += [event for event in events_]

    if not len(timedep_config):
        events = [(0, 0, 0)]

    events = [(exp, run, ev) for (ev, run, exp) in events]
    events = sorted(list(set(events)))
    events = [(ev_, run_, exp_) for (exp_, run_, ev_) in events]

    fileName = 'TimedepConfigEvent_exp{}run{}ev{}.root'
    files = []

    for index, event in enumerate(events):
        #  conditions.reset()
        #  conditions.override_globaltags(global_tags)

        ev, run, exp = event
        path = create_path()
        path.add_module("EventInfoSetter",
                        skipNEvents=ev,
                        evtNumList=[ev + 1],
                        runList=[run],
                        expList=[exp])
        path.add_module('Progress')
        this_filename = fileName.format(exp, run, ev)
        path.add_module('RootOutput', outputFileName=this_filename, ignoreCommandLineOverride=True)
        files.append(this_filename)
        process(path)
        print(statistics)

    print(files)

    conditions.override_globaltags(global_tags)

    path = create_path()
    path.add_module("RootInput", inputFileNames=files, ignoreCommandLineOverride=True)
    path.add_module('HistoManager')
    path.add_module('Progress')
    path.add_module('Gearbox')
    path.add_module('Geometry')

    collector = path.add_module('MillepedeCollector',
                                timedepConfig=timedep_config)

    constraint_files = []
    for constraint_set in constraint_sets:
        constraint_set.configure_collector(collector)
        constraint_files.append(constraint_set.filename)
        path.add_module(ConstraintsGenerator(constraint_set))

    process(path)
    print(statistics)

    return [os.path.abspath(file) for file in constraint_files]


def cmp(a, b):
    return (a > b) - (a < b)


def cdc_layer_label(layer, param):
    wire = 511
    wireid = Belle2.WireID(layer, wire).getEWire()
    label = Belle2.GlobalLabel()
    label.construct(Belle2.CDCAlignment.getGlobalUniqueID(), wireid, param)
    return label.label()


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

    def genenerate(self):
        consts = []
        return consts

    def configure_collector(self, collector):
        pass


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


class CDCLayerConstraints(Constraints):
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

    def genenerate(self):
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

if __name__ == '__main__':
    consts6 = CDCLayerConstraints('cdc-layer-constraints-6D.txt')
    consts7 = CDCLayerConstraints('cdc-layer-constraints-7D.txt', rigid=True, z_offset=True, r_scale=False, z_scale=False)

    timedep = [([cdc_layer_label(1, ipar) for ipar in [1, 2, 6, 11, 12, 16] for layer in range(0, 56)], [(0, 0, 0), (10, 0, 0)])]

    files = gen_constraints([consts6, consts7, VXDHierarchyConstraints(
        type=1, pxd=False), Constraints("my_file.txt")], timedep_config=timedep)
    print(files)
