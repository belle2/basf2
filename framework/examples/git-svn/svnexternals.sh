#!/bin/sh

# Git seems to prepend /usr/bin to the PATH sometimes which hides our python.
# So let's call the correct python interpreter if we know it anyway
# (This is only really a problem on SL5, which has Python 2.4)

DIRNAME=`dirname $0`
PYTHON=python3
if [ -n "$BELLE2_EXTERNALS_DIR" ]; then
    PYTHON=$BELLE2_EXTERNALS_DIR/$BELLE2_ARCH/common/bin/python3
fi
if [ -n "$VIRTUAL_ENV" ] && [ -e "$VIRTUAL_ENV/bin/python3" ]; then
    PYTHON=$VIRTUAL_ENV/bin/python3
fi

$PYTHON $DIRNAME/svnexternals.py "$@"
