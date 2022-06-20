/*
 * SPDX-FileCopyrightText: 2016 Red Hat Inc
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 *
 * SPDX-FileCopyrightText: 2016 Jan Grulich <jgrulich@redhat.com>
 */

#include "desktopportal.h"
#include "desktopportal_debug.h"

#include "dynamiclauncher.h"

DesktopPortal::DesktopPortal(QObject *parent)
    : QObject(parent)
    , m_access(new AccessPortal(this))
    , m_account(new AccountPortal(this))
    , m_appChooser(new AppChooserPortal(this))
    , m_email(new EmailPortal(this))
    , m_fileChooser(new FileChooserPortal(this))
    , m_inhibit(new InhibitPortal(this))
    , m_notification(new NotificationPortal(this))
    , m_print(new PrintPortal(this))
    , m_settings(new SettingsPortal(this))
    , m_dynamicLauncher(new DynamicLauncherPortal(this))
{
    const QByteArray xdgCurrentDesktop = qgetenv("XDG_CURRENT_DESKTOP");
    if (xdgCurrentDesktop.compare("KDE", Qt::CaseInsensitive) == 0) {
        m_background = new BackgroundPortal(this);
        m_screenCast = new ScreenCastPortal(this);
        m_remoteDesktop = new RemoteDesktopPortal(this);
        m_screenshot = new ScreenshotPortal(this);
        WaylandIntegration::init();
    }
}

DesktopPortal::~DesktopPortal()
{
}
