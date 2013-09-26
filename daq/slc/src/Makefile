.PHONY : all clean util node system nsm db xml runcontrol dqm

SRCDIR=$(B2SLC_PATH)/src
INCDIR=$(B2SLC_PATH)/include
LIBDIR=$(B2SLC_PATH)/lib

all: util system node nsm runcontrol dqm

full: all xml db

util:
	@ make -C util
	@ if [ ! -e $(INCDIR)/util ]; then ln -s $(SRCDIR)/util/include $(INCDIR)/util; fi
	@ if [ ! -e ../lib/libB2SLCUTIL.so ]; then ln -s $(SRCDIR)/util/lib/libB2SLCUTIL.so ../lib/; fi

node:
	@ make -C node
	@ if [ ! -e $(INCDIR)/node ]; then ln -s $(SRCDIR)/node/include $(INCDIR)/node; fi
	@ if [ ! -e ../lib/libB2SLCNODE.so ]; then ln -s $(SRCDIR)/node/lib/libB2SLCNODE.so ../lib/; fi

system:
	@ make -C system
	@ if [ ! -e $(INCDIR)/system ]; then ln -s $(SRCDIR)/system/include $(INCDIR)/system; fi
	@ if [ ! -e ../lib/libB2SLCSYS.so ]; then ln -s $(SRCDIR)/system/lib/libB2SLCSYS.so ../lib/; fi

xml:
	@ make -C xml
	@ if [ ! -e $(INCDIR)/xml ]; then ln -s $(SRCDIR)/xml/include $(INCDIR)/xml; fi
	@ if [ ! -e ../lib/libB2SLCXML.so ]; then ln -s $(SRCDIR)/xml/lib/libB2SLCXML.so ../lib/; fi

db:
	@ make -C db
	@ if [ ! -e $(INCDIR)/db ]; then ln -s $(SRCDIR)/db/include $(INCDIR)/db; fi
	@ if [ ! -e ../lib/libB2SLCDB.so ]; then ln -s $(SRCDIR)/db/lib/libB2SLCDB.so ../lib/; fi

nsm:
	@ make -C nsm
	@ if [ ! -e $(INCDIR)/nsm ]; then ln -s $(SRCDIR)/nsm/include $(INCDIR)/nsm; fi
	@ if [ ! -e ../lib/libB2SLCNSM.so ]; then ln -s $(SRCDIR)/nsm/lib/libB2SLCNSM.so ../lib/; fi

runcontrol:
	@ make -C runcontrol
	@ if [ ! -e $(INCDIR)/runcontrol ]; then ln -s $(SRCDIR)/runcontrol/include $(INCDIR)/runcontrol; fi
	@ if [ ! -e ../lib/libB2SLCRC.so ]; then ln -s $(SRCDIR)/runcontrol/lib/libB2SLCRC.so ../lib/; fi

gui:
	@ make -C gui
	@ if [ ! -e $(INCDIR)/gui ]; then ln -s $(SRCDIR)/gui/include $(INCDIR)/gui; fi
	@ if [ ! -e ../lib/libB2SLCGUI.so ]; then ln -s $(SRCDIR)/gui/lib/libB2SLCGUI.so ../lib/; fi

dqm:
	@ make -C dqm
	@ if [ ! -e $(INCDIR)/dqm ]; then ln -s $(SRCDIR)/dqm/include $(INCDIR)/dqm; fi
	@ if [ ! -e ../lib/libB2SLCDQM.so ]; then ln -s $(SRCDIR)/dqm/lib/libB2SLCDQM.so ../lib/; fi

clean:
	@ make -C util clean
	@ make -C node clean
	@ make -C system clean
	@ make -C nsm clean
	@ make -C runcontrol clean
	@ make -C xml clean
	@ make -C db clean
	@ make -C dqm clean
	@ rm -rf $(INCDIR)/* $(LIBDIR)/*.so
	@ rm -rf *~ \#*~ .*~

# LocalWords:  gui
