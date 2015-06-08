import basf2

from ROOT import gSystem
import ROOT

import numpy as np

import tracking.validation.classification as classification
from tracking.validation.utilities import root_cd

import logging


def get_logger():
    return logging.getLogger(__name__)


def get_truth_name(variable_names):
    """Selects the shortest variable name form containing the 'truth'."""
    truth_names = [name for name in variable_names if "truth" in name]

    # select the shortest
    try:
        truth_name = min(truth_names, key=len)
    except ValueError:
        raise ValueError("variable_names='%s' does not contain a truth variable" % variable_names)
    else:
        return truth_name


class FastBDTClassifier(object):

    @staticmethod
    def load_plugins(name):
        """Helper function to setup the fast boosted decision tree in TMVA"""
        base = "TMVA@@MethodBase"
        regexp1 = ".*_" + str(name) + ".*"
        regexp2 = ".*" + str(name) + ".*"
        class_name = "TMVA::Method" + str(name)
        plugin_name = "TMVA" + str(name)
        ctor1 = "Method" + str(name) + "(DataSetInfo&,TString)"
        ctor2 = "Method" + str(name) + "(TString&,TString&,DataSetInfo&,TString&)"
        ROOT.gPluginMgr.AddHandler(base, regexp1, class_name, plugin_name, ctor1)
        ROOT.gPluginMgr.AddHandler(base, regexp2, class_name, plugin_name, ctor2)

    def __init__(self,
                 name="FastBDT",
                 output_file_name=None,
                 truth=None,
                 exclude=[],
                 select=[],
                 replace_nan=None):
        """Setup a fast boosted decision tree for classification from TMVA.

        Parameters
        ----------
        name : str
            A name for the decision tree
        output_file_name : str, optional
            Name of the output file to which the trained expertise is written
            Default is the name postfixed by .root
        truth :  str, optional
            Name of the branch carrying the classification target
            Defaults to the shortest branch name containing "truth"
        exclude : list(str), optional
            Branch names to be excluded from the training.
            Defaults to excluding nothing.
        select : list(str)
            Branch names to be used for training.
            Defaults to selecting all.
        replace_nan : float
            Finite value with which to replace non-finit values (typically -999)
            Defaults to no replacement.
        """

        self.load_plugins("FastBDT")

        #: A name for the decision tree
        self.name = name

        #: Name of the output file to which the trained expertise is written
        self.output_file_name = output_file_name or name + ".root"

        #: Branch names to be excluded from the training.
        self.exclude = exclude

        #: Branch names to be used for training.
        self.select = select

        #: Name of the branch carrying the classification target.
        self.truth = truth

        #: Finite value with which to replace non-finit values (typically -999)
        self.replace_nan = replace_nan

    def train(self, input_tree):
        """Train the boosted decision tree to model to classification target"""

        if isinstance(self.output_file_name, str):
            output_file = ROOT.TFile(self.output_file_name, "RECREATE")
        else:
            output_file = self.output_file_name

        factory = ROOT.TMVA.Factory(self.name, output_file, "!V:!Silent:Transformations=I")
        factory.AddSignalTree(input_tree)
        factory.AddBackgroundTree(input_tree)

        column_names = [leave.GetName() for leave in input_tree.GetListOfLeaves()]

        tree_name = input_tree.GetName()

        truth = self.truth

        if truth is None:
            truth = "truth"

        if truth not in column_names:
            raise KeyError("Truth column {truth} not in tree {tree}".format(truth=truth, tree=tree_name))

        variable_names = [name for name in column_names if name != truth]

        exclude = self.exclude
        select = self.select

        if select:
            variable_names = [name for name in variable_names if name in select]

        if exclude:
            variable_names = [name for name in variable_names if name not in exclude]

        # Remove the variables that have Monte Carlo truth information unless explicitly selected
        variable_names = [name for name
                          in variable_names
                          if "truth" not in name or name in select]

        if "weight" in variable_names and "weight" not in select:
            variable_names.remove("weight")

        for variable_name in variable_names:
            if self.replace_nan:
                replace_nan = self.replace_nan
                variable_expression = "TMath::Finite({0})?{0}:{1}".format(variable_name, replace_nan)
            else:
                variable_expression = variable_name

            factory.AddVariable(variable_expression)

        if "weight" in column_names and "weight" not in exclude:
            get_logger().info("Setting weight column")
            factory.SetWeightExpression("weight")

        cut_signal = "{truth}!=0.0".format(truth=truth)
        cut_background = "!({cut_signal})".format(cut_signal=cut_signal)
        factory.PrepareTrainingAndTestTree(ROOT.TCut(cut_signal), ROOT.TCut(cut_background), "SplitMode=Random")

        factory.BookMethod(
            ROOT.TMVA.Types.kPlugins,
            "FastBDT",
            "!H:!V:NTrees=100:Shrinkage=0.15:RandRatio=0.5:NCutLevel=8:IgnoreNegWeightsInTraining",
        )

        factory.TrainAllMethods()
        factory.TestAllMethods()
        factory.EvaluateAllMethods()

        if isinstance(self.output_file_name, str):
            output_file.Close()


class ClassificationOverview:

    """Class to generate overview plots for the classification power of various variables from a TTree.

    In order to get an overview, which variables from a diverse set, generated from a recording filter
    or some other sort of validation, perform well in classification task.

    """

    def __init__(self, output_file_name, truth_name=None, select=[], exclude=[]):
        self.output_file_name = output_file_name
        self.truth_name = truth_name
        self.select = select
        self.exclude = exclude

        self.classification_analyses = []

    def train(self, input_tree):
        """Main method feed with a TTree containing the truth variable and the variables to be investigated.

        Branches that contain "truth" in the name are considered to directly contain information about
        true classification target and are not analysed here.

        Args:
            input_tree (ROOT.TTree) : Tree containing the variables to be investigated
                as well as the classification target.
            truth_name (str, optional) : Name of the branch of the classification target.
                If not given the Branch with the shortest name containing "truth" is selected.
        """

        if isinstance(self.output_file_name, str):
            output_file = ROOT.TFile(self.output_file_name, "RECREATE")
        else:
            output_file = self.output_file_name

        output_file.cd()

        column_names = [leave.GetName() for leave in input_tree.GetListOfLeaves()]

        tree_name = input_tree.GetName()

        truth_name = self.truth_name

        if truth_name is None:
            truth_name = get_truth_name(column_names)

        if truth_name not in column_names:
            raise KeyError("Truth column {truth} not in tree {tree}".format(truth=truth_name,
                                                                            tree=tree_name))
        variable_names = [name for name in column_names if name != truth_name]

        exclude = self.exclude
        select = self.select

        if select:
            variable_names = [name for name in variable_names if name in select]

        if exclude:
            variable_names = [name for name in variable_names if name not in exclude]

        # Remove the variables that have Monte Carlo truth information unless explicitly selected
        variable_names = [name for name
                          in variable_names
                          if "truth" not in name or name in select]

        print "Truth name", truth_name
        print "Variable names", variable_names

        import root_numpy
        print "Loading tree"
        branch_names = variable_names + [truth_name]
        input_record_array = root_numpy.tree2rec(input_tree, branches=branch_names)
        print "Loaded tree"
        truths = input_record_array[truth_name]

        for variable_name in variable_names:
            print 'Analyse', variable_name

            # Get the truths as a numpy array
            estimates = input_record_array[variable_name]

            classification_analysis = classification.ClassificationAnalysis(
                contact="",
                quantity_name=variable_name,
                outlier_z_score=5.0,
                allow_discrete=True,
            )
            classification_analysis.analyse(estimates, truths)

            with root_cd(variable_name) as tdirectory:
                classification_analysis.write(tdirectory)

            self.classification_analyses.append(classification_analysis)

        if isinstance(self.output_file_name, str):
            output_file.Close()

        print "Saved overviews completely"
