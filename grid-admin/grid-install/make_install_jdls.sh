#!/bin/bash
# Tom Fifield 20101-12
# Makes a JDL for every CE listed in the belle VO, then a DIRAC JDL for it
SOFTWARE_VERSION=$1
VO=${2:-"belle2"}
sites=`lcg-infosites -v 1 --vo belle ce | awk -F\: '{ print $1}' | sort |uniq`

for i in $sites
do
cat  << EOF > jdls/$i.jdl
 Arguments="$SOFTWARE_VERSION";
 StdOutput="std.out";
 StdError="std.err";
 InputSandbox = {"install-$VO-software.sh", "dirac-platform.py"};
 Executable = "install-$VO-software.sh";
 OutputSandbox={"std.out","std.err"};
 GridRequiredCEs = "$i";   
 JobGroup = "installation";
 RetryCount = 0;
 Rank = other.GlueCEStateFreeCPUs;
 Type = "Job";
 JobType = "Normal";
 PilotType = "private";
EOF
done
