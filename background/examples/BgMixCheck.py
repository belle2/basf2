#!/usr/bin/env python
# -*- coding: utf-8 -*-

##############################################################################
# Example script - 2013 Belle II Collaboration
##############################################################################
import sys
from basf2 import *
import ROOT
from ROOT import Belle2
from ROOT import gSystem
from array import array
import subprocess

# load some libraries for ROOT to understand things;
gSystem.Load('libdataobjects')
gSystem.Load('libvxd')

subdetectorNames = {
    1: 'PXD',
    2: 'SVD',
    3: 'CDC',
    4: 'TOP',
    5: 'ARICH',
    6: 'ECL',
    7: 'EKLM',
    8: 'BKLM',
    9: 'ECLsim',
    }

# This script:
# 1. Gets the detector/component/generator data from ROF files.
# 2. Gets lists of simhit counts for individual ROFs
# 3. Runs a script using background mixer to generate some events
# 4. Get lists of simhit counts for individual simulated events
# 5. Compares and reports confclicts
#
# Usage:
# chmod a+x BgMixCheck.py
# ./background/examples/BgMixCheck.py <list-of-rof-files>
# or
# python background/examples/BgMixCheck.py <list-of-rof-files>
# Wildcards can be used to specify the path and names of rof files,
# such as /data/belle2/BG/rof/rof*.root

# Background codes.
bg_codes = dict(
    bg_none=0,
    bg_Coulomb_LER=1,
    bg_Coulomb_HER=2,
    bg_RBB_LER=3,
    bg_RBB_HER=4,
    bg_Touschek_LER=5,
    bg_Touschek_HER=6,
    bg_twoPhoton=7,
    bg_other=99,
    )


def encodeBg(component, generator):
    '''
    Set the background tag for SimHits.
    Sets bg tag according to a (component, generator) pair.
    Sets bg tag to bg_other if it can't decode the input.
    Never sets bg tag to bg_none.
    '''

    tag = bg_codes['bg_none']
    isHER = 'HER' in generator.upper()
    if 'COULOMB' in component.upper():
        if isHER:
            tag = bg_codes['bg_Coulomb_HER']
        else:
            tag = bg_codes['bg_Coulomb_LER']
    elif 'RBB' in component.upper():
        if isHER:
            tag = bg_codes['bg_RBB_HER']
        else:
            tag = bg_codes['bg_RBB_LER']
    elif 'TOUSCHEK' in component.upper():
        if isHER:
            tag = bg_codes['bg_Touschek_HER']
        else:
            tag = bg_codes['bg_Touschek_LER']
    elif 'PHOTON' in component.upper():
        tag = bg_codes['bg_twoPhoton']
    else:
        tag = bg_codes['bg_other']
    return tag


def decodeBg(tag):
    '''
    Return a (component,generator) pair for a bgTag.
    Returns something for all background codes.
    '''

    try:
        if tag == bg_codes['bg_none']:
            return ('none', 'none')
        elif tag == bg_codes['bg_other']:
            return ('unknown', 'unknown')
        elif tag == bg_codes['bg_twoPhoton']:
            return ('TwoPhoton', 'BDK')
        elif tag == bg_codes['bg_Touschek_LER']:
            return ('Touschek', 'LER')
        elif tag == bg_codes['bg_Touschek_HER']:
            return ('Touschek', 'HER')
        elif tag == bg_codes['bg_RBB_LER']:
            return ('RBB', 'LER')
        elif tag == bg_codes['bg_RBB_HER']:
            return ('RBB', 'HER')
        elif tag == bg_codes['bg_Coulomb_LER']:
            return ('Coulomb', 'LER')
        elif tag == bg_codes['bg_Coulomb_HER']:
            return ('Coulomb', 'HER')
        else:
            raise Exception(tag)
    except Exception, badBgTag:
        print 'Wrong background tag encountered: ' + badBgTag
        return ('unknown', 'unknown')


# List of files is expected on the command-line.
# A wildcard expression is OK, it will be globbed by Python
# before passing.
n_files = len(sys.argv)
backgrounds = dict()
# Ipen the files one by one and get the identification data and
# numbers of SimHits per each ROF
# Also check the number of complete ROFs that we can use.
nFrames = 50  # don't actually need more.
for file_name in sys.argv[1:n_files]:
    # Get ROF file ID data
    f = ROOT.TFile(file_name, 'READ')
    ctree = f.Get('ContentTree')
    detector = ''
    component = ''
    generator = ''
    for record in ctree:
        detector = subdetectorNames[record.Subdetector]
        component = str(record.Component)
        # generator may be whatever, but hopefully contains HER/LER
        if str(record.Generator).upper().find('LER') > -1:
            generator = 'LER'
        else:
            generator = 'HER'
    print file_name + ':' + '\tDetector: ' + str(detector) + '\tComponent: ' \
        + str(component) + '\tGenerator: ' + str(generator)
    # Add this beauty to the background structure
    if detector in backgrounds.keys():
        if component in backgrounds[detector].keys():
            backgrounds[detector][component][generator] = {'ROF': array('L'),
                    'MIX': array('L')}
        else:
            backgrounds[detector][component] = {generator: {'ROF': array('L'),
                    'MIX': array('L')}}
    else:
        backgrounds[detector] = {component: {generator: {'ROF': array('L'),
                                 'MIX': array('L')}}}
    rtree = f.Get('ROFTree')
    nFrames = min(nFrames, rtree.GetEntries())
    for record in rtree:

        backgrounds[detector][component][generator]['ROF'
                ].append(record.ReadoutFrames.GetEntries())
    f.Close()

print 'Background data structure:'
for (detector, components) in backgrounds.iteritems():
    print detector
    for (component, generators) in components.iteritems():
        print '    ' + component
        for (generator, simhitArrays) in generators.iteritems():
            print '        ' + generator
            for (arrayname, data) in simhitArrays.iteritems():
                print '            ' + arrayname + ': ' + str(len(data))

# Now run the basf2 script to produce root output.
subprocess.call(['basf2', 'background/examples/OnlyBackground.py']
                + [str(nFrames)] + sys.argv[1:n_files])
# ... and analyze the output file
outf = ROOT.TFile('OnlyBackgroundOutput.root', 'READ')
tree = outf.Get('tree')
# We have to explicitly initialize the bgmixer simhitArrays
n_entries = tree.GetEntries()
for (detector, components) in backgrounds.iteritems():
    for (component, generators) in components.iteritems():
        for (generator, simhitArrays) in generators.iteritems():
            simhitArrays['MIX'] = array('L', n_entries * [0])

# We need to refer to SimHit simhitArrays by names, so we need
# a more laborious approach.
# Initialize and activate the SimHit branches:
reader = dict([])
for detector in backgrounds.keys():
    simhitName = detector.upper() + 'SimHit'
    if detector == 'ECL':
        simhitName = 'ECLHit'
    elif detector == 'ECLsim':
        simhitName = 'ECLSimHit'
    simhitArray = ROOT.TClonesArray('Belle2::' + simhitName, 20000, True)
    tree.SetBranchAddress(simhitName + 's', simhitArray)
    simhitBranch = tree.GetBranch(simhitName + 's')
    reader[detector] = (simhitArray, simhitBranch)

for i_record in range(n_entries):
    for detector in backgrounds.keys():
        (simhitArray, simhitBranch) = reader[detector]
        simhitBranch.GetEntry(i_record)
        for simhit in simhitArray:
            tag = simhit.getBackgroundTag()
            if tag == bg_codes['bg_none'] or tag == bg_codes['bg_other'] \
                or tag == bg_codes['bg_twoPhoton']:
                print 'ERROR: background tag {tag} should not appear here.'.format(tag=tag)
            else:
                (component, generator) = decodeBg(tag)

                backgrounds[detector][component][generator]['MIX'
                        ][i_record] += 1
f.Close()
# Now print a nice summary of the results
for (detector, components) in backgrounds.iteritems():
    print detector
    for (component, generators) in components.iteritems():
        print '    ' + component
        for (generator, arrays) in generators.iteritems():
            print '        ' + generator
            sum = 0
            diffs = 0
            for i in range(len(arrays['MIX'])):
                sum += arrays['ROF'][i]
                if arrays['MIX'][i] != arrays['ROF'][i]:
                    diffs += 1
                    print '                {i} {mix} {rof}'.format(i=i,
                            mix=arrays['MIX'][i], rof=arrays['ROF'][i])
            print '            {d} differences in {sum} simhits.'.format(d=diffs,
                    sum=sum)
