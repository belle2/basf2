#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# Contributors: Ludovico Massaccesi
#
# Copyright 2021 Belle II Collaboration
#
# Utilities to add SVD DQM modules to a path.


def add_svd_dqm_dose(path, offlineZSShaperDigits, **params):
    """Adds the six default instances of SVDDQMDose to the given path.

    The argument offlineZSShaperDigits shall be the name of the
    collection of digits to be used by the module (usually it will be
    zero-suppressed digits with SVDZeroSuppressionEmulator and
    SNthreshold=5).

    Extra parameters will be passed to all instances of SVDDQMDose.
    """
    path.add_module(
        'SVDDQMDose', offlineZSShaperDigits=offlineZSShaperDigits, eventTypeFilter=1,
        histogramDirectoryName="SVDDoseHERInjPois", **params).set_name("SVDDQMDose_HERInjPois")
    path.add_module(
        'SVDDQMDose', offlineZSShaperDigits=offlineZSShaperDigits, eventTypeFilter=2,
        histogramDirectoryName="SVDDoseLERInjPois", **params).set_name("SVDDQMDose_LERInjPois")
    path.add_module(
        'SVDDQMDose', offlineZSShaperDigits=offlineZSShaperDigits, eventTypeFilter=4,
        histogramDirectoryName="SVDDoseNoInjPois", **params).set_name("SVDDQMDose_NoInjPois")
    params['trgTypes'] = []
    path.add_module(
        'SVDDQMDose', offlineZSShaperDigits=offlineZSShaperDigits, eventTypeFilter=1,
        histogramDirectoryName="SVDDoseHERInjAll", **params).set_name("SVDDQMDose_HERInjAll")
    path.add_module(
        'SVDDQMDose', offlineZSShaperDigits=offlineZSShaperDigits, eventTypeFilter=2,
        histogramDirectoryName="SVDDoseLERInjAll", **params).set_name("SVDDQMDose_LERInjAll")
    path.add_module(
        'SVDDQMDose', offlineZSShaperDigits=offlineZSShaperDigits, eventTypeFilter=4,
        histogramDirectoryName="SVDDoseNoInjAll", **params).set_name("SVDDQMDose_NoInjAll")
