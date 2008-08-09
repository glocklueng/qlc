/*
  Q Light Controller
  hidinput.h

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

#ifndef HIDINPUT_H
#define HIDINPUT_H

#include <QList>

#include "common/qlcinplugin.h"
#include "common/qlctypes.h"

#include "hiddevice.h"
#include "hidpoller.h"

/*****************************************************************************
 * HIDInput
 *****************************************************************************/

class HIDInput : public QObject, public QLCInPlugin
{
	Q_OBJECT
	Q_INTERFACES(QLCInPlugin)

	friend class ConfigureHIDInput;
	friend class HIDPoller;

	/*********************************************************************
	 * Initialization
	 *********************************************************************/
public:
	void init();

	/*********************************************************************
	 * Open/close
	 *********************************************************************/
public:
	int open();
	int close();

protected:
	HIDDevice* device(const QString& path);
	HIDDevice* device(const unsigned int index);

protected:
	QList <HIDDevice*> m_devices;
	int m_refCount;

	/*********************************************************************
	 * Name
	 *********************************************************************/
public:
	QString name();

	/*********************************************************************
	 * Inputs & channels
	 *********************************************************************/
public:
	t_input inputs();
	t_input_channel channels(t_input input);

	/*********************************************************************
	 * Configuration
	 *********************************************************************/
public:
	int configure();

	/*********************************************************************
	 * Status
	 *********************************************************************/
public:
	QString infoText();

	/*********************************************************************
	 * Device poller
	 *********************************************************************/
protected:
	HIDPoller* m_poller;

	/*********************************************************************
	 * Input data
	 *********************************************************************/
public:
	void feedBack(t_input input, t_input_channel channel,
		      t_input_value value);
};

#endif
