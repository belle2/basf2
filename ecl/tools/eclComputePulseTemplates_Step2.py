import matplotlib
import matplotlib.pyplot as plt
import numpy as np
from scipy.fftpack import fft, ifft
from scipy import signal
import ROOT
from ROOT import TCanvas, TGraph, TFile
import scipy
import sys
import cppyy
import libPyROOT as _backend
from array import array
#
# See eclComputePulseTemplates_Step0.cc for README instructions.
#


def EvalGamComp(tin, ttrg):
    if tin < ttrg:
        return 0
    Ratio = 0.56936
    FastTau = 851.28 * (0.88348588)  # Converting to sample number time scale
    SlowTau = 5802.0 * (0.88348588)  # Converting to sample number time scale
    val = Ratio * ((1. / FastTau) * np.exp(-(tin - ttrg) / FastTau)) + \
        (1 - Ratio) * ((1. / SlowTau) * np.exp(-(tin - ttrg) / SlowTau))
    return val


def EvalHadronComp(tin, ttrg):
    HadronTau = 630. * (0.88348588)  # Converting to sample number time scale
    if tin < ttrg:
        return 0
    val = ((1. / HadronTau) * np.exp(-(tin - ttrg) / HadronTau))
    return val


def EvalAnyComp(tin, ttrg, TauIn):
    TauIn *= (0.88348588)  # Converting to sample number time scale
    if tin < ttrg:
        return 0
    val = ((1. / TauIn) * np.exp(-(tin - ttrg) / TauIn))
    return val


def CalcShaperOutput(muonShaper, muonInitial, inputPreShaper, ITER):
    impulse = ((ifft((fft(muonShaper) / fft(muonInitial)))))
    outputShaper = np.real(ifft(fft(impulse) * fft(inputPreShaper)))
    base = outputShaper[ITER]
    i = 0
    while i < len(outputShaper):
        outputShaper[i] -= base
        i = i + 1
    outputShaper[0] = 0
    print(outputShaper.max())
    return outputShaper


def GetShaperOutput(ratio, flg, shaperMuonFunc):

    Ns = 100000
    TLen = 100000.
    to_ns = 1  # TLen/Ns
    #
    PMT_trigger_time = 1000. * to_ns
    #
    Time = []
    TimeShp = []
    ShaperDSP_output_muon_array = []
    PMT_output_muon_array = []
    PMT_output_array = []
    #
    for i in range(0, Ns):
        t = i * to_ns
        Time.append(t)
        TimeShp.append(t)
        ShaperDSP_output_muon_array.append(shaperMuonFunc[i])
        PMT_output_muon_array.append(EvalGamComp(t, PMT_trigger_time))
        if ratio <= 1.:
            PMT_output_array.append((1 - ratio) * EvalGamComp(t, PMT_trigger_time) + ratio * (EvalHadronComp(t, PMT_trigger_time)))
        else:
            PMT_output_array.append((2 - ratio) * EvalGamComp(t, PMT_trigger_time) +
                                    (ratio - 1) * (EvalAnyComp(t, PMT_trigger_time, 10)))
    #
    if(flg == 0):
        return Time, PMT_output_array
    if(flg == 1):
        return Time, PMT_output_muon_array
    if(flg == 2):
        return Time, ShaperDSP_output_muon_array
    if(flg == 3):
        ShaperDSP_output_array = []
        if(ratio <= 1.):
            ShaperDSP_output_array = CalcShaperOutput(ShaperDSP_output_muon_array, PMT_output_muon_array, PMT_output_array, 0)
        else:
            ShaperDSP_output_array = CalcShaperOutput(ShaperDSP_output_muon_array, PMT_output_muon_array, PMT_output_array, 30)
        j = 0
        Time_us = []
        while j < len(Time):
            Time_us.append(Time[j] / (1000.))
            j = j + 1
        return Time_us, ShaperDSP_output_array


Low = int(sys.argv[1])
High = int(sys.argv[2])
#
f1 = ROOT.TFile("PhotonShapes_Low" + str(Low) + "_High" + str(High) + ".root", "update")
f1.cd()
mt = f1.Get("mtree")
entries = mt.GetEntries()
print(entries)
#
TFactor = 30
#
outFile = TFile("HadronShapes_Low" + str(Low) + "_High" + str(High) + ".root", "RECREATE")
outTree = ROOT.TTree("HadronTree", "")
TimeAll_A = array('d', 1000 * [0.])
ValuePhoton_A = array('d', 1000 * [0.])
ValueHadron_A = array('d', 1000 * [0.])
ValueDiode_A = array('d', 1000 * [0.])
outTree.Branch("TimeAll_A", TimeAll_A, 'TimeAll_A[1000]/D')
outTree.Branch("ValuePhoton_A", ValuePhoton_A, 'ValuePhoton_A[1000]/D')
outTree.Branch("ValueHadron_A", ValueHadron_A, 'ValueHadron_A[1000]/D')
outTree.Branch("ValueDiode_A", ValueDiode_A, 'ValueDiode_A[1000]/D')
#
i = 0
for i in range(entries):
    mt.GetEntry(i)
    if(mt.PhotonArray[1] < -100):
        for j in range(1000):
            ValuePhoton_A[j] = -999
            TimeAll_A[j] = -999
            ValueHadron_A[j] = -999
            ValueDiode_A[j] = -999
    else:
        flag = 3
        Time3, ValuesHadron_A = GetShaperOutput(1, flag, mt.PhotonArray)
        Time4, ValuesDiode_A = GetShaperOutput(2, flag, mt.PhotonArray)
        Time6, ValuesPhoton_A = GetShaperOutput(0, flag, mt.PhotonArray)
        #
        factor = TFactor
        for j in range(1000):
            if(flag == 3):
                ValuePhoton_A[j] = mt.PhotonArray[j * factor]
            else:
                ValuePhoton_A[j] = ValuesPhoton_A[j * factor]
            TimeAll_A[j] = Time3[j * factor]
            ValueHadron_A[j] = ValuesHadron_A[j * factor]
            ValueDiode_A[j] = ValuesDiode_A[j * factor]
    outTree.Fill()
outFile.cd()
outTree.Write()
outFile.Write()
sys.exit()
