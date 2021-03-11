#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from simulation import add_simulation

import basf2 as b2
from svd import add_svd_reconstruction
from ROOT import Belle2, TH1F, TH2F, TFile

numEvents = 2000

# bkgFiles = glob.glob('/sw/belle2/bkg/*.root')  # Phase3 background
bkgFiles = None  # uncomment to remove  background
simulateJitter = False
ROIfinding = False

b2.set_random_seed(1234)
# logging.log_level = LogLevel.DEBUG

expList = [1003]


class SVDClustersQuickCheck(b2.Module):
    ''' quick check of cluster reconstruction'''

    def initialize(self):
        '''define histograms'''

        #: \cond
        self.test = []
        self.testNew = []
        self.size = TH1F("cl_size", "Cluster Size", 20, 0, 20)
        self.sizeNew = TH1F("clNew_size", "New Cluster Size", 20, 0, 20)
        self.time = TH1F("cl_time", "Cluster Time", 300, -100, 200)
        self.timeNew = TH1F("clNew_time", "New Cluster Time", 300, -100, 200)
        self.ff = TH1F("cl_ff", "Cluster FirstFrame", 4, -0.4, 3.5)
        self.ffNew = TH1F("clNew_ff", "New Cluster FirstFrame", 4, -0.4, 3.5)
        self.charge = TH1F("cl_charge", "Cluster Charge", 300, 0, 100)
        self.chargeNew = TH1F("clNew_charge", "New Cluster Charge", 300, 0, 100)
        self.SNR = TH1F("cl_SNR", "Cluster SNR", 100, 0, 100)
        self.SNRNew = TH1F("clNew_SNR", "New Cluster SNR", 100, 0, 100)
        self.position = TH1F("cl_position", "Cluster Position", 300, -6, 6)
        self.positionNew = TH1F("clNew_position", "New Cluster Position", 300, -6, 6)
        self.positionSigma = TH1F("cl_positionSigma", "Cluster Position Error", 300, 0, 100)
        self.positionSigmaNew = TH1F("clNew_positionSigma", "New Cluster Position Error", 300, 0, 100)
        self.positionS1 = TH1F("cl_positionS1", "Cluster Position Size 1", 300, -6, 6)
        self.positionS1New = TH1F("clNew_positionS1", "New Cluster Position Size 1", 300, -6, 6)
        self.positionS1Sigma = TH1F("cl_positionS1Sigma", "Cluster Position Error Size 1", 300, 0, 100)
        self.positionS1SigmaNew = TH1F("clNew_positionS1Sigma", "New Cluster Position Error Size 1", 300, 0, 100)
        self.positionS2 = TH1F("cl_positionS2", "Cluster Position Size 2", 300, -6, 6)
        self.positionS2New = TH1F("clNew_positionS2", "New Cluster Position Size 2", 300, -6, 6)
        self.positionS2Sigma = TH1F("cl_positionS2Sigma", "Cluster Position Size Error 2", 300, 0, 100)
        self.positionS2SigmaNew = TH1F("clNew_positionS2Sigma", "New Cluster Position Error Size 2", 300, 0, 100)
        self.positionS3 = TH1F("cl_positionS3", "Cluster Position Size >2", 300, -6, 6)
        self.positionS3New = TH1F("clNew_positionS3", "New Cluster Position Size >2", 300, -6, 6)
        self.positionS3Sigma = TH1F("cl_positionS3Sigma", "Cluster Position Error Size >2", 300, 0, 100)
        self.positionS3SigmaNew = TH1F("clNew_positionS3Sigma", "New Cluster Position Error Size >2", 300, 0, 100)
        self.positionPull = TH1F("cl_positionPull", "Cluster Position Pull", 200, -10, 10)
        self.positionPullNew = TH1F("clNew_positionPull", "New Cluster Position Pull", 200, -10, 10)
        self.positionPull1 = TH1F("cl_positionPull1", "Cluster Position Pull Size 1", 200, -10, 10)
        self.positionPull1New = TH1F("clNew_positionPull1", "New Cluster Position Pull Size 1", 200, -10, 10)
        self.positionPull2 = TH1F("cl_positionPull2", "Cluster Position Pull Size 2", 200, -10, 10)
        self.positionPull2New = TH1F("clNew_positionPull2", "New Cluster Position Pull Size 2", 200, -10, 10)
        self.positionPull3 = TH1F("cl_positionPull3", "Cluster Position Pull Size >2", 200, -10, 10)
        self.positionPull3New = TH1F("clNew_positionPull3", "New Cluster Position Pull Size >2", 200, -10, 10)
        #: \endcond

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
        ''' look at cluster and new clusters'''

        clusterList = Belle2.PyStoreArray("SVDClusters")
        clusterListNew = Belle2.PyStoreArray("SVDNewClusters")
        print("number of clusters = "+str(clusterList.getEntries())+" (old) VS "+str(clusterListNew.getEntries())+" (new)")

        for d in clusterList:
            trueList = d.getRelationsTo('SVDTrueHits')  # SVDClustersToSVDTrueHits
            isU = 0
            if(d.isUCluster()):
                isU = 0.5
                truePos = trueList[0].getU()
            else:
                truePos = trueList[0].getV()

            self.size.Fill(d.getSize())
            self.time.Fill(d.getClsTime())
            self.ff.Fill(d.getFirstFrame())
            self.charge.Fill(d.getCharge()/1000)
            self.SNR.Fill(d.getSNR())
            self.position.Fill(d.getPosition())
            self.positionSigma.Fill(d.getPositionSigma() * 1e4)
            if d.getSize() == 1:
                self.positionS1.Fill(d.getPosition())
                self.positionS1Sigma.Fill(d.getPositionSigma() * 1e4)
                self.positionPull1.Fill((d.getPosition() - truePos)/d.getPositionSigma())
            if d.getSize() == 2:
                self.positionS2.Fill(d.getPosition())
                self.positionS2Sigma.Fill(d.getPositionSigma() * 1e4)
                self.positionPull2.Fill((d.getPosition() - truePos)/d.getPositionSigma())
            if d.getSize() > 2:
                self.positionS3.Fill(d.getPosition())
                self.positionS3Sigma.Fill(d.getPositionSigma() * 1e4)
                self.positionPull3.Fill((d.getPosition() - truePos)/d.getPositionSigma())

            self.test[d.getSensorID().getLayerNumber()-3].Fill(d.getSensorID().getLadderNumber(),
                                                               d.getSensorID().getSensorNumber()+isU)
            self.positionPull.Fill((d.getPosition() - truePos)/d.getPositionSigma())

        for d in clusterListNew:

            isU = 0
            trueListNew = d.getRelationsTo('SVDTrueHits')  # SVDClustersToSVDTrueHits
            if(d.isUCluster()):
                isU = 0.5
                truePos = trueListNew[0].getU()
            else:
                truePos = trueListNew[0].getV()

            self.sizeNew.Fill(d.getSize())
            self.timeNew.Fill(d.getClsTime())
            self.ffNew.Fill(d.getFirstFrame())
            self.chargeNew.Fill(d.getCharge()/1000)
            self.SNRNew.Fill(d.getSNR())
            self.positionNew.Fill(d.getPosition())
            self.positionSigmaNew.Fill(d.getPositionSigma() * 1e4)
            if d.getSize() == 1:
                self.positionS1New.Fill(d.getPosition())
                self.positionS1SigmaNew.Fill(d.getPositionSigma() * 1e4)
                self.positionPull1New.Fill((d.getPosition() - truePos)/d.getPositionSigma())
            if d.getSize() == 2:
                self.positionS2New.Fill(d.getPosition())
                self.positionS2SigmaNew.Fill(d.getPositionSigma() * 1e4)
                self.positionPull2New.Fill((d.getPosition() - truePos)/d.getPositionSigma())
            if d.getSize() > 2:
                self.positionS3New.Fill(d.getPosition())
                self.positionS3SigmaNew.Fill(d.getPositionSigma() * 1e4)
                self.positionPull3New.Fill((d.getPosition() - truePos)/d.getPositionSigma())

            self.testNew[d.getSensorID().getLayerNumber()-3].Fill(d.getSensorID().getLadderNumber(),
                                                                  d.getSensorID().getSensorNumber()+isU)
            self.positionPullNew.Fill((d.getPosition() - truePos)/d.getPositionSigma())

    def terminate(self):
        '''write'''

        f = TFile("quicktestSVDClusterOldDefault.root", "RECREATE")
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
        self.sizeNew.GetXaxis().SetTitle("cluster size")
        self.sizeNew.Write()

        self.time.GetXaxis().SetTitle("cluster time (ns)")
        self.time.Write()
        self.timeNew.GetXaxis().SetTitle("cluster time (ns)")
        self.timeNew.Write()

        self.ff.GetXaxis().SetTitle("cluster firstFrame")
        self.ff.Write()
        self.ffNew.GetXaxis().SetTitle("cluster firstFrame")
        self.ffNew.Write()

        self.charge.GetXaxis().SetTitle("cluster charge (ke-)")
        self.charge.Write()
        self.chargeNew.GetXaxis().SetTitle("cluster charge (ke-)")
        self.chargeNew.Write()

        self.SNR.GetXaxis().SetTitle("cluster SNR")
        self.SNR.Write()
        self.SNRNew.GetXaxis().SetTitle("cluster SNR")
        self.SNRNew.Write()

        self.position.GetXaxis().SetTitle("cluster position (cm)")
        self.position.Write()
        self.positionNew.GetXaxis().SetTitle("cluster position (cm)")
        self.positionNew.Write()

        self.positionSigma.GetXaxis().SetTitle("cluster position error (#mum)")
        self.positionSigma.Write()
        self.positionSigmaNew.GetXaxis().SetTitle("cluster position error (#mum)")
        self.positionSigmaNew.Write()

        self.positionS1.GetXaxis().SetTitle("cluster position (cm)")
        self.positionS1.Write()
        self.positionS1New.GetXaxis().SetTitle("cluster position (cm)")
        self.positionS1New.Write()

        self.positionS1Sigma.GetXaxis().SetTitle("cluster position error (#mum)")
        self.positionS1Sigma.Write()
        self.positionS1SigmaNew.GetXaxis().SetTitle("cluster position error (#mum)")
        self.positionS1SigmaNew.Write()

        self.positionS2.GetXaxis().SetTitle("cluster position (cm)")
        self.positionS2.Write()
        self.positionS2New.GetXaxis().SetTitle("cluster position (cm)")
        self.positionS2New.Write()

        self.positionS2Sigma.GetXaxis().SetTitle("cluster position error (#mum)")
        self.positionS2Sigma.Write()
        self.positionS2SigmaNew.GetXaxis().SetTitle("cluster position error (#mum)")
        self.positionS2SigmaNew.Write()

        self.positionS3.GetXaxis().SetTitle("cluster position (cm)")
        self.positionS3.Write()
        self.positionS3New.GetXaxis().SetTitle("cluster position (cm)")
        self.positionS3New.Write()

        self.positionS3Sigma.GetXaxis().SetTitle("cluster position error (#mum)")
        self.positionS3Sigma.Write()
        self.positionS3SigmaNew.GetXaxis().SetTitle("cluster position error (#mum)")
        self.positionS3SigmaNew.Write()

        self.positionPull.GetXaxis().SetTitle("cluster position pull")
        self.positionPull.Write()
        self.positionPullNew.GetXaxis().SetTitle("cluster position pull")
        self.positionPullNew.Write()
        self.positionPull1.GetXaxis().SetTitle("cluster position pull")
        self.positionPull1.Write()
        self.positionPull1New.GetXaxis().SetTitle("cluster position pull")
        self.positionPull1New.Write()
        self.positionPull2.GetXaxis().SetTitle("cluster position pull")
        self.positionPull2.Write()
        self.positionPull2New.GetXaxis().SetTitle("cluster position pull")
        self.positionPull2New.Write()
        self.positionPull3.GetXaxis().SetTitle("cluster position pull")
        self.positionPull3.Write()
        self.positionPull3New.GetXaxis().SetTitle("cluster position pull")
        self.positionPull3New.Write()

        f.Close()


class SVDRecoDigitsQuickCheck(b2.Module):
    '''quick check of SVDRecoDigits'''

    def initialize(self):
        '''define histograms'''

        #: \cond
        self.test = []
        self.testNew = []
        self.time = TH1F("rd_time", "RecoDigit Time", 300, -100, 200)
        self.timeNew = TH1F("rdNew_time", "New RecoDigit Time", 300, -100, 200)
        self.charge = TH1F("rd_charge", "RecoDigit Charge", 300, 0, 100000)
        self.chargeNew = TH1F("rdNew_charge", "New RecoDigit Charge", 300, 0, 100000)
        #: \endcond

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
        '''look at old and new reco digits'''

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
        '''write'''

        f = TFile("quicktestSVDRecoDigitOldDefault.root", "RECREATE")
        for hist in self.test:
            hist.GetXaxis().SetTitle("ladder #")
            hist.GetYaxis().SetTitle("sensor # + 0.5 is isU")
            hist.Write()
        for hist in self.testNew:
            hist.GetXaxis().SetTitle("ladder #")
            hist.GetYaxis().SetTitle("sensor # + 0.5 is isU")
            hist.Write()

        self.time.GetXaxis().SetTitle("recodigit time (ns)")
        self.time.Write()
        self.timeNew.GetXaxis().SetTitle("recodigit time (ns)")
        self.timeNew.Write()

        self.charge.GetXaxis().SetTitle("recodigit charge (ke-)")
        self.charge.Write()
        self.chargeNew.GetXaxis().SetTitle("recodigit charge (ke-)")
        self.chargeNew.Write()

        f.Close()

# b2conditions.prepend_globaltag("svd_test_svdRecoConfiguration")


main = b2.create_path()

eventinfosetter = b2.register_module('EventInfoSetter')
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
        mod.param("HeadTailSize", 3)

clusterizer = b2.register_module('SVDClusterizer')
clusterizer.param('timeAlgorithm6Samples', "CoG6")
clusterizer.param('timeAlgorithm3Samples', "CoG6")
clusterizer.param('chargeAlgorithm6Samples', "MaxSample")
clusterizer.param('chargeAlgorithm3Samples', "MaxSample")
clusterizer.param('positionAlgorithm6Samples', "oldDefault")
clusterizer.param('positionAlgorithm3Samples', "oldDefault")
clusterizer.param('stripTimeAlgorithm6Samples', "dontdo")
clusterizer.param('stripTimeAlgorithm3Samples', "dontdo")
clusterizer.param('stripChargeAlgorithm6Samples', "MaxSample")
clusterizer.param('stripChargeAlgorithm3Samples', "MaxSample")
clusterizer.param('Clusters', "SVDNewClusters")
clusterizer.param('useDB', False)
main.add_module(clusterizer)

recoDigitCreator = b2.register_module('SVDRecoDigitCreator')
recoDigitCreator.param('timeAlgorithm6Samples', "CoG6")
recoDigitCreator.param('timeAlgorithm3Samples', "CoG6")
recoDigitCreator.param('chargeAlgorithm6Samples', "MaxSample")
recoDigitCreator.param('chargeAlgorithm3Samples', "MaxSample")
recoDigitCreator.param('RecoDigits', "SVDNewRecoDigits")
recoDigitCreator.param('useDB', False)
main.add_module(recoDigitCreator)

main.add_module(SVDClustersQuickCheck())
main.add_module(SVDRecoDigitsQuickCheck())

# main.add_module('RootOutput')

main.add_module('Progress')

b2.print_path(main)

b2.process(main)

print(b2.statistics)
