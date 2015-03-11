#!/usr/bin/env python
# -*- coding: utf-8 -*-

from tracking.validation.plot import ValidationPlot, compose_axis_label
from tracking.validation.fom import ValidationFiguresOfMerit

import tracking.validation.scores as scores

# get error function as a np.ufunc vectorised for numpy array
from tracking.validation.utilities import erf, root_save_name

import math
import collections

import numpy as np


class ClassificationAnalysis(object):

    def __init__(
        self,
        contact,
        prediction_name,
    ):
        """Performs a comparision of an estimated quantity to their truths by generating standardized validation plots."""

        self._contact = contact
        self.prediction_name = prediction_name

        self.histogram = None
        self.fom = None

    def analyse(
        self,
        predictions,
        truths
    ):
        """Compares the concrete estimate to the truth and efficiency, purity and background rejection
        as figure of merit and plots the selection as a stacked plot over the truths.

        Parameters
        ----------
        predictions : array_like
            Selection variable to compare to the truths
        truths : array_like
            Binary true class values.
        """

        prediction_name = self.prediction_name

        plot_name = "{prediction_name}_classification_histogram".format(
            prediction_name=prediction_name
        )

        classification_histogram = ValidationPlot(plot_name)
        classification_histogram.hist(
            predictions,
            stackby=truths,
        )

        fom_name = "{prediction_name}_classification_figures_of_merits".format(
            prediction_name=prediction_name
        )

        fom_description = "Efficiency, purity and background rejection of the classifiction with {prediction_name}".format(
            prediction_name=prediction_name
        )

        fom_check = "Check that the classifcation quality stays stable."

        fom_title = "Summary of the classification quality with {prediction_name}".format(
            prediction_name=prediction_name
        )

        classification_fom = ValidationFiguresOfMerit(
            name=fom_name,
            title=fom_title,
            description=fom_description,
            check=fom_check,
            contact=self.contact,
        )

        efficiency = scores.efficiency(truths, predictions)
        purity = scores.purity(truths, predictions)
        background_rejection = scores.background_rejection(truths, predictions)

        classification_fom['efficiency'] = efficiency
        classification_fom['purity'] = purity
        classification_fom['background_rejection'] = background_rejection

        classification_histogram.add_stats_entry('eff.', efficiency)
        classification_histogram.add_stats_entry('pur.', purity)
        classification_histogram.add_stats_entry('bkg. rej.', background_rejection)

        self.histogram = classification_histogram
        self.fom = classification_fom

    @property
    def contact(self):
        return self._contact

    @contact.setter
    def contact(self, contact):
        self._contact = contact

        if self.histogram:
            self.histogram.contact = contact

        if self.fom:
            self.fom.contact = contact

    def write(self, tdirectory=None):
        if self.histogram:
            self.histogram.write(tdirectory)

        if self.fom:
            self.fom.write(tdirectory)
