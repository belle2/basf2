##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

'''
Test if the creation of the geometry payloads works fine.
'''


import basf2 as b2
import b2test_utils as b2tu


# We will test few configurations
# 1: default settings
with b2tu.clean_working_directory():
    main1 = b2.Path()
    main1.add_module('EventInfoSetter')
    main1.add_module('Gearbox')
    main1.add_module('Geometry',
                     createPayloads=True,
                     payloadIov=[0, 1, 2, 3]
                     )
    assert(b2tu.safe_process(main1) == 0)

# 2: include only few components
with b2tu.clean_working_directory():
    main2 = b2.Path()
    main2.add_module('EventInfoSetter')
    main2.add_module('Gearbox')
    main2.add_module('Geometry',
                     createPayloads=True,
                     components=['ARICH', 'ECL', 'PXD'],
                     payloadIov=[0, 1, 2, 3]
                     )
    assert(b2tu.safe_process(main2) == 0)

# 3: exclude few components
with b2tu.clean_working_directory():
    main3 = b2.Path()
    main3.add_module('EventInfoSetter')
    main3.add_module('Gearbox')
    main3.add_module('Geometry',
                     createPayloads=True,
                     excludedComponents=['ARICH', 'ECL', 'PXD'],
                     payloadIov=[0, 1, 2, 3]
                     )
    assert(b2tu.safe_process(main3) == 0)
