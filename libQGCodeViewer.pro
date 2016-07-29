CONFIG       += ordered
TEMPLATE      = subdirs
SUBDIRS       = QGCodeViewer DesignerPlugin

QT_VERSION=$$[QT_VERSION]

contains( QT_VERSION, "^5.*" ) {
        cache()
}
