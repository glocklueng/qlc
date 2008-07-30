/*
  Q Light Controller
  qlcinplugin.h

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

#ifndef QLCINPLUGIN_H
#define QLCINPLUGIN_H

#include <QtPlugin>

#include "qlctypes.h"

/*****************************************************************************
 * InputPlugin
 *****************************************************************************/

class QLCInPlugin
{
public:
	/**
	 * Destroy the plugin
	 */
	virtual ~QLCInPlugin() {}
	
	/**
	 * Initialize the plugin. Since plugins cannot have a user-defined
	 * constructor, any initialization prior to opening any HW must be
	 * done thru this second-stage initialization method. InputMap calls
	 * this function for all plugins exactly once after loading, before
	 * calling any other method from the plugin.
	 *
	 * This is a pure virtual function that must be implemented
	 * in all plugins.
	 */
	virtual void init() = 0;

	/**
	 * Open the plugin. InputMap calls open() whenever the plugin is used
	 * by QLC to send/receive input data and when the plugin goes dormant,
	 * InputMap calls close(). If you need to open() the plugin for
	 * configuration or some other activity, you should implement reference
	 * counting to ensure that used plugins won't get closed after the
	 * configuration dialog is closed.
	 *
	 * This is a pure virtual function that must be implemented
	 * in all plugins.
	 */
	virtual int open() = 0;

	/**
	 * Close the plugin. InputMap calls open() whenever the plugin is used
	 * by QLC to send/receive input data and when the plugin goes dormant,
	 * InputMap calls close(). If you need to open() the plugin for
	 * configuration or some other activity, you should implement reference
	 * counting to ensure that used plugins won't get closed after the
	 * configuration dialog is closed.
	 *
	 * This is a pure virtual function that must be implemented
	 * in all plugins.
	 */
	virtual int close() = 0;

	/**
	 * Invoke a configuration dialog for the plugin
	 *
	 * This is a pure virtual function that must be implemented
	 * in all plugins.
	 */
	virtual int configure() = 0;

	/**
	 * Provide an information text to be displayed in the plugin manager
	 *
	 * This is a pure virtual function that must be implemented
	 * in all plugins.
	 */
	virtual QString infoText() = 0;

	/**
	 * Get the plugin's name
	 */
	virtual QString name() = 0;

	/*********************************************************************
	 * Inputs
	 *********************************************************************/
public:
	/**
	 * Get the number of inputs provided by the plugin
	 *
	 * @todo Make pure virtual
	 *
	 * @return Number of inputs provided by the plugin
	 */
	virtual t_input inputs() = 0;

	/**
	 * Get the number of channels provided by a plugin input
	 *
	 * @todo Make pure virtual
	 *
	 * @param input An input line whose number of channels to get
	 * @return Number of input channels in the given input
	 */
	virtual t_input_channel channels(t_input input) = 0;

	/*********************************************************************
	 * Feedback
	 *********************************************************************/
public:
	/**
	 * Send a value back to an input line's channel. This method can be
	 * used for example to move motorized sliders with QLC sliders.
	 *
	 * @param input The input line to send feedback to
	 * @param channel A channel in the input line to send feedback to
	 * @param value An input value to send back to the input channel
	 */
	virtual void feedBack(t_input input, t_input_channel channel,
			      t_input_value value) = 0;
#if 0
signals:
	/**
	 * Signal a value change in an input's channel. This is THE signal that
	 * sends input values to QLC components.
	 *
	 * @param input An input line whose channel's value has changed
	 * @param channel A channel whose value has changed
	 * @param value The changed value
	 */
	void valueChanged(t_input input,
			  t_input_channel channel,
			  t_input_value value);
#endif
};

Q_DECLARE_INTERFACE(QLCInPlugin, "QLCInPlugin")

#endif
