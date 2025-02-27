/*  This file is part of the KDE project
    SPDX-FileCopyrightText: 2021 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as QQC2
import org.kde.kirigami 2.14 as Kirigami
import org.kde.plasma.workspace.dialogs 1.0 as PWD
import org.kde.taskmanager 0.1 as TaskManager

PWD.SystemDialog
{
    id: root
    property alias outputsModel: outputsView.model
    property alias windowsModel: windowsView.model
    property bool multiple: false
    property alias allowRestore: allowRestoreItem.checked
    iconName: "video-display"
    acceptable: (outputsModel && outputsModel.hasSelection) || (windowsModel && windowsModel.hasSelection)

    signal clearSelection()

    ColumnLayout {
        spacing: 0

        QQC2.TabBar {
            id: tabView
            Layout.fillWidth: true
            visible: root.outputsModel && root.windowsModel
            currentIndex: outputsView.count > 0 ? 0 : 1

            QQC2.TabButton {
                text: i18n("Screens")
            }
            QQC2.TabButton {
                text: i18n("Windows")
            }
        }

        QQC2.Frame {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.preferredHeight: Kirigami.Units.gridUnit * 20
            Layout.preferredWidth: Kirigami.Units.gridUnit * 30
            Kirigami.Theme.inherit: false
            Kirigami.Theme.colorSet: Kirigami.Theme.View
            background: Rectangle {
                color: Kirigami.Theme.backgroundColor
                property color borderColor: Kirigami.Theme.textColor
                border.color: Qt.rgba(borderColor.r, borderColor.g, borderColor.b, 0.3)
            }

            StackLayout {
                anchors.fill: parent
                currentIndex: tabView.currentIndex

                QQC2.ScrollView {
                    Kirigami.CardsLayout {
                        Repeater {
                            id: outputsView
                            model: null
                            PipeWireDelegate {
                                banner {
                                    title: model.display
                                    titleIcon: model.decoration
                                    titleLevel: 3
                                }
                                checkable: true
                                checked: model.checked === Qt.Checked
                                nodeId: waylandItem.nodeId
                                TaskManager.ScreencastingRequest {
                                    id: waylandItem
                                    outputName: model.name
                                }

                                onToggled: {
                                    var to = model.checked !== Qt.Checked ? Qt.Checked : Qt.Unchecked;
                                    if (!root.multiple && to === Qt.Checked) {
                                        root.clearSelection()
                                    }
                                    outputsView.model.setData(outputsView.model.index(model.row, 0), to, Qt.CheckStateRole)
                                }
                            }
                        }
                    }
                }
                QQC2.ScrollView {
                    Kirigami.CardsLayout {
                        Repeater {
                            id: windowsView
                            model: null
                            PipeWireDelegate {
                                banner {
                                    title: model.DisplayRole || ""
                                    titleIcon: model.DecorationRole || ""
                                    titleLevel: 3
                                }
                                checkable: true
                                checked: model.checked === Qt.Checked
                                nodeId: waylandItem.nodeId
                                TaskManager.ScreencastingRequest {
                                    id: waylandItem
                                    uuid: model.Uuid
                                }

                                onToggled: {
                                    var to = model.checked !== Qt.Checked ? Qt.Checked : Qt.Unchecked;
                                    if (!root.multiple && to === Qt.Checked) {
                                        root.clearSelection()
                                    }
                                    windowsView.model.setData(windowsView.model.index(model.row, 0), to, Qt.CheckStateRole)
                                }
                            }
                        }
                    }
                }
            }
        }

        QQC2.CheckBox {
            id: allowRestoreItem
            checked: true
            text: i18n("Allow restoring on future sessions")
        }
    }

    standardButtons: QQC2.DialogButtonBox.Ok | QQC2.DialogButtonBox.Cancel
    Component.onCompleted: {
        dialogButtonBox.standardButton(QQC2.DialogButtonBox.Ok).text = i18n("Share")

        // If there's only one thing in the list, pre-select it to save the user a click
        if (outputsView.count == 1 && windowsView.count == 0) {
            outputsView.model.setData(outputsView.model.index(0, 0), Qt.Checked, Qt.CheckStateRole);
        } else if (windowsView.count == 1 && outputsView.count == 0) {
            windowsView.model.setData(outputsView.model.index(0, 0), Qt.Checked, Qt.CheckStateRole);
        }
    }
}
