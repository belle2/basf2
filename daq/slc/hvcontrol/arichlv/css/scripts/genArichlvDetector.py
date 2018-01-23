#!/usr/bin/python
# generator script for arich detector view opi
from xml.dom import minidom
import numpy as np
import math


def rotation_matrix(axis, theta):
    """
    Return the rotation matrix associated with counterclockwise rotation about
    the given axis by theta radians.Example:

v = [3, 5, 0]
axis = [4, 4, 1]
theta = 1.2
print(np.dot(rotation_matrix(axis,theta), v))
# [ 2.74911638  4.77180932  1.91629719]
    """
    axis = np.asarray(axis)
    axis = axis / math.sqrt(np.dot(axis, axis))
    a = math.cos(theta / 2.0)
    b, c, d = -axis * math.sin(theta / 2.0)
    aa, bb, cc, dd = a * a, b * b, c * c, d * d
    bc, ad, ac, ab, bd, cd = b * c, a * d, a * c, a * b, b * d, c * d
    return np.array([[aa + bb - cc - dd, 2 * (bc + ad), 2 * (bd - ac)],
                     [2 * (bc - ad), aa + cc - bb - dd, 2 * (cd + ab)],
                     [2 * (bd + ac), 2 * (cd - ab), aa + dd - bb - cc]])


def rotatePoint(centerPoint, point, angle):
    """Rotates a 2d point around another centerPoint. Angle is in degrees.
    Rotation is counter-clockwise.
    """
    angle = math.radians(angle)
    temp_point = point[0] - centerPoint[0], point[1] - centerPoint[1]
    temp_point = (
        temp_point[0] *
        math.cos(angle) -
        temp_point[1] *
        math.sin(angle),
        temp_point[0] *
        math.sin(angle) +
        temp_point[1] *
        math.cos(angle))
    temp_point = temp_point[0] + centerPoint[0], temp_point[1] + centerPoint[1]
    return temp_point


print(rotatePoint((1, 1), (2, 2), 45))


def rotatePolygon(polygon, theta):
    """Rotates the given polygon which consists of corners represented in 2D as (x,y),
    around the ORIGIN, clock-wise, theta degrees"""
    theta = math.radians(theta)
    rotatedPolygon = []
    for corner in polygon:
        rotatedPolygon.append(
            (corner[0] *
             math.cos(theta) -
             corner[1] *
                math.sin(theta),
                corner[0] *
                math.sin(theta) +
                corner[1] *
                math.cos(theta)))
    return rotatedPolygon


def translatePolygon(polygon, translation):
    """Translates the given polygon which consists of corners represented in 2D as (x,y),
    in the direction of translation"""
    translatedPolygon = []
    for corner in polygon:
        translatedPolygon.append((corner[0] + translation[0], corner[1] + translation[1]))
    return translatedPolygon


def scalePolygon(polygon, scale):
    """Translates the given polygon which consists of corners represented in 2D as (x,y),
    in the direction of translation"""
    scaledPolygon = []
    for corner in polygon:
        scaledPolygon.append((corner[0] * scale[0], corner[1] * scale[1]))
    return scaledPolygon


# print rotatePolygon(translatePolygon(scalePolygon(hapd, (0.5,0.5) ), (0.5,0.5) ),90)

opi = minidom.parse('arichlvtemplate.opi')

xmldoc = minidom.parse('genArichlvMap.xml')
mergers = xmldoc.getElementsByTagName('merger')
print(len(mergers))

rad = [574.17, 659.37, 744.46, 829.45, 914.38, 999.24, 1084.05]
nfi = [7, 8, 9, 10, 11, 12, 13]
fx = 3.0
xcenter = 115
ycenter = 115

display = opi.getElementsByTagName('display')[0]
widgets = opi.getElementsByTagName('widget')
for x in widgets:
    print(x.attributes['typeId'].value)
    if x.attributes['typeId'].value == "org.csstudio.opibuilder.widgets.polygon":
        polygon = x
    if x.attributes['typeId'].value == "org.csstudio.opibuilder.widgets.Label":
        label = x
    if x.attributes['typeId'].value == "org.csstudio.opibuilder.widgets.polyline":
        polyline = x
    if x.attributes['typeId'].value == "org.csstudio.opibuilder.widgets.Image":
        image = x
    w = x.parentNode
    w.removeChild(x)

oldpoints = polygon.getElementsByTagName('points')

for l in oldpoints:
    p = l.parentNode
    p.removeChild(l)

newImage = image.cloneNode(True)

node = newImage.getElementsByTagName('x')[0]
node.firstChild.replaceWholeText(str(320))

node = newImage.getElementsByTagName('y')[0]
node.firstChild.replaceWholeText(str(320))
display.appendChild(newImage)

for sectorid in [0, 1, 2, 3, 4, 5]:

    mline = translatePolygon(rotatePolygon([(10, 0), (50, 0)], sectorid * 60), (xcenter, ycenter))
    mlabel = translatePolygon(rotatePolygon([(25, 0)], (sectorid + 0.5) * 60), (xcenter, ycenter))
    newPolyline = polyline.cloneNode(True)

    points = opi.createElement("points")
    for corner in mline:
        point = opi.createElement("point")
        point.setAttribute("x", str(int(corner[0] * fx)))
        point.setAttribute("y", str(int(2 * ycenter * fx - corner[1] * fx)))
        points.appendChild(point)

    newPolyline.appendChild(points)

    newLabel = label.cloneNode(True)
    node = newLabel.getElementsByTagName('x')[0]
    node.firstChild.replaceWholeText(str(int(mlabel[0][0] * fx - 40)))

    node = newLabel.getElementsByTagName('y')[0]
    node.firstChild.replaceWholeText(str(int(2 * ycenter * fx - mlabel[0][1] * fx - 10)))

    node = newLabel.getElementsByTagName('text')[0]
    node.firstChild.replaceWholeText('Sector {}'.format(sectorid + 1))

    display.appendChild(newPolyline)
    display.appendChild(newLabel)
    for merger in mergers:

        print('S{} {}'.format(sectorid, merger.attributes['id'].value))
        crateid = int(merger.attributes['crateid'].value) + sectorid / 3
        slotid = int(merger.attributes['slotid'].value) + (sectorid % 3) * 2
        channelid = int(merger.attributes['channelid'].value)
        febs = merger.getElementsByTagName('feb')
        for feb in febs:
            r = int(feb.attributes['row'].value)
            c = int(feb.attributes['col'].value)

            scale = 8               # square size (cm)
            xc = rad[r - 1] * 0.1       # translation (cm)
            yc = 60.0 * (c - 0.5) / nfi[r - 1] + sectorid * 60  # rotation angle
            hapd = [(0.5, 0.5), (-0.5, 0.5), (-0.5, -0.5), (0.5, -0.5), (0.5, 0.5)]

            febhapd = translatePolygon(
                rotatePolygon(
                    translatePolygon(
                        scalePolygon(
                            hapd, (scale, scale)), (xc, 0)), yc), (xcenter, ycenter))

            xmin = 1000
            ymin = 1000

            newWidget = polygon.cloneNode(True)

            points = opi.createElement("points")
            for corner in febhapd:
                if (corner[0] < xmin):
                    xmin = corner[0]
                if (corner[1] < ymin):
                    ymin = corner[1]

                point = opi.createElement("point")
                point.setAttribute("x", str(int(corner[0] * fx)))
                point.setAttribute("y", str(int(2 * ycenter * fx - corner[1] * fx)))

                points.appendChild(point)

            newWidget.appendChild(points)

            scripts = newWidget.getElementsByTagName('scripts')[0]
            script = scripts.getElementsByTagName('path')[0]
            pvs = script.getElementsByTagName('pv')
            for l in pvs:
                p = l.parentNode
                p.removeChild(l)
            pv = opi.createElement("pv")
            pv.setAttribute("trig", 'true')
            htext = opi.createTextNode('nsm://get:ARICHLV:crate[{}]:slot[{}]:channel[{}]:switch'.format(crateid, slotid, channelid))
            pv.appendChild(htext)
            script.appendChild(pv)
            node = newWidget.getElementsByTagName('x')[0]
            node.firstChild.replaceWholeText(str(int(xmin * fx)))

            node = newWidget.getElementsByTagName('y')[0]
            node.firstChild.replaceWholeText(str(int(2 * ycenter * fx - ymin * fx)))

            node = newWidget.getElementsByTagName('pv_name')[0]
            node.firstChild.replaceWholeText(
                'nsm://get:ARICHLV:crate[{}]:slot[{}]:channel[{}]:switch'.format(crateid, slotid, channelid))

            node = newWidget.getElementsByTagName('tooltip')[0]
            node.firstChild.replaceWholeText(
                'S{} {} Cr{} Sl{} Ch{}'.format(
                    sectorid,
                    merger.attributes['id'].value,
                    crateid,
                    slotid,
                    channelid))

            display.appendChild(newWidget)

# print (opi.toxml())
# print opi.toprettyxml(indent="  ",encoding="UTF-8")
tmp_config = 'ArichlvDetector.opi'
fw = open(tmp_config, 'wb')
opi.writexml(fw)
fw.close()
