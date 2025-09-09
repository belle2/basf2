import basf2
import ROOT  # noqa
from ROOT.Belle2 import PyStoreArray, DataStore

import generators as ge
import simulation as si


class PXDReader(basf2.Module):
    '''
    Simple module that reads the content of the PyStoreArray
    and prints a dictionary with the read values
    '''

    def initialize(self):
        '''
        Initialize the PyStoreArray and regiter it into the DataStore
        '''
        #: PyStoreArray
        self.pxddigits = PyStoreArray("PXDDigits", DataStore.c_Event)
        self.pxddigits.registerInDataStore("PXDDigits")

    def event(self):
        '''
        Loop over the number of events and read the content of the
        PyStoreArray into a dictionary
        '''
        digits = self.pxddigits.readArray()
        print(digits)


main = basf2.Path()
main.add_module('EventInfoSetter', evtNumList=[1], expList=[0])
ge.add_evtgen_generator(path=main, finalstate='mixed')
si.add_simulation(path=main)
main.add_module(PXDReader())

basf2.process(main)
basf2.statistics
