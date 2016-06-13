#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# minor modification of root-pandas
# original
# (https://github.com/ibab/root_pandas) by Igor Babuschkin
# Copyright (c) 2015 Igor Babuschkin

# Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
# documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
# rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
# permit persons to whom the Software is furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included in all copies or substantial portions of
# the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO
# THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
# TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

# modified by Jochen Gemmler 2016

# return numpy-array instead of a dataframe (less memory consuming)
# this might be necessary if large root files are required as single numpy array


from __future__ import division
from __future__ import print_function

from fnmatch import fnmatch
from math import ceil
import re
import itertools

from root_numpy import root2array, list_trees
from root_numpy import list_branches
from root_numpy.extern.six import string_types
import ROOT


def expand_braces(orig):
    r = r'.*(\{.+?[^\\]\})'
    p = re.compile(r)

    s = orig[:]
    res = list()

    m = p.search(s)
    if m is not None:
        sub = m.group(1)
        open_brace = s.find(sub)
        close_brace = open_brace + len(sub) - 1
        if sub.find(',') != -1:
            for pat in sub.strip('{}').split(','):
                res.extend(expand_braces(s[:open_brace] + pat + s[close_brace + 1:]))

        else:
            res.extend(expand_braces(s[:open_brace] + sub.replace('}', '\\}') + s[close_brace + 1:]))

    else:
        res.append(s.replace('\\}', '}'))

    return list(set(res))


def get_matching_variables(branches, patterns, fail=True):

    selected = []

    for p in patterns:
        found = False
        for b in branches:
            if fnmatch(b, p):
                found = True
            if fnmatch(b, p) and b not in selected:
                selected.append(b)
        if not found and fail:
            raise ValueError("Pattern '{}' didn't match any branch".format(p))
    return selected


def read_root_to_array(
        path,
        tree_key=None,
        columns=None,
        ignore=None,
        chunksize=None,
        where=None,
        start=None,
        stop=None,
        *kargs,
        **kwargs):
    """
    Read a ROOT file into a numpy structured array.
    Further *kargs and *kwargs are passed to root_numpy's root2array.
    If the root file contains a branch called index, it will become the DataFrame's index.

    Parameters
    ----------
    path: string
        The path to the root file
    tree_key: string
        The key of the tree to load.
    columns: str or sequence of str
        A sequence of shell-patterns (can contain *, ?, [] or {}). Matching columns are read.
    ignore: str or sequence of str
        A sequence of shell-patterns (can contain *, ?, [] or {}). All matching columns are ignored
        (overriding the columns argument)
    chunksize: int
        If this parameter is specified, an iterator is returned that yields DataFrames with `chunksize` rows
    where: str
        Only rows that match the expression will be read

    Returns
    -------
        Structured Numpy Array created from matching data in the specified TTree

    Notes
    -----

        >>> arr = read_root_to_array('test.root', 'MyTree', columns=['A{B,C}*', 'D'], where='ABB > 100')

    """
    if not tree_key:
        branches = list_trees(path)
        if len(branches) == 1:
            tree_key = branches[0]
        else:
            raise ValueError('More than one tree found in {}'.format(path))

    branches = list_branches(path, tree_key)

    if not columns:
        all_vars = branches
    else:
        # index is always loaded if it exists
        if isinstance(columns, string_types):
            columns = [columns]
        if 'index' in branches:
            columns = columns[:]
            columns.append('index')
        columns = list(itertools.chain.from_iterable(list(map(expand_braces, columns))))
        all_vars = get_matching_variables(branches, columns)

    if ignore:
        if isinstance(ignore, string_types):
            ignore = [ignore]
        ignored = get_matching_variables(branches, ignore, fail=False)
        ignored = list(itertools.chain.from_iterable(list(map(expand_braces, ignored))))
        if 'index' in ignored:
            raise ValueError('index variable is being ignored!')
        for var in ignored:
            if var not in all_vars:
                continue
            all_vars.remove(var)

    if chunksize:
        f = ROOT.TFile(path)
        n_entries = f.Get(tree_key).GetEntries()
        f.Close()

        def genchunks():
            for chunk in range(int(ceil(float(n_entries) / chunksize))):
                arr = root2array(
                    path,
                    tree_key,
                    all_vars,
                    start=chunk *
                    chunksize,
                    stop=(
                        chunk +
                        1) *
                    chunksize,
                    selection=where,
                    *
                    kargs,
                    **kwargs)
                yield arr
        return genchunks()

    arr = root2array(path, tree_key, all_vars, selection=where, start=start, stop=stop, *kargs, **kwargs)
    return arr
