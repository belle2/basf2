#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import basf2 as b2
b2.logging.log_level = b2.LogLevel.WARNING

# -------------------------
# here we register modules
# -------------------------

progress = b2.register_module('Progress')
paramloader = b2.register_module('Gearbox')
geometry = b2.register_module('Geometry')
geometry.param('components', ['TOP'])
input = b2.register_module('RootInput')
topdigi = b2.register_module('TOPDigitizer')
topback = b2.register_module('TOPBackground')

param_back = {'TimeOfSimulation': 100.0, 'Type': 'RBB_LER',
              'Output': 'RBB_LER.root'}
# TimeOfSimulation in us
# Type the backgound type just for labeing

topback.param(param_back)
# --------------------------------------
# here we set the parameters of modules
# --------------------------------------

input.param('inputFileNames', ['/home/belle/tara/public/basf2/Work_MCgen/output/output_RBB_LER_study_0.root',
                               '/home/belle/tara/public/basf2/Work_MCgen/output/output_RBB_LER_study_1.root',
                               '/home/belle/tara/public/basf2/Work_MCgen/output/output_RBB_LER_study_2.root',
                               '/home/belle/tara/public/basf2/Work_MCgen/output/output_RBB_LER_study_3.root',
                               '/home/belle/tara/public/basf2/Work_MCgen/output/output_RBB_LER_study_4.root',
                               '/home/belle/tara/public/basf2/Work_MCgen/output/output_RBB_LER_study_5.root',
                               '/home/belle/tara/public/basf2/Work_MCgen/output/output_RBB_LER_study_6.root',
                               '/home/belle/tara/public/basf2/Work_MCgen/output/output_RBB_LER_study_7.root',
                               '/home/belle/tara/public/basf2/Work_MCgen/output/output_RBB_LER_study_8.root',
                               '/home/belle/tara/public/basf2/Work_MCgen/output/output_RBB_LER_study_9.root',
                               '/home/belle/tara/public/basf2/Work_MCgen/output/output_RBB_LER_study_10.root',
                               '/home/belle/tara/public/basf2/Work_MCgen/output/output_RBB_LER_study_11.root',
                               '/home/belle/tara/public/basf2/Work_MCgen/output/output_RBB_LER_study_12.root',
                               '/home/belle/tara/public/basf2/Work_MCgen/output/output_RBB_LER_study_13.root',
                               '/home/belle/tara/public/basf2/Work_MCgen/output/output_RBB_LER_study_14.root',
                               '/home/belle/tara/public/basf2/Work_MCgen/output/output_RBB_LER_study_15.root',
                               '/home/belle/tara/public/basf2/Work_MCgen/output/output_RBB_LER_study_16.root',
                               '/home/belle/tara/public/basf2/Work_MCgen/output/output_RBB_LER_study_17.root',
                               '/home/belle/tara/public/basf2/Work_MCgen/output/output_RBB_LER_study_18.root',
                               '/home/belle/tara/public/basf2/Work_MCgen/output/output_RBB_LER_study_19.root',
                               '/home/belle/tara/public/basf2/Work_MCgen/output/output_RBB_LER_study_20.root',
                               '/home/belle/tara/public/basf2/Work_MCgen/output/output_RBB_LER_study_21.root',
                               '/home/belle/tara/public/basf2/Work_MCgen/output/output_RBB_LER_study_22.root',
                               '/home/belle/tara/public/basf2/Work_MCgen/output/output_RBB_LER_study_23.root',
                               '/home/belle/tara/public/basf2/Work_MCgen/output/output_RBB_LER_study_24.root',
                               '/home/belle/tara/public/basf2/Work_MCgen/output/output_RBB_LER_study_25.root',
                               '/home/belle/tara/public/basf2/Work_MCgen/output/output_RBB_LER_study_26.root',
                               '/home/belle/tara/public/basf2/Work_MCgen/output/output_RBB_LER_study_27.root',
                               '/home/belle/tara/public/basf2/Work_MCgen/output/output_RBB_LER_study_28.root',
                               '/home/belle/tara/public/basf2/Work_MCgen/output/output_RBB_LER_study_29.root',
                               '/home/belle/tara/public/basf2/Work_MCgen/output/output_RBB_LER_study_30.root',
                               '/home/belle/tara/public/basf2/Work_MCgen/output/output_RBB_LER_study_31.root',
                               '/home/belle/tara/public/basf2/Work_MCgen/output/output_RBB_LER_study_32.root',
                               '/home/belle/tara/public/basf2/Work_MCgen/output/output_RBB_LER_study_33.root',
                               '/home/belle/tara/public/basf2/Work_MCgen/output/output_RBB_LER_study_34.root',
                               '/home/belle/tara/public/basf2/Work_MCgen/output/output_RBB_LER_study_35.root',
                               '/home/belle/tara/public/basf2/Work_MCgen/output/output_RBB_LER_study_36.root',
                               '/home/belle/tara/public/basf2/Work_MCgen/output/output_RBB_LER_study_37.root',
                               '/home/belle/tara/public/basf2/Work_MCgen/output/output_RBB_LER_study_38.root',
                               '/home/belle/tara/public/basf2/Work_MCgen/output/output_RBB_LER_study_39.root',
                               '/home/belle/tara/public/basf2/Work_MCgen/output/output_RBB_LER_study_40.root',
                               '/home/belle/tara/public/basf2/Work_MCgen/output/output_RBB_LER_study_41.root',
                               '/home/belle/tara/public/basf2/Work_MCgen/output/output_RBB_LER_study_42.root',
                               '/home/belle/tara/public/basf2/Work_MCgen/output/output_RBB_LER_study_43.root',
                               '/home/belle/tara/public/basf2/Work_MCgen/output/output_RBB_LER_study_44.root',
                               '/home/belle/tara/public/basf2/Work_MCgen/output/output_RBB_LER_study_45.root',
                               '/home/belle/tara/public/basf2/Work_MCgen/output/output_RBB_LER_study_46.root',
                               '/home/belle/tara/public/basf2/Work_MCgen/output/output_RBB_LER_study_47.root',
                               '/home/belle/tara/public/basf2/Work_MCgen/output/output_RBB_LER_study_48.root',
                               '/home/belle/tara/public/basf2/Work_MCgen/output/output_RBB_LER_study_49.root',
                               '/home/belle/tara/public/basf2/Work_MCgen/output/output_RBB_LER_study_50.root',
                               '/home/belle/tara/public/basf2/Work_MCgen/output/output_RBB_LER_study_51.root',
                               '/home/belle/tara/public/basf2/Work_MCgen/output/output_RBB_LER_study_52.root',
                               '/home/belle/tara/public/basf2/Work_MCgen/output/output_RBB_LER_study_53.root',
                               '/home/belle/tara/public/basf2/Work_MCgen/output/output_RBB_LER_study_54.root',
                               '/home/belle/tara/public/basf2/Work_MCgen/output/output_RBB_LER_study_55.root',
                               '/home/belle/tara/public/basf2/Work_MCgen/output/output_RBB_LER_study_56.root',
                               '/home/belle/tara/public/basf2/Work_MCgen/output/output_RBB_LER_study_57.root',
                               '/home/belle/tara/public/basf2/Work_MCgen/output/output_RBB_LER_study_58.root',
                               '/home/belle/tara/public/basf2/Work_MCgen/output/output_RBB_LER_study_59.root',
                               '/home/belle/tara/public/basf2/Work_MCgen/output/output_RBB_LER_study_60.root',
                               '/home/belle/tara/public/basf2/Work_MCgen/output/output_RBB_LER_study_61.root',
                               '/home/belle/tara/public/basf2/Work_MCgen/output/output_RBB_LER_study_62.root',
                               '/home/belle/tara/public/basf2/Work_MCgen/output/output_RBB_LER_study_63.root',
                               '/home/belle/tara/public/basf2/Work_MCgen/output/output_RBB_LER_study_64.root',
                               '/home/belle/tara/public/basf2/Work_MCgen/output/output_RBB_LER_study_65.root',
                               '/home/belle/tara/public/basf2/Work_MCgen/output/output_RBB_LER_study_66.root',
                               '/home/belle/tara/public/basf2/Work_MCgen/output/output_RBB_LER_study_67.root',
                               '/home/belle/tara/public/basf2/Work_MCgen/output/output_RBB_LER_study_68.root',
                               '/home/belle/tara/public/basf2/Work_MCgen/output/output_RBB_LER_study_69.root',
                               '/home/belle/tara/public/basf2/Work_MCgen/output/output_RBB_LER_study_70.root',
                               '/home/belle/tara/public/basf2/Work_MCgen/output/output_RBB_LER_study_71.root',
                               '/home/belle/tara/public/basf2/Work_MCgen/output/output_RBB_LER_study_72.root',
                               '/home/belle/tara/public/basf2/Work_MCgen/output/output_RBB_LER_study_73.root',
                               '/home/belle/tara/public/basf2/Work_MCgen/output/output_RBB_LER_study_74.root',
                               '/home/belle/tara/public/basf2/Work_MCgen/output/output_RBB_LER_study_75.root',
                               '/home/belle/tara/public/basf2/Work_MCgen/output/output_RBB_LER_study_76.root',
                               '/home/belle/tara/public/basf2/Work_MCgen/output/output_RBB_LER_study_77.root',
                               '/home/belle/tara/public/basf2/Work_MCgen/output/output_RBB_LER_study_78.root',
                               '/home/belle/tara/public/basf2/Work_MCgen/output/output_RBB_LER_study_79.root',
                               '/home/belle/tara/public/basf2/Work_MCgen/output/output_RBB_LER_study_80.root',
                               '/home/belle/tara/public/basf2/Work_MCgen/output/output_RBB_LER_study_81.root',
                               '/home/belle/tara/public/basf2/Work_MCgen/output/output_RBB_LER_study_82.root',
                               '/home/belle/tara/public/basf2/Work_MCgen/output/output_RBB_LER_study_83.root',
                               '/home/belle/tara/public/basf2/Work_MCgen/output/output_RBB_LER_study_84.root',
                               '/home/belle/tara/public/basf2/Work_MCgen/output/output_RBB_LER_study_85.root',
                               '/home/belle/tara/public/basf2/Work_MCgen/output/output_RBB_LER_study_86.root',
                               '/home/belle/tara/public/basf2/Work_MCgen/output/output_RBB_LER_study_87.root',
                               '/home/belle/tara/public/basf2/Work_MCgen/output/output_RBB_LER_study_88.root',
                               '/home/belle/tara/public/basf2/Work_MCgen/output/output_RBB_LER_study_89.root',
                               '/home/belle/tara/public/basf2/Work_MCgen/output/output_RBB_LER_study_90.root',
                               '/home/belle/tara/public/basf2/Work_MCgen/output/output_RBB_LER_study_91.root',
                               '/home/belle/tara/public/basf2/Work_MCgen/output/output_RBB_LER_study_92.root',
                               '/home/belle/tara/public/basf2/Work_MCgen/output/output_RBB_LER_study_93.root',
                               '/home/belle/tara/public/basf2/Work_MCgen/output/output_RBB_LER_study_94.root',
                               '/home/belle/tara/public/basf2/Work_MCgen/output/output_RBB_LER_study_95.root',
                               '/home/belle/tara/public/basf2/Work_MCgen/output/output_RBB_LER_study_96.root',
                               '/home/belle/tara/public/basf2/Work_MCgen/output/output_RBB_LER_study_97.root',
                               '/home/belle/tara/public/basf2/Work_MCgen/output/output_RBB_LER_study_98.root',
                               '/home/belle/tara/public/basf2/Work_MCgen/output/output_RBB_LER_study_99.root'])

# create path
main = b2.create_path()

# add modules to path

main.add_module(input)
main.add_module(progress)
main.add_module(paramloader)
main.add_module(geometry)
main.add_module(topdigi)
main.add_module(topback)

b2.process(main)

print(b2.statistics)
