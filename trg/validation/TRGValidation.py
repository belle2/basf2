#!/usr/bin/env python3
##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
<header>
  <input>TRGValidationGen.root</input>
  <output>TRGValidation.root</output>
  <contact>yinjh@nankai.edu.cn</contact>
  <description>makes validation plots for TRG</description>
</header>

"""

import basf2
import modularAnalysis as ma  # a shorthand for the analysis tools namespace
import stdCharged as stdc
import variables.utils as vu

import ROOT
from ROOT import Belle2, TH1F, TFile, TNamed, TEfficiency, TMath
from math import pi as PI


inputBits = ["t3_0", "ty_0", "t2_0", "ts_0", "ta_0", "typ", "ehigh", "elow", "elum", "ecl_3dbha", "cdc_open90",
             "clst_0", "clst_1", "clst_2", "clst_3", "klm_hit", "klm_0", "klm_1", "klm_2", "eklm_0", "eklm_1", "eklm_2"]
outputBits = ["fff", "ffy", "ffz", "fzo", "fso", "fyo", "ffb", "fsb", "ssb", "stt", "hie", "c4", "bha3d",
              "lml1", "mu_b2b", "mu_eb2b", "eklm2", "beklm", "cdcklm1", "cdcklm2", "seklm1", "seklm2", "ecleklm1"]
moniInbits = []
moniOutbits = []

inputBits_expert = [
    't3_0',
    't3_1',
    't3_2',
    't3_3',
    'ty_0',
    'ty_1',
    'ty_2',
    'ty_3',
    't2_0',
    't2_1',
    't2_2',
    't2_3',
    'ts_0',
    'ts_1',
    'ts_2',
    'ts_3',
    'ta_0',
    'ta_1',
    'ta_2',
    'ta_3',
    'typ',
    'typ4',
    'typ5',
    'cdc_open90',
    'cdc_active',
    'cdc_b2b3',
    'cdc_b2b5',
    'cdc_b2b7',
    'cdc_b2b9',
    'itsfb2b',
    'ti',
    'i2io',
    'i2fo',
    'f2f30',
    's2f30',
    's2s30',
    's2s3',
    's2s5',
    's2so',
    's2f3',
    's2f5',
    's2fo',
    'fwd_s',
    'bwd_s',
    'track',
    'trkflt',
    'ehigh',
    'elow',
    'elum',
    'ecl_bha',
    'ecl_3dbha',
    'bha_veto',
    'bha_type_0',
    'bha_type_1',
    'bha_intrk',
    'bha_theta_0',
    'bha_theta_1',
    'clst_0',
    'clst_1',
    'clst_2',
    'clst_3',
    'ecl_active',
    'ecl_timing_fwd',
    'ecl_timing_brl',
    'ecl_timing_bwd',
    'ecl_phys',
    'ecl_oflo',
    'ecl_lml_0',
    'ecl_lml_1',
    'ecl_lml_2',
    'ecl_lml_3',
    'ecl_lml_4',
    'ecl_lml_5',
    'ecl_lml_6',
    'ecl_lml_7',
    'ecl_lml_8',
    'ecl_lml_9',
    'ecl_lml_10',
    'ecl_lml_12',
    'ecl_lml_13',
    'ecl_mumu',
    'ecl_bhapur',
    'ecl_bst',
    'top_0',
    'top_1',
    'top_2',
    'top_bb',
    'top_active',
    'klm_hit',
    'klm_0',
    'klm_1',
    'klm_2',
    'klmb2b',
    'eklm_hit',
    'eklm_0',
    'eklm_1',
    'eklm_2',
    'eklmb2b',
    'revo',
    'her_kick',
    'ler_kick',
    'bha_delay',
    'pseud_rand',
    'plsin',
    'poissonin',
    'veto',
    'injv',
    'secl',
    'iecl_0',
    'iecl_1',
    'samhem',
    'opohem',
    'd3',
    'd5',
    'd7',
    'p3',
    'p5',
    'p7',
    'typ6',
    'cdcecl_0',
    'cdcecl_1',
    'cdcecl_2',
    'cdcecl_3',
    'c2gev_0',
    'c2gev_1',
    'c2gev_2',
    'c2gev_3',
    'cdctop_0',
    'cdctop_1',
    'cdctop_2',
    'cdctop_3',
    'cdcklm_0',
    'cdcklm_1',
    'seklm_0',
    'seklm_1',
    'ecleklm',
    'ieklm',
    'fwdsb',
    'bwdsb',
    'fwdnb',
    'bwdnb',
    'brlfb1',
    'brlfb2',
    'brlnb1',
    'brlnb2',
    'trkbha1',
    'trkbha2',
    'grlgg1',
    'grlgg2',
    'nimin0',
    'nimin1',
    'ecl_taub2b',
    'ehigh1',
    'ehigh2',
    'ehigh3']
outputBits_expert = [
    'fff',
    'ffs',
    'fss',
    'sss',
    'ffz',
    'fzz',
    'zzz',
    'ffy',
    'fyy',
    'yyy',
    'ff',
    'fs',
    'ss',
    'fz',
    'zz',
    'fy',
    'yy',
    'ffo',
    'fso',
    'sso',
    'fzo',
    'fyo',
    'ffb',
    'fsb',
    'ssb',
    'fzb',
    'fyb',
    'aaa',
    'aaao',
    'aao',
    'aab',
    'aa',
    'hie',
    'lowe',
    'lume',
    'hade',
    'c2',
    'c3',
    'c4',
    'c5',
    'bha3d',
    'bhabha',
    'bhabha_trk',
    'bhabha_brl',
    'bhabha_ecp',
    'bhapur',
    'eclmumu',
    'bhauni',
    'ecloflo',
    'eclbst',
    'g_high',
    'g_c1',
    'gg',
    'eed',
    'fed',
    'fp',
    'sp',
    'zp',
    'yp',
    'd_5',
    'shem',
    'ohem',
    'toptiming',
    'ecltiming',
    'cdctiming',
    'cdcbb',
    'mu_pair',
    'mu_b2b',
    'klmhit',
    'mu_epair',
    'mu_eb2b',
    'eklmhit',
    'revolution',
    'random',
    'bg',
    'pls',
    'poisson',
    'vetout',
    'f',
    's',
    'z',
    'y',
    'a',
    'n1gev1',
    'n1gev2',
    'n1gev3',
    'n1gev4',
    'n2gev1',
    'n2gev2',
    'n2gev3',
    'n2gev4',
    'c2gev1',
    'c2gev2',
    'c2gev3',
    'c2gev4',
    'cdcecl1',
    'cdcecl2',
    'cdcecl3',
    'cdcecl4',
    'cdcklm1',
    'cdcklm2',
    'cdcklm3',
    'cdcklm4',
    'cdctop1',
    'cdctop2',
    'cdctop3',
    'cdctop4',
    'c1hie',
    'c1lume',
    'n1hie',
    'n1lume',
    'c3hie',
    'c3lume',
    'n3hie',
    'n3lume',
    'lml0',
    'lml1',
    'lml2',
    'lml3',
    'lml4',
    'lml5',
    'lml6',
    'lml7',
    'lml8',
    'lml9',
    'lml10',
    'lml12',
    'lml13',
    'zzzv',
    'yyyv',
    'fffv',
    'zzv',
    'yyv',
    'ffov',
    'fffov',
    'hiev',
    'lumev',
    'c4v',
    'bhabhav',
    'mu_pairv',
    'bha3dv',
    'bffo',
    'bhie',
    'ffoc',
    'ffoc2',
    'fffo',
    'ff30',
    'fs30',
    'ss30',
    'itsf_b2b',
    'gggrl',
    'ggtsf',
    'ggbrl',
    'bhabrl',
    'bhamtc1',
    'bhamtc2',
    'bhaf',
    'nim0',
    'nima01',
    'nimo01']
moniInbits_expert = []
moniOutbits_expert = []


class MakePlots(basf2.Module):
    '''
    Make validation histograms for trg ecl/cdc/klm
    '''

    def set_descr_shifter(self, histogram, description, check):
        '''
        Sets description, check and contact to validation histogram.
        :param histogram validation histogram
        :param description description text
        :param check information on what to check in comparison with reference
        '''

        descr = TNamed("Description", description)
        histogram.GetListOfFunctions().Add(descr)
        Check = TNamed("Check", check)
        histogram.GetListOfFunctions().Add(Check)
        contact = TNamed("Contact", "yinjh@nankai.edu.cn")
        histogram.GetListOfFunctions().Add(contact)
        Meta = TNamed("MetaOptions", "shifter,pvalue-warn=0.02,pvalue-error=0.01")
        histogram.GetListOfFunctions().Add(Meta)

    def set_descr_expert(self, histogram, description, check):
        '''
        Sets description, check and contact to validation histogram.
        :param histogram validation histogram
        :param description description text
        :param check information on what to check in comparison with reference
        '''

        descr = TNamed("Description", description)
        histogram.GetListOfFunctions().Add(descr)
        Check = TNamed("Check", check)
        histogram.GetListOfFunctions().Add(Check)
        contact = TNamed("Contact", "yinjh@nankai.edu.cn")
        histogram.GetListOfFunctions().Add(contact)
        Meta = TNamed("MetaOptions", "expert,pvalue-warn=0.02,pvalue-error=0.01")
        histogram.GetListOfFunctions().Add(Meta)

    def set_style(self, histogram, xtitle, ytitle):
        '''
        Sets x-y titles, and sets histogram style.
        :param histogram validation histogram
        :param xtitle X-axis title
        :param xtitle Y-axis title
        '''

        histogram.GetXaxis().SetTitle(xtitle)
        histogram.GetYaxis().SetTitle(ytitle)
        histogram.GetYaxis().SetTitleOffset(1.4)
        histogram.GetXaxis().SetTitleSize(0.045)
        histogram.GetYaxis().SetLabelSize(0.020)
        histogram.SetLineColor(ROOT.kBlack)

    def get_relative(self, hist1, hist2, title, particle, trgbit):
        '''
        Get relative ratio between two hists.
        :param hist1 numerator
        :param hist2 denominator
        :param title new histogram title
        :param particle particle name
        :param trgbit trigger bit
        '''

        bin1 = hist1.GetXaxis().GetNbins()
        bin2 = hist2.GetXaxis().GetNbins()
        Xmin1 = hist1.GetXaxis().GetXmin()
        Xmax1 = hist1.GetXaxis().GetXmax()
        Xmin2 = hist2.GetXaxis().GetXmin()
        Xmax2 = hist2.GetXaxis().GetXmax()
        if bin1 != bin2 or Xmin1 != Xmin2 or Xmax1 != Xmax2:
            print("warning: two histograms cannot be compared!!")
            return 0

        teff = TEfficiency(hist1, hist2)
        htitle = f'h_eff_{title}_{particle}_{trgbit}'
        heff = TH1F(htitle, f"efficiency histogram of {particle} for {trgbit}", bin1, Xmin1, Xmax1)
        for ibin in range(1, bin1+1):
            binc0 = teff.GetEfficiency(ibin)
            bine0 = (teff.GetEfficiencyErrorUp(ibin) + teff.GetEfficiencyErrorLow(ibin)) / 2.0
            heff.SetBinContent(ibin, binc0)
            heff.SetBinError(ibin, bine0)
        heff.GetYaxis().SetRangeUser(0.0, 1.2)
        return heff

    def initialize(self):
        '''
        initialize class members
        '''

        print('start read')
        #: output file
        self.tfile = TFile("./TRGValidation.root", "update")
        #: number of events
        self.Nevent = 0

        m_dbinput = Belle2.PyDBObj("TRGGDLDBInputBits")
        m_dbftdl = Belle2.PyDBObj("TRGGDLDBFTDLBits")
        for i in range(320):
            inbitname = m_dbinput.getinbitname(i)
            outbitname = m_dbftdl.getoutbitname(i)
            if inbitname in inputBits:
                moniInbits.append({"name": inbitname, "index": i})
            if outbitname in outputBits:
                moniOutbits.append({"name": outbitname, "index": i})
            if inbitname in inputBits_expert:
                moniInbits_expert.append({"name": inbitname, "index": i})
            if outbitname in outputBits_expert:
                moniOutbits_expert.append({"name": outbitname, "index": i})

        mc_px = "MCParticles.m_momentum_x"
        mc_py = "MCParticles.m_momentum_y"
        trk2d_omega = "TRGCDC2DFinderTracks.m_omega"
        trk2d_phi = "TRGCDC2DFinderTracks.m_phi0"

        ROOT.gROOT.SetBatch(True)
        ROOT.gStyle.SetOptStat(ROOT.kFALSE)

        #: validation histogram
        n_inbit_test = len(inputBits)
        self.hist_inbit = TH1F("hin", "trigger input bits", n_inbit_test, 0, n_inbit_test)
        self.hist_inbit.GetXaxis().SetRangeUser(0, n_inbit_test)
        self.hist_inbit.GetXaxis().SetLabelSize(0.04)
        self.set_descr_shifter(self.hist_inbit, "trigger input bits for shifter",
                               "Efficiency should not drop significantly for any trigger bit")
        self.set_style(self.hist_inbit, "", "Efficiency")

        #: validation histogram
        n_oubit_test = len(outputBits)
        self.hist_outbit = TH1F("hout", "trigger output bits", n_oubit_test, 0, n_oubit_test)
        self.hist_outbit.GetXaxis().SetRangeUser(0, n_oubit_test)
        self.hist_outbit.GetXaxis().SetLabelSize(0.04)
        self.set_descr_shifter(
            self.hist_outbit,
            "trigger ftdl bits for shifter",
            "Efficiency should not drop significantly. For some output bits the efficiency is very low, close to zero.")
        self.set_style(self.hist_outbit, "", "Efficiency")

        for i in range(n_inbit_test):
            self.hist_inbit.GetXaxis().SetBinLabel(
                 self.hist_inbit.GetXaxis().FindBin(i + 0.5), inputBits[i])
        for i in range(n_oubit_test):
            self.hist_outbit.GetXaxis().SetBinLabel(
                 self.hist_outbit.GetXaxis().FindBin(i + 0.5), outputBits[i])

        #: validation histogram
        self.h_E_ECL = TH1F("h_E_ECL", "ECL cluster energy [50 MeV]", 100, 0, 5)
        self.set_descr_shifter(self.h_E_ECL, "TRG ECL cluster energy", "Exponentially falling distribution")
        self.set_style(self.h_E_ECL, "ECL cluster energy (GeV)", "Events/(50 MeV)")

        #: validation histogram
        self.h_Esum_ECL = TH1F("h_Esum_ECL", "sum of ECL cluster energy [50 MeV]", 100, 0, 5)
        self.set_descr_shifter(self.h_Esum_ECL, "sum of TRG ECL cluster energy", "Peak at 200 MeV with tail")
        self.set_style(self.h_Esum_ECL, "sum of ECL cluster energy (GeV)", "Events/(50 MeV)")

        #: validation histogram
        self.h_theta_ECL = TH1F("h_theta_ECL", "TRG ECL cluster #theta [1.4 degrees]", 128, 0, 180)
        self.set_descr_shifter(self.h_theta_ECL, "TRG ECL cluster polar angle", "uniform in barrel")
        self.set_style(self.h_theta_ECL, "TRG ECL cluster polar angle (degree)", "Events/(1.4 degree)")

        #: validation histogram
        self.h_thetaID_ECL = TH1F("h_thetaID_ECL", "ECL cluster TC ID", 610, 0, 610)
        self.set_descr_shifter(self.h_thetaID_ECL, "TRG ECL cluster theta ID", "uniform in barrel")
        self.set_style(self.h_thetaID_ECL, "ECL cluster TC ID", "Events/(1)")

        #: validation histogram
        self.h_phi_ECL = TH1F("h_phi_ECL", "TRG ECL cluster phi [2.0 degrees]", 180, -180, 180)
        self.set_descr_shifter(self.h_phi_ECL, "TRG ECL cluster phi distribution", "distribute uniformly")
        self.set_style(self.h_phi_ECL, "ECL cluster #phi (degree) ", "Events/(2.0 degrees)")

        #: validation histogram
        self.h_sector_BKLM = TH1F("h_sector_BKLM", "BKLM TRG hit sector", 10, 0, 10)
        self.set_descr_shifter(self.h_sector_BKLM, "# of BKLM TRG sector", "peak at 0")
        self.set_style(self.h_sector_BKLM, "# of BKLM TRG sector", "Events/(1)")

        #: validation histogram
        self.h_sector_EKLM = TH1F("h_sector_EKLM", "EKLM TRG hit sector", 10, 0, 10)
        self.set_descr_shifter(self.h_sector_EKLM, "# of EKLM TRG sector", "peak at 0")
        self.set_style(self.h_sector_EKLM, "# of EKLM TRG sector", "Events/(1)")

        #: validation histogram for experts
        n_inbit_expert = len(inputBits_expert)
        self.hist_inbit_expert = TH1F("hin_expert", "trigger input bits", n_inbit_expert, 0, n_inbit_expert)
        self.hist_inbit_expert.GetXaxis().SetRangeUser(0, n_inbit_expert)
        self.set_descr_expert(self.hist_inbit_expert, "# of EKLM TRG sector", "peak at 0")
        self.set_style(self.hist_inbit_expert, "", "Efficiency")

        #: validation histogram for experts
        n_oubit_expert = len(outputBits_expert)
        self.hist_outbit_expert = TH1F("hout_expert", "trigger output bits", n_oubit_expert, 0, n_oubit_expert)
        self.hist_outbit_expert.GetXaxis().SetRangeUser(0, n_oubit_expert)
        self.set_descr_expert(self.hist_outbit_expert, "# of EKLM TRG sector", "peak at 0")
        self.set_style(self.hist_outbit_expert, "", "Efficiency")

        for i in range(n_inbit_expert):
            self.hist_inbit_expert.GetXaxis().SetBinLabel(
                 self.hist_inbit_expert.GetXaxis().FindBin(i + 0.5), inputBits_expert[i])
        for i in range(n_oubit_expert):
            self.hist_outbit_expert.GetXaxis().SetBinLabel(
                 self.hist_outbit_expert.GetXaxis().FindBin(i + 0.5), outputBits_expert[i])

        mc = "abs(MCParticles.m_pdg)==11&&MCParticles.m_status==11"
        tree = ROOT.TChain("tree")
        tree.Add("../TRGValidationGen.root")

        #: validation histogram
        self.d_w = TH1F("d_w", "#Deltaw of CDC 2D finder, w = 0.00449/p_{t}", 50, -0.02, 0.02)
        tree.Draw(f"({trk2d_omega} - 0.00449)/sqrt({mc_px}*{mc_px} + {mc_py}*{mc_py})>> d_w",
                  "MCParticles.m_pdg<0&&" + mc)
        #: validation histogram
        self.d_w_2 = TH1F("d_w_2", "d_w_2", 50, -0.02, 0.02)
        tree.Draw(f"({trk2d_omega} + 0.00449)/sqrt({mc_px}*{mc_px} + {mc_py}*{mc_py}) >> d_w_2",
                  "MCParticles.m_pdg>0 && " + mc)
        self.d_w.Add(self.d_w_2)
        self.set_descr_shifter(self.d_w, "Comparison on w (=0.00449/pt) of a track between CDC 2D finder output and MC.",
                               "A clear peak at 0 with tail.")
        self.set_style(self.d_w, "#Deltaw", "Events/(0.08)")

        #: validation histogram
        self.d_phi = TH1F("d_phi", "#Delta#phi of CDC 2D finder", 50, -0.5, 0.5)
        tree.Draw(f"{trk2d_phi}-atan({mc_py}/{mc_px})>>d_phi",
                  "MCParticles.m_status==11&&abs(MCParticles.m_pdg)==11",
                  f"fabs({trk2d_phi}-atan({mc_py}/{mc_px}))<{PI}&&" + mc)

        #: validation histogram
        self.d_phi_2 = TH1F("d_phi_2", "d_phi_2", 50, -0.5, 0.5)
        tree.Draw(f"{trk2d_phi}-atan({mc_py}/{mc_px})-{PI}>>d_phi_2",
                  "MCParticles.m_status==11&&abs(MCParticles.m_pdg)==11",
                  f"{trk2d_phi}-atan({mc_py}/{mc_px})>={PI} &&" + mc)

        #: validation histogram
        self.d_phi_3 = TH1F("d_phi_3", "d_phi_3", 50, -0.5, 0.5)
        tree.Draw(f"{trk2d_phi}-atan({mc_py}/{mc_px})+{PI}>>d_phi_2",
                  "MCParticles.m_status==11&&abs(MCParticles.m_pdg)==11",
                  f"{trk2d_phi}-atan({mc_py}/{mc_px})<=-{PI}&&" + mc)

        self.d_phi.Add(self.d_phi_2)
        self.d_phi.Add(self.d_phi_3)
        self.set_descr_shifter(self.d_phi, "Comparison on phi_i of a track between CDC 2D finder output and MC.",
                               "A Gaussian peak at 0.")
        self.set_style(self.d_phi, "#Delta#phi [rad]", "Events/(0.02 rad)")

        #: validation histogram
        self.d_z0_3d = TH1F("d_z0_3d", "#Deltaz0 of CDC 3D fitter", 60, -30, 30)
        tree.Draw("TRGCDC3DFitterTracks.m_z0-MCParticles.m_productionVertex_z>>d_z0_3d", mc)
        self.set_descr_shifter(self.d_z0_3d, "Comparison on z0 of a track between CDC 2D fitter output and MC.",
                               "A Gaussian peak at 0 with small tail.")
        self.set_style(self.d_z0_3d, "#Deltaz0 [cm]", "Events/(1 cm)")

        #: validation histogram
        self.d_z0_nn = TH1F("d_z0_nn", "#Deltaz0 of CDC Neuro", 60, -30, 30)
        tree.Draw("TRGCDCNeuroTracks.m_z0-MCParticles.m_productionVertex_z>>d_z0_nn", mc)
        self.set_descr_shifter(self.d_z0_nn, "Comparison on z0 of a track between CDC Neuro output and MC.",
                               "A Gaussian peak at 0 with small tail.")
        self.set_style(self.d_z0_nn, "#Deltaz0 [cm]", "Events/(1 cm)")

        #: validation histogram
        self.d_E_ECL = TH1F("d_E_ECL", "#DeltaE of ECL clustering", 100, -6, 6)
        tree.Draw("TRGECLClusters.m_edep-MCParticles.m_energy>>d_E_ECL", mc)
        self.set_descr_shifter(self.d_E_ECL, "Comparison on deposit energy between ECL cluster output and MC.",
                               "A peak around -0.5 ~ 0 with a tail toward -6.")
        self.set_style(self.d_E_ECL, "#DeltaE [GeV]", "Events/(0.12 GeV)")

    def beginRun(self):
        '''
        reset all histograms at the begin of a new run
        '''

        self.hist_inbit.Reset()
        self.hist_outbit.Reset()
        self.hist_inbit_expert.Reset()
        self.hist_outbit_expert.Reset()
        self.h_Esum_ECL.Reset()
        self.h_E_ECL.Reset()
        self.h_theta_ECL.Reset()
        self.h_thetaID_ECL.Reset()
        self.h_phi_ECL.Reset()

    def event(self):
        '''
        event loop
        '''

        trg_summary = Belle2.PyStoreObj("TRGSummary")
        clusters = Belle2.PyStoreArray("TRGECLClusters")
        klmSummary = Belle2.PyStoreObj("KLMTrgSummary")

        for bit in moniInbits:
            binIndex = inputBits.index(bit["name"])
            bitIndex = bit["index"]
            if trg_summary.testInput(bitIndex):
                self.hist_inbit.Fill(binIndex + 0.5)
        for bit in moniOutbits:
            binIndex = outputBits.index(bit["name"])
            bitIndex = bit["index"]
            if trg_summary.testFtdl(bitIndex):
                self.hist_outbit.Fill(binIndex + 0.5)

        for bit in moniInbits_expert:
            binIndex = inputBits_expert.index(bit["name"])
            bitIndex = bit["index"]
            if trg_summary.testInput(bitIndex):
                self.hist_inbit_expert.Fill(binIndex + 0.5)
        for bit in moniOutbits_expert:
            binIndex = outputBits_expert.index(bit["name"])
            bitIndex = bit["index"]
            if trg_summary.testFtdl(bitIndex):
                self.hist_outbit_expert.Fill(binIndex + 0.5)

        etot = 0
        for cluster in clusters:
            x = cluster.getPositionX()
            y = cluster.getPositionY()
            z = cluster.getPositionZ()
            e = cluster.getEnergyDep()
            self.h_E_ECL.Fill(e)
            etot += e
            vec = ROOT.Math.XYZVector(x, y, z)
            self.h_theta_ECL.Fill(vec.Theta() * TMath.RadToDeg())
            self.h_thetaID_ECL.Fill(cluster.getMaxTCId())
            self.h_phi_ECL.Fill(vec.Phi() * TMath.RadToDeg())

        if etot > 0:
            self.h_Esum_ECL.Fill(etot)

        self.h_sector_BKLM.Fill(klmSummary.getBKLM_n_trg_sectors())
        self.h_sector_EKLM.Fill(klmSummary.getEKLM_n_trg_sectors())

        self.Nevent = self.Nevent + 1

    def endRun(self):
        '''
        end of run
        '''

        print("end")

    def terminate(self):
        '''
        write histograms
        '''

        bits = ['ty_0', 'ehigh', 'clst_0']
        part = 'e'
        h_gen_p = self.tfile.Get(f"{part}_all/p")
        h_gen_E = self.tfile.Get(f"{part}_all/clusterE")
        h_gen_theta = self.tfile.Get(f"{part}_all/theta")
        h_gen_phi = self.tfile.Get(f"{part}_all/phi")

        for bit in bits:
            h_p = self.tfile.Get(f"{part}_{bit}/p")
            h_E = self.tfile.Get(f"{part}_{bit}/clusterE")
            h_theta = self.tfile.Get(f"{part}_{bit}/theta")
            h_phi = self.tfile.Get(f"{part}_{bit}/phi")

            h_eff_p = self.get_relative(h_p, h_gen_p, 'p', part, bit)
            self.set_style(h_eff_p, "Momentum", "Efficiency")
            self.set_descr_expert(h_eff_p, "Momentum dependent efficiency for experts", "")

            h_eff_E = self.get_relative(h_E, h_gen_E, 'E', part, bit)
            self.set_style(h_eff_E, "Energy", "Efficiency")
            if bit == 'ehigh':
                self.set_descr_shifter(h_eff_E, "Energy dependent efficiency for shifter",
                                       "Efficiency around 40% below 1 GeV, then jump up to 100%")
            elif bit == 'clst_0':
                self.set_descr_shifter(h_eff_E, "Energy dependent efficiency for shifter",
                                       "Turn-on curve from 40% efficiency at 0.5 GeV to nearly 100% above 1.5 GeV")
            else:
                self.set_descr_expert(h_eff_E, "Energy dependent efficiency for experts", "")

            h_eff_theta = self.get_relative(h_theta, h_gen_theta, 'theta', part, bit)
            self.set_style(h_eff_theta, "Polar angle", "Efficiency")
            self.set_descr_expert(h_eff_theta, "polar angle dependent efficiency for experts", "")

            h_eff_phi = self.get_relative(h_phi, h_gen_phi, 'phi', part, bit)
            self.set_style(h_eff_phi, "phi angle", "Efficiency")
            self.set_descr_expert(h_eff_phi, "azimuth angle dependent efficiency for experts", "")

            h_eff_p.Write()
            h_eff_E.Write()
            h_eff_theta.Write()
            h_eff_phi.Write()

        bits = ['ty_0', 'klm_0', "klm_hit"]
        part = "mu"
        h_gen_p = self.tfile.Get(f"{part}_all/p")
        h_gen_E = self.tfile.Get(f"{part}_all/clusterE")
        h_gen_theta = self.tfile.Get(f"{part}_all/theta")
        h_gen_phi = self.tfile.Get(f"{part}_all/phi")

        for bit in bits:
            h_p = self.tfile.Get(f"{part}_{bit}/p")
            h_theta = self.tfile.Get(f"{part}_{bit}/theta")
            h_phi = self.tfile.Get(f"{part}_{bit}/phi")

            h_eff_p = self.get_relative(h_p, h_gen_p, 'p', part, bit)
            self.set_style(h_eff_p, "Momentum", "Efficiency")
            if bit == 'ty_0':
                self.set_descr_shifter(h_eff_p, "Momentum dependent efficiency for shifter",
                                       "Efficiency should be above 90% for the whole momentum range")
            elif bit == 'klm_0':
                self.set_descr_shifter(h_eff_p, "Momentum dependent efficiency for shifter",
                                       "Efficiency should rise to about 90% for momenta greater than 1 GeV")
            else:
                self.set_descr_expert(h_eff_p, "Momentum dependent efficiency for experts", "")

            h_eff_theta = self.get_relative(h_theta, h_gen_theta, 'theta', part, bit)
            self.set_style(h_eff_theta, "Polar angle", "Efficiency")
            self.set_descr_expert(h_eff_theta, "polar angle dependent efficiency for experts", "")

            h_eff_phi = self.get_relative(h_phi, h_gen_phi, 'phi', part, bit)
            self.set_style(h_eff_phi, "phi angle", "Efficiency")
            self.set_descr_expert(h_eff_phi, "azimuth angle dependent efficiency for experts", "")

            h_eff_p.Write()
            h_eff_theta.Write()
            h_eff_phi.Write()

        self.hist_inbit.Scale(1./self.Nevent)
        self.hist_outbit.Scale(1.0/self.Nevent)
        self.hist_inbit_expert.Scale(1./self.Nevent)
        self.hist_outbit_expert.Scale(1.0/self.Nevent)

        self.hist_inbit.Write()
        self.hist_outbit.Write()
        self.hist_inbit_expert.Write()
        self.hist_outbit_expert.Write()
        self.h_Esum_ECL.Write()
        self.h_E_ECL.Write()
        self.h_theta_ECL.Write()
        self.h_thetaID_ECL.Write()
        self.h_phi_ECL.Write()
        self.h_sector_BKLM.Write()
        self.h_sector_EKLM.Write()
        self.d_w.Write()
        self.d_phi.Write()
        self.d_z0_3d.Write()
        self.d_z0_nn.Write()
        self.d_E_ECL.Write()
        self.tfile.Close()

#####################################################


mypath = basf2.Path()  # create a new path

# add input data and ParticleLoader modules to the path
ma.inputMdst("../TRGValidationGen.root", path=mypath)


stdc.stdMu(listtype='all', path=mypath)
stdc.stdE(listtype='all', path=mypath)

ma.matchMCTruth('mu+:all', path=mypath)
ma.matchMCTruth('e+:all', path=mypath)

ma.cutAndCopyList('mu+:true', 'mu+:all', cut='isSignal>0 and mcPrimary>0', path=mypath)
ma.cutAndCopyList('e+:true', 'e+:all', cut='isSignal>0 and mcPrimary>0', path=mypath)


pvars = ["p", "theta", "phi"]
dvars = vu.create_aliases(pvars, 'daughter(0,{variable})', "emu")

bits = ['ty_0', 'ehigh', 'clst_0', 'klm_0', "klm_hit"]
varlists = []
for bit in bits:
    inb_cut = f"L1Input({bit})>0"
    ma.cutAndCopyList(f'mu+:{bit}', 'mu+:true', cut=inb_cut, path=mypath)
    ma.cutAndCopyList(f'e+:{bit}', 'e+:true', cut=inb_cut, path=mypath)

    ma.variablesToHistogram(decayString=f'mu+:{bit}',
                            variables=[('p', 50, 0.5, 3.0),
                                       ('clusterE', 50, 0.5, 3.0),
                                       ('theta', 40, 0.5, 2.5),
                                       ('phi', 40, -3.141593, 3.141593)],
                            filename='TRGValidation.root',
                            directory=f'mu_{bit}',
                            path=mypath
                            )

    ma.variablesToHistogram(decayString=f'e+:{bit}',
                            variables=[('p', 50, 0.5, 3.0),
                                       ('clusterE', 50, 0.5, 3.0),
                                       ('theta', 40, 0.5, 2.5),
                                       ('phi', 40, -3.141593, 3.141593)],
                            filename='TRGValidation.root',
                            directory=f'e_{bit}',
                            path=mypath
                            )


ma.variablesToHistogram(decayString='mu+:true',
                        variables=[('p', 50, 0.5, 3.0),
                                   ('clusterE', 50, 0.5, 3.0),
                                   ('theta', 40, 0.5, 2.5),
                                   ('phi', 40, -3.141593, 3.141593)],
                        filename='TRGValidation.root',
                        directory='mu_all',
                        path=mypath
                        )

ma.variablesToHistogram(decayString='e+:true',
                        variables=[('p', 50, 0.5, 3.0),
                                   ('clusterE', 50, 0.5, 3.0),
                                   ('theta', 40, 0.5, 2.5),
                                   ('phi', 40, -3.141593, 3.141593)],
                        filename='TRGValidation.root',
                        directory='e_all',
                        path=mypath
                        )


# process the data
basf2.process(mypath)

# INput
main = basf2.create_path()
root_input = basf2.register_module("RootInput")
root_input.param("inputFileName", "../TRGValidationGen.root")
main.add_module(root_input)
main.add_module(MakePlots())
main.add_module('Progress')
basf2.process(main)
