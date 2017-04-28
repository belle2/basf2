import numpy as np
import matplotlib.pyplot as plt
import ROOT

f = ROOT.TFile.Open("TOPOutput.root")
t = f.Get("tree")
# one entry per event
t.GetEntry(0)
# one collection per track
x = t.TOPPDFCollections[0]
# we create the image from the channels on X, and time bins on Y
X, Y = np.meshgrid(range(512), np.linspace(0, 0.3 * 199, 200))
# x.m_data is a map of hypotheses to a 2D PDF
Z = np.array(x.m_data[1]).T
plt.pcolor(X, Y, Z)

ch = []
times = []
digits = t.TOPDigits
for d in digits:
    ch.append(d.getChannel())
    times.append(d.getTime())

plt.plot(ch, times, 'ro')
# plt.show()
plt.savefig("pdf.png")
