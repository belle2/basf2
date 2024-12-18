##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################
'''
Check if the database correctly returns the names of all the payloads used by the basf2 process.
'''

import basf2 as b2
import b2test_utils as b2tu


class CheckPayloadNames(b2.Module):
    '''
    Small module that asks for some payloads and checks their names.
    '''

    def __init__(self):
        '''Constructor.'''
        from ROOT import Belle2 as B2
        super().__init__()
        #: Payload names to be required
        self.payload_names = [
            'BeamParameters',
            'BeamSpot',
            'BunchStructure',
            'CollisionBoostVector',
            'HardwareClockSettings',
            'MagneticField']
        self.payload_names.sort()
        for payload_name in self.payload_names:
            B2.PyDBObj(payload_name)

    def event(self):
        '''Event.'''
        from ROOT import Belle2 as B2  # this is only run once
        payload_names = sorted(B2.Database.Instance().getAllDataNames())
        print('Set payloads:', self.payload_names)
        print('Got payloads:', payload_names)
        assert (self.payload_names == payload_names)


if __name__ == '__main__':

    with b2tu.clean_working_directory():
        main = b2.Path()
        main.add_module('EventInfoSetter', expList=[0], runList=[0], evtNumList=[1])
        main.add_module(CheckPayloadNames())
        b2.process(main)
