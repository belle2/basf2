#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import Module

from ROOT import gSystem
gSystem.Load('libframework')  # for PyStoreArray
gSystem.Load('libcdc')  # for CDCSimHit
gSystem.Load('libtracking')  # for CDCHit and so on
gSystem.Load('libgenfit2')  # for GFTrackCands

from ROOT import Belle2  # make Belle2 namespace available
from ROOT import std
from ROOT import genfit

import subprocess
from datetime import datetime

import svgdrawing

import os
import os.path
import math

# from svgdrawing.attributemaps import *
import svgdrawing.attributemaps as attributemaps


class CDCSVGDisplayModule(Module):

    def __init__(self, output_folder, interactive=True):
        super(CDCSVGDisplayModule, self).__init__()
        # call constructor of base class, required!

        self.interactive = interactive
        self.output_folder = output_folder

        if not os.path.exists(output_folder):
            print "CDCSVGDisplay.__init__ : Output folder '", output_folder, \
                "' does not exist."
            answer = raw_input('Create it? (y or n)')
            if answer == 'y':
                os.makedirs(output_folder)

        self.draw_wires = True  # and False
        self.draw_wirehits = True and False
        self.draw_hits = True  # and False

        self.draw_mcparticles = True and False
        self.draw_mcpdgcodes = True and False
        self.draw_mcprimary = True and False
        self.draw_mcsegments = True and False

        self.draw_mcvertices = True and False

        self.draw_simhits = True and False
        self.draw_rlinfo = True and False
        self.draw_tof = True and False
        self.draw_reassigned = True and False

        self.draw_clusters = True and False

        self.draw_segments = True and False
        self.draw_tangentsegments = True and False

        self.draw_segments_mctrackid = True and False

        self.draw_segmenttriples = True and False
        self.draw_tracks = True and False

        self.draw_gftrackcands = True and False

        self.draw_segmenttriple_trajectories = True and False
        self.draw_segment_trajectories = True and False
        self.draw_gftrackcand_trajectories = True and False

        self.draw_superlayer_boundaries = True  # and False
        self.draw_interactionpoint = True  # and False

    @property
    def drawoptions(self):
        return [option for option in self.__dict__ if option.startswith('draw_'
                )]

    def initialize(self):
        print 'initialize()'

    def beginRun(self):
        print 'beginRun()'

    def draw_storearray(self, storearray_name, styleDict):
        print 'Drawing', storearray_name,
        print
        storearray = Belle2.PyStoreArray(storearray_name)
        if storearray:
            print 'with', storearray.getEntries(), 'entries'
            print 'Attributes are'
            for (key, value) in styleDict.items():
                print str(key), ':', str(value)

            self.plotter.append(storearray, **styleDict)
        else:

            print ' which is not present in the DataStore'

    def event(self):
        print '##################### DISPLAY EVENT ###########################'

        if not hasattr(Belle2, 'CDCLocalTracking'):
            print 'CDCLocalTracking namespace not available from Python'
            print 'Did you compile with LOCALTRACKING_USE_ROOT?'
            print 'Activate in cdcLocalTracking/mockroot/include/ToggleMockRoot.h and recompile.'
            r = raw_input('')
            return
        else:
            print 'CDCLocalTracking namespace available from Python'
            print 'dir(Belle2)', dir(Belle2)
            print 'dir(Belle2.CDCLocalTracking)', dir(Belle2.CDCLocalTracking)
            print 'dir(genfit)', dir(genfit)

        plotter = svgdrawing.CDCSVGPlotter()
        self.plotter = plotter

        # ######### CDCWires ##########
        # Draw wires from cdcwire objects
        # Now prefered way of ploting the wires
        if self.draw_wires:
            print 'Drawing wires'
            theCDCWireTopology = \
                Belle2.CDCLocalTracking.CDCWireTopology.getInstance()

            for iLayer in range(theCDCWireTopology.getNLayers()):
                wirelayer = theCDCWireTopology.getWireLayer(iLayer)
                for iWire in range(wirelayer.size()):
                    wire = wirelayer.getWireSave(iWire)
                    plotter.append(wire, stroke='gray')

        # ######### CDCHits ##########
        # Draw wirehits or
        # Draw the raw CDCHits
        if self.draw_hits or self.draw_wirehits:
            styleDict = {'stroke': attributemaps.ZeroDriftLengthColorMap(),
                         'stroke-width': attributemaps.ZeroDriftLengthStrokeWidthMap()}
            self.draw_storearray('CDCHits', styleDict)

        # Draw  mcparticle id
        if self.draw_mcparticles:
            styleDict = {'stroke-width': '0.5',
                         'stroke': attributemaps.MCParticleColorMap()}
                # 'stroke-opacity': '0.5',
            self.draw_storearray('CDCHits', styleDict)

        # Draw the in track segment id
        if self.draw_mcsegments:
            styleDict = {'stroke-width': '0.5',
                         'stroke': attributemaps.MCSegmentIdColorMap()}
            self.draw_storearray('CDCHits', styleDict)

        # Draw monte carlo pdg codes
        if self.draw_mcpdgcodes:
            styleDict = {'stroke-width': '0.5',
                         'stroke': attributemaps.MCPDGCodeColorMap()}
                # 'stroke-opacity': '0.5',
            self.draw_storearray('CDCHits', styleDict)

        # Draw monte carlo pdg codes
        if self.draw_mcprimary:
            styleDict = {'stroke-width': '0.5',
                         'stroke': attributemaps.MCPrimaryColorMap}
                # 'stroke-opacity': '0.5',
            self.draw_storearray('CDCHits', styleDict)

        # Draw SimHits
        if self.draw_simhits:
            print 'Drawing simulated hits'
            cdchits_storearray = Belle2.PyStoreArray('CDCHits')
            simhits_storearray = Belle2.PyStoreArray('CDCHits')
            if simhits_storearray:
                simhits_related_to_cdchit = [cdchit.getRelated('CDCSimHits')
                        for cdchit in cdchits_storearray]
                print '#CDCSimHits', simhits_storearray.getEntries()
                styleDict = {'stroke': 'orange', 'stroke-width': '0.2'}
                # plotter.append(simhits_storearray, **styleDict)
                plotter.append(simhits_related_to_cdchit, **styleDict)

        # Draw RL MC info
        if self.draw_rlinfo:
            styleDict = {'stroke-width': '0.2',
                         'stroke': attributemaps.RLColorMap()}
            self.draw_storearray('CDCHits', styleDict)

        # Draw tof info
        if self.draw_tof:
            styleDict = {'stroke-width': '1',
                         'stroke': attributesmaps.TOFColorMap()}
            self.draw_storearray('CDCHits', styleDict)

        # Draw the reassignment information of hits
        if self.draw_reassigned:
            styleDict = {'stroke-width': '0.5',
                         'stroke': attributemaps.ReassignedSecondaryMap()}
            self.draw_storearray('CDCHits', styleDict)

        # ######### CDCClusters
        # Draw clusters
        if self.draw_clusters:
            styleDict = {'stroke': attributemaps.listColors,
                         'stroke-width': '0.5'}
            self.draw_storearray('CDCWireHitClusters', styleDict)

        # ######### CDCRecoSegments2D ##########
        # Draw Segments

        if self.draw_segments:
            styleDict = {'stroke': attributemaps.listColors,
                         'stroke-width': '0.5'}
            self.draw_storearray('CDCRecoHit2DSegmentsSelected', styleDict)

        if self.draw_segments_mctrackid:
            styleDict = {'stroke': attributemaps.SegmentMCTrackIdColorMap(),
                         'stroke-width': '0.5'}
            self.draw_storearray('CDCRecoHit2DSegmentsSelected', styleDict)

        # Draw mc vertices
        if self.draw_mcvertices:
            print 'Drawing Monte Carlo vertices of the cdc hits'
            wirehit_collection_storeobj = \
                Belle2.PyStoreObj('CDCAllWireHitCollection')

            if wirehit_collection_storeobj:
                wirehit_collection = wirehit_collection_storeobj.obj()
                print '#WireHits', wirehit_collection.size()

                mcLookUp = Belle2.CDCLocalTracking.CDCMCLookUp.Instance()
                iterWireHits = (wirehit_collection.at(iWireHit)
                    for iWireHit in xrange(wirehit_collection.size()))
                for wirehit in iterWireHits:

                    trackId = int(mcLookUp.getMCTrackId(wirehit))
                    colorId = trackId % len(attributemaps.listColors)
                    color = attributemaps.listColors[colorId]

                    simhit = mcLookUp.getSimHit(wirehit)
                    toTPos = simhit.getPosTrack()
                    toPos = Belle2.CDCLocalTracking.Vector3D(toTPos)
                    # plotter.append(toPos,stroke=color)

                    mcPart = mcLookUp.getMCParticle(wirehit)
                    while mcPart != None:
                        mcTPos = mcPart.getVertex()
                        mcPos = Belle2.CDCLocalTracking.Vector3D(mcTPos)
                        plotter.append(mcPos, stroke=color)
                        mcPart = mcPart.getMother()

        # Draw Tangent segments
        if self.draw_tangentsegments:
            styleDict = {'stroke': 'black', 'stroke-width': '0.2'}
            self.draw_storearray('CDCRecoTangentSegments', styleDict)

        # Draw segment triples
        if self.draw_segmenttriples:
            styleDict = {'stroke': attributemaps.listColors,
                         'stroke-width': '0.5'}
            self.draw_storearray('CDCSegmentTriples', styleDict)

        # Draw Tracks
        if self.draw_tracks:
            styleDict = {'stroke': attributemaps.listColors,
                         'stroke-width': '0.5'}
            self.draw_storearray('CDCTracks', styleDict)

        # Draw the genfit track candidates
        if self.draw_gftrackcands:
            styleDict = {'stroke': attributemaps.listColors,
                         'stroke-width': '0.5'}
            self.draw_storearray('TrackCands', styleDict)

        # Draw the superlayer boundaries
        if self.draw_superlayer_boundaries:
            print 'Drawing superlayer boundaries'
            theCDCWireTopology = \
                Belle2.CDCLocalTracking.CDCWireTopology.getInstance()

            for iSuperLayer in range(theCDCWireTopology.getNSuperLayers()):
                superlayer = theCDCWireTopology.getWireSuperLayer(iSuperLayer)
                # superlayers get converted to two circles marking their
                # inner and outer radius
                plotter.append(superlayer)

        # Draw interaction point
        if self.draw_interactionpoint:
            print 'Drawing interaction point'
            interactionPoint = Belle2.CDCLocalTracking.Vector2D(0.0, 0.0)

            styleDict = {'stroke': 'black', 'stroke-width': '1', 'r': '1'}

            plotter.append(interactionPoint, **styleDict)

        # Draw segment triples fits
        if self.draw_segmenttriple_trajectories:
            print 'Drawing segment triple fits'
            segmentTriple_storearray = Belle2.PyStoreArray('CDCSegmentTriples')
            if segmentTriple_storearray:
                print '#2D Trajectories from', \
                    segmentTriple_storearray.getEntries(), 'segment triples'

                iterSegmentTriples = iter(segmentTriple_storearray)
                iterTrajectories = (segmentTriple.getTrajectory2D()
                    for segmentTriple in iterSegmentTriples)
                plotter.append(iterTrajectories)

        # Draw the fits to the segments
        if self.draw_segment_trajectories:
            print 'Drawing the fits to the selected RecoHit2DSegments'
            segment_storearray = \
                Belle2.PyStoreArray('CDCRecoHit2DSegmentsSelected')
            if segment_storearray:
                print '#Trajectories', segment_storearray.getEntries()
                for segment in segment_storearray:
                    if segment.getAxialType() == 0:

                        fitter = Belle2.CDCLocalTracking.CDCRiemannFitter()
                        # fitter.useOnlyPosition()
                        fitter.useOnlyOrientation()
                        # fitter.usePositionAndOrientation()
                        trajectory2D = fitter.fit(segment)
                        plotter.append(trajectory2D, **{'stroke': 'black',
                                       'stroke-width': '0.2'})

                        com = segment.getCenterOfMass2D()
                        plotter.append(com)

                        centralPoint = trajectory2D.getClosest(com)
                        plotter.append(centralPoint)

        # Draw the trajectories of the genfit track candidates
        if self.draw_gftrackcand_trajectories:
            print 'Drawing trajectories of the exported Genfit tracks'
            gftrackcand_storearray = Belle2.PyStoreArray('TrackCands')
            if gftrackcand_storearray:
                print '#Genfit tracks', gftrackcand_storearray.getEntries()

                def color_map(iTrajectory, trajectory):
                    # return "black"
                    return attributemaps.listColors[iTrajectory
                            % len(attributemaps.listColors)]

                styleDict = {'stroke-width': '0.5', 'stroke': color_map}

                trajectories = []
                for gftrackcand in gftrackcand_storearray:
                    tMomentum = gftrackcand.getMomSeed()
                    charge = gftrackcand.getChargeSeed()
                    tPosition = gftrackcand.getPosSeed()

                    momentum = Belle2.CDCLocalTracking.Vector2D(tMomentum.X(),
                            tMomentum.Y())
                    position = Belle2.CDCLocalTracking.Vector2D(tPosition.X(),
                            tPosition.Y())

                    trajectory = \
                        Belle2.CDCLocalTracking.CDCTrajectory2D(position,
                            momentum, charge)
                    trajectories.append(trajectory)

                plotter.append(trajectories, **styleDict)

        plotter.updateViewBox()

        fileName = self.new_output_filename()
        plotter.saveSVGFile(fileName)

        if self.interactive:
            print " Use the 'display' command to show the svg file", fileName, \
                'generated for the last event'
            # 'display' is part of the ImageMagic package commonly installed in linux
            procDisplay = subprocess.Popen(['display', fileName])
            # procDisplay = subprocess.Popen(['display','-background','white',
            # '-flatten',fileName])

            raw_input('Hit enter for next event')

    def endRun(self):
        print 'endRun()'

    def terminate(self):
        print 'terminate()'

    def new_output_basename(self):
        output_basename = datetime.now().isoformat() + '.svg'
        return output_basename

    def new_output_filename(self):
        return os.path.join(self.output_folder, self.new_output_basename())


