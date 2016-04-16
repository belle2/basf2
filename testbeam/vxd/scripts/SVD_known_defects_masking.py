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
# 16 April 2016

import os
import sys
import math
import numpy as np
import time
import csv
import re
from os import listdir
from os.path import isfile, join


def update_z_filenames(path):

    for i in range(3, 7):  # loop on layer
        # specify path to look for xml files
        path_layer = path + '/L' + str(i) + '/'
        files_list = [l for l in listdir(path_layer) if isfile(join(path_layer, l))]

        for name in files_list:
            if '__z_' in name:
                filename = path_layer + name
                with open(filename, 'rt') as w:
                    reader2 = csv.reader(w, delimiter=',')
                    b = 0
                    # look for row just before sensor type specification
                    for i, row in enumerate(reader2):
                        if row:
                            if 'tags' in row[0]:
                                b = i
                                break
                    # then get z type
                    for i, row in enumerate(reader2):
                        if row:
                            for sub in row:
                                if 'z' in sub:
                                    z_str = sub
                                    break
                # create new filename
                filename_new = path_layer + name.replace('_z', z_str)
                # rename file
                os.rename(filename, filename_new)
    return


def hasNumbers(inputString):
    return any(char.isdigit() for char in inputString)


def print_header(f, i):
    f.write('<?xml version="1.0" encoding="UTF-8"?>\n')
    f.write('<Meta>\n')
    f.write('\t<Date>' + time.strftime("%d/%m/%Y") + '<\Date>\n')
    f.write('\t<Description short=\"Ignore strips list for SVD plane #' +
            str(i) + ' in 2016 VXD TB\">\n')
    f.write('\t\tList of known strip defects from layer' + str(i) + ' group\n')
    f.write('\t</Description>\n')
    f.write('\t<Author>Giacomo Caria</Author>\n')
    f.write('</Meta>\n')
    return


def if_file_exists(i, j, path_layer):
    files_list = [l for l in listdir(path_layer) if isfile(join(path_layer, l))]
    header = 'L' + str(i)
    bool = False
    for name in files_list:
        if (header in name):
            bool = True
            break
    return bool


def get_filename(i, j, k, path_layer):

    # get all filenames in directory
    files_list = [l for l in listdir(path_layer) if isfile(join(path_layer, l))]

    # strings to be searched in filenames
    layer = 'L' + str(i)
    type = ['_bw_', '_fw_']
    # type=['_bw_','_fw_','_z_','_ce_',''] # +z,-z --> _z
    np_side = ['Nside', 'Pside']  # mapping this to side=['v', 'u']

    filename = 'nofile'

    # look up file based on layer, sensor type and side
    strings = [layer, np_side[k], type[j - 1]]

    for name in files_list:
        if all(x in name for x in strings):
            filename = name
            break
    return path_layer + filename


def print_file_content(filename, f):

    with open(filename, 'rt') as x:
        reader = csv.reader(x, delimiter=',')
        a = 0

        # look for row just before strip numbers
        for i, row in enumerate(reader):
            if row:
                if 'strip number' in row[0]:
                    a = i
                    break

        # get strip numbers, enumerate now starts from 'strip number' row
        for j, rows in enumerate(reader):
            if rows and hasNumbers(rows[0]):
                f.write('\t\t\t\t\t<strip stripNo = \"' +
                        re.findall(r'\d+', rows[0])[0] + '\"><strip>\n')  # strip number cell might contain spaces
            else:
                if rows and 'parameter' in rows[0]:
                    break  # stop when we hit the 'parameters' row, strip numbers have finished
    return


def main():

    uv_side = ['v', 'u']
    np_side = ['Nside', 'Pside']
    path = './input_files'

    # rename z files with appropriate '+z' or '-z' labels
    # update_z_filenames(path)

    for i in range(3, 7):  # loop on layers

        # specify path to look for xml files
        path_layer = path + '/L' + str(i) + '/'
        # open file
        output = 'SVD' + str(i) + '_MaskListKnownDefects.xml'
        f = open(output, 'w')

        print_header(f, i)

        f.write('<SVD>\n')
        f.write('\t<layer n=\"' + str(i) + '\">\n')
        f.write('\t\t<ladder n=\"' + str(1) + '\">\n')

        for j in range(1, 3):  # loop on sensors, only doing bw and fw now

            # if needed file is not found, continue
            if not if_file_exists(i, j, path_layer):
                print('Files for layer ' + str(i) + ', sensor ' + str(j) + ', not found. Moving on ..')
                continue

            # write sensor details
            f.write('\t\t\t<sensor n=\"' + str(j) + '\">\n')

            for k in range(0, 2):  # loop on sides

                filename = get_filename(i, j, k, path_layer)

                if 'nofile' in filename:  # file hasn't been found
                    print('File for layer ' + str(i) + ', sensor ' + str(j) + ', ' +
                          np_side[k] + ', not found. Moving on ..')
                    continue
                else:  # file has been found
                    print('Found file: ' + filename)
                    f.write('\t\t\t\t<side side=\"' + uv_side[k] + '\">\n')
                    print_file_content(filename, f)
                    # write closing lines
                    f.write('\t\t\t\t</side>\n')
            f.write('\t\t\t</sensor>\n')

        f.write('\t\t</ladder>\n')
        f.write('\t</layer>\n')
        f.write('</SVD>\n')
    return

if __name__ == '__main__':
    main()
