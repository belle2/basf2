##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################
'''
   Prepare CDC x-talk simulation  DB object, usage:

   python3 prepareAsicCrosstalkSimDB.py <input_asic_root> <output_path> <exp_num> <data_type>

     input_asic_root:   input path and files containing asic information,
                        produced by AsicBackgroundLibraryCreator.py
     output_path: output path
     exp_num:   experiment number of raw data or mc samples
     data_type: data or mc

'''

import uproot
from root_pandas import to_root
import matplotlib.pyplot as plt
import numpy as np
from scipy.interpolate import UnivariateSpline

import basf2
from ROOT.Belle2 import FileSystem
from ROOT.Belle2 import CDCDatabaseImporter
from ROOT import TH1F, TFile
import sys


def getEff(var, cut, n=40, limits=(0., 2500.)):
    ''' Simple efficiency estimator
       var: pandas series/np array of variable to study
       cut: bool series/np array pass/fail
       n  : number of bins
       limits : histogram limits
    '''
    a = np.histogram(var, n, limits)
    b = np.histogram(var[cut], n, limits)
    eff = b[0]/a[0]
    # Simple binomial formula:
    effErr = 1/a[0]*np.sqrt(a[0]*eff*(1-eff))
    x = 0.5*(a[1][1:]+a[1][:-1])
    return x, eff, effErr


# Problematic boards ID
remB = {'22': [24, 196],
        '24': [0, 15, 24, 40, 117, 175, 196, 202, 86, 89],
        '25': [0, 24, 15],
        '26': [24, 210, 62],
        '27': [92, 24, 51, 106, 53],
        }
#
InputFile = sys.argv[1]
OutputPath = sys.argv[2]
exp = sys.argv[3]
data_type = sys.argv[4]

# Dataframe, containing relevant variables from the root file
df = uproot.open(InputFile, flatten=True)["ASIC"].arrays(["Channel", "ADC", "Board", "Nhit", "Asic"], library="pd")
df.columns = ['_'.join(col) if col[1] != '' else col[0] for col in df.columns.values]
# Defining asic
df['asic'] = df.Channel//8
# Remove the problematic boards
mask = df['Board'].isin([m for m in remB[f'{exp}']])
df = df[~mask]

nhits = 3
#: auxiliary variable, split ADC range in two < 1024 and above (u1: below)
u1 = getEff(df[(df.asic % 3 == 1)].ADC_ADC_Sig, df.Nhit > f'{nhits}', 128, (0, 1024.))
#: auxiliary variable, split ADC range in two < 1024 and above (u2: above)
u2 = getEff(df[(df.asic % 3 == 1)].ADC_ADC_Sig, df.Nhit > f'{nhits}', 8, (1024, 4096.))
#: merged u1 and u2
u = np.append(u1, u2, axis=1)

plt.figure(figsize=(10, 4))
#: aux. ADC variable, remove NaNs
x = np.nan_to_num(u[0])
#: aux. x-talk prob. variable, remove NaNs
e = np.nan_to_num(u[1])
#: aux. x-talk prob. error varlable, remove NaNs
ee = np.where(np.nan_to_num(u[2]) == 0, 1000., u[2])

#: spline parameterisation of efficiency
f = UnivariateSpline(x, e, 1/ee)
#: location of points for the plot
xp = np.arange(-0.5, 4096.5, 1)

plt.subplot(121)
plt.xlim(0, 4096.)
plt.ylim(0., 1.02)
plt.errorbar(x, e, u[2], fmt='.')
plt.ylabel(f'Fraction of Nhit>{nhits}')
plt.title(f'exp{exp} {data_type}')
plt.plot(xp, f(xp))

plt.subplot(122)
plt.xlim(0, 1000.)
plt.ylim(0., 0.5)
plt.errorbar(x, e, u[2], fmt='.', label='eff: asic=1,4; Nhit>3')
plt.plot(xp, f(xp), label='spline fit')
plt.xlabel('ADC')
plt.legend()
plt.savefig(f'{OutputPath}/xTalkProb_{exp}_{data_type}.pdf')

# Write out root file
#: variables to write out
names = ["Board", "Channel", 'Nhit', 'asic']
for i in range(8):
    names += ['Asic_ADC{:d}'.format(i), 'Asic_TDC{:d}'.format(i), 'Asic_TOT{:d}'.format(i)]
#: do not write index
to_root(df[(df.nhit > 3) & (df.asic % 3 == 1)][names], f'{OutputPath}/xTalkProb_{exp}_{data_type}.root', index=False)
#: open root file to store x-talk probability histogram
fi = TFile(f'{OutputPath}/xTalkProb_{exp}_{data_type}.root', "update")
#: histogram to store x-talk probability
t = TH1F("ProbXTalk", "Prob xTalk", 4096, 0, 4096)
t.SetContent(f(xp))
fi.Write()
fi.Close()

#: prepared payload:
INPUT = FileSystem.findFile(f'{OutputPath}/xTalkProb_{exp}_{data_type}.root')

# Specify the exp and run where iov is valid.
# N.B. -1 means unbound.
#: starting experiment for payload
expFirst = 0
#: last experiment for payload
expLast = -1
#: first run for payload
runFirst = 0
#: last run for payload
runLast = -1
# basf2.use_local_database("localdb/database.txt", "localdb")
basf2.conditions.testing_payloads = [f'localdb/exp{exp}/database_{exp}.txt']

#: path
main = basf2.create_path()

#: Event info setter - execute single event
eventinfosetter = basf2.register_module('EventInfoSetter')
main.add_module(eventinfosetter)

# process single event
basf2.process(main)

#: get the importer module
dbImporter = CDCDatabaseImporter(expFirst, runFirst, expLast, runLast)
dbImporter.importCDCCrossTalkLibrary(INPUT)

dbImporter.printCDCCrossTalkLibrary()
dbImporter.testCDCCrossTalkLibrary()
