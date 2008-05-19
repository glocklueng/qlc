/*
  Q Light Controller
  function.cpp

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

#include <QApplication>
#include <QMessageBox>
#include <iostream>
#include <QString>
#include <QPixmap>
#include <QtXml>

#include "common/qlcfile.h"

#include "function.h"
#include "bus.h"
#include "app.h"
#include "doc.h"

using namespace std;

extern App* _app;

const QString KSceneString      (      "Scene" );
const QString KChaserString     (     "Chaser" );
const QString KEFXString        (        "EFX" );
const QString KCollectionString ( "Collection" );
const QString KUndefinedString  (  "Undefined" );

const QString KLoopString       (       "Loop" );
const QString KPingPongString   (   "PingPong" );
const QString KSingleShotString ( "SingleShot" );

const QString KBackwardString   (   "Backward" );
const QString KForwardString    (    "Forward" );

/*****************************************************************************
 * Initialization
 *****************************************************************************/

Function::Function(QObject* parent, Type type) : QThread(parent)
{
	m_id = KNoID;
	m_name = QString::null;
	m_type = type;
	m_runOrder = Loop;
	m_direction = Forward;
	m_fixture = KNoID;
	m_busID = KBusIDInvalid;
	m_channels = 0;
	m_eventBuffer = NULL;

	// Receive bus value change signals
	connect(Bus::emitter(),	SIGNAL(valueChanged(t_bus_id, t_bus_value)),
		this, SLOT(slotBusValueChanged(t_bus_id, t_bus_value)));
}

Function::~Function()
{
}

/*****************************************************************************
 * ID
 *****************************************************************************/

void Function::setID(t_function_id id)
{
	m_id = id;
}

/*****************************************************************************
 * Name
 *****************************************************************************/

void Function::setName(QString name)
{
	m_name = QString(name);
	_app->doc()->setModified();
	_app->doc()->emitFunctionChanged(m_id);
}

/*****************************************************************************
 * Type
 *****************************************************************************/

QString Function::typeString() const
{
	return Function::typeToString(type());
}

QString Function::typeToString(const Type type)
{
	switch (type)
	{
	case Scene:
		return KSceneString;
	case Chaser:
		return KChaserString;
	case EFX:
		return KEFXString;
	case Collection:
		return KCollectionString;
	case Undefined:
	default:
		return KUndefinedString;
	}
}

Function::Type Function::stringToType(QString string)
{
	if (string == KSceneString)
		return Scene;
	else if (string == KChaserString)
		return Chaser;
	else if (string == KEFXString)
		return EFX;
	else if (string == KCollectionString)
		return Collection;
	else
		return Undefined;
}

QPixmap Function::pixmap() const
{
	switch (m_type)
	{
		case Scene:
			return QPixmap(QString(PIXMAPS) +
					QString("/scene.png"));
		case Chaser:
			return QPixmap(QString(PIXMAPS) +
					QString("/chaser.png"));
		case EFX:
			return QPixmap(QString(PIXMAPS) +
					QString("/efx.png"));
		case Collection:
			return QPixmap(QString(PIXMAPS) +
					QString("/collection.png"));
		default:
			return QPixmap(QString(PIXMAPS) +
					QString("/function.png"));
	}
}

/*****************************************************************************
 * Running order
 *****************************************************************************/

void Function::setRunOrder(Function::RunOrder order)
{
	m_runOrder = order;
	_app->doc()->setModified();
	_app->doc()->emitFunctionChanged(m_id);	
}

QString Function::runOrderToString(RunOrder order)
{
	switch (order)
	{
	case Loop:
		return KLoopString;
		break;

	case PingPong:
		return KPingPongString;
		break;

	case SingleShot:
		return KSingleShotString;
		break;

	default:
		return KUndefinedString;
		break;
	}
}

Function::RunOrder Function::stringToRunOrder(QString str)
{
	if (str == KLoopString)
		return Loop;
	else if (str == KPingPongString)
		return PingPong;
	else if (str == KSingleShotString)
		return SingleShot;
	else
		return Loop;
}

/*****************************************************************************
 * Direction
 *****************************************************************************/

void Function::setDirection(Function::Direction dir)
{
	m_direction = dir;
	_app->doc()->setModified();
	_app->doc()->emitFunctionChanged(m_id);	
}

QString Function::directionToString(Direction dir)
{
	switch (dir)
	{
	case Forward:
		return KForwardString;
		break;

	case Backward:
		return KBackwardString;
		break;

	default:
		return KUndefinedString;
		break;
	}
}

Function::Direction Function::stringToDirection(QString str)
{
	if (str == KForwardString)
		return Forward;
	else if (str == KBackwardString)
		return Backward;
	else
		return Forward;
}

/*****************************************************************************
 * Fixture
 *****************************************************************************/

void Function::setFixture(t_fixture_id id)
{
	m_fixture = id;
	_app->doc()->setModified();
	_app->doc()->emitFunctionChanged(m_id);
}

/*****************************************************************************
 * Bus
 *****************************************************************************/

void Function::setBus(t_bus_id id)
{
	if (id < KBusIDMin || id >= KBusCount)
	{
		if (m_type == Scene)
			m_busID = KBusIDDefaultFade;
		else if (m_type == Chaser)
			m_busID = KBusIDDefaultHold;
		else
			m_busID = KNoID;
	}
	else
	{
		m_busID = id;
	}
	
	_app->doc()->setModified();
	_app->doc()->emitFunctionChanged(m_id);
}

QString Function::busName() const
{
	QString text;
	
	if (busID() != KNoID)
	{
		text.sprintf("%.2d: ", busID() + 1);
		text += Bus::name(busID());
	}
	else
	{
		text = QString("N/A");
	}

	return text;
}

/*****************************************************************************
 * Load & Save
 *****************************************************************************/

Function* Function::loader(QDomDocument* doc, QDomElement* root)
{
	Function* function = NULL;
	t_fixture_id fxi_id = 0;
	t_function_id func_id = 0;
	Type func_type;
	QString func_name;
	
	Q_ASSERT(doc != NULL);
	Q_ASSERT(root != NULL);

	if (root->tagName() != KXMLQLCFunction)
	{
		qWarning("Function node not found!");
		return NULL;
	}

	/* Extract function ID */
	func_id = root->attribute(KXMLQLCFunctionID).toInt();
	Q_ASSERT(func_id >= 0 && func_id < KFunctionArraySize);

	/* Extract fixture ID (might be also invalid) */
	fxi_id = root->attribute(KXMLQLCFunctionFixture).toInt();

	/* Extract function name */
	func_name = root->attribute(KXMLQLCFunctionName);

	/* Extract function type */
	func_type = Function::stringToType(root->attribute(KXMLQLCFunctionType));

	/* Create a new function into Doc using the loaded information 
	   and continue loading the specific function contents from Doc */
	return _app->doc()->newFunction(func_type, func_id, func_name, 
					fxi_id, doc, root);
}

/*****************************************************************************
 * Running
 *****************************************************************************/

bool Function::start()
{
	if (isRunning() == true)
	{
		cout << "Function " << name().toStdString()
		     << " is already running!" << endl;
		return false;
	}
	else
	{
		QThread::start();
		return true;
	}
}
