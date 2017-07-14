import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import ROOT


def getHitArray(fname):
    f = ROOT.TFile.Open(fname)
    pixels = []
    times = []
    t = f.Get("tree")
    for ev in range(t.GetEntries()):
        t.GetEntry(ev)
        digits = t.TOPDigits
        for d in range(digits.GetEntries()):
            pixels.append(t.TOPDigits[d].getPixelID())
            times.append(t.TOPDigits[d].getTime())
    return pixels, times

p, t = getHitArray("pi10k.root")
plt.scatter(p, t, s=0.01, marker='.', c='b')
p, t = getHitArray("kaon10k.root")
plt.scatter(p, t, s=0.01, marker='.', c='r')
plt.savefig("scatter.pdf")
