import basf2
from ROOT import Belle2
import numpy as np
import pickle


class nntd(basf2.Module):
    '''
    This class represents a dataset.
    '''
    version = 2  # changes, when form of self.array changes
    # dict to store the content for each entry in a track vector
    varnum = {}
    varnum["recoz"] = [0, r'$Z_{Reco}$', r'$[cm]$']
    varnum["recotheta"] = [1, r'$\theta_{Reco}$', r'$[°]$']
    varnum["recophi"] = [2, r'$\phi_{Reco}$', r'$[°]$']
    varnum["recopt"] = [3, r'$P_{t, Reco}$', r'$[GeV]$']
    varnum["neuroz"] = [4, r'$Z_{Neuro}$', r'$[cm]$']
    varnum["neurotheta"] = [5, r'$\theta_{Neuro}$', r'$[°]$']
    varnum["neurophi"] = [6, r'$\phi_{Neuro}$', r'$[°]$']
    varnum["neuropt"] = [7, r'$P_{t, Neuro}$', r'$[GeV]$']
    varnum["neuroval"] = [8, r'Validity', '']
    varnum["neuroqual"] = [9, r'Quality', '']
    varnum["neurots"] = [10, r'TSVector', '']
    varnum["neuroexp"] = [11, r'Expert Number', '']
    varnum["neurodriftth"] = [12, r'Driftthreshold', '']
    varnum["neuroquad"] = [13, r'Quadrant', '']
    varnum["neurofp"] = [14, r'Fastestpriority Eventtime', 'clocks']
    varnum["neuroetf"] = [15, r'ETF Eventtime', 'clocks']
    varnum["twodphi"] = [16, r'$\phi_{2D}$', r'$[°]$']
    varnum["twodpt"] = [17, r'$P_{t, 2D}$', r'$[GeV]$']
    varnum["twodfot"] = [18, r'FoundOldTrack', '']
    varnum["hwneuroz"] = [4, r'$Z_{HWNeuro}$', r'$[cm]$']
    varnum["hwneurotheta"] = [5, r'$\theta_{HWNeuro}$', r'$[°]$']
    varnum["hwneurophi"] = [6, r'$\phi_{HWNeuro}$', r'$[°]$']
    varnum["hwneuropt"] = [7, r'$P_{t, HWNeuro}$', r'$[GeV]$']
    varnum["hwNeuroval"] = [8, r'Validity', '']
    varnum["hwNeuroqual"] = [9, r'Quality', '']
    varnum["hwNeurots"] = [10, r'TSVector', '']
    varnum["hwNeuroexp"] = [11, r'Expert Number', '']
    varnum["hwNeurodriftth"] = [12, r'Driftthreshold', '']
    varnum["hwNeuroquad"] = [13, r'Quadrant', '']
    varnum["hwNeurofp"] = [14, r'Fastestpriority Eventtime', 'clocks']
    varnum["hwNeuroetf"] = [15, r'ETF Eventtime', 'clocks']
    nonelist = [None for i in range(17)]

    def initialize(self):
        # TODO:
        # check if folder is present or create it
        # initialize all plots somehow
        # initialize filters somehow, so they can be looped over in the evetn function
        # setup histograms
        self.data = None  # np.array([[[]]])
        self.eventlist = []
        # TODO
        # # dict of plots, which should be plotted during the processing and updated every 5000 events.
        # self.plotdict = {}
        self.recotracksname = "RecoTracks"  # recotracksname
        self.neurotracksname = "TSimNeuroTracks"  # "TRGCDCNeuroTracks"  # neurotracksname
        self.hwneurotracksname = "CDCTriggerNeuroTracks"  # "TRGCDCNeuroTracks"  # neurotracksname
        self.twodtracksname = "CDCTriggerNNInput2DFinderTracks"  # "TRGCDC2DFinderTracks"  # twodtracksname
        self.etfname = "CDCTriggerNeuroETFT0"
        self.tsname = "CDCTriggerNNInputSegmentHits"

        # storearrays
        self.recotracks = Belle2.PyStoreArray(self.recotracksname)
        self.neurotracks = Belle2.PyStoreArray(self.neurotracksname)
        self.hwneurotracks = Belle2.PyStoreArray(self.hwneurotracksname)
        self.twodtracks = Belle2.PyStoreArray(self.twodtracksname)
        self.ts = Belle2.PyStoreArray(self.tsname)
        self.etf = Belle2.PyStoreObj(self.etfname)

        self.varnum = nntd.varnum
        self.networkname = None

    def costotheta(self, x):
        if isinstance(x, list):
            ret = []
            for y in x:
                ret.append(self.costotheta(y))
            return ret
        else:
            ret = None
            if not x:
                return None
            else:
                if x < -1 or x > 1:
                    x = np.round(x)
                return 180. / np.pi * np.arccos(x)

    def getrecovals(self, fitres):
        ret = []
        if fitres:
            ret.append(fitres.getPosition().Z())
            ret.append(self.costotheta(fitres.getMomentum().CosTheta()))
            ret.append(fitres.getMomentum().Phi())
            ret.append(fitres.getMomentum().Pt())
        else:
            for i in range(4):
                ret.append(None)
        return ret

    def getneurovals(self, neuro):
        ret = []
        if neuro:
            ret.append(neuro.getZ0())
            ret.append(self.costotheta(neuro.getCotTheta() / np.sqrt(1 + neuro.getCotTheta()**2)))
            ret.append(neuro.getPhi0())
            ret.append(neuro.getPt())
            ret.append(neuro.getValidStereoBit())
            ret.append(neuro.getQualityVector())
            ret.append(None)  # int(neuro.getTSVector()))
            ret.append(neuro.getExpert())
            ret.append(None)  # int(neuro.getDriftThreshold()))
            ret.append(neuro.getQuadrant())
            fpt = 9999
            for ts in neuro.getRelationsTo(self.tsname):
                if ts.priorityTime() < fpt:
                    fpt = ts.priorityTime()
            if self.etf.hasBinnedEventT0(Belle2.Const.CDC):
                eft = self.etf.getBinnedEventT0(Belle2.Const.CDC)
            else:
                eft = None
            ret.append(fpt)
            ret.append(eft)
        else:
            for i in range(12):
                ret.append(None)
        return ret

    def gettwodvals(self, twod):
        ret = []
        if twod:
            ret.append(twod.getPhi0())
            ret.append(twod.getPt())
            ret.append(None)  # int(twod.getFoundOldTrack()))
        else:
            for i in range(3):
                ret.append(None)
        return ret

    def event(self):
        # TODO: update the plots every nth time
        # if self.showplots != 0:
        #     if eventnumber % self.showplots = 0:
        #         show plots

        # loop over events
        event = []
        for reco in self.recotracks:
            track = reco.getRelatedFrom("Tracks")
            if not track:
                print("no track found for recotrack")
                continue
            whishPdg = 211  # pion
            fitres = track.getTrackFitResultWithClosestMass(Belle2.Const.ChargedStable(whishPdg))
            if not fitres:
                continue
            # neuro = reco.getRelatedTo(self.neurotracksname)
            event.append([])
            try:
                neuro = reco.getRelatedTo(self.neurotracksname)
            except BaseException:
                neuro = None
            try:
                hwneuro = reco.getRelatedTo(self.hwneurotracksname)
            except BaseException:
                hwneuro = None
            try:
                twod = reco.getRelatedTo(self.twodtracksname)
            except BaseException:
                twod = None
            event[-1] += self.getrecovals(fitres)
            event[-1] += self.getneurovals(neuro)
            event[-1] += self.gettwodvals(twod)
            event[-1] += self.getneurovals(hwneuro)
        for neuro in self.neurotracks:
            # print("neuroloop")
            # print(len(neuro.getRelationsFrom(self.recotracksname)))
            if len(neuro.getRelationsFrom(self.recotracksname)) > 0:
                # this track is already stored in a recoline
                # print("skipping...")
                continue
            event.append([])
            try:
                twod = reco.getRelatedTo(self.twodtracksname)
            except BaseException:
                twod = None
            try:
                hwneuro = neuro.getRelatedTo(self.hwneurotracksname)
            except BaseException:
                twod = None
            event[-1] += self.getrecovals(None)
            event[-1] += self.getneurovals(neuro)
            event[-1] += self.gettwodvals(twod)
            event[-1] += self.getneurovals(hwneuro)
        for twod in self.twodtracks:
            # print("twodloop")
            # print(len(twod.getRelationsFrom(self.neurotracksname)))
            if len(twod.getRelationsFrom(self.neurotracksname)) > 0:
                # print("skipping...")
                # this track is already stored in a recoline or twodline
                continue
            event.append([])
            event[-1] += self.getrecovals(None)
            event[-1] += self.getneurovals(None)
            event[-1] += self.gettwodvals(twod)
            event[-1] += self.getneurovals(None)

        # attach an array for every event
        if len(event) > 100:
            event = event[0:100]
        elif len(event) < 100:
            for i in range(100 - len(event)):
                event.append(self.getrecovals(None) + self.getneurovals(None) + self.gettwodvals(None) + self.getneurovals(None))
        self.eventlist.append(event)

    def terminate(self):
        # self.eventfilters()
        # self.makearray(self.eventlist)
        # convert eventlist to data array
        # initialize histograms and fill them
        # both save histograms to file and show them in the plots
        pass

    def save(self, filename, netname):
        # save the dataset as an array, the corresponding varnum,
        # and a description about the dataset into a pickle file
        savedict = {}
        savedict["eventlist"] = self.eventlist
        savedict["varnum"] = self.varnum
        savedict["networkname"] = netname
        savedict["version"] = nntd.version
        f = open(filename, 'wb')
        pickle.dump(savedict, f)
        f.close()
        print('file ' + filename + ' has been saved. ')

    def loadmore(self, filenames):
        for x in filenames:
            f = open(x, 'rb')
            savedict = pickle.load(f)
            f.close()
            if self.version != savedict["version"]:
                print("Error! loaded file was made with different version of nntd! exiting ... ")
                exit()
            self.networkname = savedict["networkname"]
            self.eventlist += savedict["eventlist"]
            self.varnum = savedict["varnum"]
            print("Loaded file: " + x)
        self.makearray(self.eventlist)

    def load(self, filename):
        # load a given pickle file
        f = open(filename, 'rb')
        savedict = pickle.load(f)
        f.close()
        if self.version != savedict["version"]:
            print("Error! loaded file was made with different version of nntd! exiting ... ")
            exit()
        self.eventlist = savedict["eventlist"]
        self.varnum = savedict["varnum"]
        self.networkname = savedict["networkname"]
        # self.eventfilters()
        self.makearray(self.eventlist)

    def makearray(self, evlist):
        # TODO: apply filters
        self.data = np.array(evlist)
