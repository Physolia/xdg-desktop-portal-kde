/*
 * SPDX-FileCopyrightText: 2018 Red Hat Inc
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 *
 * SPDX-FileCopyrightText: 2018 Jan Grulich <jgrulich@redhat.com>
 */

#ifndef XDG_DESKTOP_PORTAL_KDE_SESSION_H
#define XDG_DESKTOP_PORTAL_KDE_SESSION_H

#include <QDBusVirtualObject>
#include <QObject>
#include <QShortcut>

#include "globalshortcuts.h"
#include "remotedesktop.h"
#include "screencast.h"

class KGlobalAccelInterface;
class KGlobalAccelComponentInterface;

class Session : public QDBusVirtualObject
{
    Q_OBJECT
public:
    explicit Session(QObject *parent = nullptr, const QString &appId = QString(), const QString &path = QString());
    ~Session() override;

    enum SessionType {
        ScreenCast = 0,
        RemoteDesktop = 1,
        GlobalShortcuts = 2,
    };

    bool handleMessage(const QDBusMessage &message, const QDBusConnection &connection) override;
    QString introspect(const QString &path) const override;

    bool close();
    virtual SessionType type() const = 0;

    static Session *createSession(QObject *parent, SessionType type, const QString &appId, const QString &path);
    static Session *getSession(const QString &sessionHandle);

    QString handle() const
    {
        return m_path;
    }

Q_SIGNALS:
    void closed();

protected:
    const QString m_appId;
    const QString m_path;
};

class ScreenCastSession : public Session
{
    Q_OBJECT
public:
    explicit ScreenCastSession(QObject *parent = nullptr, const QString &appId = QString(), const QString &path = QString());
    ~ScreenCastSession() override;

    void setOptions(const QVariantMap &options);

    ScreenCastPortal::CursorModes cursorMode() const;
    bool multipleSources() const;
    ScreenCastPortal::SourceType types() const;

    SessionType type() const override
    {
        return SessionType::ScreenCast;
    }
    QVariant restoreData() const
    {
        return m_restoreData;
    }
    ScreenCastPortal::PersistMode persistMode() const
    {
        return m_persistMode;
    }

private:
    bool m_multipleSources;
    ScreenCastPortal::CursorModes m_cursorMode;
    ScreenCastPortal::SourceType m_types;
    ScreenCastPortal::PersistMode m_persistMode = ScreenCastPortal::NoPersist;

    QVariant m_restoreData;
};

class RemoteDesktopSession : public ScreenCastSession
{
    Q_OBJECT
public:
    explicit RemoteDesktopSession(QObject *parent = nullptr, const QString &appId = QString(), const QString &path = QString());
    ~RemoteDesktopSession() override;

    RemoteDesktopPortal::DeviceTypes deviceTypes() const;
    void setDeviceTypes(RemoteDesktopPortal::DeviceTypes deviceTypes);

    bool screenSharingEnabled() const;
    void setScreenSharingEnabled(bool enabled);

    SessionType type() const override
    {
        return SessionType::RemoteDesktop;
    }

private:
    bool m_screenSharingEnabled;
    RemoteDesktopPortal::DeviceTypes m_deviceTypes;
};

class GlobalShortcutsSession : public Session
{
    Q_OBJECT
public:
    explicit GlobalShortcutsSession(QObject *parent, const QString &appId, const QString &path);
    ~GlobalShortcutsSession() override;

    SessionType type() const override
    {
        return SessionType::GlobalShortcuts;
    }

    void restoreActions(const QVariant &shortcurts);
    QHash<QString, QAction *> shortcuts() const
    {
        return m_shortcuts;
    }

    QVariant shortcutDescriptionsVariant() const;
    VariantMapMap shortcutDescriptions() const;
    KGlobalAccelComponentInterface *component() const
    {
        return m_component;
    }
    QString componentName() const
    {
        return m_appId + m_token;
    }

Q_SIGNALS:
    void shortcurtsChanged();
    void shortcutActivated(const QString &shortcutName, qlonglong timestamp);
    void shortcutDeactivated(const QString &shortcutName, qlonglong timestamp);

private:
    const QString m_token;
    QHash<QString, QAction *> m_shortcuts;
    KGlobalAccelInterface *const m_globalAccelInterface;
    KGlobalAccelComponentInterface *const m_component;
};

#endif // XDG_DESKTOP_PORTAL_KDE_SESSION_H
