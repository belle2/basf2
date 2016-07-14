#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
root_pandas
-----------

Provides a simple interface to convert between ROOT trees and panda data frames
"""

from root_numpy import root2array, list_trees, list_branches, list_structures, array2root, tree2array
import os
import pandas as pd
import numpy as np
import ROOT


def _get_pandas_branches(filename, tree, t, branches):
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

    usable_column_types = ["int", "float", "double", "char", "long", "long long"]

    structure = list_structures(filename, tree)

    if branches is None or len(branches) == 0:
        used_branches = structure
    else:
        used_branches = branches
    all_numerical_branches = [branch_name for branch_name in used_branches if structure[branch_name][0][1] in usable_column_types]
    if len(all_numerical_branches) == 0:
        raise ValueError("Could not find any numerical branch in the tree %s. Can not export this tree." % tree)

    # Filter out the branches which are not one dimensional in the first column. We assume here that the root tree looks
    # the same on each row.
    first_row = tree2array(t, branches=all_numerical_branches, start=0, stop=1, step=1)[0]
    good_indices = [x[0] for x in [element_with_index for element_with_index in enumerate(first_row)
                                   if element_with_index[1].size == 1]]
    good_branches = [all_numerical_branches[b] for b in good_indices]
    if len(good_branches) == 0:
        raise ValueError(
            "Could not find any numerical branch which is one-dimensional in the tree %s. Can not export this tree." %
            tree)
    return good_branches


class RootReader():

    """ An interator class to read out root files in chunks of data. See read_root for usage"""

    def __init__(self, root_file, tree, branches, chunksize):
        """ Construcor with the filename, the tree and the branches (for reference) and the chunksize """

        #: Pointer to the root file (we must keep it because otherwise python cleans it up)
        self.root_file = root_file

        #: TTree to handle
        self.tree = tree

        #: List of branches to handle
        self.branches = branches

        #: Chunksize (in lines) each chunk of retuned data should have
        self.chunksize = chunksize

        #: Counter for the current chunk
        self.current_chunk = 0

    def __iter__(self):
        """ Iterator """
        return self

    def __next__(self):
        """ Python 3 iterator """
        return next(self)

    def __next__(self):
        """ The next function """
        root_array = tree2array(self.tree, self.branches,
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
    root_file = ROOT.TFile(filename)
    for tree in trees:
        t = root_file.Get(tree)
        good_branches = _get_pandas_branches(filename=filename, tree=tree, t=t, branches=branches)

        if chunksize is not None:
            result_dataframes.update({tree: RootReader(root_file, t, good_branches, chunksize)})
        else:
            root_array = tree2array(t, good_branches)
            dataframe = pd.DataFrame(root_array)
            result_dataframes.update({tree: dataframe})

    if len(result_dataframes) == 1:
        return list(result_dataframes.values())[0]
    else:
        return result_dataframes


def to_root(df, path, tree_key="default", mode='w', *kargs, **kwargs):
    """
    Write DataFrame to a ROOT file.
    Coded after the root_pandas project on github.

    Arguments
    ----------
    path: string
        File path to new ROOT file (will be overwritten)
    tree_key: string
        Name of tree that the DataFrame will be saved as
    mode: string, {'w', 'a'}
        Mode that the file should be opened in (default: 'w')

    Notes
    -----
    Further *kargs and *kwargs are passed to root_numpy's array2root.
    >>> df = DataFrame({'x': [1,2,3], 'y': [4,5,6]})
    >>> df.to_root('test.root')

    The DataFrame index will be saved as a branch called 'index'.
    """

    if mode == 'a':
        mode = 'update'
    elif mode == 'w':
        mode = 'recreate'
    else:
        raise ValueError('Unknown mode: {}. Must be "a" or "w".'.format(mode))

    arr = df.to_records()
    array2root(arr, path, tree_key, mode=mode, *kargs, **kwargs)
