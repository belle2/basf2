#!/usr/bin/env python3

import ROOT


def add_properties(object, dictionary):
    """ Adds metaoptions to an object. """
    for key, value in dictionary.items():
        object.GetListOfFunctions().Add(ROOT.TNamed(key, value))
