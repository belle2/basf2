#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Thomas Keck 2016

import basf2_mva

import tempfile

import numpy as np

import ROOT
from ROOT import Belle2


def tree2dict(tree, tree_columns, dict_columns=None):
    """
    Convert a ROOT.TTree into a dictionary of np.arrays
    @param tree the ROOT.TTree
    @param tree_columns the column (or branch) names in the tree
    @param dict_columns the corresponding column names in the dictionary
    """
    if len(tree_columns) == 0:
        return dict()
    if dict_columns is None:
        dict_columns = tree_columns
    try:
        import root_numpy
        d = root_numpy.tree2array(tree, branches=tree_columns)
        d.dtype.names = dict_columns
    except ImportError:
        d = {column: np.zeros((tree.GetEntries(),)) for column in dict_columns}
        for iEvent, event in enumerate(tree):
            for dict_column, tree_column in zip(dict_columns, tree_columns):
                d[dict_column][iEvent] = getattr(event, tree_column)
    return d


def calculate_roc_auc(p, t):
    """
    Calculates the area under the receiver oeprating characteristic curve (AUC ROC)
    @param p np.array filled with the probability output of a classifier
    @param t np.array filled with the target (0 or 1)
    """
    N = len(t)
    T = np.sum(t)
    index = np.argsort(p)
    efficiency = (T - np.cumsum(t[index])) / float(T)
    purity = (T - np.cumsum(t[index])) / (N - np.cumsum(np.ones(N)))
    purity = np.where(np.isnan(purity), 0, purity)
    return np.abs(np.trapz(purity, efficiency))


def calculate_flatness(f, p, w=None):
    """
    Calculates the flatness of a feature under cuts on a signal probability
    @param f the feature values
    @param p the probability values
    @param w optional weights
    @return the mean standard deviation between the local and global cut selection efficiency
    """
    quantiles = list(range(101))
    binning_feature = np.unique(np.percentile(f, q=quantiles))
    binning_probability = np.unique(np.percentile(p, q=quantiles))
    if len(binning_feature) < 2:
        binning_feature = np.array([np.min(f) - 1, np.max(f) + 1])
    if len(binning_probability) < 2:
        binning_probability = np.array([np.min(p) - 1, np.max(p) + 1])
    hist_n, _ = np.histogramdd(np.c_[p, f],
                               bins=[binning_probability, binning_feature],
                               weights=w)
    hist_inc = hist_n.sum(axis=1)
    hist_inc /= hist_inc.sum(axis=0)
    hist_n /= hist_n.sum(axis=0)
    hist_n = hist_n.cumsum(axis=0)
    hist_inc = hist_inc.cumsum(axis=0)
    diff = (hist_n.T - hist_inc)**2
    return np.sqrt(diff.sum() / (100 * 99))


class Method(object):
    """
    Wrapper class providing an interface to the method stored under the given identifier.
    It loads the Options, can apply the expert and train new ones using the current as a prototype.
    This class is used by the basf_mva_evaluation tools
    """

    def __init__(self, identifier):
        """
        Load a method stored under the given identifier
        @param identifier identifying the method
        """
        #: Identifier of the method
        self.identifier = identifier
        #: Weightfile of the method
        self.weightfile = basf2_mva.Weightfile.load(self.identifier)
        #: General options of the method
        self.general_options = basf2_mva.GeneralOptions()
        self.general_options.load(self.weightfile.getXMLTree())

        # This piece of code should be correct but leads to random segmentation faults
        # inside python, llvm or pyroot, therefore we use the more dirty code below
        # Ideas why this is happening:
        # 1. Ownership of the unique_ptr returned by getOptions()
        # 2. Some kind of object slicing, although pyroot identifies the correct type
        # 3. Bug in pyroot
        # interfaces = basf2_mva.AbstractInterface.getSupportedInterfaces()
        # self.interface = interfaces[self.general_options.m_method]
        # self.specific_options = self.interface.getOptions()

        #: Specific options of the method
        self.specific_options = None
        if self.general_options.m_method == "FastBDT":
            self.specific_options = basf2_mva.FastBDTOptions()
        elif self.general_options.m_method == "TMVAClassification":
            self.specific_options = basf2_mva.TMVAOptionsClassification()
        elif self.general_options.m_method == "TMVARegression":
            self.specific_options = basf2_mva.TMVAOptionsRegression()
        elif self.general_options.m_method == "FANN":
            self.specific_options = basf2_mva.FANNOptions()
        elif self.general_options.m_method == "Python":
            self.specific_options = basf2_mva.PythonOptions()
        elif self.general_options.m_method == "PDF":
            self.specific_options = basf2_mva.PDFOptions()
        elif self.general_options.m_method == "Combination":
            self.specific_options = basf2_mva.CombinationOptions()
        elif self.general_options.m_method == "Reweighter":
            self.specific_options = basf2_mva.ReweighterOptions()
        elif self.general_options.m_method == "Trivial":
            self.specific_options = basf2_mva.TrivialOptions()
        else:
            raise RuntimeError("Unkown method " + self.general_options.m_method)

        self.specific_options.load(self.weightfile.getXMLTree())

        variables = [str(v) for v in self.general_options.m_variables]
        importances = self.weightfile.getFeatureImportance()

        #: Dictionary of the variable importances calculated by the mehtod
        self.importances = {k: importances[k] for k in variables}
        #: List of variables sorted by their importance
        self.variables = list(sorted(variables, key=lambda v: self.importances.get(v, 0.0)))
        #: List of the variable importances calculated by the method, but with the root compatible variable names
        self.root_variables = [Belle2.makeROOTCompatible(v) for v in self.variables]
        #: Dictionary of the variables sorted by their importance but with root compatoble variable names
        self.root_importances = {k: importances[k] for k in self.root_variables}
        #: Description of the method as a xml string returned by basf2_mva.info
        self.description = str(basf2_mva.info(self.identifier))
        #: List of spectators
        self.spectators = [str(v) for v in self.general_options.m_spectators]
        #: List of spectators with root compatible names
        self.root_spectators = [Belle2.makeROOTCompatible(v) for v in self.spectators]

    def train_teacher(self, datafiles, treename, general_options=None, specific_options=None):
        """
        Train a new method using this method as a prototype
        @param datafiles the training datafiles
        @param treename the name of the tree containing the training data
        @param general_options general options given to basf2_mva.teacher (if None the options of this method are used)
        @param specific_options specific options given to basf2_mva.teacher (if None the options of this method are used)
        """
        if isinstance(datafiles, str):
            datafiles = [datafiles]
        if general_options is None:
            general_options = self.general_options
        if specific_options is None:
            specific_options = self.specific_options

        with tempfile.TemporaryDirectory() as tempdir:
            identifier = tempdir + "/weightfile.xml"

            general_options.m_datafiles = basf2_mva.vector(*datafiles)
            general_options.m_identifier = identifier

            basf2_mva.teacher(general_options, specific_options)

            method = Method(identifier)
        return method

    def apply_expert(self, datafiles, treename):
        """
        Apply the expert of the method to data and return the calculated probability and the target
        @param datafiles the datafiles
        @param treename the name of the tree containing the data
        """
        if isinstance(datafiles, str):
            datafiles = [datafiles]
        with tempfile.TemporaryDirectory() as tempdir:
            identifier = tempdir + "/weightfile.xml"
            basf2_mva.Weightfile.save(self.weightfile, identifier)

            rootfilename = tempdir + '/expert.root'
            basf2_mva.expert(basf2_mva.vector(identifier),
                             basf2_mva.vector(*datafiles),
                             treename,
                             rootfilename)
            rootfile = ROOT.TFile(rootfilename, "UPDATE")
            roottree = rootfile.Get("variables")

            expert_target = identifier + '_' + self.general_options.m_target_variable
            stripped_expert_target = self.identifier + '_' + self.general_options.m_target_variable
            d = tree2dict(roottree,
                          [Belle2.makeROOTCompatible(identifier), Belle2.makeROOTCompatible(expert_target)],
                          [self.identifier, stripped_expert_target])
        return d[self.identifier], d[stripped_expert_target]
