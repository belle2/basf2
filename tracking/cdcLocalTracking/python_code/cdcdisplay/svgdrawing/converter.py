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

print dir(genfit)
print dir(genfit.TrackCand)
print dir(Belle2)
# from ROOT import GFTrackCand

import primitives


class CDCDataobjectsConverter:

    """
    Helper object to translate the various tracking objects from BASF2 to a XML-DOM representation.
    """

    def __init__(self, elementFactory, animate=False):
        """
        Construction method, setting up a the XML-DOM element factory and a dispatch dictionary mapping tracking enties from BASF2 to their respective converted methods.
        """

        ## Switch if an animated version of the elements shall be generated
        self.animate = animate

        ## SVG primitive elements factory
        self.svgElementFactory = \
            primitives.SVGPrimitivesFactory(elementFactory)

        ## Dispatch map from tracking object types to specialised translator functions
        self.toSVGFunctions_by_type = {  # for adhoc tangents
            tuple: self.TupleToSVG,
            Belle2.PyStoreObj: self.PyStoreObjToSVG,
            Belle2.PyStoreArray: self.PyStoreArrayToSVG,
            Belle2.CDCSimHit: self.CDCSimHitToSVG,
            Belle2.CDCHit: self.CDCHitToSVG,
            genfit.TrackCand: self.GFTrackCandToSVG,
            }

        if hasattr(Belle2.CDCLocalTracking, 'CDCWireHit'):
            # CDCLOCALTRACKING_USE_ROOT is active
            # use pyROOT interface to draw various specialised objects from the local finder.
            self.toSVGFunctions_by_type.update({
                Belle2.CDCLocalTracking.Vector2D: self.Vector2DToSVG,
                Belle2.CDCLocalTracking.Vector3D: self.Vector3DToSVG,
                Belle2.CDCLocalTracking.CDCWire: self.CDCWireToSVG,
                Belle2.CDCLocalTracking.CDCWireSuperLayer: self.CDCWireSuperLayerToSVG,
                Belle2.CDCLocalTracking.CDCWireHit: self.CDCWireHitToSVG,
                Belle2.CDCLocalTracking.CDCWireHitVector: self.CDCGenericHitCollectionToSVG,
                Belle2.CDCLocalTracking.CDCRecoHit2D: self.CDCRecoHit2DToSVG,
                Belle2.CDCLocalTracking.CDCRecoHit2DVector: self.CDCGenericHitCollectionToSVG,
                Belle2.CDCLocalTracking.CDCRecoTangent: self.CDCRecoTangentToSVG,
                Belle2.CDCLocalTracking.CDCRecoTangentVector: self.CDCGenericHitCollectionToSVG,
                Belle2.CDCLocalTracking.CDCRecoFacetVector: self.CDCGenericHitCollectionToSVG,
                Belle2.CDCLocalTracking.CDCRecoHit3D: self.CDCRecoHit3DToSVG,
                Belle2.CDCLocalTracking.CDCRecoHit3DVector: self.CDCGenericHitCollectionToSVG,
                Belle2.CDCLocalTracking.CDCWireHitCluster: self.CDCGenericHitCollectionToSVG,
                Belle2.CDCLocalTracking.CDCRecoSegment2D: self.CDCGenericHitCollectionToSVG,
                Belle2.CDCLocalTracking.CDCAxialAxialSegmentPair: self.CDCAxialAxialSegmentPairToSVG,
                Belle2.CDCLocalTracking.CDCSegmentTriple: self.CDCSegmentTripleToSVG,
                Belle2.CDCLocalTracking.CDCTrack: self.CDCGenericHitCollectionToSVG,
                Belle2.CDCLocalTracking.CDCTrajectory2D: self.CDCTrajectory2DToSVG,
                })

            # Generics
            for name in Belle2.__dict__:
                if 'CDCGenericHitCollection' in name:
                    self.toSVGFunctions_by_type[Belle2.__dict__[name]] = \
                        self.CDCGenericHitCollectionToSVG

    def nanoSecondsToAnimationTime(self, nanoSeconds):
        """Translator function from nano seconds to animation time units in seconds"""

        return str(nanoSeconds) + 's'

    def styleFillUpdate(self, attributes):
        """Helper function to set the fill property to the stroke property if not already set otherwise."""

        if 'fill' not in attributes or not attributes['fill']:
            if 'stroke' in attributes:
                attributes['fill'] = attributes['stroke']

    @staticmethod
    def unpackKwd(kwd, iObj=0, obj=None):
        """Mapping function to unpack the attributes from the attribute maps. Mechanism and interface inspired by d3.js"""

        result = {}
        for (key, value) in kwd.items():
            if callable(value):
                result[key] = value(iObj, obj)
            elif isinstance(value, str):
                result[key] = value
            elif hasattr(value, '__getitem__'):
                result[key] = value[iObj % len(value)]
            else:
                result[key] = value
        return result

    def toSVG(self, obj, **kwd):
        """Common entry point for the object translation managing the dispatch to specialised functions"""

        # implements dispatch on object type
        objtype = type(obj)

        toSVGForType = self.toSVGFunctions_by_type.get(objtype,
                self.IterableToSVG)

        return toSVGForType(obj, **kwd)

    def IterableToSVG(self, iterable, **kwd):
        """Mapping an iterable object to a group of svg objects"""

        toSVG = self.toSVG
        unpackKwd = self.unpackKwd

        try:
            iter(iterable)
        except TypeError, e:
            print type(iterable), \
                ' is not iterable. Consider adding a specialised converter function for this object type.'

        childElements = []
        for (iObj, obj) in enumerate(iterable):
            obj_kwd = unpackKwd(kwd, iObj, obj)
            childElements.append(toSVG(obj, **obj_kwd))

        iterChildElements = iter(childElements)

        groupElement = \
            self.svgElementFactory.createGroupFromIterable(iterChildElements)

        return groupElement

    def TupleToSVG(self, tup, **kwd):
        """Mapping a tuple to a group of svg objects. Contains a special case for pairs of simhits to mimic a tangent."""

        if len(tup) != 2:
            return IterableToSVG(tup, **kwd)
        else:
            fromSimHit = tup[0]
            toSimHit = tup[1]

            if not isinstance(fromSimHit, Belle2.CDCSimHit) \
                or not isinstance(toSimHit, Belle2.CDCSimHit):
                raise ValueError('No conversion for pairs of '
                                 + str(type(fromSimHit)) + ', '
                                 + str(type(toSimHit)))

            styleDict = {'stroke': 'black', 'stroke-width': '0.2'}
            styleDict.update(kwd)

            fromPoint = (fromSimHit.getPosTrack().X(),
                         fromSimHit.getPosTrack().Y())
            toPoint = (toSimHit.getPosTrack().X(), toSimHit.getPosTrack().Y())

            lineElement = self.svgElementFactory.createLine(fromPoint,
                    toPoint, **styleDict)

            return lineElement

    def PyStoreObjToSVG(self, pystoreobj, **kwd):
        """Unpacks a PyStoreObj and translates the content to svg."""

        if pystoreobj:
            unpackedObject = pystoreobj.obj()
            return self.toSVG(unpackedObject, **kwd)
        else:
            return self.svgElementFactory.createGroup()

    def PyStoreArrayToSVG(self, pystorearray, **kwd):
        """Mapping a PyStoreArray to a group of svg objects."""

        if pystorearray:
            iterItems = iter(pystorearray)
            return self.IterableToSVG(iterItems, **kwd)
        else:
            print 'PyStoreArray does not reference a legal StoreArray'
            return self.svgElementFactory.createGroup()

    def CDCSimHitToSVG(self, cdcsimhit, **kwd):
        """Maps a CDCSimHit to a momentum arrow starting at the current position with a length proportional to its momentum."""

        childElements = []

        styleDict = {'stroke': 'yellow', 'stroke-width': '0.02'}
        styleDict.update(kwd)

    # mark trackPoint where hit occured
        simhitVector = cdcsimhit.getPosTrack()
        simhitPoint = (simhitVector.X(), simhitVector.Y())

        supportPointRadius = 0.015
        circleElement = self.svgElementFactory.createCircle(simhitPoint,
                supportPointRadius, **styleDict)
        childElements.append(circleElement)

        simhitMomentumVec = cdcsimhit.getMomentum()
        scale = 1.5
        simhitMom = (simhitVector.X() + simhitMomentumVec.X() * scale,
                     simhitVector.Y() + simhitMomentumVec.Y() * scale)

        fromPointMom = simhitPoint
        toPointMom = simhitMom

        momStyleDict = dict(styleDict)
        momStyleDict['marker-end'] = 'url(#markerEndArrow)'
        momLineElement = self.svgElementFactory.createLine(fromPointMom,
                toPointMom, **momStyleDict)
        childElements.append(momLineElement)

        fromVector = cdcsimhit.getPosIn()
        fromPoint = (fromVector.X(), fromVector.Y())

        toVector = cdcsimhit.getPosOut()
        toPoint = (toVector.X(), toVector.Y())

        flightLineElement = self.svgElementFactory.createLine(fromPoint,
                toPoint, **styleDict)
        # childElements.append(flightLineElement)

        if self.animate:
            tof = cdcsimhit.getFlightTime()

            # stretch time by a common factor
            end = self.nanoSecondsToAnimationTime(tof)

            # make elements hidden in the beginning an uncover at their time of flight
            setElement = self.svgElementFactory.createSet('visibility',
                    'hidden', begin='0s', end=end)

            childElements.append(setElement)

        groupElement = self.svgElementFactory.createGroup(*childElements)

        return groupElement

    def CDCHitToSVG(self, cdchit, **kwd):
        """Maps a CDCHit to a drift circle at the two dimensional reference wire position"""

        childElements = []

        mcHitLookUp = Belle2.CDCLocalTracking.CDCMCHitLookUp.getInstance()
        tCenterPostion = mcHitLookUp.getRefPos2D(cdchit)

        center = (tCenterPostion.X(), tCenterPostion.Y())
        driftRadius = mcHitLookUp.getRefDriftLength(cdchit)

        # Draw the wire
        wireRadius = 0.1

        wireStyleDict = {'stroke': 'black', 'stroke-width': repr(wireRadius)}
        wireStyleDict.update(kwd)
        self.styleFillUpdate(wireStyleDict)

        wireCircleElement = self.svgElementFactory.createCircle(center,
                wireRadius, **wireStyleDict)
        childElements.append(wireCircleElement)

        driftCircleStyleDict = {'stroke': 'black', 'stroke-width': '0.02',
                                'fill': 'none'}

        driftCircleStyleDict.update(kwd)

        driftCircleElement = self.svgElementFactory.createCircle(center,
                driftRadius, **driftCircleStyleDict)
        childElements.append(driftCircleElement)

        if self.animate:
            simhit = cdchit.getRelated('CDCSimHits')
            tof = simhit.getFlightTime()

            # stretch time by a common factor
            end = self.nanoSecondsToAnimationTime(tof)

            # make elements hidden in the beginning an uncover at their time of flight
            setElement = self.svgElementFactory.createSet('visibility',
                    'hidden', begin='0s', end=end)

            childElements.append(setElement)

        groupElement = self.svgElementFactory.createGroup(*childElements)

        return groupElement

    def GFTrackCandToSVG(self, gftrackcand, **kwd):
        """Maps a Genfit track candiadte to a group of hits"""

        storeHits = Belle2.PyStoreArray('CDCHits')
        hitIDs = gftrackcand.getHitIDs(3)
        iterHitIDs = (hitIDs[iHit] for iHit in xrange(hitIDs.size()))
        cdcHits = [storeHits[hitID] for hitID in iterHitIDs]
        return self.toSVG(cdcHits, **kwd)

    def Vector2DToSVG(self, vec, **kwd):
        """Maps a Vector2D to a point at the position pointed to"""

        defaultStyleDict = {'stroke': 'black', 'stroke-width': '0.02'}
        defaultStyleDict.update(kwd)
        self.styleFillUpdate(defaultStyleDict)

        point = (vec.x(), vec.y())
        supportPointRadius = 0.2

        circleElement = self.svgElementFactory.createCircle(point,
                supportPointRadius, **defaultStyleDict)
        return circleElement

    def Vector3DToSVG(self, vec, **kwd):
        """Maps a Vector2D to a two dimensional point at the position pointed to"""

        defaultStyleDict = {'stroke': 'black', 'stroke-width': '0.02'}
        defaultStyleDict.update(kwd)
        self.styleFillUpdate(defaultStyleDict)

        point = (vec.x(), vec.y())
        supportPointRadius = 0.2

        circleElement = self.svgElementFactory.createCircle(point,
                supportPointRadius, **defaultStyleDict)
        return circleElement

    def CDCWireToSVG(self, cdcwire, **kwd):
        """Maps a CDCWire to the two dimensional wire reference position."""

        wireRadius = 0.1

        styleDict = {'stroke': 'black', 'stroke-width': repr(wireRadius)}
        styleDict.update(kwd)
        self.styleFillUpdate(styleDict)

        centerPostion = cdcwire.getRefPos3D()
        center = (centerPostion.x(), centerPostion.y())

        radius = wireRadius

        circleElement = self.svgElementFactory.createCircle(center, radius,
                **styleDict)

        return circleElement

    def CDCWireSuperLayerToSVG(self, cdcWireSuperLayer, **kwd):
        """Maps a CDCWireSuperLayer to two circles representing the inner and outer radius of the superlayer"""

        styleDict = {'stroke': 'black', 'stroke-width': '0.2', 'fill': 'none'}
        styleDict.update(kwd)
        self.styleFillUpdate(styleDict)

        center = (0.0, 0.0)
        innerRadius = cdcWireSuperLayer.getInnerPolarR()
        outerRadius = cdcWireSuperLayer.getOuterPolarR()

        innerCircleElement = self.svgElementFactory.createCircle(center,
                innerRadius, **styleDict)
        outerCircleElement = self.svgElementFactory.createCircle(center,
                outerRadius, **styleDict)

        groupElement = self.svgElementFactory.createGroup(innerCircleElement,
                outerCircleElement)

        return groupElement

    def CDCWireHitToSVG(self, wirehit, **kwd):
        """Maps a CDCWireHit to a drift circle at the two dimensional reference wire position"""

        wire = wirehit.getWire()
        wireSVGElement = self.toSVG(wire, **kwd)

    # defaultStyleDict = {'stroke':'black','stroke-width':'0.1','fill':'none'}
        defaultStyleDict = {'stroke': 'black', 'stroke-width': '0.02',
                            'fill': 'none'}

        defaultStyleDict.update(kwd)

        centerPostion = wire.getRefPos2D()
        center = (centerPostion.x(), centerPostion.y())

        radius = wirehit.getRefDriftLength()

        circleElement = self.svgElementFactory.createCircle(center, radius,
                **defaultStyleDict)

        groupElement = self.svgElementFactory.createGroup(circleElement,
                wireSVGElement)

        return groupElement

    def CDCRecoHit2DToSVG(self, recohit, **kwd):
        """Maps a CDCRecoHit2D to a drift circle at the two dimensional reference wire position and a point at the reconstructed position"""

        position = recohit.getRecoPos2D()
        point = (position.x(), position.y())

        if point[0] != point[0] or point[1] != point[1]:
            kwd['stroke'] = 'red'
        # print "NAN position"

        wirehit = recohit.getWireHit()
        wirehitElement = self.toSVG(wirehit, **kwd)

        defaultStyleDict = {'stroke': 'black', 'stroke-width': '0.02'}
        defaultStyleDict.update(kwd)
        self.styleFillUpdate(defaultStyleDict)

        supportPointRadius = 0.2

        circleElement = self.svgElementFactory.createCircle(point,
                supportPointRadius, **defaultStyleDict)

        return self.svgElementFactory.createGroup(wirehitElement,
                circleElement)

    def CDCRecoHit3DToSVG(self, recohit, **kwd):
        """Maps a CDCRecoHit2D to a drift circle at the two dimensional reference wire position and a point at the reconstructed position"""

        wirehit = recohit.getWireHit()
        wirehitElement = self.toSVG(wirehit, **kwd)

        defaultStyleDict = {'stroke': 'black', 'stroke-width': '0.02'}
        defaultStyleDict.update(kwd)
        defaultStyleDict['stroke'] = 'black'
        self.styleFillUpdate(defaultStyleDict)

        position = recohit.getPos3D()
        point = (position.x(), position.y())

        supportPointRadius = 0.2

        circleElement = self.svgElementFactory.createCircle(point,
                supportPointRadius, **defaultStyleDict)

        return self.svgElementFactory.createGroup(wirehitElement,
                circleElement)

    def CDCRecoTangentToSVG(self, recotangent, **kwd):
        """Maps a CDCRecoTangent to line connecting the from and the to reconstructed position"""

        styleDict = {'stroke': 'black', 'stroke-width': '0.02'}
        styleDict.update(kwd)

        fromPoint = (recotangent.getFromRecoPos2D().x(),
                     recotangent.getFromRecoPos2D().y())
        toPoint = (recotangent.getToRecoPos2D().x(),
                   recotangent.getToRecoPos2D().y())

        lineElement = self.svgElementFactory.createLine(fromPoint, toPoint,
                **styleDict)

    # supportPoints
        supportPointRadius = 0.015

        self.styleFillUpdate(styleDict)
        circleElement1 = self.svgElementFactory.createCircle(fromPoint,
                supportPointRadius, **styleDict)
        circleElement2 = self.svgElementFactory.createCircle(toPoint,
                supportPointRadius, **styleDict)

        groupElement = self.svgElementFactory.createGroup(lineElement,
                circleElement1, circleElement2)

        return groupElement

    def CDCAxialAxialSegmentPairToSVG(self, segmentTriple, **kwd):
        """Maps a CDCAxialAxialSegmentPair to a line connecting the center of masses of the two segments."""

        childElements = []
        defaultStyleDict = {'stroke': 'black', 'stroke-width': '0.02'}
        defaultStyleDict.update(kwd)

        startSegment = segmentTriple.getStart()
        endSegment = segmentTriple.getEnd()

        startElement = self.toSVG(startSegment, **kwd)
        endElement = self.toSVG(endSegment, **kwd)

        comStart = segmentTriple.getStart().getCenterOfMass2D()
        comEnd = segmentTriple.getEnd().getCenterOfMass2D()

        arrowStyleDict = dict(defaultStyleDict)
        arrowStyleDict['marker-end'] = 'url(#markerEndArrow)'

        fromPoint1 = (comStart.x(), comStart.y())
        toPoint1 = (comEnd.x(), comEnd.y())

        arrowElement1 = self.svgElementFactory.createLine(fromPoint1,
                toPoint1, **arrowStyleDict)
        childElements.append(arrowElement1)

        groupElement = self.svgElementFactory.createGroup(*childElements)

        return groupElement

    def CDCSegmentTripleToSVG(self, segmentTriple, **kwd):
        """Maps a CDCAxialAxialSegmentPair to two lines connecting the center of masses of the three segments."""

        childElements = []
        defaultStyleDict = {'stroke': 'black', 'stroke-width': '0.02'}
        defaultStyleDict.update(kwd)

    # print segmentTriple.getStartISuperLayer(),
    # segmentTriple.getMiddleISuperLayer(), segmentTriple.getEndISuperLayer()
    # print "plot segments"

        startSegment = segmentTriple.getStart()
        middleSegment = segmentTriple.getMiddle()
        endSegment = segmentTriple.getEnd()

        startElement = self.toSVG(startSegment, **kwd)
        middleElement = self.toSVG(middleSegment, **kwd)
        endElement = self.toSVG(endSegment, **kwd)

    # childElements.append(startElement)
    # childElements.append(middleElement)
    # childElements.append(endElement)

    # print "Start", startSegment.size()
    # print type(startSegment)
    # print dir(startSegment)

    # print "plotted segments"

        xyFit = segmentTriple.getTrajectory2D()

        comStart = segmentTriple.getStart().getCenterOfMass2D()
        comMiddle = segmentTriple.getMiddle().getCenterOfMass2D()
        comEnd = segmentTriple.getEnd().getCenterOfMass2D()

    # print "comStart", comStart.x(),comStart.y()
    # print "comMiddle", comMiddle.x(),comMiddle.y()
    # print "comStart", comEnd.x(),comEnd.y()

    # print "get closest approaches"

        fitPointStart = xyFit.getClosest(comStart)
        fitPointMiddle = xyFit.getClosest(comMiddle)
        fitPointEnd = xyFit.getClosest(comEnd)

    # print "fitPointStart", fitPointStart.x(),fitPointStart.y()
    # print "fitPointMiddle", fitPointMiddle.x(),fitPointMiddle.y()
    # print "fitPointEnd", fitPointEnd.x(),fitPointEnd.y()

        startPointElement = self.toSVG(fitPointStart, **kwd)
        middlePointElement = self.toSVG(fitPointMiddle, **kwd)
        endPointElement = self.toSVG(fitPointEnd, **kwd)

    # print "got central points"

        arrowStyleDict = dict(defaultStyleDict)
        arrowStyleDict['marker-end'] = 'url(#markerEndArrow)'

        fromPoint1 = (fitPointStart.x(), fitPointStart.y())
        toPoint1 = (fitPointMiddle.x(), fitPointMiddle.y())

        arrowElement1 = self.svgElementFactory.createLine(fromPoint1,
                toPoint1, **arrowStyleDict)
        childElements.append(arrowElement1)

        fromPoint2 = (fitPointMiddle.x(), fitPointMiddle.y())
        toPoint2 = (fitPointEnd.x(), fitPointEnd.y())

        arrowElement2 = self.svgElementFactory.createLine(fromPoint2,
                toPoint2, **arrowStyleDict)
        childElements.append(arrowElement2)

        fitElement = self.toSVG(xyFit)
    # childElements.append(fitElement)

        groupElement = self.svgElementFactory.createGroup(*childElements)

        return groupElement

    def CDCGenericHitCollectionToSVG(self, genericHitCollection, **kwd):
        """Maps a collection of tracking entities to a group of svg elements"""

        nItems = genericHitCollection.size()
        iterItems = (genericHitCollection.at(iItem) for iItem in
            xrange(nItems))
        return self.IterableToSVG(iterItems, **kwd)

    def CDCTrajectory2DToSVG(self, fit, **kwd):
        """Maps a CDCTrajectory to a circle section arc at the reference start position on the trajectory until it exits the CDC."""

        svgElements = []

        styleDict = {'stroke': 'black', 'stroke-width': '0.02'}
        styleDict.update(kwd)

    # fit.getGenCircle().normalize()

        if fit.getGenCircle().isCircle():

            charge = fit.getChargeSign()

            if charge > 0:
                styleDict['stroke'] = 'red'
            elif charge < 0:

                styleDict['stroke'] = 'blue'
            else:
                styleDict['stroke'] = 'green'

            styleDict['fill'] = 'none'

            radius = fit.getGenCircle().radius()
            signedRadius = fit.getGenCircle().signedRadius()
            impactParameter = fit.getGenCircle().impact()

            center = fit.getGenCircle().center()
            centerPoint = (center.x(), center.y())

            start = fit.getStartPos2D()
            startPoint = (start.x(), start.y())

            trajectoryExit = fit.getExit()
            exitPoint = (trajectoryExit.x(), trajectoryExit.y())

            if trajectoryExit.hasNAN():
        # no exit
        # draw full circle
                circleElement = \
                    self.svgElementFactory.createCircle(centerPoint, radius,
                        **styleDict)
                svgElements.append(circleElement)
            else:

                if charge == 1:
                    sweep_flag = 0
                else:
                    sweep_flag = 1

                if fit.calcPerpS(trajectoryExit) > 0:
          # check if exit point is on the close or
          # on the far side of the circle
                    long_arc = 0
                else:
                    long_arc = 1

                circleArcElement = self.svgElementFactory.createCircleArc(
                    startPoint,
                    exitPoint,
                    radius,
                    long_arc,
                    sweep_flag,
                    **styleDict
                    )
                svgElements.append(circleArcElement)
        elif fit.getGenCircle().isLine():

      # print "isline"
            start = fit.getStartPos2D()
            startPoint = (start.x(), start.y())

            trajectoryExit = fit.getExit()
            exitPoint = (trajectoryExit.x(), trajectoryExit.y())

      # print  "fitted line", fromPoint, toPoint

            styleDict = {'stroke': 'black', 'stroke-width': '0.02'}
            styleDict.update(kwd)
            styleDict['stroke'] = 'green'

            if not trajectoryExit.hasNAN():

                lineElement = self.svgElementFactory.createLine(startPoint,
                        exitPoint, **styleDict)
                svgElements.append(lineElement)

        groupElement = \
            self.svgElementFactory.createGroupFromIterable(svgElements)
        return groupElement


