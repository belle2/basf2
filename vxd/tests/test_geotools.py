import basf2
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
        """ constructor """
        super().__init__()

    def initialize(self):
        """ Initialize the DQMCommonUtils class here. It builds
        its lookup arrrays, so we want to do it only once."""
        #: Reference to GeoTools class
        self.gTools = Belle2.VXD.GeoCache.getInstance().getGeoTools()
        if not self.gTools:
            basf2.B2FATAL("GeoTools not available!")

    def event(self):
        """ We test sensor and chip numbering by going from ID to
        list number to ID."""
        #
        # 0. General parameters
        #
        # 0a. Layer counts
        if (self.gTools.getNumberOfLayers() != 6):
            basf2.B2ERROR('Layer count failure: \n {0} layers reported, 6 actually.'.format(self.gTools.getNumberOfLayers()))
        if (self.gTools.getNumberOfPXDLayers() != 2):
            basf2.B2ERROR('PXD layer count failure: \n {0} layers reported, 2 actually.'.format(self.gTools.getNumberOfPXDLayers()))
        if (self.gTools.getNumberOfSVDLayers() != 4):
            basf2.B2ERROR('SVD layer count failure: \n {0} layers reported, 4 actually.'.format(self.gTools.getNumberOfSVDLayers()))
        # 0b. Layer numbers
        layers = self.gTools.getLayers()
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

        layers = self.gTools.getPXDLayers()
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

        layers = self.gTools.getSVDLayers()
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
        if (self.gTools.getFirstLayer() != 1):
            basf2.B2ERROR('First layer number is 1, reported {0}.'.format(self.gTools.getFirstLayer()))
        if (self.gTools.getLastLayer() != 6):
            basf2.B2ERROR('Last layer number is 6, reported {0}.'.format(self.gTools.getLastLayer()))
        if (self.gTools.getFirstPXDLayer() != 1):
            basf2.B2ERROR('First PXD layer number is 1, reported {0}.'.format(self.gTools.getFirstPXDLayer()))
        if (self.gTools.getLastPXDLayer() != 2):
            basf2.B2ERROR('Last PXD layer number is 2, reported {0}.'.format(self.gTools.getLastPXDLayer()))
        if (self.gTools.getFirstSVDLayer() != 3):
            basf2.B2ERROR('First SVD layer number is 3, reported {0}.'.format(self.gTools.getFirstSVDLayer()))
        if (self.gTools.getLastSVDLayer() != 6):
            basf2.B2ERROR('Last SVD layer number is 6, reported {0}.'.format(self.gTools.getLastSVDLayer()))
        # 0d. Sensor indexing range
        # PXD
        # First PXD sensor should be (first PXD layer)/1/1,
        idOfFirstPXDSensor = self.gTools.getSensorIDFromPXDIndex(0)
        layer = idOfFirstPXDSensor.getLayerNumber()
        ladder = idOfFirstPXDSensor.getLadderNumber()
        sensor = idOfFirstPXDSensor.getSensorNumber()
        if layer != self.gTools.getFirstPXDLayer() or ladder != 1 or sensor != 1:
            basf2.B2ERROR('Mismatch in first PXD sensor placement:\n' +
                          'Expected {0}/{1}/{2}\nGot: {3|/{4}/{5}'.format(
                              layer, ladder, sensor,
                              self.gTools.getFirstPXDLayer(), 1, 1))
        idOfLastPXDSensor = self.gTools.getSensorIDFromPXDIndex(self.gTools.getNumberOfPXDSensors() - 1)
        layer = idOfLastPXDSensor.getLayerNumber()
        if layer != self.gTools.getLastPXDLayer():
            basf2.B2ERROR('Mismatch in last PXD sensor placement:\n' +
                          'Expected layer {0} got layer {1|'.format(
                              layer, self.gTools.getLastPXDLayer()))
        # SVD
        # First SVD sensor should be (first PXD layer)/1/1,
        idOfFirstSVDSensor = self.gTools.getSensorIDFromSVDIndex(0)
        layer = idOfFirstSVDSensor.getLayerNumber()
        ladder = idOfFirstSVDSensor.getLadderNumber()
        sensor = idOfFirstSVDSensor.getSensorNumber()
        if layer != self.gTools.getFirstSVDLayer() or ladder != 1 or sensor != 1:
            basf2.B2ERROR('Mismatch in first SVD sensor placement:\n' +
                          'Expected {0}/{1}/{2}\nGot: {3}/{4}/{5}'.format(
                              layer, ladder, sensor,
                              self.gTools.getFirstSVDLayer(), 1, 1))
        idOfLastSVDSensor = self.gTools.getSensorIDFromSVDIndex(self.gTools.getNumberOfSVDSensors() - 1)
        layer = idOfLastSVDSensor.getLayerNumber()
        if layer != self.gTools.getLastSVDLayer():
            basf2.B2ERROR('Mismatch in last SVD sensor placement:\n' +
                          'Expected layer {0} got layer {1}'.format(
                              layer, self.gTools.getLastSVDLayer()))
        #
        # 1. General sensor indexing
        #
        # 1a. Basic loop
        init_sensorID = Belle2.VxdID(3, 1, 1)
        init_sensor_index = self.gTools.getSensorIndex(init_sensorID)
        ret_sensorID = self.gTools.getSensorIDFromIndex(init_sensor_index)
        if init_sensorID.getID() != ret_sensorID.getID():
            basf2.B2ERROR(
                "Sensor index failure: \n Initial id: {0} \n VXD index: {1} \n Retrieved id: {2}.".format(
                    init_sensorID, init_sensor_index, ret_sensorID))
        # 1b. Neighbourhood test
        next_sensorID = self.gTools.getSensorIDFromIndex(init_sensor_index + 1)
        next_expected_sensorID = Belle2.VxdID(init_sensorID)
        next_expected_sensorID.setSensorNumber(init_sensorID.getSensorNumber() + 1)
        if next_expected_sensorID.getID() != next_sensorID.getID():
            basf2.B2ERROR('Sensor index neighbourhood test failure: \n' +
                          'Initial id: {0} \n Expected id: {1} \n Actaul id: {2} \n Index: {3}.'.format(
                              init_sensorID, next_expected_sensorID,
                              next_sensorID, init_sensor_index + 1
                          ))

        # 1c. Sensor counting
        num_sensors_expected = 212
        if (self.gTools.getNumberOfSensors() != num_sensors_expected):
            basf2.B2ERROR('Number of sensors mismatch: \n' +
                          'Expected: {0}, got {1}.'.format(
                              num_sensors_expected, self.gTools.getNumberOfSensors()
                          ))
        #
        # 2. PXD sensor indexing
        #
        # 2a. Basic loop
        init_sensorID = Belle2.VxdID(1, 2, 1)
        init_sensor_index = self.gTools.getPXDSensorIndex(
            init_sensorID.getLayerNumber(),
            init_sensorID.getLadderNumber(),
            init_sensorID.getSensorNumber())
        ret_sensorID = self.gTools.getSensorIDFromPXDIndex(init_sensor_index)
        if init_sensorID.getID() != ret_sensorID.getID():
            basf2.B2ERROR(
                "PXD sensor index failure: \n Initial id: {0} \n VXD index: {1} \n Retrieved id: {2}.".format(
                    init_sensorID, init_sensor_index, ret_sensorID))
        # 2b. Neighbourhood test
        next_sensorID = self.gTools.getSensorIDFromPXDIndex(init_sensor_index + 1)
        next_expected_sensorID = Belle2.VxdID(init_sensorID)
        next_expected_sensorID.setSensorNumber(init_sensorID.getSensorNumber() + 1)
        if next_expected_sensorID.getID() != next_sensorID.getID():
            basf2.B2ERROR(
                'PXD sensor index neighbourhood test failure: \n' +
                'Initial id: {0} \n Expected id: {1} \n Actaul id: {2} \n ' +
                'Index: {3}.'.format(init_sensorID, next_expected_sensorID,
                                     next_sensorID, init_sensor_index + 1))
        # 2c. Sensor counting
        num_sensors_expected = 40
        if (self.gTools.getNumberOfPXDSensors() != num_sensors_expected):
            basf2.B2ERROR('Number of PXD sensors mismatch: \n' +
                          'Expected: {0}, got {1}.'.format(
                              num_sensors_expected, self.gTools.getNumberOfPXDSensors()
                          ))
        #
        # 3. SVD sensor indexing
        #
        # 2a. Basic loop
        init_sensorID = Belle2.VxdID(4, 2, 1)
        init_sensor_index = self.gTools.getSVDSensorIndex(
            init_sensorID.getLayerNumber(),
            init_sensorID.getLadderNumber(),
            init_sensorID.getSensorNumber())
        ret_sensorID = self.gTools.getSensorIDFromSVDIndex(init_sensor_index)
        if init_sensorID.getID() != ret_sensorID.getID():
            basf2.B2ERROR(
                "SVD sensor index failure: \n Initial id: {0} \n VXD index: {1} \n Retrieved id: {2}.".format(
                    init_sensorID, init_sensor_index, ret_sensorID))
        # 2b. Neighbourhood test
        next_sensorID = self.gTools.getSensorIDFromSVDIndex(init_sensor_index + 1)
        next_expected_sensorID = Belle2.VxdID(init_sensorID)
        next_expected_sensorID.setSensorNumber(init_sensorID.getSensorNumber() + 1)
        if next_expected_sensorID.getID() != next_sensorID.getID():
            basf2.B2ERROR(
                'SVD sensor index neighbourhood test failure: \n' +
                'Initial id: {0} \n Expected id: {1} \n Actaul id: {2} \n ' +
                'Index: {3}.'.format(init_sensorID, next_expected_sensorID,
                                     next_sensorID, init_sensor_index + 1))
        # 3c. Sensor counting
        num_sensors_expected = 172
        if (self.gTools.getNumberOfSVDSensors() != num_sensors_expected):
            basf2.B2ERROR('Number of SVD sensors mismatch: \n' +
                          'Expected: {0}, got {1}.'.format(
                              num_sensors_expected, self.gTools.getNumberOfSVDSensors()
                          ))
        #
        # 4. Layer indexing
        #
        # 4a. Basic loop
        init_layer = 4
        init_layer_index = self.gTools.getLayerIndex(init_layer)
        ret_layer = self.gTools.getLayerNumberFromLayerIndex(init_layer_index)
        if init_layer != ret_layer:
            basf2.B2ERROR(
                "Layer index failure: \n Initial: {0} \n Index: {1} \n Retrieved: {2}.".format(
                    init_layer, init_layer_index, ret_layer))
        # 2b. Neighbourhood test
        next_layer = self.gTools.getLayerNumberFromLayerIndex(init_layer_index + 1)
        next_expected_layer = init_layer + 1
        if next_layer != next_expected_layer:
            basf2.B2ERROR(
                "Layer index neighbourhood test failure: \n Initial id: {0} \n Expected: {1} \n Actaul: {2} \n Index: {3}.".format(
                    init_layer,
                    next_expected_layer,
                    next_layer,
                    init_layer_index +
                    1))
        #
        # 5. PXD chip indexing
        #
        # 5a. Chip-on-sensor counts
        if self.gTools.getTotalPXDChips() != 400:
            basf2.B2ERROR('PXD chip count mismatch: \n' +
                          'Expected: {0}, got {1}.'.format(
                              2120, self.gTools.getTotalPXDChips())
                          )
        if self.gTools.getNumberOfPXDUSideChips() != 4:
            basf2.B2ERROR('PXD u-side chip count mismatch: \n' +
                          'Expected: {0}, got {1}.'.format(
                              4, self.gTools.getNumberOfPXDUSideChips())
                          )
        if self.gTools.getNumberOfPXDVSideChips() != 6:
            basf2.B2ERROR('PXD v-side chip count mismatch: \n' +
                          'Expected: {0}, got {1}.'.format(
                              6, self.gTools.getNumberOfPXDVSideChips())
                          )
        if self.gTools.getTotalSVDChips() != 1748:
            basf2.B2ERROR('SVD chip count mismatch: \n' +
                          'Expected: {0}, got {1}.'.format(
                              2148, self.gTools.getTotalSVDChips())
                          )
        if self.gTools.getNumberOfSVDUSideChips() != 6:
            basf2.B2ERROR('SVD u-side chip count mismatch: \n' +
                          'Expected: {0}, got {1}.'.format(
                              6, self.gTools.getNumberOfSVDUSideChips())
                          )
        if self.gTools.getNumberOfSVDVSideChips(3) != 6:
            basf2.B2ERROR('SVD v-side chip count mismatch: \n' +
                          'Expected: {0}, got {1}.'.format(
                              6, self.gTools.getNumberOfSVDVSideChips(3))
                          )
        if self.gTools.getNumberOfSVDVSideChips(4) != 4:
            basf2.B2ERROR('SVD v-side chip count mismatch: \n' +
                          'Expected: {0}, got {1}.'.format(
                              4, self.gTools.getNumberOfSVDVSideChips(4))
                          )
        # 5b. PXD chip indexing - loop
        init_layer = 1
        init_ladder = 2
        init_sensor = 1
        init_uSide = True
        init_chipNo = 4
        index_with_vxdid = self.gTools.getPXDChipIndex(Belle2.VxdID(init_layer, init_ladder, init_sensor), init_uSide, init_chipNo)
        index_with_llss = self.gTools.getPXDChipIndex(init_layer, init_ladder, init_sensor, init_uSide, init_chipNo)
        if index_with_vxdid != index_with_llss:
            basf2.B2ERROR('Mismatch between VxdID-based and layer-ladder-sensor based index:\n' +
                          'VxdID-based: {0}\nlayer-ladder-sensor: {1}'.format(
                              index_with_vxdid, index_with_llss)
                          )
        returned_chipID = self.gTools.getChipIDFromPXDIndex(index_with_vxdid)
        returned_layer = returned_chipID.getLayerNumber()
        returned_ladder = returned_chipID.getLadderNumber()
        returned_sensor = returned_chipID.getSensorNumber()
        returned_uSide = self.gTools.isPXDSideU(returned_chipID)
        returned_chipNo = self.gTools.getPXDChipNumber(returned_chipID)
        match = (init_layer == returned_layer) and \
            (init_ladder == returned_ladder) and \
            (init_sensor == returned_sensor) and \
            (init_uSide == returned_uSide) and \
            (init_chipNo == returned_chipNo)
        if not match:
            basf2.B2ERROR('Mismatch in PXD chip indexing:\n' +
                          'Initial: {0} {1} {2} {3} {4}\n'.format(
                              init_layer, init_ladder, init_sensor, init_uSide, init_chipNo) +
                          'Final: {0} {1} {2} {3} {4}.'.format(
                              returned_layer, returned_ladder, returned_sensor, returned_uSide, returned_chipNo)
                          )
        # 5c. PXD chip indexing - neighbourhood
        # We are expecting same sensor, v-side, chip 0
        expected_layer = init_layer
        expected_ladder = init_ladder
        expected_sensor = init_sensor
        expected_uSide = False
        expected_chipNo = 1
        neighbour_chipID = self.gTools.getChipIDFromPXDIndex(index_with_vxdid + 1)
        neighbour_layer = neighbour_chipID.getLayerNumber()
        neighbour_ladder = neighbour_chipID.getLadderNumber()
        neighbour_sensor = neighbour_chipID.getSensorNumber()
        neighbour_uSide = self.gTools.isPXDSideU(neighbour_chipID)
        neighbour_chipNo = self.gTools.getPXDChipNumber(neighbour_chipID)
        match = (expected_layer == neighbour_layer) and \
            (expected_ladder == neighbour_ladder) and \
            (expected_sensor == neighbour_sensor) and \
            (expected_uSide == neighbour_uSide) and \
            (expected_chipNo == neighbour_chipNo)
        if not match:
            basf2.B2ERROR('Mismatch in PXD chip index neighbourship:\n' +
                          'Expected: {0} {1} {2} {3} {4}\n'.format(
                              expected_layer, expected_ladder, expected_sensor, expected_uSide, expected_chipNo) +
                          'Obtained: {0} {1} {2} {3} {4}.'.format(
                              neighbour_layer, neighbour_ladder, neighbour_sensor, neighbour_uSide, neighbour_chipNo)
                          )

        # 5d. SVD chip indexing - loop
        init_layer = 3
        init_ladder = 2
        init_sensor = 1
        init_uSide = True
        init_chipNo = 6
        index_with_vxdid = self.gTools.getSVDChipIndex(Belle2.VxdID(init_layer, init_ladder, init_sensor), init_uSide, init_chipNo)
        index_with_llss = self.gTools.getSVDChipIndex(init_layer, init_ladder, init_sensor, init_uSide, init_chipNo)
        if index_with_vxdid != index_with_llss:
            basf2.B2ERROR('Mismatch between VxdID-based and layer-ladder-sensor based index:\n' +
                          'VxdID-based: {0}\nlayer-ladder-sensor: {1}'.format(
                              index_with_vxdid, index_with_llss)
                          )
        returned_chipID = self.gTools.getChipIDFromSVDIndex(index_with_vxdid)
        returned_layer = returned_chipID.getLayerNumber()
        returned_ladder = returned_chipID.getLadderNumber()
        returned_sensor = returned_chipID.getSensorNumber()
        returned_uSide = self.gTools.isSVDSideU(returned_chipID)
        returned_chipNo = self.gTools.getSVDChipNumber(returned_chipID)
        match = (init_layer == returned_layer) and \
            (init_ladder == returned_ladder) and \
            (init_sensor == returned_sensor) and \
            (init_uSide == returned_uSide) and \
            (init_chipNo == returned_chipNo)
        if not match:
            basf2.B2ERROR('Mismatch in SVD chip indexing:\n' +
                          'Initial: {0} {1} {2} {3} {4}\n'.format(
                              init_layer, init_ladder, init_sensor, init_uSide, init_chipNo) +
                          'Final: {0} {1} {2} {3} {4}.'.format(
                              returned_layer, returned_ladder, returned_sensor, returned_uSide, returned_chipNo)
                          )
        # 5e. SVD chip indexing - neighbourhood
        # We are expecting same sensor, v-side, chip 0
        expected_layer = init_layer
        expected_ladder = init_ladder
        expected_sensor = init_sensor
        expected_uSide = False
        expected_chipNo = 1
        neighbour_chipID = self.gTools.getChipIDFromSVDIndex(index_with_vxdid + 1)
        neighbour_layer = neighbour_chipID.getLayerNumber()
        neighbour_ladder = neighbour_chipID.getLadderNumber()
        neighbour_sensor = neighbour_chipID.getSensorNumber()
        neighbour_uSide = self.gTools.isSVDSideU(neighbour_chipID)
        neighbour_chipNo = self.gTools.getSVDChipNumber(neighbour_chipID)
        match = (expected_layer == neighbour_layer) and \
            (expected_ladder == neighbour_ladder) and \
            (expected_sensor == neighbour_sensor) and \
            (expected_uSide == neighbour_uSide) and \
            (expected_chipNo == neighbour_chipNo)
        if not match:
            basf2.B2ERROR('Mismatch in SVD chip index neighbourship:\n' +
                          'Expected: {0} {1} {2} {3} {4}\n'.format(
                              expected_layer, expected_ladder, expected_sensor, expected_uSide, expected_chipNo) +
                          'Obtained: {0} {1} {2} {3} {4}.'.format(
                              neighbour_layer, neighbour_ladder, neighbour_sensor, neighbour_uSide, neighbour_chipNo)
                          )
        # 5f. Test of PXD chip assignment
        # We want to see that the first and last chips have correct placement.
        indexOfFirstPXDChip = 0
        # First chip must be on first PXD layer, 1st ladder, 1st sensor, u-sie,
        # chip 0.
        idOfFirstPXDChip = self.gTools.getChipIDFromPXDIndex(indexOfFirstPXDChip)
        firstChipLayer = idOfFirstPXDChip.getLayerNumber()
        firstChipLadder = idOfFirstPXDChip.getLadderNumber()
        firstChipSensor = idOfFirstPXDChip.getSensorNumber()
        firstChipUSide = self.gTools.isPXDSideU(idOfFirstPXDChip)
        firstChipNo = self.gTools.getPXDChipNumber(idOfFirstPXDChip)
        if (firstChipLayer != self.gTools.getFirstPXDLayer() or
                firstChipLadder != 1 or
                firstChipSensor != 1 or
                not firstChipUSide or
                firstChipNo != 1):
            basf2.B2ERROR(
                'Wrong 1st PXD chip assignment:\nExpected {0}/{1}/{2} side {3} chip {4},\nGot {5}/{6}/{7} side {8} chip {9}'.format(
                    self.gTools.getFirstPXDLayer(),
                    1,
                    1,
                    'u',
                    1,
                    firstChipLayer,
                    firstChipLadder,
                    firstChipSensor,
                    'u' if firstChipUSide else 'v',
                    firstChipNo))
        indexOfLastPXDChip = self.gTools.getTotalPXDChips() - 1
        # Last chip must be on last PXD layer, v-sie, chip 6.
        idOfLastPXDChip = self.gTools.getChipIDFromPXDIndex(indexOfLastPXDChip)
        lastChipLayer = idOfLastPXDChip.getLayerNumber()
        lastChipUSide = self.gTools.isPXDSideU(idOfLastPXDChip)
        lastChipNo = self.gTools.getPXDChipNumber(idOfLastPXDChip)
        expectedLastChipNo = self.gTools.getNumberOfPXDVSideChips(self.gTools.getLastPXDLayer())
        if (lastChipLayer != self.gTools.getLastPXDLayer() or
                lastChipUSide or
                lastChipNo != expectedLastChipNo):
            basf2.B2ERROR(
                'Wrong last PXD chip assignment:\n' +
                'Expected layer {0} side {1} chip {2},\nGot layer {3} side {4} chip {5}'.format(
                    self.gTools.getLastPXDLayer(),
                    'v',
                    expectedLastChipNo,
                    lastChipLayer,
                    'u' if lastChipUSide else 'v',
                    lastChipNo))
        # 5g. Test of SVD chip assignment
        # We want to see that the first and last chips have correct placement.
        indexOfFirstSVDChip = 0
        # First chip must be on first SVD layer, 1st ladder, 1st sensor, u-sie,
        # chip 0.
        idOfFirstSVDChip = self.gTools.getChipIDFromSVDIndex(indexOfFirstSVDChip)
        firstChipLayer = idOfFirstSVDChip.getLayerNumber()
        firstChipLadder = idOfFirstSVDChip.getLadderNumber()
        firstChipSensor = idOfFirstSVDChip.getSensorNumber()
        firstChipUSide = self.gTools.isSVDSideU(idOfFirstSVDChip)
        firstChipNo = self.gTools.getSVDChipNumber(idOfFirstSVDChip)
        if (firstChipLayer != self.gTools.getFirstSVDLayer() or
                firstChipLadder != 1 or
                firstChipSensor != 1 or
                not firstChipUSide or
                firstChipNo != 1):
            basf2.B2ERROR(
                'Wrong 1st SVD chip assignment:\nExpected {0}/{1}/{2} side {3} chip {4},\nGot {5}/{6}/{7} side {8} chip {9}'.format(
                    self.gTools.getFirstSVDLayer(),
                    1,
                    1,
                    'u',
                    1,
                    firstChipLayer,
                    firstChipLadder,
                    firstChipSensor,
                    'u' if firstChipUSide else 'v',
                    firstChipNo))
        indexOfLastSVDChip = self.gTools.getTotalSVDChips() - 1
        # Last chip must be on last SVD layer, v-sie, chip 6.
        idOfLastSVDChip = self.gTools.getChipIDFromSVDIndex(indexOfLastSVDChip)
        lastChipLayer = idOfLastSVDChip.getLayerNumber()
        lastChipUSide = self.gTools.isSVDSideU(idOfLastSVDChip)
        lastChipNo = self.gTools.getSVDChipNumber(idOfLastSVDChip)
        expectedLastChipNo = self.gTools.getNumberOfSVDVSideChips(self.gTools.getLastSVDLayer())
        if (lastChipLayer != self.gTools.getLastSVDLayer() or
                lastChipUSide or
                lastChipNo != expectedLastChipNo):
            basf2.B2ERROR(
                'Wrong last SVD chip assignment:\n' +
                'Expected layer {0} side {1} chip {2},\nGot layer {3} side {4} chip {5}'.format(
                    self.gTools.getLastSVDLayer(),
                    'v',
                    expectedLastChipNo,
                    lastChipLayer,
                    'u' if lastChipUSide else 'v',
                    lastChipNo))

            # END OF TESTS


if __name__ == "__main__":

    with b2test_utils.clean_working_directory():

        basf2.B2INFO('Setting up a basf2 run...')

        check_numbering = basf2.create_path()
        check_numbering.add_module('EventInfoSetter', expList=[0], runList=[0], evtNumList=[1])
        check_numbering.add_module('Gearbox')
        check_numbering.add_module('Geometry', components=['MagneticField', 'PXD', 'SVD'])
        check_numbering.add_module(CheckNumbering())
        # with b2test_utils.show_only_errors():
        b2test_utils.safe_process(check_numbering)

        basf2.B2INFO('Test complete.')
