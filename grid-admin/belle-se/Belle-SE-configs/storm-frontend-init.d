#!/bin/sh

# Comments to support chkconfig on RedHat Linux
# chkconfig: 2345 94 20
# description: StoRM FrontEnd server

# StoRM Frontend start script

prog="storm-frontend"
PIDDIR=/var/run
SUBSYSDIR=/var/lock/subsys
PIDFILE=$PIDDIR/$prog.pid

FRONTEND_DAEMON=/opt/storm/frontend/sbin/$prog
CONFIGURATION_FILE=/opt/storm/frontend/etc/storm-frontend.conf

if [ -z $STORM_FE_USER ]; then
    #XXX STORM_FE_USER=storm
    STORM_FE_USER=root
fi

if [ -z $GRIDMAPDIR ]; then
    export GRIDMAPDIR=/etc/grid-security/gridmapdir
fi

if [ -z $GRIDMAP ]; then
    export GRIDMAP=/etc/grid-security/grid-mapfile
fi

SUCCESS=0
ERROR=1
TRUE=1
FALSE=0

get_pid() {
   if [ -e $PIDFILE ]; then
       PID=`cat $PIDFILE`
   else
       return $ERROR
   fi
   return $SUCCESS
}

check_if_running() {
    get_pid
    if [ $? -ne $SUCCESS ]; then
        return $FALSE # not running
    fi
    IS_UP=`netstat -lp | grep $PID`
    if [ -z "$IS_UP" ]; then
        return $FALSE  # not running
    fi
    return $TRUE       # is running
}


# Blindly attempt to create useful directories
[ ! -d $PIDDIR ] && mkdir -p $PIDDIR >& /dev/null
#[ ! -d $SUBSYSDIR ] && mkdir -p $SUBSYSDIR >& /dev/null

RETVAL=0

case "$1" in
  start)
    # Script must be launched as root
    if [ ! `whoami` = "root" ]; then
        echo "Error: you must be root to run this script."
        exit 1
    fi

    echo -n "Starting $prog as user \"$STORM_FE_USER\"... "

    # Check if another instance is already running
    get_pid
    if [ $? -eq $SUCCESS ]; then
        check_if_running
        if [ $? -eq $TRUE ]; then
            echo "already running."
            exit 1
        else
            # There's an old PIDFILE (maybe previous instance was crashed)
            # and it must be removed
            rm -f $PIDFILE
        fi
    fi
    
    # Make sure the FE can write the logfile and save the proxies
    # and set environment variable
    if [ "root" != $STORM_FE_USER ]; then
        mkdir -p /opt/storm/frontend/var
        chown ${STORM_FE_USER}:${STORM_FE_USER} /opt/storm/frontend/var
        STORM_USER_CERT_DIR=/etc/grid-security/$STORM_FE_USER
        if [ -d $STORM_USER_CERT_DIR ]; then
            export X509_USER_CERT=$STORM_USER_CERT_DIR/hostcert.pem
            export X509_USER_KEY=$STORM_USER_CERT_DIR/hostkey.pem
        else
            echo "Error: Directory $STORM_USER_CERT_DIR does not exists. You should:"
            echo "1. make sure the user \"$STORM_FE_USER\" exists"
            echo "2. create that directory and put there hostcert.pem and hostkey.pem"
            echo "3. make sure user \"$STORM_FE_USER\" has the rights to read that files."
            exit 1
        fi
    fi

    # Launch the frontend
    su -m -s "/bin/bash" -c "$FRONTEND_DAEMON -c $CONFIGURATION_FILE" $STORM_FE_USER

    if [ $? -eq 0 ]; then
        pid=`ps -eo pid,ppid,comm | grep " 1 $prog$" | awk '{print $1}'`
        echo $pid > $PIDFILE
        RETVAL=0
        echo "ok."
    else
        echo "ERROR: not started (see the log file for the details)."
        RETVAL=1
    fi
    ;;
  stop)
    check_if_running
    if [ $? -eq $FALSE ]; then
        echo "$prog already stopped"
        RETVAL=0
    else
        echo -n "Stopping $prog.."
        kill -2 `cat $PIDFILE` >& /dev/null
        if [ $? -eq 0 ]; then
            check_if_running
            while [ $? -eq $TRUE ]; do
                echo -n "."
                sleep 1
                check_if_running
            done
            rm -f $PIDFILE
            RETVAL=0
            echo " ok"
        else
            RETVAL=1
            echo " error"
        fi
    fi            
    ;;
  restart)
    $0 stop
    sleep 1
    if [ $? -eq 0 ]; then
        $0 start
    fi
    RETVAL=$?
    ;;
  status)
    check_if_running
    if [ $? -eq $TRUE ]; then
        echo "$prog (pid $PID) is running..."
        RETVAL=0
    else
        echo "$prog is NOT running"
        RETVAL=1
    fi
    ;;
  *)
    echo "Usage: $0 {start|stop|status|restart}"
    RETVAL=1
    ;;
esac

exit $RETVAL
