##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################
''' Prepare CDC x-talk simulation  DB object '''

import uproot
import matplotlib.pyplot as plt
import numpy as np
from scipy.interpolate import UnivariateSpline

import basf2
from ROOT.Belle2 import FileSystem
from ROOT.Belle2 import CDCDatabaseImporter
from ROOT import TH1F, TFile


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
    # simple binomial formula:
    effErr = 1/a[0]*np.sqrt(a[0]*eff*(1-eff))
    x = 0.5*(a[1][1:]+a[1][:-1])
    return x, eff, effErr


#
# Specify file name here:
#
#: Input root file, produced by AsicBackgroundLibraryCreator
InputFile = "cosmic.0008.03420_03427.root"

#: Dataframe, containing relevant variables from the root file
df = uproot.open(InputFile)["tree"].arrays(["Channel", "ADC", "Board", "Nhit", "Asic"], library="pd")
df['asic'] = df.Channel//8
#: auxiliary variable, split ADC range in two < 1024 and above (u1: below)
u1 = getEff(df[(df.asic % 3 == 1)].ADC_ADC_Sig, df.Nhit > 1, 128, (0, 1024.))
#: auxiliary variable, split ADC range in two < 1024 and above (u2: above)
u2 = getEff(df[(df.asic % 3 == 1)].ADC_ADC_Sig, df.Nhit > 1, 16, (1024, 7800.))
#: merged u1 and u2
u = np.append(u1, u2, axis=1)

plt.figure()

#: aux. ADC variable, remove NaNs
x = np.nan_to_num(u[0])
#: aux. x-talk prob. variable, remove NaNs
e = np.nan_to_num(u[1])
#: aux. x-talk prob. error varlable, remove NaNs
ee = np.where(np.nan_to_num(u[2]) == 0, 1000., u[2])

#: spline parameterisation of efficiency
f = UnivariateSpline(x, e, 1/ee)

#: location of points for the plot
xp = np.arange(-0.5, 8197.5, 1)

plt.subplot(211)
plt.xlim(0, 8196.)
plt.ylim(0., 1.02)
#: use dots
plt.errorbar(x, e, u[2], fmt='.')
plt.ylabel('Fraction of $N_{hit}>1$')
plt.plot(xp, f(xp))

plt.subplot(212)
plt.xlim(0, 1000.)
plt.ylim(0., 0.5)
plt.errorbar(x, e, u[2], fmt='.')
plt.plot(xp, f(xp))
plt.xlabel('ADC')
plt.savefig("xTalkProb.pdf")

# Write out root file

#: variables to write out
names = ["Board", "Channel"]
for i in range(8):
    names += ['Asic_ADC{:d}'.format(i), 'Asic_TDC{:d}'.format(i), 'Asic_TOT{:d}'.format(i)]
#: do not write index
df[(df.Nhit > 1) & (df.asic % 3 == 1)][names].to_root("t.root", index=False)
#: open root file to store x-talk probability histogram
fi = TFile("t.root", "update")
#: histogram to store x-talk probability
t = TH1F("ProbXTalk", "Prob xTalk", 8196, 0, 8196)
t.SetContent(f(xp))
fi.Write()
fi.Close()

#: prepared payload:
INPUT = FileSystem.findFile("t.root")

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
basf2.use_local_database("localdb/database.txt", "localdb")

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
