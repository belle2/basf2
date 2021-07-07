##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################
from ROOT import TFile, TH1F, TH2F
from ROOT import gROOT, Double, TCut
import numpy
import math


def plot(inputfile=None, histofile=None, uaxis=(10, 0, 0), vaxis=(10, 0, 0)):

    if inputfile is None or histofile is None:
        return None

    # Get access to hits
    tree = inputfile.Get("Hit")

    ubins = uaxis[0]
    vbins = vaxis[0]

    counter = numpy.zeros(shape=(ubins, vbins))
    h2_cs = [[TH1F("hcs_%d_%d" % (iv, iu), "", 255, 0, 255) for iv in range(vbins)] for iu in range(ubins)]
    h2_ss = [[TH1F("hss_%d_%d" % (iv, iu), "", 255, 0, 255) for iv in range(vbins)] for iu in range(ubins)]
    h2_uu = [[TH1F("huu_%d_%d" % (iv, iu), "", 20, 0, 20) for iv in range(vbins)] for iu in range(ubins)]
    h2_vv = [[TH1F("hvv_%d_%d" % (iv, iu), "", 20, 0, 20) for iv in range(vbins)] for iu in range(ubins)]
    h2_px = [[TH1F("hpx_%d_%d" % (iv, iu), "", 20, 0, 20) for iv in range(vbins)] for iu in range(ubins)]

    histofile.cd("")

    # Final histos
    h2c = TH2F("h2c", "h2c", uaxis[0], uaxis[1], uaxis[2], vaxis[0], vaxis[1], vaxis[2])
    h2cs = TH2F("h2cs", "h2cs", uaxis[0], uaxis[1], uaxis[2], vaxis[0], vaxis[1], vaxis[2])
    h2ss = TH2F("h2ss", "h2ss", uaxis[0], uaxis[1], uaxis[2], vaxis[0], vaxis[1], vaxis[2])
    h2uu = TH2F("h2uu", "h2uu", uaxis[0], uaxis[1], uaxis[2], vaxis[0], vaxis[1], vaxis[2])
    h2vv = TH2F("h2vv", "h2vv", uaxis[0], uaxis[1], uaxis[2], vaxis[0], vaxis[1], vaxis[2])
    h2px = TH2F("h2px", "h2px", uaxis[0], uaxis[1], uaxis[2], vaxis[0], vaxis[1], vaxis[2])

    for event in tree:
        if event.hasTrack == 0:
            m_u = event.u_fit
            m_v = event.v_fit

            if m_u < uaxis[1]:
                continue
            if m_u > uaxis[2]:
                continue
            if m_v < vaxis[1]:
                continue
            if m_v > vaxis[2]:
                continue

            # Note: This must be c-style index
            iu = h2cs.GetXaxis().FindBin(m_u)-1
            iv = h2cs.GetYaxis().FindBin(m_v)-1

            # Fill maps
            counter[iu, iv] += 1
            h2_cs[iu][iv].Fill(event.clusterCharge)
            h2_ss[iu][iv].Fill(event.seedCharge)
            h2_uu[iu][iv].Fill(event.sizeU)
            h2_vv[iu][iv].Fill(event.sizeV)
            h2_px[iu][iv].Fill(event.size)

    # fill 2d histos

    for iu in range(ubins):
        for iv in range(vbins):
            mean_cs = h2_cs[iu][iv].GetMean()
            mean_ss = h2_ss[iu][iv].GetMean()
            mean_uu = h2_uu[iu][iv].GetMean()
            mean_vv = h2_vv[iu][iv].GetMean()
            mean_px = h2_px[iu][iv].GetMean()

            h2cs.SetBinContent(iu+1, iv+1, mean_cs)
            h2ss.SetBinContent(iu+1, iv+1, mean_ss)
            h2c.SetBinContent(iu+1, iv+1, (counter[iu, iv]))
            h2uu.SetBinContent(iu+1, iv+1, (mean_uu))
            h2vv.SetBinContent(iu+1, iv+1, (mean_vv))
            h2px.SetBinContent(iu+1, iv+1, (mean_px))

    # plot histograms

    h2c.SetTitle("Track intersection map")
    h2c.GetXaxis().SetTitle("u_fit [mm]")
    h2c.GetYaxis().SetTitle("v_fit [mm]")
    h2c.GetZaxis().SetTitle("Number of tracks")
    h2c.SetStats(0)
    h2c.Write()

    h2ss.SetTitle("Seed signal map")
    h2ss.GetXaxis().SetTitle("u_fit [mm]")
    h2ss.GetYaxis().SetTitle("v_fit [mm]")
    h2ss.GetZaxis().SetTitle("Mean Seed Signal [LSB]")
    h2ss.SetStats(0)
    h2ss.Write()

    h2cs.SetTitle("Cluster signal map")
    h2cs.GetXaxis().SetTitle("u_fit [mm]")
    h2cs.GetYaxis().SetTitle("v_fit [mm]")
    h2cs.GetZaxis().SetTitle("Mean Cluster Signal [LSB]")
    h2cs.SetStats(0)
    h2cs.Write()

    h2uu.SetTitle("Cluster size u projection map")
    h2uu.GetXaxis().SetTitle("u_fit [mm]")
    h2uu.GetYaxis().SetTitle("v_fit [mm]")
    h2uu.GetZaxis().SetTitle("Mean sizeU [cells]")
    h2uu.SetStats(0)
    h2uu.Write()

    h2vv.SetTitle("Cluster size v projection map")
    h2vv.GetXaxis().SetTitle("u_fit [mm]")
    h2vv.GetYaxis().SetTitle("v_fit [mm]")
    h2vv.GetZaxis().SetTitle("Mean sizeV [cells]")
    h2vv.SetStats(0)
    h2vv.Write()

    h2px.SetTitle("Cluster size map")
    h2px.GetXaxis().SetTitle("u_fit [mm]")
    h2px.GetYaxis().SetTitle("v_fit [mm]")
    h2px.GetZaxis().SetTitle("Mean size [cells]")
    h2px.SetStats(0)
    h2px.Write()


def plot_superpixel(inputfile=None, histofile=None, pixeltype=0, upitch=0.0, vpitch=0.0, ubins=10, vbins=10, ufold=2, vfold=2):

    if inputfile is None or histofile is None:
        return None

    # Get access to hits
    tree = inputfile.Get("Hit")

    # Super pixel contains upix x vpix cells
    upix = ufold
    vpix = vfold

    counter = numpy.zeros(shape=(ubins, vbins))

    h2_super_cs = [[TH1F("hcs_super_%d_%d" % (iv, iu), "", 400, 0, 400) for iv in range(vbins)] for iu in range(ubins)]
    h2_super_ss = [[TH1F("hss_super_%d_%d" % (iv, iu), "", 400, 0, 400) for iv in range(vbins)] for iu in range(ubins)]
    h2_super_uu = [[TH1F("huu_super_%d_%d" % (iv, iu), "", 10, 0, 10) for iv in range(vbins)] for iu in range(ubins)]
    h2_super_vv = [[TH1F("hvv_super_%d_%d" % (iv, iu), "", 10, 0, 10) for iv in range(vbins)] for iu in range(ubins)]
    h2_super_px = [[TH1F("hpx_super_%d_%d" % (iv, iu), "", 10, 0, 10) for iv in range(vbins)] for iu in range(ubins)]

    histofile.cd("")

    for event in tree:
        # if event.hasTrack == 0 and event.pixeltype == pixeltype:

        m_u = (event.u_fit - event.cellUCenter_fit)
        m_v = (event.v_fit - event.cellVCenter_fit)

        m_u += upitch/2
        m_v += vpitch/2

        if m_u < 0:
            continue
        if m_u >= upitch:
            continue
        if m_v < 0:
            continue
        if m_v >= vpitch:
            continue

        # Pitch of inpixel bins
        subpitchU = (upix*upitch)/ubins
        subpitchV = (vpix*vpitch)/vbins

        # Calculate inpixel bins
        if event.cellU_fit < 0:
            continue
        if event.cellV_fit < 0:
            continue

        m_u += (event.cellU_fit % upix) * upitch
        m_v += (event.cellV_fit % vpix) * vpitch

        iu = int(m_u / subpitchU)
        iv = int(m_v / subpitchV)

        # Fill maps
        counter[iu, iv] += 1
        h2_super_cs[iu][iv].Fill(event.clusterCharge)
        h2_super_ss[iu][iv].Fill(event.seedCharge)
        h2_super_uu[iu][iv].Fill(event.sizeU)
        h2_super_vv[iu][iv].Fill(event.sizeV)
        h2_super_px[iu][iv].Fill(event.size)

    h2c_super = TH2F("h2c_super", "h2c_super", ubins, 0, upix*upitch, vbins, 0, vpix*vpitch)
    h2cs_super = TH2F("h2cs_super", "h2cs_super", ubins, 0, upix*upitch, vbins, 0, vpix*vpitch)
    h2ss_super = TH2F("h2ss_super", "h2ss_super", ubins, 0, upix*upitch, vbins, 0, vpix*vpitch)
    h2uu_super = TH2F("h2uu_super", "h2uu_super", ubins, 0, upix*upitch, vbins, 0, vpix*vpitch)
    h2vv_super = TH2F("h2vv_super", "h2vv_super", ubins, 0, upix*upitch, vbins, 0, vpix*vpitch)
    h2px_super = TH2F("h2px_super", "h2px_super", ubins, 0, upix*upitch, vbins, 0, vpix*vpitch)

    # fill 2d histos

    for iu in range(ubins):
        for iv in range(vbins):
            mean_cs = h2_super_cs[iu][iv].GetMean()
            mean_ss = h2_super_ss[iu][iv].GetMean()
            mean_uu = h2_super_uu[iu][iv].GetMean()
            mean_vv = h2_super_vv[iu][iv].GetMean()
            mean_px = h2_super_px[iu][iv].GetMean()

            h2cs_super.SetBinContent(iu+1, iv+1, mean_cs)
            h2ss_super.SetBinContent(iu+1, iv+1, mean_ss)
            h2c_super.SetBinContent(iu+1, iv+1, (counter[iu, iv]))
            h2uu_super.SetBinContent(iu+1, iv+1, mean_uu)
            h2vv_super.SetBinContent(iu+1, iv+1, mean_vv)
            h2px_super.SetBinContent(iu+1, iv+1, mean_px)

    # plot histograms

    h2c_super.SetTitle("SuperPixel track intersection map")
    h2c_super.GetXaxis().SetTitle("u_{m} [mm]")
    h2c_super.GetYaxis().SetTitle("v_{m} [mm]")
    h2c_super.GetZaxis().SetTitle("Number of tracks")
    h2c_super.SetStats(0)
    h2c_super.Write()

    h2ss_super.SetTitle("SuperPixel seed signal map")
    h2ss_super.GetXaxis().SetTitle("u_{m} [mm]")
    h2ss_super.GetYaxis().SetTitle("v_{m} [mm]")
    h2ss_super.GetZaxis().SetTitle("Mean Seed Signal [LSB]")
    h2ss_super.SetStats(0)
    h2ss_super.Write()

    h2cs_super.SetTitle("SuperPixel cluster signal map")
    h2cs_super.GetXaxis().SetTitle("u_{m} [mm]")
    h2cs_super.GetYaxis().SetTitle("v_{m} [mm]")
    h2cs_super.GetZaxis().SetTitle("Mean Cluster Signal [LSB]")
    h2cs_super.SetStats(0)
    h2cs_super.Write()

    h2ss_super_x = h2ss_super.ProjectionX("h2ss_super_x")
    h2ss_super_x.SetTitle("SuperPixel seed signal u projection")
    h2ss_super_x.GetXaxis().SetTitle("u_{m} [mm]")
    h2ss_super_x.GetYaxis().SetTitle("Mean Seed Signal [LSB]")
    h2ss_super_x.SetStats(0)

    h2ss_super_y = h2ss_super.ProjectionY("h2ss_super_y")
    h2ss_super_y.SetTitle("SuperPixel seed signal v projection")
    h2ss_super_y.GetXaxis().SetTitle("v_{m} [mm]")
    h2ss_super_y.GetYaxis().SetTitle("Mean Seed Signal [LSB]")
    h2ss_super_y.SetStats(0)

    h2uu_super.SetTitle("SuperPixel cluster size u projection map")
    h2uu_super.GetXaxis().SetTitle("u_{m} [mm]")
    h2uu_super.GetYaxis().SetTitle("v_{m} [mm]")
    h2uu_super.GetZaxis().SetTitle("Mean sizeU [uCells]")
    h2uu_super.SetStats(0)
    h2uu_super.Write()

    h2vv_super.SetTitle("SuperPixel cluster size v projection map")
    h2vv_super.GetXaxis().SetTitle("u_{m} [mm]")
    h2vv_super.GetYaxis().SetTitle("v_{m} [mm]")
    h2vv_super.GetZaxis().SetTitle("Mean sizeV [vCells]")
    h2vv_super.SetStats(0)
    h2vv_super.Write()

    h2px_super.SetTitle("SuperPixel cluster size map")
    h2px_super.GetXaxis().SetTitle("u_{m} [mm]")
    h2px_super.GetYaxis().SetTitle("v_{m} [mm]")
    h2px_super.GetZaxis().SetTitle("Mean size [Pixel]")
    h2px_super.SetStats(0)
    h2px_super.Write()
