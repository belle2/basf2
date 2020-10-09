#!/usr/bin/env python3

import basf2_mva
import pandas as pd
from root_pandas import to_root, read_root
import numpy as np
from matplotlib import pyplot as plt


def train_mva_method(file_name):
    weight_file = "weightfile.root"

    general_options = basf2_mva.GeneralOptions()
    general_options.m_datafiles = basf2_mva.vector(file_name)
    general_options.m_treename = "tree"
    general_options.m_identifier = weight_file
    general_options.m_variables = basf2_mva.vector('A', 'B')
    general_options.m_target_variable = 'C'

    regression_fastbdt_options = basf2_mva.RegressionFastBDTOptions()
    # You can set the regression specific settings here
    regression_fastbdt_options.setMaximalBinNumber(20)
    # or the options specific to the base classifier
    #   base_options = regression_fastbdt_options.getBaseClassifierOptions()
    #   ...

    basf2_mva.teacher(general_options, regression_fastbdt_options)

    return weight_file


def create_random_data():
    file_name = "data.root"

    # We generate a very simple example dataset: A and B are random and the target C is the average of A and B
    df = pd.DataFrame({"A": np.random.rand(1000), "B": np.random.rand(1000)})
    df["C"] = (df.A + df.B) / 2

    to_root(df, file_name, store_index=False, key="tree")
    return file_name


def apply_expert(file_name, weight_file):
    output_file = 'expert.root'
    basf2_mva.expert(basf2_mva.vector(weight_file), basf2_mva.vector(file_name), 'tree', output_file)
    return output_file


def create_plot(expert_file):
    df = read_root(expert_file)
    df.plot.scatter("weightfile__ptroot_C", "weightfile__ptroot", ax=plt.gca())
    plt.xlabel("Correct")
    plt.ylabel("Output")
    plt.savefig("result.pdf")


if __name__ == "__main__":
    from basf2 import conditions
    # NOTE: do not use testing payloads in production! Any results obtained like this WILL NOT BE PUBLISHED
    conditions.testing_payloads = [
        'localdb/database.txt'
    ]

    # Lets create some random data
    file_name = create_random_data()

    # Train a regression MVA method
    weight_file = train_mva_method(file_name)

    # Apply the trained methods on data
    expert_file = apply_expert(file_name, weight_file)

    # And generate an example plot
    create_plot(expert_file)
