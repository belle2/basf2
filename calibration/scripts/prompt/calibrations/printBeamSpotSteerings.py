#!/usr/bin/env python3

from glob import glob
import re


def storeSteering(name, tag):

    baseDir = '/group/belle2/dataprod/dp_managers/ddossett/airflow/output_data'

    def getRun(x): return int(re.findall('[0-9]*$', x)[0])

    res = None
    if not isinstance(tag, list):
        tag = str(tag)
        res = glob(baseDir+'/'+tag+'/mumu_2trk/e12/*')
    else:
        res = []
        for t in tag:
            t = str(t)
            res += glob(baseDir+'/'+t+'/mumu_2trk/e12/*')

    res = sorted(res, key=getRun)

#   print(res)
#   import sys
#   sys.exit(0)

    fOut = open(name, 'w')

    fOut.write('{"hlt_mumu": [')  # , end ="")

    for r in res:
        n = getRun
        fOut.write('["' + r + '", [12, '+str(n)+']], ')  # , end =" ")

    fOut.write(']}')  # , end ="")
    fOut.close()
    # print('')
# print(res)


def storeSteering2(name, tag):
    baseDir = '/group/belle2/dataprod/Data/OfficialReco/proc11/e00'+tag+'/4S/GoodRuns/r*/skim/hlt_mumu_2trk/mdst'
    if tag == 'bucket12':
        baseDir = '/group/belle2/dataprod/Data/release-04-02-04/DB00001113/' +\
                  'bucket12/e0012/4S_offres/GoodRuns/r*/skim/hlt_mumu_2trk/mdst'

    res = glob(baseDir)
    # print(glob(baseDir))

    def getRun(r): return int(re.findall('/r[0-9]*/', r)[0][2:-1])

    res = sorted(res, key=getRun)

    fOut = open(name, 'w')

    fOut.write('{"hlt_mumu": [')  # , end ="")

    for r in res:
        n = getRun(r)
        fOut.write('["' + r + '", [12, '+str(n)+']], ')  # , end =" ")

    fOut.write(']}')  # , end ="")
    fOut.close()


storeSteering2('steerings/exp7.json',  '07')
storeSteering2('steerings/exp8.json',  '08')
storeSteering2('steerings/exp10.json', '10')

storeSteering('steerings/input_data_bucket9.json', 71)
storeSteering('steerings/input_data_bucket10.json', 91)
storeSteering('steerings/input_data_bucket11.json', [110, 112])
storeSteering2('steerings/input_data_bucket12.json', 'bucket12')
storeSteering('steerings/input_data_bucket13.json', 139)
storeSteering('steerings/input_data_bucket14.json', 166)
storeSteering('steerings/input_data_bucket15.json', 192)
