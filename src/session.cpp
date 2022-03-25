/*
 * SPDX-FileCopyrightText: 2018 Red Hat Inc
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 *
 * SPDX-FileCopyrightText: 2018 Jan Grulich <jgrulich@redhat.com>
 */

#include "session.h"
#include "desktopportal.h"

#include <kglobalaccel.h>
#include <kglobalaccel_component_interface.h>
#include <kglobalaccel_interface.h>

#include <QAction>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusPendingCall>
#include <QDBusPendingCallWatcher>
#include <QLoggingCategory>

#include <KLocalizedString>
#include <KeySequenceRecorder>

Q_LOGGING_CATEGORY(XdgSessionKdeSession, "xdp-kde-session")

static QMap<QString, Session *> sessionList;

Session::Session(QObject *parent, const QString &appId, const QString &path)
    : QDBusVirtualObject(parent)
    , m_appId(appId)
    , m_path(path)
{
}

Session::~Session()
{
}

bool Session::handleMessage(const QDBusMessage &message, const QDBusConnection &connection)
{
    Q_UNUSED(connection);

    if (message.path() != m_path) {
        return false;
    }

    /* Check to make sure we're getting properties on our interface */
    if (message.type() != QDBusMessage::MessageType::MethodCallMessage) {
        return false;
    }

    qCDebug(XdgSessionKdeSession) << message.interface();
    qCDebug(XdgSessionKdeSession) << message.member();
    qCDebug(XdgSessionKdeSession) << message.path();

    if (message.interface() == QLatin1String("org.freedesktop.impl.portal.Session")) {
        if (message.member() == QLatin1String("Close")) {
            close();
            Q_EMIT closed();
            QDBusMessage reply = message.createReply();
            return connection.send(reply);
        }
    } else if (message.interface() == QLatin1String("org.freedesktop.DBus.Properties")) {
        if (message.member() == QLatin1String("Get")) {
            if (message.arguments().count() == 2) {
                const QString interface = message.arguments().at(0).toString();
                const QString property = message.arguments().at(1).toString();

                if (interface == QLatin1String("org.freedesktop.impl.portal.Session") && property == QLatin1String("version")) {
                    QList<QVariant> arguments;
                    arguments << 1;

                    QDBusMessage reply = message.createReply();
                    reply.setArguments(arguments);
                    return connection.send(reply);
                }
            }
        }
    }

    return false;
}

QString Session::introspect(const QString &path) const
{
    QString nodes;

    if (path.startsWith(QLatin1String("/org/freedesktop/portal/desktop/session/"))) {
        nodes = QStringLiteral(
            "<interface name=\"org.freedesktop.impl.portal.Session\">"
            "    <method name=\"Close\">"
            "    </method>"
            "<signal name=\"Closed\">"
            "</signal>"
            "<property name=\"version\" type=\"u\" access=\"read\"/>"
            "</interface>");
    }

    qCDebug(XdgSessionKdeSession) << nodes;

    return nodes;
}

bool Session::close()
{
    QDBusMessage reply = QDBusMessage::createSignal(m_path, QStringLiteral("org.freedesktop.impl.portal.Session"), QStringLiteral("Closed"));
    const bool result = QDBusConnection::sessionBus().send(reply);

    sessionList.remove(m_path);
    QDBusConnection::sessionBus().unregisterObject(m_path);

    deleteLater();

    return result;
}

Session *Session::createSession(QObject *parent, SessionType type, const QString &appId, const QString &path)
{
    QDBusConnection sessionBus = QDBusConnection::sessionBus();

    Session *session = nullptr;
    switch (type) {
    case ScreenCast:
        session = new ScreenCastSession(parent, appId, path);
        break;
    case RemoteDesktop:
        session = new RemoteDesktopSession(parent, appId, path);
        break;
    case GlobalShortcuts:
        session = new GlobalShortcutsSession(parent, appId, path);
        break;
    }

    if (sessionBus.registerVirtualObject(path, session, QDBusConnection::VirtualObjectRegisterOption::SubPath)) {
        connect(session, &Session::closed, [session, path]() {
            sessionList.remove(path);
            QDBusConnection::sessionBus().unregisterObject(path);
            session->deleteLater();
        });
        sessionList.insert(path, session);
        return session;
    } else {
        qCDebug(XdgSessionKdeSession) << sessionBus.lastError().message();
        qCDebug(XdgSessionKdeSession) << "Failed to register session object: " << path;
        session->deleteLater();
        return nullptr;
    }
}

Session *Session::getSession(const QString &sessionHandle)
{
    return sessionList.value(sessionHandle);
}

ScreenCastSession::ScreenCastSession(QObject *parent, const QString &appId, const QString &path)
    : Session(parent, appId, path)
{
}

ScreenCastSession::~ScreenCastSession()
{
}

bool ScreenCastSession::multipleSources() const
{
    return m_multipleSources;
}

ScreenCastPortal::SourceType ScreenCastSession::types() const
{
    return m_types;
}

ScreenCastPortal::CursorModes ScreenCastSession::cursorMode() const
{
    return m_cursorMode;
}

void ScreenCastSession::setOptions(const QVariantMap &options)
{
    m_multipleSources = options.value(QStringLiteral("multiple")).toBool();
    m_cursorMode = ScreenCastPortal::CursorModes(options.value(QStringLiteral("cursor_mode")).toUInt());
    m_persistMode = ScreenCastPortal::PersistMode(options.value("persist_mode").toUInt());
    m_restoreData = options.value(QStringLiteral("restore_data"));
    m_types = ScreenCastPortal::SourceType(options.value(QStringLiteral("types")).toUInt());

    if (m_types == 0) {
        m_types = ScreenCastPortal::Monitor;
    }
}

RemoteDesktopSession::RemoteDesktopSession(QObject *parent, const QString &appId, const QString &path)
    : ScreenCastSession(parent, appId, path)
    , m_screenSharingEnabled(false)
{
}

RemoteDesktopSession::~RemoteDesktopSession()
{
}

RemoteDesktopPortal::DeviceTypes RemoteDesktopSession::deviceTypes() const
{
    return m_deviceTypes;
}

void RemoteDesktopSession::setDeviceTypes(RemoteDesktopPortal::DeviceTypes deviceTypes)
{
    m_deviceTypes = deviceTypes;
}

bool RemoteDesktopSession::screenSharingEnabled() const
{
    return m_screenSharingEnabled;
}

void RemoteDesktopSession::setScreenSharingEnabled(bool enabled)
{
    m_screenSharingEnabled = enabled;
}

GlobalShortcutsSession::GlobalShortcutsSession(QObject *parent, const QString &appId, const QString &path)
    : Session(parent, appId, path)
    , m_token(path.mid(path.lastIndexOf('/') + 1))
    , m_globalAccelInterface(
          new KGlobalAccelInterface(QStringLiteral("org.kde.kglobalaccel"), QStringLiteral("/kglobalaccel"), QDBusConnection::sessionBus(), this))
    , m_component(new KGlobalAccelComponentInterface(m_globalAccelInterface->service(),
                                                     m_globalAccelInterface->getComponent(componentName()).value().path(),
                                                     m_globalAccelInterface->connection()))
{
    qDBusRegisterMetaType<KGlobalShortcutInfo>();
    qDBusRegisterMetaType<QList<KGlobalShortcutInfo>>();

    connect(m_globalAccelInterface,
            &KGlobalAccelInterface::yourShortcutsChanged,
            this,
            [this](const QStringList &actionId, const QList<QKeySequence> &newKeys) {
                Q_UNUSED(newKeys);
                if (actionId[KGlobalAccel::ComponentUnique] == componentName()) {
                    m_shortcuts[actionId[KGlobalAccel::ActionUnique]]->setShortcuts(newKeys);
                    Q_EMIT shortcurtsChanged();
                }
            });
    connect(m_component,
            &KGlobalAccelComponentInterface::globalShortcutPressed,
            this,
            [this](const QString &componentUnique, const QString &actionUnique, quint64 timestamp) {
                if (componentUnique != componentName())
                    return;

                Q_EMIT shortcutActivated(actionUnique, timestamp);
            });
    connect(m_component,
            &KGlobalAccelComponentInterface::globalShortcutReleased,
            this,
            [this](const QString &componentUnique, const QString &actionUnique, quint64 timestamp) {
                if (componentUnique != componentName())
                    return;

                Q_EMIT shortcutDeactivated(actionUnique, timestamp);
            });
}

GlobalShortcutsSession::~GlobalShortcutsSession() = default;

void GlobalShortcutsSession::restoreActions(const QVariant &shortcutsVariant)
{
    VariantMapMap shortcuts;
    auto arg = shortcutsVariant.value<QDBusArgument>();
    if (arg.currentSignature() != VariantMapMapSignature) {
        qCWarning(XdgSessionKdeSession) << "Wrong global shortcuts type, should be " << VariantMapMapSignature << "instead of " << arg.currentSignature();
        return;
    }
    arg >> shortcuts;

    const QList<KGlobalShortcutInfo> shortcutInfos = m_component->allShortcutInfos();
    QHash<QString, KGlobalShortcutInfo> shortcutInfosByName;
    shortcutInfosByName.reserve(shortcutInfos.size());
    for (const auto &shortcutInfo : shortcutInfos) {
        shortcutInfosByName[shortcutInfo.uniqueName()] = shortcutInfo;
    }

    for (auto it = shortcuts.begin(), itEnd = shortcuts.end(); it != itEnd; ++it) {
        const QString description = (*it)["description"].toString();
        if (description.isEmpty() || it.key().isEmpty()) {
            qCWarning(XdgSessionKdeSession) << "Shortcut without name or description" << it.key() << "for" << componentName();
            continue;
        }

        QAction *&action = m_shortcuts[it.key()];
        if (!action) {
            action = new QAction(this);
        }
        action->setProperty("componentName", componentName());
        action->setProperty("componentDisplayName", componentName());
        action->setObjectName(it.key());
        action->setText(description);
        action->setShortcuts(shortcutInfosByName[it.key()].keys());
        KGlobalAccel::self()->setGlobalShortcut(action, action->shortcuts());

        shortcutInfosByName.remove(it.key());
    }

    // We can forget the shortcuts that aren't around anymore
    while (!shortcutInfosByName.isEmpty()) {
        const QString shortcutName = shortcutInfosByName.begin().key();
        auto action = m_shortcuts.take(shortcutName);
        KGlobalAccel::self()->removeAllShortcuts(action);
        shortcutInfosByName.erase(shortcutInfosByName.begin());
    }

    Q_ASSERT(m_shortcuts.size() == shortcuts.size());
}

QVariant GlobalShortcutsSession::shortcutDescriptionsVariant() const
{
    QDBusArgument retVar;
    retVar << shortcutDescriptions();
    return QVariant::fromValue(retVar);
}

VariantMapMap GlobalShortcutsSession::shortcutDescriptions() const
{
    VariantMapMap ret;
    for (auto it = m_shortcuts.cbegin(), itEnd = m_shortcuts.cend(); it != itEnd; ++it) {
        QStringList triggers;
        triggers.reserve((*it)->shortcuts().size());
        const auto shortcuts = (*it)->shortcuts();
        for (auto shortcut : shortcuts) {
            triggers += shortcut.toString(QKeySequence::NativeText);
        }

        ret[it.key()] = QVariantMap{
            {QStringLiteral("description"), (*it)->text()},
            {QStringLiteral("triggers"), triggers.join(i18n(", "))},
        };
    }
    Q_ASSERT(ret.size() == m_shortcuts.size());
    return ret;
}
