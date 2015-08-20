#!/bin/sh

# Git seems to prepend /usr/bin to the PATH sometimes which hides our python.
# So let's call the correct python interpreter if we know it anyway
# (This is only really a problem on SL5, which has Python 2.4)

DIRNAME=`dirname $0`
PYTHON=python
if [ -n "$VIRTUAL_ENV" ]; then
    PYTHON=$VIRTUAL_ENV/bin/python
fi

$PYTHON $DIRNAME/svnexternals.py "$@"
