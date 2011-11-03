/*
  Q Light Controller
  app.h

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

#ifndef APP_H
#define APP_H

#include <QMainWindow>
#include <QString>
#include <QList>
#include <QFile>

#include "qlcfixturedefcache.h"
#include "doc.h"

class QProgressDialog;
class QDomDocument;
class QDomElement;
class QMessageBox;
class QToolButton;
class QFileDialog;
class QToolBar;
class QPixmap;
class QAction;
class QLabel;
class QMenu;
class App;

#define KXMLQLCWorkspace "Workspace"

class App : public QMainWindow
{
    Q_OBJECT
    Q_DISABLE_COPY(App)

    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    App();
    ~App();

private:
    void init();
    void closeEvent(QCloseEvent*);

    /*********************************************************************
     * Progress dialog
     *********************************************************************/
public:
    void createProgressDialog();
    void destroyProgressDialog();

public slots:
    void slotSetProgressText(const QString& text);

private:
    QProgressDialog* m_progressDialog;

    /*********************************************************************
     * Doc
     *********************************************************************/
public:
    void clearDocument();

private slots:
    void slotDocModified(bool state);

private:
    void initDoc();

private:
    Doc* m_doc;

    /*********************************************************************
     * Main operating mode
     *********************************************************************/
public slots:
    void slotModeOperate();
    void slotModeDesign();
    void slotModeToggle();

    void slotModeChanged(Doc::Mode mode);

    /*********************************************************************
     * Menus & toolbars
     *********************************************************************/
private:
    void initActions();
    void initMenuBar();
    void initToolBar();

    /** Displays an error message if a file error has occurred */
    bool handleFileError(QFile::FileError error);

public slots:
    bool slotFileNew();
    QFile::FileError slotFileOpen();
    QFile::FileError slotFileSave();
    QFile::FileError slotFileSaveAs();
    void slotFileQuit();

    void slotControlMonitor();
    void slotControlFullScreen();
    void slotControlBlackout();
    void slotBlackoutChanged(bool state);
    void slotControlPanic();
    void slotRunningFunctionsChanged();

    void slotHelpIndex();
    void slotHelpAbout();

private:
    QAction* m_fileNewAction;
    QAction* m_fileOpenAction;
    QAction* m_fileSaveAction;
    QAction* m_fileSaveAsAction;
    QAction* m_fileQuitAction;

    QAction* m_modeToggleAction;
    QAction* m_controlMonitorAction;
    QAction* m_controlFullScreenAction;
    QAction* m_controlBlackoutAction;
    QAction* m_controlPanicAction;

    QAction* m_helpIndexAction;
    QAction* m_helpAboutAction;

private:
#ifdef __APPLE__
    QMenu* m_fileMenu;
#endif
    QToolBar* m_toolbar;

    /*********************************************************************
     * Load & Save
     *********************************************************************/
public:
    /**
     * Set the name of the current workspace file
     */
    void setFileName(const QString& fileName);

    /**
     * Get the name of the current workspace file
     */
    QString fileName() const;

    /**
     * Load workspace contents from a file with the given name.
     *
     * @param fileName The name of the file to load from.
     * @return QFile::NoError if successful.
     */
    QFile::FileError loadXML(const QString& fileName);

    /**
     * Load workspace contents from the given XML document.
     *
     * @param doc The XML document to load from.
     */
    bool loadXML(const QDomDocument* doc);

    /**
     * Save workspace contents to a file with the given name. Changes the
     * current workspace file name to the given fileName.
     *
     * @param fileName The name of the file to save to.
     * @return QFile::NoError if successful.
     */
    QFile::FileError saveXML(const QString& fileName);

private:
    QString m_fileName;
};

#endif
