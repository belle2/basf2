#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import set_random_seed, create_path, Module
from ROOT import Belle2
from b2test_utils import safe_process, clean_working_directory

set_random_seed("something important")


class MakeRelations(Module):
    '''Simple module that creates some relations.'''

    def initialize(self):
        '''Initialize.'''
        #: Tracks
        self.tracks = Belle2.PyStoreArray('Tracks')
        #: TOPLikelihoods
        self.likelihoods = Belle2.PyStoreArray('TOPLikelihoods')
        self.tracks.registerInDataStore()
        self.likelihoods.registerInDataStore()
        self.tracks.registerRelationTo(self.likelihoods)
        #: Bool for flagging the first event processed.
        self.first = True

    def event(self):
        '''Event.'''
        if not self.first:
            track = self.tracks.appendNew()
            likelihood = self.likelihoods.appendNew()
            track.addRelationTo(likelihood)
        self.first = False


def create_file():
    """Create file with empty first event"""
    path = create_path()
    path.add_module('EventInfoSetter')
    path.add_module(MakeRelations())
    path.add_module('RootOutput', outputFileName='test.root')
    safe_process(path, 2)


def read_file():
    """Read file with empty first event"""
    path = create_path()
    path.add_module('RootInput', inputFileName='test.root')
    path.add_module('PrintCollections', printForEvent=0)
    safe_process(path)


if __name__ == "__main__":
    with clean_working_directory():
        create_file()
        read_file()
