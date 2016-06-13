#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Jochen Gemmler 2016

"""
    Generates the theano function from an pylearn2 model.
"""

import pickle
import sys
import theano as th

if len(sys.argv) != 3:
    print("Usage: Pylearn2-Model Theano-Function-Output-Name")

file_name = sys.argv[1]
output_name = sys.argv[2]

with open(file_name, 'rb') as f:
    f_model = pickle.load(f)

th_x = f_model.get_input_space().make_theano_batch()
th_y = f_model.fprop(th_x)
th_f = th.function([th_x], th_y)

with open(output_name, 'wb') as f:
    pickle.dump(th_f, f)
