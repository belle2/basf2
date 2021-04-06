#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""\
This module contains classes for plotting calibration constants.
Author: qingyuan.liu@desy.de
"""

__all__ = ["ConditionCheckerBase", "PXDMaskedPixelsChecker", "PXDDeadPixelsChecker", "PXDOccupancyInfoChecker"]

from abc import ABC, abstractmethod
from ROOT import Belle2
import ROOT
import numpy as np
from pxd.utils import get_sensor_graphs, get_sensor_maps, sensorID_list
from pxd.utils import latex_r, nPixels, nVCells, nUCells

# import basf2

# lookup dictionary for finding a checker based on objType
__checker_dict__ = {
    Belle2.PXDMaskedPixelPar: "PXDMaskedPixelsChecker",
    Belle2.PXDDeadPixelPar: "PXDDeadPixelsChecker",
    Belle2.PXDOccupancyInfoPar: "PXDOccupancyInfoChecker",
}

# plot categories and styles for PXDHotPixelMasking calibration
type_list = ['hot', 'dead', 'hot_dead', 'occ_no_mask', 'occ_masked']
# label_list = ['Hot', 'Dead', 'Hot/Dead', 'Occupancy (No Mask)', 'Occupancy (With Mask)']
label_list = ['Hot pixels', 'Dead pixels', 'Hot/Dead pixels', 'No Mask', 'With Mask']
color_list = [ROOT.kRed + 1, ROOT.kAzure - 5, ROOT.kGray + 2, ROOT.kOrange + 1, ROOT.kSpring + 5]
max_list = [4., 4., 20., 3., 3.]
plot_type_dict = {
    _type: {'label': _label, 'color': _color, 'max': _max}
    for (_type, _label, _color, _max) in zip(type_list, label_list, color_list, max_list)
}
# marker_style = 20


# condition db object (payload) checkers
class ConditionCheckerBase(ABC):
    """Abstract base class describing interfaces to make plots from condition db."""

    def __init__(self, name, objType, tfile, rundir="", use_hist=False):
        """
        """
        super().__init__()
        #: DBObj name
        self.name = name
        #: DBObj type
        self._objType = objType
        #: DBObjPtr for the condition
        self.dbObj = Belle2.PyDBObj(self.name, self.objType.Class())
        #: pointer to the event metadata
        self.eventMetaData = Belle2.PyStoreObj("EventMetaData")
        #: TFile for saving plots
        self.tfile = tfile
        #: Directory for writing histograms of each run
        self.rundir = rundir  # empty means skipping self.hists
        if self.rundir != "":
            self.tfile.cd()
            self.tfile.mkdir(self.rundir)
        #: Dictionary of plots (TGraph) summarizing variables vs run
        self.graphs = {}
        #: Dictionary of plots (TH1) to be saved for each run
        self.hists = {}
        #: Hist title suffix
        self.hist_title_suffix = ""
        #: The 1st run
        self.runstart = 999999  # a big number that will never be used
        #: The last run
        self.runend = -1
        #: The 1st exp
        self.expstart = 999999
        #: Flag to get TGraph values from a histogram (TH2F)
        self.use_hist = use_hist

        self.define_graphs()

    @property
    def objType(self):
        """
        DBObj type (read only)
        """
        return self._objType

    @property
    def run(self):
        return self.eventMetaData.getRun()

    @property
    def exp(self):
        return self.eventMetaData.getExperiment()

    def beginRun(self):
        """
        Call functions at beginning of a run
        """
        self.define_hists()
        self.fill_plots()

    def define_graphs(self, ytitle=""):
        """
        Method to define TGraph
        Parameters:
          ytitle (str): Label for TGraph y-axis
        """
        self.tfile.cd()
        self.graphs.update(get_sensor_graphs(ytitle))

    def define_hists(self, name=None, title=None, ztitle=None):
        """
        Method to define TH2
        Parameters:
          name (str): name for TH2, to which sensor name will be attached
          title (str): title for TH2
          ztitle (str): title for z-axis (color bar)
        """
        if self.rundir:
            self.tfile.cd(self.rundir)
            self.hists.update(get_sensor_maps(name, title, ztitle, self.run))

    @abstractmethod
    def get_db_content(self):
        """
        Abstract method to get content of a payload
        Should return a Dictionary with sensorID.getID() as key and relaated calibration results as value
        """

    @abstractmethod
    def get_graph_value(self, sensor_db_content):
        """
        Abstract method to get a value for each TGraph
        Parameters:
          sensor_db_content (Any): Calibration results of a module
        """

    def get_graph_value_from_hist(self, h2=None):
        """
        Method to get a value for each TGraph
        Parameters:
          h2 (TH2): If not none, get value from h2
        """
        return None

    def set_hist_content(self, h2, sensor_db_content):
        """
        Method to set TH2 bins
        Parameters:
          h2 (TH2): TH2F object for handling values of a pixel matrix
          sensor_db_content (Any): Calibration results of a module
        """
        pass

    def fill_plots(self):
        """
        Method to fill plot objects
        """
        if self.exp < self.expstart:
            self.expstart = self.exp
        if self.run < self.runstart:
            self.runstart = self.run
        if self.run > self.runend:
            self.runend = self.run

        db_content = self.get_db_content()
        values_dict = {}
        pre_values_dict = {}
        for sensorID in sensorID_list:
            sensor_db_content = db_content[sensorID.getID()]
            if self.graphs:
                if not self.use_hist:
                    values_dict[sensorID.getID()] = self.get_graph_value(sensor_db_content)
                gr = self.graphs[sensorID.getID()]
                pre_values_dict[sensorID.getID()] = gr.GetPointY(gr.GetN() - 1)
            # Saving graph points only for different values (sensor-wise)
                # value = self.get_graph_value(sensor_db_content)
                # pre_value = gr.GetPointY(gr.GetN()-1)
                # if value != pre_value:
                #     gr.SetPoint(gr.GetN(), self.run, value)
            # Saving masked map
            if self.rundir != "" and self.hists:
                h2 = self.hists[sensorID.getID()]
                self.set_hist_content(h2, sensor_db_content)
                # Update title with total count
                h2.SetTitle(h2.GetTitle() + self.hist_title_suffix)
                self.tfile.cd(self.rundir)
                h2.Write()
                if self.use_hist:
                    values_dict[sensorID.getID()] = self.get_graph_value_from_hist(h2)
        # Saving graph points only for different conditions
        if values_dict != pre_values_dict:
            for sensorID in sensorID_list:
                gr = self.graphs[sensorID.getID()]
                gr.SetPoint(gr.GetN(), self.run, values_dict[sensorID.getID()])

    def draw_plots(self, canvas=None, cname="", ymin=0., ymax=None):
        """
        Method to draw plots on a TCanvas
        Parameters:
          canvas (TCanvas): ROOT TCanvas for plotting
          canme (str): Name of the canvas
          ymin (float): minimum value of y-axis for plotting
          ymax (float): maximum of y-axis for plotting
        """
        if not canvas:
            return
        else:
            canvas.Clear()
            canvas.SetWindowSize(1000, 400)
            canvas.SetLeftMargin(0.09)
            canvas.SetRightMargin(0.05)
        self.tfile.cd()
        for i, sensorID in enumerate(sensorID_list):
            sensor_id = sensorID.getID()
            if i == 0:
                if ymax:
                    self.graphs[sensor_id].SetMaximum(ymax)
                self.graphs[sensor_id].SetMinimum(ymin)
                self.graphs[sensor_id].Draw("AP")
            else:
                self.graphs[sensor_id].Draw("P")
        if self.graphs["TLegends"]:
            for leg in self.graphs["TLegends"]:
                leg.Draw()
        self.save_canvas(canvas, cname)

    def save_canvas(self, canvas, cname, with_logy=False):
        if cname:
            canvas.SetName(cname)
        exp_run = f"e{self.expstart:05} r{self.runstart:04}-{self.runend:04}"
        # Draw Belle II label
        latex_r.DrawLatex(0.95, 0.92, "Belle II Experiment: " + exp_run)
        # Print and write TCanvas
        canvas.SetLogy(0)
        canvas.Print(f"{exp_run}_{cname}_vs_run.png")
        canvas.Print(f"{exp_run}_{cname}_vs_run.pdf")
        if with_logy:
            canvas.SetLogy(1)
            canvas.Update()
            canvas.Modified()
            canvas.Print(f"{exp_run}_{cname}_vs_run_logy.png")
            canvas.Print(f"{exp_run}_{cname}_vs_run_logy.pdf")
        self.tfile.cd()
        canvas.Write()


class PXDOccupancyInfoChecker(ConditionCheckerBase):
    """
    Checker for PXDOccupancyInfoPar
    """

    def __init__(self, name, tfile, rundir=""):
        """
        """
        super().__init__(name, Belle2.PXDOccupancyInfoPar, tfile, "")

    def define_graphs(self):
        super().define_graphs(ytitle="Occupancy (With Mask) [%]")

    def get_db_content(self):
        content_dic = {}
        for sensorID in sensorID_list:
            numID = sensorID.getID()
            raw_occ = self.dbObj.getOccupancy(numID)
            # The default value is -1. It seems 0 will not be updated into the payload.
            content_dic[numID] = raw_occ if raw_occ >= 0 else 0

        return content_dic

    def get_graph_value(self, sensor_db_content):
        return sensor_db_content * 100

    def draw_plots(self, canvas=None):
        # We don't use raw occupanncy. They will be corrected after takig out dead pixels
        pass
        # super().draw_plots(canvas=canvas, cname="OccupancyWithMask", ymin=0., ymax=plot_type_dict["occ_masked"]["max"])


class PXDMaskedPixelsChecker(ConditionCheckerBase):
    """
    Checker for PXDMaskedPixelPar
    """

    def __init__(self, name, tfile, rundir="maps"):
        """
        """
        super().__init__(name, Belle2.PXDMaskedPixelPar, tfile, rundir)

    def define_graphs(self):
        super().define_graphs(ytitle="Hot pixels [%]")

    def define_hists(self):
        super().define_hists(name="MaskedPixels", title="Masked Pixels", ztitle="isMasked")

    def get_db_content(self):
        return self.dbObj.getMaskedPixelMap()

    def get_graph_value(self, sensor_db_content):
        hotcount = len(sensor_db_content)
        self.hist_title_suffix = f" ({hotcount} pixels)"
        return hotcount * 100. / nPixels

    def set_hist_content(self, h2, sensor_db_content):
        # loop over all masked pixels
        for pixelID in sensor_db_content:
            uCell = int(pixelID / nVCells)
            vCell = pixelID % nVCells
            h2.SetBinContent(int(uCell + 1), int(vCell + 1), 1)

    def draw_plots(self, canvas=None, cname="PXDHotPixel", ymin=0., ymax=plot_type_dict["hot"]["max"]):
        super().draw_plots(canvas=canvas, cname=cname, ymin=ymin, ymax=ymax)


class PXDDeadPixelsChecker(ConditionCheckerBase):
    """
    Checker for PXDDeadPixelPar
    """

    def __init__(self, name, tfile, rundir="maps", use_hist=True):
        """
        """
        super().__init__(name, Belle2.PXDDeadPixelPar, tfile, rundir)

    def define_graphs(self):
        super().define_graphs(ytitle="Dead pixels [%]")

    def define_hists(self):
        super().define_hists(name="DeadPixels", title="Dead Pixels", ztitle="isDead")

    def get_db_content(self):
        deadsensormap = self.dbObj.getDeadSensorMap()
        deaddrainmap = self.dbObj.getDeadDrainMap()
        deadrowmap = self.dbObj.getDeadRowMap()
        deadsinglesmap = self.dbObj.getDeadSinglePixelMap()

        content_dic = {}
        for sensorID in sensorID_list:
            numID = sensorID.getID()
            content_dic[numID] = {}
            if numID in deadsensormap:
                content_dic[numID]["deadsensor"] = True
            else:
                content_dic[numID]["deadsensor"] = False
                content_dic[numID]["deaddrains"] = deaddrainmap[numID]
                content_dic[numID]["deadrows"] = deadrowmap[numID]
                content_dic[numID]["deadsingles"] = deadsinglesmap[numID]
        return content_dic

    def get_graph_value(self, sensor_db_content):
        deadcount = 0
        if sensor_db_content["deadsensor"]:
            deadcount = nPixels
        else:
            n_deaddrains = len(sensor_db_content["deaddrains"])
            n_deadrows = len(sensor_db_content["deadrows"])
            if n_deaddrains == 0 or n_deadrows == 0:
                # Every dead drain counts for 192 dead pixels
                deadcount = n_deaddrains * 192
                # Every dead row counts for 250 dead pixels
                # This can lead to double counting of dead pixel from dead drains
                # The double counting can be avoided by using TH2.Integral().
                deadcount += n_deadrows * 250
                # Every dead single pixels
                deadcount += len(sensor_db_content["deadsingles"])
            else:  # Using a histogram to avoid double counting
                # Just create a temporary TH2
                h2 = list(get_sensor_maps(sensorID_list=sensorID_list[0:1]).values())[0]
                self.set_hist_content(h2, sensor_db_content)
                deadcount = h2.Integral()
        self.hist_title_suffix = f" ({deadcount} pixels)"
        return min(deadcount, nPixels) * 100. / nPixels

    def get_graph_value_from_hist(self, h2):
        return h2.Integral() * 100. / nPixels

    def set_hist_content(self, h2, sensor_db_content):
        # loop over all dead pixels
        if sensor_db_content["deadsensor"]:
            ones = np.ones(nPixels)
            h2.SetContent(ones)
        else:
            for drainID in sensor_db_content["deaddrains"]:
                for iGate in range(192):
                    uCell = drainID / 4
                    vCell = drainID % 4 + iGate * 4
                    h2.SetBinContent(int(uCell + 1), int(vCell + 1), 1)

            for vCell in sensor_db_content["deadrows"]:
                for uCell in range(nUCells):
                    h2.SetBinContent(int(uCell + 1), int(vCell + 1), 1)

            for pixelID in sensor_db_content["deadsingles"]:
                uCell = int(pixelID / nVCells)
                vCell = pixelID % nVCells
                h2.SetBinContent(int(uCell + 1), int(vCell + 1), 1)

    def draw_plots(self, canvas=None, cname="PXDDeadPixel", ymin=0., ymax=plot_type_dict["dead"]["max"]):
        super().draw_plots(canvas=canvas, cname=cname, ymin=ymin, ymax=ymax)
