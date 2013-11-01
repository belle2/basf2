.PHONY : all clean base system nsm database xml dqm

SRCDIR=$(B2SLC_PATH)/src
INCDIR=$(B2SLC_PATH)/include
LIBDIR=$(B2SLC_PATH)/lib

all: base system nsm

full: all xml database

base:
	@ make -C base
	@ if [ ! -e $(INCDIR)/base ]; then ln -s $(SRCDIR)/base/include $(INCDIR)/base; fi
	@ if [ ! -e ../lib/libdaq_slc_base.so ]; then ln -s $(SRCDIR)/base/lib/libdaq_slc_base.so ../lib/; fi

system:
	@ make -C system
	@ if [ ! -e $(INCDIR)/system ]; then ln -s $(SRCDIR)/system/include $(INCDIR)/system; fi
	@ if [ ! -e ../lib/libdaq_slc_system.so ]; then ln -s $(SRCDIR)/system/lib/libdaq_slc_system.so ../lib/; fi

xml:
	@ make -C xml
	@ if [ ! -e $(INCDIR)/xml ]; then ln -s $(SRCDIR)/xml/include $(INCDIR)/xml; fi
	@ if [ ! -e ../lib/libdaq_slc_xml.so ]; then ln -s $(SRCDIR)/xml/lib/libdaq_slc_xml.so ../lib/; fi

database:
	@ make -C database
	@ if [ ! -e $(INCDIR)/database ]; then ln -s $(SRCDIR)/database/include $(INCDIR)/database; fi
	@ if [ ! -e ../lib/libdaq_slc_database.so ]; then ln -s $(SRCDIR)/database/lib/libdaq_slc_database.so ../lib/; fi

nsm:
	@ make -C nsm
	@ if [ ! -e $(INCDIR)/nsm ]; then ln -s $(SRCDIR)/nsm/include $(INCDIR)/nsm; fi
	@ if [ ! -e ../lib/libdaq_slc_nsm.so ]; then ln -s $(SRCDIR)/nsm/lib/libdaq_slc_nsm.so ../lib/; fi

dqm:
	@ make -C dqm
	@ if [ ! -e $(INCDIR)/dqm ]; then ln -s $(SRCDIR)/dqm/include $(INCDIR)/dqm; fi
	@ if [ ! -e ../lib/libdaq_slc_dqm.so ]; then ln -s $(SRCDIR)/dqm/lib/libdaq_slc_dqm.so ../lib/; fi

clean:
	@ make -C base clean
	@ make -C system clean
	@ make -C nsm clean
	@ make -C xml clean
	@ make -C database clean
	@ make -C dqm clean
	@ rm -rf $(INCDIR)/* $(LIBDIR)/*.so
	@ rm -rf *~ \#*~ .*~

# LocalWords:  gui
