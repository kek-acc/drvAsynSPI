TOP=../..

include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE
#=============================

#==================================================
# build a support library

LIBRARY_IOC += drvAsynSPI

ifdef STREAM_WORKAROUND
USR_CXXFLAGS += -DSTREAM_WORKAROUND
endif

INC += asynSPI.h

# xxxRecord.h will be created from xxxRecord.dbd
#DBDINC += xxxRecord
# install drvAsynSPI.dbd into <top>/dbd
DBD += drvAsynSPI.dbd

# specify all source files to be compiled and added to the library
drvAsynSPI_SRCS += drvAsynSPI.cpp

drvAsynSPI_LIBS += $(EPICS_BASE_IOC_LIBS) asyn

#===========================

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

