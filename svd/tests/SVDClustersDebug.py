#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import basf2
from basf2 import *
from generators import add_evtgen_generator
from modularAnalysis import setupEventInfo
from simulation import add_simulation
from svd import *
import ROOT
from ROOT import Belle2, TH1F, TH2F, TFile
from ROOT.Belle2 import SVDNoiseCalibrations
import os.path
import sys

##################################################################################
#
# Phase3 SVD COMMISSIONING
#
# this script check Clusterizer
#
##################################################################################

# GLOBAL_TAG = "svd_commissioning_20180711" #for commissioning data

fileIN = "given by the user"  # is the -i option is used
fileOUT = "given by the user"  # if the -o option is used
cluster3 = "SVDClustersZS3"
reco3 = "SVDRecoDigitsZS3"
shaper3 = "SVDShaperDigitsZS3"
cluster5 = "SVDClustersZS5"
reco5 = "SVDRecoDigitsZS5"
shaper5 = "SVDShaperDigitsZS5"

if(len(sys.argv) == 1 + 1):
    fileOUT = sys.argv[1]


class SVDClustersDebug(basf2.Module):

    def initialize(self):

        self.test = []

        geoCache = Belle2.VXD.GeoCache.getInstance()

        self.fb2u = TH2F("nCl_L4L5S2U", "4.5.2 U ZS3 vs ZS5 clusters", 100, 0, 100, 100, 0, 100)

    def event(self):

        evt = Belle2.PyStoreObj("EventMetaData")

        clustersZS3 = Belle2.PyStoreArray(cluster3)
        clustersZS5 = Belle2.PyStoreArray(cluster5)

        nCl3 = 0
        nCl5 = 0

        for d in clustersZS5:
            if(str(d.getSensorID()) == "4.5.2"):
                if(d.isUCluster()):
                    nCl5 = nCl5 + 1

        for d in clustersZS3:
            if(str(d.getSensorID()) == "4.5.2"):
                if(d.isUCluster()):
                    nCl3 = nCl3 + 1

        self.fb2u.Fill(nCl3, nCl5)

        noisePayload = Belle2.SVDNoiseCalibrations()
#        print(type(noisePayload))

        if nCl3 != nCl5:
            print("different number of clusters in event " + str(evt.getEvent()) +
                  "! ZS3 = " + str(nCl3) + ", ZS5 = " + str(nCl5) + "    ")

            # for d in clustersZS5:
            #    if(str(d.getSensorID()) == "4.5.2") :
            #        if(d.isUCluster()):
            #            print(" ZS5: seed charge = "+str(d.getSeedCharge())+",\
            # size = "+str(d.getSize())+", position = "+str(d.getPosition())+", charge = "+str(d.getCharge())+"   ")

            # for d in clustersZS3:
            #    if(str(d.getSensorID()) == "4.5.2") :
            #        if(d.isUCluster()):
            #            print(" ZS3: seed charge = "+str(d.getSeedCharge())+",\
            # size = "+str(d.getSize())+", position = "+str(d.getPosition())+", charge = "+str(d.getCharge())+"   ")

            for d3 in clustersZS3:
                if(str(d3.getSensorID()) == "4.5.2"):
                    if(d3.isUCluster()):
                        found = False
                        for d5 in clustersZS5:
                            if(str(d5.getSensorID()) == "4.5.2"):
                                if(d5.isUCluster()):
                                    if(d5.getSeedCharge() == d3.getSeedCharge()):
                                        found = True
                        if not found:
                            print(" ONLY ZS3: seed charge = " + str(d3.getSeedCharge()) + ", size = " + str(d3.getSize()) +
                                  ", position = " + str(d3.getPosition()) + ", charge = " + str(d3.getCharge()) + "   ")
                            reco = d3.getRelationsTo(reco3)
#                            print("test size cluster: "+str(reco.size()))
                            for r in reco:
                                charge = r.getCharge()
                                noise = noisePayload.getNoiseInElectrons(r.getSensorID(), True, r.getCellID())
                                print(" SNR = " + str(charge / noise) + ", charge = " + str(charge) + ", noise = " + str(noise))
# shaper = r.getRelatedTo(shaper3)

    def terminate(self):
        f = TFile(fileOUT, "RECREATE")
        self.fb2u.GetXaxis().SetTitle("# cl ZS3")
        self.fb2u.GetYaxis().SetTitle("# cl ZS5")
        self.fb2u.Write()

        f.Close()


# setup database for commissioning data
# reset_database()
# use_central_database(GLOBAL_TAG, LogLevel.WARNING)

# Create paths
main = create_path()

# if run on real data
# main.add_module('RootInput')

# generation and simulation
setupEventInfo(100, main)
add_evtgen_generator(main, 'charged')
add_simulation(main, components=['SVD'])


# if run on real data
# main.add_module("Gearbox")
# main.add_module('Geometry', useDB=True)
# main.add_module('SVDUnpacker', svdShaperDigitListName='SVDShaperDigitsToFilter')

# ZS 3
zs3 = register_module('SVDZeroSuppressionEmulator')
zs3.set_name("SVDZeroSuppressionEmulator_ZS3")
zs3.param('SNthreshold', 3)
zs3.param('ShaperDigits', 'SVDShaperDigits')  # ToFilter
zs3.param('ShaperDigitsIN', shaper3)
zs3.param('FADCmode', True)
# zs3.param('FADCmode',False)
main.add_module(zs3)

fitterZS3 = register_module('SVDCoGTimeEstimator')
fitterZS3.set_name('SVDCoGTimeEstimatorZS3')
fitterZS3.param('RecoDigits', reco3)
fitterZS3.param('ShaperDigits', shaper3)
main.add_module(fitterZS3)

clusterizerZS3 = register_module('SVDSimpleClusterizer')
clusterizerZS3.set_name('SVDSimpleClusterizerZS3')
clusterizerZS3.param('RecoDigits', reco3)
clusterizerZS3.param('Clusters', cluster3)
main.add_module(clusterizerZS3)

# ZS 5
zs5 = register_module('SVDZeroSuppressionEmulator')
zs5.set_name("SVDZeroSuppressionEmulator_ZS5")
zs5.param('SNthreshold', 5)
zs5.param('ShaperDigits', 'SVDShaperDigits')  # ToFilter
zs5.param('ShaperDigitsIN', shaper5)
zs5.param('FADCmode', True)
# zs5.param('FADCmode',False)
main.add_module(zs5)

fitterZS5 = register_module('SVDCoGTimeEstimator')
fitterZS5.set_name('SVDCoGTimeEstimatorZS5')
fitterZS5.param('RecoDigits', reco5)
fitterZS5.param('ShaperDigits', shaper5)
main.add_module(fitterZS5)

clusterizerZS5 = register_module('SVDSimpleClusterizer')
clusterizerZS5.set_name('SVDSimpleClusterizerZS5')
clusterizerZS5.param('RecoDigits', reco5)
clusterizerZS5.param('Clusters', cluster5)
main.add_module(clusterizerZS5)


main.add_module(SVDClustersDebug())

# main.add_module('Progress')
# main.add_module('ProgressBar')

# main.add_module('RootOutput')
print_path(main)

# Process events
process(main)

print(statistics)
