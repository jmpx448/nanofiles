/*
    Copyright (C) 2018 Michał Szczepaniak

    This file is part of Musikilo.

    Morsender is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Morsender is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Morsender.  If not, see <http://www.gnu.org/licenses/>.
*/

import QtQuick 2.1
import Sailfish.Silica 1.0

Item {
    id: tabPageHeader

    property var view: null
    // [{func, icon, label, shortLabel}], see Main.qml
    property var actions: []
    property bool showLabels: false

    anchors { bottom: parent.bottom; left: parent.left; right: parent.right }
    height: Theme.itemSizeLarge

    Row {
        anchors.fill: parent

        Repeater {
            id: sectionRepeater
            model: actions
            delegate: BackgroundItem {
                width: tabPageHeader.width / sectionRepeater.count
                height: tabPageHeader.height

                Column {
                    anchors.centerIn: parent
                    width: parent.width
                    spacing: Theme.paddingSmall / 2

                    Image {
                        anchors.horizontalCenter: parent.horizontalCenter
                        height: Theme.iconSizeSmall * 1.4
                        width: Theme.iconSizeSmall * 1.4
                        source: modelData.icon
                    }

                    Label {
                        visible: showLabels
                        width: parent.width - Theme.paddingSmall
                        anchors.horizontalCenter: parent.horizontalCenter
                        horizontalAlignment: Text.AlignHCenter
                        truncationMode: TruncationMode.Fade
                        font.pixelSize: Theme.fontSizeTiny
                        color: highlighted ? Theme.highlightColor : Theme.primaryColor
                        text: modelData.shortLabel
                    }
                }

                onClicked: view.handleButton(modelData.func)
            }
        }
    }
}
