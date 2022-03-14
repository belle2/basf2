##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

'''
When the database is initialized, the globaltag names are trimmed in order to remove
leading and trailing whitespaces, which would make basf2 crashing.

This test checks if the trim is correctly applied.
'''

import basf2 as b2


tag = b2.conditions.default_globaltags[0]
tags = [f'  {tag}', f'{tag} ', f' {tag}  ']
b2.conditions.override_globaltags(tags)

main = b2.Path()
main.add_module('EventInfoSetter')
main.add_module('PrintBeamParameters')  # this module "touches" the database
b2.process(main)  # if the process does not crash, we are happy
