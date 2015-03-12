Import('env')

if (FindFile('ARICHGeometryPar.cc', '../../../arich/geometry/src/')) is not "None":
   env.Append(CPPDEFINES=['DOARICH'])
   env['LIBS'] = ['ecl', 'framework', 'arich', 'ecl_dataobjects', 'simulation_dataobjects', 'mdst_dataobjects',
                  'Geom', '$ROOT_LIBS']
else:
   env['LIBS'] = ['ecl', 'framework', 'ecl_dataobjects', 'simulation_dataobjects', 'mdst_dataobjects',
                  'Geom', '$ROOT_LIBS']

Return('env')
