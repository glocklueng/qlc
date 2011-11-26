/*
  Q Light Controller
  cuestackmodel.h

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

#ifndef CUESTACKMODEL_H
#define CUESTACKMODEL_H

#include <QAbstractItemModel>

class CueStack;
class CueStackModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    CueStackModel(QObject* parent = 0);
    ~CueStackModel();

    void setCueStack(CueStack* cs);
    CueStack* cueStack() const;

private:
    CueStack* m_cueStack;

    /************************************************************************
     * CueStack slots
     ************************************************************************/
private slots:
    void slotAdded(int index);
    void slotRemoved(int index);
    void slotChanged(int index);
    void slotCurrentCueChanged(int index);

    /************************************************************************
     * QAbstractItemModel
     ************************************************************************/
public:
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex& index) const;
    int rowCount(const QModelIndex& parent = QModelIndex()) const;
    int columnCount(const QModelIndex& parent = QModelIndex()) const;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
};

#endif
