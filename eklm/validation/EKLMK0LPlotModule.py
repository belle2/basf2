#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
<header>
    <contact>Kirill Chilikin (chilikin@lebedev.ru)</contact>
    <description>Creation of EKLM K0L validation plots.</description>
</header>
"""

from basf2 import *
import ROOT
from ROOT import Belle2
from ROOT import TFile, TH1F, TNamed
import math


class EKLMK0LPlotModule(Module):
    """ Class for creation of EKLM K0L validation plots. """

    def __init__(self):
        """Initialization."""
        super(EKLMK0LPlotModule, self).__init__()
        #: Output file.
        self.output_file = ROOT.TFile('EKLMK0L.root', 'recreate')
        contact = 'Kirill Chilikin (chilikin@lebedev.ru)'
        #: Number of K0L histogram.
        self.hist_nkl = ROOT.TH1F('k0l_number',
                                  'Number of KLM clusters per 1 MC particle',
                                  5, -0.5, 4.5)
        self.hist_nkl.SetXTitle('KLM clusters')
        self.hist_nkl.SetYTitle('Events')
        l = self.hist_nkl.GetListOfFunctions()
        l.Add(TNamed('Description', 'Number of KLM clusters per 1 MC particle'))
        l.Add(TNamed('Check', 'No efficiency decrease or multiple candidates \
                     increase'))
        l.Add(TNamed('Contact', contact))
        #: X resolution histogram.
        self.hist_xres = ROOT.TH1F('k0l_xres',
                                   'EKLM K0L decay vertex X resolution',
                                   100, -50, 50)
        self.hist_xres.SetXTitle('cm')
        self.hist_xres.SetYTitle('Events')
        l = self.hist_xres.GetListOfFunctions()
        l.Add(TNamed('Description', 'X resolution'))
        l.Add(TNamed('Check', 'No bias, resolution ~ 10 cm.'))
        l.Add(TNamed('Contact', contact))
        #: Y resolution histogram.
        self.hist_yres = ROOT.TH1F('k0l_yres',
                                   'EKLM K0L decay vertex Y resolution',
                                   100, -50, 50)
        self.hist_yres.SetXTitle('cm')
        self.hist_yres.SetYTitle('Events')
        l = self.hist_yres.GetListOfFunctions()
        l.Add(TNamed('Description', 'Y resolution'))
        l.Add(TNamed('Check', 'No bias, resolution ~ 10 cm.'))
        l.Add(TNamed('Contact', contact))
        #: Z resolution histogram.
        self.hist_zres = ROOT.TH1F('k0l_zres',
                                   'EKLM K0L decay vertex Z resolution',
                                   100, -50, 50)
        self.hist_zres.SetXTitle('cm')
        self.hist_zres.SetYTitle('Events')
        l = self.hist_zres.GetListOfFunctions()
        l.Add(TNamed('Description', 'Z resolution'))
        l.Add(TNamed('Check', 'No bias, resolution ~ 10 cm.'))
        l.Add(TNamed('Contact', contact))
        #: Time resolution histogram.
        self.hist_tres = ROOT.TH1F('k0l_tres',
                                   'EKLM K0L decay time resolution',
                                   100, -10., 10.)
        self.hist_tres.SetXTitle('ns')
        self.hist_tres.SetYTitle('Events')
        l = self.hist_tres.GetListOfFunctions()
        l.Add(TNamed('Description', 'Time resolution'))
        l.Add(TNamed('Check', 'No bias.'))
        l.Add(TNamed('Contact', contact))
        #: Momentum resolution histogram.
        self.hist_pres = ROOT.TH1F('k0l_pres',
                                   'EKLM K0L momentum resolution',
                                   100, -1., 1.)
        self.hist_pres.SetXTitle('GeV')
        self.hist_pres.SetYTitle('Events')
        l = self.hist_pres.GetListOfFunctions()
        l.Add(TNamed('Description', 'Momentum resolution'))
        l.Add(TNamed('Check', 'No bias.'))
        l.Add(TNamed('Contact', contact))
        #: Momentum theta resolution histogram.
        self.hist_ptres = ROOT.TH1F('k0l_ptres',
                                    'EKLM K0L momentum theta resolution',
                                    100, -0.2, 0.2)
        self.hist_ptres.SetXTitle('rad')
        self.hist_ptres.SetYTitle('Events')
        l = self.hist_ptres.GetListOfFunctions()
        l.Add(TNamed('Description', 'Momentum theta resolution'))
        l.Add(TNamed('Check', 'No bias, resolution ~ 0.03'))
        l.Add(TNamed('Contact', contact))
        #: Momentum phi resolution histogram.
        self.hist_ppres = ROOT.TH1F('k0l_ppres',
                                    'EKLM K0L momentum phi resolution',
                                    100, -0.2, 0.2)
        self.hist_ppres.SetXTitle('rad')
        self.hist_ppres.SetYTitle('Events')
        l = self.hist_ppres.GetListOfFunctions()
        l.Add(TNamed('Description', 'Momentum phi resolution'))
        l.Add(TNamed('Check', 'No bias, resolution ~ 0.05'))
        l.Add(TNamed('Contact', contact))

    def event(self):
        """ Event function. """
        mc_particles = Belle2.PyStoreArray('MCParticles')
        for mc_particle in mc_particles:
            # Select K_L0.
            if (mc_particle.getPDG() != 130):
                continue
            # Select primary K_L0.
            if (mc_particle.getProductionTime() > 0):
                continue
            # Select K_L0 with decay point in EKLM.
            vertex = mc_particle.getDecayVertex()
            time = mc_particle.getDecayTime()
            momentum = mc_particle.getMomentum()
            x = vertex.x()
            y = vertex.y()
            z = vertex.z()
            r = math.sqrt(x * x + y * y)
            if (r < 132.5 or r > 329.0):
                continue
            if (abs(x) < 8.2 or abs(y) < 8.2):
                continue
            if (not((z > -315.1 and z < -183.0) or (z > 277.0 and z < 409.1))):
                continue
            klm_clusters = mc_particle.getRelationsFrom('KLMClusters')
            self.hist_nkl.Fill(len(klm_clusters))
            for klm_cluster in klm_clusters:
                vertex_k = klm_cluster.getClusterPosition() - vertex
                time_k = klm_cluster.getTime()
                momentum_k = klm_cluster.getMomentum().Vect()
                self.hist_xres.Fill(vertex_k.x())
                self.hist_yres.Fill(vertex_k.y())
                self.hist_zres.Fill(vertex_k.z())
                self.hist_tres.Fill(time_k - time)
                self.hist_pres.Fill(momentum_k.Mag() - momentum.Mag())
                self.hist_ptres.Fill(momentum_k.Theta() - momentum.Theta())
                self.hist_ppres.Fill(momentum_k.Phi() - momentum.Phi())

    def terminate(self):
        """ Termination function. """
        self.output_file.cd()
        self.hist_nkl.Write()
        self.hist_xres.Write()
        self.hist_yres.Write()
        self.hist_zres.Write()
        self.hist_tres.Write()
        self.hist_pres.Write()
        self.hist_ptres.Write()
        self.hist_ppres.Write()
        self.output_file.Close()
