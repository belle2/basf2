from root_numpy import root2array, list_trees, list_branches, list_structures
import os
import pandas as pd
import numpy as np


def _get_pandas_branches(filename, tree, branches):
    """
    Get all branches which are int or float from the given tree in the given filename.
    If branches is given, use only those branches.
    Filter out any moredimensional branches.

    Arguments
    ---------

    filename: The filename of the root file
    tree: The tree name in the root file
    branches: Ane preselection for branches (set branches to None if you want to use all branches)

    TODO: Are there more possibilities for "wrong" branches?
    """

    usable_column_types = [u"int", u"float", u"double", u"char"]

    structure = list_structures(filename, tree)
    if branches is None or len(branches) == 0:
        used_branches = structure
    else:
        used_branches = branches
    all_numerical_branches = filter(lambda branch_name: structure[branch_name][0][1] in usable_column_types, used_branches)
    if len(all_numerical_branches) == 0:
        raise ValueError("Could not find any numerical branch in the tree %s. Can not export this tree." % tree)

    # Filter out the branches which are not one dimensional in the first column. We assume here that the root tree looks
    # the same on each row.
    first_row = root2array(filenames=filename, treename=tree, branches=all_numerical_branches, start=0, stop=1, step=1)[0]
    good_indices = map(lambda x: x[0], filter(lambda element_with_index: element_with_index[1].size == 1, enumerate(first_row)))
    good_branches = [all_numerical_branches[b] for b in good_indices]
    if len(good_branches) == 0:
        raise ValueError(
            "Could not find any numerical branch which is one-dimensional in the tree %s. Can not export this tree." %
            tree)
    return good_branches


class RootReader():

    """ An interator class to read out root files in chunks of data. See read_root for usage"""

    def __init__(self, filename, tree, branches, chunksize):
        self.filename = filename
        self.tree = tree
        self.branches = branches
        self.chunksize = chunksize
        self.current_chunk = 0

    def __iter__(self):
        """ Iterator """
        return self

    def __next__(self):
        """ Python 3 iterator """
        return self.next()

    def next(self):
        """ The next function """
        root_array = root2array(self.filename, self.tree, self.branches,
                                start=self.current_chunk * self.chunksize, stop=(self.current_chunk + 1) * self.chunksize, step=1)
        if len(root_array) == 0:
            raise StopIteration()
        self.current_chunk += 1
        dataframe = pd.DataFrame(root_array)
        return dataframe


def read_root(filename, branches=None, tree_key=None, chunksize=None):
    """
    Export the given root file to a pandas dataframe.
    Coded after the root_pandas project on github.

    Arguments
    ---------
    If tree_key is not None use only the given tree -
       else use all trees and return a dict tree_name->dataframe if the number of trees os bigger than one.
    If branches is not None use only the given branches (if they are numerical and one dimensional), else use all branches.
    If chunksize is not None return a ReadRoot-object - an iterator - which can be uses to receive the data in chunks.
    """

    if not os.path.exists(filename):
        raise IOError("File %s does not exists." % filename)

    if tree_key is not None:
        trees = [tree_key]
    else:
        trees = list_trees(filename)
    result_dataframes = {}
    for tree in trees:
        good_branches = _get_pandas_branches(filename=filename, tree=tree, branches=branches)

        if chunksize is not None:
            result_dataframes.update({tree: RootReader(filename, tree, good_branches, chunksize)})
        else:
            root_array = root2array(filename, tree, good_branches)
            dataframe = pd.DataFrame(root_array)
            result_dataframes.update({tree: dataframe})

    if len(result_dataframes) == 1:
        return result_dataframes.values()[0]
    else:
        return result_dataframes
