#!/bin/bash
#Add gbasf2 path to the environment PATH
gbasf2_root=`pwd`/..
if [ -n "${PATH}" ]; then
  export PATH=${gbasf2_root}/bin:${PATH}
else
  export PATH=${gbasf2_root}/bin
fi

