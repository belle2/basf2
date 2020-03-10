''' Prepare CDC x-talk simulation  DB object '''

from root_pandas import read_root
import matplotlib.pyplot as plt
import numpy as np
from scipy.interpolate import UnivariateSpline

import basf2
import ROOT
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

InputFile = "cosmic.0008.03420_03427.root"

df = read_root(InputFile, columns=["Channel", "ADC", "Board", "Nhit", "Asic"])
df['asic'] = df.Channel//8
u1 = getEff(df[(df.asic % 3 == 1)].ADC_ADC_Sig, df.Nhit > 1, 128, (0, 1024.))
u2 = getEff(df[(df.asic % 3 == 1)].ADC_ADC_Sig, df.Nhit > 1, 16, (1024, 7800.))

u = np.append(u1, u2, axis=1)

plt.figure()

x = np.nan_to_num(u[0])
e = np.nan_to_num(u[1])
ee = np.where(np.nan_to_num(u[2]) == 0, 1000., u[2])

f = UnivariateSpline(x, e, 1/ee)

xp = np.arange(-0.5, 8197.5, 1)

plt.subplot(211)
plt.xlim(0, 8196.)
plt.ylim(0., 1.02)
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

# Write out root file:
names = ["Board", "Channel"]
for i in range(8):
    names += ['Asic_ADC{:d}'.format(i), 'Asic_TDC{:d}'.format(i), 'Asic_TOT{:d}'.format(i)]
df[(df.Nhit > 1) & (df.asic % 3 == 1)][names].to_root("t.root", index=False)
fi = TFile("t.root", "update")
t = TH1F("ProbXTalk", "Prob xTalk", 8196, 0, 8196)
t.SetContent(f(xp))
fi.Write()
fi.Close()


INPUT = FileSystem.findFile("t.root")

# Specify the exp and run where iov is valid.
# N.B. -1 means unbound.
expFirst = 0
expLast = -1
runFirst = 0
runLast = -1
basf2.use_local_database("localdb/database.txt", "localdb")

main = basf2.create_path()

# Event info setter - execute single event
eventinfosetter = basf2.register_module('EventInfoSetter')
main.add_module(eventinfosetter)

# process single event
basf2.process(main)

dbImporter = CDCDatabaseImporter(expFirst, runFirst, expLast, runLast)
dbImporter.importCDCCrossTalkLibrary(INPUT)

dbImporter.printCDCCrossTalkLibrary()
dbImporter.testCDCCrossTalkLibrary()
