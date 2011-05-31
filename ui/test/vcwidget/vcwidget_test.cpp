/*
  Q Light Controller
  vcwidget_test.cpp

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

#include <QFrame>
#include <QtTest>

#define protected public
#define private public
#include "virtualconsole.h"
#include "vcwidget.h"
#undef private
#undef protected

#include "qlcfixturedefcache.h"
#include "vcwidget_test.h"
#include "mastertimer.h"
#include "stubwidget.h"
#include "outputmap.h"
#include "inputmap.h"
#include "doc.h"
#include "bus.h"

void VCWidget_Test::initTestCase()
{
    Bus::init(this);
}

void VCWidget_Test::initial()
{
    QLCFixtureDefCache fdc;
    Doc doc(this, fdc);
    OutputMap om(this, 4);
    InputMap im(this, 4);
    MasterTimer mt(this, &om);
    QWidget w;

    StubWidget stub(&w, &doc, &om, &im, &mt);
    QCOMPARE(stub.objectName(), QString("VCWidget"));
    QCOMPARE(stub.hasCustomBackgroundColor(), false);
    QCOMPARE(stub.hasCustomForegroundColor(), false);
    QCOMPARE(stub.hasCustomFont(), false);
    QCOMPARE(stub.frameStyle(), 0);
    QCOMPARE(stub.inputUniverse(), InputMap::invalidUniverse());
    QCOMPARE(stub.inputChannel(), InputMap::invalidChannel());
    QCOMPARE(stub.allowChildren(), false);
    QCOMPARE(stub.customMenu(0), (QMenu*) 0);
}

void VCWidget_Test::bgImage()
{
    QLCFixtureDefCache fdc;
    Doc doc(this, fdc);
    OutputMap om(this, 4);
    InputMap im(this, 4);
    MasterTimer mt(this, &om);
    QWidget w;

    QSignalSpy spy(&doc, SIGNAL(modified(bool)));

    StubWidget stub(&w, &doc, &om, &im, &mt);
    QCOMPARE(stub.backgroundImage(), QString());
    QCOMPARE(stub.hasCustomBackgroundColor(), false);

    stub.setBackgroundImage("../../../gfx/qlc.png");
    QCOMPARE(stub.backgroundImage(), QString("../../../gfx/qlc.png"));
    QCOMPARE(stub.palette().brush(QPalette::Window).texture().isNull(), false);
    QCOMPARE(stub.hasCustomBackgroundColor(), false);
    QCOMPARE(spy.size(), 1);

    stub.setBackgroundColor(QColor(Qt::red));
    QCOMPARE(spy.size(), 2);

    stub.setBackgroundImage("../../../gfx/qlc.png");
    QCOMPARE(stub.backgroundImage(), QString("../../../gfx/qlc.png"));
    QCOMPARE(stub.palette().brush(QPalette::Window).texture().isNull(), false);
    QCOMPARE(stub.hasCustomBackgroundColor(), false);
    QCOMPARE(spy.size(), 3);
}

void VCWidget_Test::bgColor()
{
    QLCFixtureDefCache fdc;
    Doc doc(this, fdc);
    OutputMap om(this, 4);
    InputMap im(this, 4);
    MasterTimer mt(this, &om);
    QWidget w;

    QSignalSpy spy(&doc, SIGNAL(modified(bool)));

    StubWidget stub(&w, &doc, &om, &im, &mt);
    QCOMPARE(stub.backgroundColor(), w.palette().color(QPalette::Window));
    QCOMPARE(stub.hasCustomBackgroundColor(), false);

    stub.setBackgroundColor(QColor(Qt::red));
    QCOMPARE(stub.backgroundImage(), QString());
    QCOMPARE(stub.backgroundColor(), QColor(Qt::red));
    QCOMPARE(stub.hasCustomBackgroundColor(), true);
    QCOMPARE(stub.palette().brush(QPalette::Window).texture().isNull(), true);
    QCOMPARE(stub.palette().brush(QPalette::Window).color(), QColor(Qt::red));
    QCOMPARE(spy.size(), 1);

    stub.setBackgroundImage("../../../gfx/qlc.png");
    QCOMPARE(spy.size(), 2);

    stub.setBackgroundColor(QColor(Qt::red));
    QCOMPARE(stub.backgroundImage(), QString());
    QCOMPARE(stub.backgroundColor(), QColor(Qt::red));
    QCOMPARE(stub.palette().brush(QPalette::Window).texture().isNull(), true);
    QCOMPARE(stub.palette().brush(QPalette::Window).color(), QColor(Qt::red));
    QCOMPARE(spy.size(), 3);
}

void VCWidget_Test::fgColor()
{
    QLCFixtureDefCache fdc;
    Doc doc(this, fdc);
    OutputMap om(this, 4);
    InputMap im(this, 4);
    MasterTimer mt(this, &om);
    QWidget w;

    QSignalSpy spy(&doc, SIGNAL(modified(bool)));

    StubWidget stub(&w, &doc, &om, &im, &mt);
    QCOMPARE(stub.backgroundColor(), w.palette().color(QPalette::Window));
    QCOMPARE(stub.hasCustomBackgroundColor(), false);

    stub.setForegroundColor(QColor(Qt::blue));
    QCOMPARE(stub.foregroundColor(), QColor(Qt::blue));
    QCOMPARE(stub.hasCustomForegroundColor(), true);
    QCOMPARE(stub.palette().brush(QPalette::WindowText).texture().isNull(), true);
    QCOMPARE(stub.palette().brush(QPalette::WindowText).color(), QColor(Qt::blue));
    QCOMPARE(spy.size(), 1);
}

void VCWidget_Test::resetBg()
{
    QLCFixtureDefCache fdc;
    Doc doc(this, fdc);
    OutputMap om(this, 4);
    InputMap im(this, 4);
    MasterTimer mt(this, &om);
    QWidget w;

    QSignalSpy spy(&doc, SIGNAL(modified(bool)));

    StubWidget stub(&w, &doc, &om, &im, &mt);
    stub.setBackgroundColor(QColor(Qt::red));
    stub.setForegroundColor(QColor(Qt::cyan));
    QCOMPARE(spy.size(), 2);

    stub.resetBackgroundColor();
    QCOMPARE(stub.backgroundImage(), QString());
    QCOMPARE(stub.backgroundColor(), w.palette().color(QPalette::Window));
    QCOMPARE(stub.hasCustomBackgroundColor(), false);
    QCOMPARE(stub.palette().brush(QPalette::Window).color(), w.palette().color(QPalette::Window));
    QCOMPARE(stub.foregroundColor(), QColor(Qt::cyan));
    QCOMPARE(stub.hasCustomForegroundColor(), true);
    QCOMPARE(stub.palette().brush(QPalette::WindowText).texture().isNull(), true);
    QCOMPARE(stub.palette().brush(QPalette::WindowText).color(), QColor(Qt::cyan));
    QCOMPARE(spy.size(), 3);

    stub.setBackgroundImage("../../../gfx/qlc.png");
    QCOMPARE(spy.size(), 4);

    stub.resetBackgroundColor();
    QCOMPARE(stub.backgroundImage(), QString());
    QCOMPARE(stub.backgroundColor(), w.palette().color(QPalette::Window));
    QCOMPARE(stub.hasCustomBackgroundColor(), false);
    QCOMPARE(stub.palette().brush(QPalette::Window).color(), w.palette().color(QPalette::Window));
    QCOMPARE(stub.foregroundColor(), QColor(Qt::cyan));
    QCOMPARE(stub.hasCustomForegroundColor(), true);
    QCOMPARE(stub.palette().brush(QPalette::WindowText).texture().isNull(), true);
    QCOMPARE(stub.palette().brush(QPalette::WindowText).color(), QColor(Qt::cyan));
    QCOMPARE(spy.size(), 5);
}

void VCWidget_Test::resetFg()
{
    QLCFixtureDefCache fdc;
    Doc doc(this, fdc);
    OutputMap om(this, 4);
    InputMap im(this, 4);
    MasterTimer mt(this, &om);
    QWidget w;

    QSignalSpy spy(&doc, SIGNAL(modified(bool)));

    StubWidget stub(&w, &doc, &om, &im, &mt);
    stub.setBackgroundColor(QColor(Qt::red));
    stub.setForegroundColor(QColor(Qt::cyan));
    QCOMPARE(spy.size(), 2);

    stub.resetForegroundColor();
    QCOMPARE(stub.backgroundImage(), QString());
    QCOMPARE(stub.backgroundColor(), QColor(Qt::red));
    QCOMPARE(stub.hasCustomBackgroundColor(), true);
    QCOMPARE(stub.palette().brush(QPalette::Window).texture().isNull(), true);
    QCOMPARE(stub.palette().brush(QPalette::Window).color(), QColor(Qt::red));
    QCOMPARE(stub.foregroundColor(), w.palette().color(QPalette::WindowText));
    QCOMPARE(stub.hasCustomForegroundColor(), false);
    QCOMPARE(stub.palette().brush(QPalette::WindowText).texture().isNull(), true);
    QCOMPARE(stub.palette().brush(QPalette::WindowText).color(), w.palette().color(QPalette::WindowText));
    QCOMPARE(spy.size(), 3);

    stub.setBackgroundImage("../../../gfx/qlc.png");
    QCOMPARE(spy.size(), 4);

    stub.resetForegroundColor();
    QCOMPARE(stub.backgroundImage(), QString("../../../gfx/qlc.png"));
    QCOMPARE(stub.hasCustomBackgroundColor(), false);
    QCOMPARE(stub.palette().brush(QPalette::Window).texture().isNull(), false);
    QCOMPARE(stub.foregroundColor(), w.palette().color(QPalette::WindowText));
    QCOMPARE(stub.hasCustomForegroundColor(), false);
    QCOMPARE(stub.palette().brush(QPalette::WindowText).texture().isNull(), true);
    QCOMPARE(stub.palette().brush(QPalette::WindowText).color(), w.palette().color(QPalette::WindowText));
    QCOMPARE(spy.size(), 5);

    stub.resetBackgroundColor();
    QCOMPARE(spy.size(), 6);

    stub.resetForegroundColor();
    QCOMPARE(stub.backgroundImage(), QString());
    QCOMPARE(stub.hasCustomBackgroundColor(), false);
    QCOMPARE(stub.foregroundColor(), w.palette().color(QPalette::WindowText));
    QCOMPARE(stub.hasCustomForegroundColor(), false);
    QCOMPARE(stub.palette().brush(QPalette::WindowText).texture().isNull(), true);
    QCOMPARE(stub.palette().brush(QPalette::WindowText).color(), w.palette().color(QPalette::WindowText));
    QCOMPARE(spy.size(), 7);
}

void VCWidget_Test::font()
{
    QLCFixtureDefCache fdc;
    Doc doc(this, fdc);
    OutputMap om(this, 4);
    InputMap im(this, 4);
    MasterTimer mt(this, &om);
    QWidget w;

    QSignalSpy spy(&doc, SIGNAL(modified(bool)));

    QFont font(w.font());
    font.setBold(!font.bold());
    QVERIFY(font != w.font());

    StubWidget stub(&w, &doc, &om, &im, &mt);
    stub.setFont(font);
    QCOMPARE(stub.font(), font);
    QCOMPARE(stub.hasCustomFont(), true);
    QCOMPARE(spy.size(), 1);

    stub.resetFont();
    QCOMPARE(stub.font(), w.font());
    QCOMPARE(stub.hasCustomFont(), false);
    QCOMPARE(spy.size(), 2);
}

void VCWidget_Test::caption()
{
    QLCFixtureDefCache fdc;
    Doc doc(this, fdc);
    OutputMap om(this, 4);
    InputMap im(this, 4);
    MasterTimer mt(this, &om);
    QWidget w;

    QSignalSpy spy(&doc, SIGNAL(modified(bool)));

    StubWidget stub(&w, &doc, &om, &im, &mt);
    stub.setCaption("Foobar");
    QCOMPARE(stub.caption(), QString("Foobar"));
    QCOMPARE(spy.size(), 1);
}

void VCWidget_Test::frame()
{
    QLCFixtureDefCache fdc;
    Doc doc(this, fdc);
    OutputMap om(this, 4);
    InputMap im(this, 4);
    MasterTimer mt(this, &om);
    QWidget w;

    QSignalSpy spy(&doc, SIGNAL(modified(bool)));

    StubWidget stub(&w, &doc, &om, &im, &mt);
    stub.setFrameStyle(KVCFrameStyleSunken);
    QCOMPARE(stub.frameStyle(), (int) KVCFrameStyleSunken);
    QCOMPARE(spy.size(), 1);

    stub.setFrameStyle(KVCFrameStyleRaised);
    QCOMPARE(stub.frameStyle(), (int) KVCFrameStyleRaised);
    QCOMPARE(spy.size(), 2);

    stub.resetFrameStyle();
    QCOMPARE(stub.frameStyle(), (int) KVCFrameStyleNone);
    QCOMPARE(spy.size(), 3);

    QCOMPARE(stub.frameStyleToString(KVCFrameStyleSunken), QString("Sunken"));
    QCOMPARE(stub.frameStyleToString(KVCFrameStyleRaised), QString("Raised"));
    QCOMPARE(stub.frameStyleToString(KVCFrameStyleNone), QString("None"));
    QCOMPARE(stub.frameStyleToString(QFrame::Plain), QString("None"));

    QCOMPARE(stub.stringToFrameStyle("Sunken"), (int) KVCFrameStyleSunken);
    QCOMPARE(stub.stringToFrameStyle("Raised"), (int) KVCFrameStyleRaised);
    QCOMPARE(stub.stringToFrameStyle("None"), (int) KVCFrameStyleNone);
    QCOMPARE(stub.stringToFrameStyle("Foo"), (int) KVCFrameStyleNone);
}

void VCWidget_Test::inputSource()
{
    QLCFixtureDefCache fdc;
    Doc doc(this, fdc);
    OutputMap om(this, 4);
    InputMap im(this, 4);
    MasterTimer mt(this, &om);
    QWidget w;

    StubWidget stub(&w, &doc, &om, &im, &mt);
    stub.setInputSource(1, 2);
    QCOMPARE(stub.inputUniverse(), quint32(1));
    QCOMPARE(stub.inputChannel(), quint32(2));

    stub.setInputSource(4, 5);
    QCOMPARE(stub.inputUniverse(), quint32(4));
    QCOMPARE(stub.inputChannel(), quint32(5));

    stub.setInputSource(InputMap::invalidUniverse(), InputMap::invalidChannel());
    QCOMPARE(stub.inputUniverse(), InputMap::invalidUniverse());
    QCOMPARE(stub.inputChannel(), InputMap::invalidChannel());

    // Just for coverage - this method call does nothing
    stub.slotInputValueChanged(0, 1, 2);
}

void VCWidget_Test::copy()
{
    QLCFixtureDefCache fdc;
    Doc doc(this, fdc);
    OutputMap om(this, 4);
    InputMap im(this, 4);
    MasterTimer mt(this, &om);
    QWidget w;

    StubWidget stub(&w, &doc, &om, &im, &mt);
    stub.setCaption("Pertti Pasanen");
    stub.setBackgroundColor(QColor(Qt::red));
    stub.setForegroundColor(QColor(Qt::green));
    QFont font(w.font());
    font.setBold(!font.bold());
    stub.setFont(font);
    stub.setFrameStyle(KVCFrameStyleRaised);
    stub.move(QPoint(10, 20));
    stub.resize(QSize(20, 30));
    stub.setInputSource(0, 12);

    StubWidget copy(&w, &doc, &om, &im, &mt);
    copy.copyFrom(&stub);
    QCOMPARE(copy.caption(), QString("Pertti Pasanen"));
    QCOMPARE(copy.hasCustomBackgroundColor(), true);
    QCOMPARE(copy.backgroundColor(), QColor(Qt::red));
    QCOMPARE(copy.hasCustomForegroundColor(), true);
    QCOMPARE(copy.foregroundColor(), QColor(Qt::green));
    QCOMPARE(copy.font(), font);
    QCOMPARE(copy.frameStyle(), (int) KVCFrameStyleRaised);
    QCOMPARE(copy.pos(), QPoint(10, 20));
    QCOMPARE(copy.size(), QSize(20, 30));
    QCOMPARE(copy.inputUniverse(), quint32(0));
    QCOMPARE(copy.inputChannel(), quint32(12));
}

void VCWidget_Test::keyPress()
{
    QLCFixtureDefCache fdc;
    Doc doc(this, fdc);
    OutputMap om(this, 4);
    InputMap im(this, 4);
    MasterTimer mt(this, &om);
    QWidget w;

    StubWidget stub(&w, &doc, &om, &im, &mt);
    QSignalSpy pspy(&stub, SIGNAL(keyPressed(QKeySequence)));
    QSignalSpy rspy(&stub, SIGNAL(keyReleased(QKeySequence)));

    stub.slotKeyPressed(QKeySequence(QKeySequence::Copy));
    QCOMPARE(pspy.size(), 1);
    QCOMPARE(pspy[0].size(), 1);
    QCOMPARE(pspy[0][0].value<QKeySequence>(), QKeySequence(QKeySequence::Copy));

    stub.slotKeyReleased(QKeySequence(QKeySequence::Copy));
    QCOMPARE(rspy.size(), 1);
    QCOMPARE(rspy[0].size(), 1);
    QCOMPARE(rspy[0][0].value<QKeySequence>(), QKeySequence(QKeySequence::Copy));
}

void VCWidget_Test::loadInput()
{
    QLCFixtureDefCache fdc;
    Doc doc(this, fdc);
    OutputMap om(this, 4);
    InputMap im(this, 4);
    MasterTimer mt(this, &om);
    QWidget w;

    QDomDocument xmldoc;
    QDomElement root = xmldoc.createElement("Input");
    root.setAttribute("Universe", "12");
    root.setAttribute("Channel", "34");
    xmldoc.appendChild(root);

    StubWidget stub(&w, &doc, &om, &im, &mt);
    QCOMPARE(stub.loadXMLInput(&root), true);
    QCOMPARE(stub.inputUniverse(), quint32(12));
    QCOMPARE(stub.inputChannel(), quint32(34));

    root.setTagName("Output");
    QCOMPARE(stub.loadXMLInput(&root), false);
    QCOMPARE(stub.inputUniverse(), quint32(12));
    QCOMPARE(stub.inputChannel(), quint32(34));
}

void VCWidget_Test::loadAppearance()
{
    QLCFixtureDefCache fdc;
    Doc doc(this, fdc);
    OutputMap om(this, 4);
    InputMap im(this, 4);
    MasterTimer mt(this, &om);
    QWidget w;

    QDomDocument xmldoc;
    QDomElement root = xmldoc.createElement("Appearance");
    xmldoc.appendChild(root);

    QDomElement frame = xmldoc.createElement("FrameStyle");
    QDomText frameText = xmldoc.createTextNode("Sunken");
    frame.appendChild(frameText);
    root.appendChild(frame);

    QDomElement fg = xmldoc.createElement("ForegroundColor");
    QDomText fgText = xmldoc.createTextNode(QString("%1").arg(QColor(Qt::red).rgb()));
    fg.appendChild(fgText);
    root.appendChild(fg);

    QDomElement bg = xmldoc.createElement("BackgroundColor");
    QDomText bgText = xmldoc.createTextNode(QString("%1").arg(QColor(Qt::blue).rgb()));
    bg.appendChild(bgText);
    root.appendChild(bg);

    QDomElement bgImage = xmldoc.createElement("BackgroundImage");
    QDomText bgImageText = xmldoc.createTextNode("None");
    bgImage.appendChild(bgImageText);
    root.appendChild(bgImage);

    QDomElement foo = xmldoc.createElement("Foo");
    root.appendChild(foo);

    QFont font(w.font());
    font.setItalic(true);
    QDomElement fn = xmldoc.createElement("Font");
    QDomText fnText = xmldoc.createTextNode(font.toString());
    fn.appendChild(fnText);
    root.appendChild(fn);

    StubWidget stub(&w, &doc, &om, &im, &mt);
    QVERIFY(stub.loadXMLAppearance(&root) == true);
    QCOMPARE(stub.frameStyle(), (int) KVCFrameStyleSunken);
    QCOMPARE(stub.hasCustomForegroundColor(), true);
    QCOMPARE(stub.foregroundColor(), QColor(Qt::red));
    QCOMPARE(stub.hasCustomBackgroundColor(), true);
    QCOMPARE(stub.backgroundColor(), QColor(Qt::blue));
    QCOMPARE(stub.font(), font);

    fgText.setData("Default");
    bgText.setData("Default");
    bgImageText.setData("../../../gfx/qlc.png");
    QVERIFY(stub.loadXMLAppearance(&root) == true);
    QCOMPARE(stub.frameStyle(), (int) KVCFrameStyleSunken);
    QCOMPARE(stub.hasCustomForegroundColor(), false);
    QCOMPARE(stub.hasCustomBackgroundColor(), false);
    QCOMPARE(stub.backgroundImage(), QString("../../../gfx/qlc.png"));
    QCOMPARE(stub.font(), font);

    root.setTagName("Appiarenz");
    QVERIFY(stub.loadXMLAppearance(&root) == false);
    QCOMPARE(stub.frameStyle(), (int) KVCFrameStyleSunken);
    QCOMPARE(stub.hasCustomForegroundColor(), false);
    QCOMPARE(stub.hasCustomBackgroundColor(), false);
    QCOMPARE(stub.backgroundImage(), QString("../../../gfx/qlc.png"));
    QCOMPARE(stub.font(), font);
}

void VCWidget_Test::saveInput()
{
    QLCFixtureDefCache fdc;
    Doc doc(this, fdc);
    OutputMap om(this, 4);
    InputMap im(this, 4);
    MasterTimer mt(this, &om);
    QWidget w;

    StubWidget stub(&w, &doc, &om, &im, &mt);

    QDomDocument xmldoc;
    QDomElement root = xmldoc.createElement("Root");
    xmldoc.appendChild(root);

    QVERIFY(stub.saveXMLInput(&xmldoc, &root) == true);
    QCOMPARE(root.childNodes().count(), 0);

    stub.setInputSource(34, 56);
    QVERIFY(stub.saveXMLInput(&xmldoc, &root) == true);
    QCOMPARE(root.childNodes().count(), 1);
    QCOMPARE(root.firstChild().toElement().tagName(), QString("Input"));
    QCOMPARE(root.firstChild().toElement().attribute("Universe"), QString("34"));
    QCOMPARE(root.firstChild().toElement().attribute("Channel"), QString("56"));
}

void VCWidget_Test::saveAppearance()
{
    QLCFixtureDefCache fdc;
    Doc doc(this, fdc);
    OutputMap om(this, 4);
    InputMap im(this, 4);
    MasterTimer mt(this, &om);
    QWidget w;

    StubWidget stub(&w, &doc, &om, &im, &mt);
    stub.setBackgroundColor(QColor(Qt::red));
    stub.setForegroundColor(QColor(Qt::green));
    QFont fn(w.font());
    fn.setBold(!fn.bold());
    stub.setFont(fn);
    stub.setFrameStyle(KVCFrameStyleRaised);

    QDomDocument xmldoc;
    QDomElement root = xmldoc.createElement("Root");
    xmldoc.appendChild(root);

    int bgcolor = 0, bgimage = 0, fgcolor = 0, font = 0, frame = 0;

    QCOMPARE(stub.saveXMLAppearance(&xmldoc, &root), true);
    QDomNode node = root.firstChild();
    QCOMPARE(node.toElement().tagName(), QString("Appearance"));
    node = node.firstChild();
    while (node.isNull() == false)
    {
        QDomElement tag = node.toElement();
        if (tag.tagName() == "BackgroundColor")
        {
            bgcolor++;
            QCOMPARE(tag.text(), QString::number(QColor(Qt::red).rgb()));
        }
        else if (tag.tagName() == "BackgroundImage")
        {
            bgimage++;
            QCOMPARE(tag.text(), QString("None"));
        }
        else if (tag.tagName() == "ForegroundColor")
        {
            fgcolor++;
            QCOMPARE(tag.text(), QString::number(QColor(Qt::green).rgb()));
        }
        else if (tag.tagName() == "Font")
        {
            font++;
            QCOMPARE(tag.text(), fn.toString());
        }
        else if (tag.tagName() == "FrameStyle")
        {
            frame++;
            QCOMPARE(tag.text(), QString("Raised"));
        }
        else
        {
            QFAIL(QString("Unexpected tag: %1").arg(tag.tagName()).toUtf8().constData());
        }

        node = node.nextSibling();
    }

    QCOMPARE(bgcolor, 1);
    QCOMPARE(bgimage, 1);
    QCOMPARE(fgcolor, 1);
    QCOMPARE(font, 1);
    QCOMPARE(frame, 1);
}

void VCWidget_Test::saveAppearanceDefaultsImage()
{
    QLCFixtureDefCache fdc;
    Doc doc(this, fdc);
    OutputMap om(this, 4);
    InputMap im(this, 4);
    MasterTimer mt(this, &om);
    QWidget w;

    StubWidget stub(&w, &doc, &om, &im, &mt);
    stub.setBackgroundImage("../../../gfx/qlc.png");

    QDomDocument xmldoc;
    QDomElement root = xmldoc.createElement("Root");
    xmldoc.appendChild(root);

    int bgcolor = 0, bgimage = 0, fgcolor = 0, font = 0, frame = 0;

    QCOMPARE(stub.saveXMLAppearance(&xmldoc, &root), true);
    QDomNode node = root.firstChild();
    QCOMPARE(node.toElement().tagName(), QString("Appearance"));
    node = node.firstChild();
    while (node.isNull() == false)
    {
        QDomElement tag = node.toElement();
        if (tag.tagName() == "BackgroundColor")
        {
            bgcolor++;
            QCOMPARE(tag.text(), QString("Default"));
        }
        else if (tag.tagName() == "BackgroundImage")
        {
            bgimage++;
            QCOMPARE(tag.text(), QString("../../../gfx/qlc.png"));
        }
        else if (tag.tagName() == "ForegroundColor")
        {
            fgcolor++;
            QCOMPARE(tag.text(), QString("Default"));
        }
        else if (tag.tagName() == "Font")
        {
            font++;
            QCOMPARE(tag.text(), QString("Default"));
        }
        else if (tag.tagName() == "FrameStyle")
        {
            frame++;
            QCOMPARE(tag.text(), QString("None"));
        }
        else
        {
            QFAIL(QString("Unexpected tag: %1").arg(tag.tagName()).toUtf8().constData());
        }

        node = node.nextSibling();
    }

    QCOMPARE(bgcolor, 1);
    QCOMPARE(bgimage, 1);
    QCOMPARE(fgcolor, 1);
    QCOMPARE(font, 1);
    QCOMPARE(frame, 1);
}

void VCWidget_Test::saveWindowState()
{
    QLCFixtureDefCache fdc;
    Doc doc(this, fdc);
    OutputMap om(this, 4);
    InputMap im(this, 4);
    MasterTimer mt(this, &om);
    QWidget w;

    StubWidget stub(&w, &doc, &om, &im, &mt);
    w.show();
    w.resize(QSize(100, 100));
    stub.resize(QSize(30, 40));
    stub.move(QPoint(10, 20));
    stub.show();

    QDomDocument xmldoc;
    QDomElement root = xmldoc.createElement("Root");
    xmldoc.appendChild(root);

    QCOMPARE(stub.saveXMLWindowState(&xmldoc, &root), true);
    QDomElement tag = root.firstChild().toElement();
    QCOMPARE(tag.tagName(), QString("WindowState"));
    QCOMPARE(tag.attribute("X"), QString("10"));
    QCOMPARE(tag.attribute("Y"), QString("20"));
    QCOMPARE(tag.attribute("Width"), QString("30"));
    QCOMPARE(tag.attribute("Height"), QString("40"));
    QCOMPARE(tag.attribute("Visible"), QString("True"));

    root.removeChild(tag);

    w.hide();
    QCOMPARE(stub.saveXMLWindowState(&xmldoc, &root), true);
    tag = root.firstChild().toElement();
    QCOMPARE(tag.tagName(), QString("WindowState"));
    QCOMPARE(tag.attribute("X"), QString("10"));
    QCOMPARE(tag.attribute("Y"), QString("20"));
    QCOMPARE(tag.attribute("Width"), QString("30"));
    QCOMPARE(tag.attribute("Height"), QString("40"));
    QCOMPARE(tag.attribute("Visible"), QString("False"));
}

void VCWidget_Test::loadWindowState()
{
    QLCFixtureDefCache fdc;
    Doc doc(this, fdc);
    OutputMap om(this, 4);
    InputMap im(this, 4);
    MasterTimer mt(this, &om);
    QWidget parent;

    StubWidget stub(&parent, &doc, &om, &im, &mt);

    QDomDocument xmldoc;
    QDomElement root = xmldoc.createElement("WindowState");
    root.setAttribute("X", "20");
    root.setAttribute("Y", "10");
    root.setAttribute("Width", "40");
    root.setAttribute("Height", "30");
    root.setAttribute("Visible", "True");
    xmldoc.appendChild(root);

    int x = 0, y = 0, w = 0, h = 0;
    bool v = false;
    QCOMPARE(stub.loadXMLWindowState(&root, &x, &y, &w, &h, NULL), false);
    QCOMPARE(stub.loadXMLWindowState(&root, &x, &y, &w, NULL, &v), false);
    QCOMPARE(stub.loadXMLWindowState(&root, &x, &y, NULL, &h, &v), false);
    QCOMPARE(stub.loadXMLWindowState(&root, &x, NULL, &w, &h, &v), false);
    QCOMPARE(stub.loadXMLWindowState(&root, NULL, &y, &w, &h, &v), false);
    QCOMPARE(stub.loadXMLWindowState(NULL, &x, &y, &w, &h, &v), false);

    QCOMPARE(stub.loadXMLWindowState(&root, &x, &y, &w, &h, &v), true);
    QCOMPARE(x, 20);
    QCOMPARE(y, 10);
    QCOMPARE(w, 40);
    QCOMPARE(h, 30);
    QCOMPARE(v, true);

    root.setAttribute("Visible", "False");
    QCOMPARE(stub.loadXMLWindowState(&root, &x, &y, &w, &h, &v), true);
    QCOMPARE(x, 20);
    QCOMPARE(y, 10);
    QCOMPARE(w, 40);
    QCOMPARE(h, 30);
    QCOMPARE(v, false);

    root.setTagName("WinduhState");
    QCOMPARE(stub.loadXMLWindowState(&root, &x, &y, &w, &h, &v), false);
}

QTEST_MAIN(VCWidget_Test)
