/*
 * SPDX-FileCopyrightText: 2021 Aleix Pol Gonzalez <aleixpol@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "outputsmodel.h"
#include <KLocalizedString>
#include <QIcon>

OutputsModel::OutputsModel(QObject *parent)
    : QAbstractListModel(parent)
{
    m_outputs = WaylandIntegration::screens().values();
}

int OutputsModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_outputs.count();
}

QHash<int, QByteArray> OutputsModel::roleNames() const
{
    return QHash<int, QByteArray>{
        {Qt::DisplayRole, "display"},
        {Qt::DecorationRole, "decoration"},
        {Qt::UserRole, "outputName"},
        {Qt::CheckStateRole, "checked"},
    };
}

QVariant OutputsModel::data(const QModelIndex &index, int role) const
{
    if (!checkIndex(index, CheckIndexOption::IndexIsValid)) {
        return {};
    }

    const auto &output = m_outputs[index.row()];
    switch (role) {
    case Qt::UserRole:
        return output.waylandOutputName();
    case Qt::DecorationRole:
        switch (output.outputType()) {
        case WaylandIntegration::WaylandOutput::Laptop:
            return QIcon::fromTheme(QStringLiteral("computer-laptop"));
        case WaylandIntegration::WaylandOutput::Monitor:
            return QIcon::fromTheme(QStringLiteral("video-display"));
        case WaylandIntegration::WaylandOutput::Television:
            return QIcon::fromTheme(QStringLiteral("video-television"));
        }
    case Qt::DisplayRole:
        switch (output.outputType()) {
        case WaylandIntegration::WaylandOutput::Laptop:
            return i18n("Laptop screen\nModel: %1", output.model());
        default:
            return i18n("Manufacturer: %1\nModel: %2", output.manufacturer(), output.model());
        }
    case Qt::CheckStateRole:
        return m_selected.contains(output.waylandOutputName()) ? Qt::Checked : Qt::Unchecked;
    }
    return {};
}

bool OutputsModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!checkIndex(index, CheckIndexOption::IndexIsValid) || role != Qt::CheckStateRole) {
        return false;
    }

    if (index.data(Qt::CheckStateRole) == value) {
        return true;
    }

    const auto &output = m_outputs[index.row()];
    if (value == Qt::Checked) {
        m_selected.insert(output.waylandOutputName());
    } else {
        m_selected.remove(output.waylandOutputName());
    }
    Q_EMIT dataChanged(index, index, {role});
    if (m_selected.count() <= 1) {
        Q_EMIT hasSelectionChanged();
    }
    return true;
}

QList<quint32> OutputsModel::selectedScreens() const
{
    return m_selected.values();
}

void OutputsModel::clearSelection()
{
    if (m_selected.isEmpty())
        return;

    auto selected = m_selected;
    m_selected.clear();
    int i = 0;
    for (const auto &output : m_outputs) {
        if (selected.contains(output.waylandOutputName())) {
            const auto idx = index(i, 0);
            Q_EMIT dataChanged(idx, idx, {Qt::CheckStateRole});
        }
        ++i;
    }
    Q_EMIT hasSelectionChanged();
}
