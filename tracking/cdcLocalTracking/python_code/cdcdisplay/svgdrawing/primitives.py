#!/usr/bin/env python
# -*- coding: utf-8 -*-

import xml.dom.minidom


def createSVGDocument(width, height, viewBox=''):
    """Creates a xml.dom.Document setup with the right header for svg content

  Parameters:
  width - width of the svg image
  height - height of the svg image

  Optional parameters:
  viewbox - string of the format '{x_min} {y_min} {x_max} {y_max}' setting
            up the local coordinate system within the image
            defaults to '0 0 {width} {height}'
  """

    implementation = xml.dom.minidom.getDOMImplementation()
    doctype = implementation.createDocumentType(qualifiedName='svg',
            publicId='-//W3C//DTD SVG 1.1//EN',
            systemId='http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd')

    document = implementation.createDocument(None, 'svg', doctype)

    svgElement = document.documentElement

    svgElement.setAttribute('xmlns', 'http://www.w3.org/2000/svg')
    svgElement.setAttributeNS('xmlns', 'xlink', 'http://www.w3.org/1999/xlink')
    svgElement.setAttributeNS('xmlns', 'ev',
                              'http://www.w3.org/2001/xml-events')
    svgElement.setAttribute('version', '1.1')
    svgElement.setAttribute('baseProfile', 'full')

    svgElement.setAttribute('width', str(width))
    svgElement.setAttribute('height', str(height))

    if not viewBox:
        viewBox = '0 0 ' + str(width) + ' ' + str(height)

    svgElement.setAttribute('viewBox', viewBox)

    return document


class Bound:

    """Class representing the rectangular boundary of an svg
            element in its local coordinates"""

    ignoreList = ['defs', 'set']

    @classmethod
    def fromViewBox(cls, viewbox=''):
        """Constucts a bound object from a viewbox sting"""

        if viewbox:

            (left_str, top_str, width_str, height_str) = viewbox.split(' ')

            left = float(left_str)
            top = float(top_str)
            width = float(width_str)
            height = float(height_str)

            bottom = top + height
            right = left + width

            return cls(top=top, left=left, bottom=bottom, right=right)
        else:

            return None

    @classmethod
    def fromSVG(cls, svgElement):
        """Generates a Bound object by traversing a svg DOM tree combining the individual bounds"""

        tagName = svgElement.tagName

        if tagName in cls.ignoreList:
            return None
        elif tagName == 'g' or tagName == 'svg':

            bound = None
            for childElement in svgElement.childNodes:
                if bound is None:
                    bound = cls.fromSVG(childElement)
                else:
                    bound += cls.fromSVG(childElement)
            return bound
        elif tagName == 'circle':

            center_x = float(svgElement.getAttribute('cx'))
            center_y = float(svgElement.getAttribute('cy'))

            radius = float(svgElement.getAttribute('r'))

            stroke_width_raw = svgElement.getAttribute('stroke-width')
            if stroke_width_raw:
                stroke_width = float(stroke_width_raw)
            else:
                style = svgElement.getAttribute('style')
                styleDict = converter.createStyleDictFromText(style)
                stroke_width_raw = styleDict.get('stroke-width', '1')
                stroke_width = float(stroke_width_raw)

            totalradius = radius + stroke_width / 2.0

            right = center_x + totalradius
            left = center_x - totalradius

            bottom = center_y + totalradius
            top = center_y - totalradius

            return cls(top=top, left=left, bottom=bottom, right=right)
        elif tagName == 'line':

            x1 = float(svgElement.getAttribute('x1'))
            x2 = float(svgElement.getAttribute('x2'))

            y1 = float(svgElement.getAttribute('y1'))
            y2 = float(svgElement.getAttribute('y2'))

            right = max((x1, x2))
            left = min((x1, x2))

            bottom = max((y1, y2))
            top = min((y1, y2))

            return cls(top=top, left=left, bottom=bottom, right=right)
        elif tagName == 'path':

            d_value = svgElement.getAttribute('d')
            d_parts = d_value.split(' ')
            if d_parts[0] == 'M' and d_parts[3] == 'A' and len(d_parts) == 11 \
                and d_parts[4] == d_parts[5]:
        # Path is a circle arc
        # d_value = "M {x0} {y0} A {r} {r} {rotation} {long_arc}
        # {sweep_flag} {x1} {y1}"
                x0 = float(d_parts[1])
                y0 = float(d_parts[2])
                r = float(d_parts[4])
        # r = float(d_parts[5])
                rotation = float(d_parts[6])

                long_arc = int(d_parts[7]) != 0
                sweep_flag = int(d_parts[8]) != 0

                x1 = float(d_parts[9])
                y1 = float(d_parts[10])

                return None

        print 'Tag ' + tagName + ' not handled'
        return None

    def __init__(
        self,
        top=0,
        left=0,
        bottom=0,
        right=0,
        ):
        """
        Construction method taking the boundaries
        @param top Top boundary
        @param left Left boundary
        @param bottom Bottom boundary
        @param right Right boundary
        """

        # # The top boundary
        self.top = top
        # # The bottom boundary
        self.bottom = bottom
        # # The left boundary
        self.left = left
        # # The right boundary
        self.right = right

    @property
    def width(self):
        """Getter property for the width of the bound area"""

        return self.right - self.left

    @property
    def height(self):
        """Getter property for the height of the bound area"""

        return self.bottom - self.top

    def __iadd__(self, other):
        """
        Function to update the current boundary object extending it such that includes the area of the given other boundary object. (Invoked by expressions like this += other)
        """

        if other is None:
            return self

        if self.top > other.top:
            self.top = other.top

        if self.bottom < other.bottom:
            self.bottom = other.bottom

        if self.left > other.left:
            self.left = other.left

        if self.right < other.right:
            self.right = other.right

        return self

    def __str__(self):
        """Informal string representation of the boundaries"""

        return 'top = ' + repr(self.top) + ' bottom = ' + repr(self.bottom) \
            + ' left = ' + repr(self.left) + ' right = ' + repr(self.right)

    @property
    def viewbox(self):
        """Property combining the boundaries to a view box string usable in the view box attribute of the svg toplevel element."""

        return repr(self.left) + ' ' + repr(self.top) + ' ' + repr(self.width) \
            + ' ' + repr(self.height)


class SVGDefsFactory:

    """Factory to compose definition elements to be attached to an SVG document"""

    def __init__(self, elementFactory):
        """Constuction method receiving to fundamental element factory to be used to create the XML DOM elements"""

        # # The fundamental factory to generate the DOM elements
        self.elementFactory = elementFactory

    def createDefs(self, *defs):
        """Creates a defs group from a variadic number of XML DOM elements"""

        defsElement = self.elementFactory.createElement('defs')

        for defElement in defs:
            defsElement.appendChild(defElement)

        return defsElement

    def createEndArrow(self, markerName='endArrow', color='darkblue'):
        """Creates a defintion of an end arrow to be referenced by other svg elements"""

        return self.createSimpleEndArrow(markerName, color)

    def createSimpleEndArrow(self, markerName='endArrow', color='darkblue'):
        """Creates a simple defintion of an end arrow to be referenced by other svg elements"""

        arrowMarkerString = \
            """
    <marker id="%(markerName)s" viewBox="0 0 10 10" refX="1" refY="5"
    markerUnits="strokeWidth" orient="auto"
    markerWidth="4" markerHeight="3">
    <polyline points="0,0 10,5 0,10 1,5" fill="%(fill)s" />
    </marker>
    """ \
            % {'markerName': markerName, 'fill': color}

        arrowMarkerDocument = xml.dom.minidom.parseString(arrowMarkerString)
        return arrowMarkerDocument.documentElement


# original


class SVGPrimitivesFactory:

    """Factory to compose geometric primitiv XML DOM elements"""

    def __init__(self, elementFactory):
        """Constuction method receiving to fundamental element factory to be used to create the XML DOM elements"""

        # # The fundamental factory to generate the DOM elements
        self.elementFactory = elementFactory

    def createCircle(
        self,
        center=(0.0, 0.0),
        radius=1.0,
        **kwd
        ):
        """Construct a SVG circle from center and radius. Additional keyword arguments are added a attributes to the element."""

        cx = center[0]
        cy = center[1]

        circleElement = self.elementFactory.createElement('circle')

        circleElement.setAttribute('cx', repr(cx))
        circleElement.setAttribute('cy', repr(cy))

        circleElement.setAttribute('r', repr(radius))
                # the stroke-width fills the rest

        for (attrName, attrValue) in kwd.iteritems():
            circleElement.setAttribute(attrName, str(attrValue))

        return circleElement

    def createLine(
        self,
        fromPoint=(0.0, 0.0),
        toPoint=(1.0, 1.0),
        **kwd
        ):
        """Construct a SVG line between two points. Additional keyword arguments are added a attributes to the element."""

        lineElement = self.elementFactory.createElement('line')

        lineElement.setAttribute('x1', repr(fromPoint[0]))
        lineElement.setAttribute('y1', repr(fromPoint[1]))

        lineElement.setAttribute('x2', repr(toPoint[0]))
        lineElement.setAttribute('y2', repr(toPoint[1]))

        for (attrName, attrValue) in kwd.iteritems():
            lineElement.setAttribute(attrName, str(attrValue))

        return lineElement

    def createCircleArc(
        self,
        fromPoint,
        toPoint,
        radius,
        long_arc=False,
        sweep_flag=False,
        **kwd
        ):
        """
        Construct a SVG circle arc between two points and radius. 
        To select one of four remaining posibilities the flags long_arc and sweep_flag have to be given.
        Additional keyword arguments are added a attributes to the element.
        """

        if long_arc:
            long_arc = 1
        else:
            long_arc = 0

        if sweep_flag:
            sweep_flag = 1
        else:
            sweep_flag = 0

    # Compose path specification
    # d_value = "M {x0} {y0} A {rx} {ry} {rotation}
    # {long_arc} {sweep_flag} {x1} {y1}"

        d_value_parts = [
            'M',
            str(fromPoint[0]),
            str(fromPoint[1]),
            'A',
            str(radius),
            str(radius),
            '0',
            str(long_arc),
            str(sweep_flag),
            str(toPoint[0]),
            str(toPoint[1]),
            ]

        d_value = ' '.join(d_value_parts)

        pathElement = self.elementFactory.createElement('path')
        pathElement.setAttribute('d', d_value)

        for (attrName, attrValue) in kwd.iteritems():
            pathElement.setAttribute(attrName, str(attrValue))

        return pathElement

    def createSet(
        self,
        attr,
        value,
        **kwd
        ):
        """Construct a SVG animation set element, which sets a specific attribut of the surrounding svg element at a specified."""

        setElement = self.elementFactory.createElement('set')

        setElement.setAttribute('attributeName', attr)
        setElement.setAttribute('to', value)

        for (attrName, attrValue) in kwd.iteritems():
            setElement.setAttribute(attrName, str(attrValue))

        return setElement

    def createGroup(self, *pos, **kwd):
        """Constructs a SVG group element from a variadic list of other primitive svg elements"""

        return self.createGroupFromIterable(pos, **kwd)

    def createGroupFromIterable(self, iterChildElements, **kwd):
        """Constructs a SVG group element from an iterable of other primitive svg elements"""

        groupElement = self.elementFactory.createElement('g')

        for childElement in iterChildElements:
            if childElement:
                groupElement.appendChild(childElement)

        for (attrName, attrValue) in kwd.iteritems():
            groupElement.setAttribute(attrName, str(attrValue))

        return groupElement


