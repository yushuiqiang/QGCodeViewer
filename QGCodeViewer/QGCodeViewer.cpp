#include "QGCodeViewer.h"

#include <limits>

#include <GL/glut.h>

#include <QKeyEvent>
#include <QDebug>
#include <QStandardPaths>

#include "canonMotion.hpp"


QGCodeViewer::QGCodeViewer(QWidget *parent) : QGLViewer(parent)  
{
  
    setAttribute(Qt::WA_DeleteOnClose);

    lines.reserve(20000);
    g2m = new g2m::g2m(); // g-code interpreter

    connect( this, SIGNAL( setGcodeFile(QString) ),     g2m, SLOT( setFile(QString)) );
    connect( this, SIGNAL( setRS274(QString) ),         g2m, SLOT( setInterp(QString)) );
    connect( this, SIGNAL( setToolTable(QString) ),     g2m, SLOT( setToolTable(QString)) );
    connect( this, SIGNAL( interpret() ),               g2m, SLOT( interpret_file() ) );
    
    connect( g2m, SIGNAL( signalCanonLine(canonLine*) ), this, SLOT( appendCanonLine(canonLine*) ));
    connect( g2m, SIGNAL( signalNCend() ),               this, SLOT( update() ) );
    connect( g2m, SIGNAL( signalError(QString) ),        this, SLOT( update() ) );
    
    home_dir = QStandardPaths::locate(QStandardPaths::HomeLocation, QString(), QStandardPaths::LocateDirectory);
    openFile = "";
}

QGCodeViewer::~QGCodeViewer() 
{
  // qDebug() << "QGCodeViewer::~QGCodeViewer()";
}

void QGCodeViewer::close()
{
  // qDebug() << "QGCodeViewer::close()";
    // savePrefs();
  QGLViewer::close();
}

void QGCodeViewer::clear()
{
    // reset bounding box
    aabb[0] = std::numeric_limits<double>::max();
    aabb[1] = std::numeric_limits<double>::max();
    aabb[2] = std::numeric_limits<double>::max();
    
    aabb[3] = std::numeric_limits<double>::min();
    aabb[4] = std::numeric_limits<double>::min();
    aabb[5] = std::numeric_limits<double>::min();

    setSceneRadius(1);
    
    lines.clear();
    dirty = false;
    //updateGL();
}

/**
GCodeViewer is designed to be used with a GCodeEditor widget to hold the
.ngc code, a pointer to which is passed during initialisation.
Then it can read directly from the editor and update the view rendered
as each charactor is changed.
 */

void QGCodeViewer::changedGCode()
{
    if(gcode_editor == NULL)
        {
        qDebug() << "ERROR:  QGCodeViewer::gcode_editor is not set";
        return;
        }
        
    if (gcode_editor->toPlainText().isEmpty()) 
        {
        clear();
        return;
        }

    QFile f(gcodefile);
    if(f.open( QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Text)) 
        {
        QTextStream out(&f);
        out << gcode_editor->toPlainText();
        f.close();

        clear();

        emit setRS274(rs274);
        emit setToolTable(tooltable);
        emit setGcodeFile(gcodefile);

        emit interpret();
        }
}


void QGCodeViewer::onOpenGCode(QString filename)
{
QFile fin(filename);
QFile fout(gcodefile);

    if (fin.open(QFile::ReadOnly | QFile::Text) &&  ( fout.open( QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Text) ) )
        {
        QTextStream ints(&fin);
        QTextStream outts(&fout);
        while(!ints.atEnd())
            outts <<  ints.readLine() << "\n";
        fin.close();
        fout.close();

        clear();
        emit setRS274(rs274);
        emit setToolTable(tooltable);
        emit setGcodeFile(openFile = gcodefile);

        emit interpret();
        }
}

void QGCodeViewer::resetCamView() 
{
    camera()->setPosition(initialCameraPosition);
    camera()->setOrientation(initialCameraOrientation);
    updateGL();
}

void QGCodeViewer::init() 
{
    setShortcut(DISPLAY_FPS, Qt::CTRL+Qt::Key_F);
    setShortcut(DRAW_GRID, 0);
    
    setManipulatedFrame(new ManipulatedFrame());

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glEnable(GL_DEPTH_TEST);
    glDrawBuffer(GL_BACK);
    //glDisable(GL_CULL_FACE);
    //glLineStipple(2, 0xFFFF);
    glDisable(GL_LIGHTING);
    glClearColor(0.25,0.25,0.25,0);
    //glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    //glLineWidth(3);
  
    //setGridIsDrawn();
    
    if (!restoreStateFromFile()) {
        showEntireScene();
    }
    
    setMouseBinding(Qt::AltModifier, Qt::RightButton, SHOW_ENTIRE_SCENE, true, Qt::MidButton);

    clear();

    //startAnimation();
}

void QGCodeViewer::initializeGL() 
{
    QGLViewer::initializeGL();
    camera()->setZNearCoefficient(0.000001);
    camera()->setZClippingCoefficient(1000.0);
}

///////////////////////////////////////////////////////////////////////

void QGCodeViewer::appendCanonLine(canonLine *l) 
{
    //QMutexLocker locker(&mutex);
    lines.push_back(l);
    dirty = true; // for updateGL()

    if (l->isMotion()) {
        Point pos1 = l->point(0);
        Point pos2 = l->point(1);

        double oaabbmin[3], oaabbmax[3];

        oaabbmin[0] = qMin(pos1.x, pos2.x);
        oaabbmin[1] = qMin(pos1.y, pos2.y);
        oaabbmin[2] = qMin(pos1.z, pos2.z);

        oaabbmax[0] = qMax(pos1.x, pos2.x);
        oaabbmax[1] = qMax(pos1.y, pos2.y);
        oaabbmax[2] = qMax(pos1.z, pos2.z);

        for (int i = 0; i < 3; ++i) {
            aabb[  i] = qMin(aabb[  i], oaabbmin[i]);
            aabb[3+i] = qMax(aabb[3+i], oaabbmax[i]);
        }

        //qDebug() << "getAABB()" << aabb[0] << aabb[1] << aabb[2] << aabb[3] << aabb[4] << aabb[5];

        Vec c((aabb[0] + aabb[3])/2, (aabb[1] + aabb[4])/2, (aabb[2] + aabb[5])/2);
        setSceneCenter(c);
    }
}

///////////////////////////////////////////////////////////////////////////

void QGCodeViewer::drawObjects(bool simplified = false) 
{
    Q_UNUSED(simplified);

    if (lines.size() == 0)
        return;
    
    //int nbSteps = 600;
    //int nbSub = 50;

//    QMutexLocker locker(&mutex);

    /*
    if (simplified) {
        nbSteps = 60;
        nbSub = 2;
        }*/

//    qDebug() << "lines.size: " << lines.size();

    double step_size = 0.1;

    if (simplified)
        step_size = 0.75;
    
    double inv_ds = 1.0 / step_size;

    int skip_dyn = 1;
    double line_width = 3.0;

    if (lines.size() > 10000) { // skip some segments in big gcode files
        skip_dyn = 2;
        line_width = 2.0; // thinner lines
    }

    if (lines.size() > 100000) {
        skip_dyn = 4;
        line_width = 1.0;
    }

    int skip = (simplified) ? skip_dyn * 2 : skip_dyn;

    //glEnable(GL_MULTISAMPLE);
    //glEnable(GL_LINE_SMOOTH);
    //glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    //glHint(GL_MULTISAMPLE_FILTER_HINT_NV, GL_NICEST);

    /*
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glHint (GL_LINE_SMOOTH_HINT, GL_DONT_CARE);
    */
    
    for (unsigned long i = 0; i < lines.size(); i+=skip) {
        g2m::canonLine *l = lines[i];
        if (l->isMotion()) {
            double move_length = l->length();
            int n_samples = std::max((int)(move_length * inv_ds ), 2);
            double interval_size = move_length /(double)(n_samples-1);
            
            if (l->getMotionType() == TRAVERSE) {
                glColor3f(0.0, 128.0/255.0 , 128.0/255.0);
                glLineWidth(1);
            } else {
                glColor3f(255.0/255.0, 215.0/255.0, 94.0/255.0);
                glLineWidth(line_width);
            }

            glBegin(GL_LINES);
            for (double m = 0; m <= n_samples-1; m+=0.5) {
                Point pos1 = l->point( (double)(m) * interval_size);
                glVertex3f(pos1.x, pos1.y, pos1.z);
                Point pos2 = l->point( std::min(1.0, (double)(m+0.5) * interval_size));
                glVertex3f(pos2.x, pos2.y, pos2.z);
            }
            glEnd();
        }
        
        //glDisable(GL_BLEND);
    }
    
    glLineWidth(2);
    glColor3f(0.0, 0.0/255.0 , 0.0/255.0);
    glBegin(GL_LINES);

    // Top
    glVertex3f(aabb[0], aabb[1], aabb[2]);
    glVertex3f(aabb[3], aabb[1], aabb[2]);
    glVertex3f(aabb[3], aabb[1], aabb[2]);
    glVertex3f(aabb[3], aabb[1], aabb[5]);
    glVertex3f(aabb[3], aabb[1], aabb[5]);
    glVertex3f(aabb[0], aabb[1], aabb[5]);
    glVertex3f(aabb[0], aabb[1], aabb[5]);
    glVertex3f(aabb[0], aabb[1], aabb[2]);
    
    // Bottom
    glVertex3f(aabb[0], aabb[4], aabb[2]);
    glVertex3f(aabb[3], aabb[4], aabb[2]);
    glVertex3f(aabb[3], aabb[4], aabb[2]);
    glVertex3f(aabb[3], aabb[4], aabb[5]);
    glVertex3f(aabb[3], aabb[4], aabb[5]);
    glVertex3f(aabb[0], aabb[4], aabb[5]);
    glVertex3f(aabb[0], aabb[4], aabb[5]);
    glVertex3f(aabb[0], aabb[4], aabb[2]);
    
    // Sides
    glVertex3f(aabb[0], aabb[1], aabb[2]);
    glVertex3f(aabb[0], aabb[4], aabb[2]);
    glVertex3f(aabb[3], aabb[1], aabb[2]);
    glVertex3f(aabb[3], aabb[4], aabb[2]);
    glVertex3f(aabb[3], aabb[1], aabb[5]);
    glVertex3f(aabb[3], aabb[4], aabb[5]);
    glVertex3f(aabb[0], aabb[1], aabb[5]);
    glVertex3f(aabb[0], aabb[4], aabb[5]);
    
    glEnd();

}

void QGCodeViewer::draw() 
{
  // TODO render objects
    drawObjects();
}

void QGCodeViewer::fastDraw() 
{
    drawObjects(true);
}

void QGCodeViewer::postDraw() 
{
  QGLViewer::postDraw();
}

void QGCodeViewer::update() 
{
    // qDebug() << "QGCodeViewer::update";

    Vec c((aabb[0] + aabb[3])/2, (aabb[1] + aabb[4])/2, (aabb[2] + aabb[5])/2);
    double radius = qMax(aabb[0] + aabb[3], aabb[1] + aabb[4]);
    radius = qMax(radius, aabb[2] + aabb[5]) / 2;
    
    setSceneRadius(radius);

    qDebug() << "radius" << radius;

    if (_autoZoom)
        showEntireScene();

    if (dirty) {
        dirty = false;
        updateGL();
    }
}


/*
void QGCodeViewer::keyPressEvent(QKeyEvent *e) 
{
    int keyInt = e->key();
    Qt::Key key = static_cast<Qt::Key>(keyInt);
    
    if (key == Qt::Key_unknown) {
        qDebug() << "Unknown key from a macro probably";
        return;
    }
    
    // the user have clicked just and only the special keys Ctrl, Shift, Alt, Meta.
    if (key == Qt::Key_Control ||
       key == Qt::Key_Shift ||
       key == Qt::Key_Alt ||
       key == Qt::Key_Meta)
    {
        // qDebug() << "Single click of special key: Ctrl, Shift, Alt or Meta";
        // qDebug() << "New KeySequence:" << QKeySequence(keyInt).toString(QKeySequence::NativeText);
        // return;
    }

    // check for a combination of user clicks
    Qt::KeyboardModifiers modifiers = e->modifiers();
    QString keyText = e->text();
    // if the keyText is empty than it's a special key like F1, F5, ...
    //  qDebug() << "Pressed Key:" << keyText;
    
    QList<Qt::Key> modifiersList;
    if (modifiers & Qt::ShiftModifier)
        keyInt += Qt::SHIFT;
    if (modifiers & Qt::ControlModifier)
        keyInt += Qt::CTRL;
    if (modifiers & Qt::AltModifier)
        keyInt += Qt::ALT;
    if (modifiers & Qt::MetaModifier)
        keyInt += Qt::META;
    
    QString seq = QKeySequence(keyInt).toString(QKeySequence::NativeText);
    // qDebug() << "KeySequence:" << seq;
    
    return; // skip built in command if overridden by shortcut

  switch (e->key()) {
  case Qt::Key_S :
    break;
  case Qt::Key_P :
    break;
  case Qt::Key_C :
    resetCamView();
    break;
  default:
    QGLViewer::keyPressEvent(e);
}

    QGLViewer::keyPressEvent(e);
}
*/