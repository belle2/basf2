#!/usr/bin/env python
#

# Volker.Buege@cern.ch (VMB)
# Universitaet Karlsruhe
# 17.12.2008

# ToDo:
# 

# Changes:
#


import sys
import os
import time
import signal
import string
import popen2
import ConfigParser
from xml.dom.minidom import Document


#auskommentiert, da wir nicht auf cms server wollen, also auch kein pwd file haben
#pwfile = ConfigParser.ConfigParser()
#try:
#	pwfile.readfp(open('/home/cmssgm/vobox-tools/monitoring/qstatXMLdump.pwd'))
#except IOError:
#	sys.stdout.write('Could not find password file!')
#	sys.exit(1)
#
#theUploadPw = pwfile.get('upload','pw')

def getSeconds(inString):
    fields = string.split(inString,":")
    if len(fields) != 3:
        print "Wrong time format in def getSeconds"
        sys.exit(1)
    return (int(fields[0])*24+int(fields[1]))*60+int(fields[2])                       


def uploadFile(theFile):
    theServer = 'http://'+theUploadPw+'@ekphappyface.physik.uni-karlsruhe.de/~happyface/upload/in/uploadFile.php'
    theCommand = 'curl -F \"Datei=@'+theFile+'\" '+theServer
    if os.system(theCommand) == 0:
        print "qstatXMLdump.py: Upload successful: "+theFile
    else:
        print "qstatXMLdump.py: Upload error: "+theFile
        sys.exit(1)

def callWithTimeoutChristophe(timeout, *args):
    old = signal.getsignal(signal.SIGCHLD)
    try:
        signal.signal(signal.SIGCHLD, lambda x, y: 0)
        pid = os.spawnlp(os.P_NOWAIT, args[0], *args)
        time.sleep(timeout)
        xpid, status = os.waitpid(pid, os.WNOHANG)
        if xpid == 0:
            os.kill(pid, signal.SIGTERM)
            xpid, status = os.waitpid(pid, 0)
    finally:
        signal.signal(signal.SIGCHLD, old)

    return status



def callWithTimeout(timeout, command, arg=""):
    old = signal.getsignal(signal.SIGCHLD)
    try:
        signal.signal(signal.SIGCHLD, lambda x, y: 0)
        proc = popen2.Popen3([command,arg],True)
        pid = proc.pid
        time.sleep(timeout)
        xpid, status = os.waitpid(pid, os.WNOHANG)
        if xpid == 0:
            os.kill(pid, signal.SIGTERM)
            xpid, status = os.waitpid(pid, 0)
    finally:
        signal.signal(signal.SIGCHLD, old)

    return status,proc.fromchild.readlines()



if __name__ == '__main__':

    theQstatCommand = '/usr/pbs/bin/qstat -f'
    theLogFile      = '/tmp/qstat.log'
    theXMLFile      = '/tmp/qstat.xml'
#    theLockFile     = '/tmp/qstatXMLdump.lock'

    exprJobSummary  = ['all','belle']  #geaendert
    exprJobDetails  = ['belle']          #was muss hier hin? haben alle belle jobs, belle im namen?


# Check if process is already running
#    if os.path.exists(theLockFile):
#        print "qstatXMLdump.py: process locked by "+theLockFile
#        print "                 Exit with code 1"
#        sys.exit(1)
        

# Create lock file for this job
#    open(theLockFile,'w').close()


    theGlobalInfo = {}

    theGlobalInfo["qstatStart"] = time.strftime("%a, %d %b %Y, %H:%M:%S")
    print "qstatXMLdump.py: Starting qstat"

#    status,out = callWithTimeout(180,'qstat','-f')

    status = 0

    os.system(theQstatCommand+' > '+theLogFile)
    out = open(theLogFile).readlines()

    if status != 0 :
        print "qstatXMLdump.py: "+theQstatCommand+" with exitcode != 0."
        sys.exit(1)
    else:
        print "qstatXMLdump.py: "+theQstatCommand+" finished."

    theGlobalInfo["qstatEnd"]   = time.strftime("%a, %d %b %Y, %H:%M:%S")


    print "qstatXMLdump.py: start processing."
    
    theJobInfo = {}
    theJobId = ""
    i = 0
    for fileLine in out:
        fileLine = fileLine.replace("\n","")
        if fileLine.count('Job Id'):
            theJobId = fileLine
            theJobInfo[theJobId] = {}
            theJobInfo[theJobId]['id'] = fileLine.split(": ")[1]


        if fileLine.count('Job_Owner'):
            theSplit = fileLine.split(" = ")[1].split("@")
            theJobInfo[theJobId]['user'] = theSplit[0]
            theJobInfo[theJobId]['ce'] = theSplit[1]
            
        if fileLine.count('resources_used.cpupercent'):
            theJobInfo[theJobId]['cpupercent'] = fileLine.split(" = ")[1]

        if fileLine.count('resources_used.cput'):
            theJobInfo[theJobId]['cput'] = fileLine.split(" = ")[1]

        if fileLine.count('resources_used.walltime'):
            theJobInfo[theJobId]['walltime'] = fileLine.split(" = ")[1]

        if fileLine.count('job_state'):
            theJobInfo[theJobId]['job_state'] = fileLine.split(" = ")[1]

        if fileLine.count('queue'):
            theJobInfo[theJobId]['queue'] = fileLine.split(" = ")[1]

        if fileLine.count('ctime'):
            theJobInfo[theJobId]['ctime'] = fileLine.split(" = ")[1]

        if fileLine.count('exec_host'):
            theJobInfo[theJobId]['exec_host'] = fileLine.split(" = ")[1]


    for job in theJobInfo:
        if 'walltime' in theJobInfo[job].keys() and 'cput' in theJobInfo[job].keys():
            wallSec = getSeconds(theJobInfo[job]['walltime'])
            cputSec = getSeconds(theJobInfo[job]['cput'])
            cpuwallratio = 0

            if wallSec >= 180:
                cpuwallratio = int(round((float(cputSec)/wallSec),2)*100)
            else:
                if 'cpupercent' in theJobInfo[job].keys():
                    cpuwallratio = theJobInfo[job]['cpupercent']

        
            theJobInfo[job]['cpuwallratio'] = cpuwallratio

                

    jobSummary = {}
    theMinRatio = 10


    for expr in exprJobSummary:
        jobSummary[expr] = {}
        jobSummary[expr]['total'] = 0
        jobSummary[expr]['running'] = 0
        jobSummary[expr]['ratio'+str(theMinRatio)] = 0
        for job in theJobInfo:
            if expr.count('all') or theJobInfo[job]['user'].count(expr):
                jobSummary[expr]['total'] += 1
                if 'job_state' in theJobInfo[job].keys():
                    if theJobInfo[job]['job_state'] == "R":
                        jobSummary[expr]['running'] +=1
                        
                if 'cpuwallratio' in theJobInfo[job].keys():
                    if int(theJobInfo[job]['cpuwallratio']) <=theMinRatio:
                        jobSummary[expr]['ratio'+str(theMinRatio)] += 1



    print "qstatXMLdump.py: processing finished."
    print "qstatXMLdump.py: start xml output."
    

    doc = Document()
    
    batchInfo = doc.createElement("batchInfo")
    doc.appendChild(batchInfo)

    globalInfo = doc.createElement("GlobalInfo")
    for tag in theGlobalInfo:
        globalInfo.setAttribute(tag,str(theGlobalInfo[tag]))
    batchInfo.appendChild(globalInfo)
    


    # Only job details for cms jobs written out
    jobDetails = doc.createElement("jobDetails")
    batchInfo.appendChild(jobDetails)
    jobDetails.setAttribute('selection',string.join(exprJobDetails," "))

    for job in theJobInfo:
        userin = False
        for username in exprJobDetails:
            if theJobInfo[job]['user'].count(username):
                userin = True

        if userin:
            jobInfo = doc.createElement("job")
            for tag in theJobInfo[job]:
                jobInfo.setAttribute(tag,str(theJobInfo[job][tag]))

            jobDetails.appendChild(jobInfo)
   


    jobSum = doc.createElement("jobsummary")
    batchInfo.appendChild(jobSum)

    for vo in jobSummary:
        jobInfo = doc.createElement(vo)
        for entry in jobSummary[vo]:
            jobInfo.setAttribute(entry,str(jobSummary[vo][entry]))
 
        jobSum.appendChild(jobInfo)
    
       
    
    # Print our newly created XML
    # print doc.toprettyxml(indent="  ")

    outXMLfile = open(theXMLFile,'w')
    outXMLfile.write(doc.toprettyxml(indent="  "))
    outXMLfile.close()

    print "qstatXMLdump.py: xml output finished."

    
#    auskommentiert weil wir nicht auf cms server uploaden wollen
#    uploadFile(theXMLFile)
    
# Remove lock file    
#    os.remove(theLockFile)
    
