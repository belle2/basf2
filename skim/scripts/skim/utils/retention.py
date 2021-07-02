# !/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""\
Provides class for tracking retention rate of each cut in a skim.
"""

import os
import matplotlib.pyplot as plt

from ROOT import Belle2

import basf2 as b2


class RetentionCheck(b2.Module):
    """Check the retention rate and the number of candidates for a given set of particle lists.

    The module stores its results in the static variable "summary".

    To monitor the effect of every module of an initial path, this module should be added after
    each module of the path. A function was written (`skim.utils.retention.pathWithRetentionCheck`) to do it:

    >>> path = pathWithRetentionCheck(particle_lists, path)

    After the path processing, the result of the RetentionCheck can be printed with

    >>> RetentionCheck.print_results()

    or plotted with (check the corresponding documentation)

    >>> RetentionCheck.plot_retention(...)

    and the summary dictionary can be accessed through

    >>> RetentionCheck.summary

    Authors:

        Cyrille Praz, Slavomira Stefkova

    Parameters:

        module_name (str): name of the module after which the retention rate is measured
        module_number (int): index of the module after which the retention rate is measured
        particle_lists (list(str)): list of particle list names which will be tracked by the module
    """

    summary = {}  # static dictionary containing the results (retention rates, number of candidates, ...)
    output_override = None  # if the -o option is provided to basf2, this variable store the ouptut for the plotting

    def __init__(self, module_name='', module_number=0, particle_lists=None):

        if particle_lists is None:
            particle_lists = []

        self.module_name = str(module_name)
        self.module_number = int(module_number)

        self.candidate_count = {pl: 0 for pl in particle_lists}
        self.event_with_candidate_count = {pl: 0 for pl in particle_lists}

        self.particle_lists = particle_lists

        self._key = "{:04}. {}".format(int(self.module_number), str(self.module_name))
        type(self).summary[self._key] = {}

        if type(self).output_override is None:
            type(self).output_override = Belle2.Environment.Instance().getOutputFileOverride()

        super().__init__()

    def event(self):
        """"""

        for particle_list in self.particle_lists:

            pl = Belle2.PyStoreObj(Belle2.ParticleList.Class(), particle_list)

            if pl.isValid():

                self.candidate_count[particle_list] += pl.getListSize()

                if pl.getListSize() != 0:

                    self.event_with_candidate_count[particle_list] += 1

    def terminate(self):
        """"""

        N = Belle2.Environment.Instance().getNumberOfEvents()

        for particle_list in self.particle_lists:

            if N > 0:

                retention_rate = float(self.event_with_candidate_count[particle_list]) / N

            else:

                b2.B2WARNING("Belle2.Environment.Instance().getNumberOfEvents() gives 0 or less.")
                retention_rate = 0

            type(self).summary[self._key][particle_list] = {"retention_rate": retention_rate,
                                                            "#candidates": self.candidate_count[particle_list],
                                                            "#evts_with_candidates": self.event_with_candidate_count[particle_list],
                                                            "total_#events": N}

    @classmethod
    def print_results(cls):
        """ Print the results, should be called after the path processing."""
        summary_tables = {}  # one summary table per particle list
        table_headline = "{:<100}|{:>9}|{:>12}|{:>22}|{:>12}|\n"
        table_line = "{:<100}|{:>9.3f}|{:>12}|{:>22}|{:>12}|\n"

        atLeastOneEntry = {}  # check if there is at least one non-zero retention for a given particle list

        for module, module_results in cls.summary.items():

            for particle_list, list_results in module_results.items():

                if particle_list not in summary_tables.keys():

                    atLeastOneEntry[particle_list] = False

                    summary_tables[particle_list] = table_headline.format(
                        "Module", "Retention", "# Candidates", "# Evts with candidates", "Total # evts")
                    summary_tables[particle_list] += "=" * 160 + "\n"

                else:

                    if list_results["retention_rate"] > 0 or atLeastOneEntry[particle_list]:

                        atLeastOneEntry[particle_list] = True
                        if len(module) > 100:  # module name tool long
                            module = module[:96] + "..."
                        summary_tables[particle_list] += table_line.format(module, *list_results.values())

        for particle_list, summary_table in summary_tables.items():
            b2.B2INFO("\n" + "=" * 160 + "\n" +
                      "Results of the modules RetentionCheck for the list " + particle_list + ".\n" +
                      "=" * 160 + "\n" +
                      "Note: the module RetentionCheck is defined in skim/scripts/skim/utils/retention.py\n" +
                      "=" * 160 + "\n" +
                      summary_table +
                      "=" * 160 + "\n" +
                      "End of the results of the modules RetentionCheck for the list " + particle_list + ".\n" +
                      "=" * 160 + "\n"
                      )

    @classmethod
    def plot_retention(cls, particle_list, plot_title="", save_as=None, module_name_max_length=80):
        """ Plot the result of the RetentionCheck for a given particle list.

        Example of use (to be put after process(path)):

        >>> RetentionCheck.plot_retention('B+:semileptonic','skim:feiSLBplus','retention_plots/plot.pdf')

        Parameters:

            particle_list (str): particle list name
            title (str): plot title (overwritten by the -o argument in basf2)
            save_as (str): output filename (overwritten by the -o argument in basf2)
            module_name_max_length (int): if the module name length is higher than this value, do not display the full name
        """
        module_name = []
        retention = []

        at_least_one_entry = False
        for module, results in cls.summary.items():

            if particle_list not in results.keys():
                b2.B2WARNING(particle_list + " is not present in the results of the RetentionCheck for the module {}."
                             .format(module))
                return

            if results[particle_list]['retention_rate'] > 0 or at_least_one_entry:
                at_least_one_entry = True
                if len(module) > module_name_max_length and module_name_max_length > 3:  # module name tool long
                    module = module[:module_name_max_length - 3] + "..."
                module_name.append(module)
                retention.append(100 * (results[particle_list]['retention_rate']))

        if not at_least_one_entry:
            b2.B2WARNING(particle_list + " seems to have a zero retention rate when created (if created).")
            return

        plt.figure()
        bars = plt.barh(module_name, retention, label=particle_list, color=(0.67, 0.15, 0.31, 0.6))

        for bar in bars:
            yval = bar.get_width()
            plt.text(0.5, bar.get_y() + bar.get_height() / 2.0 + 0.1, str(round(yval, 3)))

        plt.gca().invert_yaxis()
        plt.xticks(rotation=45)
        plt.xlim(0, 100)
        plt.axvline(x=10.0, linewidth=1, linestyle="--", color='k', alpha=0.5)
        plt.xlabel('Retention Rate [%]')
        plt.legend(loc='lower right')

        if save_as or cls.output_override:
            if cls.output_override:
                plot_title = (cls.output_override).split(".")[0]
                save_as = plot_title + '.pdf'
            if '/' in save_as:
                os.makedirs(os.path.dirname(save_as), exist_ok=True)
            plt.title(plot_title)
            plt.savefig(save_as, bbox_inches="tight")
            b2.B2RESULT("Retention rate results for list {} saved in {}."
                        .format(particle_list, os.getcwd() + "/" + save_as))


def pathWithRetentionCheck(particle_lists, path):
    """ Return a new path with the module RetentionCheck inserted between each module of a given path.

    This allows for checking how the retention rate is modified by each module of the path.

    Example of use (to be put just before process(path)):

    >>> path = pathWithRetentionCheck(['B+:semileptonic'], path)

    Warning: pathWithRetentionCheck(['B+:semileptonic'], path) does not modify path,
    it only returns a new one.

    After the path processing, the result of the RetentionCheck can be printed with

    >>> RetentionCheck.print_results()

    or plotted with (check the corresponding documentation)

    >>> RetentionCheck.plot_retention(...)

    and the summary dictionary can be accessed through

    >>> RetentionCheck.summary

    Parameters:

        particle_lists (list(str)): list of particle list names which will be tracked by RetentionCheck
        path (basf2.Path): initial path (it is not modified, see warning above and example of use)
    """
    new_path = b2.Path()
    for module_number, module in enumerate(path.modules()):
        new_path.add_module(module)
        if 'ParticleSelector' in module.name():
            name = module.name() + '(' + module.available_params()[0].values + ')'  # get the cut string
        else:
            name = module.name()
        new_path.add_module(RetentionCheck(name, module_number, particle_lists))
    return new_path
