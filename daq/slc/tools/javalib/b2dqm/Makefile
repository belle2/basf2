JCC        = javac 
JAR        = jar
RM         = rm -rf
CP         = cp -f

TARGET     = b2dqm

MANIFEST  = Belle2DQMBrowser.mf

SRCDIR     = ./src
OUTDIR     = ./classes
JARDIR     = ./jars

MAIN       = src/b2dqm/java/Belle2DQMBrowser.java

JARFILE  = $(JARDIR)/b2dqm.jar
JAREXE  = $(JARDIR)/b2dqm-exe.jar

JAVALIB_PATH = $(BELLE2_DAQ_SLC_DIR)/tools/javalib

CLASSPATH  = $(JAVALIB_PATH)/b2daq/classes:$(SRCDIR)
SOURCEPATH = $(JAVALIB_PATH)/b2daq/src:$(SRCDIR)
JFLAG      = -classpath $(CLASSPATH) -d $(OUTDIR) -sourcepath $(SOURCEPATH)

WEBJARDIR = $(DCONLINE_WEB_HOME)/java/jars

all:
	@ echo "<< compiling $(MAIN) >>"
	@ if [ ! -d $(OUTDIR) ];then mkdir $(OUTDIR); fi
	@ $(JCC) $(JFLAG) $(MAIN)
	@ cp $(SRCDIR)/b2dqm/java/ui/*.png $(OUTDIR)/b2dqm/java/ui

jar : all 
	@ echo '<< creating jar file $(JARFILE) >>'
	@ if [ ! -d $(JARDIR) ];then mkdir $(JARDIR); fi
	@ $(JAR) cf $(JARFILE) -C $(OUTDIR) $(TARGET)
	@ $(JAR) uf $(JARFILE) -C $(JAVALIB_PATH)/b2daq/classes b2daq

exe : all 
	@ echo '<< creating jar file $(JAREXE) >>'
	@ if [ ! -d $(JARDIR) ];then mkdir $(JARDIR); fi
	@ $(JAR) cfm $(JAREXE) $(MANIFEST) -C $(OUTDIR) $(TARGET)
	@ $(JAR) uf $(JAREXE) -C $(JAVALIB_PATH)/b2daq/classes b2daq

clean:
	@ echo '<< cleaning directory >>'
	@ $(RM) *~ $(OUTDIR) $(BINDIR) $(JARDIR) $(CONDIR)

install: $(JARTARGET)
	@ echo '<< installing jar files to html directory >>'
	@ $(CP) $(JARTARGET) $(WEBJARDIR)

ant:
	@ ant