/*
  Q Light Controller
  fixturemanager.h

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

#ifndef FIXTUREMANAGER_H
#define FIXTUREMANAGER_H

#include <QWidget>

#include "function.h"
#include "fixture.h"
#include "doc.h"

class QLCFixtureDefCache;
class FixtureGroupEditor;
class QTreeWidgetItem;
class QTextBrowser;
class QTreeWidget;
class QTabWidget;
class OutputMap;
class QSplitter;
class QAction;
class QMenu;

#define KXMLQLCFixtureManager "FixtureManager"
#define KXMLQLCFixtureManagerSplitterSize "SplitterSize"

class FixtureManager : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(FixtureManager)

    /********************************************************************
     * Initialization
     ********************************************************************/
public:
    /** Get the monitor singleton instance. Can be NULL. */
    static FixtureManager* instance();

    /** Create or show the FixtureManager window. */
    static void createAndShow(QWidget* parent, Doc* doc);
    ~FixtureManager();

private:
    /** Protected constructor to prevent multiple instances. */
    FixtureManager(QWidget* parent, Doc* doc, Qt::WindowFlags flags = 0);

private:
    /** The singleton FixtureManager instance */
    static FixtureManager* s_instance;

    /********************************************************************
     * Doc signal handlers
     ********************************************************************/
public slots:
    /** Callback for Doc::fixtureRemoved() signals */
    void slotFixtureRemoved(quint32 id);

    /** Callback that listens to mode change signals */
    void slotModeChanged(Doc::Mode mode);

    /** Callback that listens to fixture group removals */
    void slotFixtureGroupRemoved(quint32 id);

    /** Callback that listens to fixture group modifications */
    void slotFixtureGroupChanged(quint32 id);

private:
    Doc* m_doc;

    /********************************************************************
     * Data view
     ********************************************************************/
public:
    /** Update the list of fixtures */
    void updateView();

private:
    /** Get a QTreeWidgetItem whose fixture ID is $id */
    QTreeWidgetItem* fixtureItem(quint32 id) const;

    /** Get a QTreeWidgetItem whose group ID is $id */
    QTreeWidgetItem* groupItem(quint32 id) const;

    /** Construct the list view and data view */
    void initDataView();

    /** Update a single fixture's data into a QTreeWidgetItem */
    void updateFixtureItem(QTreeWidgetItem* item, const Fixture* fxi);

    /** Update a group's data to and under $item */
    void updateGroupItem(QTreeWidgetItem* item, const FixtureGroup* grp);

    /** Display an error message if fixture add fails */
    void addFixtureErrorMessage();

    /** Handle single fixture selection */
    void fixtureSelected(quint32 id);

    /** Handle fixture group selection */
    void fixtureGroupSelected(FixtureGroup* grp);

    /** Create the text browser for displaying information */
    void createInfo();

private slots:
    /** Callback for fixture list selection changes */
    void slotSelectionChanged();

    /** Callback for mouse double clicks */
    void slotDoubleClicked(QTreeWidgetItem* item);

private:
    /** Select a fixture group */
    void selectGroup(quint32 id);

    /** Get a CSS style sheet & HTML header for fixture info */
    QString fixtureInfoStyleSheetHeader();

private:
    QSplitter* m_splitter;
    QTreeWidget* m_tree;

    QTextBrowser* m_info;
    FixtureGroupEditor* m_groupEditor;

    /********************************************************************
     * Menu & Toolbar & Actions
     ********************************************************************/
private:
    /** Construct actions for toolbar & context menu */
    void initActions();

    /** Update the contents of the group menu */
    void updateGroupMenu();

    /** Construct the toolbar */
    void initToolBar();

    /** Edit properties for the fixture represented by $item */
    void editFixtureProperties(QTreeWidgetItem* item);

    /** Count the number of heads in the list of fixture items */
    int headCount(const QList <QTreeWidgetItem*>& items) const;

private slots:
    void slotAdd();
    void slotRemove();
    void slotProperties();
    void slotUnGroup();
    void slotGroupSelected(QAction* action);

    /** Callback for right mouse button clicks over a fixture item */
    void slotContextMenuRequested(const QPoint& pos);

private:
    QAction* m_addAction;
    QAction* m_removeAction;
    QAction* m_propertiesAction;
    QAction* m_groupAction;
    QAction* m_unGroupAction;
    QAction* m_newGroupAction;
    QMenu* m_groupMenu;
};

#endif
