#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

#############################################################
# This steering file generates, simulates, and reconstructs
# a sample of 10 BBbar events with bg overlay. The output are
# histograms of VTX hit occupancy per layer and sensor.
#
# Usage: basf2 vtx_occupancy.py
#############################################################

from simulation import add_simulation
from L1trigger import add_tsim
from reconstruction import add_reconstruction, add_cdst_output, add_mc_reconstruction
from mdst import add_mdst_output
import ROOT
from ROOT import Belle2

import basf2 as b2
import numpy as np


# Need to use default global tag prepended with upgrade GT
from vtx import get_upgrade_globaltag, get_upgrade_background_files
b2.conditions.disable_globaltag_replay()
b2.conditions.prepend_globaltag(get_upgrade_globaltag())


def getID(VxdId):
    return "{layer}.{ladder}.{sensor}".format(
        layer=VxdId.getLayerNumber(),
        ladder=VxdId.getLadderNumber(),
        sensor=VxdId.getSensorNumber())


class VTXOccupancy(b2.Module):
    """
    Compute and plot the vtx occupancy for all sensors and layers
    """

    def initialize(self, outputpath="VTXOccupancy.root"):
        """
        Create ROOT TProfiles for all layers.
        """

        # let's create a root file to store all profiles
        #: Output file to store all plots
        self.rfile = ROOT.TFile(outputpath, "RECREATE")
        self.rfile.cd()
        #: dictionary sensor occupancy
        self.histos = {}
        #: disctionary sensor shape
        self.shape = {}
        #: dictionar of layers
        self.layers = {}
        #: name template
        self.histo_name = "Occupancy_sensor{id}"
        #: title template
        self.histo_title = "Occupancy, sensor={id}"

    def beginRun(self):

        geo = Belle2.VXD.GeoCache.getInstance()
        for layer in geo.getLayers(3):
            self.layers[layer.getLayerNumber()] = []
            for ladder in geo.getLadders(layer):
                for sensorID in geo.getSensors(ladder):
                    self.layers[layer.getLayerNumber()].append(getID(sensorID))
                    info = Belle2.VXD.GeoCache.get(sensorID)
                    self.shape[getID(sensorID)] = float(info.getUCells()*info.getVCells())
                    print(
                        'Get ucells={:f} and vcells={:f} and prod={:f}'.format(
                            info.getUCells(),
                            info.getVCells(),
                            info.getUCells() *
                            info.getVCells()))
                    name = self.histo_name.format(id=getID(sensorID))
                    title = self.histo_title.format(id=getID(sensorID))
                    self.histos[getID(sensorID)] = ROOT.TH1D(name, title, 100000, 0, 1)

    def event(self):
        """
        Update the occupancy histos by iterating over all digits
        """

        # Reset the counters
        counter = {k: 0.0 for k, v in self.histos.items()}

        digits = Belle2.PyStoreArray("VTXDigits")
        for digit in digits:
            sensorID = digit.getSensorID()
            counter[getID(sensorID)] += 100.0/self.shape[getID(sensorID)]

        for sensorID, histo in self.histos.items():
            histo.Fill(counter[sensorID])

    def terminate(self):
        """
        Format all histos and write the ROOT file.
        """

        for sensorID, histo in self.histos.items():
            mean = histo.GetMean()
            print("Sensor={sensorID} has occupancy={occ}".format(sensorID=sensorID, occ=mean))

        hlayers = ROOT.TH1D("hlayers", "Mean occupancy", 7, 1, 8)
        hlayers.GetXaxis().SetTitle("Layer number")
        hlayers.GetYaxis().SetTitle("Occupancy [%]")
        layer_means = []
        for layer, sensors in self.layers.items():
            sensor_means = []
            for sensorID in sensors:
                mean = self.histos[sensorID].GetMean()
                sensor_means.append(mean)

            layer_mean = np.mean(np.array(sensor_means))
            layer_means.append(layer_mean)
            print("Layer {layer} has mean occupancy={occ}%".format(layer=layer, occ=layer_mean))
            hlayers.SetBinContent(layer, layer_mean)

        self.rfile.Write()
        self.rfile.Close()


# create path
main = b2.create_path()

# specify number of events to be generated
main.add_module('EventInfoSetter', evtNumList=[10])

# main.add_module('HistoManager', histoFileName="VTX_DQM_Histos.root")

# print event numbers
main.add_module('EventInfoPrinter')

# generate BBbar events
main.add_module('EvtGenInput')

# detector simulation
add_simulation(main, bkgfiles=get_upgrade_background_files(), useVTX=True)

# trigger simulation
add_tsim(main)

# reconstruction
add_mc_reconstruction(main, pruneTracks=False, useVTX=True)

main.add_module(VTXOccupancy())

# main.add_module('VTXEventPlot')

# vtxdqmExpReco = register_module('VTXDQMExpressReco')
# main.add_module(vtxdqmExpReco)

# process events and print call statistics
b2.process(main)
print(b2.statistics)
