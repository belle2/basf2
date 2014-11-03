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

import os
import os.path
import math

import subprocess
from datetime import datetime

import svgdrawing
import svgdrawing.attributemaps as attributemaps


class CDCSVGDisplayModule(Module):

    """
    Personal two dimensional event display based on scalable vector graphics
    """

    def __init__(self, output_folder, interactive=True):
        """
        Constructor method

        Parameters
        ----------
        output_folder : str
            Target folder for the output
        interactive : bool, optional
            Switch to display each event to the user and ask to continue after each event
            Defaults to True
        """

        super(CDCSVGDisplayModule, self).__init__()
        # call constructor of base class, required!

        ## Switch if the module shall show the event to the user and wait to continue or just generate the images
        self.interactive = interactive

        ## Folder the images shall be saved to
        self.output_folder = output_folder

        if not os.path.exists(output_folder):
            print "CDCSVGDisplay.__init__ : Output folder '", output_folder, \
                "' does not exist."
            answer = raw_input('Create it? (y or n)')
            if answer == 'y':
                os.makedirs(output_folder)

        ## List of drawing options
        # Animate the display by uncovering the drawn objects in order of their time of flight
        # This can be seen in most standard browsers. Note however that you should switch of
        # the wires in this case to reduce the rendering load.
        ## Switch to make an animated event display by means of animated SVG.
        self.animate = False

        # The following options can be used independent of the track finder to view Monte Carlo information after the simulation is done

        ## Switch to draw the wires. This is wrapped by an extra property, because it should always be deactivated in case the scene is animated
        self._draw_wires = True  # and False

        ## Switch to draw the CDCHits. Default: active
        self.draw_hits = True  # and False

        ## Switch to draw the superlayer boundaries. Default: active
        self.draw_superlayer_boundaries = True  # and False

        ## Switch to draw the interaction point. Default: active
        self.draw_interactionpoint = True  # and False

        ## Switch to draw the MCParticle::getArrayIndex property. Default: inactive
        self.draw_mcparticle_id = True and False

        ## Switch to draw the MCParticle::getPDGCode property. Default: inactive
        self.draw_mcparticle_pdgcodes = True and False

        ## Switch to draw the MCParticle::hasStatus(c_PrimaryParticle) property. Default: inactive
        self.draw_mcparticle_primary = True and False

        ## Switch to draw the Monte Carlo segments as generated in CDCMCTrackStore. Default: inactive
        self.draw_mcsegments = True and False

        ## Switch to draw the CDCSimHits with momentum information. Default: inactive
        self.draw_simhits = True and False

        ## Switch to draw the CDCSimHits color coded by their time of flight. Default: inactive
        self.draw_simhit_tof = True and False

        ## Switch to draw the CDCSimHits color coded by their getPosFlag() property. Default: inactive
        self.draw_simhit_posflag = True and False

        ## Switch to draw the CDCSimHits color coded by their getPDGCode() property. Default: inactive
        self.draw_simhit_pdgcode = True and False

        ## Switch to draw the CDCSimHits connected in the order of their getFlightTime for each Monte Carlo particle. Default: inactive
        self.draw_connect_tof = True and False

        ## Switch to draw the CDCSimHits color coded by their local right left passage information. Default: inactive
        self.draw_rlinfo = True and False

        ## Switch to draw the CDCSimHits color coded by their reassignement information to a different MCParticle. Default: inactive
        self.draw_reassigned = True and False

        ## Switch to draw the axial to axial segment pairs from Monte Carlo truth. Default: inactive
        self.draw_mcaxialaxialpairs = True and False

        ## Switch to draw the axial to stereo segment pairs from Monte Carlo truth. Default: inactive
        self.draw_mcaxialstereopairs = True and False

        ## Switch to draw the segment triples from Monte Carlo truth. Default: inactive
        self.draw_mcsegmenttriples = True and False

        # Those are only available if the local track finder is in the module chain
        # and specific compile time flags enable to transportation of this data

        ## Switch to draw the clusters generated by the finder
        self.draw_clusters = True and False

        ## Switch to draw the segments generated by the finder
        self.draw_segments = True and False

        ## Switch to draw the tangent segments generated by the finder
        self.draw_tangentsegments = True and False

        ## Switch to draw the segments generated by the finder colored with the Monte Carlo track id
        self.draw_segments_mctrackid = True and False

        ## Switch to draw the segments generated by the finder colored by the coalignment information (forward, backward, undetermined)
        self.draw_segments_fbinfo = True and False

        ## Switch to draw the segments generated by the finder colored by the frist in track hit id
        self.draw_segments_firstInTrackId = True and False

        ## Switch to draw the segments generated by the finder colored by the second in track hit id
        self.draw_segments_lastInTrackId = True and False

        ## Switch to draw the segments generated by the finder colored by the number of passed superlayers assoziated to the first hit in the segment
        self.draw_segments_firstNPassedSuperLayers = True and False

        ## Switch to draw the segments generated by the finder colored by the number of passed superlayers assoziated to the last hit in the segment
        self.draw_segments_lastNPassedSuperLayers = True and False

        ## Switch to draw the axial stereo segment pairs generated by the finder
        self.draw_axialstereosegmentpairs = True and False

        ## Switch to draw the segment triples generated by the finder
        self.draw_segmenttriples = True and False

        ## Switch to draw the tracks generated by the finder
        self.draw_tracks = True and False

        ## Switch to draw the trajectories fitted to the segment triples generated by the finder
        self.draw_segmenttriple_trajectories = True and False

        ## Switch to draw the trajectories fitted to the segments generated by the finder
        self.draw_segment_trajectories = True and False

        # Those are only available, if any track finder is in the module chain (not tested for others than the local track finder)

        ## Draw the output Genfit tracks
        self.draw_gftrackcands = True and False

        ## Draw the trajectories stored in the output Genfit tracks
        self.draw_gftrackcand_trajectories = True and False

    @property
    def drawoptions(self):
        """
        Property that collects the various names of the draw options to a list.
        @return list of strings naming the different switches that can be activated.
        """

        draw_options = [option for option in self.__dict__
                        if option.startswith('draw_')]
        _draw_options = [option[1:] for option in self.__dict__
                         if option.startswith('_draw_')]

        result = draw_options + _draw_options

        # animate as a special case
        result.append('animate')
        return result

    @property
    def draw_wires(self):
        """
        Getter for the draw option that indicates if all wires shall be drawn.
        Since this has some performance impact in animated events the wires 
        are prevented from being drawn in this case.
        """

        return self._draw_wires and not self.animate

    @draw_wires.setter
    def draw_wires(self, draw_wires):
        """
        Setter for the option to draw all wires.
        """

        self._draw_wires

    def initialize(self):
        """
        Initialisation method of the module.
        Checks if the CDCLocalTracking namespace is present, since this module relies on
        objects in this namespace.
        """

        # Check if the CDCLocalTracking namespace is available for object look up should always be true now
        if not hasattr(Belle2, 'CDCLocalTracking'):
            print 'CDCLocalTracking namespace not available from Python'
            print 'Did you compile with -DCDCLOCALTRACKING_USE_ROOT?'
            r = raw_input('')
            return
        else:
            print 'CDCLocalTracking namespace available from Python'
            print 'dir(Belle2)', dir(Belle2)
            print 'dir(Belle2.CDCLocalTracking)', dir(Belle2.CDCLocalTracking)
            print 'dir(genfit)', dir(genfit)

        pass

    def beginRun(self):
        """ 
        Begin run method of the module. Empty here.
        """

        pass

    def draw_storearray(self, storearray_name, styleDict):
        """
        Factored method to draw PyStoreArrays from a given name.
        """

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
            print '### not present in the DataStore'

    def event(self):
        """ 
        Event method of the module. Draws the event into a new svg file.
        """

        print '##################### DISPLAY EVENT ###########################'

        plotter = svgdrawing.CDCSVGPlotter(animate=self.animate)

        ## Plotter instance receiving drawable tracking objects.
        self.plotter = plotter

        # Construct additional information from basic Monte Carlo data, if it is available from the DataStore
        # Belle2.CDCLocalTracking.CDCMCHitLookUp.getInstance().fill()

        ########## CDCWires ##########
        # Draw wires from cdcwire objects
        # Now prefered way of ploting the wires
        if self.draw_wires:
            print 'Drawing wires'
            theCDCWireTopology = \
                Belle2.CDCLocalTracking.CDCWireTopology.getInstance()

            for iLayer in range(theCDCWireTopology.getNLayers()):
                wirelayer = theCDCWireTopology.getWireLayer(iLayer)
                for iWire in range(wirelayer.size()):
                    wire = wirelayer.getWireSafe(iWire)
                    plotter.append(wire, stroke='gray')

        ########## CDCHits ##########
        # Draw the raw CDCHits
        if self.draw_hits:
            styleDict = {'stroke': attributemaps.ZeroDriftLengthColorMap(),
                         'stroke-width': attributemaps.ZeroDriftLengthStrokeWidthMap()}
            self.draw_storearray('CDCHits', styleDict)

        # Draw  mcparticle id
        if self.draw_mcparticle_id:
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
        if self.draw_mcparticle_pdgcodes:
            styleDict = {'stroke-width': '0.5',
                         'stroke': attributemaps.MCPDGCodeColorMap()}
                # 'stroke-opacity': '0.5',

            self.draw_storearray('CDCHits', styleDict)

        # Draw monte carlo pdg codes
        if self.draw_mcparticle_primary:
            styleDict = {'stroke-width': '0.5',
                         'stroke': attributemaps.MCPrimaryColorMap()}
            self.draw_storearray('CDCHits', styleDict)

        # Draw SimHits
        if self.draw_simhits:
            print 'Drawing simulated hits'
            hit_storearray = Belle2.PyStoreArray('CDCHits')
            if hit_storearray:
                simHits_related_to_hits = [hit.getRelated('CDCSimHits')
                        for hit in hit_storearray]
                print '#CDCSimHits', hit_storearray.getEntries()
                styleDict = {'stroke': 'orange', 'stroke-width': '0.2'}
                plotter.append(simHits_related_to_hits, **styleDict)

        # Draw RL MC info
        if self.draw_simhit_posflag:
            styleDict = {'stroke-width': '0.2',
                         'stroke': attributemaps.PosFlagColorMap()}
            self.draw_storearray('CDCHits', styleDict)

        # Draw local RL info
        if self.draw_rlinfo:
            styleDict = {'stroke-width': '0.2',
                         'stroke': attributemaps.RLColorMap()}
            self.draw_storearray('CDCHits', styleDict)

        # Draw tof info
        if self.draw_simhit_tof:
            styleDict = {'stroke-width': '1',
                         'stroke': attributemaps.TOFColorMap()}
            self.draw_storearray('CDCHits', styleDict)

        if self.draw_connect_tof:
            print 'Drawing simulated hits connected by tof'
            cdchits_storearray = Belle2.PyStoreArray('CDCHits')
            if not cdchits_storearray:
                print 'Store array not present'
            if cdchits_storearray:
                simhits_related_to_cdchit = [cdchit.getRelated('CDCSimHits')
                        for cdchit in cdchits_storearray]

                # group them by their mcparticle id
                simhits_by_mcparticle = {}
                for simhit in simhits_related_to_cdchit:
                    mcparticle = simhit.getRelated('MCParticles')
                    if not mcparticle == None:
                        mcTrackId = mcparticle.getArrayIndex()
                        simhits_by_mcparticle.setdefault(mcTrackId, [])
                        simhits_by_mcparticle[mcTrackId].append(simhit)

                for simhits_for_mcparticle in simhits_by_mcparticle.values():
                    simhits_for_mcparticle.sort(key=lambda simhit: \
                            simhit.getFlightTime())

                    nSimHits = len(simhits_for_mcparticle)
                    for iSimHit in range(nSimHits - 1):
                        fromSimHit = simhits_for_mcparticle[iSimHit]
                        toSimHit = simhits_for_mcparticle[iSimHit + 1]

                        styleDict = {'stroke': 'black', 'stroke-width': '0.2'}
                        plotter.append((fromSimHit, toSimHit), **styleDict)

        if self.draw_simhit_pdgcode:

            def color_map(iHit, hit):
                simHit = hit.getRelated('CDCSimHits')
                pdgCode = simHit.getPDGCode()
                color = \
                    attributemaps.MCPDGCodeColorMap.color_by_pdgcode.get(pdgCode,
                        'orange')
                return color

            styleDict = {'stroke': color_map, 'stroke-width': '0.5'}
            self.draw_storearray('CDCHits', styleDict)

        # Draw the reassignment information of hits
        if self.draw_reassigned:
            styleDict = {'stroke-width': '0.5',
                         'stroke': attributemaps.ReassignedSecondaryMap()}
            self.draw_storearray('CDCHits', styleDict)

        ########## CDCClusters
        # Draw clusters
        if self.draw_clusters:
            styleDict = {'stroke': attributemaps.listColors,
                         'stroke-width': '0.5'}
            self.draw_storearray('CDCWireHitClusters', styleDict)

        ########## CDCRecoSegments2D ##########
        # Draw Segments

        if self.draw_segments:
            styleDict = {'stroke': attributemaps.listColors,
                         'stroke-width': '0.5'}
            self.draw_storearray('CDCRecoSegment2Ds', styleDict)

        if self.draw_segments_mctrackid:
            styleDict = {'stroke': attributemaps.SegmentMCTrackIdColorMap(),
                         'stroke-width': '0.5'}
            self.draw_storearray('CDCRecoSegment2Ds', styleDict)

        if self.draw_segments_fbinfo:
            styleDict = {'stroke': attributemaps.SegmentFBInfoColorMap(),
                         'stroke-width': '0.5'}
            self.draw_storearray('CDCRecoSegment2Ds', styleDict)

        if self.draw_segments_firstInTrackId:
            styleDict = \
                {'stroke': attributemaps.SegmentFirstInTrackIdColorMap(),
                 'stroke-width': '0.5'}
            self.draw_storearray('CDCRecoSegment2Ds', styleDict)

        if self.draw_segments_lastInTrackId:
            styleDict = \
                {'stroke': attributemaps.SegmentLastInTrackIdColorMap(),
                 'stroke-width': '0.5'}
            self.draw_storearray('CDCRecoSegment2Ds', styleDict)

        if self.draw_segments_firstNPassedSuperLayers:
            styleDict = \
                {'stroke': attributemaps.SegmentFirstNPassedSuperLayersColorMap(),
                 'stroke-width': '0.5'}
            self.draw_storearray('CDCRecoSegment2Ds', styleDict)

        if self.draw_segments_lastNPassedSuperLayers:
            styleDict = \
                {'stroke': attributemaps.SegmentLastNPassedSuperLayersColorMap(),
                 'stroke-width': '0.5'}
            self.draw_storearray('CDCRecoSegment2Ds', styleDict)

        # Mimic axial to axial pair selection
        if self.draw_mcaxialaxialpairs:
            print 'Draw axial to axial segment pairs'
            segment_storearray = Belle2.PyStoreArray('CDCRecoSegment2Ds')
            if segment_storearray:
                print '#Segment', segment_storearray.getEntries()
                axial_segments = [segment for segment in segment_storearray
                                  if segment.getStereoType() == 0]

                mc_axial_axial_segment_filter = \
                    Belle2.CDCLocalTracking.MCAxialAxialSegmentPairFilter()
                axial_axial_segment_pairs = \
                    (Belle2.CDCLocalTracking.CDCAxialAxialSegmentPair(startSegment,
                        endSegment) for startSegment in axial_segments
                    for endSegment in axial_segments)

                def is_good_pair(pair):
                    weight = \
                        mc_axial_axial_segment_filter.isGoodAxialAxialSegmentPair(pair)
                    return weight == weight  # not nan

                good_axial_axial_segment_pairs = [pair for pair in
                        axial_axial_segment_pairs if is_good_pair(pair)]

                print '#Pairs', len(good_axial_axial_segment_pairs)
                styleDict = {'stroke': 'black', 'stroke-width': '0.2'}

                plotter.append(good_axial_axial_segment_pairs, **styleDict)

        if self.draw_mcaxialstereopairs:
            styleDict = {'stroke': 'black', 'stroke-width': '0.2'}
            self.draw_storearray('CDCAxialStereoSegmentPairs', styleDict)

        if self.draw_mcsegmenttriples:
            print 'Draw axial to axial segment pairs'
            segment_storearray = Belle2.PyStoreArray('CDCRecoSegment2Ds')
            if segment_storearray:
                print '#Segment', segment_storearray.getEntries()
                axial_segments = [segment for segment in segment_storearray
                                  if segment.getStereoType() == 0]

                stereo_segments = [segment for segment in segment_storearray
                                   if segment.getStereoType() != 0]

                # Misuse this a bit but still does what we want
                mc_axial_axial_segment_filter = \
                    Belle2.CDCLocalTracking.MCAxialAxialSegmentPairFilter()
                mc_segment_lookup = \
                    Belle2.CDCLocalTracking.CDCMCSegmentLookUp.getInstance()

                segment_triples = \
                    (Belle2.CDCLocalTracking.CDCSegmentTriple(startSegment,
                        middleSegment, endSegment) for startSegment in
                    axial_segments for middleSegment in stereo_segments
                    for endSegment in axial_segments)

                def is_good_triple(triple):
                    start = triple.getStart()
                    middle = triple.getMiddle()
                    end = triple.getEnd()

                    pairWeight = \
                        mc_axial_axial_segment_filter.isGoodAxialAxialSegmentPair(triple)

                    if not pairWeight == pairWeight:
                        return False

                    startToMiddleFBInfo = \
                        mc_segment_lookup.areAlignedInMCTrack(start, middle)
                    if abs(startToMiddleFBInfo) > 1:
                        return False

                    middleToEndFBInfo = \
                        mc_segment_lookup.areAlignedInMCTrack(middle, end)
                    if abs(middleToEndFBInfo) > 1:
                        return False

                    if startToMiddleFBInfo == middleToEndFBInfo:
                        return True
                    else:
                        return False

                good_segment_triples = [triple for triple in segment_triples
                        if is_good_triple(triple)]

                print '#Triple', len(good_segment_triples)
                styleDict = {'stroke': 'black', 'stroke-width': '0.2'}

                plotter.append(good_segment_triples, **styleDict)

        # Draw Tangent segments
        if self.draw_tangentsegments:
            styleDict = {'stroke': 'black', 'stroke-width': '0.2'}
            self.draw_storearray('CDCRecoTangentSegments', styleDict)

        # Draw axial stereo segment pairs
        if self.draw_axialstereosegmentpairs:
            styleDict = {'stroke': attributemaps.listColors,
                         'stroke-width': '0.5'}
            self.draw_storearray('CDCAxialStereoSegmentPairs', styleDict)

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
                print '#2D Trajectories', \
                    segmentTriple_storearray.getEntries(), \
                    'from segment triples'

                iterSegmentTriples = iter(segmentTriple_storearray)
                iterTrajectories = (segmentTriple.getTrajectory2D()
                    for segmentTriple in iterSegmentTriples)
                plotter.append(iterTrajectories)

        # Draw the fits to the segments
        if self.draw_segment_trajectories:
            print 'Drawing the fits to the selected RecoHit2DSegments'
            segment_storearray = Belle2.PyStoreArray('CDCRecoSegment2Ds')
            if segment_storearray:
                print '#2D Trajectories', segment_storearray.getEntries(), \
                    'from segments'
                iterSegments = iter(segment_storearray)
                iterTrajectories = (segment.getTrajectory2D() for segment in
                    iterSegments)
                plotter.append(iterTrajectories)

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
        """
        endRun methode of the module. Empty here.
        """

        pass

    def terminate(self):
        """
        teminate methode of the module. Empty here.
        """

        pass

    def new_output_basename(self):
        """
        Generates a new unique name for the current event without the folder prefix
        """

        output_basename = datetime.now().isoformat() + '.svg'
        return output_basename

    def new_output_filename(self):
        """
        Generates a new unique name for the current event with the folder prefix
        """

        return os.path.join(self.output_folder, self.new_output_basename())


