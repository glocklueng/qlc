/*
  Q Light Controller
  functionmanager.h

  Copyright (C) Heikki Junnila

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

#ifndef FUNCTIONMANAGER_H
#define FUNCTIONMANAGER_H

#include <QWidget>
#include <QList>

#include "function.h"
#include "doc.h"

class QTreeWidgetItem;
class QMdiSubWindow;
class QTreeWidget;
class QSplitter;
class QToolBar;
class QAction;
class Fixture;
class QMenu;
class Doc;

class FunctionManager : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(FunctionManager)

    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    /** Get the FunctionManager singleton instance. Can be NULL. */
    static FunctionManager* instance();

    /** Create or show FunctionManager */
    static void createAndShow(QWidget* parent, Doc* doc);

    /** Normal public destructor */
    ~FunctionManager();

signals:
    /** Emitted when the FunctionManager's QMdiSubWindow is in/activated */
    void functionManagerActive(bool active);

protected:
    /** Protected constructor to prevent multiple instances. */
    FunctionManager(QWidget* parent, Doc* doc, Qt::WindowFlags flags = 0);

protected slots:
    void slotModeChanged();
    void slotDocClearing();
    void slotFunctionChanged(quint32 id);
    void slotFunctionAdded(quint32 id);
    void slotSubWindowActivated(QMdiSubWindow* sub);

protected:
    static FunctionManager* s_instance;
    Doc* m_doc;

    /*********************************************************************
     * Function tree
     *********************************************************************/
public:
    /** Update all functions to function tree */
    void updateTree();

    /** Select the function with the given ID */
    void selectFunction(quint32 id);

private:
    /** Init the splitter view */
    void initSplitterView();

    /** Init function tree view */
    void initTree();

    /** Update $item's contents from the given $function */
    void updateFunctionItem(QTreeWidgetItem* item, const Function* function);

    /** Return a suitable parent item for the $function's type */
    QTreeWidgetItem* parentItem(const Function* function);

    /** Get the ID of the function represented by $item. */
    quint32 itemFunctionId(const QTreeWidgetItem* item) const;

    /** Get the item that represents the given function. */
    QTreeWidgetItem* functionItem(const Function* function);

    /** Get an icon that represents the given function's type */
    QIcon functionIcon(const Function* function) const;

    /** Delete all currently selected functions */
    void deleteSelectedFunctions();

private slots:
    /** Function selection was changed */
    void slotTreeSelectionChanged();

    /** Right mouse button was clicked on function tree */
    void slotTreeContextMenuRequested();

private:
    QSplitter* m_splitter;
    QTreeWidget* m_tree;

    /*********************************************************************
     * Menus, toolbar & actions
     *********************************************************************/
protected:
    void initActions();
    void initToolbar();

protected slots:
    void slotAddScene();
    void slotAddChaser();
    void slotAddCollection();
    void slotAddEFX();
    void slotAddRGBMatrix();
    void slotAddScript();
    void slotWizard();

    void slotClone();
    void slotDelete();
    void slotSelectAll();

protected:
    void updateActionStatus();

protected:
    QToolBar* m_toolbar;

    QAction* m_addSceneAction;
    QAction* m_addChaserAction;
    QAction* m_addCollectionAction;
    QAction* m_addEFXAction;
    QAction* m_addRGBMatrixAction;
    QAction* m_addScriptAction;

    QAction* m_wizardAction;
    QAction* m_cloneAction;
    QAction* m_deleteAction;
    QAction* m_selectAllAction;

    /*********************************************************************
     * Helpers
     *********************************************************************/
protected:
    /** Create a copy of the given function */
    void copyFunction(quint32 fid);

    /** Open an editor for the given function */
    void editFunction(Function* function);

    /** Get the currently-active editor (or NULL if one doesn't exist) */
    QWidget* currentEditor() const;
};

#endif
