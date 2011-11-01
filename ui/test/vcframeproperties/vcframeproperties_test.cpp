/*
  Q Light Controller
  vcframeproperties_test.cpp

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

#include <QMdiArea>
#include <QFrame>
#include <QtTest>

#define protected public
#include "vcframeproperties_test.h"
#include "qlcfixturedefcache.h"
#include "vcframeproperties.h"
#include "virtualconsole.h"
#include "mastertimer.h"
#include "outputmap.h"
#include "inputmap.h"
#include "vcwidget.h"
#include "vcframe.h"
#include "doc.h"
#undef protected

void VCFrameProperties_Test::initTestCase()
{
    m_doc = NULL;
    m_area = NULL;
}

void VCFrameProperties_Test::init()
{
    m_doc = new Doc(this);
    m_area = new QMdiArea;
    VirtualConsole::createAndShow(m_area, m_doc);
}

void VCFrameProperties_Test::cleanup()
{
    delete VirtualConsole::instance();
    delete m_area;
    delete m_doc;
}

void VCFrameProperties_Test::initial()
{
    VCFrame* frame = new VCFrame(VirtualConsole::instance()->contents(), m_doc);
    frame->setAllowChildren(false);
    frame->setAllowResize(true);

    VCFrameProperties prop(m_area, frame);
    QCOMPARE(prop.m_allowChildrenCheck->isChecked(), false);
    QCOMPARE(prop.m_allowResizeCheck->isChecked(), true);
    prop.m_allowChildrenCheck->setChecked(true);
    prop.m_allowResizeCheck->setChecked(false);
    prop.accept();
    QCOMPARE(prop.allowChildren(), true);
    QCOMPARE(prop.allowResize(), false);
}

QTEST_MAIN(VCFrameProperties_Test)
