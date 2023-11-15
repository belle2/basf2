# disable doxygen check for this file
# @cond

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


from prompt import ValidationSettings

import ROOT
import sys
import subprocess
import json

import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
from matplotlib import rcParams

import re
import os
from glob import glob

from datetime import datetime

from collections import OrderedDict
from shutil import copyfile, rmtree


#: Tells the automated system some details of this script
settings = ValidationSettings(name='eCMS Calibrations',
                              description=__doc__,
                              download_files=['stdout'],
                              expert_config={})


def setPlotRange(df, tag):
    """
    Function which adjusts the y-axis range of the plot according to tag
    """

    if tag == '4S':
        df4S = df[df['Ecms'] > 10560]['Ecms']
        if len(df4S) > 0:
            yMin = df4S.min()
            plt.ylim(bottom=yMin-3)

        dfHigh = df[df['Ecms'] < 10600]['Ecms']
        if len(dfHigh) > 0:
            yMax = dfHigh.max()
            plt.ylim(top=yMax+3)

    elif tag == 'Off':
        dfOff = df[df['Ecms'] < 10560]['Ecms']
        if len(dfOff) > 0:
            yMax = dfOff.max()
            plt.ylim(top=yMax+3)


def toJST(times):
    """
    Converts time from UTC to the JST
    """
    return np.vectorize(lambda t:  datetime.utcfromtimestamp((t + 9) * 3600))(times)


def plotSplitLines(dfC):
    """
    Plot vertical lines in places where run energy type changes
    """
    indx = np.where(np.diff(dfC['pull'] == 0))[0]
    for i in indx:
        tt1 = toJST(dfC['t2'].loc[i]).item()
        tt2 = toJST(dfC['t1'].loc[i+1]).item()
        tt = tt1 + (tt2 - tt1) / 2
        plt.axvline(x=tt, color='g', linestyle='--')


def get_Ecms_values(path):
    """
    Load the values of the Ecms properties from the payloads into the list
    """

    runDict = {}
    with open(path + '/database.txt') as fDB:
        for ll in fDB:
            ll = ll.strip()
            ll = ll.split()

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


class Plotter():
    def __init__(self, location):
        """
        Data are loaded from text files to pandas
        """

        # read B-only calibration
        self.dfB = pd.read_csv(f'{location}/BonlyEcmsCalib.txt', delim_whitespace=True)

        # read combined calibration
        self.dfC = pd.read_csv(f'{location}/finalEcmsCalib.txt', delim_whitespace=True)

        # read mumu calibration
        self.dfM = pd.read_csv(f'{location}/mumuEcalib.txt', delim_whitespace=True)

        # add the state
        dfM = self.dfM
        dfC = self.dfC
        state = -1
        dfM['type'] = 1
        dfC['type'] = 1
        for i in range(len(dfM)):
            if (dfM['id'][i]) == 0:
                state *= -1

            dfM.at[i, 'type'] = state
            dfC.at[i, 'type'] = state

    @staticmethod
    def plotLine(df, var, color, label):
        """
        Plot single line with error band
        """

        varUnc = var + 'Unc'
        nan = np.full(len(df), np.nan)
        avg = (df['t1']+df['t2']).to_numpy()/2
        times = np.c_[df[['t1',    't2']].to_numpy(), avg].ravel()
        eCMS = np.c_[df[[var, var]].to_numpy(), nan].ravel()
        eCMSu = np.c_[df[[varUnc, varUnc]].to_numpy(), nan].ravel()

        timesg = np.c_[df['t1'].to_numpy(),   avg,  df['t2'].to_numpy()].ravel()
        eCMSg = np.c_[df[var].to_numpy(), nan,  df[var].to_numpy()].ravel()

        times = toJST(times)
        timesg = toJST(timesg)

        plt.fill_between(times, eCMS-eCMSu, eCMS+eCMSu, alpha=0.2, color=color)
        plt.plot(times, eCMS, linewidth=2, color=color, label=label)
        plt.plot(timesg, eCMSg, linewidth=2, color=color, alpha=0.35)

    def plotEcmsComparison(self, limits=None, tag=''):
        """
        Plot Ecms obtained from combined, hadB and mumu method
        """

        Plotter.plotLine(self.dfB, 'Ecms', 'green', label='B decay method')
        Plotter.plotLine(self.dfC, 'Ecms', 'red', label='Combined method')

        d = np.nanmedian(self.dfC['Ecms']-self.dfM['Ecms'])

        dfMc = self.dfM.copy()
        dfMc['Ecms'] += d
        Plotter.plotLine(dfMc, 'Ecms', 'blue', label=f'mumu method (+{round(d,1)} MeV)')

        plotSplitLines(self.dfC)
        setPlotRange(self.dfC, tag)

        plt.xlabel('time')
        plt.ylabel('Ecms [MeV]')

        plt.legend()

        loc = 'plots/allData'
        if limits is not None:
            plt.xlim(datetime.strptime(limits[0], '%Y-%m-%d'), datetime.strptime(limits[1], '%Y-%m-%d'))
            loc = 'plots/' + limits[0] + 'to' + limits[1]

        os.makedirs(loc, exist_ok=True)
        plt.savefig(f'{loc}/EcmsComparison{tag}.png')
        plt.clf()

    def plotSpreadComparison(self, limits=None):
        """
        Plot Ecms spread estimated from the combined method and B-decay method
        """

        Plotter.plotLine(self.dfB, 'spread', 'green', label='B decay method')
        Plotter.plotLine(self.dfC, 'spread', 'red', label='Combined method')

        plotSplitLines(self.dfC)

        plt.legend()

        plt.xlabel('time')
        plt.ylabel('spread [MeV]')

        loc = 'plots/allData'
        if limits is not None:
            plt.xlim(datetime.strptime(limits[0], '%Y-%m-%d'), datetime.strptime(limits[1], '%Y-%m-%d'))
            loc = 'plots/' + limits[0] + 'to' + limits[1]

        os.makedirs(loc, exist_ok=True)
        plt.savefig(f'{loc}/SpreadComparison.png')
        plt.clf()

    @staticmethod
    def plotCurve(df, var, label, withBand=True, withCurve=True):
        """
        Plot curve with possible error band where large intervals are distinguished by color
        """

        def plotBands(df, var, color, withBand=True, withCurve=True):
            varUnc = var + 'Unc'
            nan = np.full(len(df), np.nan)
            avg = (df['t1']+df['t2']).to_numpy()/2
            times = np.c_[df[['t1',    't2']].to_numpy(), avg].ravel()
            eCMS = np.c_[df[[var, var]].to_numpy(), nan].ravel()
            times = toJST(times)

            if withBand:
                eCMSu = np.c_[df[[varUnc, varUnc]].to_numpy(), nan].ravel()
                plt.fill_between(times, eCMS-eCMSu, eCMS+eCMSu, alpha=0.15, color=color)
            if withCurve:
                plt.plot(times, eCMS, linewidth=2, color=color)

        nan = np.full(len(df), np.nan)
        avg = (df['t1']+df['t2']).to_numpy()/2
        timesg = np.c_[df['t1'].to_numpy(),   avg,  df['t2'].to_numpy()].ravel()
        eCMSg = np.c_[df[var].to_numpy(), nan,  df[var].to_numpy()].ravel()
        timesg = toJST(timesg)

        if withCurve:
            plt.plot(timesg, eCMSg, linewidth=2, color='gray', alpha=0.35)

        plotBands(df[df['type'] == 1], var, 'red', withBand, withCurve)
        plotBands(df[df['type'] == -1], var, 'blue', withBand, withCurve)

    def plotShift(self, limits=None):
        """
        Plot shift between Bhad and mumu method
        """

        Plotter.plotCurve(self.dfC, 'shift',  label='Combined method')

        plotSplitLines(self.dfC)

        plt.xlabel('time')
        plt.ylabel('shift [MeV]')

        loc = 'plots/allData'
        if limits is not None:
            plt.xlim(datetime.strptime(limits[0], '%Y-%m-%d'), datetime.strptime(limits[1], '%Y-%m-%d'))
            loc = 'plots/' + limits[0] + 'to' + limits[1]

        os.makedirs(loc, exist_ok=True)
        plt.savefig(f'{loc}/Shift.png')

        plt.clf()

    def plotEcms(self, limits=None,  tag=''):
        """
        Plot Ecms of the combined fit, 'tag' can be '4S' means that y-axis is zoomed to 4S mass region, 'Off' to off-resonance
        """

        Plotter.plotCurve(self.dfC, 'Ecms',  label='Combined method')

        plotSplitLines(self.dfC)

        plt.xlabel('time')
        plt.ylabel('Ecms [MeV]')

        setPlotRange(self.dfC, tag)

        loc = 'plots/allData'
        if limits is not None:
            plt.xlim(datetime.strptime(limits[0], '%Y-%m-%d'), datetime.strptime(limits[1], '%Y-%m-%d'))
            loc = 'plots/' + limits[0] + 'to' + limits[1]

        os.makedirs(loc, exist_ok=True)
        plt.savefig(f'{loc}/Ecms{tag}.png')
        plt.clf()

    def plotPulls(self, limits=None):
        """
        Plot pulls of the combined fit.
        """

        dfC = self.dfC.copy()
        dfC['ref'] = 0
        dfC['refUnc'] = 1
        Plotter.plotCurve(dfC, 'pull', label='Combined method', withBand=False)
        Plotter.plotCurve(dfC, 'ref',  label='Combined method', withCurve=False)

        plotSplitLines(self.dfC)

        plt.ylim(-1.5, 1.5)
        plt.xlabel('time')
        plt.ylabel('pull')

        loc = 'plots/allData'
        if limits is not None:
            plt.xlim(datetime.strptime(limits[0], '%Y-%m-%d'), datetime.strptime(limits[1], '%Y-%m-%d'))
            loc = 'plots/' + limits[0] + 'to' + limits[1]

        os.makedirs(loc, exist_ok=True)
        plt.savefig(f'{loc}/Pull.png')
        plt.clf()


# Get the results from the combined calibration
def read_Combined_data(outputDir):
    """
    It reads the calibration table from the text file produced by the CAF calibration.
    This text file includes the results from the final calibration.
    (combined or mumu-only)
    """

    arr = []
    with open(outputDir + '/finalEcmsCalib.txt') as text_file:
        for i, ll in enumerate(text_file):
            if i == 0:
                continue
            ll = ll.strip().split()
            arr.append(
                ((float(
                    ll[0]), float(
                    ll[1])), (int(ll[2]), int(ll[3]), int(ll[4]), int(ll[5])),     int(
                    ll[2+4]), (float(
                        ll[3+4]), float(
                        ll[4+4])), float(
                            ll[5+4]), (float(
                                ll[6+4]), float(
                                    ll[7+4])), (float(
                                        ll[8+4]), float(
                                            ll[9+4]))))
    return arr


# Get the results from the combined calibration
def read_Bonly_data(outputDir):
    """
    It reads the calibration table from the text file produced by the CAF calibration.
    This text file includes the results from the B-only calibration.
    """

    arr = []
    with open(outputDir + '/BonlyEcmsCalib.txt') as text_file:
        for i, ll in enumerate(text_file):
            if i == 0:
                continue
            ll = ll.strip().split()
            arr.append(
                ((float(
                    ll[0]), float(
                    ll[1])), (int(ll[2]), int(ll[3]), int(ll[4]), int(ll[5])),
                 (float(ll[2+4]), float(ll[3+4])), (float(ll[4+4]), float(ll[5+4]))))
    return arr


def read_mumu_data(outputDir):
    """
    It reads the calibration table from the text file produced by the CAF calibration.
    This text file includes the results from the mumu-based calibration.
    """

    arr = []
    with open(outputDir + '/mumuEcalib.txt') as text_file:
        for i, ll in enumerate(text_file):
            if i == 0:
                continue
            ll = ll.strip().split()
            arr.append(
                ((float(
                    ll[2+0]), float(
                    ll[2+1])), (int(ll[2+2]), int(ll[2+3]), int(ll[2+4]), int(ll[2+5])),
                 (float(ll[2+6]), float(ll[2+7])), (int(ll[0]), int(ll[1]))))
    return arr


# Create multi-page pdf file with the fit plots
def create_hadB_fit_plots(outputDir, pdflatex):
    """
    Create multi-page pdf file with the fit plots for hadronic B decays (with mumu constraint).
    The file is created using pdflatex
    """

    arr = read_Combined_data(outputDir)

    dName = 'plotsHadB'

    files = glob(outputDir+'/'+dName + '/*.pdf')
    files = list(map(os.path.basename, files))

    items = OrderedDict()

    for f in files:
        res = re.search('B[p0]_([0-9]*)_([0-9]*)\\.pdf', f)
        t = int(res.group(1))
        i = int(res.group(2))
        if t not in items:
            items[t] = 1
        else:
            items[t] = max(items[t], i + 1)

    times = sorted(items)

    header = """\\documentclass[aspectratio=169]{beamer}
    \\usepackage{graphicx}

    \\begin{document}
    """

    body = ""
    for t in times:

        i0 = None
        for i0Temp in range(len(arr)):
            if int(round(arr[i0Temp][0][0], 0)) == t:
                i0 = i0Temp
                break
        assert(i0 is not None)

        frac = 1. / (items[t] + 0.2)
        if items[t] >= 6:
            frac = 1. / (items[t] + 0.3)

        body += '\\begin{frame}[t]\n'
        shift, shiftE = str(round(arr[i0][5][0], 1)), str(round(arr[i0][5][1], 1))
        spread, spreadE = str(round(arr[i0][6][0], 1)), str(round(arr[i0][6][1], 1))

        body += '$E_\\mathrm{shift}    = (' + shift + '\\pm' + shiftE + ')$~MeV \\hspace{2cm}   \n'
        body += '$E_\\mathrm{spread}   = (' + spread + '\\pm' + spreadE + ')$~MeV    \\\\ \\vspace{0.5cm}\n'
        for iShort in range(items[t]):
            i = i0 + iShort
            tStart, tEnd = arr[i][0][0], arr[i][0][1]

            exp1, run1, exp2, run2 = map(str, arr[i][1])

            eCMS, eCMSe = str(round(arr[i][3][0], 1)), str(round(arr[i][3][1], 1))
            pull = str(round(arr[i][4], 2))

            t1 = datetime.utcfromtimestamp((tStart + 9) * 3600).strftime('%y-%m-%d %H:%M')
            t2 = datetime.utcfromtimestamp((tEnd + 9) * 3600).strftime('%y-%m-%d %H:%M')

            body += '\\begin{minipage}{' + str(frac) + '\\textwidth}\n'
            body += '\\begin{center}\n'
            body += '\\scriptsize ' + exp1 + ' '+run1 + ' \\\\\n'
            body += '\\scriptsize ' + exp2 + ' '+run2 + ' \\\\\n'
            body += '\\scriptsize ' + t1 + ' \\\\\n'
            body += '\\scriptsize ' + t2 + ' \\\\\n'
            body += '\\scriptsize ' + str(round(tEnd - tStart, 1)) + ' hours \\\\ \\vspace{0.3cm}\n'
            body += '$(' + eCMS + '\\pm' + eCMSe + ')$~MeV    \\\\\n'
            body += '$\\mathrm{pull}    = ' + pull + '$    \\\\\n'
            body += '\\includegraphics[width=1.0\\textwidth]{' + outputDir + \
                '/' + dName + '/B0_' + str(t) + '_' + str(iShort) + '.pdf}\\\\\n'
            body += '\\includegraphics[width=1.0\\textwidth]{' + outputDir + \
                '/' + dName + '/Bp_' + str(t) + '_' + str(iShort) + '.pdf}\n'
            body += '\\end{center}\n'
            body += '\\end{minipage}\n'

        body += '\\end{frame}\n\n'

    tail = '\n\\end{document}'

    whole = header + body + tail

    os.makedirs('tmp', exist_ok=True)

    with open("tmp/hadBfits.tex", "w") as text_file:
        text_file.write(whole)

    subprocess.call(f'{pdflatex} tmp/hadBfits.tex', shell=True)

    os.makedirs('plots', exist_ok=True)
    copyfile('hadBfits.pdf', 'plots/hadBfits.pdf')

    ext = ['aux', 'log', 'nav', 'out', 'pdf', 'snm', 'toc']
    for e in ext:
        if os.path.exists(f'hadBfits.{e}'):
            os.remove(f'hadBfits.{e}')

    rmtree('tmp')


# Create multi-page pdf file with the fit plots
def create_hadBonly_fit_plots(outputDir, pdflatex):
    """
    Create multi-page pdf file with the fit plots for hadronic B decays (without using mumu constraint).
    The file is created using pdflatex
    """

    arr = read_Bonly_data(outputDir)

    dName = 'plotsHadBonly'

    files = glob(outputDir+'/'+dName + '/*.pdf')
    files = list(map(os.path.basename, files))

    items = set()

    for f in files:
        res = re.search('B[p0]Single_([0-9]*)\\.pdf', f)
        t = int(res.group(1))
        items.add(t)  # [t] = 1

    items = sorted(items)

    header = """\\documentclass[aspectratio=169]{beamer}
    \\usepackage{graphicx}

    \\begin{document}
    """

    body = ""
    for i, t in enumerate(items):

        body += '\\begin{frame}[t]\n'

        exp1, run1, exp2, run2 = map(str, arr[i][1])
        eCMS, eCMSe = str(round(arr[i][2][0], 1)), str(round(arr[i][2][1], 1))
        spread, spreadE = str(round(arr[i][3][0], 1)), str(round(arr[i][3][1], 1))

        body += '$E_\\mathrm{cms} = (' + eCMS + '\\pm' + eCMSe + ')$~MeV    \\\\\n'
        body += '$E_\\mathrm{spread}   = (' + spread + '\\pm' + spreadE + ')$~MeV    \\\\ \\vspace{0.5cm}\n'

        tStart, tEnd = arr[i][0][0], arr[i][0][1]

        t1 = datetime.utcfromtimestamp((tStart + 9) * 3600).strftime('%y-%m-%d %H:%M')
        t2 = datetime.utcfromtimestamp((tEnd + 9) * 3600).strftime('%y-%m-%d %H:%M')

        body += '\\begin{minipage}{' + str(0.99) + '\\textwidth}\n'
        body += '\\begin{center}\n'
        body += '\\scriptsize ' + exp1 + ' ' + run1 + '\\hspace{1cm} ' + t1 + ' \\\\\n'
        body += '\\scriptsize ' + exp2 + ' ' + run2 + '\\hspace{1cm} ' + t2 + ' \\\\\n'
        body += '\\scriptsize ' + str(round(tEnd - tStart, 1)) + ' hours \\\\ \\vspace{0.3cm}\n'
        body += '\\includegraphics[width=0.48\\textwidth]{'+outputDir + '/' + dName + '/B0Single_' + str(t) + '.pdf}\n'
        body += '\\includegraphics[width=0.48\\textwidth]{'+outputDir + '/' + dName + '/BpSingle_' + str(t) + '.pdf}\n'
        body += '\\end{center}\n'
        body += '\\end{minipage}\n'

        body += '\\end{frame}\n\n'

    tail = '\n\\end{document}'

    whole = header + body + tail

    os.makedirs('tmp', exist_ok=True)

    with open("tmp/hadBonlyFits.tex", "w") as text_file:
        text_file.write(whole)

    subprocess.call(f'{pdflatex} tmp/hadBonlyFits.tex', shell=True)

    os.makedirs('plots', exist_ok=True)
    copyfile('hadBonlyFits.pdf', 'plots/hadBonlyFits.pdf')

    ext = ['aux', 'log', 'nav', 'out', 'pdf', 'snm', 'toc']
    for e in ext:
        if os.path.exists(f'hadBonlyFits.{e}'):
            os.remove(f'hadBonlyFits.{e}')

    if os.path.exists("tmp/hadBonlyFits.tex"):
        os.remove("tmp/hadBonlyFits.tex")
    rmtree('tmp')


def create_mumu_fit_plots(outputDir, pdflatex):
    """
    Create multi-page pdf file with the fit plots for mumu method.
    The file is created using pdflatex
    """

    arr = read_mumu_data(outputDir)

    limits = []
    for i, a in enumerate(arr):
        if a[3][1] == a[3][0] - 1:
            limits.append((i - a[3][1], a[3][0]))

    dName = 'plotsMuMu'

    files = glob(outputDir+'/'+dName + '/*.pdf')
    files = list(map(os.path.basename, files))

    items = set()

    for f in files:
        res = re.search('mumu_([0-9]*)\\.pdf', f)
        t = int(res.group(1))
        items.add(t)

    items = sorted(items)

    header = """\\documentclass[aspectratio=169]{beamer}
    \\usepackage{graphicx}

    \\begin{document}
    """

    body = ""
    for k, n in limits:

        frac = None
        if n >= 11:
            frac = 0.159
        elif n >= 9:
            frac = 0.193
        elif n >= 7:
            frac = 0.24
        elif n >= 5:
            frac = 0.3
        elif n >= 3:
            frac = 0.33
        elif n >= 2:
            frac = 0.48
        elif n >= 1:
            frac = 0.75

        body += '\\begin{frame}[t]\n'

        for i in range(k, k+n):

            body += '\\begin{minipage}{' + str(frac) + '\\textwidth}\n'

            exp1, run1, exp2, run2 = map(str, arr[i][1])
            eCMS, eCMSe = str(round(arr[i][2][0], 1)), str(round(arr[i][2][1], 1))

            tStart, tEnd = arr[i][0][0], arr[i][0][1]

            t1 = datetime.utcfromtimestamp((tStart + 9) * 3600).strftime('%y-%m-%d %H:%M')
            t2 = datetime.utcfromtimestamp((tEnd + 9) * 3600).strftime('%y-%m-%d %H:%M')

            body += '\\begin{center}\n'
            body += '\\tiny  $E_\\mathrm{cms} = (' + eCMS + '\\pm' + eCMSe + ')$~MeV    \\\\\n'
            body += '\\tiny ' + exp1 + ' ' + run1 + '\\hspace{0.3cm} ' + t1 + ' \\\\\n'
            body += '\\tiny ' + exp2 + ' ' + run2 + '\\hspace{0.3cm} ' + t2 + ' \\\\\n'
            body += '\\tiny ' + str(round(tEnd - tStart, 1)) + ' hours  \\vspace{0.3cm}\n'
            body += '\\includegraphics[trim=0.3cm 0.0cm 1.3cm 0.7cm,clip=true,width=0.99\\textwidth]{' + \
                outputDir + '/' + dName + '/mumu_' + str(items[i]) + '.pdf}\n'
            body += '\\end{center}\n'
            body += '\\end{minipage}\n'

        body += '\\end{frame}\n\n'

    tail = '\n\\end{document}'

    whole = header + body + tail

    os.makedirs('tmp', exist_ok=True)

    with open("tmp/mumuFits.tex", "w") as text_file:
        text_file.write(whole)

    subprocess.call(f'{pdflatex} tmp/mumuFits.tex', shell=True)

    os.makedirs('plots', exist_ok=True)
    copyfile('mumuFits.pdf', 'plots/mumuFits.pdf')

    ext = ['aux', 'log', 'nav', 'out', 'pdf', 'snm', 'toc']
    for e in ext:
        if os.path.exists(f'mumuFits.{e}'):
            os.remove(f'mumuFits.{e}')

    if os.path.exists("tmp/mumuFits.tex"):
        os.remove("tmp/mumuFits.tex")
    rmtree('tmp')


def run_validation(job_path, input_data_path, requested_iov, expert_config):
    """
    Create validation plots related to the Ecms calibration
    """

    # get location of pdflatex
    result = subprocess.run('locate pdflatex | grep "/pdflatex$"', stdout=subprocess.PIPE, shell=True)
    pdflatex = result.stdout.strip().decode('utf-8')

    # Expert config can contain the time ranges of the plots
    if expert_config != '':
        expert_config = json.loads(expert_config)

    allLimits = [None]
    if expert_config is not None and 'plotsRanges' in expert_config:
        allLimits += expert_config['plotsRanges']

    plt.figure(figsize=(18, 9))
    rcParams['axes.formatter.useoffset'] = False

    location = f'{job_path}/eCMS/0/algorithm_output'

    plotter = Plotter(location)
    # plot the results
    for limits in allLimits:
        plotter.plotEcmsComparison(limits, tag='4S')
        plotter.plotEcmsComparison(limits, tag='Off')
        plotter.plotEcmsComparison(limits)
        plotter.plotSpreadComparison(limits)

        plotter.plotEcms(limits, tag='4S')
        plotter.plotEcms(limits, tag='Off')
        plotter.plotEcms(limits, tag='')

        plotter.plotShift(limits)
        plotter.plotPulls(limits)

    # create pdf with plots of fits
    create_hadB_fit_plots(location, pdflatex)
    create_hadBonly_fit_plots(location, pdflatex)
    create_mumu_fit_plots(location, pdflatex)

    # copy csv files to validation directory
    copyfile(f'{location}/BonlyEcmsCalib.txt', 'plots/BonlyEcmsCalib.txt')
    copyfile(f'{location}/finalEcmsCalib.txt', 'plots/finalEcmsCalib.txt')
    copyfile(f'{location}/mumuEcalib.txt', 'plots/mumuEcalib.txt')


if __name__ == "__main__":
    run_validation(*sys.argv[1:])

# @endcond
