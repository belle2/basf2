name=`echo ${1,,}`
bigname=`echo ${1^^}`
classname=`echo ${name^}`

dir=${BELLE2_LOCAL_DIR}/daq/slc/apps

mkdir $dir/${name}d/
mkdir $dir/${name}d/include
mkdir $dir/${name}d/src
mkdir $dir/${name}d/tools

sed "s/RCTemplate/$classname/g" $dir/rctemplated/include/RCTemplateCallback.h | \
sed "s/rctemplate/${name}/g" > $dir/${name}d/include/${classname}Callback.h 

sed "s/RCTemplate/$classname/g" $dir/rctemplated/src/RCTemplateCallback.cc | \
sed "s/rctemplate/${name}/g"  > $dir/${name}d/src/${classname}Callback.cc 

sed "s/RCTemplate/$classname/g" $dir/rctemplated/tools/rctemplated.cc | \
sed "s/rctemplate/${name}/g" > $dir/${name}d/tools/${name}d.cc

sed "s/rctemplate/${name}/g" $dir/rctemplated/Makefile > $dir/${name}d/Makefile
sed "s/rctemplate/${name}/g" $dir/rctemplated/SConscript > $dir/${name}d/SConscript
sed "s/rctemplate/${name}/g" $dir/rctemplated/tools/Makefile > $dir/${name}d/tools/Makefile
sed "s/rctemplate/${name}/g" $dir/rctemplated/tools/SConscript > $dir/${name}d/tools/SConscript

dir=${BELLE2_LOCAL_DIR}/daq/slc/data/config

sed "s/rctemplate/${name}/g" $dir/rctemplate.conf | sed "s/RCTEMPLATE/$bigname/g" > $dir/${name}.conf

dir=${BELLE2_LOCAL_DIR}/daq/slc/data/database

sed "s/rctemplate/${name}/g" $dir/rctemplated.conf | sed "s/RCTEMPLATE/$bigname/g" > $dir/${name}d.conf
