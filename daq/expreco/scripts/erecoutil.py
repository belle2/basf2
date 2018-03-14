#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
import sys
import subprocess
import signal
import socket
import time


# Basic Utilities
# Get full path of the configuration file

def get_configpath(conffile):
    confdir = str(os.environ.get('ERECO_CONFDIR'))
    if confdir == 'None':
        print 'ERECO_CONFDIR is not defined. Exit.'
        sys.exit()
    cmd = confdir + '/' + conffile + '.conf'
    return cmd


# Get Configuration from config file
def get_ergetconf(conffile, item1, item2='NULL', item3='NULL'):

    confdir = str(os.environ.get('ERECO_CONFDIR'))
    if confdir == 'None':
        print 'ERECO_CONFDIR is not defined. Exit.'
        sys.exit()
    cmd = 'rfgetconf ' + get_configpath(conffile) + ' ' + item1 + ' ' + item2 \
        + ' ' + item3
    p = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE,
                         stderr=subprocess.PIPE)
    p.wait()
    output = p.stdout.read()
#    print output
    return output


# print "waiting"
# confout = p.stdout.read()

# NSMD related utilities
# Run NSMD

def run_nsmd(nsmdir, port, nsmhost):
    # Check directory for loggin
    if not os.path.exists(nsmdir + '/' + nsmhost):
        os.mkdir(nsmdir + '/' + nsmhost)
    nsmd = str(os.environ.get('BELLE2_EXTERNALS_BIN')) + '/nsmd2 -f -p ' \
        + port + ' -s ' + port + ' -h '
    cmd = 'ssh ' + nsmhost + ' "cd ' + nsmdir + '/' + nsmhost \
        + '; setenv NSMLOGDIR ' + nsmdir + '/' + nsmhost + ';' + nsmd \
        + nsmhost + '"'
    print cmd
    p = subprocess.Popen(cmd, shell=True)
    time.sleep(1)


#    p.wait()

# Kill NSMD

def kill_nsmd(port, nsmhost):
    cmd = 'ssh ' + nsmhost + ' "ps -fC nsmd2 | grep ' + port \
        + "| awk '{print \$2}' \" > temp.pid"
#    cmd = "ssh -v " + nsmhost + " \"ps -fC nsmd2 | grep " + port + "| awk '{printf(\"klll \%d\", \$2)} | sh' \""
#    print cmd
    p = subprocess.Popen(cmd, shell=True)
    p.wait()
    for line in open('temp.pid', 'r'):
        pid = int(line)
        if pid > 0:
            cmd = 'ssh ' + nsmhost + ' "kill ' + str(pid) + '"'
#            print cmd
            p = subprocess.Popen(cmd, shell=True)
            p.wait()


# Start NSMD on all nodes

def start_nsmd(conffile):
    # Global parameters
    nsmdir = get_ergetconf(conffile, 'system', 'nsmdir_base')
    port = get_ergetconf(conffile, 'system', 'nsmport')

    # Run nsmd on control node
    ctlhost = get_ergetconf(conffile, 'master', 'ctlhost')
    run_nsmd(nsmdir, port, ctlhost)
    print 'nsmd on %s started' % ctlhost

    # Run nsmd on event server node
    evshost = get_ergetconf(conffile, 'distributor', 'ctlhost')
    if ctlhost.find(evshost) == -1:
        run_nsmd(nsmdir, port, evshost)
        print 'nsmd on %s started' % evshost

    # Run nsmd on event processor nodes
    nnodes = int(get_ergetconf(conffile, 'processor', 'nnodes'))
    procid = int(get_ergetconf(conffile, 'processor', 'idbase'))
    badlist = get_ergetconf(conffile, 'processor', 'badlist')
    evphostbase = get_ergetconf(conffile, 'processor', 'ctlhostbase')
    for i in range(procid, procid + nnodes):
        nodeid = '%2.2d' % i
        if badlist.find(nodeid) == -1:
            evphost = evphostbase + nodeid
            run_nsmd(nsmdir, port, evphost)
            print 'nsmd on %s started' % evphost


def stop_nsmd(conffile):
    port = get_ergetconf(conffile, 'system', 'nsmport')

    # Kill nsmd on control node
    ctlhost = get_ergetconf(conffile, 'master', 'ctlhost')
    kill_nsmd(port, ctlhost)
    print 'nsmd on %s stopped' % ctlhost

    # Run nsmd on event server node
    evshost = get_ergetconf(conffile, 'distributor', 'ctlhost')
    if ctlhost.find(evshost) == -1:
        kill_nsmd(port, evshost)
        print 'nsmd on %s stopped' % evshost

    # Run nsmd on event processor nodes
    nnodes = int(get_ergetconf(conffile, 'processor', 'nnodes'))
    procid = int(get_ergetconf(conffile, 'processor', 'idbase'))
    badlist = get_ergetconf(conffile, 'processor', 'badlist')
    evphostbase = get_ergetconf(conffile, 'processor', 'ctlhostbase')
    for i in range(procid, procid + nnodes):
        nodeid = '%2.2d' % i
        if badlist.find(nodeid) == -1:
            evphost = evphostbase + nodeid
            kill_nsmd(port, evphost)
            print 'nsmd on %s stopped' % evphost


# RFARM server operations
# Run eventserver

def run_distributor(conffile):
    evshost = get_ergetconf(conffile, 'distributor', 'ctlhost')
    basedir = get_ergetconf(conffile, 'system', 'execdir_base')
    port = get_ergetconf(conffile, 'system', 'nsmport')
    if not os.path.exists(basedir + '/distributor'):
        os.mkdir(basedir + '/distributor')
    cmd = 'ssh ' + evshost + ' "cd ' + basedir + '; setenv NSM2_PORT ' + port \
        + '; ereco_distributor ' + get_configpath(conffile) \
        + ' > & distributor/nsmlog.log" '
    print cmd
    p = subprocess.Popen(cmd, shell=True)
    time.sleep(1)


# Stop eventserver

def stop_distributor(conffile):
    evshost = get_ergetconf(conffile, 'distributor', 'ctlhost')
    basedir = get_ergetconf(conffile, 'system', 'execdir_base')
    unit = get_ergetconf(conffile, 'system', 'unitname')
    ringbuf = get_ergetconf(conffile, 'distributor', 'ringbuffer')
    rbufname = unit + ':' + ringbuf
    shmname = unit + ':distributor'
    p = subprocess.Popen('rfcommand ' + conffile +
                         ' distributor RC_ABORT', shell=True)
    p.wait()
    pidfile = basedir + '/distributor/pid.data'
    for pid in open(pidfile, 'r'):
        cmd = 'ssh ' + evshost + ' "kill ' + pid + '; removerb ' + rbufname \
            + "; removeshm " + shmname + '"'
        print cmd
        p = subprocess.Popen(cmd, shell=True)
        p.wait()


# Start event procesor

def run_eventprocessor(conffile):
    hostbase = get_ergetconf(conffile, 'processor', 'ctlhostbase')
    nodebase = get_ergetconf(conffile, 'processor', 'nodebase')
    basedir = get_ergetconf(conffile, 'system', 'execdir_base')
    port = get_ergetconf(conffile, 'system', 'nsmport')
    nnodes = int(get_ergetconf(conffile, 'processor', 'nnodes'))
    procid = int(get_ergetconf(conffile, 'processor', 'idbase'))
    badlist = get_ergetconf(conffile, 'processor', 'badlist')
    id = int(get_ergetconf(conffile, 'processor', 'idbase'))

    for i in range(procid, procid + nnodes):
        nodeid = '%2.2d' % i
        if badlist.find(nodeid) == -1:
            evphost = hostbase + nodeid
            nodename = nodebase + nodeid
            if not os.path.exists(basedir + '/evp_' + nodename):
                os.mkdir(basedir + '/evp_' + nodename)
            cmd = 'ssh ' + evphost + ' "cd ' + basedir + '; setenv NSM2_PORT ' \
                + port + '; ereco_eventprocessor ' + get_configpath(conffile) \
                + ' > & evp_' + nodename + '/nsmlog.log" '
            print cmd
            p = subprocess.Popen(cmd, shell=True)
            time.sleep(1)


# Stop event procesor

def stop_eventprocessor(conffile):
    hostbase = get_ergetconf(conffile, 'processor', 'ctlhostbase')
    nodebase = get_ergetconf(conffile, 'processor', 'nodebase')
    basedir = get_ergetconf(conffile, 'system', 'execdir_base')
    port = get_ergetconf(conffile, 'system', 'nsmport')
    nnodes = int(get_ergetconf(conffile, 'processor', 'nnodes'))
    procid = int(get_ergetconf(conffile, 'processor', 'idbase'))
    badlist = get_ergetconf(conffile, 'processor', 'badlist')
    id = int(get_ergetconf(conffile, 'processor', 'idbase'))

    unit = get_ergetconf(conffile, 'system', 'unitname')
    rbufin = get_ergetconf(conffile, 'collector', 'ringbufin')
    rbufout = get_ergetconf(conffile, 'collector', 'ringbufout')
    rbufinname = unit + ':' + rbufin
    rbufoutname = unit + ':' + rbufout

    for i in range(procid, procid + nnodes):
        nodeid = '%2.2d' % i
        if badlist.find(nodeid) == -1:
            evphost = hostbase + nodeid
            nodename = 'evp_' + nodebase + nodeid
            shmname = unit + ':' + nodename
            print shmname
            p = subprocess.Popen('rfcommand ' + conffile + ' ' + nodename +
                                 ' RC_ABORT', shell=True)
            p.wait()
            pidfile = basedir + '/' + nodename + '/pid.data'
            for pid in open(pidfile, 'r'):
                cmd = 'ssh ' + evphost + ' "kill ' + pid + '; removerb ' \
                    + rbufinname + '; removerb ' + rbufoutname \
                    + '; removeshm ' + shmname + '; clear_basf2_ipc"'
#                    + '; removeshm ' +  '"'
#                    + '; removeshm ' + shmname + '"'
                print cmd
                p = subprocess.Popen(cmd, shell=True)
                p.wait()


# Run dqmserver

def run_dqmserver(conffile):
    dqmhost = get_ergetconf(conffile, 'dqmserver', 'ctlhost')
    basedir = get_ergetconf(conffile, 'system', 'execdir_base')
    port = get_ergetconf(conffile, 'system', 'nsmport')
    if not os.path.exists(basedir + '/dqmserver'):
        os.mkdir(basedir + '/dqmserver')
    cmd = 'ssh ' + dqmhost + ' "cd ' + basedir + '; setenv NSM2_PORT ' + port \
        + '; rf_dqmserver ' + get_configpath(conffile) \
        + ' > & dqmserver/nsmlog.log" '
    print cmd
    p = subprocess.Popen(cmd, shell=True)
    time.sleep(1)


# Stop dqmserver

def stop_dqmserver(conffile):
    dqmhost = get_ergetconf(conffile, 'dqmserver', 'ctlhost')
    basedir = get_ergetconf(conffile, 'system', 'execdir_base')
    p = subprocess.Popen('rfcommand ' + conffile + ' dqmserver RC_ABORT', shell=True)
    p.wait()
    pidfile = basedir + '/dqmserver/pid.data'
    for pid in open(pidfile, 'r'):
        cmd = 'ssh ' + dqmhost + ' "kill ' + pid + '"'
        print cmd
        p = subprocess.Popen(cmd, shell=True)
        p.wait()

# Run eventsampler


def run_eventsampler(conffile):
    samplerhost = get_ergetconf(conffile, 'eventsampler', 'ctlhost')
    basedir = get_ergetconf(conffile, 'system', 'execdir_base')
    port = get_ergetconf(conffile, 'system', 'nsmport')
    if not os.path.exists(basedir + '/sampler'):
        os.mkdir(basedir + '/sampler')
    cmd = 'ssh ' + samplerhost + ' "cd ' + basedir + '; setenv NSM2_PORT ' + port \
        + '; ereco_eventsampler ' + get_configpath(conffile) \
        + ' > & sampler/nsmlog.log" '
    print cmd
    p = subprocess.Popen(cmd, shell=True)
    time.sleep(1)


# Stop eventsampler

def stop_eventsampler(conffile):
    samplerhost = get_ergetconf(conffile, 'eventsampler', 'ctlhost')
    basedir = get_ergetconf(conffile, 'system', 'execdir_base')
    p = subprocess.Popen('rfcommand ' + conffile + ' sampler RC_ABORT', shell=True)
    p.wait()
    pidfile = basedir + '/sampler/pid.data'
    for pid in open(pidfile, 'r'):
        cmd = 'ssh ' + samplerhost + ' "kill ' + pid + '"'
        print cmd
        p = subprocess.Popen(cmd, shell=True)
        p.wait()


# Run local master

def run_master(conffile):
    masterhost = get_ergetconf(conffile, 'master', 'ctlhost')
    basedir = get_ergetconf(conffile, 'system', 'execdir_base')
    port = get_ergetconf(conffile, 'system', 'nsmport')
    if not os.path.exists(basedir + '/master'):
        os.mkdir(basedir + '/master')
    cmd = 'ssh ' + masterhost + ' "cd ' + basedir + '; setenv NSM2_PORT ' \
        + port + '; ereco_master_local ' + get_configpath(conffile) \
        + ' > & master/nsmlog.log" '
    print cmd
    p = subprocess.Popen(cmd, shell=True)
    time.sleep(1)


# Stop local master

def stop_master(conffile):
    masterhost = get_ergetconf(conffile, 'master', 'ctlhost')
    basedir = get_ergetconf(conffile, 'system', 'execdir_base')
#    p = subprocess.Popen ( "rfcommand " + conffile + " master RF_UNCONFIGURE", shell=True );
#    p.wait();
    pidfile = basedir + '/master/pid.data'
    for pid in open(pidfile, 'r'):
        cmd = 'ssh ' + masterhost + ' "kill ' + pid + '"'
        print cmd
        p = subprocess.Popen(cmd, shell=True)
        p.wait()


def start_ereco_components(conffile):
    run_eventprocessor(conffile)
    run_distributor(conffile)
    run_dqmserver(conffile)
    run_eventsampler(conffile)


# Stop ERECO components

def stop_ereco_components(conffile):
    stop_eventsampler(conffile)
    stop_dqmserver(conffile)
    stop_distributor(conffile)
    stop_eventprocessor(conffile)


# Start ERECO local operation

def start_ereco_local(conffile):
    start_ereco_components(conffile)
    run_master(conffile)


# Stop ERECO local operation

def stop_ereco_local(conffile):
    #    stop_eventprocessor(conffile)
    stop_ereco_components(conffile)
    stop_master(conffile)
