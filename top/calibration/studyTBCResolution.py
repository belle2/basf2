#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
import sys
import glob
from ROOT import Belle2
from ROOT import TH1F, TH2F, TF1, TFile, TGraphErrors

#
# ------------------------------------------------------------------------
# Module to study the features of the double pulse used for the TOP calibration.
# To be used to determine the TBC quality and precision
#
# Contributors: Umberto Tamponi (tamponi@to.infn.it)
#
# usage: basf2 studyTBCResolution.py dbaddress (path|none) type (local|pocket) output_name.root path_to_sroot run1 run2 ... runN
#        The run number accepts wildcards
# ------------------------------------------------------------------------


class TOPTBCResolution(Module):

    ''' Module to study resolution and performances of the TOP Time Base Calibration.'''

    #: Width VS amplitude, first calibration pulse
    h_WidthVSAmplitude_1 = TH2F(
        'WidthVSAmplitude_1',
        'Width VS amplidute of the TOPDigits, first calibration pulse',
        2000, 0., 2000, 100, 0., 10.)
    #: Width VS amplitude, second calibration pulse
    h_WidthVSAmplitude_2 = TH2F(
        'WidthVSAmplitude_2',
        'Width VS amplidute of the TOPDigits, second calibration pulse',
        2000, 0., 2000, 100, 0., 10.)

    #: dV/dt on the rising edge, first calibration pulse
    h_dVdtRising_1 = TH1F('dVdtRising_1', ' dV/dt of the TOPRawDigits (rising edge), first calibration pulse', 1000, 0, 1000)
    #: dV/dt on the rising edge, second calibration pulse
    h_dVdtRising_2 = TH1F('dVdtRising_2', ' dV/dt of the TOPRawDigits (rising edge), second calibration pulse', 1000, 0, 1000)
    #: dV/dt on the falling edge, first calibration pulse
    h_dVdtFalling_1 = TH1F('dVdtFalling_1', ' dV/dt of the TOPRawDigits (falling edge), first calibration pulse', 1000, 0, 1000)
    #: dV/dt on the falling edge, second calibration pulse
    h_dVdtFalling_2 = TH1F('dVdtFalling_2', ' dV/dt of the TOPRawDigits (falling edge), second calibration pulse', 1000, 0, 1000)
    #: dV/dt on the rising edge VS dV/dt on the falling edge, first calibration pulse
    h_dVdtRisingVSdVdtFalling_1 = TH2F(
        'dVdtRisingVSdVdtFalling_1',
        ' dV/dt  of the TOPRawDigit: rising edge  VS falling edge, first calibration pulse ',
        1000, 0, 1000, 1000, 0., 1000.)
    #: dV/dt on the rising edge VS dV/dt on the falling edge, first calibration pulse
    h_dVdtRisingVSdVdtFalling_2 = TH2F(
        'dVdtRisingVSdVdtFalling_2',
        ' dV/dt  of the TOPRawDigit: rising edge  VS falling edge, second calibration pulse ',
        1000, 0, 1000, 1000, 0., 1000.)
    #: Difference between the dV/dt of the first and the second calpulse, using the rising edges
    h_dVdtRisingDifference = TH1F(
        'dVdtRisingDifference', ' difference between the rising edge dV/dt of the first and the second pulse', 1000, -500, 500)
    #: Difference between the dV/dt of the first and the second calpulse, using the rising edges
    h_dVdtFallingDifference = TH1F(
        'dVdtFallingDifference', ' difference between the falling edge dV/dt of the first and the second pulse', 1000, -500, 500)

    #: DeltaT rising-rising
    h_DeltaT_RR = TH1F('DeltaT_RR', ' DeltaT bewteen the rising edges', 4000, 10, 30)
    #: DeltaT falling-falling
    h_DeltaT_FF = TH1F('DeltaT_FF', ' DeltaT bewteen the falling edges', 4000, 10, 30)
    #: DeltaT falling-rising
    h_DeltaT_FR = TH1F('DeltaT_FR', ' DeltaT bewteen falling and rising edges', 4000, 10, 30)
    #: DeltaT rising-falling
    h_DeltaT_RF = TH1F('DeltaT_RF', ' DeltaT bewteen rising and falling edges', 4000, 10, 30)

    #: DeltaT rising-rising VS channel
    h_DeltaTVSChannel_RR = TH2F(
        'DeltaTVSChannel_RR',
        ' DeltaT bewteen the rising edges, as function of the channel number',
        512 * 16, 0, 512 * 16, 4000, 10., 30.)
    #: DeltaT falling-falling VS channel
    h_DeltaTVSChannel_FF = TH2F(
        'DeltaTVSChannel_FF',
        ' DeltaT bewteen the falling edges, as function of the channel number',
        512 * 16, 0, 512 * 16, 4000, 10., 30.)
    #: DeltaT falling-rising VS channel
    h_DeltaTVSChannel_FR = TH2F(
        'DeltaTVSChannel_FR',
        ' DeltaT bewteen falling (pulse 1) and rising (pulse 2) edge, as function of the channel number',
        512 * 16, 0, 512 * 16, 4000, 10., 30.)
    #: DeltaT rising-falling VS channel
    h_DeltaTVSChannel_RF = TH2F(
        'DeltaTVSChannel_RF',
        ' DeltaT bewteen rising (pulse 1) and falling (pulse 2) edge, as function of the channel number',
        512 * 16, 0, 512 * 16, 4000, 10., 30.)
    #: DeltaT rising-rising VS average of dV/dt on the first and second pulse
    h_DeltaTVSdVdt_RR = TH2F(
        'DeltaTVSdVdt_RR',
        'DeltaT bewteen the rising edges VS average of dV/dt on the first and second pulser',
        1000, 0., 1000., 4000, 10., 30.)
    #: DeltaT falling-falling VS average of dV/dt on the first and second pulse
    h_DeltaTVSdVdt_FF = TH2F(
        'DeltaTVSdVdt_FF',
        'DeltaT bewteen the rising edges VS average of dV/dt on the first and second pulser',
        1000, 0., 1000., 4000, 10., 30.)

    #: DeltaT resolution VS average of dV/dt (falling-falling)
    h_ResolutionVSdVdt_FF = TGraphErrors()
    #: DeltaT resolution VS average of dV/dt (rising-rising)
    h_ResolutionVSdVdt_RR = TGraphErrors()

    #: output root file
    outname = 'outStudyTBCResolution.root'
    #: maximum width to  flag a calpulse candidate
    m_calpulseMaxWidth = 3.
    #: minimum width to  flag a calpulse candidate
    m_calpulseMinWidth = 0.5
    #: minimum amplitude to  flag a calpulse candidate
    m_calpulseMaxAmp = 700.
    #: minimum amplitude to  flag a calpulse candidate
    m_calpulseMinAmp = 250.
    #: ignores the hits wthout calibration
    m_ignoreNotCalibrated = True

    def setOutputName(self, outputname):
        ''' Sets the output file name '''
        #: output name
        self.outname = outputname

    def setMaxWidth(self, maxWidth):
        ''' Sets the maximum calpulse width '''
        #: output name
        self.m_calpulseMaxWidth = maxWidth

    def setMinWidth(self, minWidth):
        ''' Sets the minimum calpulse width '''
        #: output name
        self.m_calpulseMinWidth = minWidth

    def setMaxAmp(self, maxAmp):
        ''' Sets the maximum calpulse amplitude '''
        #: output name
        self.m_calpulseMaxAmp = maxAmp

    def setMinAmp(self, minAmp):
        ''' Sets the minimum calpulse amplitude '''
        #: output name
        self.m_calpulseMinAmp = minAmp

    def ignoreNotCalibrated(self, ignoreNotCal):
        ''' Sets the flag to ingore the hits without calibration '''
        #: output name
        self.m_ignoreNotCalibrated = ignoreNotCal

    def event(self):
        ''' Event processor: fill histograms '''

        digits = Belle2.PyStoreArray('TOPDigits')

        for ipulse1, digit1 in enumerate(digits):
            if(self.ignoreNotCalibrated and not digit1.isTimeBaseCalibrated()):
                continue
            if (digit1.getHitQuality() != 0 and
                digit1.getPulseHeight() > self.m_calpulseMinAmp and
                digit1.getPulseHeight() < self.m_calpulseMaxAmp and
                digit1.getPulseWidth() > self.m_calpulseMinWidth and
                    digit1.getPulseWidth() < self.m_calpulseMaxWidth):

                slotID = digit1.getModuleID()
                hwchan = digit1.getChannel()
                for ipulse2, digit2 in enumerate(digits, start=ipulse1 + 1):
                    if(self.ignoreNotCalibrated and not digit2.isTimeBaseCalibrated()):
                        continue

                    if (digit2.getHitQuality() != 0 and
                        digit2.getPulseHeight() > self.m_calpulseMinAmp and
                        digit2.getPulseHeight() < self.m_calpulseMaxAmp and
                        digit2.getPulseWidth() > self.m_calpulseMinWidth and
                        digit2.getPulseWidth() < self.m_calpulseMaxWidth and
                        slotID == digit2.getModuleID() and
                            hwchan == digit2.getChannel()):

                        # finds which one is the first calpulse
                        rawDigitFirst = digit1.getRelated('TOPRawDigits')
                        rawDigitSecond = digit2.getRelated('TOPRawDigits')
                        if digit1.getTime() > digit2.getTime():
                            rawDigitFirst = digit2.getRelated('TOPRawDigits')
                            rawDigitSecond = digit1.getRelated('TOPRawDigits')
                        digitFirst = rawDigitFirst.getRelated('TOPDigits')
                        digitSecond = rawDigitSecond.getRelated('TOPDigits')

                        globalCh = hwchan + 512 * (slotID - 1)
                        dV1_R = rawDigitFirst.getLeadingSlope()
                        dV1_F = -rawDigitFirst.getFallingSlope()
                        dV2_R = rawDigitSecond.getLeadingSlope()
                        dV2_F = -rawDigitSecond.getFallingSlope()
                        t1_R = digitFirst.getTime()
                        t1_F = digitFirst.getTime() + digitFirst.getPulseWidth()
                        t2_R = digitSecond.getTime()
                        t2_F = digitSecond.getTime() + digitSecond.getPulseWidth()
                        amp1 = digitFirst.getPulseHeight()
                        amp2 = digitSecond.getPulseHeight()
                        w1 = digitFirst.getPulseWidth()
                        w2 = digitSecond.getPulseWidth()

                        self.h_WidthVSAmplitude_1.Fill(amp1, w1)
                        self.h_WidthVSAmplitude_2.Fill(amp2, w2)
                        self.h_dVdtRising_1.Fill(dV1_R)
                        self.h_dVdtRising_2.Fill(dV2_R)
                        self.h_dVdtFalling_1.Fill(dV1_F)
                        self.h_dVdtFalling_2.Fill(dV2_F)
                        self.h_dVdtRisingVSdVdtFalling_1.Fill(dV1_F, dV1_R)
                        self.h_dVdtRisingVSdVdtFalling_2.Fill(dV2_F, dV2_R)
                        self.h_dVdtRisingDifference.Fill(dV1_R - dV2_R)
                        self.h_dVdtFallingDifference.Fill(dV1_F - dV2_F)
                        self.h_DeltaT_RR.Fill(t2_R - t1_R)
                        self.h_DeltaT_FF.Fill(t2_F - t1_F)
                        self.h_DeltaT_FR.Fill(t2_F - t1_R)
                        self.h_DeltaT_RF.Fill(t2_R - t1_F)
                        self.h_DeltaTVSChannel_RR.Fill(globalCh, t2_R - t1_R)
                        self.h_DeltaTVSChannel_FF.Fill(globalCh, t2_F - t1_F)
                        self.h_DeltaTVSChannel_FR.Fill(globalCh, t2_F - t1_R)
                        self.h_DeltaTVSChannel_RF.Fill(globalCh, t2_R - t1_F)
                        self.h_DeltaTVSdVdt_RR.Fill(0.5 * (dV1_R + dV2_R), t2_R - t1_R)
                        self.h_DeltaTVSdVdt_FF.Fill(0.5 * (dV1_F + dV2_F), t2_F - t1_F)

    def terminate(self):
        ''' Write histograms to file, fills and fits the resolution plots'''

        self.h_ResolutionVSdVdt_RR.SetName('ResolutionVSdVdt_RR')
        self.h_ResolutionVSdVdt_RR.SetTitle('Resolution VS dV/dt (rising-rising)')
        self.h_ResolutionVSdVdt_FF.SetName('ResolutionVSdVdt_FF')
        self.h_ResolutionVSdVdt_FF.SetTitle('Resolution VS dV/dt (falling-falling)')

        for ibin in range(0, 10):
            projection = self.h_DeltaTVSdVdt_RR.ProjectionY("tmpProj", ibin * 100 + 1, (ibin + 1) * 100)
            gaussFit = TF1("gaussFit", "[0]*exp(-0.5*((x-[1])/[2])**2)", 10., 30.)
            gaussFit.SetParameter(0, 1.)
            gaussFit.SetParameter(1, projection.GetMean())
            gaussFit.SetParameter(2, projection.GetRMS())
            gaussFit.SetParLimits(2, 0., 3. * projection.GetRMS())

            projection.Fit("gaussFit")
            self.h_ResolutionVSdVdt_RR.SetPoint(ibin, ibin * 100. + 50., gaussFit.GetParameter(2))
            self.h_ResolutionVSdVdt_RR.SetPointError(ibin, 50., gaussFit.GetParError(2))

        tfile = TFile(self.outname, 'recreate')
        self.h_WidthVSAmplitude_1.GetXaxis().SetTitle("TOPDigit amplitude [ADC counts]")
        self.h_WidthVSAmplitude_1.GetYaxis().SetTitle("TOPDigit width [ns]")
        self.h_WidthVSAmplitude_1.Write()
        self.h_WidthVSAmplitude_2.GetXaxis().SetTitle("TOPDigit amplitude [ADC counts]")
        self.h_WidthVSAmplitude_2.GetYaxis().SetTitle("TOPDigit width [ns]")
        self.h_WidthVSAmplitude_2.Write()

        self.h_dVdtRising_1.GetXaxis().SetTitle("dV/dt [ADC counts / sample]")
        self.h_dVdtRising_1.Write()
        self.h_dVdtRising_2.GetXaxis().SetTitle("dV/dt [ADC counts / sample]")
        self.h_dVdtRising_2.Write()
        self.h_dVdtFalling_1.GetXaxis().SetTitle("dV/dt [ADC counts / sample]")
        self.h_dVdtFalling_1.Write()
        self.h_dVdtFalling_2.GetXaxis().SetTitle("dV/dt [ADC counts / sample]")
        self.h_dVdtFalling_2.Write()

        self.h_dVdtRisingVSdVdtFalling_1.GetXaxis().SetTitle("dV/dt on the falling edge [ADC counts / sample]")
        self.h_dVdtRisingVSdVdtFalling_1.GetYaxis().SetTitle("dV/dt on the rising edge [ADC counts / sample]")
        self.h_dVdtRisingVSdVdtFalling_1.Write()

        self.h_dVdtRisingVSdVdtFalling_2.GetXaxis().SetTitle("dV/dt on the falling edge [ADC counts / sample]")
        self.h_dVdtRisingVSdVdtFalling_2.GetYaxis().SetTitle("dV/dt on the rising edge [ADC counts / sample]")
        self.h_dVdtRisingVSdVdtFalling_2.Write()

        self.h_dVdtFallingDifference.GetXaxis().SetTitle("dV/dt_1 - dV/dt_2 [ADC counts / sample]")
        self.h_dVdtFallingDifference.Write()

        self.h_dVdtRisingDifference.GetXaxis().SetTitle("dV/dt_1 - dV/dt_2  [ADC counts / sample]")
        self.h_dVdtRisingDifference.Write()

        self.h_DeltaT_RR.GetXaxis().SetTitle("#Delta t  [ns]")
        self.h_DeltaT_RR.Write()
        self.h_DeltaT_FF.GetXaxis().SetTitle("#Delta t  [ns]")
        self.h_DeltaT_FF.Write()
        self.h_DeltaT_FR.GetXaxis().SetTitle("#Delta t  [ns]")
        self.h_DeltaT_FR.Write()
        self.h_DeltaT_RF.GetXaxis().SetTitle("#Delta t  [ns]")
        self.h_DeltaT_RF.Write()

        self.h_DeltaTVSChannel_RR.GetXaxis().SetTitle("Global channel number [hwChannel + 512*(slotID-1)]")
        self.h_DeltaTVSChannel_RR.GetYaxis().SetTitle("#Delta t  [ns]")
        self.h_DeltaTVSChannel_RR.Write()
        self.h_DeltaTVSChannel_FF.GetXaxis().SetTitle("Global channel number [hwChannel + 512*(slotID-1)]")
        self.h_DeltaTVSChannel_FF.GetYaxis().SetTitle("#Delta t  [ns]")
        self.h_DeltaTVSChannel_FF.Write()
        self.h_DeltaTVSChannel_FR.GetXaxis().SetTitle("Global channel number [hwChannel + 512*(slotID-1)]")
        self.h_DeltaTVSChannel_FR.GetYaxis().SetTitle("#Delta t  [ns]")
        self.h_DeltaTVSChannel_FR.Write()
        self.h_DeltaTVSChannel_RF.GetXaxis().SetTitle("Global channel number [hwChannel + 512*(slotID-1)]")
        self.h_DeltaTVSChannel_RF.GetYaxis().SetTitle("#Delta t  [ns]")
        self.h_DeltaTVSChannel_RF.Write()

        self.h_DeltaTVSdVdt_RR.GetXaxis().SetTitle("Average of dV/dt on first and second pulse [ACD counts / sample]")
        self.h_DeltaTVSdVdt_RR.GetYaxis().SetTitle("#Delta t  [ns]")
        self.h_DeltaTVSdVdt_RR.Write()

        self.h_DeltaTVSdVdt_FF.GetXaxis().SetTitle("Average of dV/dt on first and second pulse [ACD counts / sample]")
        self.h_DeltaTVSdVdt_FF.GetYaxis().SetTitle("#Delta t  [ns]")
        self.h_DeltaTVSdVdt_FF.Write()

        self.h_ResolutionVSdVdt_RR.Write()
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

# Unpacking (format auto detection should work now)
unpack = register_module('TOPUnpacker')
# unpack.param('swapBytes', True)
# unpack.param('dataFormat', 0x0301)
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
converter.param('useChannelT0Calibration', False)
converter.param('useModuleT0Calibration', False)
converter.param('useCommonT0Calibration', False)
converter.param('calibrationChannel', -1)  # do not specify the calpulse channel
converter.param('lookBackWindows', 28)  # in number of windows
main.add_module(converter)

# resolution plotter
resolutionModule = TOPTBCResolution()
resolutionModule.setOutputName(outfile)
resolutionModule.setMinWidth(0.5)  # calpluse candidate selection
resolutionModule.setMaxWidth(3.5)  # calpluse candidate selection
resolutionModule.setMinAmp(150)  # calpluse candidate selection
resolutionModule.setMaxAmp(999)  # calpluse candidate selection
if dbaddress == 'none':
    resolutionModule.ignoreNotCalibrated(False)
else:
    resolutionModule.ignoreNotCalibrated(True)
main.add_module(resolutionModule)

# Show progress of processing
progress = register_module('Progress')
main.add_module(progress)

# Process events
process(main)

# Print call statistics
print(statistics)
print(statistics(statistics.TERM))
