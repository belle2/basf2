##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################
# This builds an input to the Misalignment module.

import random
from lxml import etree
from lxml.etree import Element
from lxml.etree import SubElement

svd_layers = [3, 4, 5, 6]
svd_nsensors = {3: 2, 4: 3, 5: 4, 6: 5}
svd_nladders = {3: 7, 4: 10, 5: 12, 6: 16}

misalignment_shifts = ['du', 'dv', 'dw']
sigma_shifts = 2.0  # microns
misalignment_rotations = ['dalpha', 'dbeta', 'dgamma']
sigma_rotations = 3.0  # mrad

# <SVDMisalignment/>
svd_misalignment = Element('SVD', name='Random misalignment')


# <SVDMisalignment><layer/>
for i_layer in svd_layers:
    layer_id = str(i_layer) + '.0.0'
    layer = SubElement(svd_misalignment, 'layer', id=layer_id)
    # <SVDMisalignment><layer><ladder/>
    for i_ladder in range(svd_nladders[i_layer]):
        ladder_id = str(i_layer) + '.' + str(i_ladder + 1) + '.0'
        ladder = SubElement(layer, 'ladder', id=ladder_id)
        # <SVDMisalignment><layer><ladder><sensor/>
        for i_sensor in range(svd_nsensors[i_layer]):
            sensor_id = str(i_layer) + '.' + str(i_ladder + 1) + '.' + str(i_sensor + 1)
            sensor = SubElement(ladder, 'sensor', id=sensor_id)
            # Add misalignment at this level
            for param in misalignment_shifts:
                dx = random.normalvariate(0.0, sigma_shifts)
                p = SubElement(sensor, param, unit='um', sigma=str(sigma_shifts))
                p.text = str(dx)
            for param in misalignment_rotations:
                dx = random.normalvariate(0.0, sigma_rotations)
                p = SubElement(sensor, param, unit='mrad', sigma=str(sigma_rotations))
                p.text = str(dx)

tree = etree.ElementTree(svd_misalignment)

tree.write('SVDMisalignment.xml', pretty_print=True, xml_declaration=True, encoding='utf-8', method='xml')
