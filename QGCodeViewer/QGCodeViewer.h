#ifndef QGCODEVIEWER_H
#define QGCODEVIEWER_H

#include <QGLViewer/qglviewer.h>
#include <QGLViewer/manipulatedFrame.h>
#include <QGCodeEditor.h>
#include <QMutex>

#include "g2m/g2m.hpp"

using namespace qglviewer;
using namespace g2m;

class QGCodeViewer : public QGLViewer
{
    Q_OBJECT;
    Q_PROPERTY(bool autoZoom READ autoZoom WRITE setAutoZoom RESET unsetAutoZoom);

public:
    QGCodeViewer(QWidget *parent = NULL);
    ~QGCodeViewer();

    void resetCamView();

    bool autoZoom()const{ return _autoZoom; };
    void setAutoZoom(bool autoZoom){ _autoZoom = autoZoom; update(); };
    void unsetAutoZoom(){ _autoZoom = true; update(); };

    QString rs274;
    QString tooltable;
    QString gcodefile;
    
    QGCodeEditor *gcode_editor;
    
public slots:
    virtual void close();
    
    virtual void changedGCode();
    
    virtual void onOpenGCode(QString filename);

    virtual void appendCanonLine(canonLine *l);
    
    virtual void toggleAutoZoom() { setAutoZoom(!autoZoom()); }
    
    virtual void clear();

    virtual void update();

    //void keyPressEvent(QKeyEvent *e);
  
signals:
    void setRS274(QString );
    void setToolTable(QString );
    void setGcodeFile(QString );
    void interpret();
    
protected:
    void init();
    void initializeGL();
      
    virtual void draw();
    virtual void fastDraw();
    virtual void postDraw();

    void drawObjects(bool simplified);

private:
    Vec initialCameraPosition;
    Quaternion initialCameraOrientation;

    QString home_dir, openFile;

  
    g2m::g2m *g2m;

    QMutex mutex;

    std::vector<g2m::canonLine*> lines;

    bool dirty;
    double aabb[6];
    bool _autoZoom;
};

#endif // VIEW_H

