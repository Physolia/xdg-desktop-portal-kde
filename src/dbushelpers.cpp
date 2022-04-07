/*
 * SPDX-FileCopyrightText: 2018-2019 Red Hat Inc
 * SPDX-FileCopyrightText: 2022 Aleix Pol Gonzalez <aleixpol@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 *
 * SPDX-FileCopyrightText: 2018-2019 Jan Grulich <jgrulich@redhat.com>
 */

#include "dbushelpers.h"

Q_DECLARE_METATYPE(VariantMapMap)

QDBusArgument &operator<<(QDBusArgument &argument, const VariantMapMap &mymap)
{
    argument.beginMap(QVariant::String, QVariant::Map);

    QMapIterator<QString, QVariantMap> i(mymap);
    while (i.hasNext()) {
        i.next();
        argument.beginMapEntry();
        argument << i.key() << i.value();
        argument.endMapEntry();
    }
    argument.endMap();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, VariantMapMap &mymap)
{
    argument.beginMap();
    mymap.clear();

    while (!argument.atEnd()) {
        QString key;
        QVariantMap value;
        argument.beginMapEntry();
        argument >> key >> value;
        argument.endMapEntry();
        mymap.insert(key, value);
    }

    argument.endMap();
    return argument;
}
