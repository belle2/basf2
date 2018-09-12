#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
import os

logfile = os.path.dirname(os.path.abspath(sys.argv[0])) + '/decparse.log'
enable_colours = True
overwrite = True
dkfilespath = '../dec/'
obsoletepath = '../doc/'
partdictpath = os.path.dirname(os.path.abspath(sys.argv[0])) \
    + '/particle_dictionary'

use_url = False
dec_url = \
    'https://stash.desy.de/projects/B2/repos/software/decfiles/dec/'
obs_url = \
    'https://stash.desy.de/projects/B2/repos/software/raw/decfiles/doc/table_obsolete.sql'
cuts_url = \
    'https://stash.desy.de/projects/B2/repos/software/decfiles/doc/cuts'
table_url = 'http://svn.cern.ch/guest/evtgen/tags/R01-03-00/evt.pdl'

groups = [
    'SL',
    'EWP',
    'TCPV',
    'HAD',
    'CHARM',
    'ONIA',
    'TAU',
    ]

terminators = [
    'VSS',
    'VSS_BMIX',
    'PHOTOS',
    'VLL',
    'VVPIPI',
    'PARTWAVE',
    'HELAMP',
    'PYTHIA',
    'HQET',
    'HQET2',
    'ISGW2',
    'GOITY_ROBERTS',
    'VUB',
    'PHSP',
    'SVS',
    'SVV_HELAMP',
    'BTOXSGAMMA',
    'BTOSLLBALL',
    'BTOXSLL',
    'BTO3PI_CP',
    'STS',
    'SLN',
    'CB3PI-P00',
    'CB3PI-MPP',
    'VSP_PWAVE',
    'SVP_HELAMP',
    'BTOSLLALI',
    'TAULNUNU',
    'TAUSCALARNU',
    'TAUHADNU',
    'TAUVECTORNU',
    'D_DALITZ',
    'VVS_PWAVE',
    'TVS_PWAVE',
    'TSS',
    'PI0_DALITZ',
    'ETA_DALITZ',
    'OMEGA_DALITZ',
    'VVP',
    'BTOSLLMS',
    ]

longlived = [
    'pi+',
    'pi-',
    'K+',
    'K-',
    'K_L0',
    'p+',
    'anti-p-',
    'n0',
    'anti-n0',
    'e+',
    'e-',
    'mu+',
    'mu-',
    ]

nickslation = {'B0': 'Bd', 'B+': 'Bu', 'D*+': 'Dst'}

# Obsolete
descripslation = {'D*+': 'D*(2010)+'}
# "K_S0" : "KS0"
