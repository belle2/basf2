FROM ubuntu:18.04

RUN apt-get update && apt-get install -y python binutils gcc g++ git make patch libperl-dev subversion bzip2 \
                                         xz-utils unzip libpng-dev libx11-dev libxext-dev libxpm-dev libxft-dev \
                                         libncurses-dev libssl-dev libreadline-dev lsof net-tools

RUN echo "export BELLE2_NO_TOOLS_CHECK=1; \
          cd /mnt/basf2/software; \
          source ../tools/b2setup > /dev/null 2> /dev/null; \
          cd /mnt/daq_slc; \
          source setenv > /dev/null 2> /dev/null; \
          cd /work; \
          export HOME=/work; \
          export USER=root; \
          export BELLE2_SLC_FILE=/work/slowcontrol.conf" >> /root/setup_basf2
RUN echo "source /root/setup_basf2" >> ~/.profile
RUN echo "source /root/setup_basf2" >> ~/.bashrc
CMD ["/work/scripts/start.sh"]
