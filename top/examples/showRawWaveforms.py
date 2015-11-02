#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# -----------------------------------------------------------------------------------
# Event display of raw waveforms.
# Input from CRT/test beam raw data files (.dat) or from standard Basf2 root files
# usage: basf2 top/examples/showRawWaveforms.py <path/file_name>
# -----------------------------------------------------------------------------------

# avoid race conditions beetween pyroot and GUI thread
from ROOT import PyConfig
PyConfig.StartGuiThread = False

from basf2 import *
import sys
from ROOT import Belle2
from ROOT import TH1F, TCanvas

argvs = sys.argv
if len(argvs) > 1:
    fileName = argvs[1]
else:
    print('usage: basf2 top/examples/showRawWaveforms.py <file_name>')
    sys.exit()

if not os.path.exists(fileName):
    print(fileName + ': file not found')
    sys.exit()


fileType = ''
if fileName.rfind('.dat') + len('.dat') == len(fileName):
    fileType = 'dat'
elif fileName.rfind('.root') + len('.root') == len(fileName):
    fileType = 'root'
else:
    print('unknown file extension')
    sys.exit()


def wait():
    ''' wait for user respond '''

    try:
        q = 0
        Q = 0
        abc = eval(input('Type <CR> to continue or Q to quit '))
        evtMetaData = Belle2.PyStoreObj('EventMetaData')
        evtMetaData.obj().setEndOfData()
        return True
    except:
        return False


class WFDisplay(Module):

    '''
    Simple event display of raw waveforms.
    '''

    #: 1D histograms
    hist = [TH1F('h' + str(i), 'wf', 64, 0.0, 64.0) for i in range(16)]
    #: canvas
    c1 = TCanvas('c1', 'WF event display', 800, 800)

    def initialize(self):
        ''' Initialize the Module: open the canvas. '''

        self.c1.Divide(4, 4)
        self.c1.Show()

    def draw(
        self,
        k,
        event,
        run,
    ):
        ''' Draw histograms and wait for user respond '''

        self.c1.Clear()
        self.c1.Divide(4, 4)
        title = 'WF event display:' + ' run ' + str(run) + ' event ' \
            + str(event)
        self.c1.SetTitle(title)
        for i in range(k):
            self.c1.cd(i + 1)
            self.hist[i].Draw()
        self.c1.Update()
        stat = wait()
        for i in range(k):
            self.hist[i].Reset()
        return stat

    def event(self):
        '''
        Event processor: fill the histograms, display them,
        wait for user respond.
        '''

        evtMetaData = Belle2.PyStoreObj('EventMetaData')
        event = evtMetaData.obj().getEvent()
        run = evtMetaData.obj().getRun()

        waveforms = Belle2.PyStoreArray('TOPRawWaveforms')
        k = 0
        for waveform in waveforms:
            wf = waveform.getWaveform()
            self.hist[k].Reset()
            self.hist[k].SetMinimum(0)
            self.hist[k].SetMaximum(4096)
            chan = waveform.getChannelID()
            window = waveform.getStorageWindow()
            refwin = waveform.getReferenceWindow()
            title = 'chan ' + str(chan) + ' '
            title += 'win ' + str(window) + ' (' + str(refwin) + ')'
            self.hist[k].SetTitle(title)
            self.hist[k].SetStats(False)
            i = 0
            for adc in wf:
                i = i + 1
                self.hist[k].SetBinContent(i, adc)
            k = k + 1
            if k == 16:
                stat = self.draw(k, event, run)
                if stat:
                    return
                k = 0
        if k > 0:
            self.draw(k, event, run)


set_log_level(LogLevel.ERROR)

main = create_path()

if fileType == 'dat':
    reader = register_module('TOPRawdataInput')
    reader.param('inputFileName', fileName)
    main.add_module(reader)
elif fileType == 'root':
    reader = register_module('RootInput')
    reader.param('inputFileName', fileName)
    main.add_module(reader)
else:
    sys.exit()

gearbox = register_module('Gearbox')
bar1 = \
    "/DetectorComponent[@name='TOP']//FrontEndMapping/TOPModule[@barID='1']/"
col0 = bar1 + "ElectronicsModule[@col = '0']/SCRODid"
col1 = bar1 + "ElectronicsModule[@col = '1']/SCRODid"
col2 = bar1 + "ElectronicsModule[@col = '2']/SCRODid"
col3 = bar1 + "ElectronicsModule[@col = '3']/SCRODid"
gearbox.param('override', [(col0, '59', ''), (col1, '66', ''), (col2, '65', ''
                                                                ), (col3, '67', '')])
main.add_module(gearbox)

geometry = register_module('Geometry')
geometry.param('components', ['TOP'])
main.add_module(geometry)

unpack = register_module('TOPUnpacker')
main.add_module(unpack)

main.add_module(WFDisplay())

progress = register_module('Progress')
main.add_module(progress)

process(main)
print(statistics)
