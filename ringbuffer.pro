TEMPLATE = app
TARGET = 
DEPENDPATH += . \
	src/lib \
	src/test \
	include
INCLUDEPATH += . include

# Input
HEADERS += include/ringbuffer/ringbuffer.h
SOURCES += src/lib/ringbuffer.cpp \
	src/test/test_seq.cpp \
	src/test/test_par.cpp

OTHER_FILES += src/lib/CMakeLists.txt \
	src/test/CMakeLists.txt \
	src/CMakeLists.txt \
	src/ringbuffer.pc.in \
	CMakeLists.txt \
	src/config.h.in
