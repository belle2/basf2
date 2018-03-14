#!/usr/bin/python
# generator script for arich status view opi
from xml.dom import minidom
import numpy as np
import math


opi = minidom.parse('arichlvtemplate.opi')


xmldoc = minidom.parse('genArichlvMap.xml')
mergers = xmldoc.getElementsByTagName('merger')
print(len(mergers))


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
    if x.attributes['typeId'].value == "org.csstudio.opibuilder.widgets.groupingContainer":
        grouping = x
    if x.attributes['typeId'].value == "org.csstudio.opibuilder.widgets.linkingContainer":
        linking = x
    w = x.parentNode
    w.removeChild(x)

oldpoints = polygon.getElementsByTagName('points')

for l in oldpoints:
    p = l.parentNode
    p.removeChild(l)

newGrouping = grouping.cloneNode(True)

node = newGrouping.getElementsByTagName('x')[0]
node.firstChild.replaceWholeText(str(2))

node = newGrouping.getElementsByTagName('y')[0]
node.firstChild.replaceWholeText(str(2))


cnt = -1
asize = 25

node = newGrouping.getElementsByTagName('width')[0]
node.firstChild.replaceWholeText(str(900))

node = newGrouping.getElementsByTagName('height')[0]
node.firstChild.replaceWholeText(str(asize * 25 + 2))

for crateid in [1, 2]:
    for slotid in [0, 1, 2, 3, 4, 5]:
        for channelid in [0, 4]:
            cnt = cnt + 1
            newLinking = linking.cloneNode(True)
            node = newLinking.getElementsByTagName('x')[0]
            node.firstChild.replaceWholeText(str(2))

            node = newLinking.getElementsByTagName('y')[0]
            node.firstChild.replaceWholeText(str(cnt * asize + 2))

            node = newLinking.getElementsByTagName('height')[0]
            node.firstChild.replaceWholeText(str(asize - 1))

            node = newLinking.getElementsByTagName('width')[0]
            node.firstChild.replaceWholeText(str(895))

            node = newLinking.getElementsByTagName('name')[0]
            node.firstChild.replaceWholeText('Status:crate[{}]:slot[{}]:channel[{}]'.format(crateid, slotid, channelid))

            node = newLinking.getElementsByTagName('opi_file')[0]
            node.firstChild.replaceWholeText('arichlv_chstatus_x4.opi')

            macros = newLinking.getElementsByTagName('macros')[0]

            parameter = opi.createElement("CRATEID")
            htext = opi.createTextNode('{}'.format(crateid))
            parameter.appendChild(htext)
            macros.appendChild(parameter)

            parameter = opi.createElement("SLOTID")
            htext = opi.createTextNode('{}'.format(slotid))
            parameter.appendChild(htext)
            macros.appendChild(parameter)
            for k in range(0, 4):
                parameter = opi.createElement("ID" + str(k))
                htext = opi.createTextNode('{}'.format(int(channelid) + k))
                parameter.appendChild(htext)
                macros.appendChild(parameter)
            newGrouping.appendChild(newLinking)

display.appendChild(newGrouping)

tmp_config = 'ArichlvStatus.opi'
fw = open(tmp_config, 'wb')
opi.writexml(fw)
fw.close()
