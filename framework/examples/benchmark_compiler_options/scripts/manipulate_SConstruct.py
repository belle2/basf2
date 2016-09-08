#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
from basf2 import *
from ROOT import Belle2
sconstruct_path = Belle2.FileSystem.findFile('site_scons/SConstruct')
# sconstruct_path = '../site_scons/SConstruct'

fobj = open(sconstruct_path, 'r')

optionlist = ['    global_env.Append(CCFLAGS = [\'-Wextra\'])\n',
              '    global_env.Append(CCFLAGS = [\'-Wextra\',\'-O3\'])\n',
              '    global_env.Append(CCFLAGS = [\'-Wextra\',\'-O3\', \'-native\'])\n'
              ]
option = optionlist[int(sys.argv[1])]
a = False
output = []
input_data = []
for line in fobj:
    input_data.append(line)
for i in range(0, len(input_data)):
    if input_data[i].startswith('elif option == \'opt\':'):
        a = True
        print('line found')
    if a:
        if input_data[i].startswith('    global_env.Append(CCFLAGS'):
            output.append(option)
            a = False
            print('option changed')
        else:
            output.append(input_data[i])
    else:
        output.append(input_data[i])
fobj.close()
fobj = open(sconstruct_path, 'w')
for i in output:
    fobj.write(i)
