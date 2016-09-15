#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Thomas Keck 2016

import basf2_mva

import tempfile

import numpy as np

import ROOT
from ROOT import Belle2
from ROOT import gSystem

import shutil


def tree2dict(tree, tree_columns, dict_columns=None):
    if dict_columns is None:
        dict_columns = tree_columns
    d = {column: np.zeros((tree.GetEntries(),)) for column in dict_columns}
    for iEvent, event in enumerate(tree):
        for dict_column, tree_column in zip(dict_columns, tree_columns):
            d[dict_column][iEvent] = getattr(event, tree_column)
    return d


def calculate_roc_auc(p, t):
    N = len(t)
    T = np.sum(t)
    index = np.argsort(p)
    efficiency = (T - np.cumsum(t[index])) / float(T)
    purity = (T - np.cumsum(t[index])) / (N - np.cumsum(np.ones(N)))
    purity = np.where(np.isnan(purity), 0, purity)
    return np.abs(np.trapz(purity, efficiency))


class Method(object):
    def __init__(self, identifier):

        self.identifier = identifier

        self.weightfile = basf2_mva.Weightfile.load(self.identifier)
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

        if self.general_options.m_method == "FastBDT":
            self.specific_options = basf2_mva.FastBDTOptions()
        elif self.general_options.m_method == "NeuroBayes":
            self.specific_options = basf2_mva.NeuroBayesOptions()
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
        elif self.general_options.m_method == "Trivial":
            self.specific_options = basf2_mva.TrivialOptions()
        else:
            raise RuntimeError("Unkown method " + self.general_options.m_method)

        self.specific_options.load(self.weightfile.getXMLTree())

        variables = [str(v) for v in self.general_options.m_variables]
        importances = self.weightfile.getFeatureImportance()
        self.importances = {k: importances[k] for k in variables}

        self.variables = list(sorted(variables, key=lambda v: self.importances.get(v, 0.0)))
        self.root_variables = [Belle2.makeROOTCompatible(v) for v in self.variables]
        self.root_importances = {k: importances[k] for k in self.root_variables}

        self.description = str(basf2_mva.info(self.identifier))

    def train_teacher(self, datafiles, treename, general_options=None, specific_options=None):
        if general_options is None:
            general_options = self.general_options
        if specific_options is None:
            specific_options = self.specific_options

        tempdir = tempfile.mkdtemp()
        identifier = tempdir + "weightfile.xml"

        general_options.m_datafiles = basf2_mva.vector(*datafiles)
        general_options.m_weightfile = identifier

        basf2_mva.teacher(general_options, specific_options)

        method = Method(identifier)
        shutil.rmtree(tempdir)
        return method

    def apply_expert(self, datafiles, treename):
        tempdir = tempfile.mkdtemp()

        rootfilename = tempdir + '/expert.root'
        basf2_mva.expert(basf2_mva.vector(self.identifier),
                         basf2_mva.vector(*datafiles),
                         treename,
                         rootfilename)
        rootfile = ROOT.TFile(rootfilename, "UPDATE")
        roottree = rootfile.Get("variables")

        expert_probability = self.identifier
        expert_target = self.identifier + '_' + self.general_options.m_target_variable
        d = tree2dict(roottree,
                      [Belle2.makeROOTCompatible(expert_probability), Belle2.makeROOTCompatible(expert_target)],
                      [expert_probability, expert_target])
        shutil.rmtree(tempdir)
        return d[expert_probability], d[expert_target]
