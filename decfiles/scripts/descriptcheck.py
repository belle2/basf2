#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# --------------------------------------
# Class for comparing decfile descriptors
# --------------------------------------

import settings
from colours import warning

partdict = {}
mesgdict = []
tag = False
for line in open(settings.partdictpath):
    if not tag:
        if '<DICTIONARY>' in line:
            tag = True
            continue
    if tag:
        partdict[line.split()[1]] = line.split()[0]


# --------------------------------------
# Functions for comparing descriptors
# --------------------------------------

def inlist(partA, listA):
    if isinstance(listA, list):
        for part in listA:
            if partA == part:
                return True
            if isinstance(part, list):
                if inlist(partA, part):
                    return True
    else:
        return partA == listA


def convertToList(stringA):
    stringA = stringA.replace('(', ' ( ')
    stringA = stringA.replace(')', ' ) ')
    stringA = stringA.replace(']', ' ] ')
    stringA = stringA.replace('[', ' [ ')
    stringA = stringA.replace('}', ' } ')
    stringA = stringA.replace('{', ' { ')
    list_base = stringA.split()
    list_base = cleanList('{}', list_base)
    list_base = cleanList('[]', list_base)
    list_base = cleanList('()', list_base)
    while len(list_base) == 1:
        list_base = list_base[0]
    list_base = postProcess(list_base)
    return list_base


def postProcess(list_base):
    if not isinstance(list_base, list):
        warning('Error building descriptor, not a list')
    i = 0
    while i < len(list_base):
        if isinstance(list_base[i], list):
            if len(list_base[i]) == 1:
                if i > 0:
                    if isinstance(list_base[i - 1], str):
                        if list_base[i][0] == 'os' or list_base[i][0] == 'nos':
                            list_base[i - 1] = list_base[i - 1] + '[' \
                                + list_base[i][0] + ']'
                            list_base.pop(i)
                            i -= 1
                        else:
                            list_base[i - 1] = list_base[i - 1] + '(' \
                                + list_base[i][0] + ')'
                            list_base.pop(i)
                            i -= 1
                    else:
                        list_base[i] = list_base[i][0]
                        warning("You have a lone particle enclosed in '()' or some such stuff: " + list_base[i])
                else:
                    list_base[i] = '(' + list_base[i][0] + ')'
            else:
                list_base[i] = postProcess(list_base[i])
        i += 1
    return list_base


def cleanList(stringA, list_base):
    tag1 = stringA[0]
    tag2 = stringA[1]

    while inlist(tag1, list_base):
        start_index = 0
        end_index = -1
        for i in range(len(list_base)):
            if isinstance(list_base[i], list):
                list_base[i] = cleanList(stringA, list_base[i])
            if tag1 == list_base[i]:
                start_index = i
            if tag2 == list_base[i]:
                newlist = list_base[start_index + 1:i]
                list_base = list_base[:start_index] + [newlist] + list_base[i + 1:]
                break
    return list_base


# converts the particle to the appropriate pdg id

def convertPart(partA):
    if partA in partdict:
        return partdict[partA]
    else:
        if partA not in [partdict[x] for x in partdict] and not partA.lower() == 'cc' and not partA == '->':
            # mesgdict+=["Particle "+partA+" not recognised!"]
            pass
    return partA


def compareList(listA, listB):
    while not (listA == [] and listB == []):
        toBreak = False
        for partA in listA:
            if toBreak:
                break
            for partB in listB:
                if comparePart(partA, partB):
                    listA.remove(partA)
                    listB.remove(partB)
                    toBreak = True
                    break
        # if this ever processes then the for loops went through without matching a particle and lists are not empty
        if toBreak:
            continue
        return False
    return True


def comparePart(partA, partB):
    if isinstance(partA, list) and isinstance(partB, list):
        return compareList(partA, partB)
    elif not isinstance(partA, list) and not isinstance(partB, list):
        if convertPart(partA) == convertPart(partB):
            return True
    return False
