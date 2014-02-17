#!/usr/bin/env python
# -*- coding: utf-8 -*-

from ROOT import gSystem
gSystem.Load('libframework')  # for PyStoreArray
gSystem.Load('libcdc')  # for CDCSimHit
gSystem.Load('libtracking')  # for CDCWire and so on
gSystem.Load('libgenfit2')  # for GFTrackCands

gSystem.Load('libdataobjects')

from ROOT import Belle2  # make Belle2 namespace available
from ROOT import std
from ROOT import genfit

import colorsys

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


class CDCHitStrokeWidthMap:

    def __call__(self, iCDCHit, cdcHit):
        return 0.2


class ZeroDriftLengthStrokeWidthMap(CDCHitStrokeWidthMap):

    def __call__(self, iCDCHit, cdcHit):
        wirehit = Belle2.CDCLocalTracking.CDCWireHit(cdcHit, 0)
        if wirehit.getRefDriftLength() == 0.0:
            return 1
        else:
            return 0.2


class CDCHitColorMap:

    bkgHitColor = 'orange'

    def __call__(self, iCDCHit, cdcHit):
        return self.bkgHitColor


class ZeroDriftLengthColorMap(CDCHitColorMap):

    def __call__(self, iCDCHit, cdcHit):
        wirehit = Belle2.CDCLocalTracking.CDCWireHit(cdcHit, 0)
        if wirehit.getRefDriftLength() == 0.0:
            return 'red'
        else:
            return self.bkgHitColor


class PosFlagColorMap(CDCHitColorMap):

    def __call__(self, iCDCHit, cdcHit):
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


class RLColorMap(CDCHitColorMap):

    def __call__(self, iCDCHit, cdcHit):
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


class MCSegmentIdColorMap(CDCHitColorMap):

    def __call__(self, iCDCHit, cdcHit):
        mcHitLookUp = Belle2.CDCLocalTracking.CDCMCHitLookUp.getInstance()
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

    def __call__(self, iCDCHit, cdcHit):
        simHit = cdcHit.getRelated('CDCSimHits')
        timeOfFlight = simhit.getFlightTime()

        # values are all fractions of their respective scale
        hue = 36 * timeOfFlight % 360.0 / 360.0
        saturation = 0.75
        lightness = 0.5

        (red, green, blue) = colorsys.hls_to_rgb(hue, lightness, saturation)

        color = 'rgb({0:.2%}, {1:.2%}, {2:.2%})'.format(red, green, blue)
        return color


class ReassignedSecondaryMap(CDCHitColorMap):

    def __call__(self, iCDCHit, cdcHit):
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

    def __init__(self):
        self.color_by_mcparticleId = {-1: self.bkgHitColor}

    def __call__(self, iCDCHit, cdcHit):

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

    missing_pdg_color = 'lime'

    def __init__(self):
        pass

    def __call__(self, iCDCHit, cdcHit):

        mcParticle = cdcHit.getRelated('MCParticles')
        if mcParticle:
            pdgcode = mcParticle.getPDG()
        else:

            # getSecondaryPhysicsProcess()
            pdgcode = -999

        if pdgcode in self.color_by_pdgcode:
            color = self.color_by_pdgcode[pdgcode]
        else:
            print 'Unknown PDG code', pdgcode
            color = self.missing_pdg_color

        return color

    def __str__(self):
        legend_head = 'Legend:\n'

        pdg_code_by_color = {}

        for (pdgcode, color) in self.color_by_pdgcode.items():
            pdg_code_by_color.setdefault(color, [])
            pdg_code_by_color[color].append(pdgcode)

        legend_content = '\n'.join(str(color) + '->'
                                   + str(pdg_code_by_color[color])
                                   for color in pdg_code_by_color)

        return legend_head + legend_content


class MCPrimaryColorMap(CDCHitColorMap):

    def __init__(self):
        self.n_hits_by_secondary_type = {}

    def __call__(self, iCDCHit, cdcHit):
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
        return """
Legend:
blue->primary
green->secondary decay in flight
red->secondary other process
orange->beam background
""" \
            + str(self.n_hits_by_secondary_type)


