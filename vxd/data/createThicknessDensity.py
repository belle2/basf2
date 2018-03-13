# Script of creating the xml file for tickness and density of each cell volume.
import xml.dom
import numpy as np

dom1 = xml.dom.getDOMImplementation()
doc = dom1.createDocument(None, "TicknessDensity", None)
top_element = doc.documentElement

thickness_CDCback = (np.ones((16, 144)) * 1.e-5).ravel()
thickness_CDCfor = (np.ones((16, 144)) * 1.e-5).ravel()
# Input the thickness of the matirial in the backward gap between  CDC and ECL
# thickness_CDCback = (np.loadtxt(open("thickness_CDCback.csv", "rb"), delimiter=",")).ravel()

# You can change the thickness of backward cell(i,j) by:
# thickness_CDCback[i,j] = thickness

# Input the thickness of the matirial in the forward gap between  CDC and ARICH
# thickness_CDCfor = (np.loadtxt(open("thickness_CDCfor.csv", "rb"), delimiter=",")).ravel()

thickness_CDClist = list(map(str, thickness_CDCback.tolist() + thickness_CDCfor.tolist()))


density_ARICH = (np.ones((3, 144)) * 1.29e-6).ravel()
density_TOP = (np.ones(144) * 1.29e-6).ravel()
density_ECLback = (np.ones((3, 144)) * 1.29e-6).ravel()
density_ECLfor = (np.ones((3, 144)) * 1.29e-6).ravel()

# Input the density of the matirial in the forward gap between  ARICH and TOP
# density_ARICH = (np.loadtxt(open("density_ARICH.csv", "rb"), delimiter=",")).ravel()

# Input the density of the matirial in the forward gap between  TOP and ECL
# density_TOP = (np.loadtxt(open("density_TOP.csv", "rb"), delimiter=",")).ravel()

# Input the density of the matirial in the backward gap between barrel and endcap of ECL
# density_ECLback = (np.loadtxt(open("density_ECLback.csv", "rb"), delimiter=",")).ravel()

# Input the density of the matirial in the forward gap between barrel and endcap of ECL
# density_ECLfor = (np.loadtxt(open("density_ECLfor.csv", "rb"), delimiter=",")).ravel()

density_list = list(map(str, density_ARICH.tolist() + density_TOP.tolist() + density_ECLback.tolist() + density_ECLfor.tolist()))

desc = {
    'IRCDCBack': u'segmentation in R of backward',
    'IPhiCDCBack': u'segmentation in Phi of backward',
    'IRCDCFor': u'segmentation in R of forward',
    'IPhiCDCFor': u'segmentation in Phi of forward',
    'thicknesses': u'thicknesses',
    'IRARICHFor': u'segmentation in R of forward',
    'IPhiARICHFor': u'segmentation in Phi of forward',
    'IPhiTOPFor': u'segmentation in Phi of forward',
    'IRECLBack': u'segmentation in R of backward',
    'IPhiECLBack': u'segmentation in Phi of backward',
    'IRECLFor': u'segmentation in R of forward',
    'IPhiECLFor': u'segmentation in Phi of forward',
    'density': u'Density'}

# The segmentation in R, Z and Phi. And thickness, density.
value = {
    'IRCDCBack': 16,
    'IPhiCDCBack': 144,
    'IRCDCFor': 16,
    'IPhiCDCFor': 144,
    'thicknesses': thickness_CDClist,
    'IRARICHFor': 3,
    'IPhiARICHFor': 144,
    'IPhiTOPFor': 144,
    'IRECLBack': 3,
    'IPhiECLBack': 144,
    'IRECLFor': 3,
    'IPhiECLFor': 144,
    'density': density_list}

elements = [
    'IRCDCBack',
    'IPhiCDCBack',
    'IRCDCFor',
    'IPhiCDCFor',
    'thicknesses',
    'IRARICHFor',
    'IPhiARICHFor',
    'IPhiTOPFor',
    'IRECLBack',
    'IPhiECLBack',
    'IRECLFor',
    'IPhiECLFor',
    'density']

unit = {'thicknesses': 'mm', 'density': 'g/cm3'}

for element in elements:
    sNode = doc.createElement(element)
    sNode.setAttribute('desc', desc[element])
    if element in unit.keys():
        sNode.setAttribute('unit', unit[element])
# print(element)
    if isinstance(value[element], list):
        textNode = doc.createTextNode("  ".join(value[element]))
    else:
        textNode = doc.createTextNode(str(value[element]))
    sNode.appendChild(textNode)
    top_element.appendChild(sNode)

xmlfile = open('GAPS-thickness-density_vac.xml', 'w')
doc.writexml(xmlfile, addindent=' ' * 4, newl='\n', encoding='utf-8')
xmlfile.close()
