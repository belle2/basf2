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
        self.draw_wirehits = True  # and False
        self.draw_hits = True and False

        self.draw_mctracks = True and False

        self.draw_mcvertices = True and False

        self.draw_simhits = True and False
        self.draw_rlinfo = True and False
        self.draw_tof = True and False

        self.draw_clusters = True and False

        self.draw_segments = True and False
        self.draw_tangentsegments = True and False

        self.draw_segmenttriples = True and False
        self.draw_tracks = True and False

        self.draw_gftrackcands = True  # and False

        self.draw_segmenttriple_trajectories = True and False
        self.draw_segment_trajectories = True and False
        self.draw_gftrackcand_trajectories = True  # and False

        self.draw_superlayer_boundaries = True  # and False
        self.draw_interactionpoint = True  # and False

    def initialize(self):
        print 'initialize()'

    def beginRun(self):
        print 'beginRun()'

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

    # Draw wirehits
        if self.draw_wirehits:
            print 'Drawing the wirehits'
            wirehit_storeobj = Belle2.PyStoreObj('CDCAllWireHitCollection')

            if wirehit_storeobj:
                wirehit_collection = wirehit_storeobj.obj()
                print '#Wirehits', wirehit_collection.size()

                def stroke_width_map(iWirehit, wirehit):
                    if wirehit.getRefDriftLength() == 0.0:
                        return '1'
                    else:
                        return '0.4'

                def stroke_map(iWirehit, wirehit):
                    if wirehit.getRefDriftLength() == 0.0:
                        return 'yellow'
                    else:
                        return 'red'

                styleDict = {'stroke-opacity': '0.5', 'stroke': stroke_map,
                             'stroke-width': stroke_width_map}

                plotter.append(wirehit_collection, **styleDict)

    # Draw the raw CDCHits
        if self.draw_hits:
            print 'Drawing the CDCHits'
            cdchit_storearray = Belle2.PyStoreArray('CDCHits')
            if cdchit_storearray:
                print '#CDCHits', cdchit_storearray.getEntries()
                styleDict = {'stroke': 'orange', 'stroke-width': '0.2'}
                plotter.append(cdchit_storearray, **styleDict)

    # Draw  mctracks
        if self.draw_mctracks:
            print 'Drawing the Monte Carlo Tracks'
            wirehit_storeobj = Belle2.PyStoreObj('CDCAllWireHitCollection')
            if wirehit_storeobj:
                wirehit_collection = wirehit_storeobj.obj()
                print '#WireHits', wirehit_collection.size(), \
                    'colored with the Track id'

                mcLookUp = Belle2.CDCLocalTracking.CDCMCLookUp.Instance()
                color_by_trackId = {}

                def color_map(iWireHit, wirehit):
                    trackId = int(mcLookUp.getMajorMCTrackId(wirehit))
          # trackId = int(mcLookUp.getSimTrackId(wirehit))
          # trackId = int(mcLookUp.getMCTrackId(wirehit))

                    if trackId not in color_by_trackId:
                        iColor = len(color_by_trackId)
                        iColor = iColor % len(listColors)
                        color = listColors[iColor]
                        color_by_trackId[trackId] = color
                    else:
                        color = color_by_trackId[trackId]

                    return color

                styleDict = {'stroke-width': '1', 'stroke': color_map,
                             'stroke-opacity': '0.5'}

                plotter.append(wirehit_collection, **styleDict)

    # Draw SimHits
        if self.draw_simhits:
            print 'Drawing simulated hits'
            simhits_storearray = Belle2.PyStoreArray('CDCSimHits')
            if simhits_storearray:
                print '#CDCSimHits', simhits_storearray.getEntries()
                styleDict = {'stroke': 'orange', 'stroke-width': '0.2'}
                plotter.append(simhits_storearray, **styleDict)

    # Draw RL MC info
        if self.draw_rlinfo:
            print 'Drawing Monte Carlo right left information of the wirehits'
            wirehit_collection_storeobj = \
                Belle2.PyStoreObj('CDCAllWireHitCollection')
            if wirehit_collection_storeobj:
                wirehit_collection = wirehit_collection_storeobj.obj()
                print '#WireHits', wirehit_collection.size(), \
                    'colored mc right left information'

                mcLookUp = Belle2.CDCLocalTracking.CDCMCLookUp.Instance()

                def color_map(iWireHit, wirehit):
                    simhit = mcLookUp.getSimHit(wirehit)
                    recohit = \
                        Belle2.CDCLocalTracking.CDCRecoHit2D.fromSimHit(wirehit,
                            simhit)
                    if recohit.getRLInfo() == 1:
                        return 'green'
                    else:
                        return 'red'

                styleDict = {'stroke-width': '0.2', 'stroke': color_map}

                plotter.append(wirehit_collection, **styleDict)

    # Draw tof info
        if self.draw_tof:
            print 'Drawing time of flight of the wirehits'
            wirehit_collection_storeobj = \
                Belle2.PyStoreObj('CDCAllWireHitCollection')
            if wirehit_collection_storeobj:
                wirehit_collection = wirehit_collection_storeobj.obj()
                print '#WireHits', wirehit_collection.size(), \
                    'colored mc right left information'

                mcLookUp = Belle2.CDCLocalTracking.CDCMCLookUp.Instance()

                timeOfFlights = set()
                iterWireHits = (wirehit_collection.at(iWireHit)
                    for iWireHit in xrange(wirehit_collection.size()))
                for wirehit in iterWireHits:
                    simhit = mcLookUp.getSimHit(wirehit)
                    timeOfFlights.add(simhit.getFlightTime())

                max_timeOfFlight = max(timeOfFlights)
                min_timeOfFlight = min(timeOfFlights)
                print 'Number of different tof', len(timeOfFlights)

                color_by_trackId = {}

                def color_map(iWireHit, wirehit):
                    trackId = int(mcLookUp.getMajorMCTrackId(wirehit))
          # trackId = int(mcLookUp.getSimTrackId(wirehit))
          # trackId = int(mcLookUp.getMCTrackId(wirehit))

                    if trackId not in color_by_trackId:
                        iColor = len(color_by_trackId)
                        iColor = iColor % len(listColors)
                        color = listColors[iColor]
                        color_by_trackId[trackId] = color
                    else:
                        color = color_by_trackId[trackId]

                    return color

                def opacity_map(iWireHit, wirehit):
                    simhit = mcLookUp.getSimHit(wirehit)
                    timeOfFlight = simhit.getFlightTime()
                    opacity = math.exp(-(timeOfFlight - min_timeOfFlight)
                                       / (max_timeOfFlight - min_timeOfFlight)
                                       * math.log(10))
                    return str(opacity)

                styleDict = {'stroke-width': '1', 'stroke': color_map,
                             'stroke-opacity': opacity_map}

                plotter.append(wirehit_collection, **styleDict)

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
                    colorId = trackId % len(listColors)
                    color = listColors[colorId]

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

    # Draw clusters
        if self.draw_clusters:
            print 'Drawing the clusters'
            cluster_storearray = Belle2.PyStoreArray('CDCWireHitClusters')
            if cluster_storearray:
                print '#Clusters', cluster_storearray.getEntries()

                styleDict = {'stroke': listColors, 'stroke-width': '1'}

                plotter.append(cluster_storearray, **styleDict)

    # Draw Segments
        if self.draw_segments:
            print 'Drawing the selected RecoHit2DSegments'
            segment_storearray = \
                Belle2.PyStoreArray('CDCRecoHit2DSegmentsSelected')
            if segment_storearray:
                print '#RecoHit2DSegments', segment_storearray.getEntries()

                def color_map(iSegment, segment):
                    return listColors[iSegment % len(listColors)]

                styleDict = {'stroke-width': '0.5', 'stroke': color_map}

                plotter.append(segment_storearray, **styleDict)

    # Draw Tangent segments
        if self.draw_tangentsegments:
            print 'Drawing the selected CDCRecoTangentSegments'

            tangentsegment_storearray = \
                Belle2.PyStoreArray('CDCRecoTangentSegments')
            if tangentsegment_storearray:
                print '#CDCRecoTangentSegments', \
                    tangentsegment_storearray.getEntries()

                def color_map(iTangentSegment, tangentsegment):
                    return listColors[iTangentSegment % len(listColors)]

                styleDict = {'stroke-width': '0.2', 'stroke': 'black'}

                plotter.append(tangentsegment_storearray, **styleDict)

    # Draw segment triples
        if self.draw_segmenttriples:
            print 'Drawing the segment triples'
            segmentTriple_storearray = Belle2.PyStoreArray('CDCSegmentTriples')
            if segmentTriple_storearray:
                print '#SegmentTriples', segmentTriple_storearray.getEntries()

                def color_map(iSegmentTriples, segmenttriples):
          # return "black"
                    return listColors[iSegmentTriples % len(listColors)]

                styleDict = {'stroke-width': '0.5', 'stroke': color_map}

                plotter.append(segmentTriple_storearray, **styleDict)

    # Draw Tracks
        if self.draw_tracks:
            print 'Drawing reconstructed tracks'
            track_storearray = Belle2.PyStoreArray('CDCTracks')
            if track_storearray:
                print '#Tracks', track_storearray.getEntries()

                def color_map(iTrack, track):
          # return "black"
                    return listColors[iTrack % len(listColors)]

                styleDict = {'stroke-width': '0.5', 'stroke': color_map}

                plotter.append(track_storearray, **styleDict)

    # Draw the genfit track candidates
        if self.draw_gftrackcands:
            print 'Drawing exported Genfit tracks'
            gftrackcand_storearray = Belle2.PyStoreArray('GFTrackCands')
            if gftrackcand_storearray:
                print '#Genfit tracks', gftrackcand_storearray.getEntries()

                def color_map(iTrack, track):
          # return "black"
                    return listColors[iTrack % len(listColors)]

                styleDict = {'stroke-width': '0.5', 'stroke': color_map}
                plotter.append(gftrackcand_storearray, **styleDict)

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
            gftrackcand_storearray = Belle2.PyStoreArray('GFTrackCands')
            if gftrackcand_storearray:
                print '#Genfit tracks', gftrackcand_storearray.getEntries()

                def color_map(iTrajectory, trajectory):
          # return "black"
                    return listColors[iTrajectory % len(listColors)]

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


