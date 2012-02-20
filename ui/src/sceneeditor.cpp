/*
  Q Light Controller
  sceneeditor.cpp

  Copyright (c) Heikki Junnila, Stefan Krumm

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

#include <QTreeWidgetItem>
#include <QColorDialog>
#include <QTreeWidget>
#include <QMessageBox>
#include <QToolButton>
#include <QScrollArea>
#include <QTabWidget>
#include <QToolBar>
#include <QLayout>
#include <QLabel>
#include <QDebug>

#include "genericdmxsource.h"
#include "fixtureselection.h"
#include "fixtureconsole.h"
#include "qlcfixturedef.h"
#include "speedspinbox.h"
#include "sceneeditor.h"
#include "mastertimer.h"
#include "qlcchannel.h"
#include "outputmap.h"
#include "inputmap.h"
#include "fixture.h"
#include "scene.h"
#include "doc.h"

#define KColumnName         0
#define KColumnManufacturer 1
#define KColumnModel        2
#define KColumnID           3

#define KTabGeneral         0
#define KTabFirstFixture    1

#define CYAN "cyan"
#define MAGENTA "magenta"
#define YELLOW "yellow"
#define RED "red"
#define GREEN "green"
#define BLUE "blue"

SceneEditor::SceneEditor(QWidget* parent, Scene* scene, Doc* doc)
    : QWidget(parent)
    , m_doc(doc)
    , m_scene(scene)
    , m_source(new GenericDMXSource(doc))
    , m_initFinished(false)
    , m_currentTab(KTabGeneral)
{
    Q_ASSERT(doc != NULL);
    Q_ASSERT(scene != NULL);

    setupUi(this);

    init();

    // Start new (==empty) scenes from the first tab and ones with something in them
    // on the first fixture page.
    if (m_tab->count() == 0)
        slotTabChanged(KTabGeneral);
    else
        m_tab->setCurrentIndex(KTabFirstFixture);

    m_initFinished = true;

    // Set focus to the editor
    m_nameEdit->setFocus();
}

SceneEditor::~SceneEditor()
{
    delete m_source;
    m_source = NULL;
}

void SceneEditor::init()
{
    /* Actions */
    m_enableCurrentAction = new QAction(QIcon(":/check.png"),
                                        tr("Enable all channels in current fixture"), this);
    m_disableCurrentAction = new QAction(QIcon(":/uncheck.png"),
                                         tr("Disable all channels in current fixture"), this);
    m_copyAction = new QAction(QIcon(":/editcopy.png"),
                               tr("Copy current values to clipboard"), this);
    m_pasteAction = new QAction(QIcon(":/editpaste.png"),
                                tr("Paste clipboard values to current fixture"), this);
    m_copyToAllAction = new QAction(QIcon(":/editcopyall.png"),
                                    tr("Copy current values to all fixtures"), this);
    m_colorToolAction = new QAction(QIcon(":/color.png"),
                                    tr("Color tool for CMY/RGB-capable fixtures"), this);
    m_blindAction = new QAction(QIcon(":/blind.png"),
                                      tr("Don't write values to universe when editing"), this);

    m_blindAction->setCheckable(true);
    if (m_doc->mode() == Doc::Operate)
    {
        m_blindAction->setChecked(true);
        m_source->setOutputEnabled(false);
    }
    else
    {
        m_blindAction->setChecked(false);
        m_source->setOutputEnabled(true);
    }

    connect(m_enableCurrentAction, SIGNAL(triggered(bool)),
            this, SLOT(slotEnableCurrent()));
    connect(m_disableCurrentAction, SIGNAL(triggered(bool)),
            this, SLOT(slotDisableCurrent()));
    connect(m_copyAction, SIGNAL(triggered(bool)),
            this, SLOT(slotCopy()));
    connect(m_pasteAction, SIGNAL(triggered(bool)),
            this, SLOT(slotPaste()));
    connect(m_copyToAllAction, SIGNAL(triggered(bool)),
            this, SLOT(slotCopyToAll()));
    connect(m_colorToolAction, SIGNAL(triggered(bool)),
            this, SLOT(slotColorTool()));
    connect(m_blindAction, SIGNAL(triggered(bool)),
            this, SLOT(slotBlindToggled(bool)));

    /* Toolbar */
    QToolBar* toolBar = new QToolBar(this);
    layout()->setMenuBar(toolBar);
    toolBar->addAction(m_enableCurrentAction);
    toolBar->addAction(m_disableCurrentAction);
    toolBar->addSeparator();
    toolBar->addAction(m_copyAction);
    toolBar->addAction(m_pasteAction);
    toolBar->addAction(m_copyToAllAction);
    toolBar->addSeparator();
    toolBar->addAction(m_colorToolAction);
    toolBar->addSeparator();
    toolBar->addAction(m_blindAction);

    /* Tab widget */
    connect(m_tab, SIGNAL(currentChanged(int)),
            this, SLOT(slotTabChanged(int)));

    /* Add & remove buttons */
    connect(m_addFixtureButton, SIGNAL(clicked()),
            this, SLOT(slotAddFixtureClicked()));
    connect(m_removeFixtureButton, SIGNAL(clicked()),
            this, SLOT(slotRemoveFixtureClicked()));

    m_nameEdit->setText(m_scene->name());
    m_nameEdit->setSelection(0, m_nameEdit->text().length());
    connect(m_nameEdit, SIGNAL(textEdited(const QString&)),
            this, SLOT(slotNameEdited(const QString&)));

    /* Speeds */
    new QHBoxLayout(m_fadeInContainer);
    m_fadeInSpin = new SpeedSpinBox(SpeedSpinBox::Zero, m_fadeInContainer);
    m_fadeInContainer->layout()->addWidget(m_fadeInSpin);
    m_fadeInContainer->layout()->setMargin(0);
    m_fadeInSpin->setValue(m_scene->fadeInSpeed());

    new QHBoxLayout(m_fadeOutContainer);
    m_fadeOutSpin = new SpeedSpinBox(SpeedSpinBox::Zero, m_fadeOutContainer);
    m_fadeOutContainer->layout()->addWidget(m_fadeOutSpin);
    m_fadeOutContainer->layout()->setMargin(0);
    m_fadeOutSpin->setValue(m_scene->fadeOutSpeed());

    connect(m_fadeInSpin, SIGNAL(valueChanged(int)),
            this, SLOT(slotFadeInSpinChanged(int)));
    connect(m_fadeOutSpin, SIGNAL(valueChanged(int)),
            this, SLOT(slotFadeOutSpinChanged(int)));

    QListIterator <SceneValue> it(m_scene->values());
    while (it.hasNext() == true)
    {
        SceneValue scv(it.next());

        if (fixtureItem(scv.fxi) == NULL)
        {
            Fixture* fixture = m_doc->fixture(scv.fxi);
            if (fixture == NULL)
                continue;

            addFixtureItem(fixture);
            addFixtureTab(fixture);
        }

        setSceneValue(scv);
    }
}

void SceneEditor::setSceneValue(const SceneValue& scv)
{
    FixtureConsole* fc;
    Fixture* fixture;

    fixture = m_doc->fixture(scv.fxi);
    Q_ASSERT(fixture != NULL);

    fc = fixtureConsole(fixture);
    Q_ASSERT(fc != NULL);

    fc->setSceneValue(scv);
}

/*****************************************************************************
 * Common
 *****************************************************************************/

void SceneEditor::slotTabChanged(int tab)
{
    m_currentTab = tab;

    if (tab == KTabGeneral)
    {
        m_enableCurrentAction->setEnabled(false);
        m_disableCurrentAction->setEnabled(false);

        m_copyAction->setEnabled(false);
        m_pasteAction->setEnabled(false);
        m_copyToAllAction->setEnabled(false);
        m_colorToolAction->setEnabled(false);
    }
    else
    {
        m_enableCurrentAction->setEnabled(true);
        m_disableCurrentAction->setEnabled(true);

        m_copyAction->setEnabled(true);
        if (m_copy.isEmpty() == false)
            m_pasteAction->setEnabled(true);

        m_copyToAllAction->setEnabled(true);
        m_colorToolAction->setEnabled(isColorToolAvailable());
    }
}

void SceneEditor::slotEnableCurrent()
{
    /* QObject cast fails unless the widget is a FixtureConsole */
    FixtureConsole* fc = consoleTab(m_currentTab);
    if (fc != NULL)
        fc->setChecked(true);
}

void SceneEditor::slotDisableCurrent()
{
    /* QObject cast fails unless the widget is a FixtureConsole */
    FixtureConsole* fc = consoleTab(m_currentTab);
    if (fc != NULL)
        fc->setChecked(false);
}

void SceneEditor::slotCopy()
{
    /* QObject cast fails unless the widget is a FixtureConsole */
    FixtureConsole* fc = consoleTab(m_currentTab);
    if (fc != NULL)
    {
        m_copy = fc->values();
        m_pasteAction->setEnabled(true);
    }
}

void SceneEditor::slotPaste()
{
    /* QObject cast fails unless the widget is a FixtureConsole */
    FixtureConsole* fc = consoleTab(m_currentTab);
    if (fc != NULL && m_copy.isEmpty() == false)
        fc->setValues(m_copy);
}

void SceneEditor::slotCopyToAll()
{
    slotCopy();

    for (int i = KTabFirstFixture; i < m_tab->count(); i++)
    {
        FixtureConsole* fc = consoleTab(i);
        if (fc != NULL)
            fc->setValues(m_copy);
    }

    m_copy.clear();
    m_pasteAction->setEnabled(false);
}

void SceneEditor::slotColorTool()
{
    /* QObject cast fails unless the widget is a FixtureConsole */
    FixtureConsole* fc = consoleTab(m_currentTab);
    if (fc == NULL)
        return;

    Fixture* fxi = m_doc->fixture(fc->fixture());
    Q_ASSERT(fxi != NULL);

    QSet <quint32> cyan = fxi->channels(CYAN, Qt::CaseInsensitive, QLCChannel::Intensity);
    QSet <quint32> magenta = fxi->channels(MAGENTA, Qt::CaseInsensitive, QLCChannel::Intensity);
    QSet <quint32> yellow = fxi->channels(YELLOW, Qt::CaseInsensitive, QLCChannel::Intensity);
    QSet <quint32> red = fxi->channels(RED, Qt::CaseInsensitive, QLCChannel::Intensity);
    QSet <quint32> green = fxi->channels(GREEN, Qt::CaseInsensitive, QLCChannel::Intensity);
    QSet <quint32> blue = fxi->channels(BLUE, Qt::CaseInsensitive, QLCChannel::Intensity);

    if (!cyan.isEmpty() && !magenta.isEmpty() && !yellow.isEmpty())
    {
        QColor color;
        color.setCmyk(fc->value(*cyan.begin()),
                      fc->value(*magenta.begin()),
                      fc->value(*yellow.begin()),
                      0);

        color = QColorDialog::getColor(color);
        if (color.isValid() == true)
        {
            foreach (quint32 ch, cyan)
            {
                fc->setChecked(true, ch);
                fc->setValue(ch, color.cyan());
            }

            foreach (quint32 ch, magenta)
            {
                fc->setChecked(true, ch);
                fc->setValue(ch, color.magenta());
            }

            foreach (quint32 ch, yellow)
            {
                fc->setChecked(true, ch);
                fc->setValue(ch, color.yellow());
            }
        }
    }
    else if (!red.isEmpty() && !green.isEmpty() && !blue.isEmpty())
    {
        QColor color;
        color.setRgb(fc->value(*red.begin()),
                     fc->value(*green.begin()),
                     fc->value(*blue.begin()),
                     0);

        color = QColorDialog::getColor(color);
        if (color.isValid() == true)
        {
            foreach (quint32 ch, red)
            {
                fc->setChecked(true, ch);
                fc->setValue(ch, color.red());
            }

            foreach (quint32 ch, green)
            {
                fc->setChecked(true, ch);
                fc->setValue(ch, color.green());
            }

            foreach (quint32 ch, blue)
            {
                fc->setChecked(true, ch);
                fc->setValue(ch, color.blue());
            }
        }
    }
}

void SceneEditor::slotBlindToggled(bool state)
{
    if (m_source != NULL)
        m_source->setOutputEnabled(!state);
}

bool SceneEditor::isColorToolAvailable()
{
    Fixture* fxi;
    QColor color;
    quint32 cyan, magenta, yellow;
    quint32 red, green, blue;

    /* QObject cast fails unless the widget is a FixtureConsole */
    FixtureConsole* fc = consoleTab(m_currentTab);
    if (fc == NULL)
        return false;

    fxi = m_doc->fixture(fc->fixture());
    Q_ASSERT(fxi != NULL);

    cyan = fxi->channel(CYAN, Qt::CaseInsensitive, QLCChannel::Intensity);
    magenta = fxi->channel(MAGENTA, Qt::CaseInsensitive, QLCChannel::Intensity);
    yellow = fxi->channel(YELLOW, Qt::CaseInsensitive, QLCChannel::Intensity);
    red = fxi->channel(RED, Qt::CaseInsensitive, QLCChannel::Intensity);
    green = fxi->channel(GREEN, Qt::CaseInsensitive, QLCChannel::Intensity);
    blue = fxi->channel(BLUE, Qt::CaseInsensitive, QLCChannel::Intensity);

    if (cyan != QLCChannel::invalid() && magenta != QLCChannel::invalid() &&
        yellow != QLCChannel::invalid())
    {
        return true;
    }
    else if (red != QLCChannel::invalid() && green != QLCChannel::invalid() &&
             blue != QLCChannel::invalid())
    {
        return true;
    }
    else
    {
        return false;
    }
}

/*****************************************************************************
 * General page
 *****************************************************************************/

QTreeWidgetItem* SceneEditor::fixtureItem(quint32 fxi_id)
{
    QTreeWidgetItemIterator it(m_tree);
    while (*it != NULL)
    {
        QTreeWidgetItem* item = *it;
        if (item->text(KColumnID).toUInt() == fxi_id)
            return item;
        ++it;
    }

    return NULL;
}

QList <Fixture*> SceneEditor::selectedFixtures() const
{
    QListIterator <QTreeWidgetItem*> it(m_tree->selectedItems());
    QList <Fixture*> list;

    while (it.hasNext() == true)
    {
        QTreeWidgetItem* item;
        quint32 fxi_id;
        Fixture* fixture;

        item = it.next();
        fxi_id = item->text(KColumnID).toInt();
        fixture = m_doc->fixture(fxi_id);
        Q_ASSERT(fixture != NULL);

        list.append(fixture);
    }

    return list;
}

void SceneEditor::addFixtureItem(Fixture* fixture)
{
    QTreeWidgetItem* item;

    Q_ASSERT(fixture != NULL);

    item = new QTreeWidgetItem(m_tree);
    item->setText(KColumnName, fixture->name());
    item->setText(KColumnID, QString("%1").arg(fixture->id()));

    if (fixture->fixtureDef() == NULL)
    {
        item->setText(KColumnManufacturer, tr("Generic"));
        item->setText(KColumnModel, tr("Generic"));
    }
    else
    {
        item->setText(KColumnManufacturer,
                      fixture->fixtureDef()->manufacturer());
        item->setText(KColumnModel, fixture->fixtureDef()->model());
    }

    /* Select newly-added fixtures so that their channels can be
       quickly disabled/enabled */
    item->setSelected(true);
}

void SceneEditor::removeFixtureItem(Fixture* fixture)
{
    QTreeWidgetItem* item;

    Q_ASSERT(fixture != NULL);

    item = fixtureItem(fixture->id());
    delete item;
}

void SceneEditor::slotNameEdited(const QString& name)
{
    m_scene->setName(name);
}

void SceneEditor::slotAddFixtureClicked()
{
    /* Put all fixtures already present into a list of fixtures that
       will be disabled in the fixture selection dialog */
    QList <quint32> disabled;
    QTreeWidgetItemIterator twit(m_tree);
    while (*twit != NULL)
    {
        disabled.append((*twit)->text(KColumnID).toInt());
        twit++;
    }

    /* Get a list of new fixtures to add to the scene */
    FixtureSelection fs(this, m_doc);
    fs.setMultiSelection(true);
    fs.setDisabledFixtures(disabled);
    if (fs.exec() == QDialog::Accepted)
    {
        Fixture* fixture;

        QListIterator <quint32> it(fs.selection());
        while (it.hasNext() == true)
        {
            fixture = m_doc->fixture(it.next());
            Q_ASSERT(fixture != NULL);

            addFixtureItem(fixture);
            addFixtureTab(fixture);
        }
    }
}

void SceneEditor::slotRemoveFixtureClicked()
{
    int r = QMessageBox::question(
                this, tr("Remove fixtures"),
                tr("Do you want to remove the selected fixture(s)?"),
                QMessageBox::Yes, QMessageBox::No);

    if (r == QMessageBox::Yes)
    {
        QListIterator <Fixture*> it(selectedFixtures());
        while (it.hasNext() == true)
        {
            Fixture* fixture = it.next();
            Q_ASSERT(fixture != NULL);

            removeFixtureTab(fixture);
            removeFixtureItem(fixture);

            /* Remove all values associated to the fixture */
            for (quint32 i = 0; i < fixture->channels(); i++)
                m_scene->unsetValue(fixture->id(), i);
        }
    }
}

void SceneEditor::slotEnableAll()
{
    for (int i = KTabFirstFixture; i < m_tab->count(); i++)
    {
        FixtureConsole* fc = consoleTab(i);
        if (fc != NULL)
            fc->setChecked(true);
    }
}

void SceneEditor::slotDisableAll()
{
    for (int i = KTabFirstFixture; i < m_tab->count(); i++)
    {
        FixtureConsole* fc = consoleTab(i);
        if (fc != NULL)
            fc->setChecked(false);
    }
}

void SceneEditor::slotFadeInSpinChanged(int ms)
{
    m_scene->setFadeInSpeed(ms);
}

void SceneEditor::slotFadeOutSpinChanged(int ms)
{
    m_scene->setFadeOutSpeed(ms);
}

/*****************************************************************************
 * Fixture tabs
 *****************************************************************************/

FixtureConsole* SceneEditor::fixtureConsole(Fixture* fixture)
{
    Q_ASSERT(fixture != NULL);

    /* Start from the first fixture tab */
    for (int i = KTabFirstFixture; i < m_tab->count(); i++)
    {
        FixtureConsole* fc = consoleTab(i);
        if (fc != NULL && fc->fixture() == fixture->id())
            return fc;
    }

    return NULL;
}

void SceneEditor::addFixtureTab(Fixture* fixture)
{
    Q_ASSERT(fixture != NULL);

    /* Put the console inside a scroll area */
    QScrollArea* scrollArea = new QScrollArea(m_tab);

    FixtureConsole* console = new FixtureConsole(scrollArea, m_doc);
    console->setFixture(fixture->id());
    scrollArea->setWidget(console);
    scrollArea->setWidgetResizable(true);
    m_tab->addTab(scrollArea, fixture->name());

    /* Start off with all channels disabled */
    console->setChecked(false);

    connect(console, SIGNAL(valueChanged(quint32,quint32,uchar)),
            this, SLOT(slotValueChanged(quint32,quint32,uchar)));
    connect(console, SIGNAL(checked(quint32,quint32,bool)),
            this, SLOT(slotChecked(quint32,quint32,bool)));
}

void SceneEditor::removeFixtureTab(Fixture* fixture)
{
    Q_ASSERT(fixture != NULL);

    /* Start searching from the first fixture tab */
    for (int i = KTabFirstFixture; i < m_tab->count(); i++)
    {
        FixtureConsole* fc = consoleTab(i);
        if (fc != NULL && fc->fixture() == fixture->id())
        {
            /* First remove the tab because otherwise Qt might
               remove two tabs -- undocumented feature, which
               might be intended or it might not. */
            QScrollArea* area = qobject_cast<QScrollArea*> (m_tab->widget(i));
            Q_ASSERT(area != NULL);
            m_tab->removeTab(i);
            delete area; // Deletes also FixtureConsole
            break;
        }
    }
}

FixtureConsole* SceneEditor::consoleTab(int tab)
{
    if (tab >= m_tab->count() || tab <= 0)
        return NULL;

    QScrollArea* area = qobject_cast<QScrollArea*> (m_tab->widget(tab));
    Q_ASSERT(area != NULL);

    return qobject_cast<FixtureConsole*> (area->widget());
}

void SceneEditor::slotValueChanged(quint32 fxi, quint32 channel, uchar value)
{
    // Don't modify m_scene contents when doing initialization
    if (m_initFinished == true)
    {
        Q_ASSERT(m_scene != NULL);
        m_scene->setValue(SceneValue(fxi, channel, value));
    }

    if (m_source != NULL)
        m_source->set(fxi, channel, value);
}

void SceneEditor::slotChecked(quint32 fxi, quint32 channel, bool state)
{
    // Don't modify m_scene contents when doing initialization
    if (m_initFinished == true)
    {
        // When a channel is enabled, its current value is emitted with valueChanged().
        // So, state == true case doesn't need to be handled here.
        Q_ASSERT(m_scene != NULL);
        if (state == false)
        {
            m_scene->unsetValue(fxi, channel);
            if (m_source != NULL)
                m_source->unset(fxi, channel);
        }
    }
}
