#!/usr/bin/env python
# -*- coding: utf-8 -*-

import math

from ROOT import gSystem
gSystem.Load('libtracking_trackFindingCDC')

from ROOT import Belle2  # make Belle2 namespace available


class CDCSVGPlotter:

    """
    Helper class to generated the svg image from the various tracking objects.
    This is a tiny wrapper around the TrackFindingCDC.EventDataPlotter.
    """

    def __init__(self, animate=False):
        """
        Constructor methode.
        @param animate switch indicating if an animated SVG should be generated
        """
        top = -112
        left = -112
        right = 112
        bottom = 112

        default_bound = Belle2.TrackFindingCDC.BoundingBox(left, bottom, right, top)
        default_width = 1120
        default_height = 1120

        # Switch to indicating if an animated SVG should be generated
        self.animate = animate
        self.eventdata_plotter = Belle2.TrackFindingCDC.EventDataPlotter(self.animate)

        self.eventdata_plotter.setBoundingBox(default_bound)
        self.eventdata_plotter.setCanvasHeight(default_height)
        self.eventdata_plotter.setCanvasWidth(default_width)

    def clone(self):
        """
        Make a copy of the current status of the plotter.
        """
        cloned_plotter = CDCSVGPlotter(self.animate)
        cloned_plotter.eventdata_plotter = Belle2.TrackFindingCDC.EventDataPlotter(self.eventdata_plotter)
        return cloned_plotter

    def styling_to_attribute_map(self, **styling):
        attribute_map = Belle2.TrackFindingCDC.PrimitivePlotter.AttributeMap()

        for key, value in styling.items():
            attribute_map[str(key)] = str(value)

        return attribute_map

    def draw_superlayer_boundaries(self, **styling):
        attribute_map = self.styling_to_attribute_map(**styling)
        self.eventdata_plotter.drawSuperLayerBoundaries(attribute_map)

    def draw_interaction_point(self):
        self.eventdata_plotter.drawInteractionPoint()

    def draw_outer_cdc_wall(self, **styling):
        attribute_map = self.styling_to_attribute_map(**styling)
        self.eventdata_plotter.drawOuterCDCWall(attribute_map)

    def draw_inner_cdc_wall(self, **styling):
        attribute_map = self.styling_to_attribute_map(**styling)
        self.eventdata_plotter.drawInnerCDCWall(attribute_map)

    @staticmethod
    def unpack_attributes(styling, i_obj=0, obj=None):
        """Mapping function to unpack the attributes from the attribute maps. Mechanism and interface inspired by d3.js"""
        result = {}
        for (key, value) in styling.items():
            if callable(value):
                result[key] = value(i_obj, obj)
            elif isinstance(value, str):
                result[key] = value
            elif hasattr(value, '__getitem__'):
                result[key] = value[i_obj % len(value)]
            else:
                result[key] = value
        return result

    def draw_iterable(self, iterable, **styling):
        draw = self.draw
        unpack_attributes = self.unpack_attributes
        for (i_obj, obj) in enumerate(iterable):
            obj_styling = unpack_attributes(styling, i_obj, obj)
            draw(obj, **obj_styling)

    def draw_storevector(self, storeobj_name, **styling):
        print 'Drawing vector from DataStore:', storeobj_name,
        print
        pystoreobj = Belle2.PyStoreObj(storeobj_name)

        if pystoreobj:
            # Wrapper around std::vector like
            wrapped_vector = pystoreobj.obj()
            vector = wrapped_vector.get()

            print 'with', vector.size(), 'entries'
            print 'Attributes are'
            for (key, value) in styling.items():
                print str(key), ':', str(value)

            self.draw_iterable(vector, **styling)

        else:
            print "### not present in the DataStore"
            print "Current content of the DataStore"
            print "StoreArrays:"
            Belle2.PyStoreArray.printList()
            print "StoreObjPtr:"
            Belle2.PyStoreObj.printList()

    def draw_storearray(self, storearray_name, **styling):
        print 'Drawing StoreArray:', storearray_name,
        print
        storearray = Belle2.PyStoreArray(storearray_name)
        if storearray:
            print 'with', storearray.getEntries(), 'entries'
            print 'Attributes are'
            for (key, value) in styling.items():
                print str(key), ':', str(value)

            self.draw_iterable(storearray, **styling)

        else:
            print "### not present in the DataStore"
            print "Current content of the DataStore"
            print "StoreArrays:"
            Belle2.PyStoreArray.printList()
            print "StoreObjPtr:"
            Belle2.PyStoreObj.printList()

    def draw(self, obj, **styling):
        attribute_map = self.styling_to_attribute_map(**styling)
        self.eventdata_plotter.draw(obj, attribute_map)

    def saveSVGFile(self, svgFileName='display.svg'):
        """
        Save the current dom object representation to disk.
        """

        eventdata_plotter = self.eventdata_plotter

        boundingBox = eventdata_plotter.getBoundingBox()

        height = boundingBox.getHeight()
        width = boundingBox.getWidth()

        totalPoints = 1120 * 1120
        svgHeight = round(math.sqrt(totalPoints * float(height)
                                    / width))
        svgWidth = round(math.sqrt(totalPoints * float(width)
                                   / height))

        eventdata_plotter.setCanvasHeight(svgHeight)
        eventdata_plotter.setCanvasWidth(svgWidth)

        return eventdata_plotter.save(svgFileName)
