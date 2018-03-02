import xml.dom
import numpy as np

dom1 = xml.dom.getDOMImplementation()
doc = dom1.createDocument(None, "TicknessDensity", None)
top_element = doc.documentElement

thickness_CDCback = (np.ones((16, 144)) * 18).ravel()
thickness_CDCfor = (np.ones((16, 144)) * 10).ravel()
thickness_CDClist = map(str, thickness_CDCback.tolist() + thickness_CDCfor.tolist())

density_ARICH = (np.ones((3, 144)) * 1.29e-3).ravel()
density_TOP = (np.ones(144) * 1.28e-3).ravel()
density_ECLback = (np.ones((3, 144)) * 1.27e-3).ravel()
density_ECLfor = (np.ones((3, 144)) * 1.26e-3).ravel()
density_list = map(str, density_ARICH.tolist() + density_TOP.tolist() + density_ECLback.tolist() + density_ECLfor.tolist())

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

xmlfile = open('GAPS-thickness-density.xml', 'w')
doc.writexml(xmlfile, addindent=' ' * 4, newl='\n', encoding='utf-8')
xmlfile.close()
