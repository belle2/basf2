#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

##############################################################################
#
# This is an example steering file to run the SVD Unpacke.
# It decodes a RawSVD object (from COPPER) and produce a list of SVDDigit
# according to the xml map of the sensors (currently missing)
#
##############################################################################

import basf2 as b2
from ROOT import Belle2


class testSVDShaperDigitOrdering(b2.Module):

    """Module to check SVDShaperDigit ordering."""

    def __init__(self):
        '''initialize'''

        super().__init__()

    def event(self):
        ''' print shaper digits '''

        sds = Belle2.PyStoreArray('SVDShaperDigits')

        print('NEW EVENT!')
        print('layer ladder sensor side strip')

        for sd in sds:
            sensor = sd.getSensorID()
            side = 'V'
            if(sd.isUStrip()):
                side = 'U'

            print(str(sensor.getLayerNumber()) + ' ' + str(sensor.getLadderNumber()) + ' ' +
                  str(sensor.getSensorNumber()) + ' ' + side + ' ' + str(sd.getCellID()))


main = b2.create_path()

'''
# to run on 2017 testbeam data
main.add_module('RootInput', branchNames='RawSVDs')
main.add_module('Gearbox', fileName='testbeam/vxd/2017_geometry_1.xml')
main.add_module('Geometry')
main.add_module('SVDUnpacker', xmlMapFileName='testbeam/vxd/data/2017_svd_mapping.xml', GenerateShaperDigits=True, shutUpFTBError=1)
'''

main.add_module('EventInfoSetter')
main.add_module('EventInfoPrinter')
main.add_module('EvtGenInput')
main.add_module('Gearbox')
main.add_module('Geometry', components=['SVD'])
main.add_module('FullSim')
main.add_module('SVDDigitizer', GenerateShaperDigits=False)
main.add_module('SVDPacker')
main.add_module('SVDUnpacker', GenerateShaperDigits=True)

# TEST MODULE
main.add_module(testSVDShaperDigitOrdering())

# Process events
b2.process(main)

# Print call statistics
print(b2.statistics)
