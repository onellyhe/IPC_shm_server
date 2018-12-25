QT -= gui

CONFIG += c++11 console
CONFIG -= app_bundle

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += main.cpp \
    IPC_shm.cpp

HEADERS += \
    IPC_shm.hpp

#adding dynamic links of CAFFE and dependencies

#caffe
INCLUDEPATH += /home/onelly/tsq/caffe-master/include /home/onelly/tsq/caffe-master/build/src
LIBS += -L/home/onelly/tsq/caffe-master/build/lib
LIBS += -lcaffe

#cuda
INCLUDEPATH += /usr/local/cuda/include
LIBS += -L/usr/local/cuda/lib -L/usr/local/cuda/lib64 -L/usr/local/cuda/lib64/stubs
LIBS += -lcudart -lcublas -lcurand -lnvidia-ml

#opencv
LIBS += -lopencv_core -lopencv_highgui -lopencv_imgproc -lopencv_features2d# -lopencv_imgcodecs -lopencv_videoio

#other
LIBS += -lglog -lgflags -lprotobuf -lboost_system -lboost_filesystem -lboost_regex -lm -lhdf5_serial_hl -lhdf5_serial

#leveldb
LIBS += -lleveldb -lsnappy

#lmdb
LIBS += -llmdb

#linux
LIBS += -lboost_thread -lstdc++

#cudnn
LIBS += -lcudnn

#blas
#LIBS += -latlas
LIBS += -lopenblas
