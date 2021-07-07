#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
<header>
    <contact>Kirill Chilikin (chilikin@lebedev.ru)</contact>
    <description>Creation of KLM K0L validation plots.</description>
</header>
"""

import basf2
import ROOT
from ROOT import Belle2
from ROOT import TNamed
import math
import numpy


class KLMK0LPlotModule(basf2.Module):
    """ Class for creation of KLM K0L validation plots. """

    def __init__(self, output_file, evtgen, check_eklm):
        """Initialization."""
        super(KLMK0LPlotModule, self).__init__()
        #: True for evtgen events, false for particle gun.
        self.evtgen = evtgen
        #: Whether to check if cluster is in EKLM.
        self.check_eklm = check_eklm
        #: Output file.
        self.output_file = ROOT.TFile(output_file, 'recreate')
        contact = 'Kirill Chilikin (chilikin@lebedev.ru)'
        #: Number of K0L histogram.
        self.hist_nkl = ROOT.TH1F('k0l_number',
                                  'Number of KLM clusters per 1 MC particle',
                                  5, -0.5, 4.5)
        self.hist_nkl.SetXTitle('KLM clusters')
        self.hist_nkl.SetYTitle('Events')
        functions = self.hist_nkl.GetListOfFunctions()
        functions.Add(TNamed('Description', 'Number of KLM clusters per 1 MC particle'))
        functions.Add(TNamed('Check', 'No efficiency decrease or multiple candidates \
                     increase'))
        functions.Add(TNamed('Contact', contact))
        functions.Add(TNamed('MetaOptions', 'shifter'))
        #: X resolution histogram.
        self.hist_xres = ROOT.TH1F('k0l_xres',
                                   'KLM K0L decay vertex X resolution',
                                   100, -50, 50)
        self.hist_xres.SetXTitle('cm')
        self.hist_xres.SetYTitle('Events')
        functions = self.hist_xres.GetListOfFunctions()
        functions.Add(TNamed('Description', 'X resolution'))
        functions.Add(TNamed('Check', 'No bias, resolution ~ 16 cm.'))
        functions.Add(TNamed('Contact', contact))
        functions.Add(TNamed('MetaOptions', 'shifter'))
        #: Y resolution histogram.
        self.hist_yres = ROOT.TH1F('k0l_yres',
                                   'KLM K0L decay vertex Y resolution',
                                   100, -50, 50)
        self.hist_yres.SetXTitle('cm')
        self.hist_yres.SetYTitle('Events')
        functions = self.hist_yres.GetListOfFunctions()
        functions.Add(TNamed('Description', 'Y resolution'))
        functions.Add(TNamed('Check', 'No bias, resolution ~ 16 cm.'))
        functions.Add(TNamed('Contact', contact))
        functions.Add(TNamed('MetaOptions', 'shifter'))
        #: Z resolution histogram.
        self.hist_zres = ROOT.TH1F('k0l_zres',
                                   'KLM K0L decay vertex Z resolution',
                                   100, -50, 50)
        self.hist_zres.SetXTitle('cm')
        self.hist_zres.SetYTitle('Events')
        functions = self.hist_zres.GetListOfFunctions()
        functions.Add(TNamed('Description', 'Z resolution'))
        functions.Add(TNamed('Check', 'No bias, resolution ~ 16 cm.'))
        functions.Add(TNamed('Contact', contact))
        functions.Add(TNamed('MetaOptions', 'shifter'))
        #: Time resolution histogram.
        self.hist_tres = ROOT.TH1F('k0l_tres',
                                   'KLM K0L decay time resolution',
                                   100, -20., 10.)
        self.hist_tres.SetXTitle('ns')
        self.hist_tres.SetYTitle('Events')
        functions = self.hist_tres.GetListOfFunctions()
        functions.Add(TNamed('Description', 'Time resolution'))
        functions.Add(TNamed('Check', 'No bias.'))
        functions.Add(TNamed('Contact', contact))
        functions.Add(TNamed('MetaOptions', 'shifter'))
        #: Momentum resolution histogram.
        self.hist_pres = ROOT.TH1F('k0l_pres',
                                   'KLM K0L momentum resolution',
                                   100, -3., 3.)
        self.hist_pres.SetXTitle('GeV')
        self.hist_pres.SetYTitle('Events')
        functions = self.hist_pres.GetListOfFunctions()
        functions.Add(TNamed('Description', 'Momentum resolution'))
        functions.Add(TNamed('Check', 'No bias.'))
        functions.Add(TNamed('Contact', contact))
        functions.Add(TNamed('MetaOptions', 'shifter'))
        #: Momentum theta resolution histogram.
        self.hist_ptres = ROOT.TH1F('k0l_ptres',
                                    'KLM K0L momentum theta resolution',
                                    100, -0.2, 0.2)
        self.hist_ptres.SetXTitle('rad')
        self.hist_ptres.SetYTitle('Events')
        functions = self.hist_ptres.GetListOfFunctions()
        functions.Add(TNamed('Description', 'Momentum theta resolution'))
        functions.Add(TNamed('Check', 'No bias, resolution ~ 0.06'))
        functions.Add(TNamed('Contact', contact))
        functions.Add(TNamed('MetaOptions', 'shifter'))
        #: Momentum phi resolution histogram.
        self.hist_ppres = ROOT.TH1F('k0l_ppres',
                                    'KLM K0L momentum phi resolution',
                                    100, -0.2, 0.2)
        self.hist_ppres.SetXTitle('rad')
        self.hist_ppres.SetYTitle('Events')
        functions = self.hist_ppres.GetListOfFunctions()
        functions.Add(TNamed('Description', 'Momentum phi resolution'))
        functions.Add(TNamed('Check', 'No bias, resolution ~ 0.07'))
        functions.Add(TNamed('Contact', contact))
        functions.Add(TNamed('MetaOptions', 'shifter'))
        #: Covariance matrix histogram.
        self.hist_covmat = ROOT.TH1F('k0l_covmat',
                                     'KLM K0L coordinates covariance matrix',
                                     6, 0, 1)
        self.hist_covmat.GetXaxis().SetBinLabel(1, 'xx')
        self.hist_covmat.GetXaxis().SetBinLabel(2, 'xy')
        self.hist_covmat.GetXaxis().SetBinLabel(3, 'xz')
        self.hist_covmat.GetXaxis().SetBinLabel(4, 'yy')
        self.hist_covmat.GetXaxis().SetBinLabel(5, 'yz')
        self.hist_covmat.GetXaxis().SetBinLabel(6, 'zz')
        self.hist_covmat.SetYTitle('Covariance, cm^{2}')
        functions = self.hist_covmat.GetListOfFunctions()
        functions.Add(TNamed('Description', 'Momentum phi resolution'))
        functions.Add(TNamed('Check', 'No large off-diagonal elements.'))
        functions.Add(TNamed('Contact', contact))
        functions.Add(TNamed('MetaOptions', 'shifter'))
        #: Correlation matrix histogram.
        self.hist_corrmat = ROOT.TH1F('k0l_corrmat',
                                      'KLM K0L correlation matrix',
                                      10, 0, 1)
        self.hist_corrmat.GetXaxis().SetBinLabel(1, 'xx')
        self.hist_corrmat.GetXaxis().SetBinLabel(2, 'xy')
        self.hist_corrmat.GetXaxis().SetBinLabel(3, 'xz')
        self.hist_corrmat.GetXaxis().SetBinLabel(4, 'xp')
        self.hist_corrmat.GetXaxis().SetBinLabel(5, 'yy')
        self.hist_corrmat.GetXaxis().SetBinLabel(6, 'yz')
        self.hist_corrmat.GetXaxis().SetBinLabel(7, 'yp')
        self.hist_corrmat.GetXaxis().SetBinLabel(8, 'zz')
        self.hist_corrmat.GetXaxis().SetBinLabel(9, 'zp')
        self.hist_corrmat.GetXaxis().SetBinLabel(10, 'pp')
        self.hist_corrmat.SetYTitle('Correlation coefficient')
        functions = self.hist_corrmat.GetListOfFunctions()
        functions.Add(TNamed('Description', 'Momentum phi resolution'))
        functions.Add(TNamed('Check', 'No large off-diagonal elements.'))
        functions.Add(TNamed('Contact', contact))
        functions.Add(TNamed('MetaOptions', 'shifter'))
        #: Average vertex.
        self.vertex_k_av = ROOT.TVector3(0, 0, 0)
        #: Vertex list.
        self.vertex = []
        #: Average momentum.
        self.momentum_av = 0
        #: Energy list.
        self.momentum = []

    def event(self):
        """ Event function. """
        mc_particles = Belle2.PyStoreArray('MCParticles')
        for mc_particle in mc_particles:
            # Select K_L0.
            if (mc_particle.getPDG() != 130):
                continue
            # Select primary K_L0.
            if (self.evtgen):
                b_pdg = abs(mc_particle.getMother().getPDG())
                if (not(b_pdg == 511)):
                    continue
            else:
                if (mc_particle.getProductionTime() > 0):
                    continue
            vertex = mc_particle.getDecayVertex()
            time = mc_particle.getDecayTime()
            momentum = mc_particle.getMomentum()
            if (self.check_eklm):
                x = vertex.x()
                y = vertex.y()
                z = vertex.z()
                r = math.sqrt(x * x + y * y)
                if (r < 132.5 or r > 329.0):
                    continue
                if (abs(x) < 8.2 or abs(y) < 8.2):
                    continue
                if (not((z > -315.1 and z < -183.0) or
                        (z > 277.0 and z < 409.1))):
                    continue
            klm_clusters = mc_particle.getRelationsFrom('KLMClusters')
            self.hist_nkl.Fill(len(klm_clusters))
            for klm_cluster in klm_clusters:
                vertex_k = klm_cluster.getClusterPosition() - vertex
                self.vertex.append(vertex_k)
                self.vertex_k_av = self.vertex_k_av + vertex_k
                self.momentum.append(klm_cluster.getMomentumMag())
                self.momentum_av = self.momentum_av + \
                    klm_cluster.getMomentumMag()
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
        self.vertex_k_av = self.vertex_k_av * (1.0 / len(self.vertex))
        self.momentum_av = self.momentum_av * (1.0 / len(self.vertex))
        # x, y, z, e
        cov_mat = numpy.zeros((4, 4))
        cov_mat_err = numpy.zeros((4, 4))
        corr_mat = numpy.zeros((4, 4))
        corr_mat_err = numpy.zeros((4, 4))
        for i in range(len(self.vertex)):
            cov_mat[0][0] = cov_mat[0][0] + \
                (self.vertex[i].x() - self.vertex_k_av.x()) * \
                (self.vertex[i].x() - self.vertex_k_av.x())
            cov_mat[0][1] = cov_mat[0][1] + \
                (self.vertex[i].x() - self.vertex_k_av.x()) * \
                (self.vertex[i].y() - self.vertex_k_av.y())
            cov_mat[0][2] = cov_mat[0][2] + \
                (self.vertex[i].x() - self.vertex_k_av.x()) * \
                (self.vertex[i].z() - self.vertex_k_av.z())
            cov_mat[0][3] = cov_mat[0][3] + \
                (self.vertex[i].x() - self.vertex_k_av.x()) * \
                (self.momentum[i] - self.momentum_av)
            cov_mat[1][1] = cov_mat[1][1] + \
                (self.vertex[i].y() - self.vertex_k_av.y()) * \
                (self.vertex[i].y() - self.vertex_k_av.y())
            cov_mat[1][2] = cov_mat[1][2] + \
                (self.vertex[i].y() - self.vertex_k_av.y()) * \
                (self.vertex[i].z() - self.vertex_k_av.z())
            cov_mat[1][3] = cov_mat[1][3] + \
                (self.vertex[i].y() - self.vertex_k_av.y()) * \
                (self.momentum[i] - self.momentum_av)
            cov_mat[2][2] = cov_mat[2][2] + \
                (self.vertex[i].z() - self.vertex_k_av.z()) * \
                (self.vertex[i].z() - self.vertex_k_av.z())
            cov_mat[2][3] = cov_mat[2][3] + \
                (self.vertex[i].z() - self.vertex_k_av.z()) * \
                (self.momentum[i] - self.momentum_av)
            cov_mat[3][3] = cov_mat[3][3] + \
                (self.momentum[i] - self.momentum_av) * \
                (self.momentum[i] - self.momentum_av)
        for i in range(0, 4):
            for j in range(i, 4):
                cov_mat[i][j] = cov_mat[i][j] / (len(self.vertex) - 1)
        for i in range(len(self.vertex)):
            cov_mat_err[0][0] = cov_mat_err[0][0] + \
                pow((self.vertex[i].x() - self.vertex_k_av.x()) *
                    (self.vertex[i].x() - self.vertex_k_av.x()) - cov_mat[0][0], 2)
            cov_mat_err[0][1] = cov_mat_err[0][1] + \
                pow((self.vertex[i].x() - self.vertex_k_av.x()) *
                    (self.vertex[i].y() - self.vertex_k_av.y()) - cov_mat[0][1], 2)
            cov_mat_err[0][2] = cov_mat_err[0][2] + \
                pow((self.vertex[i].x() - self.vertex_k_av.x()) *
                    (self.vertex[i].z() - self.vertex_k_av.z()) - cov_mat[0][2], 2)
            cov_mat_err[0][3] = cov_mat_err[0][3] + \
                pow((self.vertex[i].x() - self.vertex_k_av.x()) *
                    (self.momentum[i] - self.momentum_av) - cov_mat[0][3], 2)
            cov_mat_err[1][1] = cov_mat_err[1][1] + \
                pow((self.vertex[i].y() - self.vertex_k_av.y()) *
                    (self.vertex[i].y() - self.vertex_k_av.y()) - cov_mat[1][1], 2)
            cov_mat_err[1][2] = cov_mat_err[1][2] + \
                pow((self.vertex[i].y() - self.vertex_k_av.y()) *
                    (self.vertex[i].z() - self.vertex_k_av.z()) - cov_mat[1][2], 2)
            cov_mat_err[1][3] = cov_mat_err[1][3] + \
                pow((self.vertex[i].y() - self.vertex_k_av.y()) *
                    (self.momentum[i] - self.momentum_av) - cov_mat[1][3], 2)
            cov_mat_err[2][2] = cov_mat_err[2][2] + \
                pow((self.vertex[i].z() - self.vertex_k_av.z()) *
                    (self.vertex[i].z() - self.vertex_k_av.z()) - cov_mat[2][2], 2)
            cov_mat_err[2][3] = cov_mat_err[2][3] + \
                pow((self.vertex[i].z() - self.vertex_k_av.z()) *
                    (self.momentum[i] - self.momentum_av) - cov_mat[2][3], 2)
            cov_mat_err[3][3] = cov_mat_err[3][3] + \
                pow((self.momentum[i] - self.momentum_av) *
                    (self.momentum[i] - self.momentum_av) - cov_mat[3][3], 2)
        for i in range(0, 4):
            for j in range(i, 4):
                cov_mat_err[i][j] = \
                    math.sqrt(cov_mat_err[i][j] /
                              ((len(self.vertex) - 2) * len(self.vertex)))
        self.hist_covmat.SetBinContent(1, cov_mat[0][0])
        self.hist_covmat.SetBinContent(2, cov_mat[0][1])
        self.hist_covmat.SetBinContent(3, cov_mat[0][2])
        self.hist_covmat.SetBinContent(4, cov_mat[1][1])
        self.hist_covmat.SetBinContent(5, cov_mat[1][2])
        self.hist_covmat.SetBinContent(6, cov_mat[2][2])
        self.hist_covmat.SetBinError(1, cov_mat_err[0][0])
        self.hist_covmat.SetBinError(2, cov_mat_err[0][1])
        self.hist_covmat.SetBinError(3, cov_mat_err[0][2])
        self.hist_covmat.SetBinError(4, cov_mat_err[1][1])
        self.hist_covmat.SetBinError(5, cov_mat_err[1][2])
        self.hist_covmat.SetBinError(6, cov_mat_err[2][2])
        for i in range(0, 4):
            for j in range(i, 4):
                corr_mat[i][j] = cov_mat[i][j] / \
                    math.sqrt(cov_mat[i][i]) / math.sqrt(cov_mat[j][j])
                # Normalization error is not taken into account.
                corr_mat_err[i][j] = cov_mat_err[i][j] / cov_mat[i][j] * \
                    corr_mat[i][j]
        self.hist_corrmat.SetBinContent(1, corr_mat[0][0])
        self.hist_corrmat.SetBinContent(2, corr_mat[0][1])
        self.hist_corrmat.SetBinContent(3, corr_mat[0][2])
        self.hist_corrmat.SetBinContent(4, corr_mat[0][3])
        self.hist_corrmat.SetBinContent(5, corr_mat[1][1])
        self.hist_corrmat.SetBinContent(6, corr_mat[1][2])
        self.hist_corrmat.SetBinContent(7, corr_mat[1][3])
        self.hist_corrmat.SetBinContent(8, corr_mat[2][2])
        self.hist_corrmat.SetBinContent(9, corr_mat[2][3])
        self.hist_corrmat.SetBinContent(10, corr_mat[3][3])
        self.hist_corrmat.SetBinError(1, corr_mat_err[0][0])
        self.hist_corrmat.SetBinError(2, corr_mat_err[0][1])
        self.hist_corrmat.SetBinError(3, corr_mat_err[0][2])
        self.hist_corrmat.SetBinError(4, corr_mat_err[0][3])
        self.hist_corrmat.SetBinError(5, corr_mat_err[1][1])
        self.hist_corrmat.SetBinError(6, corr_mat_err[1][2])
        self.hist_corrmat.SetBinError(7, corr_mat_err[1][3])
        self.hist_corrmat.SetBinError(8, corr_mat_err[2][2])
        self.hist_corrmat.SetBinError(9, corr_mat_err[2][3])
        self.hist_corrmat.SetBinError(10, corr_mat_err[3][3])
        self.output_file.cd()
        self.hist_nkl.Write()
        self.hist_xres.Write()
        self.hist_yres.Write()
        self.hist_zres.Write()
        self.hist_tres.Write()
        self.hist_pres.Write()
        self.hist_ptres.Write()
        self.hist_ppres.Write()
        self.hist_covmat.Write()
        self.hist_corrmat.Write()
        self.output_file.Close()
