#!/usr/bin/env python3

################################################################
# A simple script to produce json files with list of input data
# files for the BeamSpot calibration.
# Each bucket for exp12 is taken separately.
################################################################


from glob import glob
import re

# Produce json file for airflow-based mumu-skims (exp12)


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

    fOut = open(name, 'w')

    fOut.write('{"hlt_mumu": [')

    for i, r in enumerate(res):
        n = getRun(r)
        fOut.write('["' + r + '", [12, '+str(n)+']]')
        if i != len(res) - 1:
            fOut.write(', ')

    fOut.write(']}')
    fOut.close()


# Produce json file for bucket12 or exp7,exp8 or exp10
def storeSteering2(name, tag):
    baseDir = '/group/belle2/dataprod/Data/OfficialReco/proc11/e00'+tag+'/4S/r*/skim/hlt_mumu_2trk/mdst'
    if tag == 'bucket12':
        baseDir = '/group/belle2/dataprod/Data/release-04-02-04/DB00001113/' +\
                  'bucket12/e0012/4S_offres/GoodRuns/r*/skim/hlt_mumu_2trk/mdst'
    elif tag == '08off':
        baseDir = '/group/belle2/dataprod/Data/OfficialReco/proc11/e0008/4S_offres/r*/skim/hlt_mumu_2trk/mdst'
        tag = '08'
    elif tag == '08scan':
        baseDir = '/group/belle2/dataprod/Data/OfficialReco/proc11/e0008/4S_scan/r*/skim/hlt_mumu_2trk/mdst'
        tag = '08'

    res = glob(baseDir)

    def getRun(r): return int(re.findall('/r[0-9]*/', r)[0][2:-1])

    res = sorted(res, key=getRun)

    fOut = open(name, 'w')

    fOut.write('{"hlt_mumu": [')

    for i, r in enumerate(res):
        n = getRun(r)
        exp = 12 if tag == 'bucket12' else int(tag)
        fOut.write('["' + r + '", ['+str(exp)+', '+str(n)+']]')
        if i != len(res) - 1:
            fOut.write(', ')

    fOut.write(']}')
    fOut.close()


# Produce all the json steerings to the steering directory

storeSteering2('steerings/input_data_exp8off.json',  '08off')
storeSteering2('steerings/input_data_exp8scan.json',  '08scan')

storeSteering2('steerings/input_data_exp7.json',  '07')
storeSteering2('steerings/input_data_exp8.json',  '08')
storeSteering2('steerings/input_data_exp10.json', '10')

storeSteering('steerings/input_data_bucket9.json', 71)
storeSteering('steerings/input_data_bucket10.json', 91)
storeSteering('steerings/input_data_bucket11.json', [110, 112])
storeSteering2('steerings/input_data_bucket12.json', 'bucket12')
storeSteering('steerings/input_data_bucket13.json', 139)
storeSteering('steerings/input_data_bucket14.json', 166)
storeSteering('steerings/input_data_bucket15.json', 192)
