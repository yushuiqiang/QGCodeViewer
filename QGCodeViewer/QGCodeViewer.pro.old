TEMPLATE = lib
TARGET = QGCodeViewer
VERSION  = 0.1.0

QMAKE_CXXFLAGS_RELEASE += -O2
QMAKE_CXXFLAGS_DEBUG   += -O0

MOC_DIR = .moc
OBJECTS_DIR = .obj
#DESTDIR = .so

INCLUDEPATH += g2m

HEADERS     = \
    QGCodeViewer.h \
    g2m/canonLine.hpp    g2m/canonMotionless.hpp  g2m/gplayer.hpp        g2m/linearMotion.hpp   g2m/nanotimer.hpp \
    g2m/canonMotion.hpp  g2m/g2m.hpp              g2m/helicalMotion.hpp  g2m/machineStatus.hpp  g2m/point.hpp \
    g2m/gplayer.hpp 	 lex_analyzer.hpp
    

SOURCES     = \
    QGCodeViewer.cpp \
    g2m/canonLine.cpp    g2m/canonMotionless.cpp  g2m/helicalMotion.cpp  g2m/machineStatus.cpp \
    g2m/canonMotion.cpp  g2m/g2m.cpp              g2m/linearMotion.cpp   g2m/nanotimer.cpp 	g2m/gplayer.cpp \
    lex_analyzer.cpp

LIBS += -lQGLViewer -lQGCodeEditor

QT      *= opengl xml gui core


