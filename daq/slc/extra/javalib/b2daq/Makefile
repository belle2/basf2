JCC = javac 
JAR = jar
RM = rm -rf
CP = cp -f

TARGET = b2daq

SRCDIR = ./src
OUTDIR = ./classes
BINDIR = ./bin
JARDIR = ./jars

JARTARGET = b2daq.jar

SRCS  = $(wildcard $(SRCDIR)/$(TARGET)/*.java)
SRCS += $(wildcard $(SRCDIR)/$(TARGET)/*/*.java)
SRCS += $(wildcard $(SRCDIR)/$(TARGET)/*/*/*.java)
SRCS += $(wildcard $(SRCDIR)/$(TARGET)/*/*/*/*.java)
SRCS += $(wildcard $(SRCDIR)/$(TARGET)/*/*/*/*/*.java)

OBJS = $(patsubst $(SRCDIR)/%.java, $(OUTDIR)/%.class, $(SRCS))

CLASSPATH = $(SRCDIR)

JFLAG = -classpath $(CLASSPATH) -d $(OUTDIR) -sourcepath $(SRCDIR)

all : $(OBJS) 

jar : 
	@ echo '<< creating jar file $@ >>'
	@ mkdir -p $(JARDIR)

$(OBJS) : $(OUTDIR)/%.class: $(SRCDIR)/%.java
	@ echo '<< compiling $@ >>'
	@ mkdir -p $(OUTDIR)
	@ $(JCC) $(JFLAG) $<

clean:
	@ echo '<< cleaning directory >>'
	@ $(RM) *~ $(OUTDIR) $(BINDIR) $(JARDIR) $(CONDIR)

