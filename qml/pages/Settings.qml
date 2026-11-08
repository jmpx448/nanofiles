import QtQuick 2.2
import Sailfish.Silica 1.0
import org.nemomobile.configuration 1.0

Page {
    allowedOrientations: Orientation.All

    ConfigurationValue {
        id: actionStyle
        key: "/apps/nanofiles/actionStyle"
        defaultValue: 0
    }

    ConfigurationValue {
        id: listView
        key: "/apps/nanofiles/listView"
        defaultValue: false
    }

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: column.height + Theme.paddingLarge

        Column {
            id: column
            width: parent.width

            PageHeader { title: qsTr("Settings") }

            ComboBox {
                label: qsTr("File actions")
                description: qsTr("How actions for selected files are shown")
                currentIndex: actionStyle.value
                menu: ContextMenu {
                    MenuItem { text: qsTr("Icons") }
                    MenuItem { text: qsTr("Icons with labels") }
                    MenuItem { text: qsTr("Pull-up menu") }
                }
                onCurrentIndexChanged: if (currentIndex !== actionStyle.value) actionStyle.value = currentIndex
            }

            Label {
                visible: actionStyle.value === 2
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                wrapMode: Text.Wrap
                font.pixelSize: Theme.fontSizeExtraSmall
                color: Theme.secondaryHighlightColor
                text: qsTr("Pull up at the end of the file list to open the menu. In long folders, scroll to the end first.")
            }

            TextSwitch {
                text: qsTr("List view")
                description: qsTr("Show files as a list instead of a grid")
                checked: listView.value
                onCheckedChanged: if (checked !== listView.value) listView.value = checked
            }
        }
    }
}
