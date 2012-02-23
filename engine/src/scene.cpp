/*
  Q Light Controller
  scene.cpp

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

#include <QDomDocument>
#include <QDomElement>
#include <QDebug>
#include <QList>
#include <QFile>

#include "qlcfixturedef.h"
#include "qlcmacros.h"
#include "qlcfile.h"

#include "universearray.h"
#include "genericfader.h"
#include "mastertimer.h"
#include "scene.h"
#include "doc.h"
#include "bus.h"

/*****************************************************************************
 * Initialization
 *****************************************************************************/

Scene::Scene(Doc* doc) : Function(doc, Function::Scene)
    , m_legacyFadeBus(Bus::invalid())
    , m_fader(NULL)
{
    setName(tr("New Scene"));
}

Scene::~Scene()
{
    m_values.clear();
}

/*****************************************************************************
 * Copying
 *****************************************************************************/

Function* Scene::createCopy(Doc* doc)
{
    Q_ASSERT(doc != NULL);

    Function* copy = new Scene(doc);
    if (copy->copyFrom(this) == false || doc->addFunction(copy) == false)
    {
        delete copy;
        copy = NULL;
    }

    return copy;
}

bool Scene::copyFrom(const Function* function)
{
    const Scene* scene = qobject_cast<const Scene*> (function);
    if (scene == NULL)
        return false;

    m_values.clear();
    m_values = scene->m_values;

    return Function::copyFrom(function);
}

/*****************************************************************************
 * Values
 *****************************************************************************/

void Scene::setValue(const SceneValue& scv)
{
    m_valueListMutex.lock();
    int index = m_values.indexOf(scv);
    if (index == -1)
        m_values.append(scv);
    else
        m_values.replace(index, scv);
    qSort(m_values.begin(), m_values.end());
    m_valueListMutex.unlock();

    emit changed(this->id());
}

void Scene::setValue(quint32 fxi, quint32 ch, uchar value)
{
    setValue(SceneValue(fxi, ch, value));
}

void Scene::unsetValue(quint32 fxi, quint32 ch)
{
    m_valueListMutex.lock();
    m_values.removeAll(SceneValue(fxi, ch, 0));
    m_valueListMutex.unlock();

    emit changed(this->id());
}

uchar Scene::value(quint32 fxi, quint32 ch)
{
    SceneValue scv(fxi, ch, 0);
    int index = m_values.indexOf(scv);
    if (index == -1)
        return 0;
    else
        return m_values.at(index).value;
}

QList <SceneValue> Scene::values() const
{
    return m_values;
}

void Scene::clear()
{
    m_values.clear();
}

/*****************************************************************************
 * Fixtures
 *****************************************************************************/

void Scene::slotFixtureRemoved(quint32 fxi_id)
{
    QMutableListIterator <SceneValue> it(m_values);
    while (it.hasNext() == true)
    {
        SceneValue scv = it.next();
        if (scv.fxi == fxi_id)
            it.remove();
    }

    emit changed(this->id());
}

/*****************************************************************************
 * Load & Save
 *****************************************************************************/

bool Scene::saveXML(QDomDocument* doc, QDomElement* wksp_root)
{
    QDomElement root;
    QDomElement tag;
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

    /* Scene contents */
    QListIterator <SceneValue> it(m_values);
    while (it.hasNext() == true)
        it.next().saveXML(doc, &root);

    return true;
}

bool Scene::loadXML(const QDomElement& root)
{
    if (root.tagName() != KXMLQLCFunction)
    {
        qWarning() << Q_FUNC_INFO << "Function node not found";
        return false;
    }

    if (root.attribute(KXMLQLCFunctionType) != typeToString(Function::Scene))
    {
        qWarning() << Q_FUNC_INFO << "Function is not a scene";
        return false;
    }

    /* Load scene contents */
    QDomNode node = root.firstChild();
    while (node.isNull() == false)
    {
        QDomElement tag = node.toElement();

        if (tag.tagName() == KXMLQLCBus)
        {
            m_legacyFadeBus = tag.text().toUInt();
        }
        else if (tag.tagName() == KXMLQLCFunctionSpeed)
        {
            loadXMLSpeed(tag);
        }
        else if (tag.tagName() == KXMLQLCFunctionValue)
        {
            /* Channel value */
            SceneValue scv;
            if (scv.loadXML(tag) == true)
                setValue(scv);
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unknown scene tag:" << tag.tagName();
        }

        node = node.nextSibling();
    }

    return true;
}

void Scene::postLoad()
{
    // Map legacy bus speed to fixed speed values
    if (m_legacyFadeBus != Bus::invalid())
    {
        quint32 value = Bus::instance()->value(m_legacyFadeBus);
        setFadeInSpeed((value / MasterTimer::frequency()) * 1000);
        setFadeOutSpeed((value / MasterTimer::frequency()) * 1000);
    }

    // Remove such fixtures and channels that don't exist
    QMutableListIterator <SceneValue> it(m_values);
    while (it.hasNext() == true)
    {
        SceneValue value(it.next());
        Fixture* fxi = doc()->fixture(value.fxi);
        if (fxi == NULL || fxi->channel(value.channel) == NULL)
            it.remove();
    }
}

/****************************************************************************
 * Flashing
 ****************************************************************************/

void Scene::flash(MasterTimer* timer)
{
    if (flashing() == true)
        return;

    Q_ASSERT(timer != NULL);
    Function::flash(timer);
    timer->registerDMXSource(this);
}

void Scene::unFlash(MasterTimer* timer)
{
    if (flashing() == false)
        return;

    Q_ASSERT(timer != NULL);
    Function::unFlash(timer);
}

void Scene::writeDMX(MasterTimer* timer, UniverseArray* ua)
{
    Q_ASSERT(timer != NULL);
    Q_ASSERT(ua != NULL);

    if (flashing() == true)
    {
        // Keep HTP and LTP channels up. Flash is more or less a forceful intervention
        // so enforce all values that the user has chosen to flash.
        foreach (const SceneValue& sv, m_values)
        {
            FadeChannel fc;
            fc.setFixture(sv.fxi);
            fc.setChannel(sv.channel);
            ua->write(fc.address(doc()), sv.value, fc.group(doc()));
        }
    }
    else
    {
        timer->unregisterDMXSource(this);
    }
}

/****************************************************************************
 * Running
 ****************************************************************************/

void Scene::preRun(MasterTimer* timer)
{
    Q_ASSERT(m_fader == NULL);
    m_fader = new GenericFader(doc());
    m_fader->adjustIntensity(intensity());
    Function::preRun(timer);
}

void Scene::write(MasterTimer* timer, UniverseArray* ua)
{
    Q_UNUSED(timer);
    Q_ASSERT(ua != NULL);
    Q_ASSERT(m_fader != NULL);

    if (m_values.size() == 0)
    {
        stop();
        return;
    }

    if (elapsed() == 0)
    {
        m_valueListMutex.lock();
        QListIterator <SceneValue> it(m_values);
        while (it.hasNext() == true)
        {
            SceneValue value(it.next());

            FadeChannel fc;
            fc.setFixture(value.fxi);
            fc.setChannel(value.channel);
            fc.setTarget(value.value);
            if (overrideFadeInSpeed() == defaultSpeed())
                fc.setFadeTime(fadeInSpeed());
            else
                fc.setFadeTime(overrideFadeInSpeed());
            insertStartValue(fc, timer, ua);
            m_fader->add(fc);
        }
        m_valueListMutex.unlock();
    }

    // Run the internal GenericFader
    m_fader->write(ua);

    // Fader has nothing to do. Stop.
    if (m_fader->channels().size() == 0)
        stop();

    incrementElapsed();
}

void Scene::postRun(MasterTimer* timer, UniverseArray* ua)
{
    QHashIterator <FadeChannel,FadeChannel> it(m_fader->channels());
    while (it.hasNext() == true)
    {
        it.next();
        FadeChannel fc = it.value();

        if (fc.group(doc()) == QLCChannel::Intensity)
        {
            fc.setStart(fc.current(intensity()));
            fc.setTarget(0);
            fc.setElapsed(0);
            fc.setReady(false);
            if (overrideFadeOutSpeed() == defaultSpeed())
                fc.setFadeTime(fadeOutSpeed());
            else
                fc.setFadeTime(overrideFadeOutSpeed());
            timer->fader()->add(fc);
        }
    }

    Q_ASSERT(m_fader != NULL);
    delete m_fader;
    m_fader = NULL;

    Function::postRun(timer, ua);
}

void Scene::insertStartValue(FadeChannel& fc, const MasterTimer* timer,
                             const UniverseArray* ua)
{
    const QHash <FadeChannel,FadeChannel>& channels(timer->fader()->channels());
    if (channels.contains(fc) == true)
    {
        // MasterTimer's GenericFader contains the channel so grab its current
        // value as the new starting value to get a smoother fade
        FadeChannel existing = channels[fc];
        fc.setStart(existing.current());
        fc.setCurrent(fc.start());
    }
    else
    {
        // MasterTimer didn't have the channel. Grab the starting value from UniverseArray.
        quint32 address = fc.address(doc());
        if (fc.group(doc()) != QLCChannel::Intensity)
            fc.setStart(ua->preGMValues()[address]);
        else
            fc.setStart(0); // HTP channels must start at zero
        fc.setCurrent(fc.start());
    }
}

/****************************************************************************
 * Intensity
 ****************************************************************************/

void Scene::adjustIntensity(qreal intensity)
{
    if (m_fader != NULL)
        m_fader->adjustIntensity(intensity);
    Function::adjustIntensity(intensity);
}
