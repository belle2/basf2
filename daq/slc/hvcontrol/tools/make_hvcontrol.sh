name=`echo ${1,,}`
bigname=`echo ${1^^}`
classname=`echo ${name^}`HV

dir=${BELLE2_DAQ_SLC}/hvcontrol

mkdir $dir/$name/
mkdir $dir/$name/include
mkdir $dir/$name/src
mkdir $dir/$name/tools

sed "s/TemplateHV/$classname/g" $dir/template/include/TemplateHVControlCallback.h | \
sed "s/template/$name/g" > $dir/$name/include/${classname}ControlCallback.h 

sed "s/TemplateHV/$classname/g" $dir/template/src/TemplateHVControlCallback.cc | \
sed "s/template/$name/g"  > $dir/$name/src/${classname}ControlCallback.cc 

sed "s/TemplateHV/$classname/g" $dir/template/tools/templatehvd.cc | \
sed "s/template/$name/g" > $dir/$name/tools/${name}hvd.cc

sed "s/template/$name/g" $dir/template/Makefile > $dir/$name/Makefile
sed "s/template/$name/g" $dir/template/tools/Makefile > $dir/$name/tools/Makefile
sed "s/template/$name/g" $dir/template/tools/SConscript > $dir/$name/tools/SConscript

dir=${BELLE2_DAQ_SLC}/data/hvcontrol

sed "s/template/$name/g" $dir/template.conf | sed "s/TEMPLATE/$bigname/g" > $dir/$name.conf
sed "s/template/$name/g" $dir/template-input.conf | sed "s/TEMPLATE/$bigname/g" > $dir/$name-input.conf
