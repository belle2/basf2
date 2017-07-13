import numpy as np
import progressbar as pb
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import matplotlib.animation as animation

import ROOT
ROOT.gROOT.SetBatch(1)  # something's trying to open xwindows
from plotTOPPDFCollection import Gaussian, PDF
import os

gcmap = 'plasma'
xbins = np.linspace(0, 512, 513)
ybins = np.linspace(-5, 80, 201)
xcentres = xbins[0:-1] + 0.5 * (xbins[1:] - xbins[:-1])
ycentres = ybins[0:-1] + 0.5 * (ybins[1:] - ybins[:-1])

metadata = dict(title='Momentum_scan', artist='Jan Strube',
                comment='pdf scan across momentum')

fig = plt.figure()
ims = []
MAXFRAME = 240
bar = pb.ProgressBar(max_value=MAXFRAME)
for i in range(MAXFRAME):
    infile = "Momentum/pdf_momentum_scan_pid211_step_%03d.root" % i
    if not os.path.exists(infile):
        continue
    bar.update(i)
    f = ROOT.TFile.Open(infile)
    t = f.Get("tree")
    t.GetEntry(0)
    x = t.TOPPDFCollections[0]
    if not x:
        f.Close()
        continue
    X, Y = np.meshgrid(xcentres, ycentres)
    Z = np.zeros_like(X, dtype=np.float32)
    for pixel, pxData in enumerate(x.m_data[13]):
        lOG = []  # list of Gaussians
        for peak, width, norm in pxData:
            lOG.append(Gaussian(peak, width, norm))
            Z[:, pixel] = np.array([PDF(lOG, timebin) for timebin in ycentres])
    ims.append([plt.pcolor(X, Y, Z, cmap=gcmap)])
    # writer.grab_frame()
    f.Close()

im_ani = animation.ArtistAnimation(fig, ims, interval=50, repeat_delay=3000,
                                   blit=True)
# To save this second animation with some metadata, use the following command:
im_ani.save('momentum_pion.gif', metadata=metadata, writer="imagemagick")
