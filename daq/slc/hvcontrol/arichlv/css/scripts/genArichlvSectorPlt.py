#!/usr/bin/python
# generator script for arich status view opi
from xml.dom import minidom
import numpy as np
import math


import xml.dom.minidom

document = """\
<databrowser>
    <title></title>
    <save_changes>true</save_changes>
    <show_legend>true</show_legend>
    <show_toolbar>true</show_toolbar>
    <grid>false</grid>
    <scroll>true</scroll>
    <update_period>3.0</update_period>
    <scroll_step>5</scroll_step>
    <start>-30.45 min</start>
    <end>now</end>
    <archive_rescale>STAGGER</archive_rescale>
    <background>
        <red>255</red>
        <green>255</green>
        <blue>255</blue>
    </background>
    <title_font>Ubuntu|16|1</title_font>
    <label_font>Sans|10|1</label_font>
    <scale_font>Sans|10|0</scale_font>
    <legend_font>Ubuntu|10|0</legend_font>
    <axes>
        <axis>
            <visible>true</visible>
            <name>Vmon [V]</name>
            <use_axis_name>false</use_axis_name>
            <use_trace_names>true</use_trace_names>
            <right>false</right>
            <color>
                <red>0</red>
                <green>0</green>
                <blue>0</blue>
            </color>
            <label_font>|10|0</label_font>
            <scale_font>|10|0</scale_font>
            <min>0.0</min>
            <max>4.0</max>
            <grid>true</grid>
            <autoscale>false</autoscale>
            <log_scale>false</log_scale>
        </axis>
    </axes>
    <annotations>
    </annotations>
    <pvlist>
    </pvlist>
</databrowser>
"""

pv = """\
<pv><display_name>c0</display_name>
            <visible>true</visible>
            <name>nsm://get:ARICHLV:crate[1]:slot[0]:channel[0]:vmon</name>
            <axis>0</axis>
            <color>
                <red>0</red>
                <green>0</green>
                <blue>255</blue>
            </color>
            <trace_type>AREA</trace_type>
            <linewidth>2</linewidth>
            <point_type>NONE</point_type>
            <point_size>2</point_size>
            <waveform_index>0</waveform_index>
            <period>1.0</period>
            <ring_size>5000</ring_size>
            <request>OPTIMIZED</request>
</pv>
"""


for mon in ['vmon', 'cmon']:
    for sectorid in range(0, 6):
        opi = minidom.parseString(document)

        cnt = 0

        for slot in range(0, 2):
            for ch in range(0, 8):
                crateid = sectorid / 3 + 1
                slotid = (sectorid % 3) * 2 + slot
                pvnode = minidom.parseString(pv)

                name = 'nsm://get:ARICHLV:crate[{}]:slot[{}]:channel[{}]:{}'.format(crateid, slotid, ch, mon)
                node = pvnode.getElementsByTagName('name')[0]
                node.firstChild.replaceWholeText(name)
                mlist = ['a', 'b', 'c', 'd']
                chname = '{}{}'.format(mlist[cnt % 4], cnt / 4)
                node = pvnode.getElementsByTagName('display_name')[0]
                node.firstChild.replaceWholeText(chname)

                mcolors = [[0, 0, 255], [0, 255, 0], [255, 0, 0], [255, 0, 255]]
                rgb = mcolors[cnt / 4]
                node = pvnode.getElementsByTagName('red')[0]
                node.firstChild.replaceWholeText(rgb[0])
                node = pvnode.getElementsByTagName('green')[0]
                node.firstChild.replaceWholeText(rgb[1])
                node = pvnode.getElementsByTagName('blue')[0]
                node.firstChild.replaceWholeText(rgb[2])

                pvlistnode = opi.getElementsByTagName('pvlist')[0]
                pvlistnode.appendChild(pvnode.firstChild)
                cnt = cnt + 1
        tmp_config = '{}-Sector{}.plt'.format(mon, sectorid)
        fw = open(tmp_config, 'wb')
        opi.writexml(fw)
        fw.close()
