#!/usr/bin/env python
# -*- coding: utf-8 -*-

# Implementation of AmgaClient class used for communication with AMGA metadata catalog
# Author: Milosz Zdybal (milosz.zdybal@ifj.edu.pl)
# 2010-01

import mdclient
import mdinterface
import sys

sys.path.append('../')

# from cnfg import Config


class AmgaClient(object):

    '''
    Module to handle connection with AMGA metadata-base
    '''

    config = None
    client = None

###############################################################################

    def __init__(self):
        '''
        Constructor - needs path of yaml file with configuration
        '''

        # self.config = Config.Config(confile)
        # self.client = mdclient.MDClient('amga2.collab.unimelb.edu.au', 8822, '', '')
        # self.client = mdclient.MDClient('uib2dev.ifj.edu.pl', 8822, '', '')
        try:
            self.client = mdclient.MDClient('150.183.246.196', 8822,
                    'belle_user', 'belle')
        except Exception, ex:
            print 'Count not connect to AMGA server:', ex

###############################################################################

    def getSubdirectories(self, dir):
        '''
        Get subdirectories in metadata catalog.
        '''

        result = []
        self.client.execute('ls ' + dir)
        while not self.client.eot():
            result.append(dir + '/' + self.client.fetchRow().split('/')[-1])
        return result

###############################################################################

    def getGUIDs(self, experiment, query):
        '''
        Old way of getting LFNs filfilling query.
        '''

        guids = []
        self.client.selectAttr([experiment], query)
        while not self.client.eot():
            # guids.append('guid:' + self.client.fetchRow())
            # -  we're interesteed in LFNS now
            guids.append(self.client.fetchRow())
        return guids

###############################################################################

    def directQuery(self, experiment, query):
        '''
        Query with ability to define parameters in SQL way.
        Returns list of LFNs.
        '''

        self.client.find('*', query)
        results = []
        while not self.client.eot():
            results.append(self.client.fetchRow())
        return results

###############################################################################

    def checkDirectory(self, path):
        '''
      Checks the directory exists, and creates if it is doesn't
      To be used before insertions
      '''

        try:
            self.client.cd(path)
        except mdinterface.CommandException, ex:
        # it doesn't exist - create
            try:
                self.client.createDir(path)
            except mdinterface.CommandException, ex2:
                print 'Error:', ex2
                return False
        return True

###############################################################################

    def bulkInsert(self, basepath, entries):
        '''
      Loops through a list of entries, inserting as efficiently as possible
      entries[entry_name] = (['key1', 'key2', 'key3'],['value1', value2, value3])
      TODO: use direct SQL mode.
      '''

        if self.checkDirectory(basepath):
            for entry in entries.keys():
                try:
                    client.addentries(basepath + '/' + entry,
                                      entries[entry][0], entries[entry][1])
                except mdinterface.CommandException, ex:
                    print 'Error:', ex
                    return False
            return True
        else:
            return False


