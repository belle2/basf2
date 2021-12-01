# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
Validation of the Invariant Collision Energy calibration
"""


import basf2
from prompt import ValidationSettings
from ROOT.Belle2 import Unit

import ROOT
import sys
import subprocess
import json

import numpy as np
import scipy.linalg as la
import scipy.stats
import matplotlib.pyplot as plt
from matplotlib import rcParams

import re
import os
from glob import glob
from math import sqrt, frexp, asin, hypot, atan2

from datetime import datetime, timedelta

from collections import OrderedDict
from itertools import groupby
from shutil import copyfile
from copy import deepcopy


#: Tells the automated system some details of this script
settings = ValidationSettings(name='eCMS Calibrations',
                              description=__doc__,
                              download_files=['stdout'],
                              expert_config={})


def get_Ecms_values(path):
    """
    Load the values of the Ecms properties from the payloads into the list
    """

    runDict = {}
    with open(path + '/database.txt') as fDB:
        for ll in fDB:
            ll = ll.strip()
            ll = ll.split(' ')

            # deriving file name
            fN = ll[0].replace('/', '_') + '_rev_' + ll[1] + '.root'

            r = ll[2].split(',')
            runDict[fN] = ((int(r[0]), int(r[1])), (int(r[2]), int(r[3])))

    arr = []

    fList = glob(path + '/*.root')
    for fName in fList:
        bName = os.path.basename(fName)
        runExp = runDict[bName]

        f = ROOT.TFile.Open(fName)
        ecmsObj = f.Get("CollisionInvariantMass")
        assert(ecmsObj.ClassName() == "Belle2::CollisionInvariantMass")

        eCMS = ecmsObj.getMass()
        eCMSe = ecmsObj.getMassError()
        eCMSs = ecmsObj.getMassSpread()

        arr.append((runExp, (eCMS, eCMSe, eCMSs)))

        f.Close()

    arr = sorted(arr, key=lambda x: x[0])

    return arr


def printToFile(arr):
    """
    print the Ecms parameters from arr to the text file
    """

    with open('ecmsData.csv', 'w') as outFile:
        outFile.write('exp1 run1 exp2 run2  tStart tEnd  nInt idInt  '
                      'ecms ecmsUnc  ecmsSpread ecmsSpreadUnc  shift shiftUnc  pull\n')

        for e in arr:
            outFile.write(
                # exp1  run1
                str(e[0][0][0]) + ' ' +
                str(e[0][0][1]) + ' ' +
                # exp2  run2
                str(e[0][1][0]) + ' ' +
                str(e[0][1][1]) + ' ' +
                # tStart  tEnd
                str(e[1]) + ' ' +
                str(e[2]) + ' ' +
                # nSlice, idSlice
                str(e[3]) + ' ' +
                str(e[4]) + ' ' +
                # ecms ecmsUnc
                str(1e3 * e[5][0]) + ' ' +
                str(1e3 * e[5][1]) + ' ' +

                # spread spreadUnc
                str(e[8][0]) + ' ' +
                str(e[8][1]) + ' ' +

                # shift shiftUnc
                str(e[7][0]) + ' ' +
                str(e[7][1]) + ' ' +

                # pull
                str(e[6]) + '\n')

        return arr


def plotVar(arr, limits, vName, getterV, getterE=None):
    """
    Create a plot with specified variable
    Parameters:
        arr     : array with data
        limits  : tuple with start and end of the plotted region
        vName   : name of the variable which is used in the plot label
        getterV : getter function to get plotted variable value from arr[i]
        getterE : getter function to get plotted variable uncertainty from arr[i]
    """

    tVals = []
    Vals = []
    Errs = []

    tGapVals = []
    GapVals = []

    for i, el in enumerate(arr):
        s, e = el[1], el[2]
        s = datetime.utcfromtimestamp((s + 9) * 3600)  # Convert to the JST (+9 hours)
        e = datetime.utcfromtimestamp((e + 9) * 3600)
        tVals.append(s)
        tVals.append(e)
        Vals.append(getterV(el))
        Vals.append(getterV(el))

        if getterE is not None:
            Errs.append(getterE(el))
            Errs.append(getterE(el))

        # Add breaks for longer gap if not the last interval
        if i >= len(arr) - 1:
            continue

        dt = (arr[i + 1][1] - arr[i][2]) * 3600

        # only consider gaps longer than 10 mins
        if dt < 10 * 60:
            continue

        # start-time of gap, end-time of gap
        gS = datetime.utcfromtimestamp((arr[i][2] + 9) * 3600)  # Convert to the JST (+9 hours)
        gE = datetime.utcfromtimestamp((arr[i + 1][1] + 9) * 3600)

        tVals.append(gS + (gE - gS) / 2)
        Vals.append(np.nan)
        if getterE is not None:
            Errs.append(np.nan)

        # store curve connecting gaps
        tGapVals.append(gS - timedelta(seconds=1))
        tGapVals.append(gS)
        tGapVals.append(gE)
        tGapVals.append(gE + timedelta(seconds=1))

        GapVals.append(np.nan)
        GapVals.append(getterV(arr[i]))
        GapVals.append(getterV(arr[i + 1]))
        GapVals.append(np.nan)

    plt.plot(tVals, Vals, linewidth=2, color='C0')
    plt.plot(tGapVals, GapVals, linewidth=2, color='C0', alpha=0.35)

    Vals = np.array(Vals)
    Errs = np.array(Errs)

    if getterE is not None:

        Vals1 = deepcopy(Vals)
        Vals2 = deepcopy(Vals)

        Type = 1
        for i, el in enumerate(arr):

            if el[4] == 0:
                Type *= -1

            if Type == 1:
                Vals1[3 * i] = np.nan
                Vals1[3 * i + 1] = np.nan
            else:
                Vals2[3 * i] = np.nan
                Vals2[3 * i + 1] = np.nan

        if vName != 'pull':
            plt.fill_between(tVals, Vals1 - Errs, Vals1 + Errs, alpha=0.2)
            plt.fill_between(tVals, Vals2 - Errs, Vals2 + Errs, alpha=0.2)
        else:
            plt.fill_between(tVals, 0 * Vals1 - Errs, 0 * Vals1 + Errs, alpha=0.2)
            plt.fill_between(tVals, 0 * Vals2 - Errs, 0 * Vals2 + Errs, alpha=0.2)

    plt.xlabel('time')
    if vName != 'pull':
        plt.ylabel(vName + ' [MeV]')
    else:
        plt.ylabel(vName)

    if vName == 'pull':
        plt.ylim(-1.5, 1.5)
    elif vName == 'eCMSspread':
        plt.ylim(bottom=0)

    loc = 'plots/allData'
    if limits is not None:
        plt.xlim(datetime.strptime(limits[0], '%Y-%m-%d'), datetime.strptime(limits[1], '%Y-%m-%d'))
        loc = 'plots/' + limits[0] + 'to' + limits[1]

    # if not os.path.isdir(loc):
    os.makedirs(loc, exist_ok=True)

    plt.savefig(loc + '/' + vName + '.png')
    plt.clf()


def plotPullSpectrum(arr, vName, getterV, getterE):
    """
    Plot the pull spectrum for variable defined by functions getterV and gettterE,
    similarly as for plotVar
    """

    vals = np.array([k for k, g in groupby([getterV(v) for v in arr])])
    errs = np.array([k for k, g in groupby([getterE(v) for v in arr])])
    assert(len(vals) == len(errs))

    diffs = (vals[1:] - vals[:-1]) / np.hypot(errs[1:], errs[:-1])

    # Get 1sigma quantiles of the normal distribution
    n1, n2 = scipy.stats.norm.cdf([-1, 1])

    q1 = np.quantile(diffs, n1)
    q2 = np.quantile(diffs, n2)

    plt.hist(diffs, bins=np.linspace(-10, 10, 20), label=f'q1={round(q1,1)}, q2={round(q2,1)}')
    plt.xlabel(vName)
    plt.legend()

    loc = 'plots/allData'
    plt.savefig(loc + '/' + vName + '.png')
    plt.clf()


def get_times(job_path):
    """
    Read the start/end times of the calibration intervals from the di-muon events
    """

    times = []
    with open(job_path + '/eCMS/0/algorithm_output/mumuEcalib.txt') as fEcms:
        for ll in fEcms:
            ll = ll.strip()
            ll = ll.split(' ')
            times.append((float(ll[2]), float(ll[3]), int(ll[0]), int(ll[1])))
    return times


# Get the results from the combined calibration
def read_hadB_data(job_path):
    """
    It reads the calibration table from the text file produced by the CAF calibration.
    This text file includes the results from the final calibration.
    (combined or mumu-only)
    """

    arr = []
    with open(job_path + '/eCMS/0/algorithm_output/finalEcmsCalib.txt', "r") as text_file:
        for ll in text_file:
            ll = ll.strip().split(' ')
            arr.append(
                ((float(
                    ll[0]), float(
                    ll[1])), int(
                    ll[2]), (float(
                        ll[3]), float(
                        ll[5])), float(
                            ll[7]), (float(
                                ll[8]), float(
                                    ll[10])), (float(
                                        ll[12]), float(
                                            ll[14]))))
    return arr


# Create multi-page pdf file with the fit plots
def create_hadB_fit_plots(job_path):
    """
    Create multi-page pdf file with the fit plots for hadronic B decays.
    The file is created using pdflatex
    """

    cwd = os.getcwd()

    arr = read_hadB_data(job_path)

    os.chdir(job_path + '/eCMS/0/algorithm_output/')

    dName = 'plotsHadB'

    files = glob(dName + '/*.pdf')

    items = OrderedDict()

    for f in files:
        res = re.search(dName + '/B[p0]_([0-9]*)_([0-9]*)\\.pdf', f)
        t = int(res.group(1))
        i = int(res.group(2))
        if t not in items:
            items[t] = 1
        else:
            items[t] = max(items[t], i + 1)

    header = """\\documentclass[aspectratio=169]{beamer}
    \\usepackage{graphicx}

    \\begin{document}
    """

    body = ""
    for t in items:

        i0 = None
        for i0Temp in range(len(arr)):
            if int(round(arr[i0Temp][0][0], 0)) == t:
                i0 = i0Temp
                break
        assert(i0 is not None)

        frac = 1. / (items[t] + 0.2)
        body += '\\begin{frame}[t]\n'
        shift, shiftE = str(round(arr[i0][4][0], 1)), str(round(arr[i0][4][1], 1))
        spread, spreadE = str(round(arr[i0][5][0], 1)), str(round(arr[i0][5][1], 1))

        body += '$E_\\mathrm{shift}    = (' + shift + '\\pm' + shiftE + ')$~MeV \\hspace{2cm}   \n'
        body += '$E_\\mathrm{spread}   = (' + spread + '\\pm' + spreadE + ')$~MeV    \\\\ \\vspace{0.5cm}\n'
        for iShort in range(items[t]):
            i = i0 + iShort
            tStart, tEnd = arr[i][0][0], arr[i][0][1]
            eCMS, eCMSe = str(round(arr[i][2][0], 1)), str(round(arr[i][2][1], 1))
            pull = str(round(arr[i][3], 2))

            t1 = datetime.utcfromtimestamp((tStart + 9) * 3600).strftime('%y-%m-%d %H:%M')
            t2 = datetime.utcfromtimestamp((tEnd + 9) * 3600).strftime('%y-%m-%d %H:%M')

            body += '\\begin{minipage}{' + str(frac) + '\\textwidth}\n'
            body += '\\begin{center}\n'
            body += '\\scriptsize ' + t1 + ' \\\\\n'
            body += '\\scriptsize ' + t2 + ' \\\\\n'
            body += '\\scriptsize ' + str(round(tEnd - tStart, 1)) + ' hours \\\\ \\vspace{0.3cm}\n'
            body += '$(' + eCMS + '\\pm' + eCMSe + ')$~MeV    \\\\\n'
            body += '$\\mathrm{pull}    = ' + pull + '$    \\\\\n'
            body += '\\includegraphics[width=1.0\\textwidth]{' + dName + '/B0_' + str(t) + '_' + str(iShort) + '.pdf}\n'
            body += '\\includegraphics[width=1.0\\textwidth]{' + dName + '/Bp_' + str(t) + '_' + str(iShort) + '.pdf}\n'
            body += '\\end{center}\n'
            body += '\\end{minipage}\n'

        body += '\\end{frame}\n\n'

    tail = '\n\\end{document}'

    whole = header + body + tail

    os.makedirs('tmp', exist_ok=True)

    with open("tmp/hadBfits.tex", "w") as text_file:
        text_file.write(whole)

    return_code = subprocess.call(['pdflatex', 'tmp/hadBfits.tex'])

    os.makedirs(cwd + '/plots', exist_ok=True)
    copyfile('hadBfits.pdf', cwd + '/plots/hadBfits.pdf')

    os.chdir(cwd)


def run_validation(job_path, input_data_path, requested_iov, expert_config):
    """
    Create validation plots related to the Ecms calibration
    """

    # Expert config can contain the time ranges of the plots
    if expert_config != '':
        expert_config = json.loads(expert_config)

    allLimits = [None]
    if expert_config is not None and 'plotsRanges' in expert_config:
        allLimits += expert_config['plotsRanges']

    # Path to the database.txt file and to the payloads.
    dbFile = glob(f'{job_path}/**/database.txt', recursive=True)
    dbFile = [db for db in dbFile if 'algorithm_output' not in db]
    assert(len(dbFile) == 1)
    dbFile = dbFile[0]
    inputDir = dbFile[:dbFile.rfind('/')]

    times = get_times(job_path)
    arrNow = get_Ecms_values(inputDir)
    arrB = read_hadB_data(job_path)
    assert(len(times) == len(arrNow))
    assert(len(times) == len(arrB))

    arr = [a[:1] + t + a[1:] + ab[3:] for t, a, ab in zip(times, arrNow, arrB)]

    create_hadB_fit_plots(job_path)

    plt.figure(figsize=(18, 9))
    rcParams['axes.formatter.useoffset'] = False

    # plot the results
    for limits in allLimits:
        plotVar(arr, limits, 'eCMS', lambda e: 1e3 * e[5][0], lambda e: 1e3 * e[5][1])
        plotVar(arr, limits, 'eCMSshift', lambda e: e[7][0], lambda e: e[7][1])
        plotVar(arr, limits, 'eCMSspread', lambda e: e[8][0], lambda e: e[8][1])
        plotVar(arr, limits, 'pull', lambda e: e[6], lambda e: 1)

    # print results to the CSV files
    printToFile(arr)


if __name__ == "__main__":
    run_validation(*sys.argv[1:])
