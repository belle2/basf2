#!/usr/bin/env python3
# -*- coding: utf-8 -*-

################################################
# test for module
################################################

import basf2
import ROOT
from ROOT import Belle2


class trg_read(basf2.Module):

    root_file = ROOT.TFile('Hist_charged.root', 'recreate')
    tree = ROOT.TDirectory('input', 'input bits',)

    hist_inbit = ROOT.TH1F('hin', '', 320, 0, 320)
    hist_outbit = ROOT.TH1F('hout', '', 320, 0, 320)
    hist_outbit2 = ROOT.TH1F('hout2', '', 320, 0, 320)  # refer to hie|c4
    hist_outbit3 = ROOT.TH1F('hout3', '', 320, 0, 320)  # refer to fff|stt|fyo

    def initialize(self):
        print('initialize')

    def beginRun(self):
        print('beginRun')

    def event(self):
        trg_summary = Belle2.PyStoreObj('TRGSummary')
        m_dbftdl = Belle2.PyDBObj('TRGGDLDBFTDLBits')
        ihie = -1
        ic4 = -1
        ifff = -1
        istt = -1
        ifyo = -1
        for i in range(320):
            outbitname = m_dbftdl.getoutbitname(i)
            if outbitname == "hie":
                ihie = i
            if outbitname == "c4":
                ic4 = i
            if outbitname == "fff":
                ifff = i
            if outbitname == "fyo":
                ifyo = i
            if outbitname == "stt":
                istt = i

        for i in range(320):
            if(trg_summary.testInput(i)):
                trg_read.hist_inbit.Fill(i+0.5)
            if(trg_summary.testFtdl(i)):
                trg_read.hist_outbit.Fill(i+0.5)
            if(trg_summary.testFtdl(i) and (trg_summary.testFtdl(ihie) or trg_summary.testFtdl(ic4))):
                trg_read.hist_outbit2.Fill(i+0.5)
            if(trg_summary.testFtdl(i) and
               (trg_summary.testFtdl(ifff) or trg_summary.testFtdl(ifyo) or trg_summary.testFtdl(istt))):
                trg_read.hist_outbit3.Fill(i+0.5)

    def endRun(self):
        print('endRun')

    def terminate(self):
        print('terminate')
        m_dbinput = Belle2.PyDBObj('TRGGDLDBInputBits')
        m_dbftdl = Belle2.PyDBObj('TRGGDLDBFTDLBits')

        n_inbit = m_dbinput.getninbit()
        n_outbit = m_dbftdl.getnoutbit()

        trg_read.hist_inbit.GetXaxis().SetRangeUser(0, n_inbit)
        trg_read.hist_outbit.GetXaxis().SetRangeUser(0, n_outbit)
        trg_read.hist_outbit2.GetXaxis().SetRangeUser(0, n_outbit)

        for i in range(320):
            inbitname = m_dbinput.getinbitname(i)
            outbitname = m_dbftdl.getoutbitname(i)
            trg_read.hist_inbit.GetXaxis().SetBinLabel(trg_read.hist_inbit.GetXaxis().FindBin(i+0.5), inbitname)
            trg_read.hist_outbit.GetXaxis().SetBinLabel(trg_read.hist_outbit.GetXaxis().FindBin(i+0.5), outbitname)
            trg_read.hist_outbit2.GetXaxis().SetBinLabel(trg_read.hist_outbit2.GetXaxis().FindBin(i+0.5), outbitname)

        trg_read.hist_inbit.Write()
        trg_read.hist_outbit.Write()
        trg_read.hist_outbit2.Write()
        trg_read.tree.Write()


main = basf2.create_path()

root_input = basf2.register_module('RootInput')

root_input.param('inputFileName', '../root_charged/charged_*.root')
root_input.param('branchNames', ['EventMetaData', 'TRGSummary'])
main.add_module(root_input)
main.add_module(trg_read())

basf2.process(main)
