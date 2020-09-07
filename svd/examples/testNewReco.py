#!/usr/bin/env python3
# -*- coding: utf-8 -*-
from basf2 import *
from simulation import add_simulation
from basf2 import conditions as b2conditions
from svd import *
from tracking import *
import glob
import ROOT
from ROOT import Belle2, TH1F, TH2F, TFile

numEvents = 2000

# bkgFiles = glob.glob('/sw/belle2/bkg/*.root')  # Phase3 background
bkgFiles = None  # uncomment to remove  background
simulateJitter = False

ROIfinding = False
MCTracking = True
# set this string to identify the output rootfiles
tag = "_Y4S_noJitter_noBKG_noROI_MCTF.root"


set_random_seed(1234)
# logging.log_level = LogLevel.DEBUG

expList = [1003]


class SVDClustersQuickCheck(basf2.Module):

    def initialize(self):

        self.test = []
        self.testNew = []
        self.size = TH1F("cl_size", "Cluster Size", 20, 0, 20)
        self.sizeNew = TH1F("clNew_size", "New Cluster Size", 20, 0, 20)
        self.time = TH1F("cl_time", "Cluster Time", 300, -100, 200)
        self.timeNew = TH1F("clNew_time", "New Cluster Time", 300, -100, 200)
        self.ff = TH1F("cl_ff", "Cluster FirstFrame", 4, -0.4, 3.5)
        self.ffNew = TH1F("clNew_ff", "New Cluster FirstFrame", 4, -0.4, 3.5)
        self.charge = TH1F("cl_charge", "Cluster Charge", 300, 0, 100000)
        self.chargeNew = TH1F("clNew_charge", "New Cluster Charge", 300, 0, 100000)

        geoCache = Belle2.VXD.GeoCache.getInstance()

        ladderN = 0
        sensorN = 0

        for layer in geoCache.getLayers(Belle2.VXD.SensorInfoBase.SVD):
            layerNumber = layer.getLayerNumber()
            ladderN = 0
            for ladder in geoCache.getLadders(layer):
                ladderN = ladderN + 1
                sensorN = 0
                for sensor in geoCache.getSensors(ladder):
                    sensorN = sensorN + 1
            self.test.append(TH2F("cl_layer"+str(layerNumber), "Layer "+str(layerNumber) +
                                  " Ladder VS Sensor.Side", ladderN, 0.5, ladderN+0.5, 2*sensorN, +0.75, sensorN+0.75))
            self.testNew.append(TH2F("clNew_layer"+str(layerNumber), "Layer "+str(layerNumber) +
                                     " Ladder VS Sensor.Side", ladderN, 0.5, ladderN+0.5, 2*sensorN, +0.75, sensorN+0.75))

        print(self.test)

    def event(self):
        clusterList = Belle2.PyStoreArray("SVDClusters")
        clusterListNew = Belle2.PyStoreArray("SVDNewClusters")

        print("number of clusters = "+str(clusterList.getEntries())+" (old) VS "+str(clusterListNew.getEntries())+" (new)")

        for d in clusterList:
            self.size.Fill(d.getSize())
            self.time.Fill(d.getClsTime())
            self.ff.Fill(d.getFirstFrame())
            self.charge.Fill(d.getCharge())
            isU = 0
            if(d.isUCluster()):
                isU = 0.5
            self.test[d.getSensorID().getLayerNumber()-3].Fill(d.getSensorID().getLadderNumber(),
                                                               d.getSensorID().getSensorNumber()+isU)

        for d in clusterListNew:
            self.sizeNew.Fill(d.getSize())
            self.timeNew.Fill(d.getClsTime())
            self.ffNew.Fill(d.getFirstFrame())
            self.chargeNew.Fill(d.getCharge())
            isU = 0
            if(d.isUCluster()):
                isU = 0.5
            self.testNew[d.getSensorID().getLayerNumber()-3].Fill(d.getSensorID().getLadderNumber(),
                                                                  d.getSensorID().getSensorNumber()+isU)

    def terminate(self):
        f = TFile("quicktestSVDCluster.root", "RECREATE")
        for hist in self.test:
            hist.GetXaxis().SetTitle("ladder #")
            hist.GetYaxis().SetTitle("sensor # + 0.5 is isU")
            hist.Write()
        for hist in self.testNew:
            hist.GetXaxis().SetTitle("ladder #")
            hist.GetYaxis().SetTitle("sensor # + 0.5 is isU")
            hist.Write()

        self.size.GetXaxis().SetTitle("cluster size")
        self.size.Write()
        self.sizeNew.GetXaxis().SetTitle("clusterNew size")
        self.sizeNew.Write()

        self.time.GetXaxis().SetTitle("cluster time")
        self.time.Write()
        self.timeNew.GetXaxis().SetTitle("clusterNew time")
        self.timeNew.Write()

        self.ff.GetXaxis().SetTitle("cluster firstFrame")
        self.ff.Write()
        self.ffNew.GetXaxis().SetTitle("clusterNew firstFrame")
        self.ffNew.Write()

        self.charge.GetXaxis().SetTitle("cluster charge")
        self.charge.Write()
        self.chargeNew.GetXaxis().SetTitle("clusterNew charge")
        self.chargeNew.Write()

        f.Close()

# reco digits:


class SVDRecoDigitsQuickCheck(basf2.Module):

    def initialize(self):

        self.test = []
        self.testNew = []
        self.time = TH1F("rd_time", "RecoDigit Time", 300, -100, 200)
        self.timeNew = TH1F("rdNew_time", "New RecoDigit Time", 300, -100, 200)
        self.charge = TH1F("rd_charge", "RecoDigit Charge", 300, 0, 100000)
        self.chargeNew = TH1F("rdNew_charge", "New RecoDigit Charge", 300, 0, 100000)

        geoCache = Belle2.VXD.GeoCache.getInstance()

        ladderN = 0
        sensorN = 0

        for layer in geoCache.getLayers(Belle2.VXD.SensorInfoBase.SVD):
            layerNumber = layer.getLayerNumber()
            ladderN = 0
            for ladder in geoCache.getLadders(layer):
                ladderN = ladderN + 1
                sensorN = 0
                for sensor in geoCache.getSensors(ladder):
                    sensorN = sensorN + 1
            self.test.append(TH2F("rd_layer"+str(layerNumber), "Layer "+str(layerNumber) +
                                  " Ladder VS Sensor.Side", ladderN, 0.5, ladderN+0.5, 2*sensorN, +0.75, sensorN+0.75))
            self.testNew.append(TH2F("rdNew_layer"+str(layerNumber), "Layer "+str(layerNumber) +
                                     " Ladder VS Sensor.Side", ladderN, 0.5, ladderN+0.5, 2*sensorN, +0.75, sensorN+0.75))

        print(self.test)

    def event(self):
        recodigitList = Belle2.PyStoreArray("SVDRecoDigits")
        recodigitListNew = Belle2.PyStoreArray("SVDNewRecoDigits")

        print("number of recodigits = "+str(recodigitList.getEntries())+" (old) VS "+str(recodigitListNew.getEntries())+" (new)")

        for d in recodigitList:
            self.time.Fill(d.getTime())
            self.charge.Fill(d.getCharge())
            isU = 0
            if(d.isUStrip()):
                isU = 0.5
            self.test[d.getSensorID().getLayerNumber()-3].Fill(d.getSensorID().getLadderNumber(),
                                                               d.getSensorID().getSensorNumber()+isU)

        for d in recodigitListNew:
            self.timeNew.Fill(d.getTime())
            self.chargeNew.Fill(d.getCharge())
            isU = 0
            if(d.isUStrip()):
                isU = 0.5
            self.testNew[d.getSensorID().getLayerNumber()-3].Fill(d.getSensorID().getLadderNumber(),
                                                                  d.getSensorID().getSensorNumber()+isU)

    def terminate(self):
        f = TFile("quicktestSVDRecoDigit.root", "RECREATE")
        for hist in self.test:
            hist.GetXaxis().SetTitle("ladder #")
            hist.GetYaxis().SetTitle("sensor # + 0.5 is isU")
            hist.Write()
        for hist in self.testNew:
            hist.GetXaxis().SetTitle("ladder #")
            hist.GetYaxis().SetTitle("sensor # + 0.5 is isU")
            hist.Write()

        self.time.GetXaxis().SetTitle("recodigit time")
        self.time.Write()
        self.timeNew.GetXaxis().SetTitle("recodigitNew time")
        self.timeNew.Write()

        self.charge.GetXaxis().SetTitle("recodigit charge")
        self.charge.Write()
        self.chargeNew.GetXaxis().SetTitle("recodigitNew charge")
        self.chargeNew.Write()

        f.Close()

# b2conditions.prepend_globaltag("svd_onlySVDinGeoConfiguration")
# b2conditions.prepend_globaltag("svd_NOCoGCorrections")
# b2conditions.prepend_globaltag("svd_test_svdRecoConfiguration")

main = create_path()

eventinfosetter = register_module('EventInfoSetter')
eventinfosetter.param('expList', expList)
eventinfosetter.param('runList', [0])
eventinfosetter.param('evtNumList', [numEvents])
main.add_module(eventinfosetter)
main.add_module('EventInfoPrinter')
main.add_module('EvtGenInput')

add_simulation(
    main,
    bkgfiles=bkgFiles,
    usePXDDataReduction=ROIfinding,
    simulateT0jitter=simulateJitter)

add_svd_reconstruction(main)
for mod in main.modules():
    if(mod.name() == "SVDSimpleClusterizer"):
        mod.param("timeAlgorithm", 0)

clusterizer = register_module('SVDClusterizer')
clusterizer.param('timeAlgorithm6Samples', "CoG6")
clusterizer.param('timeAlgorithm3Samples', "CoG6")
clusterizer.param('chargeAlgorithm6Samples', "MaxSample")
clusterizer.param('chargeAlgorithm3Samples', "MaxSample")
clusterizer.param('Clusters', "SVDNewClusters")
clusterizer.param('useDB', True)
main.add_module(clusterizer)

recoDigitCreator = register_module('SVDRecoDigitCreator')
recoDigitCreator.param('timeAlgorithm6Samples', "CoG3")
recoDigitCreator.param('timeAlgorithm3Samples', "CoG6")
recoDigitCreator.param('chargeAlgorithm6Samples', "MaxSample")
recoDigitCreator.param('chargeAlgorithm3Samples', "MaxSample")
recoDigitCreator.param('RecoDigits', "SVDNewRecoDigits")
recoDigitCreator.param('useDB', False)
main.add_module(recoDigitCreator)

main.add_module(SVDClustersQuickCheck())
main.add_module(SVDRecoDigitsQuickCheck())

'''
add_tracking_reconstruction(
    main,
    components=["SVD"],
    mcTrackFinding=MCTracking,
    trackFitHypotheses=[211],
    skipHitPreparerAdding=True)


tag = "_Y4S_jitter10ns_wBKG_noROI_MCTF.root"
clseval = register_module('SVDClusterEvaluationTrueInfo')
clseval.param('outputFileName', "ClusterEvaluationTrueInfo" + str(tag))
main.add_module(clseval)

svdperf = register_module('SVDPerformance')
svdperf.param('outputFileName', "SVDPerformance" + str(tag))
main.add_module(svdperf)
'''
# main.add_module('RootOutput')
main.add_module('Progress')

print_path(main)

process(main)

print(statistics)
