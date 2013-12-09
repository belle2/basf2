JCC        = javac 
JAR        = jar
RM         = rm -rf
CP         = cp -f

TARGET     = b2logger

MANIFEST  = Belle2LogViewer.mf

SRCDIR     = ./src
OUTDIR     = ./classes
JARDIR     = ./jars

MAIN       = src/b2logger/java/Belle2LogViewer.java

JARFILE  = $(JARDIR)/b2logger.jar
JAREXE  = $(JARDIR)/b2logger-exe.jar

JAVALIB_PATH = $(BELLE2_LOCAL_DIR)/daq/slc/extra/javalib

CLASSPATH  = $(JAVALIB_PATH)/b2daq/classes:$(SRCDIR)
SOURCEPATH = $(JAVALIB_PATH)/b2daq/src:$(SRCDIR)
JFLAG      = -classpath $(CLASSPATH) -d $(OUTDIR) -sourcepath $(SOURCEPATH)

WEBJARDIR = $(DCONLINE_WEB_HOME)/java/jars

all:
	@ echo "<< compiling $(MAIN) >>"
	@ if [ ! -d $(OUTDIR) ];then mkdir $(OUTDIR); fi
	@ $(JCC) $(JFLAG) $(MAIN)
	@ cp $(SRCDIR)/b2logger/java/ui/*.png $(OUTDIR)/b2logger/java/ui
	@ cp $(SRCDIR)/b2logger/java/ui/*.wav $(OUTDIR)/b2logger/java/ui

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