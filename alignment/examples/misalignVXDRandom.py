#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Generate alignment payload for VXD

"""

import basf2 as b2

from alignment.utils import AlignmentGeneratorConfig

alignment = AlignmentGeneratorConfig()
# 100 um normal distribution for all shifts
alignment.gen_u_gaus('0.0.0', 0.01)
alignment.gen_v_gaus('0.0.0', 0.01)
alignment.gen_w_gaus('0.0.0', 0.01)
# -1 mrad to +1 mrad uniform distribution for all angles
alignment.gen_alpha_uniform('0.0.0', 0.001)
alignment.gen_beta_uniform('0.0.0', 0.001)
alignment.gen_gamma_uniform('0.0.0', 0.001)

alignment.set_all('1.1.1', 0.0)

main = b2.create_path()
main.add_module('EventInfoSetter')
main.add_module('Gearbox')
main.add_module('Geometry', components=['PXD', 'SVD'])
main.add_module('AlignmentGenerator', payloadIov=[0, 0, -1, -1], payloadName="", data=alignment.get_data(), createPayload=True)
main.add_module('Progress')

b2.process(main)
print(b2.statistics)
