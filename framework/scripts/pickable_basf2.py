#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
Pickable basf2
Import this module at the top of your steering file to get a (more) pickable version of basf2.
This is useful if you want to pickle the path using --dump-path and execute it later using --execute-path.
Usually all calls to functions in basf2 like use_central_database are forgotten if you only save the path.
With this module, these functions are executed again if you execute the pickled path using --execute-path.

Technically this works by recording all calls to basf2 functions.
If you have other code which should be pickled as well you can wrap it in the make_code_pickable function.

If you want to exclude some functions, delete them from this module using
del pickable_basf2.functionname
"""


import sys
import pickle
import inspect
import unittest.mock as mock
sys.modules['original_basf2'] = sys.modules['basf2']  # noqa
import original_basf2


class BASF2StateRecorder:
    """ Drop-in replacement of the basf2 module, which keeps track of all functions calls """

    def __getattr__(self, name):
        """ Return attribute with the given name in the basf2 namespace """
        return getattr(original_basf2, name)


#: instance of the state recorder
basf2_state_recorder = BASF2StateRecorder()
#: basic unittest Mock object to attach fake functions to
manager = mock.Mock()


def process(path, max_event=0):
    """ Process call which pickles the recorded state in addition to the path """
    sys.modules['basf2'] = original_basf2
    original_basf2.process(path, max_event)
    state = list(map(tuple, manager.mock_calls))
    pickle_path = original_basf2.get_pickle_path()
    print("Path", path)
    print("State", state)
    if pickle_path != '' and path is not None:
        serialized = original_basf2.serialize_path(path)
        serialized['state'] = state
        pickle.dump(serialized, open(pickle_path, 'bw'))
    sys.modules['basf2'] = basf2_state_recorder


for name, x in original_basf2.__dict__.items():
    # We record function and fake Boost.Python.function objects
    if inspect.isfunction(x) or isinstance(x, type(original_basf2.find_file)):
        #: mock object to simulate the original function but record calls and arguments
        mock_x = mock.Mock(x, side_effect=x)
        manager.attach_mock(mock_x, name)
        setattr(basf2_state_recorder, name, mock_x)
    # Other names we have to set as well, because otherwise they won't be important by from basf2 import *
    else:
        setattr(basf2_state_recorder, name, x)

#: set the process function to serialize/deserialize the basf2 state correctly
basf2_state_recorder.process = process
# and replace the module
sys.modules['basf2'] = basf2_state_recorder
sys.modules['pickable_basf2'] = basf2_state_recorder
