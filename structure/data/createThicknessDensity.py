#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################
import xml.dom
import numpy as np

"""
Creating the xml file for tickness and density of each cell volume
for the service matirial.

Two kinds of material can be selected:

1. Defalut value:
    The thickness and density calculated from the photo of actual
    cable arrangement and density of each cable.

2. vacuum:
    No service matirial in the gap.
"""

dom1 = xml.dom.getDOMImplementation()
doc = dom1.createDocument(None, "TicknessDensity", None)
top_element = doc.documentElement

# Select which kind of material you want to input. Defalut value: 1, vacuum: 0
DefalutValue = 0

if DefalutValue == 0:
    # Input the thickness of the matirial in the backward gap between CDC and ECL, unit is 'mm'
    thickness_CDCback = (np.ones((16, 144)) * 1.0e-5).ravel()

    # Input the thickness of the matirial in the forward gap between CDC and ARICH
    thickness_CDCfor = (np.ones((16, 144)) * 1.0e-5).ravel()

    # Input the thickness of the matirial in the forward gap between ARICH and TOP
    thickness_ARICH = (np.ones((3, 144)) * 1.0e-5).ravel()

    # Input the thickness of the matirial in the backward gap between TOP and ECL
    thickness_TOPback = (np.ones(144) * 1.0e-5).ravel()

    # Input the thickness of the matirial in the forward gap between TOP and ECL
    thickness_TOPfor = (np.ones(144) * 1.0e-5).ravel()

    # Input the thickness of the matirial in the barrel gap between ECL and COIL
    thickness_ECLCOIL = (np.ones((92, 144)) * 1.0e-5).ravel()

    # Input the thickness of the matirial in the backward gap between barrel and endcap of ECL
    thickness_ECLback = (np.ones((3, 5, 144)) * 1.0e-5).ravel()

    # Input the thickness of the matirial in the forward gap between barrel and endcap of ECL
    thickness_ECLfor = (np.ones((3, 5, 144)) * 1.0e-5).ravel()

else:
    # Input the thickness of the matirial in the backward gap between  CDC and ECL
    thickness_CDCback = (np.loadtxt(open("thickness_CDCback.csv", "rb"), delimiter=",")).ravel()
    thickness_CDCback[thickness_CDCback < 1.0e-5] = 1.0e-5

    # You can change the thickness of backward cell(i,j) by:
    # thickness_CDCback[i,j] = thickness

    # Input the thickness of the matirial in the forward gap between CDC and ARICH
    thickness_CDCfor = (np.loadtxt(open("thickness_CDCfor.csv", "rb"), delimiter=",")).ravel()
    thickness_CDCfor[thickness_CDCfor < 1.0e-5] = 1.0e-5

    # Input the thickness of the matirial in the forward gap between ARICH and TOP
    thickness_ARICH = (np.loadtxt(open("thickness_ARICH.csv", "rb"), delimiter=",")).ravel()
    thickness_ARICH[thickness_ARICH < 1.0e-5] = 1.0e-5

    # Input the thickness of the matirial in the backward gap between TOP and ECL
    thickness_TOPback = (np.loadtxt(open("thickness_TOP_back.csv", "rb"), delimiter=",")).ravel()
    thickness_TOPback[thickness_TOPback < 1.0e-5] = 1.0e-5

    # Input the thickness of the matirial in the forward gap between TOP and ECL
    thickness_TOPfor = (np.loadtxt(open("thickness_TOP_for.csv", "rb"), delimiter=",")).ravel()
    thickness_TOPfor[thickness_TOPfor < 1.0e-5] = 1.0e-5

    # Input the thickness of the matirial in the barrel gap between ECL and Coil
    thickness_ECLCOIL = (np.loadtxt(open("thickness_ECLCOIL.csv", "rb"), delimiter=",")).ravel()
    thickness_ECLCOIL[thickness_ECLCOIL < 1.0e-5] = 1.0e-5

    # Input the thickness of the matirial in the backward gap between barrel and endcap of ECL
    thickness_ECLback = (np.loadtxt(open("thickness_ECLback.csv", "rb"), delimiter=",")).ravel()
    thickness_ECLback[thickness_ECLback < 1.0e-5] = 1.0e-5

    # Input the thickness of the matirial in the forward gap between barrel and endcap of ECL
    thickness_ECLfor = (np.loadtxt(open("thickness_ECLfor.csv", "rb"), delimiter=",")).ravel()
    thickness_ECLfor[thickness_ECLfor < 1.0e-5] = 1.0e-5

thickness_list = list(map(str, thickness_CDCback.tolist() +
                          thickness_CDCfor.tolist() +
                          thickness_ARICH.tolist() +
                          thickness_TOPback.tolist() +
                          thickness_TOPfor.tolist() +
                          thickness_ECLCOIL.tolist() +
                          thickness_ECLback.tolist() +
                          thickness_ECLfor.tolist()))


# Density of the servece material in each gap. Unity is 'g/cm3'
if DefalutValue == 0:
    density_ARICH = (np.ones(3) * 1.29e-10).ravel()
    density_TOPback = (np.ones(1) * 1.29e-10).ravel()
    density_TOPfor = (np.ones(1) * 1.29e-10).ravel()
    density_ECLCOIL = (np.ones(1) * 1.29e-10).ravel()
    density_ECLback = (np.ones(5) * 1.29e-10).ravel()
    density_ECLfor = (np.ones(5) * 1.29e-10).ravel()
else:
    density_ARICH = (np.ones(3) * 8.96).ravel()
    density_TOPback = (np.ones(1) * 8.96).ravel()
    density_TOPfor = (np.ones(1) * 8.96).ravel()
    density_ECLCOIL = (np.ones(1) * 8.96).ravel()
    density_ECLback = (np.ones(5) * 8.96).ravel()
    density_ECLfor = (np.ones(5) * 8.96).ravel()


density_list = list(map(str, density_ARICH.tolist() + density_TOPback.tolist() +
                        density_TOPfor.tolist() + density_ECLCOIL.tolist() +
                        density_ECLback.tolist() + density_ECLfor.tolist()))

desc = {
    'IRCDCBack': u'segmentation in R of backward',
    'IPhiCDCBack': u'segmentation in Phi of backward',
    'IRCDCFor': u'segmentation in R of forward',
    'IPhiCDCFor': u'segmentation in Phi of forward',
    'thicknesses': u'thicknesses',
    'IZARICHFor': u'segmentation in Z of forward',
    'IPhiARICHFor': u'segmentation in Phi of forward',
    'IPhiTOPBack': u'segmentation in Phi of backward',
    'IPhiTOPFor': u'segmentation in Phi of forward',
    'IZECLCOILBar': u'segmentation in Z of barrel',
    'IPhiECLCOILBar': u'segmentation in Phi of barrel',
    'IRECLBack': u'segmentation in R of backward',
    'IZECLBack': u'segmentation in Z of backward',
    'IPhiECLBack': u'segmentation in Phi of backward',
    'IRECLFor': u'segmentation in R of forward',
    'IZECLFor': u'segmentation in Z of forward',
    'IPhiECLFor': u'segmentation in Phi of forward'}

# The segmentation in R, Z and Phi. And thickness.
value = {
    'IRCDCBack': 16,
    'IPhiCDCBack': 144,
    'IRCDCFor': 16,
    'IPhiCDCFor': 144,
    'thicknesses': thickness_list,
    'IZARICHFor': 3,
    'IPhiARICHFor': 144,
    'IPhiTOPBack': 144,
    'IPhiTOPFor': 144,
    'IZECLCOILBar': 92,
    'IPhiECLCOILBar': 144,
    'IRECLBack': 3,
    'IZECLBack': 5,
    'IPhiECLBack': 144,
    'IRECLFor': 3,
    'IZECLFor': 5,
    'IPhiECLFor': 144}

elements = [
    'IRCDCBack',
    'IPhiCDCBack',
    'IRCDCFor',
    'IPhiCDCFor',
    'thicknesses',
    'IZARICHFor',
    'IPhiARICHFor',
    'IPhiTOPBack',
    'IPhiTOPFor',
    'IZECLCOILBar',
    'IPhiECLCOILBar',
    'IRECLBack',
    'IZECLBack',
    'IPhiECLBack',
    'IRECLFor',
    'IZECLFor',
    'IPhiECLFor']

unit = {'thicknesses': 'mm'}

for element in elements:
    sNode = doc.createElement(element)
    sNode.setAttribute('desc', desc[element])
    if element in unit.keys():
        sNode.setAttribute('unit', unit[element])
    if isinstance(value[element], list):
        textNode = doc.createTextNode("  ".join(value[element]))
    else:
        textNode = doc.createTextNode(str(value[element]))
    sNode.appendChild(textNode)
    top_element.appendChild(sNode)

xmlfile = open('GAPS-thickness-density.xml', 'w')
doc.writexml(xmlfile, addindent=' ' * 4, newl='\n', encoding='utf-8')
xmlfile.close()
