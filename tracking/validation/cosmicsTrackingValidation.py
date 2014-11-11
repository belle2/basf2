#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
<header>
  <contact>oliver.frost@desy.de</contact>
  <output>cosmicsTrackingValidation.root</output>
  <description>This module validates that track finding is capable of reconstructing tracks in cosmics run.</description>
</header>
"""

VALIDATION_OUTPUT_FILE = 'cosmicsTrackingValidation.root'
CONTACT = 'oliver.frost@desy.de'
N_EVENTS = 100000

import basf2

import os
import sys
import logging

import matplotlib.pyplot as plt

from cdclocaltracking import event_generation, cdcdisplay

import ROOT
from ROOT import Belle2


class ValidationPlot(object):

    def __init__(
        self,
        name,
        n_bins=None,
        lower_bound=None,
        upper_bound=None,
        description='',
        check='',
        contact='',
        ):

        self.name = name

        self.upper_bound = upper_bound
        self.lower_bound = lower_bound
        self.n_bins = n_bins

        self.description = description
        self.check = check
        self.contact = contact

        self._xlabel = ''
        self._ylabel = ''
        self._title = ''

        self.histogram = None

    def fill(self, xs, ys=None):
        # Handle data content of the histgram
        name = self.name

        if self.lower_bound is None:
            self.lower_bound = min(xs)
        lower_bound = self.lower_bound

        if self.upper_bound is None:
            self.upper_bound = max(xs)
        upper_bound = self.upper_bound

        if self.n_bins is None:
            n_data = len(x)
            rice_n_bins = ceil(2.0 * pow(n_data, 1.0 / 3.0))
            self.n_bins = rice_n_bins
        n_bins = self.n_bins

        if ys is not None:
            histogram = ROOT.TProfile(name, '', self.n_bins, lower_bound,
                                      upper_bound)
            for (x, y) in zip(xs, ys):
                histogram.Fill(float(x), float(y))

            max_y = max(ys)
            min_y = min(ys)
            y_range = max_y - min_y

            histogram.SetMaximum(max_y + 0.1 * y_range)
            histogram.SetMinimum(min_y)
        else:

            histogram = ROOT.TH1F(name, '', self.n_bins, lower_bound,
                                  upper_bound)
            for x in xs:
                histogram.Fill(float(x))

        # Now handle to style of the histogram
        root_description = ROOT.TNamed('Descrition', self.description)
        root_check = ROOT.TNamed('Check', self.check)
        root_contact = ROOT.TNamed('Contact', self.contact)

        for root_attribute in (root_description, root_check, root_contact):
            histogram.GetListOfFunctions().Add(root_attribute)

        title = (self.title if self.title else self.name)
        histogram.SetTitle(title)

        xlabel = self.xlabel
        histogram.SetXTitle(xlabel)

        ylabel = self.ylabel
        if ylabel is None:
            if ys is None:
                # Default ylabel for a histogram plot
                ylabel = 'Count'
        histogram.SetYTitle(ylabel)

        self.histogram = histogram

    @property
    def title(self):
        return self._title

    @title.setter
    def _(self, title):
        self._title = title
        if self.histogram is not None:
            self.histogram.SetTitle(title)

    @property
    def xlabel(self):
        return self._xlabel

    @xlabel.setter
    def xlabel(self, xlabel):
        self._xlabel = xlabel
        if self.histogram is not None:
            self.histogram.SetXTitle(xlabel)

    @property
    def ylabel(self):
        return self._ylabel

    @ylabel.setter
    def ylabel(self, ylabel):
        self._ylabel = ylabel
        if self.histogram is not None:
            self.histogram.SetYTitle(ylabel)

    def draw(self):
        self.histogram.Draw()

    def write(self):
        self.histogram.Write()


class ValidationFiguresOfMerit(dict):

    def __init__(
        self,
        name,
        description='',
        check='',
        contact='',
        ):

        self.name = name
        self.description = description
        self.check = check
        self.contact = contact

        self.figures_by_name = {}

    def __str__(self):
        """Informal sting output listing the assigned figures of merit."""

        return '\n'.join('%s : %s' % (key, value) for (key, value) in
                         self.figures_by_name.items())

    def write(self):
        """Writes the figures of merit as a TNtuple to the currently open TFile in the format complient with the validation frame work."""

        name = self.name
        figure_names = list(self.figures_by_name.keys())
        values = list(self.figures_by_name.values())

        leaf_specification = ':'.join(figure_names)
        title = ''
        ntuple = ROOT.TNtuple(name, title, leaf_specification)
        ntuple.Fill(*values)

        ntuple.SetAlias('Description', self.description)
        ntuple.SetAlias('Check', self.check)
        ntuple.SetAlias('Contact', self.contact)

        ntuple.Write()

    def __setitem__(self, figure_name, value):
        """Braketed item assignement for figures of merit"""

        self.figures_by_name[figure_name] = value

    def __getitem__(self, figure_name):
        """Braketed item lookup for figures of merit"""

        return self.figures_by_name[figure_name]

    def __delitem__(self, figure_name):
        """Braketed item deletion for figures of merit"""

        del self.figures_by_name[figure_name]

    def __iter__(self):
        """Implements the iter() hook as if it was a dictionary."""

        return iter(self.figures_by_name)

    def __len__(self):
        """Returns the number of figures of merit assigned. Implements the len() hook."""

        return len(self.figures_by_name)


# Analysis module #
###################

class CosmicsTrackingValidationModule(basf2.Module):

    """Module to collect matching information about the found particles and to generate validation plots and figures of merit on the performance of track finding."""

    def initialize(self):
        self.trackMatchLookUp = Belle2.TrackMatchLookUp('MCTrackCands')

        self.isMatchedPRTracks = []

        self.isMatchedMCTracks = []
        self.mcAbsTransversMomentums = []

    def event(self):
        """Event method"""

        trackMatchLookUp = self.trackMatchLookUp

        trackCands = Belle2.PyStoreArray('TrackCands')
        if trackCands:
            print 'Number of pattern recognition tracks', \
                trackCands.getEntries()

            for (iTrackCand, trackCand) in enumerate(trackCands):
                isMatched = trackMatchLookUp.isMatchedPRTrackCand(trackCand)
                self.isMatchedPRTracks.append(isMatched)

        mcTrackCands = Belle2.PyStoreArray('MCTrackCands')
        if trackCands:
            print 'Number of Monte Carlo tracks', mcTrackCands.getEntries()

            for mcTrackCand in mcTrackCands:
                isMatched = trackMatchLookUp.isMatchedMCTrackCand(mcTrackCand)
                self.isMatchedMCTracks.append(isMatched)

                mcParticle = trackMatchLookUp.getRelatedMCParticle(mcTrackCand)
                momentum = mcParticle.getMomentum()
                absTransversMomentum = momentum.Perp()
                self.mcAbsTransversMomentums.append(absTransversMomentum)

    def terminate(self):
        track_finding_efficiency = 1.0 * sum(self.isMatchedMCTracks) \
            / len(self.isMatchedMCTracks)

        fake_rate = 1.0 - 1.0 * sum(self.isMatchedPRTracks) \
            / len(self.isMatchedPRTracks)

        cosmics_figures_of_merit = \
            ValidationFiguresOfMerit('Cosmics_track_finding_efficiency')
        cosmics_figures_of_merit['finding_efficiency'] = \
            track_finding_efficiency
        cosmics_figures_of_merit['fake_rate'] = fake_rate
        cosmics_figures_of_merit.description = \
            """
finding_efficiency - the ratio of matched Monte Carlo tracks to all Monte Carlo tracks <br/>
fake_rate - ratio of unmatched (clone, background, ghost) pattern recognition track to all pattern recognition tracks
"""
        cosmics_figures_of_merit.check = \
            'The efficiency should be well above 0.9. The fake rate should be well below 0.1'
        cosmics_figures_of_merit.contact = CONTACT

        efficiency_profile = \
            ValidationPlot('Cosmics_track_finding_efficiency_by_momentum',
                           n_bins=50)
        efficiency_profile.fill(self.mcAbsTransversMomentums,
                                self.isMatchedMCTracks)
        efficiency_profile.xlabel = 'Momentum (GeV)'
        efficiency_profile.xlabel = 'Efficiency'
        efficiency_profile.description = 'Not a serious plot yet.'
        efficiency_profile.check = ''
        efficiency_profile.contact = CONTACT
        efficiency_profile.draw()

        output_file = ROOT.TFile(VALIDATION_OUTPUT_FILE, 'recreate')
        cosmics_figures_of_merit.write()
        efficiency_profile.write()
        output_file.Close()


def main():
    """Function to be executed during the validation run"""

    main_path = event_generation.create_simulate_events_path('cosmics',
            n_events=N_EVENTS)

    # Setup track finder
    trackFinderModule = basf2.register_module('CDCLocalTracking')
    main_path.add_module(trackFinderModule)

    # Reference Monte Carlo tracks
    trackFinderMCTruthModule = basf2.register_module('TrackFinderMCTruth')
    trackFinderMCTruthModule.param({
        'UseCDCHits': True,
        'UseSVDHits': False,
        'UsePXDHits': False,
        'WhichParticles': ['primary'],
        'EnergyCut': 0.1,
        'GFTrackCandidatesColName': 'MCTrackCands',
        })
    main_path.add_module(trackFinderMCTruthModule)

    # Track matcher
    mcTrackMatcherModule = basf2.register_module('MCTrackMatcher')
    mcTrackMatcherModule.param({
        'UseCDCHits': True,
        'UseSVDHits': False,
        'UsePXDHits': False,
        'MCGFTrackCandsColName': 'MCTrackCands',
        'MinimalPurity': 0.66,
        'RelateClonesToMCParticles': True,
        })

    main_path.add_module(mcTrackMatcherModule)
    main_path.add_module(CosmicsTrackingValidationModule())

    print 'Start processing'

    basf2.process(main_path)
    print basf2.statistics


if __name__ == '__main__':
    main()

