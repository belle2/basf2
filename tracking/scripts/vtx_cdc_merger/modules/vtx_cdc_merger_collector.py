##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################


from pybasf2 import B2DEBUG
import basf2 as b2
from ROOT import Belle2
import pandas as pd
import numpy as np

from vtx_cdc_merger.var_set import extract_event_data


class VTXMergerCollector(b2.Module):
    """
    Module to collect training samples for the VTXMerger.
    """

    def __init__(
        self,
        VXDRecoTrackColName="VXDRecoTrackColName",
        CDCRecoTrackColName="CDCRecoTrackColName",
        outputdir="/home/benjamin/b2/vtx_cdc_merger_data"
    ):
        """Constructor"""
        super().__init__()

        #: cached name of the CDC RecoTracks StoreArray
        self.CDCRecoTrackColName = CDCRecoTrackColName

        #: cached name of VXD RecoTracks StoreArray
        self.VXDRecoTrackColName = VXDRecoTrackColName

        #: cached name of output directory with training data
        self.outputdir = outputdir

    def initialize(self):
        """Receive signal at the start of event processing"""

        cdcTracks = Belle2.PyStoreArray(self.CDCRecoTrackColName)
        vxdTracks = Belle2.PyStoreArray(self.VXDRecoTrackColName)

        vxdTracks.registerRelationTo(cdcTracks)

    def analyzeAndCleanTrackArray(
        self,
        recoTracks,
        mcParticles,
        trackMCParticles,
        trackMinToF,
    ):
        """
        Returns list with isFake labels for each track
        """

        isFake = []

        for recoTrack in recoTracks:

            contributingMCParticles = []
            nHits = recoTrack.getSortedCDCHitList().size() + \
                recoTrack.getSortedPXDHitList().size() + \
                recoTrack.getSortedSVDHitList().size() + \
                recoTrack.getSortedVTXHitList().size()

            # Minimum global time of all sim hits in track
            minGlobalTime = 1.0*10**9

            cdcHits = recoTrack.getSortedCDCHitList()
            for cdcHit in cdcHits:
                relatedMCParticles = cdcHit.getRelationsFrom("MCParticles")
                for i in range(relatedMCParticles.size()):
                    aParticle = relatedMCParticles.object(i)
                    contributingMCParticles.append(aParticle.getArrayIndex())

                relatedCDCSimHits = cdcHit.getRelationsFrom("CDCSimHits")
                for i in range(relatedCDCSimHits.size()):
                    aSimHit = relatedCDCSimHits.object(i)
                    tof = aSimHit.getGlobalTime()
                    if tof < minGlobalTime:
                        minGlobalTime = tof

            vtxHits = recoTrack.getSortedVTXHitList()
            for vtxHit in vtxHits:
                relatedMCParticles = vtxHit.getRelationsTo("MCParticles")
                for i in range(relatedMCParticles.size()):
                    aParticle = relatedMCParticles.object(i)
                    contributingMCParticles.append(aParticle.getArrayIndex())

                relatedVTXTrueHits = vtxHit.getRelationsTo("VTXTrueHits")
                for i in range(relatedVTXTrueHits.size()):
                    aTrueHit = relatedVTXTrueHits.object(i)
                    tof = aTrueHit.getGlobalTime()
                    if tof < minGlobalTime:
                        minGlobalTime = tof

            # Find inique mc particles and count there occuarance.
            stats = dict(zip(contributingMCParticles, [contributingMCParticles.count(i) for i in contributingMCParticles]))

            if len(stats) == 0:
                B2DEBUG(9, "No MC particle found => fake")
                trackMCParticles.append(-1)
                trackMinToF.append(minGlobalTime)
                recoTrack.setQualityIndicator(0.0)
                isFake.append(1.0)
            else:
                max_particle_id = max(stats.keys(), key=(lambda k: stats[k]))
                if float(stats[max_particle_id])/nHits < 0.66:
                    B2DEBUG(9, "Less than 66% of hits from same MCParticle => fake")
                    trackMCParticles.append(-1)
                    trackMinToF.append(minGlobalTime)
                    recoTrack.setQualityIndicator(0.0)
                    isFake.append(1.0)
                else:
                    trackMCParticles.append(max_particle_id)
                    trackMinToF.append(minGlobalTime - mcParticles[max_particle_id].getProductionTime())
                    recoTrack.setQualityIndicator(1.0)
                    isFake.append(0.0)

        return isFake

    def removeCurlersFromTrackArray(
        self,
        recoTracks,
        tracksMCParticles,
        tracksMinToF,
        relatedTracksColumnName,
    ):

        # Sparse list containing indices of curling tracks
        isCurler1, isCurler2 = [], []

        for recoTrack in recoTracks:

            # get index of matched MCParticle
            trackMCParticle_1 = tracksMCParticles[recoTrack.getArrayIndex()]

            # get min tof of track
            trackMinTof_1 = tracksMinToF[recoTrack.getArrayIndex()]

            for recoTrack2 in recoTracks:

                # skip self connections
                if recoTrack.getArrayIndex() == recoTrack2.getArrayIndex():
                    continue

                # get index of matched MCParticle
                trackMCParticle_2 = tracksMCParticles[recoTrack2.getArrayIndex()]

                # get min tof of track
                trackMinTof_2 = tracksMinToF[recoTrack2.getArrayIndex()]

                if (trackMCParticle_2 == trackMCParticle_1) and (trackMCParticle_1 >= 0):

                    if trackMinTof_2 < trackMinTof_1:
                        if recoTrack.getQualityIndicator() > 0:
                            recoTrack.setQualityIndicator(0)
                            isCurler1.append(recoTrack.getArrayIndex())

                            # Also remove the first related track
                            if recoTrack.getRelated(relatedTracksColumnName):
                                recoTrack.getRelated(relatedTracksColumnName).setQualityIndicator(0)
                                isCurler2.append(recoTrack.getRelated(relatedTracksColumnName).getArrayIndex())

                    else:
                        if recoTrack2.getQualityIndicator() > 0:
                            recoTrack2.setQualityIndicator(0)
                            isCurler2.append(recoTrack2.getArrayIndex())

                            # Also remove the first related track
                            if recoTrack2.getRelated(relatedTracksColumnName):
                                recoTrack2.getRelated(relatedTracksColumnName).setQualityIndicator(0)
                                isCurler1.append(recoTrack2.getRelated(relatedTracksColumnName).getArrayIndex())

        return isCurler1, isCurler2

    def mergeVXDAndCDCTrackArrays(
        self,
        cdcTracks,
        cdcTrackMCParticles,
        cdcTrackMinToF,
        vxdTracks,
        vxdTrackMCParticles,
        vxdTrackMinToF,
    ):

        isRelatedVXDCDC = []

        for cdcTrack in cdcTracks:
            B2DEBUG(9, "Match with CDCTrack at {}".format(cdcTrack.getArrayIndex()))

            # get index of matched MCParticle
            cdcMCParticle = cdcTrackMCParticles[cdcTrack.getArrayIndex()]

            # get min tof of CDC track
            cdcMinTof = cdcTrackMinToF[cdcTrack.getArrayIndex()]

            # skip CDC Tracks with bad quality indicator
            if cdcTrack.getQualityIndicator() == 0.0:
                continue

            B2DEBUG(9, "Good Quality ")

            # skip if CDC track has already a correct match
            cdcHasGoodRelation = False
            relatedVXDRecoTracks = cdcTrack.getRelationsWith(self.VXDRecoTrackColName)

            offset = 0
            initialSize = relatedVXDRecoTracks.size()

            for index in range(initialSize):

                relatedIndex = relatedVXDRecoTracks[index - offset].getArrayIndex()
                relatedQI = relatedVXDRecoTracks[index - offset].getQualityIndicator()
                relatedToF = vxdTrackMinToF[relatedIndex]

                if ((vxdTrackMCParticles[relatedIndex] == cdcMCParticle) and
                    (cdcMCParticle >= 0) and
                    (relatedQI > 0.0) and
                        (relatedToF < cdcMinTof)):

                    if relatedVXDRecoTracks.weight(index - offset) > 0:
                        relatedVXDRecoTracks.setWeight(index - offset, -1)

                    cdcHasGoodRelation = True
                    isRelatedVXDCDC.append((relatedIndex, cdcTrack.getArrayIndex()))
                else:
                    # Need to remove the bad relation
                    relatedVXDRecoTracks.remove(index - offset)
                    offset += 1

            if cdcHasGoodRelation:
                continue

            B2DEBUG(9, "Not yet related ")

            matched_track = False
            currentVxdTrack = -1
            bestMatchedVxdTrack = 0

            for vxdTrack in vxdTracks:

                # get index of matched MCParticle
                vxdMCParticle = vxdTrackMCParticles[vxdTrack.getArrayIndex()]

                # get min tof of VXD track
                vxdMinTof = vxdTrackMinToF[vxdTrack.getArrayIndex()]

                B2DEBUG(9, "Compare with  {}".format(vxdTrack.getArrayIndex()))
                currentVxdTrack += 1

                # skip VXD if it has already a correct match
                vxdHasGoodRelation = False
                relatedCDCRecoTracks = vxdTrack.getRelationsWith(self.CDCRecoTrackColName)

                offsetCDC = 0
                initialSizeCDC = relatedCDCRecoTracks.size()
                for index in range(initialSizeCDC):
                    relatedIndexCDC = relatedCDCRecoTracks[index - offsetCDC].getArrayIndex()
                    relatedQICDC = relatedCDCRecoTracks[index - offsetCDC].getQualityIndicator()
                    relatedToFCDC = cdcTrackMinToF[relatedIndexCDC]

                    if ((cdcTrackMCParticles[relatedIndexCDC] == vxdMCParticle) and
                        (vxdMCParticle >= 0) and
                        (relatedQICDC > 0.0) and
                            (vxdMinTof < relatedToFCDC)):

                        vxdHasGoodRelation = True
                        isRelatedVXDCDC.append((vxdTrack.getArrayIndex(), relatedIndexCDC))

                    else:
                        # Need to remove the bad relation
                        relatedCDCRecoTracks.remove(index - offsetCDC)
                        offsetCDC += 1

                if vxdHasGoodRelation:
                    continue

                if ((vxdTrackMCParticles[vxdTrack.getArrayIndex()] == cdcTrackMCParticles[cdcTrack.getArrayIndex()]) and
                    (vxdTrackMCParticles[vxdTrack.getArrayIndex()] >= 0) and
                        (vxdMinTof < cdcMinTof)):
                    matched_track = True
                    bestMatchedVxdTrack = currentVxdTrack

            if matched_track:
                # -1 is the convention for "before the CDC track" in the related tracks combiner
                vxdTracks[bestMatchedVxdTrack].addRelationTo(cdcTrack, -1)
                isRelatedVXDCDC.append((vxdTracks[bestMatchedVxdTrack].getArrayIndex(), cdcTrack.getArrayIndex()))

        # make it unique
        isRelatedVXDCDC = list(set(isRelatedVXDCDC))
        return isRelatedVXDCDC

    def event(self):
        """Event method"""

        cdcTracks = Belle2.PyStoreArray(self.CDCRecoTrackColName)
        vxdTracks = Belle2.PyStoreArray(self.VXDRecoTrackColName)
        mcparticles = Belle2.PyStoreArray("MCParticles")

        # Find a MCParticle for each track candidate
        vxdTrackMCParticles = []
        cdcTrackMCParticles = []

        # Find minimum time of flight for each track candidate
        vxdTrackMinToF = []
        cdcTrackMinToF = []

        B2DEBUG(9, "Clean VXD tracks from fakes")
        isFakeVXD = self.analyzeAndCleanTrackArray(
            vxdTracks,
            mcparticles,
            vxdTrackMCParticles,
            vxdTrackMinToF,
        )

        B2DEBUG(9, "Clean CDC tracks from fakes")
        isFakeCDC = self.analyzeAndCleanTrackArray(
            cdcTracks,
            mcparticles,
            cdcTrackMCParticles,
            cdcTrackMinToF,
        )

        B2DEBUG(9, "Merging  CDC to VXD tracks, where VXD track will be added before CDC track")
        isRelatedVXDCDC = self.mergeVXDAndCDCTrackArrays(
            cdcTracks,
            cdcTrackMCParticles,
            cdcTrackMinToF,
            vxdTracks,
            vxdTrackMCParticles,
            vxdTrackMinToF
        )

        B2DEBUG(9, "Removing curlers in VXD => treat as fake")
        isCurlerVXD2, isCurlerCDC2 = self.removeCurlersFromTrackArray(
            vxdTracks,
            vxdTrackMCParticles,
            vxdTrackMinToF,
            self.CDCRecoTrackColName,
        )

        B2DEBUG(9, "Removing curlers in CDC => treat as fake")
        isCurlerCDC, isCurlerVXD = self.removeCurlersFromTrackArray(
            cdcTracks,
            cdcTrackMCParticles,
            cdcTrackMinToF,
            self.VXDRecoTrackColName,
        )

        isCurlerVXD.extend(isCurlerVXD2)
        isCurlerCDC.extend(isCurlerCDC2)

        isCurlerCDC = list(set(isCurlerCDC))
        isCurlerVXD = list(set(isCurlerVXD))

        n_vxd = len(isFakeVXD)
        n_cdc = len(isFakeCDC)

        # Selects good VXD tracks: no fake and no curler
        true_vxd = np.ones(n_vxd)
        for i_vxd in range(n_vxd):
            if isFakeVXD[i_vxd] == 1.0:
                true_vxd[i_vxd] = 0.0

            if i_vxd in isCurlerVXD:
                true_vxd[i_vxd] = 0.0

        # Selects good CDC tracks: no fake and no culer
        true_cdc = np.ones(n_cdc)
        for i_cdc in range(n_cdc):
            if isFakeCDC[i_cdc] == 1.0:
                true_cdc[i_cdc] = 0.0

            if i_cdc in isCurlerCDC:
                true_cdc[i_cdc] = 0.0

        # Selects good links from VXD to CDC
        # VXD and CDC tracks numbered by array index
        # Edges always go from VXD to CDC, first N_CDC edges connect first VXD
        # track to all N_CDC tracks in CDC
        true_links = np.zeros(n_vxd * n_cdc)
        for edge in isRelatedVXDCDC:
            i_vxd, i_cdc = edge
            if isFakeVXD[i_vxd] == 1.0:
                continue
            if isFakeCDC[i_cdc] == 1.0:
                continue

            if i_vxd in isCurlerVXD:
                continue
            if i_cdc in isCurlerCDC:
                continue

            true_links[i_cdc+i_vxd*n_cdc] = 1.0

        true_vxd = pd.DataFrame(true_vxd, columns=['true_vxd'])
        true_cdc = pd.DataFrame(true_cdc, columns=['true_cdc'])
        true_links = pd.DataFrame(true_links, columns=['true_links'])

        cdc_hits, cdc_tracks, vxd_hits, vxd_tracks = extract_event_data(cdcTracks=cdcTracks, vxdTracks=vxdTracks)

        # Write all collected training data to HDF5 file
        event_meta_data = Belle2.PyStoreObj("EventMetaData")
        evtid = event_meta_data.getEvent() - 1  # first evtid is zero

        filename = self.outputdir + '/{}_id_{}.h5'.format("event", evtid)
        vxd_hits.to_hdf(filename, key='vxd_hits')
        cdc_hits.to_hdf(filename, key='cdc_hits')
        cdc_tracks.to_hdf(filename, key='cdc_tracks')
        vxd_tracks.to_hdf(filename, key='vxd_tracks')

        true_vxd.to_hdf(filename, key='true_vxd')
        true_cdc.to_hdf(filename, key='true_cdc')
        true_links.to_hdf(filename, key='true_links')
