#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

from ROOT import gSystem

from ROOT import Belle2  # make Belle2 namespace available

import bisect
import colorsys

gSystem.Load('libframework')  # for PyStoreArray
gSystem.Load('libcdc')  # for CDCSimHit
gSystem.Load('libtracking')  # for CDCWire and so on
gSystem.Load('libgenfit2')  # for GFTrackCands
gSystem.Load('libdataobjects')
# Standard color map for id types
listColors = [  # 'magenta',
                # 'gold',
                # 'yellow',
                # 'aquamarine',
    'red',
    'blue',
    'green',
    'orange',
    'cyan',
    'olive',
    'lime',
    'maroon',
    'tomato',
    'turquoise',
    'mediumspringgreen',
    'darkgreen',
    'indigo',
]


def timeOfFlightToColor(timeOfFlight):
    """
    Translates the given floating point time of flight to a color.
    """

    # values are all fractions of their respective scale

    # Full color circle in 3 nanoseconds
    hue = 360 / 3.0 * timeOfFlight % 360.0 / 360.0
    saturation = 0.75
    lightness = 0.5

    (red, green, blue) = colorsys.hls_to_rgb(hue, lightness, saturation)

    color = 'rgb({0:.2%}, {1:.2%}, {2:.2%})'.format(red, green, blue)
    return color


def inTrackIdToColor(inTrackId):
    """
    Translates the given integer in track id to a color.
    """

    hue = 50 * inTrackId % 360 / 360.0
    saturation = 0.75
    lightness = 0.5

    (red, green, blue) = colorsys.hls_to_rgb(hue, lightness, saturation)

    color = 'rgb({0:.2%}, {1:.2%}, {2:.2%})'.format(red, green, blue)
    return color


class CDCHitStrokeWidthMap:

    """
    Base class for CDCHit to the stroke width map functional objects.
    """

    def __call__(self, iCDCHit, cdcHit):
        """
        Function call to map the CDCHit id and object to a stroke width.
        """

        return 0.2


class ZeroDriftLengthStrokeWidthMap(CDCHitStrokeWidthMap):

    """
    CDCHit to stroke width map highlighting the CDCHits with 0 drift length.
    """

    def __call__(self, iCDCHit, cdcHit):
        """
        Function call to map the CDCHit id and object to a stroke width.
        """

        wirehit = Belle2.TrackFindingCDC.CDCWireHit(cdcHit)
        if wirehit.getRefDriftLength() == 0.0:
            return 1
        else:
            return 0.2


class CDCHitColorMap:

    """
    Base class for CDCHit to color map functional objects.
    """

    #: Default color to be used
    bkgHitColor = 'orange'

    def __call__(self, iCDCHit, cdcHit):
        """
        Function call to map the CDCHit id and object to a color.
        """

        return self.bkgHitColor


class ZeroDriftLengthColorMap(CDCHitColorMap):

    """
    CDCHit to color map highlighting the CDCHits with 0 drift length.
    """

    def __call__(self, iCDCHit, cdcHit):
        """
        Function call to map the CDCHit id and object to a color.
        """

        wirehit = Belle2.TrackFindingCDC.CDCWireHit(cdcHit)
        if wirehit.getRefDriftLength() == 0.0:
            return 'red'
        else:
            return self.bkgHitColor


class TakenFlagColorMap(CDCHitColorMap):

    """
    CDCHit to color map highlighting the CDCHits that posses the do not use flag.
    """

    def __init__(self):
        """Constructor"""
        super().__init__()
        #: cached copy of the CDCWireHitVector
        self.storedWireHits = Belle2.PyStoreObj('CDCWireHitVector')
        if not self.storedWireHits:
            print('Could not find CDCWireHitVector in the data store to lookup TakenFlag')

    def __call__(self, iCDCHit, cdcHit):
        """
        Function call to map the CDCHit id and object to a color.
        """
        if not self.storedWireHits:
            return self.bkgHitColor

        wireHits = self.storedWireHits.obj().get()
        # Search the sorted range of wire hits for the one corresponding to the given CDCHit
        wireHit = wireHits.at(bisect.bisect_left(wireHits, cdcHit))
        if wireHit.getAutomatonCell().hasTakenFlag():
            return 'red'
        else:
            return self.bkgHitColor


class RLColorMap(CDCHitColorMap):

    """
    CDCHit to color map by their local right left passage information from Monte Carlo truth
    """

    def __call__(self, iCDCHit, cdcHit):
        """
        Function call to map the CDCHit id and object to a color.
        """

        mcHitLookUp = Belle2.TrackFindingCDC.CDCMCHitLookUp.getInstance()
        rlInfo = mcHitLookUp.getRLInfo(cdcHit)
        if rlInfo == 1:
            # Right
            return 'green'
        elif rlInfo == -1 or rlInfo == 65535:  # <- The root interface mistakes the signed enum value for an unsigned value
            # Left
            return 'red'
        else:
            self.bkgHitColor

    def __str__(self):
        """
        Informal string summarizing the translation from right left passage variable to colors.
        """

        return 'Local right left passage variable: green <-> right, red <-> left, orange <-> not determinable.'


class WrongRLColorMap():

    """
    CDCRecoHit3D to color map for the correctness of the rl information
    """

    def __init__(self):
        """Constructor"""
        mcHitLookUp = Belle2.TrackFindingCDC.CDCMCHitLookUp.getInstance()
        mcHitLookUp.fill()

    def __call__(self, iCDCRecoHit, cdcRecoHit3D):
        """
        This function maps the cdcRecoHit3D to the color which inidcated the correctness of the rl passage
        """

        cdcHit = cdcRecoHit3D.getWireHit().getHit()

        mcHitLookUp = Belle2.TrackFindingCDC.CDCMCHitLookUp.getInstance()
        rlInfo = mcHitLookUp.getRLInfo(cdcHit)

        if rlInfo == -32768 or rlInfo == 32768:   # <- The root interface mistakes the signed enum value for an unsigned value
            return 'violet'
        elif rlInfo == cdcRecoHit3D.getRLInfo():
            return 'green'
        else:
            return 'red'

    def __str__(self):
        """
        Informal string summarizing the translation from right left passage variable to colors.
        """
        return 'Correct RL info: gree, wrong RL info: red'


class PosFlagColorMap(CDCHitColorMap):

    """
    CDCHit to color map by their assoziated CDCSimHit::getPosFlag property.
    """

    def __call__(self, iCDCHit, cdcHit):
        """
        Function call to map the CDCHit id and object to a color.
        """

        simHit = cdcHit.getRelated('CDCSimHits')
        posFlag = simHit.getPosFlag()
        if posFlag == 0:
            # Right
            return 'green'
        elif posFlag == 1:
            # Left
            return 'red'
        else:
            self.bkgHitColor

    def __str__(self):
        """
        Informal string summarizing the translation from CDCSimHit::getPosFlag variable to colors.
        """

        return 'PosFlag variable of the related CDCSimHit: green <-> 0 (Right), red <-> 1 (Left), orange <-> determinable.'


class BackgroundTagColorMap(CDCHitColorMap):

    """
    CDCHit to color map by their assoziated CDCSimHit::getBackgroundTag property.
    """

    #: access the SimHitBase enum of background tags
    BackgroundMetaData = Belle2.BackgroundMetaData

    #: dictionary of (tag, label) pairs
    bkgname_by_bkgtag = {
        BackgroundMetaData.bg_none: 'bg_none',
        BackgroundMetaData.bg_Coulomb_LER: 'bg_Coulomb_LER',
        BackgroundMetaData.bg_Coulomb_HER: 'bg_Coulomb_HER',
        BackgroundMetaData.bg_RBB_LER: 'bg_RBB_LER',
        BackgroundMetaData.bg_RBB_HER: 'bg_RBB_HER',
        BackgroundMetaData.bg_Touschek_LER: 'bg_Touschek_LER',
        BackgroundMetaData.bg_Touschek_HER: 'bg_Touschek_HER',
        BackgroundMetaData.bg_twoPhoton: 'bg_twoPhoton',
        BackgroundMetaData.bg_RBB_gamma: 'bg_RBB_gamma',
        BackgroundMetaData.bg_RBB_LER_far: 'bg_RBB_LER_far',
        BackgroundMetaData.bg_RBB_HER_far: 'bg_RBB_HER_far',
        BackgroundMetaData.bg_Touschek_LER_far: 'bg_Touschek_LER_far',
        BackgroundMetaData.bg_Touschek_HER_far: 'bg_Touschek_HER_far',
        BackgroundMetaData.bg_SynchRad_LER: 'bg_SynchRad_LER',
        BackgroundMetaData.bg_SynchRad_HER: 'bg_SynchRad_HER',
        BackgroundMetaData.bg_other: 'bg_other',
    }

    #: dictionary of (tag, color) pairs
    color_by_bkgtag = {
        BackgroundMetaData.bg_none: 'orange',
        BackgroundMetaData.bg_Coulomb_LER: 'red',
        BackgroundMetaData.bg_Coulomb_HER: 'darkred',
        BackgroundMetaData.bg_RBB_LER: 'blue',
        BackgroundMetaData.bg_RBB_HER: 'darkblue',
        BackgroundMetaData.bg_Touschek_LER: 'green',
        BackgroundMetaData.bg_Touschek_HER: 'darkgreen',
        BackgroundMetaData.bg_twoPhoton: 'violet',
        BackgroundMetaData.bg_RBB_gamma: 'skyblue',
        BackgroundMetaData.bg_RBB_LER_far: 'turquoise',
        BackgroundMetaData.bg_RBB_HER_far: 'darkturquoise',
        BackgroundMetaData.bg_Touschek_LER_far: 'olivergreen',
        BackgroundMetaData.bg_Touschek_HER_far: 'darkolivegreen',
        BackgroundMetaData.bg_SynchRad_LER: 'goldenrod',
        BackgroundMetaData.bg_SynchRad_HER: 'darkgoldenrod',
        BackgroundMetaData.bg_other: 'orange',
    }

    def __call__(self, iCDCHit, cdcHit):
        """
        Function call to map the CDCHit id and object to a color.
        """

        cdcSimHit = cdcHit.getRelated('CDCSimHits')
        backgroundTag = cdcSimHit.getBackgroundTag()

        color = self.color_by_bkgtag.get(backgroundTag, None)

        if color is None:
            print('Background tag %s not associated with a color.'
                  % backgroundTag)
            return 'orange'
        else:
            return color

    def __str__(self):
        """
        Informal string summarizing the translation from CDCSimHit::getBackgroundTag variable to colors.
        """

        color_by_bkgname = {}

        for backgroundTag in self.bkgname_by_bkgtag:
            name = self.bkgname_by_bkgtag[backgroundTag]
            color = self.color_by_bkgtag[backgroundTag]
            color_by_bkgname[name] = color

        bkgname_and_color = sorted(color_by_bkgname.items())

        message = 'Background tag color coding is \n%s' % '\n'.join(name + ' -> ' + color for (name, color) in bkgname_and_color)
        return message


class MCSegmentIdColorMap(CDCHitColorMap):

    """
    CDCHit to color map by their Monte Carlo segment id
    """

    def __call__(self, iCDCHit, cdcHit):
        """
        Function call to map the CDCHit id and object to a color.
        """

        mcHitLookUp = Belle2.TrackFindingCDC.CDCMCHitLookUp.getInstance()
        inTrackSegmentId = mcHitLookUp.getInTrackSegmentId(cdcHit)

        if inTrackSegmentId < 0:
            return self.bkgHitColor
        else:
            # values are all fractions of their respective scale
            hue = 50 * inTrackSegmentId % 360 / 360.0
            saturation = 0.75
            lightness = 0.5

            (red, green, blue) = colorsys.hls_to_rgb(hue, lightness,
                                                     saturation)

            color = 'rgb({0:.2%}, {1:.2%}, {2:.2%})'.format(red, green, blue)
            return color


class TOFColorMap(CDCHitColorMap):

    """
    CDCHit to color map by their assoziated CDCSimHit::getFlightTime.
    """

    def __call__(self, iCDCHit, cdcHit):
        """
        Function call to map the CDCHit id and object to a color.
        """

        simHit = cdcHit.getRelated('CDCSimHits')
        timeOfFlight = simHit.getFlightTime()

        return timeOfFlightToColor(timeOfFlight)


class ReassignedSecondaryMap(CDCHitColorMap):

    """
    CDCHit to color map indicating the reassignment to a different MCParticle.
    """

    def __call__(self, iCDCHit, cdcHit):
        """
        Function call to map the CDCHit id and object to a color.
        """

        relatedMCParticles = cdcHit.getRelationsWith('MCParticles')
        if relatedMCParticles.size() == 0:
            return self.bkgHitColor
        else:
            mcRelationWeight = relatedMCParticles.weight(0)
            if mcRelationWeight > 0:
                return 'green'
            else:
                return 'red'


class MCParticleColorMap(CDCHitColorMap):

    """
    CDCHit to color map coloring by the assoziated MCParticle::getArrayIndex()
    """

    def __init__(self):
        """
        Construction method setting up a Monte Carlo id to color dictionary which is continously filled
        as new during the event.
        """

        #: Dictionary mapping the MCParticle ids to colors for consistent and contious use of the available colors
        self.color_by_mcparticleId = {-1: self.bkgHitColor}

    def __call__(self, iCDCHit, cdcHit):
        """
        Function call to map the CDCHit id and object to a color.
        """

        mcParticle = cdcHit.getRelated('MCParticles')
        if mcParticle:
            mcParticleId = mcParticle.getArrayIndex()
        else:
            mcParticleId = -1

        # cdcSimHit = cdcHit.getRelated("CDCSimHits")
        # if cdcSimHit:
        #    cdcSimHitTrackId = cdcSimHit.getTrackId()
        # else:
        #    cdcSimHitTrackId = -1

        if mcParticleId in self.color_by_mcparticleId:
            color = self.color_by_mcparticleId[mcParticleId]
        else:
            iColor = len(self.color_by_mcparticleId)
            iColor = iColor % len(listColors)
            color = listColors[iColor]
            self.color_by_mcparticleId[mcParticleId] = color

        return color


class MCPDGCodeColorMap(CDCHitColorMap):

    """
    CDCHit to color map by the assoziated MCParticle::getPDG()
    """

    #: Dictionary to define the color for the most relevant
    color_by_pdgcode = {
        -999: CDCHitColorMap.bkgHitColor,
        11: 'blue',
        -11: 'blue',
        13: 'turquoise',
        -13: 'turquoise',
        15: 'cyan',
        -15: 'cyan',
        211: 'green',
        -211: 'green',
        321: 'olive',
        -321: 'olive',
        2212: 'red',
        -2212: 'red',
    }

    #: Color for the case a particle a pdg code not mentioned in the color_by_pdgcode map
    missing_pdg_color = 'lime'

    def __call__(self, iCDCHit, cdcHit):
        """
        Function call to map the CDCHit id and object to a color.
        """

        mcParticle = cdcHit.getRelated('MCParticles')
        if mcParticle:
            pdgcode = mcParticle.getPDG()
        else:

            # getSecondaryPhysicsProcess()
            pdgcode = -999

        if pdgcode in self.color_by_pdgcode:
            color = self.color_by_pdgcode[pdgcode]
        else:
            print('Unknown PDG code', pdgcode)
            color = self.missing_pdg_color

        return color

    def __str__(self):
        """
        Informal string summarizing the translation from pdg codes to colors.
        """

        legend_head = 'Legend:\n'

        pdg_code_by_color = {}

        for (pdgcode, color) in list(self.color_by_pdgcode.items()):
            pdg_code_by_color.setdefault(color, [])
            pdg_code_by_color[color].append(pdgcode)

        legend_content = '\n'.join(str(color) + '->' + str(pdg_code_by_color[color])
                                   for color in pdg_code_by_color)

        return legend_head + legend_content


class MCPrimaryColorMap(CDCHitColorMap):

    """
    CDCHit to color map by the isPrimary information as well as the secondary process type in case the particle is not primary.
    """

    def __init__(self):
        """
        Constuction method setting up a dictionary to count the hits for each secondary type.
        """

        #: Dictionary keeping track of the number of hits with a specific secondary process type.
        self.n_hits_by_secondary_type = {}

    def __call__(self, iCDCHit, cdcHit):
        """
        Function call to map the CDCHit id and object to a color.
        """

        mcParticle = cdcHit.getRelated('MCParticles')
        if mcParticle:
            primaryFlag = 1
            isPrimary = mcParticle.hasStatus(primaryFlag)
            secondaryProcess = mcParticle.getSecondaryPhysicsProcess()
            if secondaryProcess > 0:
                motherMCParticle = mcParticle.getMother()
                secondary_type = (motherMCParticle.getPDG(),
                                  mcParticle.getPDG())
            else:
                motherMCParticle = None
                secondary_type = (-999, mcParticle.getPDG())

            self.n_hits_by_secondary_type.setdefault(secondary_type, 0)
            self.n_hits_by_secondary_type[secondary_type] = \
                self.n_hits_by_secondary_type[secondary_type] + 1
            if isPrimary:
                return 'blue'
            elif secondaryProcess > 200:
                # decay in flight
                return 'green'
            else:
                return 'red'
        else:
            return self.bkgHitColor

    def __str__(self):
        """
        Informal string summarizing the translation from seconday process codes to colors.
        """

        return """
Legend:
blue->primary
green->secondary decay in flight
red->secondary other process
orange->beam background
""" \
            + str(self.n_hits_by_secondary_type)


class CDCSegmentColorMap:

    """
    Base class for Segments to color map functional objects.
    """

    #: Default color to be used
    bkgSegmentColor = 'orange'

    def __call__(self, iSegment, segment):
        """
        Function call to map a segments object from the local finder to a color.
        """

        return self.bkgSegmentColor


class SegmentMCTrackIdColorMap(CDCSegmentColorMap):

    """
    Segment to color map based on the matched MCTrackId
    """

    def __call__(self, iSegment, segment):
        """
        Function call to map a segments object from the local finder to a color.
        """

        mcSegmentLookUp = \
            Belle2.TrackFindingCDC.CDCMCSegment2DLookUp.getInstance()

        mcTrackId = mcSegmentLookUp.getMCTrackId(segment)
        if mcTrackId < 0:
            return self.bkgSegmentColor
        else:
            iColor = mcTrackId % len(listColors)
            color = listColors[iColor]
            return color


class SegmentFBInfoColorMap(CDCSegmentColorMap):

    """
    Segment to color map based on the forward or backward alignment relative to the match Monte Carlo track.
    """

    def __call__(self, iSegment, segment):
        """
        Function call to map a segments object from the local finder to a color.
        """

        mcSegmentLookUp = \
            Belle2.TrackFindingCDC.CDCMCSegment2DLookUp.getInstance()

        # Just to look at matched segments
        mcTrackId = mcSegmentLookUp.getMCTrackId(segment)
        if mcTrackId < 0:
            return self.bkgSegmentColor

        fbInfo = mcSegmentLookUp.isForwardOrBackwardToMCTrack(segment)
        if fbInfo == 1:
            return 'green'
        elif fbInfo == -1 or fbInfo == 65535:  # <- The root interface mistakes the signed enum value for an unsigned value
            return 'red'
        else:
            print('Segment not orientable to match track')
            return self.bkgSegmentColor


class SegmentFirstInTrackIdColorMap(CDCSegmentColorMap):

    """
    Segment to color map by the in track id of the first hit.
    """

    def __call__(self, iSegment, segment):
        """
        Function call to map a segments object from the local finder to a color.
        """

        mcSegmentLookUp = \
            Belle2.TrackFindingCDC.CDCMCSegment2DLookUp.getInstance()

        # Just to look at matched segments
        firstInTrackId = mcSegmentLookUp.getFirstInTrackId(segment)

        if firstInTrackId < 0:
            return self.bkgSegmentColor

        return inTrackIdToColor(firstInTrackId)


class SegmentLastInTrackIdColorMap(CDCSegmentColorMap):

    """
    Segment to color map by the in track id of the last hit.
    """

    def __call__(self, iSegment, segment):
        """
        Function call to map a segments object from the local finder to a color.
        """

        mcSegmentLookUp = \
            Belle2.TrackFindingCDC.CDCMCSegment2DLookUp.getInstance()

        # Just to look at matched segments
        lastInTrackId = mcSegmentLookUp.getLastInTrackId(segment)

        if lastInTrackId < 0:
            return self.bkgSegmentColor

        return inTrackIdToColor(lastInTrackId)


class SegmentFirstNPassedSuperLayersColorMap(CDCSegmentColorMap):

    """
    Segment to color map by the number of passed superlayers of the first hit.
    """

    def __call__(self, iSegment, segment):
        """
        Function call to map a segments object from the local finder to a color.
        """

        mcSegmentLookUp = \
            Belle2.TrackFindingCDC.CDCMCSegment2DLookUp.getInstance()

        # Just to look at matched segments
        firstNPassedSuperLayers = \
            mcSegmentLookUp.getFirstNPassedSuperLayers(segment)

        if firstNPassedSuperLayers < 0:
            return self.bkgSegmentColor

        return inTrackIdToColor(firstNPassedSuperLayers)


class SegmentLastNPassedSuperLayersColorMap(CDCSegmentColorMap):

    """
    Segment to color map by the number of passed superlayers of the last hit.
    """

    def __call__(self, iSegment, segment):
        """
        Function call to map a segments object from the local finder to a color.
        """

        mcSegmentLookUp = \
            Belle2.TrackFindingCDC.CDCMCSegment2DLookUp.getInstance()

        # Just to look at matched segments
        lastNPassedSuperLayers = \
            mcSegmentLookUp.getLastNPassedSuperLayers(segment)

        if lastNPassedSuperLayers < 0:
            return self.bkgSegmentColor

        return inTrackIdToColor(lastNPassedSuperLayers)
