/*
 * SPDX-FileCopyrightText: 2022 Aleix Pol Gonzalez <aleixpol@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#pragma once

#include <QDBusAbstractAdaptor>
#include <QDBusObjectPath>

#include "dbushelpers.h"

class GlobalShortcutsPortal : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.freedesktop.impl.portal.GlobalShortcuts")
    Q_PROPERTY(uint version READ version CONSTANT)
public:
    explicit GlobalShortcutsPortal(QObject *parent);
    ~GlobalShortcutsPortal() override;

public: // PROPERTIES
    uint version() const;

public Q_SLOTS: // METHODS
    uint CreateSession(const QDBusObjectPath &handle,
                       const QDBusObjectPath &session_handle,
                       const QString &app_id,
                       const QVariantMap &options,
                       QVariantMap &results);
    QVariantMap ListShortcuts(const QDBusObjectPath &handle, const QDBusObjectPath &session_handle);
    QVariantMap BindShortcuts(const QDBusObjectPath &handle,
                              const QDBusObjectPath &session_handle,
                              const QStringList &shortcuts,
                              const QString &parent_window,
                              const QVariantMap &options);

Q_SIGNALS: // SIGNALS
    void Activated(const QDBusObjectPath &session_handle, const QString &shortcutId, quint64 timestamp, const QVariantMap &unused = {});
    void Deactivated(const QDBusObjectPath &session_handle, const QString &shortcutId, quint64 timestamp, const QVariantMap &unused = {});
    void ShortcutsChanged(const QDBusObjectPath &session_handle, const VariantMapMap &shortcuts);
};
