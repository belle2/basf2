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

    def __init__(self, elementFactory):
        self.svgElementFactory = \
            primitives.SVGPrimitivesFactory(elementFactory)

        self.toSVGFunctions_by_type = {  # Belle2.CDCLocalTracking.CDCRecoSegment2D: \
                                         # self.CDCGenericHitCollectionToSVG,
            Belle2.PyStoreObj: self.PyStoreObjToSVG,
            Belle2.PyStoreArray: self.PyStoreArrayToSVG,
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
            Belle2.CDCLocalTracking.CDCSegmentTriple: self.CDCSegmentTripleToSVG,
            Belle2.CDCLocalTracking.CDCTrack: self.CDCGenericHitCollectionToSVG,
            Belle2.CDCLocalTracking.CDCTrajectory2D: self.CDCTrajectory2DToSVG,
            Belle2.CDCSimHit: self.CDCSimHitToSVG,
            Belle2.CDCHit: self.CDCHitToSVG,
            genfit.TrackCand: self.GFTrackCandToSVG,
            }

    # Generics
        for name in Belle2.__dict__:
            if 'CDCGenericHitCollection' in name:
                self.toSVGFunctions_by_type[Belle2.__dict__[name]] = \
                    self.CDCGenericHitCollectionToSVG

    def styleFillUpdate(self, attributes):
        if 'fill' not in attributes or not attributes['fill']:
            if 'stroke' in attributes:
                attributes['fill'] = attributes['stroke']

    @staticmethod
    def unpackKwd(kwd, iObj=0, obj=None):
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

    def IterableToSVG(self, iterable, **kwd):

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

    def PyStoreObjToSVG(self, pystoreobj, **kwd):
        if pystoreobj:
            unpackedObject = pystoreobj.obj()
            return self.toSVG(unpackedObject, **kwd)
        else:
            return self.svgElementFactory.createGroup()

    def PyStoreArrayToSVG(self, pystorearray, **kwd):
        if pystorearray:
            iterItems = iter(pystorearray)
            return self.IterableToSVG(iterItems, **kwd)
        else:
            print 'PyStoreArray does not reference a legal StoreArray'
            return self.svgElementFactory.createGroup()

    def Vector2DToSVG(self, vec, **kwd):

        defaultStyleDict = {'stroke': 'black', 'stroke-width': '0.02'}
        defaultStyleDict.update(kwd)
        self.styleFillUpdate(defaultStyleDict)

        point = (vec.x(), vec.y())
        supportPointRadius = 0.2

        circleElement = self.svgElementFactory.createCircle(point,
                supportPointRadius, **defaultStyleDict)
        return circleElement

    def Vector3DToSVG(self, vec, **kwd):

        defaultStyleDict = {'stroke': 'black', 'stroke-width': '0.02'}
        defaultStyleDict.update(kwd)
        self.styleFillUpdate(defaultStyleDict)

        point = (vec.x(), vec.y())
        supportPointRadius = 0.2

        circleElement = self.svgElementFactory.createCircle(point,
                supportPointRadius, **defaultStyleDict)
        return circleElement

    def CDCWireToSVG(self, cdcwire, **kwd):
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

    def CDCSegmentTripleToSVG(self, segmentTriple, **kwd):

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
        nItems = genericHitCollection.size()
        iterItems = (genericHitCollection.at(iItem) for iItem in
            xrange(nItems))
        return self.IterableToSVG(iterItems, **kwd)

    def CDCSimHitToSVG(self, cdcsimhit, **kwd):

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

        groupElement = self.svgElementFactory.createGroup(*childElements)

        return groupElement

    def CDCHitToSVG(self, cdchit, **kwd):
        wirehit = Belle2.CDCLocalTracking.CDCWireHit(cdchit, 0)
        return self.CDCWireHitToSVG(wirehit, **kwd)

    def GFTrackCandToSVG(self, gftrackcand, **kwd):
        storeHits = Belle2.PyStoreArray('CDCHits')
        hitIDs = gftrackcand.getHitIDs(3)
        iterHitIDs = (hitIDs[iHit] for iHit in xrange(hitIDs.size()))
        cdcHits = [storeHits[hitID] for hitID in iterHitIDs]
        return self.toSVG(cdcHits, **kwd)

    def CDCTrajectory2DToSVG(self, fit, **kwd):
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
            impactParameter = fit.getGenCircle().distanceToOrigin()

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

    def toSVG(self, obj, **kwd):
    # implements dispatch on object type
        objtype = type(obj)

        toSVGForType = self.toSVGFunctions_by_type.get(objtype,
                self.IterableToSVG)

        return toSVGForType(obj, **kwd)


