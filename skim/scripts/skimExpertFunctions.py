# !/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Expert functions
~~~~~~~~~~~~~~~~

Some helper functions to do common tasks relating to skims.
Like testing, and for skim name encoding(decoding).
"""
from basf2 import create_path, register_module, LogLevel, B2ERROR, AfterConditionPath
from basf2 import Path, B2WARNING, B2INFO, B2RESULT
from mdst import add_mdst_output
from modularAnalysis import removeParticlesNotInLists, skimOutputUdst, summaryOfLists
from skim.registry import skim_registry
import skimTestFilesInfo
import os
import sys
import inspect
import subprocess
import json
import matplotlib.pyplot as plt
from ROOT import Belle2
# For channels in fei skim
# from fei import Particle, MVAConfiguration, PreCutConfiguration, PostCutConfiguration


_all_skims = [name for _, name in skim_registry]


def encodeSkimName(skimScriptName):
    """
    Returns the appropriate 8 digit skim code that will be used as the output uDST
    file name for any give name of a skimming script.

    :param str skimScriptName: Name of the skim.
    """
    lookup_dict = {n: c for c, n in skim_registry}
    if skimScriptName not in lookup_dict:
        B2ERROR("Skim Unknown. Please add your skim to skimExpertFunctions.py.")
    return lookup_dict[skimScriptName]


def decodeSkimName(skimCode):
    """
    Returns the appropriate name of the skim given a specific skim code. This is useful to determine the skim script used
    to produce a specific uDST file, given the 8-digit code  name of the file itself.

    :param str code:
    """
    lookup_dict = {c: n for c, n in skim_registry}
    if skimCode not in lookup_dict:
        B2ERROR("Code Unknown. Please add your skim to skimExpertFunctions.py")
    return lookup_dict[skimCode]


def get_test_file(sample, skimCampaign):
    """
    Returns the KEKcc location of files used specifically for skim testing

    Arguments:
        sample: Type of MC sample: charged mixed ccbar uubar ddbar ssbar taupair or other of-resonance samples.
        skimCampaign: MC9, MC10, MC11, etc..
    """
    sampleName = skimCampaign + '_' + sample
    lookup_dict = {s: f for s, f in skimTestFilesInfo.kekcc_locations}
    if sampleName not in lookup_dict:
        B2ERROR("Testing file for this sample and skim campaign is not available.")
    return lookup_dict[sampleName]


def get_total_infiles(sample, skimCampaign):
    """
    Returns the total number of input Mdst files for a given sample. This is useful for resource estimate.
    Arguments:
        sample: Type of MC sample: charged mixed ccbar uubar ddbar ssbar taupair or other of-resonance samples.
        skimCampaign: MC9, MC10, MC11, etc..
    """
    sampleName = skimCampaign + '_' + sample
    lookup_dict = {s: f for s, f in skimTestFilesInfo.total_input_files}
    if sampleName not in lookup_dict:
        return 1000
    return lookup_dict[sampleName]


def add_skim(label, lists, path):
    """
    create uDST skim for given lists, saving into $label.udst.root
    Particles not necessary for the given particle lists are not saved.

    Parameters:
        label (str): the registered skim name
        lists (list(str)): the list of ParticleList names that have been created by a skim list builder function
        path (basf2.Path): modules are added to this path

    """
    skimCode = encodeSkimName(label)
    skimOutputUdst(skimCode, lists, path=path)
    summaryOfLists(lists, path=path)


def setSkimLogging(path, additional_modules=[]):
    """
    Turns the log level to ERROR for  several modules to decrease
    the total size of the skim log files

    Parameters:
        skim_path (basf2.Path): modules are added to this path
        additional_modules (list(str)): an optional list of extra noisy module
            names that should be silenced
    """
    noisy_modules = ['ParticleLoader', 'ParticleVertexFitter'] + additional_modules
    for module in path.modules():
        if module.type() in noisy_modules:
            module.set_log_level(LogLevel.ERROR)
    return


def ifEventPasses(cut, conditional_path, path):
    """
    If the event passes the given ``cut`` proceed to process everything in ``conditional_path``.
    Afterwards return here and continue processing with the next module.

    Arguments:
        cut (str): selection criteria which needs to be fulfilled in order to continue with ``conditional_path``
        conditional_path (basf2.Path): path to execute if the event fulfills the criteria ``cut``
        path (basf2.Path): modules are added to this path
    """
    eselect = path.add_module("VariableToReturnValue", variable=f"passesEventCut({cut})")
    eselect.if_value('>=1', conditional_path, AfterConditionPath.CONTINUE)


def get_eventN(fileName):
    """
    Returns the number of events in a specific file

    Arguments:
     filename: Name of the file as clearly indicated in the argument's name.
    """

    process = subprocess.Popen(['b2file-metadata-show', '--json', fileName], stdout=subprocess.PIPE)
    out = process.communicate()[0]
    if process.returncode == 0:
        metadata = json.loads(out)
        nevents = metadata['nEvents']
        return nevents
    else:
        B2ERROR("FILE INVALID OR NOT FOUND.")


def skimOutputMdst(skimDecayMode, path=None, skimParticleLists=[], outputParticleLists=[], includeArrays=[], *,
                   outputFile=None, dataDescription=None):
    """
    Create a new path for events that contain a non-empty particle list specified via skimParticleLists.
    Write the accepted events as a mdst file, saving only particles from skimParticleLists
    and from outputParticleLists. It outputs a .mdst file.
    Additional Store Arrays and Relations to be stored can be specified via includeArrays
    list argument.

    :param str skimDecayMode: Name of the skim. If no outputFile is given this is
        also the name of the output filename. This name will be added to the
        FileMetaData as an extra data description "skimDecayMode"
    :param list(str) skimParticleLists: Names of the particle lists to skim for.
        An event will be accepted if at least one of the particle lists is not empty
    :param list(str) outputParticleLists: Names of the particle lists to store in
        the output in addition to the ones in skimParticleLists
    :param list(str) includeArrays: datastore arrays/objects to write to the output
        file in addition to mdst and particle information
    :param basf2.Path path: Path to add the skim output to. Defaults to the default analysis path
    :param str outputFile: Name of the output file if different from the skim name
    :param dict dataDescription: Additional data descriptions to add to the output file. For example {"mcEventType":"mixed"}
    """

    # if no outputfile is specified, set it to the skim name
    if outputFile is None:
        outputFile = skimDecayMode

    # make sure the output filename has the correct extension
    if not outputFile.endswith(".mdst.root"):
        outputFile += ".mdst.root"

    skimfilter = register_module('SkimFilter')
    skimfilter.set_name('SkimFilter_' + skimDecayMode)
    skimfilter.param('particleLists', skimParticleLists)
    path.add_module(skimfilter)
    filter_path = create_path()
    skimfilter.if_value('=1', filter_path, AfterConditionPath.CONTINUE)

    # add_independent_path() is rather expensive, only do this for skimmed events
    skim_path = create_path()
    saveParticleLists = skimParticleLists + outputParticleLists
    removeParticlesNotInLists(saveParticleLists, path=skim_path)

    # set dataDescription: dictionary is mutable and thus not a good
    # default argument.
    if dataDescription is None:
        dataDescription = {}

    dataDescription.setdefault("skimDecayMode", skimDecayMode)
    add_mdst_output(outputFile, path)
    filter_path.add_independent_path(skim_path, "skim_" + skimDecayMode)


class RetentionCheck(Module):
    """Check the retention rate and the number of candidates for a given set of particle lists.

    The module stores its results in the static variable "summary".

    To monitor the effect of every module of an initial path, this module should be added after
    each module of the path. A function was written (`skimExpertFunctions.pathWithRetentionCheck`) to do it:

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

        module_name -- name of the module after which the retention rate is measured
        module_number -- index of the module after which the retention rate is measured
        particle_lists -- list of particle list names which will be tracked by the module
    """

    summary = {}  # static dictionary containing the results (retention rates, number of candidates, ...)
    output_override = None  # if the -o option is provided to basf2, this variable store the ouptut for the plotting

    def __init__(self, module_name='', module_number=0, particle_lists=[]):

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

        for particle_list in self.particle_lists:

            pl = Belle2.PyStoreObj(Belle2.ParticleList.Class(), particle_list)

            if pl.isValid():

                self.candidate_count[particle_list] += pl.getListSize()

                if pl.getListSize() != 0:

                    self.event_with_candidate_count[particle_list] += 1

    def terminate(self):

        for particle_list in self.particle_lists:

            retention_rate = float(self.event_with_candidate_count[particle_list]) / \
                Belle2.Environment.Instance().getNumberOfEvents()

            type(self).summary[self._key][particle_list] = {"retention_rate": retention_rate,
                                                            "#candidates": self.candidate_count[particle_list],
                                                            "#evts_with_candidates": self.event_with_candidate_count[particle_list],
                                                            "total_#events": Belle2.Environment.Instance().getNumberOfEvents()}

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
                            module = module[:96]+"..."
                        summary_tables[particle_list] += table_line.format(module, *list_results.values())

        for particle_list, summary_table in summary_tables.items():
            B2INFO("\n" + "=" * 160 + "\n" +
                   "Results of the modules RetentionCheck for the list " + particle_list + ".\n" +
                   "=" * 160 + "\n" +
                   "Note: the module RetentionCheck is defined in skim/scripts/skimExpertFunctions.py\n" +
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

            particle_lists -- particle list name
            title -- plot title (overwritten by the -o argument in basf2)
            save_as -- output filename (overwritten by the -o argument in basf2)
            module_name_max_length -- if the module name length is higher than this value, do not display the full name
        """
        module_name = []
        retention = []

        at_least_one_entry = False
        for module, results in cls.summary.items():

            if particle_list not in results.keys():
                B2WARNING(particle_list+" is not present in the results of the RetentionCheck for the module {}."
                          .format(module))
                return

            if results[particle_list]['retention_rate'] > 0 or at_least_one_entry:
                at_least_one_entry = True
                if len(module) > module_name_max_length and module_name_max_length > 3:  # module name tool long
                    module = module[:module_name_max_length-3]+"..."
                module_name.append(module)
                retention.append(100*(results[particle_list]['retention_rate']))

        if not at_least_one_entry:
            B2WARNING(particle_list+" seems to have a zero retention rate when created (if created).")
            return

        plt.figure()
        bars = plt.barh(module_name, retention, label=particle_list, color=(0.67, 0.15, 0.31, 0.6))

        for bar in bars:
            yval = bar.get_width()
            plt.text(0.5, bar.get_y()+bar.get_height()/2.0+0.1, str(round(yval, 3)))

        plt.gca().invert_yaxis()
        plt.xticks(rotation=45)
        plt.xlim(0, 100)
        plt.axvline(x=10.0, linewidth=1, linestyle="--", color='k', alpha=0.5)
        plt.xlabel('Retention Rate [%]')
        plt.legend(loc='lower right')

        if save_as or cls.output_override:
            if cls.output_override:
                plot_title = (cls.output_override).split(".")[0]
                save_as = plot_title+'.pdf'
            if '/' in save_as:
                os.makedirs(os.path.dirname(save_as), exist_ok=True)
            plt.title(plot_title)
            plt.savefig(save_as, bbox_inches="tight")
            B2RESULT("Retention rate results for list {} saved in {}."
                     .format(particle_list, os.getcwd()+"/"+save_as))


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

        particle_lists -- list of particle list names which will be tracked by RetentionCheck
        path -- initial path (it is not modified, see warning above and example of use)
    """
    new_path = Path()
    for module_number, module in enumerate(path.modules()):
        new_path.add_module(module)
        if 'ParticleSelector' in module.name():
            name = module.name()+'('+module.available_params()[0].values+')'  # get the cut string
        else:
            name = module.name()
        new_path.add_module(RetentionCheck(name, module_number, particle_lists))
    return new_path
