import basf2
import ROOT
from ROOT import Belle2
import b2test_utils

# ====================================================================
# This test checks the numberingg schemes used in VXD DQM utility class.
# It builds a single event to have geometry in place, and only checks
# that ID->list index->ID works for sensor and chip lists.
# ====================================================================


class CheckNumbering(basf2.Module):
    """This module creates a DQMCommonUtils object and tests sensor
    and chip numbering schemes."""

    def __init__(self):
        super().__init__()

    def initialize(self):
        """ Initialize the DQMCommonUtils class here. It builds
        its lookup arrrays, so we want to do it only once."""

        self.DCU = Belle2.DQMCommonUtils()

    def event(self):
        """ We test sensor and chip numbering by going from ID to
        list number to ID."""
        #
        # 0. General parameters
        #
        # 0a. Layer counts
        if (self.DCU.getNumberOfLayers() != 6):
            basf2.B2ERROR('Layer count failure: \n {0} layers reported, 6 actually.'.format(self.DCU.getNumberOfLayers()))
        if (self.DCU.getNumberOfPXDLayers() != 2):
            basf2.B2ERROR('PXD layer count failure: \n {0} layers reported, 2 actually.'.format(self.DCU.getNumberOfPXDLayers()))
        if (self.DCU.getNumberOfSVDLayers() != 4):
            basf2.B2ERROR('SVD layer count failure: \n {0} layers reported, 4 actually.'.format(self.DCU.getNumberOfSVDLayers()))
        # 0b. Layer numbers
        layers = self.DCU.getLayers()
        expected_layers = [1, 2, 3, 4, 5, 6]
        match = True
        for l1, l2 in zip(layers, expected_layers):
            if l1 != l2:
                match = False
                break
        if not match:
            l1_string = ' '.join([str(x) for x in layers])
            l2_string = ' '.join([str(x) for x in expected_layers])
            basf2.B2ERROR('Layer numbers do not match, expected {0}, got {1}!'.format(l2_string, l1_string))

        layers = self.DCU.getPXDLayers()
        expected_layers = [1, 2]
        match = True
        for l1, l2 in zip(layers, expected_layers):
            if l1 != l2:
                match = False
                break
        if not match:
            l1_string = ' '.join([str(x) for x in layers])
            l2_string = ' '.join([str(x) for x in expected_layers])
            basf2.B2ERROR('PXD layer numbers do not match, expected {0}, got {1}!'.format(l2_string, l1_string))

        layers = self.DCU.getSVDLayers()
        expected_layers = [3, 4, 5, 6]
        match = True
        for l1, l2 in zip(layers, expected_layers):
            if l1 != l2:
                match = False
                break
        if not match:
            l1_string = ' '.join([str(x) for x in layers])
            l2_string = ' '.join([str(x) for x in expected_layers])
            basf2.B2ERROR('SVD layer numbers do not match, expected {0}, got {1}!'.format(l2_string, l1_string))

        # 0c. Layer ranges
        if (self.DCU.getFirstLayer() != 1):
            basf2.B2ERROR('First layer number is 1, reported {0}.'.format(self.DCU.getFirstLayer()))
        if (self.DCU.getLastLayer() != 6):
            basf2.B2ERROR('Last layer number is 6, reported {0}.'.format(self.DCU.getLastLayer()))
        if (self.DCU.getFirstPXDLayer() != 1):
            basf2.B2ERROR('First PXD layer number is 1, reported {0}.'.format(self.DCU.getFirstPXDLayer()))
        if (self.DCU.getLastPXDLayer() != 2):
            basf2.B2ERROR('Last PXD layer number is 2, reported {0}.'.format(self.DCU.getLastPXDLayer()))
        if (self.DCU.getFirstSVDLayer() != 3):
            basf2.B2ERROR('First SVD layer number is 3, reported {0}.'.format(self.DCU.getFirstSVDLayer()))
        if (self.DCU.getLastSVDLayer() != 6):
            basf2.B2ERROR('Last SVD layer number is 6, reported {0}.'.format(self.DCU.getLastSVDLayer()))
        # 0d. Sensor numbers
        #
        # 1. General sensor indexing
        #
        # 1a. Basic loop
        init_sensorID = Belle2.VxdID(3, 1, 1)
        init_sensor_index = self.DCU.getSensorIndex(init_sensorID)
        ret_sensorID = self.DCU.getSensorIDFromIndex(init_sensor_index)
        if init_sensorID.getID() != ret_sensorID.getID():
            basf2.B2ERROR(
                "Sensor index failure: \n Initial id: {0} \n VXD index: {1} \n Retrieved id: {2}.".format(
                    init_sensorID, init_sensor_index, ret_sensorID))
        # 1b. Neighbourhood test
        next_sensorID = self.DCU.getSensorIDFromIndex(init_sensor_index + 1)
        next_expected_sensorID = Belle2.VxdID(init_sensorID)
        next_expected_sensorID.setSensorNumber(init_sensorID.getSensorNumber() + 1)
        if next_expected_sensorID.getID() != next_sensorID.getID():
            basf2.B2ERROR('Sensor index neighbourhood test failure: \n' +
                          'Initial id: {0} \n Expected id: {1} \n Actaul id: {2} \n' +
                          'Index: {3}.'.format(init_sensorID, next_expected_sensorID,
                                               next_sensorID, init_sensor_index + 1))
        #
        # 2. PXD sensor indexing
        #
        # 2a. Basic loop
        init_sensorID = Belle2.VxdID(1, 2, 1)
        init_sensor_index = self.DCU.getPXDSensorIndex(
            init_sensorID.getLayerNumber(),
            init_sensorID.getLadderNumber(),
            init_sensorID.getSensorNumber())
        ret_sensorID = self.DCU.getSensorIDFromPXDIndex(init_sensor_index)
        if init_sensorID.getID() != ret_sensorID.getID():
            basf2.B2ERROR(
                "PXD sensor index failure: \n Initial id: {0} \n VXD index: {1} \n Retrieved id: {2}.".format(
                    init_sensorID, init_sensor_index, ret_sensorID))
        # 2b. Neighbourhood test
        next_sensorID = self.DCU.getSensorIDFromPXDIndex(init_sensor_index + 1)
        next_expected_sensorID = Belle2.VxdID(init_sensorID)
        next_expected_sensorID.setSensorNumber(init_sensorID.getSensorNumber() + 1)
        if next_expected_sensorID.getID() != next_sensorID.getID():
            basf2.B2ERROR(
                'PXD sensor index neighbourhood test failure: \n' +
                'Initial id: {0} \n Expected id: {1} \n Actaul id: {2} \n ' +
                'Index: {3}.'.format(init_sensorID, next_expected_sensorID,
                                     next_sensorID, init_sensor_index + 1))
        #
        # 3. SVD sensor indexing
        #
        # 2a. Basic loop
        init_sensorID = Belle2.VxdID(4, 2, 1)
        init_sensor_index = self.DCU.getSVDSensorIndex(
            init_sensorID.getLayerNumber(),
            init_sensorID.getLadderNumber(),
            init_sensorID.getSensorNumber())
        ret_sensorID = self.DCU.getSensorIDFromSVDIndex(init_sensor_index)
        if init_sensorID.getID() != ret_sensorID.getID():
            basf2.B2ERROR(
                "SVD sensor index failure: \n Initial id: {0} \n VXD index: {1} \n Retrieved id: {2}.".format(
                    init_sensorID, init_sensor_index, ret_sensorID))
        # 2b. Neighbourhood test
        next_sensorID = self.DCU.getSensorIDFromSVDIndex(init_sensor_index + 1)
        next_expected_sensorID = Belle2.VxdID(init_sensorID)
        next_expected_sensorID.setSensorNumber(init_sensorID.getSensorNumber() + 1)
        if next_expected_sensorID.getID() != next_sensorID.getID():
            basf2.B2ERROR(
                'SVD sensor index neighbourhood test failure: \n' +
                'Initial id: {0} \n Expected id: {1} \n Actaul id: {2} \n ' +
                'Index: {3}.'.format(init_sensorID, next_expected_sensorID,
                                     next_sensorID, init_sensor_index + 1))
        #
        # 4. Layer indexing
        #
        # 4a. Basic loop
        init_layer = 4
        init_layer_index = self.DCU.getLayerIndex(init_layer)
        ret_layer = self.DCU.getLayerNumberFromLayerIndex(init_layer_index)
        if init_layer != ret_layer:
            basf2.B2ERROR(
                "Layer index failure: \n Initial: {0} \n Index: {1} \n Retrieved: {2}.".format(
                    init_layer, init_layer_index, ret_layer))
        # 2b. Neighbourhood test
        next_layer = self.DCU.getLayerNumberFromLayerIndex(init_layer_index + 1)
        next_expected_layer = init_layer + 1
        if next_layer != next_expected_layer:
            basf2.B2ERROR(
                "Layer index neighbourhood test failure: \n Initial id: {0} \n Expected: {1} \n Actaul: {2} \n Index: {3}.".format(
                    init_layer,
                    next_expected_layer,
                    next_layer,
                    init_layer_index +
                    1))


if __name__ == "__main__":

    with b2test_utils.clean_working_directory():

        basf2.B2INFO('Setting up a basf2 run...')

        check_numbering = basf2.create_path()
        check_numbering.add_module('EventInfoSetter', expList=[0], runList=[0], evtNumList=[1])
        check_numbering.add_module('Gearbox')
        check_numbering.add_module('Geometry', components=['MagneticField', 'PXD', 'SVD'])
        check_numbering.add_module(CheckNumbering())
        with b2test_utils.show_only_errors():
            b2test_utils.safe_process(check_numbering)

        basf2.B2INFO('Test complete.')
