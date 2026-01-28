/*
 * Copyright (C) 2021 CutefishOS Team.
 *
 * Author:     rekols <aj@cutefishos.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 6.0
import QtQuick.Controls 6.0
import QtQuick.Layouts 6.0
import Qt5Compat.GraphicalEffects 6.0

import FishUI 1.0 as FishUI
import cutefish.networkmanagement 1.0

import "../"

ItemPage {
    id: control
    headerTitle: qsTr("Ethernet")

    property var itemHeight: 45
    property var settingsMap: ({})

    Handler {
        id: handler
    }

    WifiSettings {
        id: wifiSettings
    }

    NetworkModel {
        id: networkModel
    }

    EnabledConnections {
        id: enabledConnections
    }

    IdentityModel {
        id: connectionModel
    }

    Configuration {
        id: configuration
    }

    Scrollable {
        anchors.fill: parent
        contentHeight: mainLayout.implicitHeight

        ColumnLayout {
            id: mainLayout
            anchors.fill: parent
            anchors.bottomMargin: FishUI.Units.largeSpacing
            spacing: FishUI.Units.largeSpacing * 2

            // Wired connection
            RoundedItem {
                visible: enabledConnections.wwanHwEnabled
                spacing: FishUI.Units.largeSpacing

                RowLayout {
                    spacing: FishUI.Units.largeSpacing

                    Label {
                        text: qsTr("Ethernet")
                        color: FishUI.Theme.disabledTextColor
                        Layout.fillWidth: true
                    }

                    Switch {
                        Layout.fillHeight: true
                        rightPadding: 0
                        checked: enabledConnections.wwanEnabled
                        onCheckedChanged: {
                            if (checked) {
                                if (!enabledConnections.wwanEnabled) {
                                    handler.enableWwan(checked)
                                }
                            } else {
                                if (enabledConnections.wwanEnabled) {
                                    handler.enableWwan(checked)
                                }
                            }
                        }
                    }
                }

                ListView {
                    id: wiredView

                    visible: enabledConnections.wwanEnabled && wiredView.count > 0

                    Layout.fillWidth: true
                    Layout.preferredHeight: wiredView.count * control.itemHeight
                    interactive: false
                    clip: true

                    model: AppletProxyModel {
                        type: AppletProxyModel.WiredType
                        sourceModel: connectionModel
                    }

                    ScrollBar.vertical: ScrollBar {}

                    delegate: WiredItem {
                        height: control.itemHeight
                        width: wiredView.width
                    }
                }
            }

            Item {
                height: FishUI.Units.largeSpacing
            }
        }
    }
}
