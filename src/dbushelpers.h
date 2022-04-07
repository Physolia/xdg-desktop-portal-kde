/*
 * SPDX-FileCopyrightText: 2018-2019 Red Hat Inc
 * SPDX-FileCopyrightText: 2022 Aleix Pol Gonzalez <aleixpol@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 *
 * SPDX-FileCopyrightText: 2018-2019 Jan Grulich <jgrulich@redhat.com>
 */

#pragma once

#include <QDBusArgument>
#include <QMap>
#include <QString>

/// a{sa{sv}}
using VariantMapMap = QMap<QString, QMap<QString, QVariant>>;
static const char *VariantMapMapSignature = "a{sa{sv}}";

QDBusArgument &operator<<(QDBusArgument &argument, const VariantMapMap &mymap);
const QDBusArgument &operator>>(const QDBusArgument &argument, VariantMapMap &mymap);
