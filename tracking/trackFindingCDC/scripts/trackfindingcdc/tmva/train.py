import basf2

from ROOT import gSystem
import ROOT


import logging


def get_logger():
    return logging.getLogger(__name__)


class FastBDTClassifier(object):

    @staticmethod
    def load_plugins(name):
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
                 select=[]):

        self.output_file_name = output_file_name or name + ".root"
        self.name = name
        self.load_plugins("FastBDT")

        self.exclude = exclude
        self.select = select
        self.truth = truth

    def train(self, input_tree):

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
                          if "truth" in not name or name in select]

        if "weight" in variable_names and "weight" in not select:
            variable_names.remove("weight")

        for variable_name in variable_names:
            factory.AddVariable(variable_name)

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


if __name__ == "__main__":
    main()
