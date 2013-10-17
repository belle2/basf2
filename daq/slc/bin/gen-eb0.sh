#!/bin/bash
DATE=$( date )
dest=
for cpr in $@
do
dest="$dest $cpr:33000"
done
cat > /home/usr/b2daq/eb/eb0.sh <<EOT
#!/bin/sh
#$DATE
cd /home/usr/b2daq/eb
./eb0 $dest
EOT                    