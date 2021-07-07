##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################
from ROOT import TFile, TH1F, TH2F
from ROOT import gROOT, Double, TCut, gROOT
import math

gROOT.SetBatch()


def make_pdf(histofile, pdfName='plots.pdf'):

    if histofile is None:
        return

    import ROOT

    c1 = ROOT.TCanvas("c1", "", 10, 10, 1100, 700)
    c1.SetRightMargin(0.2)
    c1.Print(pdfName+"(", "pdf")

    for key in histofile.GetListOfKeys():

        cl = gROOT.GetClass(key.GetClassName())
        if cl.InheritsFrom("TH1"):

            print("Printing histo " + key.GetName())

            h1 = key.ReadObj().Clone()
            c1.Clear()
            c1.cd()
            c1.SetName(key.GetName())
            c1.SetTitle(key.GetTitle())

            if cl.InheritsFrom("TH2"):
                h1.Draw("colz")
            else:
                h1.Draw()

            ROOT.gPad.Modified()
            ROOT.gPad.Update()
            c1.Print(pdfName+"(", "pdf")

        elif cl.InheritsFrom("TGraph"):

            print("Printing graph " + key.GetName())

            g1 = key.ReadObj().Clone()
            c1.Clear()
            c1.cd()
            c1.SetName(key.GetName())
            c1.SetTitle(key.GetTitle())
            g1.Draw()

            ROOT.gPad.Modified()
            ROOT.gPad.Update()
            c1.Print(pdfName+"(", "pdf")

    c1.Print(pdfName+")", "pdf")


def overlay_pdf(histofile1, histofile2, pdfName='plots.pdf'):

    if histofile1 is None:
        return
    if histofile2 is None:
        return

    import ROOT

    c1 = ROOT.TCanvas("c1", "", 10, 10, 1100, 700)
    c1.SetRightMargin(0.2)
    c1.Print(pdfName+"(", "pdf")

    for key1 in histofile1.GetListOfKeys():

        if(histofile2.GetKey(key1.GetName())):
            key2 = histofile2.GetKey(key1.GetName())
        else:
            continue

        cl = gROOT.GetClass(key1.GetClassName())
        if cl.InheritsFrom("TH1"):

            print("Printing histo " + key1.GetName())

            h1 = key1.ReadObj().Clone()
            h2 = key2.ReadObj().Clone()
            c1.Clear()
            c1.cd()
            c1.SetName(key1.GetName())
            c1.SetTitle(key1.GetTitle())

            if cl.InheritsFrom("TH2"):

                legend = ROOT.TLegend(.60, .8, .75, .9)

                if key1.GetName() in ["h2cs_super", "h2_super_efficiencymap"]:
                    # Subtract the 2 histo
                    h3 = h1.Clone()
                    h3.Add(h1, h2, 1.0, -1.0)
                    h3.Draw("colz")
                    legend.AddEntry(h3, "VTX - monopix1", "")
                else:
                    legend.AddEntry(h1, "VTX", "")
                    h1.Draw("colz")
                legend.Draw("SAME")

            else:
                legend = ROOT.TLegend(.60, .8, .75, .9)
                legend.AddEntry(h1, "VTX", "l")
                legend.AddEntry(h2, "Monopix1", "l")

                h2.SetLineColor(2)

                if key1.GetName() not in ["hpoint_resolution_u", "hpoint_resolution_v"]:

                    norm1 = h1.GetEntries()
                    norm2 = h2.GetEntries()

                    h1.Scale(1/norm1)
                    h2.Scale(1/norm2)
                    if h1.GetMaximum() > h2.GetMaximum():
                        h1.Draw("HIST")
                        h2.Draw("SAME HIST")
                    else:
                        h2.Draw("HIST")
                        h1.Draw("SAME HIST")

                else:
                    # ymin = min(h1.GetMinimum(), h2.GetMinimum())
                    # ymax = max(h1.GetMaximum(), h2.GetMaximum())
                    ymin = 4
                    ymax = 18
                    h1.SetAxisRange(ymin-1, ymax+1, "Y")
                    h1.Draw()
                    h2.Draw("SAME")

                legend.Draw("SAME")

            ROOT.gPad.Modified()
            ROOT.gPad.Update()
            c1.Print(pdfName+"(", "pdf")

        elif cl.InheritsFrom("TGraph"):

            print("Printing graph " + key1.GetName())

            g1 = key1.ReadObj().Clone()
            g2 = key2.ReadObj().Clone()
            c1.Clear()
            c1.cd()
            c1.SetName(key1.GetName())
            c1.SetTitle(key1.GetTitle())

            legend = ROOT.TLegend(.60, .8, .75, .9)
            legend.AddEntry(h1, "VTX", "l")
            legend.AddEntry(h2, "Monopix1", "l")

            g2.SetLineColor(2)
            g1.Draw()
            g2.Draw("SAME")
            legend.Draw("SAME")

            ROOT.gPad.Modified()
            ROOT.gPad.Update()
            c1.Print(pdfName+"(", "pdf")

    c1.Print(pdfName+")", "pdf")


def plot(inputfile=None, histofile=None, basecut="hasTrack==0", Config=None):

    if inputfile is None or histofile is None:
        return

    if Config is None:
        print('Missing Config objects for plotting')
        return

    # Analysis cuts
    hitcut = TCut(basecut)

    # Get access to tracks
    tracktree = inputfile.Get("Track")

    nvcells = Config['vcell_axis'][0]
    minvcell = Config['vcell_axis'][1]
    maxvcell = Config['vcell_axis'][2]
    nucells = Config['ucell_axis'][0]
    minucell = Config['ucell_axis'][1]
    maxucell = Config['ucell_axis'][2]

    htrackspot = TH2F("htrackspot", "htrackspot", nvcells, minvcell, maxvcell, nucells, minucell, maxucell)
    tracktree.Draw("cellU_fit:cellV_fit>>+htrackspot", "", "goff")
    htrackspot.SetTitle("Track intersections on DUT matrix")
    htrackspot.GetXaxis().SetTitle("cellV_{fit} [cell ID]")
    htrackspot.GetYaxis().SetTitle("cellU_{fit} [cell ID]")
    htrackspot.GetZaxis().SetTitle("number of tracks")
    htrackspot.SetStats(0)
    htrackspot.Write()

    nu = Config['sensor_u_axis'][0]
    minu = Config['sensor_u_axis'][1]
    maxu = Config['sensor_u_axis'][2]
    nv = Config['sensor_v_axis'][0]
    minv = Config['sensor_v_axis'][1]
    maxv = Config['sensor_v_axis'][2]

    hbeamspot = TH2F("hbeamspot", "hbeamspot", nu, minu, maxu, nv, minv, maxv)
    tracktree.Draw("u_fit:v_fit>>+hbeamspot", "", "goff")
    hbeamspot.SetTitle("Track intersections on DUT plane")
    hbeamspot.GetXaxis().SetTitle("v_{fit} [mm]")
    hbeamspot.GetYaxis().SetTitle("u_{fit} [mm]")
    hbeamspot.GetZaxis().SetTitle("number of tracks")
    hbeamspot.SetStats(0)
    hbeamspot.Write()

    # Get access to hits
    hittree = inputfile.Get("Hit")

    hspot = TH2F("hspot", "", nvcells, minvcell, maxvcell, nucells, minucell, maxucell)
    hittree.Draw("cellU_hit:cellV_hit>>hspot", hitcut, "goff")
    hspot.SetTitle("DUT clusters on matrix")
    hspot.GetXaxis().SetTitle("cellV_{hit} [cell ID]")
    hspot.GetYaxis().SetTitle("cellU_{hit} [cell ID]")
    hspot.GetZaxis().SetTitle("number of clusters")
    hspot.SetStats(0)
    hspot.Write()

    def make_residual_histo(nbins, xmin, xmax, size_cut='sizeU>0', axis='u', label='all'):
        """ Helper function for residual making residual histo."""
        histo = TH1F("hres_{:s}_{:s}".format(axis, label), "", nbins, xmin, xmax)
        hittree.Draw("({:s}_hit - {:s}_fit)*1000 >>+hres_{:s}_{:s}".format(axis, axis, axis, label), hitcut+TCut(size_cut), "goff")
        histo.SetTitle("Unbiased DUT residuals {:s}_{{hit}}-{:s}_{{fit}} for {:s} clusters".format(axis, axis, label))
        histo.GetXaxis().SetTitle("{:s}_{{hit}} - {:s}_{{fit}} [#mum]".format(axis, axis))
        histo.GetYaxis().SetTitle("number of clusters")
        histo.GetYaxis().SetTitleOffset(1.2)
        histo.Write()
        return histo

    def make_resolution_histo(residual_histos, tel_sigma=0, axis='u'):
        """ Helper function for making resolution histo."""
        histo = TH1F("hpoint_resolution_{:s}".format(axis), "", len(residual_histos), 0, len(residual_histos))
        histo.SetTitle("DUT pointing resolution {:s} overview".format(axis))
        histo.GetXaxis().SetTitle("")
        histo.GetYaxis().SetTitle("DUT pointing resolution #sigma_{:s} [#mum]".format(axis))
        histo.GetYaxis().SetTitleOffset(1.2)
        histo.SetStats(0)

        bin = 1
        for key, residual_histo in sorted(residual_histos.items(), key=lambda kv: kv[0][-1]):
            histo.GetXaxis().SetBinLabel(bin, key)
            if residual_histo.GetRMS()**2 - tel_sigma**2 > 0:
                dut_sigma = math.sqrt(residual_histo.GetRMS()**2 - tel_sigma**2)
                dut_sigma_sigma = residual_histo.GetRMSError()
                histo.SetBinContent(bin, dut_sigma)
                histo.SetBinError(bin, dut_sigma_sigma)
            bin += 1

        histo.Write()
        return histo

    nbinsu = Config['residual_u_axis'][0]
    minu = 1000*Config['residual_u_axis'][1]
    maxu = 1000*Config['residual_u_axis'][2]

    hfit_sigma_u = TH1F("hfit_sigma_u", "hfit_sigma_u", 200, 0, maxu)
    hittree.Draw("u_fiterr*1000 >>+hfit_sigma_u", hitcut, "goff")
    hfit_sigma_u.SetTitle("Track intersection uncertainty #sigma_{u}")
    hfit_sigma_u.GetXaxis().SetTitle("#sigma_{u_{fit}} [#mum]")
    hfit_sigma_u.GetYaxis().SetTitle("number of tracks")
    hfit_sigma_u.GetYaxis().SetTitleOffset(1.2)
    hfit_sigma_u.Write()

    residual_histos_u = {}
    residual_histos_u['sizeU>0'] = make_residual_histo(nbinsu, minu, maxu, size_cut='sizeU>0', axis='u', label='all')
    residual_histos_u['sizeU==1'] = make_residual_histo(nbinsu, minu, maxu, size_cut='sizeU==1', axis='u', label='sizeU==1')
    residual_histos_u['sizeU==2'] = make_residual_histo(nbinsu, minu, maxu, size_cut='sizeU==2', axis='u', label='sizeU==2')
    # residual_histos_u['sizeU==3'] = make_residual_histo(nbinsu, minu, maxu, size_cut='sizeU==3', axis='u', label='sizeU==3')

    make_resolution_histo(residual_histos_u, tel_sigma=hfit_sigma_u.GetMean(), axis='u')

    nbinsv = Config['residual_v_axis'][0]
    minv = 1000*Config['residual_v_axis'][1]
    maxv = 1000*Config['residual_v_axis'][2]

    hfit_sigma_v = TH1F("hfit_sigma_v", "hfit_sigma_v", 200, 0, maxv)
    hittree.Draw("v_fiterr*1000 >>+hfit_sigma_v", hitcut, "goff")
    hfit_sigma_v.SetTitle("Track intersection uncertainty #sigma_{v}")
    hfit_sigma_v.GetXaxis().SetTitle("#sigma_{v_{fit}} [#mum]")
    hfit_sigma_v.GetYaxis().SetTitle("number of tracks")
    hfit_sigma_v.GetYaxis().SetTitleOffset(1.2)
    hfit_sigma_v.Write()

    residual_histos_v = {}
    residual_histos_v['sizeV>0'] = make_residual_histo(nbinsv, minv, maxv, size_cut='sizeV>0', axis='v', label='all')
    residual_histos_v['sizeV==1'] = make_residual_histo(nbinsv, minv, maxv, size_cut='sizeV==1', axis='v', label='sizeV==1')
    residual_histos_v['sizeV==2'] = make_residual_histo(nbinsv, minv, maxv, size_cut='sizeV==2', axis='v', label='sizeV==2')
    # residual_histos_v['sizeV==3'] = make_residual_histo(nbinsv, minv, maxv, size_cut='sizeV==3', axis='v', label='sizeV==3')

    make_resolution_histo(residual_histos_v, tel_sigma=hfit_sigma_v.GetMean(), axis='v')

    hres_uv = TH2F("hres_uv", "", nbinsu, minu, maxu, nbinsv, minv, maxv)
    hittree.Draw("(u_hit - u_fit)*1000:(v_hit - v_fit)*1000 >>+hres_uv", hitcut, "goff")
    hres_uv.SetTitle("Unbiased 2D DUT residuals")
    hres_uv.GetXaxis().SetTitle("v_{hit} - v_{fit} [#mum]")
    hres_uv.GetYaxis().SetTitle("u_{hit} - u_{fit} [#mum]")
    hres_uv.GetZaxis().SetTitle("number of hits")
    hres_uv.SetStats(0)
    hres_uv.Write()

    # hchisqundof = TH1F("hchisqundof","",100,0,0)
    # hittree.Draw("trackChi2 / trackNdof >> +hchisqundof", hitcut,"goff")
    # hchisqundof.SetTitle("#chi^{2}/ndof")
    # hchisqundof.GetXaxis().SetTitle("#chi^{2}/ndof")
    # hchisqundof.GetYaxis().SetTitle("number of tracks")
    # hchisqundof.GetYaxis().SetTitleOffset(1.2)
    # hchisqundof.Write()

    nseed = Config['seed_charge_axis'][0]
    seedmin = Config['seed_charge_axis'][1]
    seedmax = Config['seed_charge_axis'][2]

    hCharge = TH1F("hCharge", "", nseed, seedmin, seedmax)
    hittree.Draw("seedCharge >>+hCharge", hitcut, "goff")
    hCharge.SetTitle("Seed Charge")
    hCharge.GetXaxis().SetTitle("seed charge [{}]".format(Config['charge_unit']))
    hCharge.GetYaxis().SetTitle("number of hits")
    hCharge.GetYaxis().SetTitleOffset(1.2)
    hCharge.Write()

    nclu = Config['clus_charge_axis'][0]
    clumin = Config['clus_charge_axis'][1]
    clumax = Config['clus_charge_axis'][2]

    hCCharge = TH1F("hCCharge", "", nclu, clumin, clumax)
    hittree.Draw("clusterCharge >>+hCCharge", hitcut, "goff")
    hCCharge.SetTitle("Cluster Charge")
    hCCharge.GetXaxis().SetTitle("cluster charge [{}]".format(Config['charge_unit']))
    hCCharge.GetYaxis().SetTitle("number of hits")
    hCCharge.GetYaxis().SetTitleOffset(1.2)
    hCCharge.Write()

    hsize = TH1F("hsize", "", 10, 0, 10)
    hittree.Draw("size >>+hsize", hitcut, "goff")
    hsize.SetTitle("Cluster size")
    hsize.GetXaxis().SetTitle("cluster size [pixels]")
    hsize.GetYaxis().SetTitle("number of hits")
    hsize.GetYaxis().SetTitleOffset(1.2)
    hsize.Write()

    hsizeU = TH1F("hsizeU", "", 10, 0, 10)
    hittree.Draw("sizeU >>+hsizeU", hitcut, "goff")
    hsizeU.SetTitle("Cluster size projection")
    hsizeU.GetXaxis().SetTitle("sizeU [cell ID]")
    hsizeU.GetYaxis().SetTitle("number of hits")
    hsizeU.GetYaxis().SetTitleOffset(1.2)
    hsizeU.Write()

    hsizeV = TH1F("hsizeV", "", 10, 0, 10)
    hittree.Draw("sizeV >>+hsizeV", hitcut, "goff")
    hsizeV.SetTitle("Cluster size projection")
    hsizeV.GetXaxis().SetTitle("sizeV [cell ID]")
    hsizeV.GetYaxis().SetTitle("number of hits")
    hsizeV.GetYaxis().SetTitleOffset(1.2)
    hsizeV.Write()
