#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
<header>
  <contact>software-tracking@belle2.org</contact>
  <input>CosmicsExtrapolation.root</input>
  <output>CosmicsExtrapolationPlots.root</output>
  <description>Validation of cosmic track extrapolation.</description>
</header>
"""


import basf2
import ROOT
from ROOT import Belle2, TNamed

contact = 'Kirill Chilikin (chilikin@lebedev.ru)'

bklm_numbers = Belle2.BKLMElementNumbers()
eklm_numbers = Belle2.EKLMElementNumbers.Instance()


class CosmicsExtapolationPlotModule(basf2.Module):
    """ Class for creation of cosmics extrapolation plot module. """

    def set_options_coordinate(self, histogram, description, shifter):
        """ Set optiions for coordinate plot. """
        histogram.SetXTitle('cm')
        histogram.SetYTitle('Events')
        function_list = histogram.GetListOfFunctions()
        function_list.Add(TNamed('Description', description))
        function_list.Add(TNamed('Check', ' No bias, no large background, resolution ~ 2 cm.'))
        function_list.Add(TNamed('Contact', contact))
        if shifter:
            basf2.B2WARNING("temporarily removed all shifter plots for the CosmicsExtrapolationPlots")
            # function_list.Add(TNamed('MetaOptions', 'shifter'))

    def set_options_momentum(self, histogram, description):
        """ Set options for momentum plot. """
        histogram.SetXTitle('p [GeV]')
        histogram.SetYTitle('Events')
        function_list = histogram.GetListOfFunctions()
        function_list.Add(TNamed('Description', description))
        function_list.Add(TNamed('Check', ' No bias, no large background, resolution ~ 2 cm.'))
        function_list.Add(TNamed('Contact', contact))

    def __init__(self):
        """Initialization."""
        super().__init__()

        #: Output file name.
        self.output_file = ROOT.TFile('CosmicsExtrapolationPlots.root',
                                      'recreate')

        #: X resolution histogram (forward propagation, mu-).
        self.hist_xres_forward_mum = \
            ROOT.TH1F('xres_forward_mum',
                      'X resolution (#mu^{-}, forward)',
                      100, -20, 20)
        self.set_options_coordinate(
            self.hist_xres_forward_mum,
            'Extrapolated hit X resolution (mu-, forward propagation).', True)

        #: X resolution histogram (forward propagation, mu+).
        self.hist_xres_forward_mup = \
            ROOT.TH1F('xres_forward_mup',
                      'X resolution (#mu^{+}, forward)',
                      100, -20, 20)
        self.set_options_coordinate(
            self.hist_xres_forward_mup,
            'Extrapolated hit X resolution (mu+, forward propagation).', True)

        #: Y resolution histogram (forward propagation, mu-).
        self.hist_yres_forward_mum = \
            ROOT.TH1F('yres_forward_mum',
                      'Y resolution (#mu^{-}, forward)',
                      100, -20, 20)
        self.set_options_coordinate(
            self.hist_yres_forward_mum,
            'Extrapolated hit Y resolution (mu-, forward propagation).', False)

        #: Y resolution histogram (forward propagation, mu+).
        self.hist_yres_forward_mup = \
            ROOT.TH1F('yres_forward_mup',
                      'Y resolution (#mu^{+}, forward)',
                      100, -20, 20)
        self.set_options_coordinate(
            self.hist_yres_forward_mup,
            'Extrapolated hit Y resolution (mu+, forward propagation).', False)

        #: Z resolution histogram (forward propagation, mu-).
        self.hist_zres_forward_mum = \
            ROOT.TH1F('zres_forward_mum',
                      'Z resolution (#mu^{-}, forward)',
                      100, -20, 20)
        self.set_options_coordinate(
            self.hist_zres_forward_mum,
            'Extrapolated hit Z resolution (mu-, forward propagation).', False)

        #: Z resolution histogram (forward propagation, mu+).
        self.hist_zres_forward_mup = \
            ROOT.TH1F('zres_forward_mup',
                      'Z resolution (#mu^{+}, forward)',
                      100, -20, 20)
        self.set_options_coordinate(
            self.hist_zres_forward_mup,
            'Extrapolated hit Z resolution (mu+, forward propagation).', False)

        #: X resolution histogram (backward propagation, mu-).
        self.hist_xres_backward_mum = \
            ROOT.TH1F('xres_backward_mum',
                      'X resolution (#mu^{-}, backward)',
                      100, -20, 20)
        self.set_options_coordinate(
            self.hist_xres_backward_mum,
            'Extrapolated hit X resolution (mu-, backward propagation).', True)

        #: X resolution histogram (backward propagation, mu+).
        self.hist_xres_backward_mup = \
            ROOT.TH1F('xres_backward_mup',
                      'X resolution (#mu^{+}, backward)',
                      100, -20, 20)
        self.set_options_coordinate(
            self.hist_xres_backward_mup,
            'Extrapolated hit X resolution (mu+, backward propagation).', True)

        #: Y resolution histogram (backward propagation, mu-).
        self.hist_yres_backward_mum = \
            ROOT.TH1F('yres_backward_mum',
                      'Y resolution (#mu^{-}, backward)',
                      100, -20, 20)
        self.set_options_coordinate(
            self.hist_yres_backward_mum,
            'Extrapolated hit Y resolution (mu-, backward propagation).', False)

        #: Y resolution histogram (backward propagation, mu+).
        self.hist_yres_backward_mup = \
            ROOT.TH1F('yres_backward_mup',
                      'Y resolution (#mu^{+}, backward)',
                      100, -20, 20)
        self.set_options_coordinate(
            self.hist_yres_backward_mup,
            'Extrapolated hit Y resolution (mu+, backward propagation).', False)

        #: Z resolution histogram (backward propagation, mu-).
        self.hist_zres_backward_mum = \
            ROOT.TH1F('zres_backward_mum',
                      'Z resolution (#mu^{-}, backward)',
                      100, -20, 20)
        self.set_options_coordinate(
            self.hist_zres_backward_mum,
            'Extrapolated hit Z resolution (mu-, backward propagation).', False)

        #: Z resolution histogram (backward propagation, mu+).
        self.hist_zres_backward_mup = \
            ROOT.TH1F('zres_backward_mup',
                      'Z resolution (#mu^{+}, backward)',
                      100, -20, 20)
        self.set_options_coordinate(
            self.hist_zres_backward_mup,
            'Extrapolated hit Z resolution (mu+, backward propagation).', False)

        #: Momentum resolution histogram (forward propagation, mu-).
        self.hist_pres_forward_mum = \
            ROOT.TH1F('pres_forward_mum',
                      'Momentum resolution (#mu^{-}, forward)',
                      100, -10, 10)
        self.set_options_momentum(
            self.hist_pres_forward_mum,
            'Momentum resolution (mu-, forward propagation).')

        #: Momentum resolution histogram (forward propagation, mu+).
        self.hist_pres_forward_mup = \
            ROOT.TH1F('pres_forward_mup',
                      'Momentum resolution (#mu^{+}, forward)',
                      100, -10, 10)
        self.set_options_momentum(
            self.hist_pres_forward_mup,
            'Momentum resolution (mu+, forward propagation).')

        #: Momentum resolution histograms (backward propagation, mu-).
        self.hist_pres_backward_mum = \
            ROOT.TH1F('pres_backward_mum',
                      'Momentum resolution (#mu^{-}, backward)',
                      100, -10, 10)
        self.set_options_momentum(
            self.hist_pres_backward_mum,
            'Momentum resolution (mu-, backward propagation).')

        #: Momentum resolution histograms (backward propagation, mu+).
        self.hist_pres_backward_mup = \
            ROOT.TH1F('pres_backward_mup',
                      'Momentum resolution (#mu^{+}, backward)',
                      100, -10, 10)
        self.set_options_momentum(
            self.hist_pres_backward_mup,
            'Momentum resolution (mu+, backward propagation).')

    def fill_histograms_exthit(self, exthit, klmhit2d):
        """ Fill histograms with ExtHit data. """
        ext_position = exthit.getPosition()
        if exthit.isBackwardPropagated():
            if exthit.getPdgCode() == 13:
                self.hist_xres_backward_mum.Fill(
                    ext_position.X() - klmhit2d.getPositionX())
                self.hist_yres_backward_mum.Fill(
                    ext_position.Y() - klmhit2d.getPositionY())
                self.hist_zres_backward_mum.Fill(
                    ext_position.Z() - klmhit2d.getPositionZ())
            elif exthit.getPdgCode() == -13:
                self.hist_xres_backward_mup.Fill(
                    ext_position.X() - klmhit2d.getPositionX())
                self.hist_yres_backward_mup.Fill(
                    ext_position.Y() - klmhit2d.getPositionY())
                self.hist_zres_backward_mup.Fill(
                    ext_position.Z() - klmhit2d.getPositionZ())
        else:
            if exthit.getPdgCode() == 13:
                self.hist_xres_forward_mum.Fill(
                    ext_position.X() - klmhit2d.getPositionX())
                self.hist_yres_forward_mum.Fill(
                    ext_position.Y() - klmhit2d.getPositionY())
                self.hist_zres_forward_mum.Fill(
                    ext_position.Z() - klmhit2d.getPositionZ())
            elif exthit.getPdgCode() == -13:
                self.hist_xres_forward_mup.Fill(
                    ext_position.X() - klmhit2d.getPositionX())
                self.hist_yres_forward_mup.Fill(
                    ext_position.Y() - klmhit2d.getPositionY())
                self.hist_zres_forward_mup.Fill(
                    ext_position.Z() - klmhit2d.getPositionZ())

    def event(self):
        """ Event function. """
        klmhit2ds = Belle2.PyStoreArray('KLMHit2ds')
        exthits = Belle2.PyStoreArray('ExtHits')
        tracks = Belle2.PyStoreArray('Tracks')
        mcparticles = Belle2.PyStoreArray('MCParticles')

        for klmhit2d in klmhit2ds:
            subdetector = klmhit2d.getSubdetector()
            section = klmhit2d.getSection()
            sector = klmhit2d.getSector()
            layer = klmhit2d.getLayer()
            if (subdetector == Belle2.KLMElementNumbers.c_BKLM):
                for exthit in exthits:
                    if exthit.getDetectorID() != Belle2.Const.BKLM:
                        continue
                    module = exthit.getCopyID()
                    section_ext = bklm_numbers.getSectionByModule(module)
                    sector_ext = bklm_numbers.getSectorByModule(module)
                    layer_ext = bklm_numbers.getLayerByModule(module)
                    if (section_ext != section or sector_ext != sector or
                            layer_ext != layer):
                        continue
                    self.fill_histograms_exthit(exthit, klmhit2d)
            else:
                section = klmhit2d.getSection()
                sector = klmhit2d.getSector()
                layer = klmhit2d.getLayer()
                for exthit in exthits:
                    if exthit.getDetectorID() != Belle2.Const.EKLM:
                        continue
                    strip_global = exthit.getCopyID()
                    section_ext = eklm_numbers.getSectionByGlobalStrip(strip_global)
                    sector_ext = eklm_numbers.getSectorByGlobalStrip(strip_global)
                    layer_ext = eklm_numbers.getLayerByGlobalStrip(strip_global)
                    if (section_ext != section or sector_ext != sector or
                            layer_ext != layer):
                        continue
                    self.fill_histograms_exthit(exthit, klmhit2d)

        for track in tracks:
            track_fit_result = track.getTrackFitResult(Belle2.Const.muon)
            momentum = track_fit_result.getMomentum()
            track_exthits = track.getRelationsTo('ExtHits')
            if len(track_exthits) == 0:
                continue
            mc_momentum = mcparticles[0].getMomentum()
            p_diff = momentum.R() - mc_momentum.R()
            muon_found = False
            for i in range(len(track_exthits)):
                if abs(track_exthits[i].getPdgCode()) == 13:
                    exthit = track_exthits[i]
                    muon_found = True
            if not muon_found:
                continue
            if exthit.isBackwardPropagated():
                if exthit.getPdgCode() == 13:
                    self.hist_pres_backward_mum.Fill(p_diff)
                elif exthit.getPdgCode() == -13:
                    self.hist_pres_backward_mup.Fill(p_diff)
            else:
                if exthit.getPdgCode() == 13:
                    self.hist_pres_forward_mum.Fill(p_diff)
                elif exthit.getPdgCode() == -13:
                    self.hist_pres_forward_mup.Fill(p_diff)

    def terminate(self):
        """ Termination function. """
        self.output_file.cd()
        self.hist_xres_forward_mum.Write()
        self.hist_xres_forward_mup.Write()
        self.hist_yres_forward_mum.Write()
        self.hist_yres_forward_mup.Write()
        self.hist_zres_forward_mum.Write()
        self.hist_zres_forward_mup.Write()
        self.hist_xres_backward_mum.Write()
        self.hist_xres_backward_mup.Write()
        self.hist_yres_backward_mum.Write()
        self.hist_yres_backward_mup.Write()
        self.hist_zres_backward_mum.Write()
        self.hist_zres_backward_mup.Write()
        self.hist_pres_forward_mum.Write()
        self.hist_pres_forward_mup.Write()
        self.hist_pres_backward_mum.Write()
        self.hist_pres_backward_mup.Write()
        self.output_file.Close()


# Input.
root_input = basf2.register_module('RootInput')
root_input.param('inputFileName', '../CosmicsExtrapolation.root')

# Plotting.
plot = CosmicsExtapolationPlotModule()

# Create main path.
main = basf2.create_path()

# Add modules to main path
main.add_module(root_input)
main.add_module(plot)

# Run.
basf2.process(main)
