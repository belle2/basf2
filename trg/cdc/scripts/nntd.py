##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import basf2

import os
import pickle


class nntd(basf2.Module):
    '''
    This class represents a dataset.
    '''
    version = 2  # changes, when form of self.array changes
    maxtracks = 100  # max number of tracks per event to be stored
    # dict to store the content for each entry in a track vector
    varnum = {}
    varnum["recoz"] = [0, r'$Z_{Reco}$', r'$[cm]$']
    varnum["recotheta"] = [1, r'$\theta_{Reco}$', r'$[°]$']
    varnum["recophi"] = [2, r'$\phi_{Reco}$', r'$[°]$']
    varnum["recopt"] = [3, r'$P_{t, Reco}$', r'$[GeV]$']
    varnum["recop"] = [4, r'$P_{Reco}$', r'$[GeV]$']
    varnum["neuroz"] = [5, r'$Z_{Neuro}$', r'$[cm]$']
    varnum["neurotheta"] = [6, r'$\theta_{Neuro}$', r'$[°]$']
    varnum["neurophi"] = [7, r'$\phi_{Neuro}$', r'$[°]$']
    varnum["neuropt"] = [8, r'$P_{Neuro}$', r'$[GeV]$']
    varnum["neurop"] = [9, r'$P_{t, Neuro}$', r'$[GeV]$']
    varnum["neuroval"] = [10, r'Validity', '']
    varnum["neuroqual"] = [11, r'Quality', '']
    varnum["neurots"] = [12, r'TSVector', '']
    varnum["neuroexp"] = [13, r'Expert Number', '']
    varnum["neurodriftth"] = [14, r'Driftthreshold', '']
    varnum["neuroquad"] = [15, r'Quadrant', '']
    varnum["neurofp"] = [16, r'Fastestpriority Eventtime', 'clocks']
    varnum["neuroetf"] = [17, r'ETF Eventtime', 'clocks']
    varnum["twodphi"] = [18, r'$\phi_{2D}$', r'$[°]$']
    varnum["twodpt"] = [19, r'$P_{t, 2D}$', r'$[GeV]$']
    varnum["twodfot"] = [20, r'FoundOldTrack', '']
    varnum["hwneuroz"] = [21, r'$Z_{HWNeuro}$', r'$[cm]$']
    varnum["hwneurotheta"] = [22, r'$\theta_{HWNeuro}$', r'$[°]$']
    varnum["hwneurophi"] = [23, r'$\phi_{HWNeuro}$', r'$[°]$']
    varnum["hwneuropt"] = [24, r'$P_{t, HWNeuro}$', r'$[GeV]$']
    varnum["hwneurop"] = [25, r'$P_{HWNeuro}$', r'$[GeV]$']
    varnum["hwneuroval"] = [26, r'Validity', '']
    varnum["hwneuroqual"] = [27, r'Quality', '']
    varnum["hwneurots"] = [28, r'TSVector', '']
    varnum["hwneuroexp"] = [29, r'Expert Number', '']
    varnum["hwneurodriftth"] = [30, r'Driftthreshold', '']
    varnum["hwneuroquad"] = [31, r'Quadrant', '']
    varnum["hwneurofp"] = [32, r'Fastestpriority Eventtime', 'clocks']
    varnum["hwneuroetf"] = [33, r'ETF Eventtime', 'clocks']
    varnum["swneuroz"] = [34, r'$Z_{SWNeuro}$', r'$[cm]$']
    varnum["swneurotheta"] = [35, r'$\theta_{SWNeuro}$', r'$[°]$']
    varnum["swneurophi"] = [36, r'$\phi_{SWNeuro}$', r'$[°]$']
    varnum["swneuropt"] = [37, r'$P_{t, SWNeuro}$', r'$[GeV]$']
    varnum["swneurop"] = [38, r'$P_{SWNeuro}$', r'$[GeV]$']
    varnum["swneuroval"] = [39, r'Validity', '']
    varnum["swneuroqual"] = [40, r'Quality', '']
    varnum["swneurots"] = [41, r'TSVector', '']
    varnum["swneuroexp"] = [42, r'Expert Number', '']
    varnum["swneurodriftth"] = [43, r'Driftthreshold', '']
    varnum["swneuroquad"] = [44, r'Quadrant', '']
    varnum["swneurofp"] = [45, r'Fastestpriority Eventtime', 'clocks']
    varnum["swneuroetf"] = [46, r'ETF Eventtime', 'clocks']
    varnum["swtwodphi"] = [47, r'$\phi_{SW2D}$', r'$[°]$']
    varnum["swtwodpt"] = [48, r'$P_{t, SW2D}$', r'$[GeV]$']
    varnum["swtwodfot"] = [49, r'FoundOldTrack', '']
    varnum["neuroats"] = [50, r'NumberOfAxials', '']
    varnum["hwneuroats"] = [51, r'NumberOfAxials', '']
    varnum["swneuroats"] = [52, r'NumberOfAxials', '']
    varnum["neuroetfcc"] = [53, r'ETF Eventtime from CC', 'clocks']
    varnum["neurohwtime"] = [54, r'Reconstructed HW Eventtime', 'clocks']
    varnum["hwneuroetfcc"] = [55, r'ETF Eventtime', 'clocks']
    varnum["hwneurohwtime"] = [56, r'Reconstructed HW Eventtime', 'clocks']
    nonelist = []
    for x in varnum:
        nonelist.append(None)

    def param(self, params):
        for key, value in params.items():
            setattr(self, key, value)

    def initialize(self):
        from ROOT import Belle2  # noqa
        # TODO:
        # check if folder is present or create it
        # initialize all plots somehow
        # initialize filters somehow, so they can be looped over in the evetn function
        # setup histograms
        self.data = None
        self.eventlist = []
        self.networkname = "unspecified net"
        self.dataname = "unspecified runs"
        # TODO
        # # dict of plots, which should be plotted during the processing and updated every 5000 events.
        # self.plotdict = {}
        self.recotracksname = "RecoTracks"  # recotracksname
        # if not hasattr(self, "neurotracksname"):
        self.neurotracksname = "TSimNeuroTracks"  # "TRGCDCNeuroTracks"  # neurotracksname
        self.hwneurotracksname = "CDCTriggerNeuroTracks"  # "TRGCDCNeuroTracks"  # neurotracksname
        self.swneurotracksname = "TRGCDCNeuroTracks"  # neurotracksname
        self.twodtracksname = "CDCTriggerNNInput2DFinderTracks"  # "TRGCDC2DFinderTracks"  # twodtracksname
        self.swtwodtracksname = "TRGCDC2DFinderTracks"  # twodtracksname
        self.etfname = "CDCTriggerNeuroETFT0"
        self.tsname = "CDCTriggerNNInputSegmentHits"

        # storearrays
        self.recotracks = Belle2.PyStoreArray(self.recotracksname)
        try:
            self.neurotracks = Belle2.PyStoreArray(self.neurotracksname)
        except ValueError:
            self.neurotracks = None
        try:
            self.hwneurotracks = Belle2.PyStoreArray(self.hwneurotracksname)
        except ValueError:
            self.hwneurotracks = None
        try:
            self.swneurotracks = Belle2.PyStoreArray(self.swneurotracksname)
        except ValueError:
            self.swneurotracks = None
        try:
            self.twodtracks = Belle2.PyStoreArray(self.twodtracksname)
        except ValueError:
            self.twodtracks = None
        try:
            self.swtwodtracks = Belle2.PyStoreArray(self.swtwodtracksname)
        except ValueError:
            self.swtwodtracks = None
        try:
            self.ts = Belle2.PyStoreArray(self.tsname)
        except ValueError:
            self.ts = None
        try:
            self.etf = Belle2.PyStoreObj(self.etfname)
        except ValueError:
            self.etf = None

        self.varnum = nntd.varnum

        self.debuglist = []
#        if not self.networkname: self.networknamne = "default"
#        if not self.filename: self.filename = "default.pkl"

    def costotheta(self, x):
        import numpy as np  # noqa
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

    def getrecovalsold(self, evlist, fitres):
        if fitres:
            evlist[self.varnum["recoz"][0]] = fitres.getPosition().Z()
            evlist[self.varnum["recotheta"][0]] = fitres.getMomentum().Theta()  # self.costotheta(fitres.getMomentum().CosTheta())
            evlist[self.varnum["recophi"][0]] = fitres.getMomentum().Phi()
            evlist[self.varnum["recopt"][0]] = fitres.getTransverseMomentum()
            evlist[self.varnum["recop"][0]] = fitres.getMomentum().R()
        return evlist

    def getrecovals(self, evlist, state):
        if state:
            evlist[self.varnum["recoz"][0]] = state.getPos().Z()
            evlist[self.varnum["recotheta"][0]] = state.getMom().Theta()  # self.costotheta(fitres.getMomentum().CosTheta())
            evlist[self.varnum["recophi"][0]] = state.getMom().Phi()
            evlist[self.varnum["recopt"][0]] = state.getMom().Pt()
            evlist[self.varnum["recop"][0]] = state.getMomMag()
        return evlist

    def getneurovals(self, evlist, neuro, status=""):
        from ROOT import Belle2  # noqa
        import numpy as np  # noqa
        pre = status
        if neuro:

            evlist[self.varnum[pre + "neuroz"][0]] = neuro.getZ0()
            evlist[self.varnum[pre + "neurotheta"][0]] = self.costotheta(neuro.getCotTheta() / np.sqrt(1 + neuro.getCotTheta()**2))
            evlist[self.varnum[pre + "neurophi"][0]] = neuro.getPhi0()
            evlist[self.varnum[pre + "neuropt"][0]] = neuro.getPt()
            evlist[self.varnum[pre + "neurop"][0]] = neuro.getPt()/np.sin(self.costotheta(neuro.getCotTheta() /
                                                                                          np.sqrt(1 + neuro.getCotTheta()**2)))
            evlist[self.varnum[pre + "neuroval"][0]] = neuro.getValidStereoBit()
            evlist[self.varnum[pre + "neuroqual"][0]] = neuro.getQualityVector()
            evlist[self.varnum[pre + "neurots"][0]] = int("".join([str(x) for x in neuro.getTSVector()]))
            xx = sum([int(i != 0) for i in neuro.getTSVector()][::2])
            if xx is None:
                xx = 0
            evlist[self.varnum[pre + "neuroats"][0]] = xx
            evlist[self.varnum[pre + "neuroexp"][0]] = neuro.getExpert()
            evlist[self.varnum[pre + "neurodriftth"][0]] = int("".join([str(int(x)) for x in neuro.getDriftThreshold()]))
            evlist[self.varnum[pre + "neuroquad"][0]] = neuro.getQuadrant()
            fpt = 9999
            for ts in neuro.getRelationsTo(self.tsname):
                if ts.priorityTime() < fpt:
                    fpt = ts.priorityTime()
            if self.etf.hasBinnedEventT0(Belle2.Const.CDC):
                eft = self.etf.getBinnedEventT0(Belle2.Const.CDC)
            else:
                eft = None

            # overwrite the etf temporarily with the etfcc

            evlist[self.varnum[pre + "neurofp"][0]] = fpt
            evlist[self.varnum[pre + "neuroetf"][0]] = eft
            if pre != "sw":
                evlist[self.varnum[pre + "neuroetfcc"][0]] = neuro.getETF_unpacked()
                evlist[self.varnum[pre + "neurohwtime"][0]] = neuro.getETF_recalced()
        return evlist

    def gettwodvals(self, evlist, twod):
        if twod:
            evlist[self.varnum["twodphi"][0]] = twod.getPhi0()
            evlist[self.varnum["twodpt"][0]] = twod.getPt()
            # evlist[self.varnum["twodfot"][0]] = int(twod.getFoundOldTrack())
        return evlist

    def getswtwodvals(self, evlist, twod):
        if twod:
            evlist[self.varnum["swtwodphi"][0]] = twod.getPhi0()
            evlist[self.varnum["swtwodpt"][0]] = twod.getPt()
            # evlist[self.varnum["twodfot"][0]] = int(twod.getFoundOldTrack())
        return evlist

    def event(self):
        from ROOT import Belle2, TVector3, XYZVector  # noqa
        # TODO: update the plots every nth time
        # if self.showplots != 0:
        #     if eventnumber % self.showplots = 0:
        #         show plots

        # loop over events
        event = []
        for reco in self.recotracks:
            track = reco.getRelatedFrom("Tracks")

            fitres = None
            state = None

            # method should be either 'old' for the old method or anything else for the new one
            method = 'old'

            if method == 'old':
                # # old way: ########################################################################

                if not track:
                    print("no track found for recotrack")
                    continue
                whishPdg = 211  # pion
                fitres = track.getTrackFitResultWithClosestMass(Belle2.Const.ChargedStable(whishPdg))
                if not fitres:
                    continue
            else:
                # # new way: ########################################################################

                reps = reco.getRepresentations()
                irep = 0
                for irep, rep in enumerate(reps):
                    if not reco.wasFitSuccessful(rep):
                        continue
                    try:
                        state = reco.getMeasuredStateOnPlaneClosestTo(XYZVector(0, 0, 0), rep)
                        rep.extrapolateToLine(state, TVector3(0, 0, -1000), TVector3(0, 0, 2000))
                    except BaseException:
                        continue
                if not state:
                    continue

            ####################################################################################

            neuro = reco.getRelatedTo(self.neurotracksname)
            event.append(self.nonelist.copy())
            try:
                neuro = reco.getRelatedTo(self.neurotracksname)
            except BaseException:
                neuro = None
            try:
                swneuro = reco.getRelatedTo(self.swneurotracksname)
            except BaseException:
                swneuro = None
            try:
                hwneuro = neuro.getRelatedFrom(self.hwneurotracksname)
            except BaseException:
                hwneuro = None
            try:
                twod = reco.getRelatedTo(self.twodtracksname)
            except BaseException:
                twod = None
            if method == 'old':
                event[-1] = self.getrecovalsold(event[-1], fitres)
            else:
                event[-1] = self.getrecovals(event[-1], state)
            event[-1] = self.getneurovals(event[-1], neuro)
            event[-1] = self.gettwodvals(event[-1], twod)
            event[-1] = self.getneurovals(event[-1], hwneuro, status="hw")
            event[-1] = self.getneurovals(event[-1], swneuro, status="sw")

        for neuro in self.neurotracks:
            # print("neuroloop")
            # print(len(neuro.getRelationsFrom(self.recotracksname)))
            if len(neuro.getRelationsFrom(self.recotracksname)) > 0:
                # this track is already stored in a recoline
                # print("skipping...")
                continue
            event.append(self.nonelist.copy())
            try:
                twod = reco.getRelatedTo(self.twodtracksname)
            except BaseException:
                twod = None
            try:
                hwneuro = neuro.getRelatedFrom(self.hwneurotracksname)
            except BaseException:
                hwneuro = None
            event[-1] = self.getneurovals(event[-1], neuro)
            event[-1] = self.gettwodvals(event[-1], twod)
            event[-1] = self.getneurovals(event[-1], hwneuro, status="hw")
        for swneuro in self.swneurotracks:
            # print("neuroloop")
            # print(len(neuro.getRelationsFrom(self.recotracksname)))
            if len(swneuro.getRelationsFrom(self.recotracksname)) > 0:
                # this track is already stored in a recoline
                # print("skipping...")
                continue
            event.append(self.nonelist.copy())
            try:
                swtwod = reco.getRelatedTo(self.swtwodtracksname)
            except BaseException:
                swtwod = None
            event[-1] = self.getneurovals(event[-1], swneuro, status="sw")
            event[-1] = self.getswtwodvals(event[-1], swtwod)
        for twod in self.twodtracks:
            # print("twodloop")
            # print(len(twod.getRelationsFrom(self.neurotracksname)))
            if len(twod.getRelationsFrom(self.neurotracksname)) > 0:
                # print("skipping...")
                # this track is already stored in a recoline or twodline
                continue
            event.append(self.nonelist.copy())
            event[-1] = self.gettwodvals(event[-1], twod)
        for swtwod in self.swtwodtracks:
            # print("twodloop")
            # print(len(twod.getRelationsFrom(self.neurotracksname)))
            if len(swtwod.getRelationsFrom(self.swneurotracksname)) > 0:
                # print("skipping...")
                # this track is already stored in a recoline or twodline
                continue
            event.append(self.nonelist.copy())
            event[-1] = self.getswtwodvals(event[-1], swtwod)

        # attach an array for every event
        if len(event) > self.maxtracks:
            event = event[0:self.maxtracks]
        elif len(event) < self.maxtracks:
            for i in range(self.maxtracks - len(event)):
                event.append(self.nonelist.copy())
        self.eventlist.append(event)

    def terminate(self):
        # self.eventfilters()
        # self.makearray(self.eventlist)
        # convert eventlist to data array
        # initialize histograms and fill them
        # both save histograms to file and show them in the plots
        self.save()

    def save(self, filename=None, netname=None, dataname=None):
        if not filename:
            filename = self.filename
        if not netname:
            netname = self.netname
        if not dataname:
            dataname = self.dataname
        # save the dataset as an array, the corresponding varnum,
        # and a description about the dataset into a pickle file
        savedict = {}
        savedict["eventlist"] = self.eventlist
        savedict["varnum"] = self.varnum
        savedict["networkname"] = netname
        savedict["dataname"] = dataname
        savedict["version"] = nntd.version
        f = open(filename, 'wb')
        pickle.dump(savedict, f)
        f.close()
        print('file ' + filename + ' has been saved. ')

    def loadmore(self, filenames):
        # first, check the amount of events and limit them to NNTD_EVLIMIT
        evlim = 0
        evnumber = 0
        skipev = 0
        if "NNTD_EVLIMIT" in os.environ:
            evlim = int(os.environ["NNTD_EVLIMIT"])
        else:
            evlim = 50000
        for i, x in enumerate(filenames):
            print("checking file: " + str(i) + "/" + str(len(filenames)))
            f = open(x, 'rb')
            evnumber += len(pickle.load(f)["eventlist"])
        if evnumber > evlim:
            print("total number of available events is " + str(evnumber))
            skipev = int(evnumber/evlim)
            print("Number of events more than " + str(evlim) + " only taking every " + str(skipev) + " event")
        else:
            skipev = 1
        for x in filenames:
            f = open(x, 'rb')
            savedict = pickle.load(f)
            f.close()
            if self.version != savedict["version"]:
                print("Error! loaded file was made with different version of nntd! exiting ... ")
                exit()
            self.networkname = savedict["networkname"]
            if "dataname" in savedict:
                self.dataname = savedict["dataname"]
            templim = evlim-len(self.eventlist)
            self.eventlist += savedict["eventlist"][::skipev][:templim]
            self.varnum = savedict["varnum"]
            print("Loaded file: " + x)
            print("length of eventlist: " + str(len(self.eventlist)))
            if evlim <= len(self.eventlist):
                print("stop loading, maximum event number reached")
                break
        self.makearray(self.eventlist)
        print("all files loaded, array.size: " + str(self.data.size) + ", array.shape: " + str(self.data.shape))

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
        self.dataname = savedict["dataname"]
        # self.eventfilters()
        self.makearray(self.eventlist)

    def makearray(self, evlist):
        import numpy as np  # noqa
        # TODO: apply filters
        self.data = np.array(evlist)
