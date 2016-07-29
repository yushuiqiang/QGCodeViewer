/********************************************************************
* Copyright (C) 2010 - 2016 ArcEye <arceye AT mgware DOT co DOT uk>
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public
* License as published by the Free Software Foundation; either
* version 2.1 of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with this library; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
********************************************************************/

#include "QGCodeViewer.h"
#include "QGCodeViewerPlugin.h"

#include <QtPlugin>

#include <QtGlobal>
#include <QtDesigner/QDesignerExportWidget>

class QDESIGNER_WIDGET_EXPORT GCodeViewer : public QGCodeViewer
{
public:
  explicit GCodeViewer(QWidget* parent=NULL)
    : QGCodeViewer(parent)
  {
  }
};

QGCodeViewerPlugin::QGCodeViewerPlugin(QObject *parent)
 : QObject(parent)
{
  initialized = false;
}
  
QString QGCodeViewerPlugin::includeFile() const
{
  return QLatin1String("QGCodeViewer.h");
}

bool QGCodeViewerPlugin::isContainer() const
{
  return false;
}
  
bool QGCodeViewerPlugin::isInitialized() const
{
  return initialized;
}
  
QString QGCodeViewerPlugin::codeTemplate() const
{
  return QString("None");
}
  
QString QGCodeViewerPlugin::domXml() const
{
  return QLatin1String("<widget class=\"QGCodeViewer\" name=\"gcodeviewer\">\n"
		       " <property name=\"geometry\">\n"
		       "  <rect>\n"
		       "   <x>0</x>\n"
		       "   <y>0</y>\n"
		       "   <width>200</width>\n"
		       "   <height>200</height>\n"
		       "  </rect>\n"
		       " </property>\n"
		       "</widget>\n");
}

QString QGCodeViewerPlugin::group() const
{
  return QLatin1String("Display Widgets");
}

QString QGCodeViewerPlugin::name() const
{
  return QLatin1String("QGCodeViewer");
}

QString QGCodeViewerPlugin::toolTip() const
{
  return QString("QGGCodeViewer derived from QPlainTextEdit");
}

QString QGCodeViewerPlugin::whatsThis() const
{
  return QString("QGCodeViewer lets you display and interact with G-Code.");
}

QWidget *QGCodeViewerPlugin::createWidget(QWidget *parent)
{
  return new QGCodeViewer(parent);
}

void QGCodeViewerPlugin::initialize(QDesignerFormEditorInterface *)
{
    initialized = true;
}


QIcon QGCodeViewerPlugin::icon() const
{
  return QIcon();
}

#if QT_VERSION < 0x050000
// First parameter must match the TARGET in .pro
// Second parameter is the class name defined in .h
Q_EXPORT_PLUGIN2(qgcodeviewerplugin, QGCodeViewerPlugin)
#endif
