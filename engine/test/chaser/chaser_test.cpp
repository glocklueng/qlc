/*
  Q Light Controller - Unit test
  chaser_test.cpp

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

#include <QtTest>
#include <QtXml>

#define protected public
#define private public
#include "mastertimer_stub.h"
#include "universearray.h"
#include "chaserrunner.h"
#include "chaser_test.h"
#include "chaserstep.h"
#include "collection.h"
#include "function.h"
#include "fixture.h"
#include "chaser.h"
#include "scene.h"
#include "doc.h"
#include "efx.h"
#include "bus.h"
#undef protected
#undef private

#include "qlcchannel.h"
#include "qlcfile.h"

#define INTERNAL_FIXTUREDIR "../../fixtures/"

void Chaser_Test::initTestCase()
{
    m_doc = NULL;
}

void Chaser_Test::cleanupTestCase()
{
    delete m_doc;
}

void Chaser_Test::init()
{
    m_doc = new Doc(this);
}

void Chaser_Test::cleanup()
{
    m_doc->clearContents();
}

void Chaser_Test::initial()
{
    Chaser c(m_doc);
    QVERIFY(c.type() == Function::Chaser);
    QVERIFY(c.name() == "New Chaser");
    QVERIFY(c.steps().size() == 0);
    QVERIFY(c.direction() == Chaser::Forward);
    QVERIFY(c.runOrder() == Chaser::Loop);
    QVERIFY(c.id() == Function::invalidId());
    QVERIFY(c.m_runner == NULL);
    QCOMPARE(c.m_legacyHoldBus, Bus::invalid());
}

void Chaser_Test::directionRunOrder()
{
    Chaser c(m_doc);

    QVERIFY(c.direction() == Chaser::Forward);
    QVERIFY(c.runOrder() == Chaser::Loop);

    c.setDirection(Chaser::Backward);
    QVERIFY(c.direction() == Chaser::Backward);

    c.setRunOrder(Chaser::PingPong);
    QVERIFY(c.runOrder() == Chaser::PingPong);

    c.setDirection(Chaser::Forward);
    QVERIFY(c.direction() == Chaser::Forward);

    c.setRunOrder(Chaser::SingleShot);
    QVERIFY(c.runOrder() == Chaser::SingleShot);

    c.setDirection(Chaser::Backward);
    QVERIFY(c.direction() == Chaser::Backward);

    c.setRunOrder(Chaser::Loop);
    QVERIFY(c.runOrder() == Chaser::Loop);

    /* Check that invalid direction results in a sane fallback value */
    c.setDirection(Chaser::Direction(15));
    QVERIFY(c.direction() == Chaser::Forward);

    /* Check that invalid run order results in a sane fallback value */
    c.setRunOrder(Chaser::RunOrder(42));
    QVERIFY(c.runOrder() == Chaser::Loop);
}

void Chaser_Test::steps()
{
    Chaser c(m_doc);
    c.setID(50);
    QVERIFY(c.steps().size() == 0);

    /* A chaser should not be allowed to be its own member */
    QVERIFY(c.addStep(ChaserStep(50)) == false);
    QVERIFY(c.steps().size() == 0);

    /* Add a function with id "12" to the chaser */
    c.addStep(12);
    QVERIFY(c.steps().size() == 1);
    QVERIFY(c.steps().at(0) == ChaserStep(12));

    /* Add another function in the middle */
    c.addStep(ChaserStep(34));
    QVERIFY(c.steps().size() == 2);
    QVERIFY(c.steps().at(0) == ChaserStep(12));
    QVERIFY(c.steps().at(1) == ChaserStep(34));

    /* Must be able to add the same function multiple times */
    c.addStep(ChaserStep(12));
    QVERIFY(c.steps().size() == 3);
    QVERIFY(c.steps().at(0) == ChaserStep(12));
    QVERIFY(c.steps().at(1) == ChaserStep(34));
    QVERIFY(c.steps().at(2) == ChaserStep(12));

    /* Removing a non-existent index should make no modifications */
    QVERIFY(c.removeStep(3) == false);
    QVERIFY(c.steps().size() == 3);
    QVERIFY(c.steps().at(0) == ChaserStep(12));
    QVERIFY(c.steps().at(1) == ChaserStep(34));
    QVERIFY(c.steps().at(2) == ChaserStep(12));

    /* Removing the last step should succeed */
    QVERIFY(c.removeStep(2) == true);
    QVERIFY(c.steps().size() == 2);
    QVERIFY(c.steps().at(0) == ChaserStep(12));
    QVERIFY(c.steps().at(1) == ChaserStep(34));

    /* Removing the first step should succeed */
    QVERIFY(c.removeStep(0) == true);
    QVERIFY(c.steps().size() == 1);
    QVERIFY(c.steps().at(0) == ChaserStep(34));

    /* Removing the only step should succeed */
    QVERIFY(c.removeStep(0) == true);
    QVERIFY(c.steps().size() == 0);
}

void Chaser_Test::clear()
{
    Chaser c(m_doc);
    c.setID(50);
    QCOMPARE(c.steps().size(), 0);

    c.addStep(ChaserStep(0));
    c.addStep(ChaserStep(1));
    c.addStep(ChaserStep(2));
    c.addStep(ChaserStep(470));
    QCOMPARE(c.steps().size(), 4);

    QSignalSpy spy(&c, SIGNAL(changed(quint32)));
    c.clear();
    QCOMPARE(c.steps().size(), 0);
    QCOMPARE(spy.size(), 1);
    QCOMPARE(spy.at(0).size(), 1);
    QCOMPARE(spy.at(0).at(0).toUInt(), quint32(50));
}

void Chaser_Test::functionRemoval()
{
    Chaser c(m_doc);
    c.setID(42);
    QVERIFY(c.steps().size() == 0);

    QVERIFY(c.addStep(ChaserStep(0)) == true);
    QVERIFY(c.addStep(ChaserStep(1)) == true);
    QVERIFY(c.addStep(ChaserStep(2)) == true);
    QVERIFY(c.addStep(ChaserStep(3)) == true);
    QVERIFY(c.steps().size() == 4);

    /* Simulate function removal signal with an uninteresting function id */
    c.slotFunctionRemoved(6);
    QVERIFY(c.steps().size() == 4);

    /* Simulate function removal signal with a function in the chaser */
    c.slotFunctionRemoved(1);
    QVERIFY(c.steps().size() == 3);
    QVERIFY(c.steps().at(0) == ChaserStep(0));
    QVERIFY(c.steps().at(1) == ChaserStep(2));
    QVERIFY(c.steps().at(2) == ChaserStep(3));

    /* Simulate function removal signal with an invalid function id */
    c.slotFunctionRemoved(Function::invalidId());
    QVERIFY(c.steps().size() == 3);
    QVERIFY(c.steps().at(0) == ChaserStep(0));
    QVERIFY(c.steps().at(1) == ChaserStep(2));
    QVERIFY(c.steps().at(2) == ChaserStep(3));

    /* Simulate function removal signal with a function in the chaser */
    c.slotFunctionRemoved(0);
    QVERIFY(c.steps().size() == 2);
    QVERIFY(c.steps().at(0) == ChaserStep(2));
    QVERIFY(c.steps().at(1) == ChaserStep(3));
}

void Chaser_Test::copyFrom()
{
    Chaser c1(m_doc);
    c1.setName("First");
    c1.setDirection(Chaser::Backward);
    c1.setRunOrder(Chaser::PingPong);
    c1.setFadeInSpeed(42);
    c1.setFadeOutSpeed(69);
    c1.setDuration(1337);
    c1.addStep(ChaserStep(2));
    c1.addStep(ChaserStep(0));
    c1.addStep(ChaserStep(1));
    c1.addStep(ChaserStep(25));

    /* Verify that chaser contents are copied */
    Chaser c2(m_doc);
    QSignalSpy spy(&c2, SIGNAL(changed(quint32)));
    QVERIFY(c2.copyFrom(&c1) == true);
    QCOMPARE(spy.size(), 1);
    QVERIFY(c2.name() == c1.name());
    QVERIFY(c2.fadeInSpeed() == 42);
    QVERIFY(c2.fadeOutSpeed() == 69);
    QVERIFY(c2.duration() == 1337);
    QVERIFY(c2.direction() == Chaser::Backward);
    QVERIFY(c2.runOrder() == Chaser::PingPong);
    QVERIFY(c2.steps().size() == 4);
    QVERIFY(c2.steps().at(0) == ChaserStep(2));
    QVERIFY(c2.steps().at(1) == ChaserStep(0));
    QVERIFY(c2.steps().at(2) == ChaserStep(1));
    QVERIFY(c2.steps().at(3) == ChaserStep(25));

    /* Verify that a Chaser gets a copy only from another Chaser */
    Scene s(m_doc);
    QVERIFY(c2.copyFrom(&s) == false);

    /* Make a third Chaser */
    Chaser c3(m_doc);
    c3.setName("Third");
    c3.setFadeInSpeed(142);
    c3.setFadeOutSpeed(169);
    c3.setDuration(11337);
    c3.setDirection(Chaser::Forward);
    c3.setRunOrder(Chaser::Loop);
    c3.addStep(ChaserStep(15));
    c3.addStep(ChaserStep(94));
    c3.addStep(ChaserStep(3));

    /* Verify that copying TO the same Chaser a second time succeeds and
       that steps are not appended but replaced completely. */
    QVERIFY(c2.copyFrom(&c3) == true);
    QVERIFY(c2.name() == c3.name());
    QVERIFY(c2.fadeInSpeed() == 142);
    QVERIFY(c2.fadeOutSpeed() == 169);
    QVERIFY(c2.duration() == 11337);
    QVERIFY(c2.direction() == Chaser::Forward);
    QVERIFY(c2.runOrder() == Chaser::Loop);
    QVERIFY(c2.steps().size() == 3);
    QVERIFY(c2.steps().at(0) == ChaserStep(15));
    QVERIFY(c2.steps().at(1) == ChaserStep(94));
    QVERIFY(c2.steps().at(2) == ChaserStep(3));
}

void Chaser_Test::createCopy()
{
    Doc doc(this);

    Chaser* c1 = new Chaser(m_doc);
    c1->setName("First");
    c1->setFadeInSpeed(42);
    c1->setFadeOutSpeed(69);
    c1->setDuration(1337);
    c1->setDirection(Chaser::Backward);
    c1->setRunOrder(Chaser::SingleShot);
    c1->addStep(ChaserStep(20));
    c1->addStep(ChaserStep(30));
    c1->addStep(ChaserStep(40));

    doc.addFunction(c1);
    QVERIFY(c1->id() != Function::invalidId());

    Function* f = c1->createCopy(&doc);
    QVERIFY(f != NULL);
    QVERIFY(f != c1);
    QVERIFY(f->id() != c1->id());

    Chaser* copy = qobject_cast<Chaser*> (f);
    QVERIFY(copy != NULL);
    QVERIFY(copy->fadeInSpeed() == 42);
    QVERIFY(copy->fadeOutSpeed() == 69);
    QVERIFY(copy->duration() == 1337);
    QVERIFY(copy->direction() == Chaser::Backward);
    QVERIFY(copy->runOrder() == Chaser::SingleShot);
    QVERIFY(copy->steps().size() == 3);
    QVERIFY(copy->steps().at(0) == ChaserStep(20));
    QVERIFY(copy->steps().at(1) == ChaserStep(30));
    QVERIFY(copy->steps().at(2) == ChaserStep(40));
}

void Chaser_Test::loadSuccessLegacy()
{
    QDomDocument doc;

    QDomElement root = doc.createElement("Function");
    root.setAttribute("Type", "Chaser");

    QDomElement bus = doc.createElement("Bus");
    bus.setAttribute("Role", "Hold");
    QDomText busText = doc.createTextNode("16");
    bus.appendChild(busText);
    root.appendChild(bus);

    QDomElement dir = doc.createElement("Direction");
    QDomText dirText = doc.createTextNode("Backward");
    dir.appendChild(dirText);
    root.appendChild(dir);

    QDomElement run = doc.createElement("RunOrder");
    QDomText runText = doc.createTextNode("SingleShot");
    run.appendChild(runText);
    root.appendChild(run);

    QDomElement s1 = doc.createElement("Step");
    s1.setAttribute("Number", 1);
    QDomText s1Text = doc.createTextNode("50");
    s1.appendChild(s1Text);
    root.appendChild(s1);

    QDomElement s2 = doc.createElement("Step");
    s2.setAttribute("Number", 2);
    QDomText s2Text = doc.createTextNode("12");
    s2.appendChild(s2Text);
    root.appendChild(s2);

    QDomElement s3 = doc.createElement("Step");
    s3.setAttribute("Number", 0);
    QDomText s3Text = doc.createTextNode("87");
    s3.appendChild(s3Text);
    root.appendChild(s3);

    // Unknown tag
    QDomElement foo = doc.createElement("Foo");
    foo.setAttribute("Number", 3);
    QDomText fooText = doc.createTextNode("1");
    foo.appendChild(fooText);
    root.appendChild(foo);

    Bus::instance()->setValue(16, MasterTimer::frequency());

    Chaser c(m_doc);
    QVERIFY(c.loadXML(&root) == true);
    QVERIFY(c.direction() == Chaser::Backward);
    QVERIFY(c.runOrder() == Chaser::SingleShot);
    QCOMPARE(c.steps().size(), 3);
    QVERIFY(c.steps().at(0) == ChaserStep(87));
    QVERIFY(c.steps().at(1) == ChaserStep(50));
    QVERIFY(c.steps().at(2) == ChaserStep(12));

    // postLoad() removes nonexistent functions so let's check this here
    c.postLoad();
    QCOMPARE(c.duration(), MasterTimer::frequency() * MasterTimer::tick());
}

void Chaser_Test::loadSuccess()
{
    QDomDocument doc;

    QDomElement root = doc.createElement("Function");
    root.setAttribute("Type", "Chaser");

    QDomElement speed = doc.createElement("Speed");
    speed.setAttribute("FadeIn", "42");
    speed.setAttribute("FadeOut", "69");
    speed.setAttribute("Duration", "1337");
    root.appendChild(speed);

    QDomElement dir = doc.createElement("Direction");
    QDomText dirText = doc.createTextNode("Backward");
    dir.appendChild(dirText);
    root.appendChild(dir);

    QDomElement run = doc.createElement("RunOrder");
    QDomText runText = doc.createTextNode("SingleShot");
    run.appendChild(runText);
    root.appendChild(run);

    QDomElement s1 = doc.createElement("Step");
    s1.setAttribute("Number", 1);
    s1.setAttribute("FadeIn", 600);
    s1.setAttribute("FadeOut", 700);
    s1.setAttribute("Duration", 800);
    QDomText s1Text = doc.createTextNode("50");
    s1.appendChild(s1Text);
    root.appendChild(s1);

    QDomElement s2 = doc.createElement("Step");
    s2.setAttribute("Number", 2);
    s2.setAttribute("FadeIn", 1600);
    s2.setAttribute("FadeOut", 1700);
    s2.setAttribute("Duration", 1800);
    QDomText s2Text = doc.createTextNode("12");
    s2.appendChild(s2Text);
    root.appendChild(s2);

    QDomElement s3 = doc.createElement("Step");
    s3.setAttribute("Number", 0);
    // Let's leave these out from this step just for test's sake
    //s3.setAttribute("FadeIn", 2600);
    //s3.setAttribute("FadeOut", 2700);
    //s3.setAttribute("Duration", 2800);
    QDomText s3Text = doc.createTextNode("87");
    s3.appendChild(s3Text);
    root.appendChild(s3);

    // Unknown tag
    QDomElement foo = doc.createElement("Foo");
    foo.setAttribute("Number", 3);
    QDomText fooText = doc.createTextNode("1");
    foo.appendChild(fooText);
    root.appendChild(foo);

    Chaser c(m_doc);
    QVERIFY(c.loadXML(&root) == true);
    QVERIFY(c.fadeInSpeed() == 42);
    QVERIFY(c.fadeOutSpeed() == 69);
    QVERIFY(c.duration() == 1337);
    QVERIFY(c.direction() == Chaser::Backward);
    QVERIFY(c.runOrder() == Chaser::SingleShot);
    QVERIFY(c.steps().size() == 3);

    QVERIFY(c.steps().at(0) == ChaserStep(87));
    QCOMPARE(c.steps().at(0).fadeIn, Function::defaultSpeed());
    QCOMPARE(c.steps().at(0).fadeOut, Function::defaultSpeed());
    QCOMPARE(c.steps().at(0).duration, Function::defaultSpeed());

    QVERIFY(c.steps().at(1) == ChaserStep(50));
    QCOMPARE(c.steps().at(1).fadeIn, uint(600));
    QCOMPARE(c.steps().at(1).fadeOut, uint(700));
    QCOMPARE(c.steps().at(1).duration, uint(800));

    QVERIFY(c.steps().at(2) == ChaserStep(12));
    QCOMPARE(c.steps().at(2).fadeIn, uint(1600));
    QCOMPARE(c.steps().at(2).fadeOut, uint(1700));
    QCOMPARE(c.steps().at(2).duration, uint(1800));
}

void Chaser_Test::loadWrongType()
{
    QDomDocument doc;

    QDomElement root = doc.createElement("Function");
    root.setAttribute("Type", "Scene");

    QDomElement bus = doc.createElement("Bus");
    bus.setAttribute("Role", "Hold");
    QDomText busText = doc.createTextNode("16");
    bus.appendChild(busText);
    root.appendChild(bus);

    QDomElement dir = doc.createElement("Direction");
    QDomText dirText = doc.createTextNode("Backward");
    dir.appendChild(dirText);
    root.appendChild(dir);

    QDomElement run = doc.createElement("RunOrder");
    QDomText runText = doc.createTextNode("SingleShot");
    run.appendChild(runText);
    root.appendChild(run);

    QDomElement s1 = doc.createElement("Step");
    s1.setAttribute("Number", 1);
    QDomText s1Text = doc.createTextNode("50");
    s1.appendChild(s1Text);
    root.appendChild(s1);

    QDomElement s2 = doc.createElement("Step");
    s2.setAttribute("Number", 2);
    QDomText s2Text = doc.createTextNode("12");
    s2.appendChild(s2Text);
    root.appendChild(s2);

    QDomElement s3 = doc.createElement("Step");
    s3.setAttribute("Number", 0);
    QDomText s3Text = doc.createTextNode("87");
    s3.appendChild(s3Text);
    root.appendChild(s3);

    Chaser c(m_doc);
    QVERIFY(c.loadXML(&root) == false);
}

void Chaser_Test::loadWrongRoot()
{
    QDomDocument doc;

    QDomElement root = doc.createElement("Chaser");
    root.setAttribute("Type", "Chaser");

    QDomElement bus = doc.createElement("Bus");
    bus.setAttribute("Role", "Hold");
    QDomText busText = doc.createTextNode("16");
    bus.appendChild(busText);
    root.appendChild(bus);

    QDomElement dir = doc.createElement("Direction");
    QDomText dirText = doc.createTextNode("Backward");
    dir.appendChild(dirText);
    root.appendChild(dir);

    QDomElement run = doc.createElement("RunOrder");
    QDomText runText = doc.createTextNode("SingleShot");
    run.appendChild(runText);
    root.appendChild(run);

    QDomElement s1 = doc.createElement("Step");
    s1.setAttribute("Number", 1);
    QDomText s1Text = doc.createTextNode("50");
    s1.appendChild(s1Text);
    root.appendChild(s1);

    QDomElement s2 = doc.createElement("Step");
    s2.setAttribute("Number", 2);
    QDomText s2Text = doc.createTextNode("12");
    s2.appendChild(s2Text);
    root.appendChild(s2);

    QDomElement s3 = doc.createElement("Step");
    s3.setAttribute("Number", 0);
    QDomText s3Text = doc.createTextNode("87");
    s3.appendChild(s3Text);
    root.appendChild(s3);

    Chaser c(m_doc);
    QVERIFY(c.loadXML(&root) == false);
}

void Chaser_Test::postLoad()
{
    Scene* sc1 = new Scene(m_doc);
    m_doc->addFunction(sc1);

    Scene* sc2 = new Scene(m_doc);
    m_doc->addFunction(sc2);

    Chaser* ch1 = new Chaser(m_doc);
    m_doc->addFunction(ch1);

    Chaser* ch2 = new Chaser(m_doc);
    m_doc->addFunction(ch2);

    Collection* co1 = new Collection(m_doc);
    m_doc->addFunction(co1);

    Collection* co2 = new Collection(m_doc);
    m_doc->addFunction(co2);

    EFX* ef1 = new EFX(m_doc);
    m_doc->addFunction(ef1);

    EFX* ef2 = new EFX(m_doc);
    m_doc->addFunction(ef2);

    QDomDocument doc;

    QDomElement root = doc.createElement("Function");
    root.setAttribute("Type", "Chaser");

    QDomElement bus = doc.createElement("Bus");
    bus.setAttribute("Role", "Hold");
    QDomText busText = doc.createTextNode("16");
    bus.appendChild(busText);
    root.appendChild(bus);

    QDomElement dir = doc.createElement("Direction");
    QDomText dirText = doc.createTextNode("Backward");
    dir.appendChild(dirText);
    root.appendChild(dir);

    QDomElement run = doc.createElement("RunOrder");
    QDomText runText = doc.createTextNode("SingleShot");
    run.appendChild(runText);
    root.appendChild(run);

    QDomElement step0 = doc.createElement("Step");
    step0.setAttribute("Number", 0);
    QDomText step0Text = doc.createTextNode(QString::number(sc1->id()));
    step0.appendChild(step0Text);
    root.appendChild(step0);

    QDomElement step1 = doc.createElement("Step");
    step1.setAttribute("Number", 1);
    QDomText step1Text = doc.createTextNode(QString::number(sc2->id()));
    step1.appendChild(step1Text);
    root.appendChild(step1);

    QDomElement step2 = doc.createElement("Step");
    step2.setAttribute("Number", 2);
    QDomText step2Text = doc.createTextNode(QString::number(ch1->id()));
    step2.appendChild(step2Text);
    root.appendChild(step2);

    QDomElement step3 = doc.createElement("Step");
    step3.setAttribute("Number", 3);
    QDomText step3Text = doc.createTextNode(QString::number(ch2->id()));
    step3.appendChild(step3Text);
    root.appendChild(step3);

    QDomElement step4 = doc.createElement("Step");
    step4.setAttribute("Number", 4);
    QDomText step4Text = doc.createTextNode(QString::number(co1->id()));
    step4.appendChild(step4Text);
    root.appendChild(step4);

    QDomElement step5 = doc.createElement("Step");
    step5.setAttribute("Number", 5);
    QDomText step5Text = doc.createTextNode(QString::number(co2->id()));
    step5.appendChild(step5Text);
    root.appendChild(step5);

    QDomElement step6 = doc.createElement("Step");
    step6.setAttribute("Number", 6);
    QDomText step6Text = doc.createTextNode(QString::number(ef1->id()));
    step6.appendChild(step6Text);
    root.appendChild(step6);

    QDomElement step7 = doc.createElement("Step");
    step7.setAttribute("Number", 7);
    QDomText step7Text = doc.createTextNode(QString::number(ef2->id()));
    step7.appendChild(step7Text);
    root.appendChild(step7);

    // Nonexistent function
    QDomElement step8 = doc.createElement("Step");
    step8.setAttribute("Number", 8);
    QDomText step8Text = doc.createTextNode(QString::number(INT_MAX - 1));
    step8.appendChild(step8Text);
    root.appendChild(step8);

    // Unknown tag
    QDomElement foo = doc.createElement("Foo");
    foo.setAttribute("Number", 3);
    QDomText fooText = doc.createTextNode("1");
    foo.appendChild(fooText);
    root.appendChild(foo);

    Chaser c(m_doc);
    QCOMPARE(c.loadXML(&root), true);
    QCOMPARE(c.direction(), Chaser::Backward);
    QCOMPARE(c.runOrder(), Chaser::SingleShot);
    QCOMPARE(c.steps().size(), 9);

    c.postLoad();
    QCOMPARE(c.steps().size(), 8);
    QCOMPARE(c.steps().at(0), ChaserStep(sc1->id()));
    QCOMPARE(c.steps().at(1), ChaserStep(sc2->id()));
    QCOMPARE(c.steps().at(2), ChaserStep(ch1->id()));
    QCOMPARE(c.steps().at(3), ChaserStep(ch2->id()));
    QCOMPARE(c.steps().at(4), ChaserStep(co1->id()));
    QCOMPARE(c.steps().at(5), ChaserStep(co2->id()));
    QCOMPARE(c.steps().at(6), ChaserStep(ef1->id()));
    QCOMPARE(c.steps().at(7), ChaserStep(ef2->id()));
}

void Chaser_Test::save()
{
    Chaser c(m_doc);
    c.setDirection(Chaser::Backward);
    c.setRunOrder(Chaser::SingleShot);
    c.setFadeInSpeed(42);
    c.setFadeOutSpeed(69);
    c.setDuration(1337);
    c.addStep(ChaserStep(3));
    c.addStep(ChaserStep(1));
    c.addStep(ChaserStep(0));
    c.addStep(ChaserStep(2));

    QDomDocument doc;
    QDomElement root = doc.createElement("TestRoot");

    QVERIFY(c.saveXML(&doc, &root) == true);
    QVERIFY(root.firstChild().toElement().tagName() == "Function");
    QVERIFY(root.firstChild().toElement().attribute("Type") == "Chaser");

    int run = 0, dir = 0, speed = 0, fids = 0;

    QDomNode node = root.firstChild().firstChild();
    while (node.isNull() == false)
    {
        QDomElement tag = node.toElement();
        if (tag.tagName() == "Bus")
        {
            QFAIL("Chaser must not write a Bus tag anymore!");
        }
        else if (tag.tagName() == "Direction")
        {
            QVERIFY(tag.text() == "Backward");
            dir++;
        }
        else if (tag.tagName() == "RunOrder")
        {
            QVERIFY(tag.text() == "SingleShot");
            run++;
        }
        else if (tag.tagName() == "Step")
        {
            quint32 fid = tag.text().toUInt();
            QVERIFY(fid == 0 || fid == 1 || fid == 2 || fid == 3);
            fids++;
        }
        else if (tag.tagName() == "Speed")
        {
            speed++;
        }
        else
        {
            QFAIL("Unhandled XML tag.");
        }

        node = node.nextSibling();
    }

    QVERIFY(speed == 1);
    QVERIFY(dir == 1);
    QVERIFY(run == 1);
    QVERIFY(fids == 4);
}

void Chaser_Test::tap()
{
    Scene* s1 = new Scene(m_doc);
    m_doc->addFunction(s1);

    Scene* s2 = new Scene(m_doc);
    m_doc->addFunction(s2);

    Scene* s3 = new Scene(m_doc);
    m_doc->addFunction(s3);

    Scene* s4 = new Scene(m_doc);
    m_doc->addFunction(s4);

    Chaser* c = new Chaser(m_doc);
    c->addStep(s1->id());
    c->addStep(s2->id());
    c->addStep(s3->id());
    c->addStep(s4->id());
    m_doc->addFunction(c);

    c->preRun(m_doc->masterTimer());
    QVERIFY(c->m_runner != NULL);
    QCOMPARE(c->duration(), uint(0));
    c->write(m_doc->masterTimer(), NULL);
    c->tap();
    QTest::qWait(500);
    c->tap();
    QCOMPARE(c->m_runner->m_next, true);
    // Different OSs and different loads produce different results. But the exact
    // value is not important here, just check that the value is altered.
    QVERIFY(c->duration() >= uint(500 - 20) && c->duration() <= uint(500 + 20));
}

void Chaser_Test::preRun()
{
    Chaser* c = new Chaser(m_doc);
    m_doc->addFunction(c);

    UniverseArray ua(512);
    MasterTimerStub timer(m_doc, ua);

    c->m_stop = true;

    c->preRun(&timer);
    QVERIFY(c->m_runner != NULL);
    QCOMPARE(c->isRunning(), true); // Make sure Function::preRun() is called
    QCOMPARE(c->m_runner->m_elapsed, uint(0)); // Make sure ChaserRunner::reset() is called
    c->postRun(&timer, &ua);
}

void Chaser_Test::write()
{
    Fixture* fxi = new Fixture(m_doc);
    fxi->setAddress(0);
    fxi->setUniverse(0);
    fxi->setChannels(1);
    m_doc->addFixture(fxi);

    Chaser* c = new Chaser(m_doc);
    c->setDuration(MasterTimer::tick() * 10);
    m_doc->addFunction(c);

    Scene* s1 = new Scene(m_doc);
    s1->setValue(fxi->id(), 0, 255);
    m_doc->addFunction(s1);
    c->addStep(s1->id());

    Scene* s2 = new Scene(m_doc);
    s2->setValue(fxi->id(), 0, 127);
    m_doc->addFunction(s2);
    c->addStep(s2->id());

    MasterTimer timer(m_doc);

    QVERIFY(c->isRunning() == false);
    QVERIFY(c->stopped() == true);
    c->start(&timer);

    for (uint i = 0; i < c->duration(); i += MasterTimer::tick())
    {
        timer.timerTick();
        QVERIFY(c->isRunning() == true);
        QVERIFY(c->stopped() == false);
        QVERIFY(s1->isRunning() == true);
        QVERIFY(s2->isRunning() == false);
    }

    for (uint i = 0; i < c->duration(); i += MasterTimer::tick())
    {
        timer.timerTick();
        QVERIFY(c->isRunning() == true);
        QVERIFY(c->stopped() == false);
        QVERIFY(s1->isRunning() == false);
        QVERIFY(s2->isRunning() == true);
    }
}

void Chaser_Test::postRun()
{
    Chaser* c = new Chaser(m_doc);
    m_doc->addFunction(c);

    UniverseArray ua(512);
    MasterTimerStub timer(m_doc, ua);

    c->preRun(&timer);
    QCOMPARE(c->isRunning(), true);

    // The chaser has no steps so ChaserRunner::postrun() shouldn't do much
    c->postRun(&timer, &ua);
    QCOMPARE(c->isRunning(), false); // Make sure Function::postRun() is called
}

void Chaser_Test::adjustIntensity()
{
    Chaser* c = new Chaser(m_doc);
    m_doc->addFunction(c);

    UniverseArray ua(512);
    MasterTimerStub timer(m_doc, ua);

    c->preRun(&timer);
    c->adjustIntensity(0.5);
    QCOMPARE(c->m_runner->m_intensity, qreal(0.5));
    c->adjustIntensity(0.8);
    QCOMPARE(c->m_runner->m_intensity, qreal(0.8));
    c->adjustIntensity(1.5);
    QCOMPARE(c->m_runner->m_intensity, qreal(1.0));
    c->adjustIntensity(-0.1);
    QCOMPARE(c->m_runner->m_intensity, qreal(0.0));
    c->postRun(&timer, &ua);

    // Mustn't crash after postRun
    c->adjustIntensity(1.0);
}

QTEST_MAIN(Chaser_Test)
