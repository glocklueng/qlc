/*
  Q Light Controller
  app.cpp

  Copyright (c) Heikki Junnila,
                Christopher Staite

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

#include <QtCore>
#include <QtGui>
#include <QtXml>

#include "functionmanager.h"
#include "virtualconsole.h"
#include "fixturemanager.h"
#include "outputmanager.h"
#include "inputmanager.h"
#include "mastertimer.h"
#include "docbrowser.h"
#include "outputmap.h"
#include "inputmap.h"
#include "aboutbox.h"
#include "monitor.h"
#include "vcframe.h"
#include "app.h"
#include "doc.h"

#ifndef WIN32
#include <X11/Xlib.h>
#endif

#include "qlcfixturedefcache.h"
#include "qlcfixturedef.h"
#include "qlcconfig.h"
#include "qlcfile.h"

#define SETTINGS_GEOMETRY "workspace/geometry"

#define KModeTextOperate QObject::tr("Operate")
#define KModeTextDesign QObject::tr("Design")
#define KInputUniverseCount 4
#define KUniverseCount 4

/*****************************************************************************
 * Initialization
 *****************************************************************************/

App::App() : QMainWindow()
    , m_progressDialog(NULL)
    , m_doc(NULL)

    , m_fileNewAction(NULL)
    , m_fileOpenAction(NULL)
    , m_fileSaveAction(NULL)
    , m_fileSaveAsAction(NULL)
    , m_fileQuitAction(NULL)

    , m_modeToggleAction(NULL)
    , m_controlMonitorAction(NULL)
    , m_controlFullScreenAction(NULL)
    , m_controlBlackoutAction(NULL)
    , m_controlPanicAction(NULL)

    , m_helpIndexAction(NULL)
    , m_helpAboutAction(NULL)

#ifdef __APPLE__
    , m_fileMenu(NULL)
#endif

    , m_toolbar(NULL)
{
    QCoreApplication::setOrganizationName("qlc");
    QCoreApplication::setOrganizationDomain("sf.net");
    QCoreApplication::setApplicationName(APPNAME);

#ifdef __APPLE__
    createProgressDialog();
#endif

    init();
    slotModeDesign();
    slotDocModified(false);

#ifdef __APPLE__
    destroyProgressDialog();
#endif
}

App::~App()
{
    QSettings settings;
    settings.setValue(SETTINGS_GEOMETRY, saveGeometry());

    if (Monitor::instance() != NULL)
        delete Monitor::instance();

    if (FixtureManager::instance() != NULL)
        delete FixtureManager::instance();

    if (FunctionManager::instance() != NULL)
        delete FunctionManager::instance();

    if (InputManager::instance() != NULL)
        delete InputManager::instance();

    if (OutputManager::instance() != NULL)
        delete OutputManager::instance();

    if (VirtualConsole::instance() != NULL)
        delete VirtualConsole::instance();

    // Delete doc
    if (m_doc != NULL)
        delete m_doc;
    m_doc = NULL;
}

void App::init()
{
    QSettings settings;

    setWindowIcon(QIcon(":/qlc.png"));

    /* MDI Area */
    QMdiArea* area = new QMdiArea(this);
    area->setViewMode(QMdiArea::TabbedView);
    area->setTabPosition(QTabWidget::South);
    //area->setDocumentMode(true);
    setCentralWidget(area);

    QVariant var = settings.value(SETTINGS_GEOMETRY);
    if (var.isValid() == true)
    {
        this->restoreGeometry(var.toByteArray());
    }
    else
    {
        /* Application geometry and window state */
        QSize size = settings.value("/workspace/size").toSize();
        if (size.isValid() == true)
            resize(size);
        else
            resize(800, 600);

        QVariant state = settings.value("/workspace/state", Qt::WindowNoState);
        if (state.isValid() == true)
            setWindowState(Qt::WindowState(state.toInt()));
    }

    // The engine object
    initDoc();
    // Main view actions
    initActions();
    // Main menu bar
    initMenuBar();
    // Main tool bar
    initToolBar();

    // Primary UI views
    FixtureManager::createAndShow(centralWidget(), m_doc);
    FunctionManager::createAndShow(centralWidget(), m_doc);
    VirtualConsole::createAndShow(centralWidget(), m_doc);
    OutputManager::createAndShow(centralWidget(), m_doc->outputMap());
    InputManager::createAndShow(centralWidget(), m_doc->inputMap());

    // Activate FixtureManager
    QMdiSubWindow* sub = area->subWindowList().first();
    Q_ASSERT(sub != NULL);
    area->setActiveSubWindow(sub);
    sub->raise();

    // Listen to blackout changes and toggle m_controlBlackoutAction
    connect(m_doc->outputMap(), SIGNAL(blackoutChanged(bool)), this, SLOT(slotBlackoutChanged(bool)));

    // Enable/Disable panic button
    connect(m_doc->masterTimer(), SIGNAL(functionListChanged()), this, SLOT(slotRunningFunctionsChanged()));
    slotRunningFunctionsChanged();

    // Start up in non-modified state
    m_doc->resetModified();
}

void App::closeEvent(QCloseEvent* e)
{
    int result = 0;

    if (m_doc->mode() == Doc::Operate)
    {
        QMessageBox::warning(this,
                             tr("Cannot exit in Operate mode"),
                             tr("You must switch back to Design mode " \
                                "to close the application."));
        e->ignore();
        return;
    }

    if (m_doc->isModified() == true)
    {
        result = QMessageBox::information(this, tr("Close"),
                                          tr("Do you wish to save the current workspace " \
                                             "before closing the application?"),
                                          QMessageBox::Yes,
                                          QMessageBox::No,
                                          QMessageBox::Cancel);

        if (result == QMessageBox::Yes)
        {
            slotFileSave();
            e->accept();
        }
        else if (result == QMessageBox::No)
        {
            e->accept();
        }
        else if (result == QMessageBox::Cancel)
        {
            e->ignore();
        }
    }
    else
    {
        e->accept();
    }
}

/*****************************************************************************
 * Progress dialog
 *****************************************************************************/

void App::createProgressDialog()
{
    m_progressDialog = new QProgressDialog;
    m_progressDialog->setCancelButton(NULL);
    m_progressDialog->show();
    m_progressDialog->raise();
    m_progressDialog->setRange(0, 10);
    slotSetProgressText(QString());
    QApplication::processEvents();
}

void App::destroyProgressDialog()
{
    delete m_progressDialog;
    m_progressDialog = NULL;
}

void App::slotSetProgressText(const QString& text)
{
    if (m_progressDialog == NULL)
        return;

    static int progress = 0;
    m_progressDialog->setValue(progress++);
    m_progressDialog->setLabelText(QString("<B>%1</B><BR/>%2")
                                   .arg(tr("Starting Q Light Controller"))
                                   .arg(text));
    QApplication::processEvents();
}

/*****************************************************************************
 * Doc
 *****************************************************************************/

void App::initDoc()
{
    Q_ASSERT(m_doc == NULL);
    m_doc = new Doc(this);

    connect(m_doc, SIGNAL(modified(bool)), this, SLOT(slotDocModified(bool)));
    connect(m_doc, SIGNAL(modeChanged(Doc::Mode)), this, SLOT(slotModeChanged(Doc::Mode)));

    /* Load user fixtures first so that they override system fixtures */
    m_doc->fixtureDefCache()->load(QLCFixtureDefCache::userDefinitionDirectory());
    m_doc->fixtureDefCache()->load(QLCFixtureDefCache::systemDefinitionDirectory());

    /* Load output plugins */
    Q_ASSERT(m_doc->outputMap() != NULL);
    connect(m_doc->outputMap(), SIGNAL(pluginAdded(const QString&)),
            this, SLOT(slotSetProgressText(const QString&)));
    m_doc->outputMap()->loadPlugins(OutputMap::systemPluginDirectory());
    m_doc->outputMap()->loadDefaults();

    /* Load input plugins & profiles */
    Q_ASSERT(m_doc->inputMap() != NULL);
    connect(m_doc->inputMap(), SIGNAL(pluginAdded(const QString&)),
            this, SLOT(slotSetProgressText(const QString&)));
    m_doc->inputMap()->loadPlugins(InputMap::systemPluginDirectory());
    m_doc->inputMap()->loadProfiles(InputMap::userProfileDirectory());
    m_doc->inputMap()->loadProfiles(InputMap::systemProfileDirectory());
    m_doc->inputMap()->loadDefaults();

    m_doc->masterTimer()->start();
}

void App::slotDocModified(bool state)
{
    QString caption(APPNAME);

    if (fileName().isEmpty() == false)
        caption += QString(" - ") + QDir::toNativeSeparators(fileName());
    else
        caption += tr(" - New Workspace");

    if (state == true)
        setWindowTitle(caption + QString(" *"));
    else
        setWindowTitle(caption);
}

/*****************************************************************************
 * Main application Mode
 *****************************************************************************/

void App::slotModeOperate()
{
    m_doc->setMode(Doc::Operate);
}

void App::slotModeDesign()
{
    if (m_doc->masterTimer()->runningFunctions() > 0)
    {
        int result = QMessageBox::warning(
                         this,
                         tr("Switch to Design Mode"),
                         tr("There are still running functions.\n"
                            "Really stop them and switch back to "
                            "Design mode?"),
                         QMessageBox::Yes,
                         QMessageBox::No);

        if (result == QMessageBox::No)
            return;
        else
            m_doc->masterTimer()->stopAllFunctions();
    }

    m_doc->setMode(Doc::Design);
}

void App::slotModeToggle()
{
    if (m_doc->mode() == Doc::Design)
        slotModeOperate();
    else
        slotModeDesign();
}

void App::slotModeChanged(Doc::Mode mode)
{
    if (mode == Doc::Operate)
    {
        /* Disable editing features */
        m_fileNewAction->setEnabled(false);
        m_fileOpenAction->setEnabled(false);
        m_fileQuitAction->setEnabled(false);

        m_modeToggleAction->setIcon(QIcon(":/design.png"));
        m_modeToggleAction->setText(tr("Design"));
        m_modeToggleAction->setToolTip(tr("Switch to design mode"));
    }
    else if (mode == Doc::Design)
    {
        /* Enable editing features */
        m_fileNewAction->setEnabled(true);
        m_fileOpenAction->setEnabled(true);
        m_fileQuitAction->setEnabled(true);

        m_modeToggleAction->setIcon(QIcon(":/operate.png"));
        m_modeToggleAction->setText(tr("Operate"));
        m_modeToggleAction->setToolTip(tr("Switch to operate mode"));
    }
}

/*****************************************************************************
 * Actions, menubar, toolbar, statusbar
 *****************************************************************************/

void App::initActions()
{
    /* File actions */
    m_fileNewAction = new QAction(QIcon(":/filenew.png"), tr("&New"), this);
    m_fileNewAction->setShortcut(QKeySequence(tr("CTRL+N", "File|New")));
    connect(m_fileNewAction, SIGNAL(triggered(bool)), this, SLOT(slotFileNew()));

    m_fileOpenAction = new QAction(QIcon(":/fileopen.png"), tr("&Open"), this);
    m_fileOpenAction->setShortcut(QKeySequence(tr("CTRL+O", "File|Open")));
    connect(m_fileOpenAction, SIGNAL(triggered(bool)), this, SLOT(slotFileOpen()));

    m_fileSaveAction = new QAction(QIcon(":/filesave.png"), tr("&Save"), this);
    m_fileSaveAction->setShortcut(QKeySequence(tr("CTRL+S", "File|Save")));
    connect(m_fileSaveAction, SIGNAL(triggered(bool)), this, SLOT(slotFileSave()));

    m_fileSaveAsAction = new QAction(QIcon(":/filesaveas.png"), tr("Save &As..."), this);
    connect(m_fileSaveAsAction, SIGNAL(triggered(bool)), this, SLOT(slotFileSaveAs()));

    m_fileQuitAction = new QAction(QIcon(":/exit.png"), tr("&Quit"), this);
    m_fileQuitAction->setShortcut(QKeySequence(tr("CTRL+Q", "File|Quit")));
    connect(m_fileQuitAction, SIGNAL(triggered(bool)), this, SLOT(slotFileQuit()));

    /* Control actions */
    m_modeToggleAction = new QAction(QIcon(":/operate.png"), tr("&Operate"), this);
    m_modeToggleAction->setToolTip(tr("Switch to operate mode"));
    m_modeToggleAction->setShortcut(QKeySequence(tr("CTRL+F12", "Control|Toggle operate/design mode")));
    connect(m_modeToggleAction, SIGNAL(triggered(bool)), this, SLOT(slotModeToggle()));

    m_controlMonitorAction = new QAction(QIcon(":/monitor.png"), tr("&Monitor"), this);
    m_controlMonitorAction->setShortcut(QKeySequence(tr("CTRL+M", "Control|Monitor")));
    connect(m_controlMonitorAction, SIGNAL(triggered(bool)), this, SLOT(slotControlMonitor()));

    m_controlBlackoutAction = new QAction(QIcon(":/blackout.png"), tr("Toggle &Blackout"), this);
    m_controlBlackoutAction->setCheckable(true);
    connect(m_controlBlackoutAction, SIGNAL(triggered(bool)), this, SLOT(slotControlBlackout()));
    m_controlBlackoutAction->setChecked(m_doc->outputMap()->blackout());

    m_controlPanicAction = new QAction(QIcon(":/panic.png"), tr("Stop ALL functions!"), this);
    m_controlPanicAction->setShortcut(QKeySequence("CTRL+SHIFT+ESC"));
    connect(m_controlPanicAction, SIGNAL(triggered(bool)), this, SLOT(slotControlPanic()));

    m_controlFullScreenAction = new QAction(QIcon(":/fullscreen.png"), tr("Toggle Full Screen"), this);
    m_controlFullScreenAction->setCheckable(true);
    m_controlFullScreenAction->setShortcut(QKeySequence(tr("CTRL+F11", "Control|Toggle Full Screen")));
    connect(m_controlFullScreenAction, SIGNAL(triggered(bool)), this, SLOT(slotControlFullScreen()));

    /* Help actions */
    m_helpIndexAction = new QAction(QIcon(":/help.png"), tr("&Index"), this);
    m_helpIndexAction->setShortcut(QKeySequence(tr("SHIFT+F1", "Help|Index")));
    connect(m_helpIndexAction, SIGNAL(triggered(bool)), this, SLOT(slotHelpIndex()));

    m_helpAboutAction = new QAction(QIcon(":/qlc.png"), tr("&About QLC"), this);
    connect(m_helpAboutAction, SIGNAL(triggered(bool)), this, SLOT(slotHelpAbout()));
}

void App::initMenuBar()
{
#ifdef __APPLE__
    QMenuBar *menuBar = new QMenuBar(this);

    /* Since the menubar is there in Apple anyway, put the quit action there. */
    m_fileMenu = new QMenu(menuBar);
    m_fileMenu->setTitle(tr("&File"));
    menuBar->addMenu(m_fileMenu);
    m_fileMenu->addAction(m_fileQuitAction);
#endif
}

void App::initToolBar()
{
    m_toolbar = new QToolBar(tr("Workspace"), this);
    m_toolbar->setFloatable(false);
    m_toolbar->setMovable(false);
    m_toolbar->setAllowedAreas(Qt::TopToolBarArea);
    addToolBar(m_toolbar);
    m_toolbar->addAction(m_fileNewAction);
    m_toolbar->addAction(m_fileOpenAction);
    m_toolbar->addAction(m_fileSaveAction);
    m_toolbar->addAction(m_fileSaveAsAction);
    m_toolbar->addSeparator();
    m_toolbar->addAction(m_controlMonitorAction);
    m_toolbar->addAction(m_controlFullScreenAction);
    m_toolbar->addSeparator();
    m_toolbar->addAction(m_helpIndexAction);
    m_toolbar->addAction(m_helpAboutAction);

    /* Create an empty widget between help items to flush them to the right */
    QWidget* widget = new QWidget(this);
    widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_toolbar->addWidget(widget);
    m_toolbar->addAction(m_controlPanicAction);
    m_toolbar->addSeparator();
    m_toolbar->addAction(m_controlBlackoutAction);
    m_toolbar->addSeparator();
    m_toolbar->addAction(m_modeToggleAction);

    connect(m_toolbar, SIGNAL(visibilityChanged(bool)), this, SLOT(slotToolBarVisibilityChanged(bool)));
}

void App::slotToolBarVisibilityChanged(bool visible)
{
    if (visible == false)
        m_toolbar->show();
}

/*****************************************************************************
 * File action slots
 *****************************************************************************/

bool App::handleFileError(QFile::FileError error)
{
    QString msg;

    switch (error)
    {
    case QFile::NoError:
        return true;
        break;
    case QFile::ReadError:
        msg = tr("Unable to read from file");
        break;
    case QFile::WriteError:
        msg = tr("Unable to write to file");
        break;
    case QFile::FatalError:
        msg = tr("A fatal error occurred");
        break;
    case QFile::ResourceError:
        msg = tr("Unable to access resource");
        break;
    case QFile::OpenError:
        msg = tr("Unable to open file for reading or writing");
        break;
    case QFile::AbortError:
        msg = tr("Operation was aborted");
        break;
    case QFile::TimeOutError:
        msg = tr("Operation timed out");
        break;
    default:
    case QFile::UnspecifiedError:
        msg = tr("An unspecified error has occurred. Nice.");
        break;
    }

    QMessageBox::warning(this, tr("File error"), msg);

    return false;
}

bool App::slotFileNew()
{
    bool result = false;

    if (m_doc->isModified())
    {
        QString msg(tr("Do you wish to save the current workspace?\n" \
                       "Changes will be lost if you don't save them."));
        int result = QMessageBox::warning(this, tr("New Workspace"),
                                          msg,
                                          QMessageBox::Yes,
                                          QMessageBox::No,
                                          QMessageBox::Cancel);
        if (result == QMessageBox::Yes)
        {
            slotFileSave();
            clearDocument();
            result = true;
        }
        else if (result == QMessageBox::No)
        {
            clearDocument();
            result = true;
        }
        else
        {
            result = false;
        }
    }
    else
    {
        clearDocument();
        result = true;
    }

    return result;
}

void App::clearDocument()
{
    m_doc->clearContents();
    VirtualConsole::instance()->resetContents();
    m_doc->outputMap()->resetUniverses();
    setFileName(QString());
    m_doc->resetModified();
}

QFile::FileError App::slotFileOpen()
{
    QString fn;

    /* Check that the user is aware of losing previous changes */
    if (m_doc->isModified() == true)
    {
        QString msg(tr("Do you wish to save the current workspace?\n" \
                       "Changes will be lost if you don't save them."));
        int result = QMessageBox::warning(this, tr("Open Workspace"),
                                          msg,
                                          QMessageBox::Yes,
                                          QMessageBox::No,
                                          QMessageBox::Cancel);
        if (result == QMessageBox::Yes)
        {
            /* Save first, but don't proceed unless it succeeded. */
            QFile::FileError error = slotFileSaveAs();
            if (handleFileError(error) == false)
                return error;
        }
        else if (result == QMessageBox::Cancel)
        {
            /* Second thoughts... Cancel loading. */
            return QFile::NoError;
        }
    }

    /* Create a file open dialog */
    QFileDialog dialog(this);
    dialog.setWindowTitle(tr("Open Workspace"));
    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    dialog.selectFile(fileName());

    /* Append file filters to the dialog */
    QStringList filters;
    filters << tr("Workspaces (*%1)").arg(KExtWorkspace);
#ifdef WIN32
    filters << tr("All Files (*.*)");
#else
    filters << tr("All Files (*)");
#endif
    dialog.setNameFilters(filters);

    /* Append useful URLs to the dialog */
    QList <QUrl> sidebar;
    sidebar.append(QUrl::fromLocalFile(QDir::homePath()));
    sidebar.append(QUrl::fromLocalFile(QDir::rootPath()));
    dialog.setSidebarUrls(sidebar);

    /* Get file name */
    if (dialog.exec() != QDialog::Accepted)
        return QFile::NoError;

    fn = dialog.selectedFiles().first();
    if (fn.isEmpty() == true)
        return QFile::NoError;

    /* Clear existing document data */
    clearDocument();

    /* Load the file */
    QFile::FileError error = loadXML(fn);
    if (handleFileError(error) == true)
        m_doc->resetModified();

    /* Update these in any case, since they are at least emptied now as
       a result of calling clearDocument() a few lines ago. */
    if (FunctionManager::instance() != NULL)
        FunctionManager::instance()->updateTree();
    if (FixtureManager::instance() != NULL)
        FixtureManager::instance()->updateView();
    if (OutputManager::instance() != NULL)
        OutputManager::instance()->updateTree();
    if (InputManager::instance() != NULL)
        InputManager::instance()->updateTree();

    return error;
}

QFile::FileError App::slotFileSave()
{
    QFile::FileError error;

    /* Attempt to save with the existing name. Fall back to Save As. */
    if (fileName().isEmpty() == true)
        error = slotFileSaveAs();
    else
        error = saveXML(fileName());

    handleFileError(error);
    return error;
}

QFile::FileError App::slotFileSaveAs()
{
    QString fn;

    /* Create a file save dialog */
    QFileDialog dialog(this);
    dialog.setWindowTitle(tr("Save Workspace As"));
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.selectFile(fileName());

    /* Append file filters to the dialog */
    QStringList filters;
    filters << tr("Workspaces (*%1)").arg(KExtWorkspace);
#ifdef WIN32
    filters << tr("All Files (*.*)");
#else
    filters << tr("All Files (*)");
#endif
    dialog.setNameFilters(filters);

    /* Append useful URLs to the dialog */
    QList <QUrl> sidebar;
    sidebar.append(QUrl::fromLocalFile(QDir::homePath()));
    sidebar.append(QUrl::fromLocalFile(QDir::rootPath()));
    dialog.setSidebarUrls(sidebar);

    /* Get file name */
    if (dialog.exec() != QDialog::Accepted)
        return QFile::NoError;

    fn = dialog.selectedFiles().first();
    if (fn.isEmpty() == true)
        return QFile::NoError;

    /* Always use the workspace suffix */
    if (fn.right(4) != KExtWorkspace)
        fn += KExtWorkspace;

    /* Save the document and set workspace name */
    QFile::FileError error = saveXML(fn);
    handleFileError(error);
    return error;
}

void App::slotFileQuit()
{
    close();
}

/*****************************************************************************
 * Control action slots
 *****************************************************************************/

void App::slotControlMonitor()
{
    Monitor::createAndShow(this, m_doc);
}

void App::slotControlBlackout()
{
    m_doc->outputMap()->setBlackout(!m_doc->outputMap()->blackout());
}

void App::slotBlackoutChanged(bool state)
{
    m_controlBlackoutAction->setChecked(state);
}

void App::slotControlPanic()
{
    m_doc->masterTimer()->stopAllFunctions();
}

void App::slotRunningFunctionsChanged()
{
    if (m_doc->masterTimer()->runningFunctions() > 0)
        m_controlPanicAction->setEnabled(true);
    else
        m_controlPanicAction->setEnabled(false);
}

void App::slotControlFullScreen()
{
    if (windowState() & Qt::WindowFullScreen)
        showNormal();
    else
        showFullScreen();
}

/*****************************************************************************
 * Help action slots
 *****************************************************************************/

void App::slotHelpIndex()
{
    DocBrowser::createAndShow(this);
}

void App::slotHelpAbout()
{
    AboutBox ab(this);
    ab.exec();
}

/*****************************************************************************
 * Load & Save
 *****************************************************************************/

void App::setFileName(const QString& fileName)
{
    m_fileName = fileName;
}

QString App::fileName() const
{
    return m_fileName;
}

QFile::FileError App::loadXML(const QString& fileName)
{
    QFile::FileError retval = QFile::NoError;

    QDomDocument doc(QLCFile::readXML(fileName));
    if (doc.isNull() == false)
    {
        if (doc.doctype().name() == KXMLQLCWorkspace)
        {
            if (loadXML(&doc) == false)
            {
                retval = QFile::ReadError;
            }
            else
            {
                setFileName(fileName);
                m_doc->resetModified();
                retval = QFile::NoError;
            }
        }
        else
        {
            retval = QFile::ReadError;
        }
    }

    return retval;
}

bool App::loadXML(const QDomDocument* doc)
{
    QDomElement root;
    QDomElement tag;
    QDomNode node;

    Q_ASSERT(m_doc != NULL);
    Q_ASSERT(doc != NULL);

    root = doc->documentElement();
    if (root.tagName() != KXMLQLCWorkspace)
    {
        qWarning() << Q_FUNC_INFO << "Workspace node not found";
        return false;
    }

    node = root.firstChild();
    while (node.isNull() == false)
    {
        tag = node.toElement();

        if (tag.tagName() == KXMLQLCEngine)
        {
            m_doc->loadXML(&tag);
        }
        else if (tag.tagName() == KXMLQLCVirtualConsole)
        {
            VirtualConsole::instance()->loadXML(tag);
        }
        else if (tag.tagName() == KXMLFixture)
        {
            /* Legacy support code, nowadays in Doc */
            Fixture::loader(&tag, m_doc);
        }
        else if (tag.tagName() == KXMLQLCFunction)
        {
            /* Legacy support code, nowadays in Doc */
            Function::loader(&tag, m_doc);
        }
        else if (tag.tagName() == KXMLQLCCreator)
        {
            /* Ignore creator information */
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unknown Workspace tag:" << tag.tagName();
        }

        node = node.nextSibling();
    }

    // Perform post-load operations
    VirtualConsole::instance()->postLoad();

    return true;
}

QFile::FileError App::saveXML(const QString& fileName)
{
    QFile::FileError retval;

    QFile file(fileName);
    if (file.open(QIODevice::WriteOnly) == false)
        return file.error();

    QDomDocument doc(QLCFile::getXMLHeader(KXMLQLCWorkspace));
    if (doc.isNull() == false)
    {
        QDomElement root;
        QDomElement tag;
        QDomText text;

        /* Create a text stream for the file */
        QTextStream stream(&file);

        /* THE MASTER XML ROOT NODE */
        root = doc.documentElement();

        /* Write engine components to the XML document */
        m_doc->saveXML(&doc, &root);

        /* Write virtual console to the XML document */
        VirtualConsole::instance()->saveXML(&doc, &root);

        /* Write the XML document to the stream (=file) */
        stream << doc.toString() << "\n";

        /* Set the file name for the current Doc instance and
           set it also in an unmodified state. */
        setFileName(fileName);
        m_doc->resetModified();

        retval = QFile::NoError;
    }
    else
    {
        retval = QFile::ReadError;
    }

    file.close();

    return retval;
}

