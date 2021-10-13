#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""\
This module contains classes for plotting calibration constants.
"""

from abc import ABC, abstractmethod
from ROOT import Belle2
from ROOT.Belle2 import PXDMaskedPixelPar, PXDDeadPixelPar, PXDOccupancyInfoPar
import ROOT
from copy import deepcopy

from pxd.utils import get_sensor_graphs, sensorID_list
# from pxd.utils import nPixels, nVCells, nUCells
from pxd.calibration.condition_checker import plot_type_dict

# import basf2

# lookup dictrionary for finding a checker based on objType
__cal_checker_dict__ = {
    "PXDHotPixelMaskCalibrationChecker": set([PXDMaskedPixelPar, PXDDeadPixelPar, PXDOccupancyInfoPar]),
}


# Calibration checkers for summarizing info of multiple payloads
class CalibrationCheckerBase(ABC):
    """
    Abstract base class for generating plots from calibrations which save multiple payloads.
    """

    def __init__(self, objType_list=[]):
        """
        Parameters:
          objType_list (list): a list of db objects used as keys to their checker.
            e.g., [Belle2.PXDMaskedPixelPar, Belle2.PXDDeadPixelPar]
        """
        #: condition checkers which are assoicated to this calibration.
        self.condition_checkers = {}
        for objType in objType_list:
            self.condition_checkers[objType] = None

    def initialize(self, checker_list=[]):
        """
        Initialize condition_checker according to condition objType
        Parameters:
          checker_list (list): list of objType checkers
        """
        checker = None
        for checker in checker_list:
            for objType in self.condition_checkers.keys():
                if objType == checker.objType:
                    self.condition_checkers[objType] = checker
        if self.valid:
            checker.tfile.cd()
            print(f"root file path: {checker.tfile}")
            self.define_graphs()

    @property
    def valid(self):
        """
        valid flag
        """
        if None in self.condition_checkers.values():
            # do nothing if any of the condition checkers is missing
            return False
        else:
            return True

    @abstractmethod
    def define_graphs(self):
        """
        Define summary graphs
        """

    @abstractmethod
    def fill_graphs(self):
        """
        Method to fill TGraphs
        """

    @abstractmethod
    def draw_plots(self):
        """
        Generate summary plots from TGraphs of all condition checkers
        """

    def beginRun(self):
        """
        function to be executed at the beginning of a run
        """
        if self.valid:
            self.fill_graphs()

    def terminate(self):
        """
        Execute when terminating a basf2 module.
        All required TGraphs should be ready at this stage.
        """
        if self.valid:
            self.draw_plots()


class PXDHotPixelMaskCalibrationChecker(CalibrationCheckerBase):
    """
    Checker for PXDHotPixelMaskCalibration
    """

    def __init__(self):
        """
        Initial with related condition db object types
        """
        super().__init__(objType_list=[
            Belle2.PXDMaskedPixelPar,
            Belle2.PXDDeadPixelPar,
            Belle2.PXDOccupancyInfoPar,
        ])

    def define_graphs(self):
        """
        Define TGraphs for filling
        """
        #: Dictionary to save TGraphs of hot/dead pixel fractions with sensor id as key
        self.hotdead_graphs = get_sensor_graphs("Hot/Dead pixels [%]")
        #: Dictionary to save TGraph of module occupancy (with mask)
        self.occupancy_masked_graphs = get_sensor_graphs("Occupancy (With Mask) [%]")
        #: Dictionary to save TGraph of module occupancy (without mask)
        self.occupancy_no_mask_graphs = get_sensor_graphs("Occupancy (No Mask) [%]")
        #: Dictionary of summary TGraphs and their TLegend
        self.sum_graphs = {}
        # TLegend
        legsum = ROOT.TLegend(0.75, 0.65, 0.90, 0.85)
        legsum.SetFillStyle(0)
        legsum.SetBorderSize(0)
        legsum.SetTextFont(43)
        legsum.SetTextSize(18)
        legocc = deepcopy(legsum)
        self.sum_graphs["TLegends"] = {"sum": legsum, "occ": legocc}

        for category, style_dict in plot_type_dict.items():
            agraph = ROOT.TGraph()
            agraph.SetLineColor(style_dict["color"])
            agraph.SetMarkerColor(style_dict["color"])
            agraph.SetMarkerStyle(20)
            if category == "hot_dead":
                agraph.GetYaxis().SetTitle('Total Hot/Dead pixels [%]')
                agraph.SetMaximum(style_dict["max"])
            elif category == "occ_no_mask":
                agraph.GetYaxis().SetTitle('Average occupancy [%]')
                agraph.SetMaximum(style_dict["max"])
            agraph.GetXaxis().SetTitle('run #')
            agraph.GetYaxis().SetTitleOffset(0.9)
            agraph.SetMinimum(0.0)
            self.sum_graphs[category] = agraph

    def fill_graphs(self):
        """
        Fill TGraphs, will be executed for each run
        """
        dead_checker = self.condition_checkers[PXDDeadPixelPar]
        masked_graphs = self.condition_checkers[PXDMaskedPixelPar].graphs
        dead_graphs = self.condition_checkers[PXDDeadPixelPar].graphs
        occ_graphs = self.condition_checkers[PXDOccupancyInfoPar].graphs

        current_run = dead_checker.run
        saved_run = -1  # to be updated
        saved_run_occ = -1  # to be updated
        sum_hotfraction = 0.
        sum_deadfraction = 0.
        sum_hotdeadfraction = 0.
        sum_occupancymasked = 0.
        sum_occupancynomask = 0.
        for sensorID in sensorID_list:
            numID = sensorID.getID()
            # This function is executed just after filling the condition checker graphs, so we don't need a python loop!
            g1 = masked_graphs[numID]
            g2 = dead_graphs[numID]
            g3 = occ_graphs[numID]
            # If two runs have the same constants, only results from the 1st run will be saved.
            hotfraction = g1.GetPointY(g1.GetN() - 1)
            deadfraction = g2.GetPointY(g2.GetN() - 1)
            occupancymasked = g3.GetPointY(g3.GetN() - 1)
            occupancynomask = 0.
            # Masked hot pixels and dead pixels could be updated for different runs.
            saved_run = max(g1.GetPointX(g1.GetN() - 1), g2.GetPointX(g2.GetN() - 1))
            saved_run_occ = max(saved_run, g3.GetPointX(g3.GetN() - 1))
            if current_run == saved_run:
                self.hotdead_graphs[numID].append(current_run, hotfraction + deadfraction)
                sum_hotfraction += hotfraction
                sum_deadfraction += deadfraction
                sum_hotdeadfraction += hotfraction + deadfraction
            if current_run == saved_run_occ:
                # occupancy correction (it's overestiamting occ_masked?)
                # By defaut, both occ and fractions are in percentage.
                if deadfraction != 100.:
                    occupancynomask = (occupancymasked + hotfraction) / (1. - deadfraction / 100)
                    occupancymasked = occupancymasked / (1. - deadfraction / 100)
                self.occupancy_no_mask_graphs[numID].append(current_run, occupancynomask)
                self.occupancy_masked_graphs[numID].append(current_run, occupancymasked)
                sum_occupancymasked += occupancymasked
                sum_occupancynomask += occupancynomask
            # print(f"hotfraction:{hotfraction}, deadfraction: {deadfraction}")
            # print(f"occ:{occupancymasked}, occ_no_mask:{occupancynomask}")
            # rawOcc = self.condition_checkers[PXDOccupancyInfoPar].dbObj.getOccupancy(numID)
            # print(f"Raw occ: {rawOcc}")
            # print(f"sum_hotfraction:{sum_hotfraction}, sum_deadfraction: {sum_deadfraction}")
            # print(f"sum_occ:{sum_occupancymasked}, sum_occ_no_mask:{sum_occupancynomask}")

        nSensors = len(sensorID_list)
        if current_run == saved_run:
            self.sum_graphs["hot"].append(current_run, sum_hotfraction / nSensors)
            self.sum_graphs["dead"].append(current_run, sum_deadfraction / nSensors)
            self.sum_graphs["hot_dead"].append(current_run, sum_hotdeadfraction / nSensors)

        nSensors = nSensors - len(dead_checker.dbObj.getDeadSensorMap())
        if current_run == saved_run_occ:
            self.sum_graphs["occ_masked"].append(current_run, sum_occupancymasked / nSensors)
            self.sum_graphs["occ_no_mask"].append(current_run, sum_occupancynomask / nSensors)

    def draw_plots(self):
        """
        Generate summary plots from TGraphs of all condition checkers
        """

        # Reuse masker checker and replace its graphs
        # checker = deepcopy(self.condition_checkers[PXDMaskedPixelPar])
        # checker.tfile = self.condition_checkers[PXDMaskedPixelPar].tfile
        checker = self.condition_checkers[PXDMaskedPixelPar]

        # Create a TCanvas under the file folder
        checker.tfile.cd()
        canvas = ROOT.TCanvas('c_pxd', 'c_pxd', 1000, 400)
        # canvas.SetLeftMargin(0.09)
        # canvas.SetRightMargin(0.05)

        # Hot/dead pixel masking
        checker.graphs = self.hotdead_graphs
        checker.draw_plots(
            canvas=canvas, cname="PXDHotnDeadPixel", ymin=0., ymax=plot_type_dict["hot_dead"]["max"])
        checker.draw_plots(canvas=canvas, cname="PXDHotnDeadPixel", ymin=0., ymax=None, logy=True)
        canvas.Write()
        # Occupancy with/without Mask
        checker.graphs = self.occupancy_masked_graphs
        checker.draw_plots(
            canvas=canvas, cname="PXDOccupancyWithMask", ymin=0., ymax=plot_type_dict["occ_masked"]["max"])
        checker.draw_plots(canvas=canvas, cname="PXDOccupancyWithMask", ymin=0., ymax=None, logy=True)
        canvas.Write()
        checker.graphs = self.occupancy_no_mask_graphs
        checker.draw_plots(
            canvas=canvas, cname="PXDOccupancyNoMask", ymin=0., ymax=plot_type_dict["occ_no_mask"]["max"])
        checker.draw_plots(canvas=canvas, cname="PXDOccupancyNoMask", ymin=0., ymax=None, logy=True)
        canvas.Write()

        # checker.save_canvas(canvas, "PXDHotnDeadPixel")

        # Summary plots
        #   Total Masking plots
        canvas.Clear()
        g1 = self.sum_graphs["hot_dead"]
        g2 = self.sum_graphs["hot"]
        g3 = self.sum_graphs["dead"]
        leg = self.sum_graphs["TLegends"]["sum"]
        g1.Draw("AP")
        g2.Draw("P same")
        g3.Draw("P same")
        leg.AddEntry(g1, 'Hot+Dead pixels', 'p')
        leg.AddEntry(g2, 'Hot pixels', 'p')
        leg.AddEntry(g3, 'Dead pixels', 'p')
        leg.Draw()
        checker.save_canvas(canvas, "PXDTotalHotnDeadPixel", logy=True)
        canvas.Write()
        checker.save_canvas(canvas, "PXDTotalHotnDeadPixel", logy=False)
        #   Average Occupancy plots
        canvas.Clear()
        g1 = self.sum_graphs["occ_no_mask"]
        g2 = self.sum_graphs["occ_masked"]
        leg = self.sum_graphs["TLegends"]["occ"]
        g1.Draw("AP")
        g2.Draw("P same")
        leg.AddEntry(g1, 'No Mask', 'p')
        leg.AddEntry(g2, 'With Mask', 'p')
        leg.Draw()
        checker.save_canvas(canvas, "PXDTotalOccupancy", logy=True)
        canvas.Write()
        checker.save_canvas(canvas, "PXDTotalOccupancy", logy=False)
