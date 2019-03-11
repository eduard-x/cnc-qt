/****************************************************************************
 * This file is a part of "grblControl" application.                        *
 * Copyright 2015 Hayrullin Denis Ravilevich                                *
 * https://github.com/Denvi/grblControl                                     *
 *                                                                          *
 * Qt, Linux developing                                                     *
 * Copyright (C) 2015-2019 by Eduard Kalinowski                             *
 * Germany, Lower Saxony, Hanover                                           *
 * eduard_kalinowski@yahoo.de                                               *
 *                                                                          *
 * C# project CNC-controller-for-mk1                                        *
 * https://github.com/selenur/CNC-controller-for-mk1                        *
 *                                                                          *
 * The Qt project                                                           *
 * https://github.com/eduard-x/cnc-qt                                       *
 *                                                                          *
 * CNC-Qt is free software; may be distributed and/or modified under the    *
 * terms of the GNU General Public License version 3 as published by the    *
 * Free Software Foundation and appearing in the file LICENSE_GPLv3         *
 * included in the packaging of this file.                                  *
 *                                                                          *
 * This program is distributed in the hope that it will be useful,          *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of           *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            *
 * GNU General Public License for more details.                             *
 *                                                                          *
 * You should have received a copy of the GNU Lesser General Public         *
 * License along with CNC-Qt. If not, see  http://www.gnu.org/licenses      *
 ****************************************************************************/

#include "GCodeTableModel.h"

//GCodeItem::GCodeItem()
//{
//    this->command = "";
//    this->state = "";
//    this->status = "";
//    this->line = 0;
//}

GCodeTableModel::GCodeTableModel(QObject *parent) :
    QAbstractTableModel(parent)
{
    m_headers << tr("#") << tr("Command") << tr("State") << tr("Response") << tr("Line") << tr("Args");
}

QVariant GCodeTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    if (index.row() >= m_data.size()) {
        return QVariant();
    }

    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        switch (index.column()) {
            case 0:
                return index.row() == this->rowCount() - 1 ? "" : QString::number(index.row() + 1);

            case 1:
                return m_data.at(index.row())->command;

            case 2:
                return m_data.at(index.row())->state;

            case 3:
                return m_data.at(index.row())->status;

            case 4:
                return m_data.at(index.row())->line;

            case 5:
                return QVariant(m_data.at(index.row())->args);
        }
    }

    if (role == Qt::TextAlignmentRole) {
        switch (index.column()) {
            case 0:
                return Qt::AlignCenter;

            default:
                return Qt::AlignVCenter;
        }
    }

    return QVariant();
}

bool GCodeTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.isValid() && role == Qt::EditRole) {
        switch (index.column()) {
            case 0:
                return false;

            case 1:
                m_data.at(index.row())->command = value.toString();
                break;

            case 2:
                m_data.at(index.row())->state = value.toString();
                break;

            case 3:
                m_data.at(index.row())->status = value.toString();
                break;

            case 4:
                m_data.at(index.row())->line = value.toInt();
                break;

            case 5:
                m_data.at(index.row())->args = value.toStringList();
                break;
        }

        emit dataChanged(index, index);
        return true;
    }

    return false;
}

bool GCodeTableModel::insertRow(int row, const QModelIndex &parent)
{
    if (row > rowCount()) {
        return false;
    }

    beginInsertRows(parent, row, row);
    m_data.insert(row, new GCodeItem());
    endInsertRows();
    return true;
}

bool GCodeTableModel::removeRow(int row, const QModelIndex &parent)
{
    //if (!index(row, 0).isValid()) return false;

    beginRemoveRows(parent, row, row);
    m_data.removeAt(row);
    endRemoveRows();
    return true;
}

void GCodeTableModel::clear()
{
    beginResetModel();

    foreach (GCodeItem * item, m_data) delete item;

    m_data.clear();
    endResetModel();
}

int GCodeTableModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)

    return m_data.size();
}

int GCodeTableModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)

    return 6;
}

QVariant GCodeTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole) {
        return QVariant();
    }

    if (orientation == Qt::Horizontal) {
        return m_headers.at(section);
    } else {
        return QString::number(section + 1);
    }
}

Qt::ItemFlags GCodeTableModel::flags(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return NULL;
    }

    if (index.column() == 1) {
        return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
    } else {
        return QAbstractTableModel::flags(index);
    }
}
