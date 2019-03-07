# Script of creating the xml file for tickness and density of each cell volume.
import xml.dom
import numpy as np

dom1 = xml.dom.getDOMImplementation()
doc = dom1.createDocument(None, "TicknessDensity", None)
top_element = doc.documentElement

thickness_CDCback = (np.ones((16, 144)) * 1.0e-5).ravel()  # 1.0e-5
thickness_CDCfor = (np.ones((16, 144)) * 1.0e-5).ravel()
thickness_ARICH = (np.ones((3, 144)) * 1.0e-5).ravel()
thickness_TOPback = (np.ones(144) * 1.0e-5).ravel()
thickness_TOPfor = (np.ones(144) * 1.0e-5).ravel()
thickness_ECLback = (np.ones((3, 5, 144)) * 1.0e-5).ravel()
thickness_ECLfor = (np.ones((3, 5, 144)) * 1.0e-5).ravel()

# Input the thickness of the matirial in the backward gap between  CDC and ECL
# thickness_CDCback = (np.loadtxt(open("thickness_CDCback.csv", "rb"), delimiter=",")).ravel()
# thickness_CDCback[thickness_CDCback < 1.0e-5] = 1.0e-5

# You can change the thickness of backward cell(i,j) by:
# thickness_CDCback[i,j] = thickness

# Input the thickness of the matirial in the forward gap between  CDC and ARICH
# thickness_CDCfor = (np.loadtxt(open("thickness_CDCfor.csv", "rb"), delimiter=",")).ravel()
# thickness_CDCfor[thickness_CDCfor < 1.0e-5] = 1.0e-5

# Input the thickness of the matirial in the forward gap between  TOP and ECL
# thickness_TOPback = (np.loadtxt(open("thickness_TOP_back.csv", "rb"), delimiter=",")).ravel()
# thickness_TOPback[thickness_TOPback < 1.0e-5] = 1.0e-5

# Input the thickness of the matirial in the forward gap between  TOP and ECL
# thickness_TOPfor = (np.loadtxt(open("thickness_TOP_for.csv", "rb"), delimiter=",")).ravel()
# thickness_TOPfor[thickness_TOPfor < 1.0e-5] = 1.0e-5

# Input the thickness of the matirial in the forward gap between  ARICH and TOP
# thickness_ARICH = (np.loadtxt(open("thickness_ARICH.csv", "rb"), delimiter=",")).ravel()
# thickness_ARICH[thickness_ARICH < 1.0e-5] = 1.0e-5

# Input the thickness of the matirial in the backward gap between barrel and endcap of ECL
# thickness_ECLback = (np.loadtxt(open("thickness_ECLback.csv", "rb"), delimiter=",")).ravel()
# thickness_ECLback[thickness_ECLback < 1.0e-5] = 1.0e-5

# Input the thickness of the matirial in the forward gap between barrel and endcap of ECL
# thickness_ECLfor = (np.loadtxt(open("thickness_ECLfor.csv", "rb"), delimiter=",")).ravel()
# thickness_ECLfor[thickness_ECLfor < 1.0e-5] = 1.0e-5

thickness_list = list(map(str, thickness_CDCback.tolist() +
                          thickness_CDCfor.tolist() +
                          thickness_ARICH.tolist() +
                          thickness_TOPback.tolist() +
                          thickness_TOPfor.tolist() +
                          thickness_ECLback.tolist() +
                          thickness_ECLfor.tolist()))


density_ARICH = (np.ones(3) * 1.29e-10).ravel()  # 1.29e-10
density_TOPback = (np.ones(1) * 1.29e-10).ravel()
density_TOPfor = (np.ones(1) * 1.29e-10).ravel()
density_ECLback = (np.ones(5) * 1.29e-10).ravel()
density_ECLfor = (np.ones(5) * 1.29e-10).ravel()


density_list = list(map(str, density_ARICH.tolist() + density_TOPback.tolist() +
                        density_TOPfor.tolist() + density_ECLback.tolist() + density_ECLfor.tolist()))

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
    'IRECLBack': u'segmentation in R of backward',
    'IZECLBack': u'segmentation in Z of backward',
    'IPhiECLBack': u'segmentation in Phi of backward',
    'IRECLFor': u'segmentation in R of forward',
    'IZECLFor': u'segmentation in Z of forward',
    'IPhiECLFor': u'segmentation in Phi of forward',
    'density': u'Density'}

# The segmentation in R, Z and Phi. And thickness, density.
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
    'IRECLBack': 3,
    'IZECLBack': 5,
    'IPhiECLBack': 144,
    'IRECLFor': 3,
    'IZECLFor': 5,
    'IPhiECLFor': 144,
    'density': density_list}

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
    'IRECLBack',
    'IZECLBack',
    'IPhiECLBack',
    'IRECLFor',
    'IZECLFor',
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
