#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# ------------------------------------------------------------------------
# Basic module to study the time resolution using the laser light
#
# Contributors: Umberto Tamponi (tamponi@to.infn.it)
#
# usage: basf2 studyLaserResolution.py dbaddress (path|none) type (local|pocket) output_name.root path_to_sroot run1 run2 ... runN
#        The run number accepts wildcards
# ------------------------------------------------------------------------


from basf2 import *
import sys
import glob
from ROOT import Belle2
from ROOT import TH1F, TH2F, TF1, TFile, TGraphErrors, TSpectrum, TCanvas
import ROOT
import pylab
import numpy
import time


class TOPLaserHistogrammerModule(Module):

    ''' Module to study resolution and performance of the top laser calibration.'''

    #: Width as function of the sample number in each channel
    h_LaserTimingVSChannel = TH2F(
        'LaserTimingVSChannel',
        'Laser timing in as function of the channel number',
        512 * 16,
        0,
        512 * 16,
        10000,
        0.,
        100)  # 10 ps binning
    h_LaserTimingVSChannelOneSlot = TH2F(
        'LaserTimingVSChannelOneSlot',
        'Laser timing in as function of the channel number',
        512,
        0,
        512,
        10000,
        0.,
        100)  # 10 ps binning

    #: output root file
    outname = 'outStudyLaserResolution.root'

    #: ignores the hits wthout calibration
    m_ignoreNotCalibrated = True
    #: ignores the hits wthout calibration
    m_runOnData = True

    #: maximum width to accept a TOPDigit
    m_maxWidth = 3.
    #: minimum width to accept a TOPDigit
    m_minWidth = 0.5
    #: minimum amplitude to accept a TOPDigit
    m_maxAmp = 700.
    #: minimum amplitude to accept a TOPDigit
    m_minAmp = 250.
    #: root file with the MC distribition of the laser light, to get the light path corrections
    m_mcCorrectionsFile = 't0MC.root'
    #: positions of the first and second peak
    m_MCPeaks = [[]]

    def setOutputName(self, outputname):
        ''' Sets the output file name '''
        #: output name
        self.outname = outputname

    def setMaxWidth(self, maxWidth):
        ''' Sets the maximum calpulse width '''
        #: output name
        self.m_maxWidth = maxWidth

    def setMinWidth(self, minWidth):
        ''' Sets the minimum calpulse width '''
        #: output name
        self.m_minWidth = minWidth

    def setMaxAmp(self, maxAmp):
        ''' Sets the maximum calpulse amplitude '''
        #: output name
        self.m_maxAmp = maxAmp

    def setMinAmp(self, minAmp):
        ''' Sets the minimum calpulse amplitude '''
        #: output name
        self.m_minAmp = minAmp

    def setMCCorrectionsFile(self, MCfile):
        ''' Sets the file containing the MC correction'''
        #: output name
        self.m_mcCorrectionsFile = MCfile

    def ignoreNotCalibrated(self, ignoreNotCal):
        ''' Sets the flag to ingore the hits without calibration '''
        #: output name
        self.m_ignoreNotCalibrated = ignoreNotCal

    def beginRun(self):
        self.m_MCPeaks = [[-1. for second in range(2)] for first in range(512)]    # default
        tfileIn = TFile(self.m_mcCorrectionsFile)
        histoMC = tfileIn.Get('LaserTimingVSChannelOneSlot')
        for kCh in range(512):
            timeProjection = histoMC.ProjectionY('projection', kCh + 1, kCh + 1)
            timeProjection.GetXaxis().SetRangeUser(0., 1.)
            spectrum = TSpectrum()
            numPeaks = spectrum.Search(timeProjection, 2., 'nobackground', 0.1)
            peaks = spectrum.GetPositionX()
            print(str(numPeaks))
            if numPeaks is not 0:
                print('channel ' + str(kCh) + ' ' + str(peaks[0]) + ' ' +
                      str(self.m_MCPeaks[kCh][0]) + ' ' + str(self.m_MCPeaks[kCh][1]))
            for iPeak in range(numPeaks):
                if iPeak < 2:
                    self.m_MCPeaks[kCh][iPeak] = peaks[iPeak]

    def event(self):
        ''' Event processor: fill histograms '''

        digits = Belle2.PyStoreArray('TOPDigits')

        for digit in digits:
            if(not self.ignoreNotCalibrated and not digit.isTimeBaseCalibrated()):
                continue

            if (digit.getHitQuality() == 1 and digit.getPulseHeight() > self.m_minAmp and digit.getPulseHeight() <
                    self.m_maxAmp and digit.getPulseWidth() > self.m_minWidth and digit.getPulseWidth() < self.m_maxWidth):
                slotID = digit.getModuleID()
                hwchan = digit.getChannel()
                self.h_LaserTimingVSChannel.Fill(512 * (slotID - 1) + hwchan, digit.getTime())
                simhits = digit.getRelationsWith('TOPSimHits')
                for simhit in simhits:
                    self.h_LaserTimingVSChannelOneSlot.Fill(hwchan, simhit.getTime())

    def terminate(self):
        ''' Write histograms to file, fills and fits the resolution plots'''
        tfile = TFile(self.outname, 'recreate')
        self.h_LaserTimingVSChannel.Write()
        self.h_LaserTimingVSChannelOneSlot.Write()
        tfile.Close()


argvs = sys.argv
print('usage: basf2', argvs[0], 'runNumber outfileName')

dbaddress = argvs[1]        # path to the calibration DB  (absolute path  or 'none')
datatype = argvs[2]         # data type  ('pocket' or 'local')
outfile = argvs[3]          # output name
folder = argvs[4]           # folder containing the sroot files
runnumbers = sys.argv[5:]   # run numbers

files = []
for runnumber in runnumbers:
    if datatype == 'root':
        files += [f for f in glob.glob(str(folder) + '/*' + str(runnumber) + '*.root')]
    else:
        files += [f for f in glob.glob(str(folder) + '/*' + str(runnumber) + '*.sroot')]
for fname in files:
    print("file: " + fname)

if dbaddress != 'none':
    print("using local DB " + dbaddress)
    reset_database()
    use_local_database(dbaddress + "/localDB.txt", dbaddress)
else:
    print("database not set. Continuing without calibrations")

# Suppress messages and warnings during processing
set_log_level(LogLevel.ERROR)

# Create path
main = create_path()

# input
if datatype == 'root':
    roinput = register_module('RootInput')
    roinput.param('inputFileNames', files)
    main.add_module(roinput)
else:
    roinput = register_module('SeqRootInput')
    roinput.param('inputFileNames', files)
    main.add_module(roinput)

# conversion from RawCOPPER or RawDataBlock to RawDetector objects
if datatype == 'pocket':
    print('pocket DAQ data assumed')
    converter = register_module('Convert2RawDet')
    main.add_module(converter)

# geometry parameters
gearbox = register_module('Gearbox')
main.add_module(gearbox)

# Geometry (only TOP needed)
geometry = register_module('Geometry')
geometry.param('components', ['TOP'])
main.add_module(geometry)

if datatype != 'root':
    # Unpacking
    unpack = register_module('TOPUnpacker')
    unpack.param('swapBytes', True)
    unpack.param('dataFormat', 0x0301)
    main.add_module(unpack)

    # Add multiple hits by running feature extraction offline
    featureExtractor = register_module('TOPWaveformFeatureExtractor')
    main.add_module(featureExtractor)

    # Convert to TOPDigits
    converter = register_module('TOPRawDigitConverter')
    if dbaddress == 'none':
        print("Not using TBC")
        converter.param('useSampleTimeCalibration', False)
    else:
        print("Using TBC")
        converter.param('useSampleTimeCalibration', True)
    converter.param('useChannelT0Calibration', True)
    converter.param('useModuleT0Calibration', True)
    converter.param('useCommonT0Calibration', False)
    converter.param('calibrationChannel', -1)  # do not specify the calpulse channel
    converter.param('lookBackWindows', 29)  # in number of windows
    main.add_module(converter)

# resolution plotter
resolutionModule = TOPLaserHistogrammerModule()
resolutionModule.setOutputName(outfile)
resolutionModule.setMinWidth(0.1)  # good TOPDigit selection
resolutionModule.setMaxWidth(999)  # good TOPDigit selection
resolutionModule.setMinAmp(100)  # good TOPDigit selection
resolutionModule.setMaxAmp(999)  # good TOPDigit selection
if dbaddress == 'none':
    resolutionModule.ignoreNotCalibrated(True)
else:
    resolutionModule.ignoreNotCalibrated(False)
main.add_module(resolutionModule)

# Show progress of processing
progress = register_module('Progress')
main.add_module(progress)

# Process events
process(main)

# Print call statistics
print(statistics)
print(statistics(statistics.TERM))
