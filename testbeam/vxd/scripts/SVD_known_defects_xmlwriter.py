#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# This script creates masking files for the known defects of the SVD sensors
# It assumes the csv input files are organized in subdirectories as:
#   input_files/
#              /L3
#              /L4
#              /L5
#              /L6
# Giacomo Caria

import os
import sys
import math
import numpy as np
import time
import csv
import re
from os import listdir
from os.path import isfile, join
import codecs

import xmlwriter_tools
from xmlwriter_tools import *


def create_file():

    # do we want to write the defects as xml content ?
    write_defect_types = False

    # do we want to write an xml file with physical strip # converted to uv
    # strip # ?
    use_uv_strip_number = True

    # list of defects to be included in this mask
    list_of_good_defects = ['pinhole_dssd',
                            'metal_open_dssd',
                            'metal_short_dssd',
                            'pinhole_et',
                            'open_et',
                            'short_et']
    uv_side = ['v', 'u']
    np_side = ['Nside', 'Pside']

    # path with input files
    path = './input_files'

    # rename z files with appropriate '+z' or '-z' labels
    update_z_filenames(path)

    # open output file
    if write_defect_types:
        output = 'SVD_KnownDefects_Advanced.xml'
    else:
        output = 'SVD_KnownDefects.xml'

    f = open(output, 'w')

    # start writing on file
    print_header(f, list_of_good_defects, use_uv_strip_number)
    f.write('<SVD>\n')

    # this is used to store the name of files used to create the output file
    list_of_used_files = []

    for i in range(3, 7):  # loop on layers

        f.write('\t<layer n=\"' + str(i) + '\">\n')
        f.write('\t\t<ladder n=\"' + str(1) + '\">\n')

        # specify path to look for xml files, layer dependend atm
        path_layer = path + '/L' + str(i) + '/'

        for j in range(1, i):  # loop on sensors

            # if needed file is not found, continue
            if not is_file_in_path(i, j, path_layer):
                print('Files for layer ' + str(i) + ', sensor ' + str(j) +
                      ' (' + from_num_to_label(i, j) + '), not found. Moving on ..')
                continue

            # write sensor details
            f.write('\t\t\t<sensor n=\"' + str(j) + '\">\n')

            for k in range(0, 2):  # loop on sides

                # look up filename for this combination of layer,sensor and strip type
                filename = get_filename(i, j, k, path_layer)

                if 'nofile' in filename:  # file hasn't been found
                    nofile_str = 'File for layer ' + str(i) + ', sensor ' + str(j) +\
                        ' (' + from_num_to_label(i, j) + '), ' +\
                        np_side[k] + ', not found.'
                    print(nofile_str)
                    list_of_used_files.append(nofile_str)
                    continue

                else:  # file has been found !!

                    print('Found file: ' + filename)
                    list_of_used_files.append(filename)

                    f.write('\t\t\t\t<side side=\"' + uv_side[k] + '\">\n')

                    # print list of strips with their defects
                    print_file_content(i, j, k, filename, f,
                                       list_of_good_defects,
                                       write_defect_types, use_uv_strip_number)

                    # write final lines
                    f.write('\t\t\t\t</side>\n')
            f.write('\t\t\t</sensor>\n')

        f.write('\t\t</ladder>\n')
        f.write('\t</layer>\n')
    f.write('</SVD>\n')

    # write list of used filenames at end of file, as a comment
    append_filenames(list_of_used_files, f)

    return


def main():
    create_file()

if __name__ == '__main__':
    main()
