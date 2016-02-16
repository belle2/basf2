import ROOT
import sys


def plotHitAnalysis(label=""):
    f = ROOT.TFile.Open(sys.argv[1], "OPEN")
    c = ROOT.TCanvas()

    scrods = f.Get("scrods")
    scrods.Draw()
    c.Print("%s_scrods.png" % label)

    laser_occupancy = f.Get("laser_occupancy")
    laser_occupancy.Draw()
    c.Print("%s_laseroccupancy.png" % label)

    asic_occupancy0 = f.Get("asic_occupancy0")
    asic_occupancy0.Draw()
    c.Print("%s_asicoccupancy0.png" % label)
    asic_occupancy1 = f.Get("asic_occupancy1")
    asic_occupancy1.Draw()
    c.Print("%s_asicoccupancy1.png" % label)
    asic_occupancy2 = f.Get("asic_occupancy2")
    asic_occupancy2.Draw()
    c.Print("%s_asicoccupancy2.png" % label)
    asic_occupancy3 = f.Get("asic_occupancy3")
    asic_occupancy3.Draw()
    c.Print("%s_asicoccupancy3.png" % label)

    ROOT.gStyle.SetPalette(56)
    ROOT.gPad.SetLogz()
    ch_widths = f.Get("ch_widths")
    ch_widths.Draw("colz")
    ch_widths.GetYaxis().SetRangeUser(0, 10)
    ch_widths.SetStats(0)
    c.Print("%s_widths_pixel.png" % label)

    width_time = f.Get("width_time")
    width_time.Draw("colz")
    width_time.GetYaxis().SetRangeUser(0, 10)
    width_time.SetStats(0)
    c.Print("%s_widths_time.png" % label)

    ch_time = f.Get("ch_time")
    ch_time.Draw("colz")
    ch_time.GetYaxis().SetRangeUser(-600, 100)
    ch_time.SetStats(0)
    c.Print("%s_times_pixel.png" % label)

    ch_tdcbin = f.Get("ch_tdcbin")
    ch_tdcbin.Draw("colz")
    ch_tdcbin.GetYaxis().SetRangeUser(0, 1200)
    ch_tdcbin.SetStats(0)
    c.Print("%s_tdcbin_pixel.png" % label)

    ch_heights = f.Get("ch_heights")
    ch_heights.Draw("colz")
    ch_heights.SetStats(0)
    c.Print("%s_heights_pixel.png" % label)

    pmtxy = f.Get("pmtxy")
    pmtxy.Draw("colz")
    pmtxy.SetStats(0)
    c.Print("%s_pmtxy.png" % label)

    f.Close()

if __name__ == '__main__':
    if len(sys.argv) > 2:
        plotHitAnalysis(sys.argv[2])
    else:
        plotHitAnalysis()
