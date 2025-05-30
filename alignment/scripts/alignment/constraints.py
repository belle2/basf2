##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import basf2
from ROOT import Belle2

import os
import math


class Constraint():
    """
    Class representing a linear constraint for global parameters
    """

    def __init__(self, comment="", value=0.):
        """
        Initialize constraint

        comment : str
          TODO: Not yet used
        value : float
          The constant term of the constraint: sum(c_i * par_i) = value
        """
        #: Value
        self.value = value
        #: Comment
        self.comment = comment
        #: Data
        self.data = []

    def add(self, label, coeff):
        """
        Add coefficient for a global parameter

        Parameters
        ----------
        label : int
          global parameter id (GlobalLabel::label())
        coeff : float
          coefficient of the parameter in constraint
        """
        entry = (label, coeff)
        self.data.append(entry)

    def get_checksum(self):
        """Get a checksum to distinguish constraints quickly (computed only from labels)
        """
        labels_only = [label for (label, coeff) in self.data]
        checksum = hash(str(labels_only))
        return checksum


class Constraints():
    """
    Base class representing a "generator" for file with set of constraints

    Can be used directly if you already have a file with constraints.
    """

    def __init__(self, filename):
        """
        filename : str
          Desired filename for the constraint file
        """
        #: File name
        self.filename = filename

    def generate(self):
        """
        Should be overridden by the deriving classes and actually
        fill the dictionary with the constraints (this runs within a basf2 module
        event function - so you have geometry available)
        """
        consts = []
        return consts

    def configure_collector(self, collector):
        """
        Can be overridden be child classes to pass additional configuration to the
        MillepedeCollector (activated by the use of the constraints)
        """
        pass


def generate_constraints(constraint_sets, timedep, global_tags, init_event):
    """
    Run the constraints generation and return file names

    Parameters
    ----------
    constraint_sets : list (alignment.Constraints)
      List of sets of constraints
    timedep_config : list(tuple(list(int), list(tuple(int, int, int))))
        Time-dependence configuration.
        Each list item is 2-tuple with list of parameter numbers (use alignment.parameters to get them) and
        the (event, run, exp) numbers at which values of these parameters can change.
    global_tags : list (str)
        List of global tag names and/or (absolute) file paths to local databases
    init_event : tuple( int, int, int)
        Event (event, run, exp) at which to initialize time-INdependent constraints
    """
    files = []
    for filename in [consts.filename for consts in constraint_sets]:
        files.append(os.path.abspath(filename))

    from alignment.constraints_generator import save_config
    ccfn = save_config(constraint_sets, timedep, global_tags, init_event)
    os.system('basf2 {} {}'.format(Belle2.FileSystem.findFile('alignment/scripts/alignment/constraints_generator.py'), ccfn))

    return files

# ----------------------------- Sub-detector specific constraint classes -----------------------------------------------


class VXDHierarchyConstraints(Constraints):
    """
    Constraints for VXD hierarchy

    They come in 3 types:

    0 : no hierarchy, just sensors aligned in their local system
    1 : sensors -> half-shells (no ladders)
    2 : sensors -> ladders ->half-shells (aka full hierarchy)

    and additionally can use them for PXD, SVD or both (default) (setting pxd/svd to False
    will disable the hierarchy for that sub-detector)

    NOTE: the filename cannot be currently changed, it is hard-coded in the collector module
    as these constraints are actually generated by C++ code and not by python modules as others.
    """

    def __init__(self, type=2, pxd=True, svd=True):
        """ Initialize
        type : int
          Type of constraints (0 - no, 1 - half-shells, 2 -full)
        pxd : bool
          Use constraints for PXD?
        svd : bool
          Use constraints for SVD?
        """

        #  TODO: cannot currently change the filename in collector, so fixed here
        super().__init__("constraints.txt")
        #: Constraint type
        self.type = type
        #: Flag for PXD
        self.pxd = pxd
        #: Flag for SVD
        self.svd = svd

    def configure_collector(self, collector):
        """
        Propagate the hierarchy configuration to the collector
        """
        collector.param('hierarchyType', self.type)
        collector.param('enablePXDHierarchy', self.pxd)
        collector.param('enableSVDHierarchy', self.svd)

    def generate(self):
        """
        Generate the constraints - does nothing. The collector will make the job
        """
        print("Generating constraints for VXD (no-op, file created by collector) ...")
        return []


class CDCLayerConstraints(Constraints):
    """
    Various constraints for CDC layers

    (Code from Claus Kleinwort)

    """
    #: CDC geometry hard-code -> TODO
    cdc = [['A1', 8, 160, 16.80, 23.80, 0., -35.9, 67.9],
           ['U2', 6, 160, 25.70, 34.80, 0.068, -51.4, 98.6],
           ['A3', 6, 192, 36.52, 45.57, 0., -57.5, 132.9],
           ['V4', 6, 224, 47.69, 56.69, -0.060, -59.6, 144.7],
           ['A5', 6, 256, 58.41, 67.41, 0., -61.8, 146.8],
           ['U6', 6, 288, 69.53, 78.53, 0.064, -63.9, 148.9],
           ['A7', 6, 320, 80.25, 89.25, 0., -66.0, 151.0],
           ['V8', 6, 352, 91.37, 100.37, -0.072, -68.2, 153.2],
           ['A9', 6, 384, 102.00, 111.14, 0., -70.2, 155.3]]

    #: layer parameters
    parameter = [(1, 'x-offset bwd'), (2, 'y-offset bwd'), (6, 'z-rotation bwd'),
                 (11, 'x-offset fwd-bwd'), (12, 'y-offset fwd-bwd'), (16, 'z-rotation fwd-bwd')]

    def __init__(self, filename='cdc-constraints.txt', rigid=True, twist=True, z_offset=False, r_scale=False, z_scale=False):
        """
        filename : str
          Filename with constraints (probably generated later)
        rigid : bool
          5D CDC constraints: x+y+rot + dx+dy
        twist : bool
          CDC twist constraint: drot
        z_offset : bool
          Constraint for Z-offset
        r_scale : bool
          Constraint for R-scale
        z_scale : bool
          Constraint for Z-scale
        """
        super().__init__(filename)
        #: 6D CDC constraints
        self.rigid = rigid
        #: CDC twist constraint
        self.twist = twist
        #: Constraint for z-offset
        self.z_offset = z_offset
        #: Constraint for r-scale
        self.r_scale = r_scale
        #: Constraint for z-scale
        self.z_scale = z_scale

    def generate(self):
        """Generate constraints from CDC geometry
        """
        print("Generating constraints for CDC layers...")

        def cdc_layer_label(layer, param):
            wire = 511
            wireid = Belle2.WireID(layer, wire).getEWire()
            label = Belle2.GlobalLabel()
            label.construct(Belle2.CDCAlignment.getGlobalUniqueID(), wireid, param)
            return label.label()

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
                if par[0] == 16:
                    continue
                const = Constraint()

                nTot = 0
                for sl in self.cdc:
                    nlyr = sl[1]
                    for lyr in range(nlyr):
                        label = cdc_layer_label(nTot + lyr, par[0])
                        const.add(label, 1.0)
                    nTot += nlyr

                consts.append(const)

        if self.twist:
            #  all layers
            for par in self.parameter:
                if par[0] != 16:
                    continue
                const = Constraint()

                nTot = 0
                for sl in self.cdc:
                    nlyr = sl[1]
                    for lyr in range(nlyr):
                        label = cdc_layer_label(nTot + lyr, par[0])
                        const.add(label, 1.0)
                    nTot += nlyr

                #  f.write('\n')
                consts.append(const)

        if self.z_offset:
            #  stereo layers (Z offset)
            par = self.parameter[2]
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
                        const.add(label, stereo * rWire)
                        rWire += dr
                nTot += nlyr

            #  f.write('\n')
            consts.append(const)

        if self.r_scale:
            #  all layers 2nd
            for par in self.parameter[:2]:
                const = Constraint()

                nTot = 0
                for sl in self.cdc:
                    nlyr = sl[1]
                    for lyr in range(nlyr):
                        label = cdc_layer_label(nTot + lyr, par[0])
                        der = cmp(2. * float(nTot + lyr) + 0.5, float(nLayer))
                        const.add(label, der)
                    nTot += nlyr

                consts.append(const)

        if self.z_scale:
            #  stereo layers (Z -scale)
            par = self.parameter[5]
            const = Constraint()
            nTot = 0
            for sl in self.cdc:
                nlyr = sl[1]
                stereo = sl[5]
                if stereo != 0.:
                    for lyr in range(nlyr):
                        label = cdc_layer_label(nTot + lyr, par[0])
                        const.add(label, stereo)
                nTot += nlyr

            #  f.write('\n')
            consts.append(const)

        return consts


class CDCTimeZerosConstraint(Constraints):
    """
    Constraint to fix sum of corrections to CDC T0's (per wire) to zero
    """

    #: CDC layer/wire configuration
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
        """ Initialize
        filename : str
          Can use different filename
        """
        super().__init__(filename)
        pass

    def generate(self):
        """
        Generate the constraints
        """
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
    """
    Various constraints for CDC wires (w.r.t. layers)
    """

    #: CDC layer/wire configuration
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

    def __init__(
            self,
            filename='cdc-wire-constraints.txt',
            layers=None,
            layer_rigid=True,
            layer_radius=None,
            cdc_radius=False,
            hemisphere=None):
        """Initialize constraint

        Parameters
        ----------
        filename : str
          Can override the default file name
        layers : list (int)
          List of layer numbers for which to generate the constraints (default is 0..55)
        layer_rigid : bool
          6 constraints - fix sum of shifts of wire ends in x/y/rotation at fwd/bwd end-plate
        layer_radius : list(int)
          2 constraints to fix average change of layer radius (wires in layer moving away from origin) for all layers in list
        cdc_radius : bool
          1 constraint - fix average change in CDC radius from all wires
        hemisphere : list(int)
          Modifies rigid and layer_radius (layer_radius constraint is added if not present for selected layer(s))
          constraint by splitting the constraints for a given list of layers to L/R up/down half of CDC

        """

        super().__init__(filename)
        #: List of layers for whose wires to generate the constraints. None = all layers
        if layers is None:
            layers = list(range(0, 56))
        #: set list of layers
        self.layers = layers
        #: 6 x 56 (6/layer) constraints. Sum(dX_B/dY_B/drot_B/dX_FB/dY_FB/drot_FB)=0 for all wires in each layer
        #  -> removes the basic unconstrained DoF when aligning wires and layers simultaneously.
        #  Average shift and rotation of wires in layer (at each end-plate) = 0. ->
        #  coherent movements of wires absorbed in layer parameters
        self.layer_rigid = layer_rigid
        #: 2 x 56 constraints: Sum(dr)=0 for all wires in each layer at each end-plate -> layer radius kept
        #  same by this constraint (1 per layer)
        if layer_radius is None:
            layer_radius = []
        #: set list of layer radii
        self.layer_radius = layer_radius
        #: 2 Constraints: Sum(dr)=0 for all wires in CDC at each end-plate -> "average CDC radius" kept same
        #  by this constraint (1 per CDC)
        self.cdc_radius = cdc_radius
        #: list of layer subject to hemisphere constraints
        if hemisphere is None:
            hemisphere = []
        #: set list of layers
        self.hemisphere = hemisphere

    def configure_collector(self, collector):
        """Enables wire-by-wire derivatives in collector
        """
        basf2.B2WARNING("Adding CDC wire constraints -> enabling wire-by-wire alignment derivatives")
        collector.param('enableWireByWireAlignment', True)

    def get_label(self, layer, wire, parameter):
        """Return GlobalLabel for wire parameter"""

        wireid = Belle2.WireID(layer, wire).getEWire()
        label = Belle2.GlobalLabel()
        label.construct(Belle2.CDCAlignment.getGlobalUniqueID(), wireid, parameter)
        return label.label()

    def generate(self):
        """Generate constraints """
        print("Generating constraints for CDC wires...")
        consts = []

        layers = self.layers

        if self.layer_rigid:
            for layer in layers:
                const = Constraint()
                # sum of wire X (BWD) in layer
                for wire in range(0, self.wires_in_layer[layer]):
                    const.add(self.get_label(layer, wire, Belle2.CDCAlignment.wireBwdX), 1.)
                consts.append(const)

            for layer in layers:
                const = Constraint()
                # sum of wire Y (BWD) in layer
                for wire in range(0, self.wires_in_layer[layer]):
                    const.add(self.get_label(layer, wire, Belle2.CDCAlignment.wireBwdY), 1.)
                consts.append(const)

            for layer in set(layers) - set(self.hemisphere):
                const = Constraint()
                # sum of wire rotations (BWD) in layer
                for wire in range(0, self.wires_in_layer[layer]):

                    wirePhi = Belle2.TrackFindingCDC.CDCWire.getInstance(Belle2.WireID(layer, wire)).getBackwardPos3D().phi()

                    const.add(self.get_label(layer, wire, Belle2.CDCAlignment.wireBwdX), -math.sin(wirePhi))
                    const.add(self.get_label(layer, wire, Belle2.CDCAlignment.wireBwdY), +math.cos(wirePhi))
                consts.append(const)

            for layer in layers:
                const = Constraint()
                # sum of wire X (FWD) in layer
                for wire in range(0, self.wires_in_layer[layer]):
                    const.add(self.get_label(layer, wire, Belle2.CDCAlignment.wireFwdX), 1.)
                consts.append(const)

            for layer in layers:
                const = Constraint()
                # sum of wire Y (FWD) in layer
                for wire in range(0, self.wires_in_layer[layer]):
                    const.add(self.get_label(layer, wire, Belle2.CDCAlignment.wireFwdY), 1.)
                consts.append(const)

            for layer in set(layers) - set(self.hemisphere):
                const = Constraint()
                # sum of wire rotations (FWD) in layer
                for wire in range(0, self.wires_in_layer[layer]):

                    wirePhi = Belle2.TrackFindingCDC.CDCWire.getInstance(Belle2.WireID(layer, wire)).getForwardPos3D().phi()

                    const.add(self.get_label(layer, wire, Belle2.CDCAlignment.wireFwdX), -math.sin(wirePhi))
                    const.add(self.get_label(layer, wire, Belle2.CDCAlignment.wireFwdY), +math.cos(wirePhi))
                consts.append(const)

        for layer in self.hemisphere:
            const = Constraint()
            # sum of wire rotations (BWD) in layer ... RIGHT hemisphere
            for wire in range(0, self.wires_in_layer[layer]):

                wirePhi = Belle2.TrackFindingCDC.CDCWire.getInstance(Belle2.WireID(layer, wire)).getBackwardPos3D().phi()

                if math.cos(wirePhi) <= 0.:
                    continue

                const.add(self.get_label(layer, wire, Belle2.CDCAlignment.wireBwdX), -math.sin(wirePhi))
                const.add(self.get_label(layer, wire, Belle2.CDCAlignment.wireBwdY), +math.cos(wirePhi))
            consts.append(const)

        for layer in self.hemisphere:
            const = Constraint()
            # sum of wire rotations (BWD) in layer ... LEFT hemisphere
            for wire in range(0, self.wires_in_layer[layer]):

                wirePhi = Belle2.TrackFindingCDC.CDCWire.getInstance(Belle2.WireID(layer, wire)).getBackwardPos3D().phi()

                if math.cos(wirePhi) > 0.:
                    continue

                const.add(self.get_label(layer, wire, Belle2.CDCAlignment.wireBwdX), -math.sin(wirePhi))
                const.add(self.get_label(layer, wire, Belle2.CDCAlignment.wireBwdY), +math.cos(wirePhi))
            consts.append(const)

        for layer in self.hemisphere:
            const = Constraint()
            # sum of wire rotations (FWD) in layer ... RIGHT
            for wire in range(0, self.wires_in_layer[layer]):

                wirePhi = Belle2.TrackFindingCDC.CDCWire.getInstance(Belle2.WireID(layer, wire)).getForwardPos3D().phi()

                if math.cos(wirePhi) <= 0.:
                    continue

                const.add(self.get_label(layer, wire, Belle2.CDCAlignment.wireFwdX), -math.sin(wirePhi))
                const.add(self.get_label(layer, wire, Belle2.CDCAlignment.wireFwdY), +math.cos(wirePhi))
            consts.append(const)

        for layer in self.hemisphere:
            const = Constraint()
            # sum of wire rotations (FWD) in layer ... LEFT
            for wire in range(0, self.wires_in_layer[layer]):

                wirePhi = Belle2.TrackFindingCDC.CDCWire.getInstance(Belle2.WireID(layer, wire)).getForwardPos3D().phi()

                if math.cos(wirePhi) > 0.:
                    continue

                const.add(self.get_label(layer, wire, Belle2.CDCAlignment.wireFwdX), -math.sin(wirePhi))
                const.add(self.get_label(layer, wire, Belle2.CDCAlignment.wireFwdY), +math.cos(wirePhi))
            consts.append(const)

        # layer_radius:
        for layer in [lyr for lyr in self.layer_radius if lyr not in self.hemisphere]:
            const = Constraint()
            # sum of wire rotations (BWD) in layer
            for wire in range(0, self.wires_in_layer[layer]):

                wirePhi = Belle2.TrackFindingCDC.CDCWire.getInstance(Belle2.WireID(layer, wire)).getBackwardPos3D().phi()

                const.add(self.get_label(layer, wire, Belle2.CDCAlignment.wireBwdX), +math.cos(wirePhi))
                const.add(self.get_label(layer, wire, Belle2.CDCAlignment.wireBwdY), +math.sin(wirePhi))
            const.add(self.get_label(layer, 0, 0), 0.)
            consts.append(const)

        for layer in [lyr for lyr in self.layer_radius if lyr not in self.hemisphere]:
            const = Constraint()
            # sum of wire rotations (FWD) in layer
            for wire in range(0, self.wires_in_layer[layer]):

                wirePhi = Belle2.TrackFindingCDC.CDCWire.getInstance(Belle2.WireID(layer, wire)).getForwardPos3D().phi()

                const.add(self.get_label(layer, wire, Belle2.CDCAlignment.wireFwdX), +math.cos(wirePhi))
                const.add(self.get_label(layer, wire, Belle2.CDCAlignment.wireFwdY), +math.sin(wirePhi))
            const.add(self.get_label(layer, 0, 0), 0.)
            consts.append(const)

        # hemisphere:
        for layer in self.hemisphere:
            const = Constraint()
            # sum of wire rotations (BWD) in layer
            for wire in range(0, self.wires_in_layer[layer]):

                wirePhi = Belle2.TrackFindingCDC.CDCWire.getInstance(Belle2.WireID(layer, wire)).getBackwardPos3D().phi()

                if math.sin(wirePhi) >= 0:
                    continue

                const.add(self.get_label(layer, wire, Belle2.CDCAlignment.wireBwdX), +math.cos(wirePhi))
                const.add(self.get_label(layer, wire, Belle2.CDCAlignment.wireBwdY), +math.sin(wirePhi))
            const.add(self.get_label(layer, 0, 0), 0.)
            consts.append(const)

        for layer in self.hemisphere:
            const = Constraint()
            # sum of wire rotations (BWD) in layer
            for wire in range(0, self.wires_in_layer[layer]):

                wirePhi = Belle2.TrackFindingCDC.CDCWire.getInstance(Belle2.WireID(layer, wire)).getBackwardPos3D().phi()

                if math.sin(wirePhi) < 0:
                    continue

                const.add(self.get_label(layer, wire, Belle2.CDCAlignment.wireBwdX), +math.cos(wirePhi))
                const.add(self.get_label(layer, wire, Belle2.CDCAlignment.wireBwdY), +math.sin(wirePhi))
            const.add(self.get_label(layer, 0, 0), 0.)
            consts.append(const)

        for layer in self.hemisphere:
            const = Constraint()
            # sum of wire rotations (FWD) in layer
            for wire in range(0, self.wires_in_layer[layer]):

                wirePhi = Belle2.TrackFindingCDC.CDCWire.getInstance(Belle2.WireID(layer, wire)).getForwardPos3D().phi()

                if math.sin(wirePhi) >= 0:
                    continue

                const.add(self.get_label(layer, wire, Belle2.CDCAlignment.wireFwdX), +math.cos(wirePhi))
                const.add(self.get_label(layer, wire, Belle2.CDCAlignment.wireFwdY), +math.sin(wirePhi))
            const.add(self.get_label(layer, 0, 0), 0.)
            consts.append(const)

        for layer in self.hemisphere:
            const = Constraint()
            # sum of wire rotations (FWD) in layer
            for wire in range(0, self.wires_in_layer[layer]):

                wirePhi = Belle2.TrackFindingCDC.CDCWire.getInstance(Belle2.WireID(layer, wire)).getForwardPos3D().phi()

                if math.sin(wirePhi) < 0:
                    continue

                const.add(self.get_label(layer, wire, Belle2.CDCAlignment.wireFwdX), +math.cos(wirePhi))
                const.add(self.get_label(layer, wire, Belle2.CDCAlignment.wireFwdY), +math.sin(wirePhi))
            const.add(self.get_label(layer, 0, 0), 0.)
            consts.append(const)

        if self.cdc_radius:
            const = Constraint()
            for layer in layers:
                # sum of wire rotations (BWD) in layer
                for wire in range(0, self.wires_in_layer[layer]):
                    wirePhi = Belle2.TrackFindingCDC.CDCWire.getInstance(Belle2.WireID(layer, wire)).getBackwardPos3D().phi()
                    const.add(self.get_label(layer, wire, Belle2.CDCAlignment.wireBwdX), +math.cos(wirePhi))
                    const.add(self.get_label(layer, wire, Belle2.CDCAlignment.wireBwdY), +math.sin(wirePhi))
            consts.append(const)

            const = Constraint()
            for layer in layers:
                # sum of wire rotations (FWD) in layer
                for wire in range(0, self.wires_in_layer[layer]):
                    wirePhi = Belle2.TrackFindingCDC.CDCWire.getInstance(Belle2.WireID(layer, wire)).getForwardPos3D().phi()
                    const.add(self.get_label(layer, wire, Belle2.CDCAlignment.wireFwdX), +math.cos(wirePhi))
                    const.add(self.get_label(layer, wire, Belle2.CDCAlignment.wireFwdY), +math.sin(wirePhi))
            consts.append(const)

        return consts


# ------------ Main: Generate some constraint files with default config (no time-dependence, default global tags) ------
if __name__ == '__main__':
    #: 6D CDC constraints
    consts6 = CDCLayerConstraints('cdc-layer-constraints-6D.txt', rigid=True, z_offset=False, r_scale=False, z_scale=False)
    #: 7D CDC constraints
    consts7 = CDCLayerConstraints('cdc-layer-constraints-7D.txt', rigid=True, z_offset=True, r_scale=False, z_scale=False)
    #: 10D CD constraints
    consts10 = CDCLayerConstraints('cdc-layer-constraints-10D.txt', rigid=True, z_offset=True, r_scale=True, z_scale=True)
    #: CDC EventT0 constraint
    cdcT0 = CDCTimeZerosConstraint()
    #: CDC wire constraints
    cdcWires = CDCWireConstraints(
        filename='cdc-wire-constraints-proc12.txt',
        layers=None,
        layer_rigid=True,
        layer_radius=[53],
        cdc_radius=True,
        hemisphere=[55])

    # phase 2
    # timedep = [([], [(0, 0, 1002)])]
    # early phase 3
    # timedep = [([], [(0, 0, 1003)])]

    # final detector (phase 3)
    #: Time dependency
    timedep = []  # [([], [(0, 0, 0)])]
    #: Initial event
    init_event = (0, 0, 0)
    #: Files
    files = generate_constraints(
        [
            consts6,
            consts7,
            consts10,
            cdcT0,
            cdcWires,
            VXDHierarchyConstraints(type=1, pxd=False),
            Constraints("my_file.txt")],

        timedep=timedep,
        global_tags=None,
        init_event=init_event)
