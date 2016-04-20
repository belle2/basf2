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
import codecs


def get_list_of_defects(filename):

    with open(filename, 'rt') as q:
        reader3 = csv.reader(q, delimiter=',')

        # look for row with defects types
        for row in reader3:
            if row and 'strip number' in row[0]:
                defects_list_raw = row
                break

    list_of_defects = []
    for cell in defects_list_raw:
        header = cell[1:3]
        # if head == 'p_' or head == 'n_': # then it's a defect type string
        list_of_defects.append(cell[3:])  # remove 'n_' from string
    return list_of_defects


def get_defect_type(row, list_of_defects):

    types = []
    for i, cell in enumerate(row):
        if i > 1 and '1' in cell:
            defect = list_of_defects[i]
            types.append(defect)

    return types


def from_num_to_label(i, j):
    Labels = [
        ['fw', 'bw'],
        ['fw', '-z', 'bw'],
        ['fw', 'ce', '-z', 'bw'],
        ['fw', '+z', 'ce', '-z', 'bw']
    ]

    Layer_labels = Labels[i - 3]
    return Layer_labels[j - 1]


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
                        if row and 'tags' in row[0]:
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


def print_header(output, list_of_good_defects):
    output.write('<?xml version="1.0" encoding="UTF-8"?>\n')
    output.write('<Meta>\n')
    output.write('\t<Date>' + time.strftime("%d/%m/%Y") + '</Date>\n')
    output.write('\t<Description short=\"Ignore strips list for SVD planes in 2016 TB\">\n')
    output.write('\t\tList of known strip defects from layers groups\n')
    output.write('\t</Description>\n')
    output.write('\t<Author>Giacomo Caria</Author>\n')
    output.write('</Meta>\n')
    output.write('<!-- List of input files used to produce this mask is appended at end of file.\n')
    output.write('     List of defects included in this mask: \n')
    # print list of defects to be considered, the 'good' ones
    for name in list_of_good_defects[:-1]:
        output.write('\t' + name + ' \n')
    output.write('\t' + list_of_good_defects[-1] + ' -->\n')

    return


def is_file_in_path(i, j, path_layer):
    files_list = [l for l in listdir(path_layer) if isfile(join(path_layer, l))]
    header = 'L' + str(i)
    bool = False
    for name in files_list:
        if (header in name):
            bool = True
            break
    return bool


def get_filename(i, j, k, path_layer):

    files_list = [l for l in listdir(path_layer) if isfile(join(path_layer, l))]

    # strings to be searched in filenames
    layer = 'L' + str(i)
    np_side = ['Nside', 'Pside']  # mapping this to side=['v', 'u']

    filename = 'nofile'

    # look up file based on layer, sensor type and side
    strings = [layer, np_side[k], from_num_to_label(i, j)]

    for name in files_list:
        if all(x in name for x in strings):
            filename = name
            break
    return path_layer + filename


def print_file_content(filename, f, list_of_good_defects):

    list_of_defects = get_list_of_defects(filename)

    with open(filename, 'rt') as x:
        reader = csv.reader(x, delimiter=',')
        a = 0

        # look for row just before strip numbers
        for i, row in enumerate(reader):
            if row:
                if 'strip number' in row[0]:
                    a = i
                    break

        # get strip numbers, enumerate now starts from the 'strip number' row
        for j, rows in enumerate(reader):
            if rows and hasNumbers(rows[0]):  # ok it's a strip defect line

                # only include strips with meaningful defect
                # i.e. with its defect in the list of considered defects
                matches = [defect for defect in get_defect_type(rows,
                                                                list_of_defects) if defect.lower() in list_of_good_defects]
                if matches:
                    f.write('\t\t\t\t\t<strip stripNo = \"' +
                            re.findall(r'\d+', rows[0])[0] + '\"></strip>\n')  # strip number cell might contain spaces
                    f.write('\t\t\t\t\t\t<!-- ' +
                            str(matches) + '--> \n')
                # if also want to include correctly parsed defect type do something like
                # f.write('\t\t\t\t\t<strip defectType = \"' +
                #        get_defect_type(rows,list_of_defects) + '\">' +
                #        re.findall(r'\d+', rows[0])[0]  +
                #        '</strip>\n')
            else:
                if rows and 'parameter' in rows[0]:
                    break  # stop when we hit the 'parameters' row, strip numbers have finished
    return


def append_filenames(list_of_used_files, file):
    file.write('<!-- List of input files used to create this xml file: \n')
    for name in list_of_used_files[:-1]:
        file.write('\t' + name + '\n')
    file.write('\t' + list_of_used_files[-1] + ' -->')


def create_file():
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
    output = 'SVD_MaskListKnownDefects.xml'
    f = open(output, 'w')

    print_header(f, list_of_good_defects)
    f.write('<SVD>\n')

    list_of_used_files = []

    for i in range(3, 7):  # loop on layers

        f.write('\t<layer n=\"' + str(i) + '\">\n')
        f.write('\t\t<ladder n=\"' + str(1) + '\">\n')

        # specify path to look for xml files
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
                    print_file_content(filename, f, list_of_good_defects)

                    # write closing lines
                    f.write('\t\t\t\t</side>\n')
            f.write('\t\t\t</sensor>\n')

        f.write('\t\t</ladder>\n')
        f.write('\t</layer>\n')
    f.write('</SVD>\n')

    append_filenames(list_of_used_files, f)

    return


def get_line_with_string(string, file):

    with open(file) as f:
        for i, line in enumerate(f):
            if string in line:
                a = i
                break


def main():
    create_file()

if __name__ == '__main__':
    main()
