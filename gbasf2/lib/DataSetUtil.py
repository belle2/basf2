#!/usr/bin/env python
# -*- coding: utf-8 -*-

from DIRAC.Core.Base import Script

import DIRAC
from DIRAC.Core.Security.Misc import *
from gbasf2util import CLIParams


class DSParam:

    def __init__(self, prog):
        self.prog = prog
        cliParams = CLIParams()
        self.Project = cliParams.getProject()
        # proxyinfo = getProxyInfo()
            # if not proxyinfo['OK']:
        # ....print "getting proxy error!"
            # ....DIRAC.exit(1)
            # else:
        # ....self.User=proxyinfo['Value']['username']
        self.User = ''
        self.DstSE = ''
        self.DstDir = ''
        self.ListReplica = 0
        self.ListSite = 0

    def setProject(self, arg):
        self.Project = arg
        # print "project is ",self.Project
        return DIRAC.S_OK()

    def setUser(self, arg):
        self.User = arg
        # print "User is ",self.User
        return DIRAC.S_OK()

    def setDstSE(self, arg):
        self.DstSE = arg
        return DIRAC.S_OK()

    def setDstDir(self, arg):
        self.DstDir = arg
        return DIRAC.S_OK()

    def print_help(self, dummy=False):
        if 'ls' in self.prog:
            usage = \
                '''%s
\t-h print help information
\t-p project name
\t-u user name
\t-r list replica
\t-i list site
''' \
                % self.prog
        elif 'rep' in self.prog or 'generate' in self.prog:
            usage = \
                '''%s
\t-h print help information
\t-p project name
\t-u user name
\t-s Destination Storage Element''' \
                % self.prog
        elif 'rm_rep' in self.prog:
            usage = \
                '''%s
\t-h print help information
\t-p project name
\t-u user name
\t-s Destination Storage Element''' \
                % self.prog
        elif 'rm' in self.prog:
            usage = \
                '''%s
\t-h print help information
\t-p project name
\t-u user name
''' \
                % self.prog
        elif 'get' in self.prog:
            usage = \
                '''%s
\t-h print help information
\t-p project name
\t-u user name
\t-d Destination Directory to download data
''' \
                % self.prog
        elif 'sync' in self.prog:
            usage = \
                '''%s
\t-h print help information
\t-p project name
\t-u user name
''' \
                % self.prog
        else:
            usage = 'programm is not supported!\n'
        print usage
        DIRAC.exit(-1)

    def setListReplica(self, dummy=False):
        self.ListReplica = 1

    def setListSite(self, dummy=False):
        self.ListSite = 1

    def getUser(self):
        if self.User == '':
            Script.parseCommandLine(ignoreErrors=True)
            proxyinfo = getProxyInfo()
            if not proxyinfo['OK']:
                print 'getting proxy error!'
                DIRAC.exit(1)
            else:
                self.User = proxyinfo['Value']['username']
        return self.User

    def getProject(self):
        return self.Project

    def getListReplica(self):
        return self.ListReplica

    def getListSite(self):
        return self.ListSite

    def getDstSE(self):
        return self.DstSE

    def getDstDir(self):
        return self.DstDir

    def register_param(self):
        Script.localCfg.commandOptionList = []
        Script.registerSwitch('p:', 'project=', 'project name',
                              self.setProject)
        Script.registerSwitch('u:', 'user=', 'user name', self.setUser)
        Script.registerSwitch('s:', 'dst_se=', 'destination SE', self.setDstSE)
        Script.registerSwitch('d:', 'dst_dir=', 'destination local directory',
                              self.setDstDir)
        Script.registerSwitch('r', 'replica', 'list  replicas from all sites '
                              , self.setListReplica)
        Script.registerSwitch('i', 'site', 'list  replicas by sites ',
                              self.setListSite)
        Script.registerSwitch('h', 'help', 'print help information ',
                              self.print_help)


