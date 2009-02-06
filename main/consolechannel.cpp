/*
  Q Light Controller
  consolechannel.cpp

  Copyright (c) Heikki Junnila

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  Versio 2 as published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details. The license is
  in the file "COPYING".

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include <QContextMenuEvent>
#include <QIntValidator>
#include <QVBoxLayout>
#include <QToolButton>
#include <QLineEdit>
#include <QSlider>
#include <QLabel>
#include <QMenu>
#include <QList>
#include <QtXml>

#include "common/qlcchannel.h"
#include "common/qlccapability.h"

#include "app.h"
#include "doc.h"
#include "scene.h"
#include "fixture.h"
#include "outputmap.h"
#include "consolechannel.h"

extern App* _app;

#define CMARGIN_LEFT      1
#define CMARGIN_TOP       1
#define CMARGIN_TOP_CHECK 15 /* Leave some space for the check box */
#define CMARGIN_RIGHT     1
#define CMARGIN_BOTTOM    1

/*****************************************************************************
 * Initialization
 *****************************************************************************/

ConsoleChannel::ConsoleChannel(QWidget* parent, t_fixture_id fixtureID,
			       t_channel channel) : QGroupBox(parent)
{
	/* Set the class name as the object name */
	setObjectName(ConsoleChannel::staticMetaObject.className());

	Q_ASSERT(fixtureID != KNoID);
	m_fixtureID = fixtureID;

	// Check that we have an actual fixture
	m_fixture = _app->doc()->fixture(m_fixtureID);
	Q_ASSERT(m_fixture != NULL);

	// Check that the given channel is valid
	Q_ASSERT(channel != KChannelInvalid);
	Q_ASSERT(channel < m_fixture->channels());
	m_channel = channel;

	m_value = 0;
	m_menu = NULL;
	m_outputDMX = true;

	m_presetButton = NULL;
	m_validator = NULL;
	m_valueEdit = NULL;
	m_valueSlider = NULL;
	m_numberLabel = NULL;
	
	setFixedWidth(50);

	init();
	updateValue();
}

ConsoleChannel::~ConsoleChannel()
{
}

void ConsoleChannel::init()
{
	const QLCChannel* ch;
	QString num;

	setCheckable(true);
	connect(this, SIGNAL(toggled(bool)), this, SLOT(slotToggled(bool)));

	new QVBoxLayout(this);
	layout()->setAlignment(Qt::AlignHCenter);
	layout()->setContentsMargins(CMARGIN_LEFT, CMARGIN_TOP_CHECK,
				     CMARGIN_RIGHT, CMARGIN_BOTTOM);

	/* Create a button only if its menu has sophisticated contents */
	if (m_fixture->fixtureDef() != NULL && m_fixture->fixtureMode() != NULL)
	{
		m_presetButton = new QToolButton(this);
		m_presetButton->setIconSize(QSize(26, 26));
		layout()->addWidget(m_presetButton);
		layout()->setAlignment(m_presetButton, Qt::AlignHCenter);
		m_presetButton->setSizePolicy(QSizePolicy::Maximum,
					      QSizePolicy::Preferred);
		initMenu();
	}

	m_valueEdit = new QLineEdit(this);
	layout()->addWidget(m_valueEdit);
	m_valueEdit->setAlignment(Qt::AlignCenter);
	m_validator = new QIntValidator(0, 255, this);
	m_valueEdit->setValidator(m_validator);
	m_valueEdit->setMinimumSize(QSize(1, 1));

	m_valueSlider = new QSlider(this);
	m_valueSlider->setStyle(App::sliderStyle());
	layout()->addWidget(m_valueSlider);
	m_valueSlider->setInvertedAppearance(false);
	m_valueSlider->setRange(0, 255);
	m_valueSlider->setPageStep(1);
	m_valueSlider->setSizePolicy(QSizePolicy::Preferred,
				     QSizePolicy::Expanding);

	m_numberLabel = new QLabel(this);
	layout()->addWidget(m_numberLabel);
	m_numberLabel->setAlignment(Qt::AlignCenter);

	// Generic fixtures don't have channel objects
	ch = m_fixture->channel(m_channel);
	if (ch != NULL)
		this->setToolTip(ch->name());
	else
		this->setToolTip(tr("Level"));

	// Set channel label
	num.sprintf("%d", m_channel + 1);
	m_numberLabel->setText(num);

	connect(m_valueEdit, SIGNAL(textEdited(const QString&)),
		this, SLOT(slotValueEdited(const QString&)));
	connect(m_valueSlider, SIGNAL(valueChanged(int)),
		this, SLOT(slotValueChange(int)));
}

/*****************************************************************************
 * Menu
 *****************************************************************************/

void ConsoleChannel::initMenu()
{
	const QLCChannel* ch;

	Q_ASSERT(m_fixture != NULL);

	ch = m_fixture->channel(m_channel);
	Q_ASSERT(ch != NULL);

	// Get rid of a possible previous menu
	if (m_menu != NULL)
	{
		delete m_menu;
		m_menu = NULL;
	}

	// Create a popup menu and set the channel name as its title
	m_menu = new QMenu(this);
	m_presetButton->setMenu(m_menu);
	m_presetButton->setPopupMode(QToolButton::InstantPopup);
	m_menu->setTitle(ch->name());

	if (ch->group() == KQLCChannelGroupPan)
		m_presetButton->setIcon(QIcon(":/pan.png"));
	else if (ch->group() == KQLCChannelGroupTilt)
		m_presetButton->setIcon(QIcon(":/tilt.png"));
	else if (ch->group() == KQLCChannelGroupColour)
	{
		if (ch->name().contains("red", Qt::CaseInsensitive) == true)
		{
			QPalette pal = m_presetButton->palette();
			pal.setColor(QPalette::Button, QColor("red"));
			m_presetButton->setPalette(pal);
		}
		else if (ch->name().contains("green", Qt::CaseInsensitive) == true)
		{
			QPalette pal = m_presetButton->palette();
			pal.setColor(QPalette::Button, QColor("green"));
			m_presetButton->setPalette(pal);
		}
		else if (ch->name().contains("blue", Qt::CaseInsensitive) == true)
		{
			QPalette pal = m_presetButton->palette();
			pal.setColor(QPalette::Button, QColor("blue"));
			m_presetButton->setPalette(pal);
		}
		else if (ch->name().contains("cyan", Qt::CaseInsensitive) == true)
		{
			QPalette pal = m_presetButton->palette();
			pal.setColor(QPalette::Button, QColor("cyan"));
			m_presetButton->setPalette(pal);
		}
		else if (ch->name().contains("magenta", Qt::CaseInsensitive) == true)
		{
			QPalette pal = m_presetButton->palette();
			pal.setColor(QPalette::Button, QColor("magenta"));
			m_presetButton->setPalette(pal);
		}
		else if (ch->name().contains("yellow", Qt::CaseInsensitive) == true)
		{
			QPalette pal = m_presetButton->palette();
			pal.setColor(QPalette::Button, QColor("yellow"));
			m_presetButton->setPalette(pal);
		}

		m_presetButton->setIcon(QIcon(":/color.png"));
	}
	else if (ch->group() == KQLCChannelGroupGobo)
		m_presetButton->setIcon(QIcon(":/gobo.png"));
	else if (ch->group() == KQLCChannelGroupShutter)
		m_presetButton->setIcon(QIcon(":/shutter.png"));
	else if (ch->group() == KQLCChannelGroupSpeed)
		m_presetButton->setIcon(QIcon(":/speed.png"));
	else if (ch->group() == KQLCChannelGroupPrism)
		m_presetButton->setIcon(QIcon(":/prism.png"));
	else if (ch->group() == KQLCChannelGroupMaintenance)
		m_presetButton->setIcon(QIcon(":/configure.png"));
	else if (ch->group() == KQLCChannelGroupIntensity)
		m_presetButton->setIcon(QIcon(":/intensity.png"));
	else if (ch->group() == KQLCChannelGroupBeam)
		m_presetButton->setIcon(QIcon(":/beam.png"));

	// Initialize the preset menu only for normal fixtures,
	// i.e. not for Generic dimmer fixtures
	if (m_fixture->fixtureDef() != NULL && m_fixture->fixtureMode() != NULL)
		initCapabilityMenu(ch);
}

void ConsoleChannel::initCapabilityMenu(const QLCChannel* ch)
{
	QLCCapability* cap;
	QMenu* valueMenu;
	QAction* action;
	QString s;
	QString t;

	QListIterator <QLCCapability*> it(ch->capabilities());
	while (it.hasNext() == true)
	{
		cap = it.next();

		// Set the value range and name as the menu item's name
		s = QString("%1: %2 - %3").arg(cap->name())
			.arg(cap->min()).arg(cap->max());

		if (cap->max() - cap->min() > 0)
		{
			// Create submenu for ranges of more than one value
			valueMenu = new QMenu(m_menu);
			valueMenu->setTitle(s);

			/* Add a color icon */
			if (ch->group() == KQLCChannelGroupColour)
				valueMenu->setIcon(colorIcon(cap->name()));

			for (int i = cap->min(); i <= cap->max(); i++)
			{
				action = valueMenu->addAction(
					t.sprintf("%.3d", i));
				action->setData(i);
			}

			m_menu->addMenu(valueMenu);
		}
		else
		{
			// Just one value in this range, put that into the menu
			action = m_menu->addAction(s);
			action->setData(cap->min());

			/* Add a color icon */
			if (ch->group() == KQLCChannelGroupColour)
				action->setIcon(colorIcon(cap->name()));
		}
	}

	// Connect menu item activation signal to this
	connect(m_menu, SIGNAL(triggered(QAction*)),
		this, SLOT(slotContextMenuTriggered(QAction*)));

	// Set the menu also as the preset button's popup menu
	m_presetButton->setMenu(m_menu);
}

const QIcon ConsoleChannel::colorIcon(const QString& name)
{
	QString colname(name.toLower().remove(QRegExp("[0-9]")).remove(' '));
	if (QColor::colorNames().contains(colname) == true)
	{
		QColor col(colname);
		QPixmap pm(32, 32);
		pm.fill(col);
		return QIcon(pm);
	}

	return QIcon();
}

void ConsoleChannel::contextMenuEvent(QContextMenuEvent* e)
{
	// Show the preset menu only of it has been created.
	// Generic dimmer fixtures don't have capabilities and so
	// they will not have these menus either.
	if (m_menu != NULL)
	{
		m_menu->exec(e->globalPos());
		e->accept();
	}
}

void ConsoleChannel::slotContextMenuTriggered(QAction* action)
{
	Q_ASSERT(action != NULL);

	// The menuitem's data contains a valid DMX value
	setValue(action->data().toInt());
}

/*****************************************************************************
 * Value
 *****************************************************************************/

int ConsoleChannel::sliderValue() const
{
	return m_valueSlider->value();
}

void ConsoleChannel::updateValue()
{
	t_value value = 0;

	Q_ASSERT(m_fixture != NULL);

	value = _app->outputMap()->value(m_fixture->universeAddress() +
					 m_channel);

	m_valueEdit->setText(QString("%1").arg(value));
	setValue(value);
}

void ConsoleChannel::setOutputDMX(bool state)
{
	m_outputDMX = state;

	/* When output is enabled again, update the current value to DMX */
	if (state == true)
		_app->outputMap()->setValue(m_fixture->universeAddress() +
					    m_channel, (t_value) m_value);
	else
		_app->outputMap()->setValue(m_fixture->universeAddress() +
					    m_channel, 0); /* Nasty...? */
}

void ConsoleChannel::setValue(t_value value)
{
	m_valueSlider->setValue(static_cast<int> (value));
}

void ConsoleChannel::slotValueEdited(const QString& text)
{
	setValue(text.toInt());
}

void ConsoleChannel::slotValueChange(int value)
{
	if (m_outputDMX == true)
		_app->outputMap()->setValue(m_fixture->universeAddress() +
					    m_channel, (t_value) value);

	m_valueEdit->setText(QString("%1").arg(value));
	m_value = value;

	emit valueChanged(m_channel, m_value, isEnabled());
}

/*****************************************************************************
 * Enable/disable
 *****************************************************************************/

void ConsoleChannel::enable(bool state)
{
	setChecked(state);

	m_value = _app->outputMap()->value(m_fixture->universeAddress() +
					   m_channel);

	emit valueChanged(m_channel, m_value, isEnabled());
}

void ConsoleChannel::slotToggled(bool state)
{
	emit valueChanged(m_channel, m_value, state);
}

/*****************************************************************************
 * Checkable
 *****************************************************************************/

void ConsoleChannel::setCheckable(bool checkable)
{
	if (layout() != NULL)
	{
		if (checkable == true)
		{
			layout()->setContentsMargins(CMARGIN_LEFT,
						     CMARGIN_TOP_CHECK,
						     CMARGIN_RIGHT,
						     CMARGIN_BOTTOM);
		}
		else
		{
			layout()->setContentsMargins(CMARGIN_LEFT,
						     CMARGIN_TOP,
						     CMARGIN_RIGHT,
						     CMARGIN_BOTTOM);
		}
	}

	QGroupBox::setCheckable(checkable);
}
