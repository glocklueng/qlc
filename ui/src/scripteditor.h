/*
  Q Light Controller
  scripteditor.h

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

#ifndef SCRIPTEDITOR_H
#define SCRIPTEDITOR_H

#include <QDialog>
#include "ui_scripteditor.h"

class QTextDocument;
class MasterTimer;
class OutputMap;
class InputMap;
class QAction;
class Script;
class QMenu;
class Doc;

class ScriptEditor : public QDialog, public Ui_ScriptEditor
{
    Q_OBJECT
    Q_DISABLE_COPY(ScriptEditor)

    /************************************************************************
     * Initialization
     ************************************************************************/
public:
    ScriptEditor(QWidget* parent, Script* script, Doc* doc, OutputMap* outputMap,
                 InputMap* inputMap, MasterTimer* masterTimer);
    ~ScriptEditor();

private:
    QTextDocument* m_document;
    Script* m_script;
    Doc* m_doc;
    OutputMap* m_outputMap;
    InputMap* m_inputMap;
    MasterTimer* m_masterTimer;

public slots:
    void accept();

    /************************************************************************
     * Add menu
     ************************************************************************/
private:
    void initAddMenu();

private slots:
    void slotAddStartFunction();
    void slotAddStopFunction();
    void slotAddWait();
    void slotAddWaitKey();
    void slotAddSetDmx();
    void slotAddSetFixture();
    void slotAddComment();

private:
    QAction* m_addStartFunctionAction;
    QAction* m_addStopFunctionAction;
    QAction* m_addWaitAction;
    QAction* m_addWaitKeyAction;
    QAction* m_addSetDmxAction;
    QAction* m_addSetFixtureAction;
    QAction* m_addCommentAction;
    QMenu* m_addMenu;
};

#endif
