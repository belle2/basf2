name=`echo ${1,,}`
bigname=`echo ${1^^}`
classname=`echo ${name^}`

dir=${BELLE2_LOCAL_DIR}/daq/slc/apps

mkdir $dir/${name}d/
mkdir $dir/${name}d/include
mkdir $dir/${name}d/src
mkdir $dir/${name}d/tools

sed "s/Template/$classname/g" $dir/templated/include/TemplateCallback.h | \
sed "s/template/${name}/g" > $dir/${name}d/include/${classname}Callback.h 

sed "s/Template/$classname/g" $dir/templated/src/TemplateCallback.cc | \
sed "s/template/${name}/g"  > $dir/${name}d/src/${classname}Callback.cc 

sed "s/Template/$classname/g" $dir/templated/tools/templated.cc | \
sed "s/template/${name}/g" > $dir/${name}d/tools/${name}d.cc

sed "s/template/${name}/g" $dir/templated/Makefile > $dir/${name}d/Makefile
sed "s/template/${name}/g" $dir/templated/SConscript > $dir/${name}d/SConscript
sed "s/template/${name}/g" $dir/templated/tools/Makefile > $dir/${name}d/tools/Makefile
sed "s/template/${name}/g" $dir/templated/tools/SConscript > $dir/${name}d/tools/SConscript

dir=${BELLE2_LOCAL_DIR}/daq/slc/data/config

sed "s/template/${name}/g" $dir/template.conf | sed "s/TEMPLATE/$bigname/g" > $dir/${name}.conf

dir=${BELLE2_LOCAL_DIR}/daq/slc/data/database

sed "s/template/${name}/g" $dir/templated.conf | sed "s/TEMPLATE/$bigname/g" > $dir/${name}d.conf
