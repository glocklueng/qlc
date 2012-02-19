/*
  Q Light Controller
  chaser.cpp

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

#include <QDebug>
#include <QFile>
#include <QtXml>

#include "qlcfixturedef.h"
#include "qlcfile.h"

#include "universearray.h"
#include "chaserrunner.h"
#include "mastertimer.h"
#include "chaserstep.h"
#include "function.h"
#include "fixture.h"
#include "chaser.h"
#include "scene.h"
#include "doc.h"
#include "bus.h"

#define KXMLQLCChaserSpeedModes "SpeedModes"
#define KXMLQLCChaserSpeedModeCommon "Common"
#define KXMLQLCChaserSpeedModePerStep "PerStep"
#define KXMLQLCChaserSpeedModeDefault "Default"

/*****************************************************************************
 * Initialization
 *****************************************************************************/

Chaser::Chaser(Doc* doc)
    : Function(doc, Function::Chaser)
    , m_legacyHoldBus(Bus::invalid())
    , m_fadeInMode(Default)
    , m_fadeOutMode(Default)
    , m_durationMode(Common)
    , m_runner(NULL)
{
    setName(tr("New Chaser"));

    // Listen to member Function removals
    connect(doc, SIGNAL(functionRemoved(quint32)),
            this, SLOT(slotFunctionRemoved(quint32)));
}

Chaser::~Chaser()
{
}

/*****************************************************************************
 * Copying
 *****************************************************************************/

Function* Chaser::createCopy(Doc* doc)
{
    Q_ASSERT(doc != NULL);

    Function* copy = new Chaser(doc);
    if (copy->copyFrom(this) == false || doc->addFunction(copy) == false)
    {
        delete copy;
        copy = NULL;
    }

    return copy;
}

bool Chaser::copyFrom(const Function* function)
{
    const Chaser* chaser = qobject_cast<const Chaser*> (function);
    if (chaser == NULL)
        return false;

    // Copy chaser stuff
    m_steps.clear();
    m_steps = chaser->m_steps;
    m_fadeInMode = chaser->m_fadeInMode;
    m_fadeOutMode = chaser->m_fadeOutMode;
    m_durationMode = chaser->m_durationMode;

    // Copy common function stuff
    return Function::copyFrom(function);
}

/*****************************************************************************
 * Contents
 *****************************************************************************/

bool Chaser::addStep(const ChaserStep& step, int index)
{
    if (step.fid != this->id())
    {
        m_stepListMutex.lock();
        if (index < 0)
            m_steps.append(step);
        else if (index <= m_steps.size())
            m_steps.insert(index, step);
        m_stepListMutex.unlock();

        emit changed(this->id());
        return true;
    }
    else
    {
        return false;
    }
}

bool Chaser::removeStep(int index)
{
    if (index >= 0 && index < m_steps.size())
    {
        m_stepListMutex.lock();
        m_steps.removeAt(index);
        m_stepListMutex.unlock();

        emit changed(this->id());
        return true;
    }
    else
    {
        return false;
    }
}

bool Chaser::replaceStep(const ChaserStep& step, int index)
{
    if (index >= 0 && index < m_steps.size())
    {
        m_stepListMutex.lock();
        m_steps[index] = step;
        m_stepListMutex.unlock();

        emit changed(this->id());
        return true;
    }
    else
    {
        return false;
    }
}

void Chaser::clear()
{
    m_steps.clear();
    emit changed(this->id());
}

QList <ChaserStep> Chaser::steps() const
{
    return m_steps;
}

void Chaser::slotFunctionRemoved(quint32 fid)
{
    m_stepListMutex.lock();
    int count = m_steps.removeAll(ChaserStep(fid));
    m_stepListMutex.unlock();

    if (count > 0)
        emit changed(this->id());
}

/*****************************************************************************
 * Speed modes
 *****************************************************************************/

void Chaser::setFadeInMode(Chaser::SpeedMode mode)
{
    m_fadeInMode = mode;
    emit changed(this->id());
}

Chaser::SpeedMode Chaser::fadeInMode() const
{
    return m_fadeInMode;
}

void Chaser::setFadeOutMode(Chaser::SpeedMode mode)
{
    m_fadeOutMode = mode;
    emit changed(this->id());
}

Chaser::SpeedMode Chaser::fadeOutMode() const
{
    return m_fadeOutMode;
}

void Chaser::setDurationMode(Chaser::SpeedMode mode)
{
    m_durationMode = mode;
    emit changed(this->id());
}

Chaser::SpeedMode Chaser::durationMode() const
{
    return m_durationMode;
}

QString Chaser::speedModeToString(Chaser::SpeedMode mode)
{
    if (mode == Common)
        return KXMLQLCChaserSpeedModeCommon;
    else if (mode == PerStep)
        return KXMLQLCChaserSpeedModePerStep;
    else
        return KXMLQLCChaserSpeedModeDefault;
}

Chaser::SpeedMode Chaser::stringToSpeedMode(const QString& str)
{
    if (str == KXMLQLCChaserSpeedModeCommon)
        return Common;
    else if (str == KXMLQLCChaserSpeedModePerStep)
        return PerStep;
    else
        return Default;
}

/*****************************************************************************
 * Save & Load
 *****************************************************************************/

bool Chaser::saveXML(QDomDocument* doc, QDomElement* wksp_root)
{
    QDomElement root;
    QDomText text;
    QString str;

    Q_ASSERT(doc != NULL);
    Q_ASSERT(wksp_root != NULL);

    /* Function tag */
    root = doc->createElement(KXMLQLCFunction);
    wksp_root->appendChild(root);

    root.setAttribute(KXMLQLCFunctionID, id());
    root.setAttribute(KXMLQLCFunctionType, Function::typeToString(type()));
    root.setAttribute(KXMLQLCFunctionName, name());

    /* Speed */
    saveXMLSpeed(doc, &root);

    /* Direction */
    saveXMLDirection(doc, &root);

    /* Run order */
    saveXMLRunOrder(doc, &root);

    /* Speed modes */
    QDomElement spd = doc->createElement(KXMLQLCChaserSpeedModes);
    spd.setAttribute(KXMLQLCFunctionSpeedFadeIn, speedModeToString(fadeInMode()));
    spd.setAttribute(KXMLQLCFunctionSpeedFadeOut, speedModeToString(fadeOutMode()));
    spd.setAttribute(KXMLQLCFunctionSpeedDuration, speedModeToString(durationMode()));
    root.appendChild(spd);

    /* Steps */
    int stepNumber = 0;
    QListIterator <ChaserStep> it(m_steps);
    while (it.hasNext() == true)
    {
        ChaserStep step(it.next());
        step.saveXML(doc, &root, stepNumber++);
    }

    return true;
}

bool Chaser::loadXML(const QDomElement& root)
{
    if (root.tagName() != KXMLQLCFunction)
    {
        qWarning() << Q_FUNC_INFO << "Function node not found";
        return false;
    }

    if (root.attribute(KXMLQLCFunctionType) != typeToString(Function::Chaser))
    {
        qWarning() << Q_FUNC_INFO << root.attribute(KXMLQLCFunctionType)
                   << "is not a chaser";
        return false;
    }

    /* Load chaser contents */
    QDomNode node = root.firstChild();
    while (node.isNull() == false)
    {
        QDomElement tag = node.toElement();

        if (tag.tagName() == KXMLQLCBus)
        {
            m_legacyHoldBus = tag.text().toUInt();
        }
        else if (tag.tagName() == KXMLQLCFunctionSpeed)
        {
            loadXMLSpeed(tag);
        }
        else if (tag.tagName() == KXMLQLCFunctionDirection)
        {
            loadXMLDirection(tag);
        }
        else if (tag.tagName() == KXMLQLCFunctionRunOrder)
        {
            loadXMLRunOrder(tag);
        }
        else if (tag.tagName() == KXMLQLCChaserSpeedModes)
        {
            QString str;

            str = tag.attribute(KXMLQLCFunctionSpeedFadeIn);
            setFadeInMode(stringToSpeedMode(str));

            str = tag.attribute(KXMLQLCFunctionSpeedFadeOut);
            setFadeOutMode(stringToSpeedMode(str));

            str = tag.attribute(KXMLQLCFunctionSpeedDuration);
            setDurationMode(stringToSpeedMode(str));
        }
        else if (tag.tagName() == KXMLQLCFunctionStep)
        {
            //! @todo stepNumber is useless if the steps are in the wrong order
            ChaserStep step;
            int stepNumber = -1;
            if (step.loadXML(tag, stepNumber) == true)
            {
                if (stepNumber >= m_steps.size())
                    m_steps.append(step);
                else
                    m_steps.insert(stepNumber, step);
            }
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unknown chaser tag:" << tag.tagName();
        }

        node = node.nextSibling();
    }

    return true;
}

void Chaser::postLoad()
{
    if (m_legacyHoldBus != Bus::invalid())
    {
        quint32 value = Bus::instance()->value(m_legacyHoldBus);
        setDuration((value / MasterTimer::frequency()) * 1000);
    }

    QMutableListIterator <ChaserStep> it(m_steps);
    while (it.hasNext() == true)
    {
        ChaserStep step(it.next());
        Function* function = doc()->function(step.fid);
        if (function == NULL)
            it.remove();
    }
}

/*****************************************************************************
 * Speed
 *****************************************************************************/

void Chaser::tap()
{
    Function::tap();
    if (m_runner != NULL)
        m_runner->next();
}

/*****************************************************************************
 * Running
 *****************************************************************************/

ChaserRunner* Chaser::createRunner(Chaser* self, Doc* doc)
{
    if (self == NULL || doc == NULL)
        return NULL;

    ChaserRunner* runner = new ChaserRunner(doc, self);
    Q_ASSERT(runner != NULL);
    runner->moveToThread(QCoreApplication::instance()->thread());
    runner->setParent(self);

    return runner;
}

void Chaser::preRun(MasterTimer* timer)
{
    Q_ASSERT(m_runner == NULL);
    m_runner = createRunner(this, doc());
    Function::preRun(timer);
}

void Chaser::write(MasterTimer* timer, UniverseArray* universes)
{
    Q_ASSERT(m_runner != NULL);

    if (m_runner->write(timer, universes) == false)
        stop();

    incrementElapsed();
}

void Chaser::postRun(MasterTimer* timer, UniverseArray* universes)
{
    m_runner->postRun(timer, universes);

    Q_ASSERT(m_runner != NULL);
    delete m_runner;
    m_runner = NULL;

    Function::postRun(timer, universes);
}

/*****************************************************************************
 * Intensity
 *****************************************************************************/

void Chaser::adjustIntensity(qreal fraction)
{
    if (m_runner != NULL)
        m_runner->adjustIntensity(fraction);
    Function::adjustIntensity(fraction);
}
