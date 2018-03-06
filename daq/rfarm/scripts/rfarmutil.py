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
    confdir = str(os.environ.get('RFARM_CONFDIR'))
    if confdir == 'None':
        print 'RFARM_CONFDIR is not defined. Exit.'
        sys.exit()
    cmd = confdir + '/' + conffile + '.conf'
    return cmd


# Get Configuration from config file
def get_rfgetconf(conffile, item1, item2='NULL', item3='NULL'):

    confdir = str(os.environ.get('RFARM_CONFDIR'))
    if confdir == 'None':
        print 'RFARM_CONFDIR is not defined. Exit.'
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
#    nsmd = str(os.environ.get('BELLE2_EXTERNALS_BIN')) + '/nsmd2 -f -p ' \
    nsmd = 'nsmd2 -f -p ' \
        + port + ' -s ' + port + ' -h '
    cmd = 'ssh ' + nsmhost + ' "cd ' + nsmdir + '/' + nsmhost \
        + '; setenv NSMLOGDIR ' + nsmdir + '/' + nsmhost + ';' + nsmd \
        + nsmhost + '"'
#    print cmd
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
    nsmdir = get_rfgetconf(conffile, 'system', 'nsmdir_base')
    port = get_rfgetconf(conffile, 'system', 'nsmport')

    # Run nsmd on control node
    ctlhost = get_rfgetconf(conffile, 'master', 'ctlhost')
    run_nsmd(nsmdir, port, ctlhost)
    print 'nsmd on %s started' % ctlhost

    # Run nsmd on event server node
    evshost = get_rfgetconf(conffile, 'distributor', 'ctlhost')
    if ctlhost.find(evshost) == -1:
        run_nsmd(nsmdir, port, evshost)
        print 'nsmd on %s started' % evshost

    # Run nsmd on output server node
    opshost = get_rfgetconf(conffile, 'collector', 'ctlhost')
    run_nsmd(nsmdir, port, opshost)
    print 'nsmd on %s started' % opshost

    # Run nsmd on event processor nodes
    nnodes = int(get_rfgetconf(conffile, 'processor', 'nnodes'))
    procid = int(get_rfgetconf(conffile, 'processor', 'idbase'))
    badlist = get_rfgetconf(conffile, 'processor', 'badlist')
    evphostbase = get_rfgetconf(conffile, 'processor', 'ctlhostbase')
    for i in range(procid, procid + nnodes):
        nodeid = '%2.2d' % i
        if badlist.find(nodeid) == -1:
            evphost = evphostbase + nodeid
            run_nsmd(nsmdir, port, evphost)
            print 'nsmd on %s started' % evphost


def stop_nsmd(conffile):
    port = get_rfgetconf(conffile, 'system', 'nsmport')

    # Kill nsmd on control node
    ctlhost = get_rfgetconf(conffile, 'master', 'ctlhost')
    kill_nsmd(port, ctlhost)
    print 'nsmd on %s stopped' % ctlhost

    # Run nsmd on event server node
    evshost = get_rfgetconf(conffile, 'distributor', 'ctlhost')
    if ctlhost.find(evshost) == -1:
        kill_nsmd(port, evshost)
        print 'nsmd on %s stopped' % evshost

    # Run nsmd on output server node
    opshost = get_rfgetconf(conffile, 'collector', 'ctlhost')
    kill_nsmd(port, opshost)
    print 'nsmd on %s stopped' % opshost

    # Run nsmd on event processor nodes
    nnodes = int(get_rfgetconf(conffile, 'processor', 'nnodes'))
    procid = int(get_rfgetconf(conffile, 'processor', 'idbase'))
    badlist = get_rfgetconf(conffile, 'processor', 'badlist')
    evphostbase = get_rfgetconf(conffile, 'processor', 'ctlhostbase')
    for i in range(procid, procid + nnodes):
        nodeid = '%2.2d' % i
        if badlist.find(nodeid) == -1:
            evphost = evphostbase + nodeid
            kill_nsmd(port, evphost)
            print 'nsmd on %s stopped' % evphost


# RFARM server operations
# Run eventserver

def run_eventserver(conffile):
    evshost = get_rfgetconf(conffile, 'distributor', 'ctlhost')
    basedir = get_rfgetconf(conffile, 'system', 'execdir_base')
    port = get_rfgetconf(conffile, 'system', 'nsmport')
    if not os.path.exists(basedir + '/distributor'):
        os.mkdir(basedir + '/distributor')
    cmd = 'ssh ' + evshost + ' "cd ' + basedir + '; setenv NSM2_PORT ' + port \
        + '; rf_eventserver ' + get_configpath(conffile) \
        + ' > & distributor/nsmlog.log" '
    print cmd
    p = subprocess.Popen(cmd, shell=True)
    time.sleep(1)


# Stop eventserver

def stop_eventserver(conffile):
    evshost = get_rfgetconf(conffile, 'distributor', 'ctlhost')
    basedir = get_rfgetconf(conffile, 'system', 'execdir_base')
    unit = get_rfgetconf(conffile, 'system', 'unitname')
    ringbuf = get_rfgetconf(conffile, 'distributor', 'ringbuffer')
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


# Run outputserver

def run_outputserver(conffile):
    opshost = get_rfgetconf(conffile, 'collector', 'ctlhost')
    basedir = get_rfgetconf(conffile, 'system', 'execdir_base')
    port = get_rfgetconf(conffile, 'system', 'nsmport')
    if not os.path.exists(basedir + '/collector'):
        os.mkdir(basedir + '/collector')
    cmd = 'ssh ' + opshost + ' "cd ' + basedir + '; setenv NSM2_PORT ' + port \
        + '; rf_outputserver ' + get_configpath(conffile) \
        + ' > & collector/nsmlog.log" '
    print cmd
    p = subprocess.Popen(cmd, shell=True)
    time.sleep(1)


# Stop outputserver

def stop_outputserver(conffile):
    opshost = get_rfgetconf(conffile, 'collector', 'ctlhost')
    basedir = get_rfgetconf(conffile, 'system', 'execdir_base')
    unit = get_rfgetconf(conffile, 'system', 'unitname')
    rbufin = get_rfgetconf(conffile, 'collector', 'ringbufin')
    rbufout = get_rfgetconf(conffile, 'collector', 'ringbufout')
    rbufinname = unit + ':' + rbufin
    rbufoutname = unit + ':' + rbufout
    shmname = unit + ':collector'
    p = subprocess.Popen('rfcommand ' + conffile + ' collector RC_ABORT', shell=True)
    p.wait()
    pidfile = basedir + '/collector/pid.data'
    for pid in open(pidfile, 'r'):
        cmd = 'ssh ' + opshost + ' "kill ' + pid + '; removerb ' + rbufinname \
              + '; removerb ' + rbufoutname + '; removeshm ' + shmname \
              + '; clear_basf2_ipc"'
        print cmd
        p = subprocess.Popen(cmd, shell=True)
        p.wait()


# Start event procesor

def run_eventprocessor(conffile):
    hostbase = get_rfgetconf(conffile, 'processor', 'ctlhostbase')
    nodebase = get_rfgetconf(conffile, 'processor', 'nodebase')
    basedir = get_rfgetconf(conffile, 'system', 'execdir_base')
    port = get_rfgetconf(conffile, 'system', 'nsmport')
    nnodes = int(get_rfgetconf(conffile, 'processor', 'nnodes'))
    procid = int(get_rfgetconf(conffile, 'processor', 'idbase'))
    badlist = get_rfgetconf(conffile, 'processor', 'badlist')
    id = int(get_rfgetconf(conffile, 'processor', 'idbase'))

    for i in range(procid, procid + nnodes):
        nodeid = '%2.2d' % i
        if badlist.find(nodeid) == -1:
            evphost = hostbase + nodeid
            nodename = nodebase + nodeid
            if not os.path.exists(basedir + '/evp_' + nodename):
                os.mkdir(basedir + '/evp_' + nodename)
            cmd = 'ssh ' + evphost + ' "cd ' + basedir + '; setenv NSM2_PORT ' \
                + port + '; rf_eventprocessor ' + get_configpath(conffile) \
                + ' > & evp_' + nodename + '/nsmlog.log" '
            print cmd
            p = subprocess.Popen(cmd, shell=True)
            time.sleep(1)


# Stop event procesor

def stop_eventprocessor(conffile):
    hostbase = get_rfgetconf(conffile, 'processor', 'ctlhostbase')
    nodebase = get_rfgetconf(conffile, 'processor', 'nodebase')
    basedir = get_rfgetconf(conffile, 'system', 'execdir_base')
    port = get_rfgetconf(conffile, 'system', 'nsmport')
    nnodes = int(get_rfgetconf(conffile, 'processor', 'nnodes'))
    procid = int(get_rfgetconf(conffile, 'processor', 'idbase'))
    badlist = get_rfgetconf(conffile, 'processor', 'badlist')
    id = int(get_rfgetconf(conffile, 'processor', 'idbase'))

    unit = get_rfgetconf(conffile, 'system', 'unitname')
    rbufin = get_rfgetconf(conffile, 'collector', 'ringbufin')
    rbufout = get_rfgetconf(conffile, 'collector', 'ringbufout')
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
    dqmhost = get_rfgetconf(conffile, 'dqmserver', 'ctlhost')
    basedir = get_rfgetconf(conffile, 'system', 'execdir_base')
    port = get_rfgetconf(conffile, 'system', 'nsmport')
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
    dqmhost = get_rfgetconf(conffile, 'dqmserver', 'ctlhost')
    basedir = get_rfgetconf(conffile, 'system', 'execdir_base')
    p = subprocess.Popen('rfcommand ' + conffile + ' dqmserver RC_ABORT', shell=True)
    p.wait()
    pidfile = basedir + '/dqmserver/pid.data'
    for pid in open(pidfile, 'r'):
        cmd = 'ssh ' + dqmhost + ' "kill ' + pid + '"'
        print cmd
        p = subprocess.Popen(cmd, shell=True)
        p.wait()


# Run roisender

def run_roisender(conffile):
    roihost = get_rfgetconf(conffile, 'roisender', 'ctlhost')
    basedir = get_rfgetconf(conffile, 'system', 'execdir_base')
    port = get_rfgetconf(conffile, 'system', 'nsmport')
    if not os.path.exists(basedir + '/roisender'):
        os.mkdir(basedir + '/roisender')
    cmd = 'ssh ' + roihost + ' "cd ' + basedir + '; setenv NSM2_PORT ' + port \
        + '; rf_roisender ' + get_configpath(conffile) \
        + ' > & roisender/nsmlog.log" '
    print cmd
    p = subprocess.Popen(cmd, shell=True)
    time.sleep(1)


# Stop roisender

def stop_roisender(conffile):
    roihost = get_rfgetconf(conffile, 'roisender', 'ctlhost')
    basedir = get_rfgetconf(conffile, 'system', 'execdir_base')
    unit = get_rfgetconf(conffile, 'system', 'unitname')
    shmname = unit + ':roisender'
    p = subprocess.Popen('rfcommand ' + conffile + ' roisender RC_ABORT', shell=True)
    p.wait()
    pidfile = basedir + '/roisender/pid.data'
    for pid in open(pidfile, 'r'):
        cmd = 'ssh ' + roihost + ' "kill ' + pid + '; removeshm ' + shmname + '"'
        print cmd
        p = subprocess.Popen(cmd, shell=True)
        p.wait()


# Run local master

def run_master(conffile):
    masterhost = get_rfgetconf(conffile, 'master', 'ctlhost')
    basedir = get_rfgetconf(conffile, 'system', 'execdir_base')
    port = get_rfgetconf(conffile, 'system', 'nsmport')
    if not os.path.exists(basedir + '/master'):
        os.mkdir(basedir + '/master')
    cmd = 'ssh ' + masterhost + ' "cd ' + basedir + '; setenv NSM2_PORT ' \
        + port + '; rf_master_local ' + get_configpath(conffile) \
        + ' > & master/nsmlog.log" '
    print cmd
    p = subprocess.Popen(cmd, shell=True)
    time.sleep(1)


# Stop local master

def stop_master(conffile):
    masterhost = get_rfgetconf(conffile, 'master', 'ctlhost')
    basedir = get_rfgetconf(conffile, 'system', 'execdir_base')
#    p = subprocess.Popen ( "rfcommand " + conffile + " master RF_UNCONFIGURE", shell=True );
#    p.wait();
    pidfile = basedir + '/master/pid.data'
    for pid in open(pidfile, 'r'):
        cmd = 'ssh ' + masterhost + ' "kill ' + pid + '"'
        print cmd
        p = subprocess.Popen(cmd, shell=True)
        p.wait()


def start_rfarm_components(conffile):
    run_eventprocessor(conffile)
    run_outputserver(conffile)
    run_eventserver(conffile)
    run_dqmserver(conffile)
    run_roisender(conffile)


# Stop RFARM components

def stop_rfarm_components(conffile):
    stop_roisender(conffile)
    stop_dqmserver(conffile)
    stop_eventserver(conffile)
    stop_outputserver(conffile)
    stop_eventprocessor(conffile)


# Start RFARM local operation

def start_rfarm_local(conffile):
    start_rfarm_components(conffile)
    run_master(conffile)


# Stop RFARM local operation

def stop_rfarm_local(conffile):
    #    stop_eventprocessor(conffile)
    stop_rfarm_components(conffile)
    stop_master(conffile)
