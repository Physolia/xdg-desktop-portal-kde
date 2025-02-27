import QtQuick 2.15
import QtQuick.Window 2.15
import QtQml 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.14 as Layouts
import org.kde.kirigami 2.19 as Kirigami

MouseArea {
    // This needs to be a mousearea in orcer for the proper mouse events to be correctly filtered
    id: root
    focus: true
    acceptedButtons: Qt.LeftButton | Qt.RightButton
    hoverEnabled: true
    LayoutMirroring.enabled: Qt.application.layoutDirection === Qt.RightToLeft
    LayoutMirroring.childrenInherit: true
    anchors.fill: parent

    cursorShape: Qt.CrossCursor
    
    readonly property point mousePosition: Qt.point(mouseX, mouseY)
    onPressed: {
        if (mouse.button & Qt.RightButton) {
            SelectionEditor.dragReset();
        }

        if (mouse.button & Qt.LeftButton) {
            SelectionEditor.dragStart(Screen.name, mouse.x, mouse.y);
        }
    }
    onMousePositionChanged: {
        SelectionEditor.setMousePosition(Screen.name, mouseX, mouseY);
    }
    onReleased: {
        if (mouse.button & Qt.LeftButton) {
            SelectionEditor.dragRelease(Screen.name, mouse.x, mouse.y);
        }
    }
    

    component Overlay: Rectangle {
        color: "black"
        opacity: 0.5
        LayoutMirroring.enabled: false
    }
    
    // top and fullscreen if nothing is selected
    Overlay {
        id: topOverlay
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.bottom: selectionRectangle.visible ? selectionRectangle.top : parent.bottom
        visible: true
    }

    Overlay {
        id: leftOverlay
        anchors.left: parent.left
        anchors.top: selectionRectangle.top
        anchors.right: selectionRectangle.left
        anchors.bottom: selectionRectangle.bottom
        visible: selectionRectangle.visible
    }

    Overlay {
        id: rightOverlay
        anchors.left: selectionRectangle.right
        anchors.top: selectionRectangle.top
        anchors.right: parent.right
        anchors.bottom: selectionRectangle.bottom
        visible: selectionRectangle.visible
    }

    Overlay {
        id: bottomOverlay
        anchors.left: parent.left
        anchors.top: selectionRectangle.bottom
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        visible: selectionRectangle.visible
    }

    Rectangle {
        id: selectionRectangle
        color: "transparent"
        border.color: palette.highlight
        border.width: 1
        visible: SelectionEditor.rect.height > 0 && SelectionEditor.rect.width > 0
        x: SelectionEditor.rect.x - border.width - Screen.virtualX
        y: SelectionEditor.rect.y - border.width - Screen.virtualY
        width: SelectionEditor.rect.width + border.width * 2
        height: SelectionEditor.rect.height + border.width * 2
        LayoutMirroring.enabled: false
        LayoutMirroring.childrenInherit: true

        SystemPalette {
            id: palette
            colorGroup: Kirigami.Theme.Active
        }
    }

    FontMetrics {
        id: fontMetrics
    }

    // drag size
    FloatingTextBox {
        id: dragSizeBox

        anchors {
            horizontalCenter: selectionRectangle.horizontalCenter
            verticalCenter: selectionRectangle.verticalCenter
        }

        visible: selectionRectangle.visible && dragSizeBox.height < selectionRectangle.height && dragSizeBox.width < selectionRectangle.width
        opacity: 1
        contents: QQC2.Label {
            text: `${SelectionEditor.rect.width}x${SelectionEditor.rect.height}`
        }
        
        Behavior on opacity {
            NumberAnimation {
                duration: Kirigami.Units.longDuration
                easing.type: Easing.OutCubic
            }
        }
    }

    // instructions while dragging
    FloatingTextBox {
        id: dragBox
        anchors {
            horizontalCenter: parent.horizontalCenter
            bottom: parent.bottom
        }
        visible: SelectionEditor.isDragging && selectionRectangle.y + selectionRectangle.height < dragBox.y
        opacity: 1
        contents: Layouts.RowLayout {
            Layouts.ColumnLayout {
                QQC2.Label {
                    text: i18n("Start streaming:")
                    Layout.alignment: Qt.AlignRight
                }
                QQC2.Label {
                    text: i18n("Clear selection:")
                    Layout.alignment: Qt.AlignRight
                }
                QQC2.Label {
                    text: i18n("Cancel:")
                    Layout.alignment: Qt.AlignRight
                }
            }
            Layouts.ColumnLayout {
                QQC2.Label {
                    text: i18nc("Mouse action", "Release left-click")
                    Layout.alignment: Qt.AlignLeft
                }
                QQC2.Label {
                    text: i18nc("Mouse action", "Right-click")
                    Layout.alignment: Qt.AlignLeft
                }
                QQC2.Label {
                    text: i18nc("Keyboard action", "Escape")
                    Layout.alignment: Qt.AlignLeft
                }
            }
        }
        Behavior on opacity {
            NumberAnimation {
                duration: Kirigami.Units.longDuration
                easing.type: Easing.OutCubic
            }
        }
    }

    // instructions not dragging
    FloatingTextBox {
        anchors {
            horizontalCenter: parent.horizontalCenter
            verticalCenter: parent.verticalCenter
        }
        visible: !SelectionEditor.isDragging
        opacity: 1

        contents: Layouts.RowLayout {
            Layouts.ColumnLayout {
                QQC2.Label {
                    text: i18n("Create selection:")
                    Layout.alignment: Qt.AlignRight
                }
                QQC2.Label {
                    text: i18n("Cancel:")
                    Layout.alignment: Qt.AlignRight
                }
            }
            Layouts.ColumnLayout {
                QQC2.Label {
                    text: i18nc("Mouse action", "Left-click and drag")
                    Layout.alignment: Qt.AlignLeft
                }
                QQC2.Label {
                    text: i18nc("Keyboard action", "Escape")
                    Layout.alignment: Qt.AlignLeft
                }
            }
        }

        Behavior on opacity {
            NumberAnimation {
                duration: Kirigami.Units.longDuration
                easing.type: Easing.OutCubic
            }
        }
    }
}
