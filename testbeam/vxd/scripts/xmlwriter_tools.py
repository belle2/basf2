#!/usr/bin/env python3
# -*- coding: utf-8 -*-


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


def get_line_with_string(string, file):

    with open(file) as f:
        for i, line in enumerate(f):
            if string in line:
                a = i
                break


def get_n_strips(i, k):

    if i == 3:
        n = 768
    else:
        if k == 0:
            n = 512
        elif k == 1:
            n = 768

    return n


def phys_to_uv_strip_number(i, j, k, phys_strip_number):  # layer, sensor and side

    if (i == 3 and j == 2) or (i > 3 and j > 1 and k == 0) or (i > 3 and j == 1 and k == 1):
        uv_strip_number = phys_strip_number
    else:
        n = get_n_strips(i, k)
        uv_coordinate = n - 1 - int(phys_strip_number)
        uv_strip_number = str(uv_coordinate)
        # print(str(i) + '.' + str(j) + '.' + str(k) + ': converted strip number from ' + phys_strip_number + ' to ' +
        #      uv_strip_number)

    return uv_strip_number


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


def print_header(output, list_of_good_defects, use_uv_strip_number):
    output.write('<?xml version="1.0" encoding="UTF-8"?>\n')
    output.write('<Meta>\n')
    output.write('\t<Date>' + time.strftime("%d/%m/%Y") + '</Date>\n')
    output.write('\t<Description short=\"Ignore strips list for SVD planes in 2016 TB\">\n')
    output.write('\t\tList of known strip defects from layers groups\n')
    output.write('\t</Description>\n')
    output.write('\t<Author>Giacomo Caria</Author>\n')
    output.write('</Meta>\n')
    output.write('<!-- Using u/v strip numbers: ' + str(use_uv_strip_number) + '.\n')
    output.write('     List of input files used to produce this mask is appended at end of file.\n')
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


def print_file_content(layer_num, sensor_num, side_num, filename, f,
                       list_of_good_defects,
                       write_defect_types, use_uv_strip_number):

    list_of_defects = get_list_of_defects(filename)

    with open(filename, 'rt') as x:
        reader = csv.reader(x, delimiter=',')
        a = 0

        # look for row just before strip numbers
        for r1, row in enumerate(reader):
            if row:
                if 'strip number' in row[0]:
                    a = r1
                    break

        # get strip numbers, enumerate now starts from the 'strip number' row
        for r2, rows in enumerate(reader):
            if rows and hasNumbers(rows[0]):  # ok it's a strip defect line

                # only include strips with meaningful defect
                # i.e. with its defect in the list of considered defects
                matches = [defect for defect in get_defect_type(rows,
                                                                list_of_defects) if defect.lower() in list_of_good_defects]
                if matches:
                    # create list of defects for this strip
                    defects_str = ''
                    for m, match in enumerate(matches):
                        defects_str += ' defect' + str(m + 1) + ' = \"' + str(match) + '\"'

                    # get strip number from cell
                    phys_strip_number = re.findall(r'\d+', rows[0])[0]

                    # convert strip number from physical(written on sensor, used
                    # in defectFinder) to the software one(u/v coordinates, used in reconstruction)
                    uv_strip_number = phys_to_uv_strip_number(layer_num, sensor_num, side_num, phys_strip_number)

                    if use_uv_strip_number:
                        strip_number = uv_strip_number
                    else:
                        strip_number = phys_strip_number

                    # write strip number and its defects in xml file
                    if write_defect_types:
                        f.write('\t\t\t\t\t<strip stripNo = \"' + strip_number + '\"' + defects_str + '></strip>\n')
                    else:
                        f.write('\t\t\t\t\t<strip stripNo = \"' + strip_number + '\"' + '></strip>\n' +
                                '\t\t\t\t\t<!-- ' + defects_str + ' -->\n')
            else:
                if rows and 'parameter' in rows[0]:
                    break  # stop when we hit the 'parameters' row, strip numbers have finished
    return


def append_filenames(list_of_used_files, file):
    file.write('<!-- List of input files used to create this xml file: \n')
    for name in list_of_used_files[:-1]:
        file.write('\t' + name + '\n')
    file.write('\t' + list_of_used_files[-1] + ' -->')
