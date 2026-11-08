import QtQuick 2.2
import Sailfish.Silica 1.0

// Text version of the bottom action bar, opened by pulling up at the end
// of the file list. The first action (cancel/clear) goes last.
PushUpMenu {
    property var view: null
    property var actions: []

    Repeater {
        model: actions.length > 1 ? actions.slice(1).concat([actions[0]]) : actions
        MenuItem {
            text: modelData.label
            onClicked: view.handleButton(modelData.func)
        }
    }
}
