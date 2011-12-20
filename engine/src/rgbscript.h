/*
  Q Light Controller
  rgbscript.h

  Copyright (c) Heikki Junnila

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  Version 2 as published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details. The license is
  in the file "COPYING".

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef RGBSCRIPT_H
#define RGBSCRIPT_H

#include <QScriptValue>
#include <QString>

class QScriptEngine;
class QSize;
class QDir;

typedef QVector<QVector<uint> > RGBMap;

class RGBScript
{
    /************************************************************************
     * Initialization
     ************************************************************************/
public:
    RGBScript();
    RGBScript(const RGBScript& s);
    ~RGBScript();

    /** Comparison operator. Uses simply fileName() == s.fileName(). */
    bool operator==(const RGBScript& s) const;

    /************************************************************************
     * Load & Evaluation
     ************************************************************************/
public:
    /** Load script contents from $file located in $dir */
    bool load(const QDir& dir, const QString& fileName);

    /** Get the filename for this script */
    QString fileName() const;

    /** Evaluate the script's contents and see if it checks out */
    bool evaluate();

private:
    QScriptEngine* m_engine;        //! The engine that runs the script
    QString m_fileName;             //! The file name that contains this script
    QString m_contents;             //! The file's contents

    /************************************************************************
     * Script API
     ************************************************************************/
public:
    /** Maximum step count for rgbMap() function */
    int rgbMapStepCount(const QSize& size);

    /** Get the RGBMap for the given step */
    RGBMap rgbMap(const QSize& size, uint rgb, int step);

    /** Get the name of the script */
    QString name();

    /** Get the name of the script's author */
    QString author();

    /** Get the script's API version. 0 == invalid or unevaluated script. */
    int apiVersion() const;

private:
    int m_apiVersion;               //! The API version that the script uses
    QScriptValue m_script;          //! The script itself
    QScriptValue m_rgbMap;          //! rgbMap() function
    QScriptValue m_rgbMapStepCount; //! rgbMapStepCount() function

    /************************************************************************
     * System & User Scripts
     ************************************************************************/
public:
    /** Get a script by its public name */
    static RGBScript script(const QString& name);

    /**
     * Get all available (user & system) RGB Scripts. The returned scripts
     * have not been evaluated for validity.
     */
    static QList <RGBScript> scripts();

    /**
     * Get available scripts from the given directory path. The returned
     * scripts have not been evaluated for validity.
     */
    static QList <RGBScript> scripts(const QDir& path);

    /** The system RGBScript directory */
    static QDir systemScriptDirectory();

    /** The user RGBScript directory */
    static QDir userScriptDirectory();

    /** Custom RGBScript directory */
    static void setCustomScriptDirectory(const QString& path);
    static QDir customScriptDirectory();

private:
    static QDir s_customScriptDirectory;
};

#endif
