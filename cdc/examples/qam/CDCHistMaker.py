#!/usr/bin/env python3
# -*- coding: utf-8 -*-
'''
Example script storing ADC, TDC and Hit distribution histograms.
Usage :
basf2 CDCHistMaker exp run
'''
from basf2 import *
from ROOT import Belle2
from ROOT import TH1D, TH2D, TCanvas, TFile
import argparse
import glob


reset_database()
use_database_chain()
use_central_database("Calibration_Offline_Development", LogLevel.INFO)


nWires = [160, 160, 160, 160, 160, 160, 160, 160,  # SL0
          160, 160, 160, 160, 160, 160,  # SL1
          192, 192, 192, 192, 192, 192,  # SL2
          224, 224, 224, 224, 224, 224,  # SL3
          256, 256, 256, 256, 256, 256,  # SL4
          288, 288, 288, 288, 288, 288,  # SL5
          320, 320, 320, 320, 320, 320,  # SL6
          352, 352, 352, 352, 352, 352,  # SL7
          384, 384, 384, 384, 384, 384   # SL8
          ]

nWiresSum = []
sum = 0
for w in nWires:
    sum = sum + w
    nWiresSum.append(sum)


def getHistID(lay, wire):
    '''
    Get histogram ID from (layer ID, wire ID).
    '''
    return nWiresSum[lay - 1] + wire if lay > 0 else wire


histADC = []
histTDC = []
histADCinLayer = []

for l in range(56):
    histADCinLayer.append(TH1D('h' + str(500000 + l),
                               'ADC Layer' + str(l),
                               400, 0.0, 400.))
    for w in range(nWires[l]):
        hid = getHistID(l, w)
        hADC = TH1D('h' + str(100000 + hid),
                    'ADC Layer' + str(l) + 'Wire' + str(w),
                    400, 0.0, 400.)
        hTDC = TH1D('h' + str(200000 + hid),
                    'TDC Layer' + str(l) + 'Wire' + str(w),
                    2000, 4000., 6000.)
        histADC.append(hADC)
        histTDC.append(hTDC)


histADCTDC = [TH2D('h' + str(300000 + l),
                   'ADC2TDC Layer' + str(l),
                   200, 0.0, 400., 200, 4000, 6000)
              for l in range(56)]

histHit = [TH1D('h' + str(400000 + l),
                'HitDist Layer' + str(l),
                400, 0.0, 400.) for l in range(56)]


class CDCHistMakerModule(Module):

    """
    Class description.
    """

    def __init__(self, exp=0, run=0, dest='.'):
        """
        call constructor of base class, required.
        """

        super(CDCHistMakerModule, self).__init__()
        #: Experimental number
        self.m_exp = exp
        #: Run number
        self.m_run = run
        #: Output directory
        self.m_dest = dest
        if os.path.exists(self.m_dest) is False:
            os.mkdir(self.m_dest)
        #: Output file name
        self.m_outputFile = self.m_dest + '/dqm.{0:0>4}.{1:0>5}.root'.format(self.m_exp, self.m_run)

    def event(self):
        """
        reimplement Module::event()
        """

        hits = Belle2.PyStoreArray('CDCHits')

        for hit in hits:
            #            rawhit = hit.getRelatedTo('CDCRawHits')
            sl = hit.getISuperLayer()
            l = hit.getILayer()
            cl = l if sl == 0 else 8 + (sl - 1) * 6 + l
            w = hit.getIWire()
            adc = hit.getADCCount()
            tdc = hit.getTDCCount()

            #            b = rawhit.getBoardId()
            #            c = rawhit.getFEChannel()
            #            B2DEBUG(99, 'sl ' + str(sl) + ' l ' + str(l) +
            #                    ' cl ' + str(cl) + ' w ' + str(w) +
            #                    ' b ' + str(b) + ' c ' + str(c))
            hid = getHistID(cl, w)
            histADCinLayer[cl].Fill(adc)
            histADC[hid].Fill(adc)
            histTDC[hid].Fill(tdc)
            histADCTDC[cl].Fill(adc, tdc)
            histHit[cl].Fill(w)

    def terminate(self):
        """
        Draw histograms on canvas and save image.
        """

        file = TFile(self.m_outputFile, "RECREATE")
        for h in histADC:
            h.Write()
        for h in histTDC:
            h.Write()
        for h in histADCinLayer:
            h.Write()
        for h in histADCTDC:
            h.Write()
        for h in histHit:
            h.Write()


def main(exp=1, run=3118, prefix='', dest=''):

    # Seach dst files.
    files = glob.glob(prefix + '/dst.cosmic.{0:0>4}.{1:0>5}'.format(exp, run) + '*.root')
    # create path
    main = create_path()
    # Input (ROOT file).
    main.add_module('RootInput',
                    inputFileNames=files)

    main.add_module(CDCHistMakerModule(exp, run, dest))
    main.add_module('Progress')
    # process events and print call statistics
    process(main)
    print(statistics)


if __name__ == "__main__":
    parser = argparse.ArgumentParser()

    parser.add_argument('exp', help='Experimental number')
    parser.add_argument('run', help='Run number')
    args = parser.parse_args()

    main(exp=args.exp, run=args.run,
         prefix='/ghi/fs01/belle2/bdata/users/karim/data/GCR1/build-2017-08-21',
         #         dest='/ghi/fs01/belle2/bdata/group/detector/CDC/qam/GCR1/build-2017-08-21'
         # dest='/ghi/fs01/belle2/bdata/group/detector/CDC/qam/GCR1/test'
         dest='.'  # Store current directory.
         )
