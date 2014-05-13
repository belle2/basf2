#!/usr/bin/env python
# -*- coding: utf-8 -*-

import math
import xml.dom.minidom

import converter
import primitives


class CDCSVGPlotter:

    """
    Helper class to generated the svg image from the various tracking objects.
    """

    def __init__(self, animate=False):
        """
        Constructor methode.
        @param animate switch indicating if an animated SVG should be generated
        """

        default_bound = primitives.Bound(top=-112, left=-112, right=112,
                bottom=112)
        default_width = 1120
        default_height = 1120
        default_viewbox = default_bound.viewbox

        ## Switch to indicating if an animated SVG should be generated
        self.animate = animate

        # Create the xml.dom.minidom document object set up with the right headers
        # The viewbox is set to a default value and is to be updated later
        document = primitives.createSVGDocument(default_width, default_height,
                default_viewbox)

        ## xml.dom.minidom.Document which holds the linked representation of all svg elements to be written to file
        self.document = document

        # Create helper object to convert the various dataobjects to svg
        # primitive element objects
        # The document serves as a factory for the xml.dom.minidom elements

        ## Helper object to translate the various tracking objects to xml.dom.minidom objects
        self.converter = converter.CDCDataobjectsConverter(document,
                animate=animate)

        defsFactory = primitives.SVGDefsFactory(document)
        defs = [defsFactory.createEndArrow('markerEndArrow')]
        defsElement = defsFactory.createDefs(*defs)

        self.document.documentElement.appendChild(defsElement)

    def clone(self):
        """
        Make a copy of the current status of the plotter.
        """

        clonedPlotter = CDCSVGPlotter()
        clonedPlotter.document = self.document.cloneNode(deep=True)
        return clonedPlotter

    def append(self, obj, **kwd):
        """
        Take the tracking object, translate it to svg and append it to the svg document
        """

        svgElement = self.converter.toSVG(obj, **kwd)
        if svgElement:
            mainElement = self.document.documentElement
            mainElement.appendChild(svgElement)

    def updateViewBox(self):
        """
        Adjust the view box to accomodate the current content of the document.
        """

        clipBound = primitives.Bound.fromSVG(self.document.documentElement)

        if not clipBound:
            defaultBound = primitives.Bound(top=-112, left=-112, right=112,
                    bottom=112)
            clipBound = defaultBound

        viewbox = clipBound.viewbox

        mainElement = self.document.documentElement
        mainElement.setAttribute('viewBox', viewbox)

    def toxml(self, totalPoints=1120 * 1120):
        """
        Generate the xml string for the current dom object representation.
        """

        mainElement = self.document.documentElement

        viewbox = mainElement.getAttribute('viewBox')
        bound = primitives.Bound.fromViewBox(viewbox)

        # set the image to proper aspect ratio
        svgHeight = round(math.sqrt(totalPoints * float(bound.height)
                          / bound.width))
        svgWidth = round(math.sqrt(totalPoints * float(bound.width)
                         / bound.height))

        mainElement.setAttribute('width', repr(svgWidth))
        mainElement.setAttribute('height', repr(svgHeight))

        return self.document.toprettyxml()

    def saveSVGFile(self, svgFileName='display.svg'):
        """
        Save the current dom object representation to disk.
        """

        output = open(svgFileName, 'w')
        output.write(self.toxml())
        output.close()


