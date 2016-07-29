# Run "qmake; make; make install" to compile and install the library on Unix systems.
TEMPLATE = lib
TARGET = QGCodeViewer
VERSION  = 0.1.0

CONFIG  += qt warn_on shared thread create_prl rtti

INCLUDEPATH += g2m

HEADERS     = \
    QGCodeViewer.h \
    g2m/canonLine.hpp    g2m/canonMotionless.hpp  g2m/gplayer.hpp        g2m/linearMotion.hpp   g2m/nanotimer.hpp \
    g2m/canonMotion.hpp  g2m/g2m.hpp              g2m/helicalMotion.hpp  g2m/machineStatus.hpp  g2m/point.hpp \
    g2m/gplayer.hpp	 lex_analyzer.hpp

SOURCES     = \
    QGCodeViewer.cpp \
    g2m/canonLine.cpp    g2m/canonMotionless.cpp  g2m/helicalMotion.cpp  g2m/machineStatus.cpp \
    g2m/canonMotion.cpp  g2m/g2m.cpp              g2m/linearMotion.cpp   g2m/nanotimer.cpp 	g2m/gplayer.cpp \
    lex_analyzer.cpp

LIBS += -lQGLViewer -lQGCodeEditor

QT      *= opengl xml gui core

DESTDIR  = $${PWD}

!isEmpty( QGCODEVIEWER_STATIC ) {
  CONFIG *= staticlib
}

unix {
        CONFIG -= debug debug_and_release
        CONFIG *= release

	# INCLUDE_DIR and LIB_DIR specify where to install the include files and the library.
	# Use qmake INCLUDE_DIR=... LIB_DIR=... , or qmake PREFIX=... to customize your installation.

        isEmpty( PREFIX ) {
                PREFIX_=/usr/local
        } else {
                PREFIX_=$${PREFIX}
        }

	isEmpty( LIB_DIR ) {
                LIB_DIR_ = $$[QT_INSTALL_LIBS]
	} else {
		isEmpty( PREFIX ) {
			LIB_DIR_ = $${LIB_DIR}
	        } else {
        	        LIB_DIR_ = $${PREFIX_}/lib
		}
        }

	isEmpty( INCLUDE_DIR ) {
               	INCLUDE_DIR_ = $$[QT_INSTALL_HEADERS]
	} else {
		isEmpty( PREFIX ) {
			INCLUDE_DIR_ = $${INCLUDE_DIR}
        	} else {
                	INCLUDE_DIR_ = $${PREFIX_}/include
	        }
	}

        isEmpty( DOC_DIR ) {
                macx|darwin-g++ {
                        isEmpty( PREFIX ) {
                                DOC_DIR = $${PWD}/Library/Developer/Shared/Documentation/QGCodeViewer
                        } else {
                                DOC_DIR = $${PREFIX}/Shared/Documentation/QGCodeViewer
                        }
                } else {
                        DOC_DIR = $${PREFIX_}/share/doc/QGCodeViewer
                }
        }

        MOC_DIR = .moc
        OBJECTS_DIR = .obj

        # Adds a -P option so that "make install" as root creates files owned by root and links are preserved.
        # This is not a standard option, and it may have to be removed on old Unix flavors.
        !hpux {
                QMAKE_COPY_FILE = $${QMAKE_COPY_FILE} -P
        }

        # Make much smaller libraries (and packages) by removing debugging informations
        QMAKE_CFLAGS_RELEASE -= -g
        QMAKE_CXXFLAGS_RELEASE -= -g

        # install header
        include.path = $${INCLUDE_DIR_}/QGCodeViewer
        include.files = $${HEADERS}

        # install static library
        staticlib.extra = make -f Makefile.Release staticlib
        staticlib.path = $${LIB_DIR_}
        staticlib.files = lib$${TARGET}.a

        # install library
        target.path = $${LIB_DIR_}

	greaterThan(QT_MAJOR_VERSION, 4) {
    		features.path = $$[QT_HOST_DATA]/mkspecs/features
	} else {
		features.path = $$[QT_INSTALL_DATA]/mkspecs/features
	}
	features.files = $$PWD/features/qgcodeeiewer.prf

        # "make install" configuration options
        INSTALLS += target include features
}

# -------------------
# --  M a c O S X  --
# -------------------
macx|darwin-g++ {
        # This setting creates a Mac framework. Comment out this line to create a dylib instead.
        !staticlib: CONFIG *= lib_bundle

# TODO

}

build_pass:CONFIG(debug, debug|release) {
  unix: TARGET = $$join(TARGET,,,_debug)
  else: TARGET = $$join(TARGET,,,d)
}
