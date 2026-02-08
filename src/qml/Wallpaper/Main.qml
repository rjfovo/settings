/*
 * Copyright (C) 2021 CutefishOS Team.
 *
 * Author:     revenmartin <revenmartin@gmail.com>
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
import Cutefish.Settings 1.0
import FishUI 1.0 as FishUI

import "../"

ItemPage {
    headerTitle: qsTr("Background")

    Background {
        id: background
    }

    Scrollable {
        anchors.fill: parent
        contentHeight: layout.implicitHeight

        ColumnLayout {
            id: layout
            anchors.fill: parent
            spacing: FishUI.Units.largeSpacing

            RoundedItem {
                RowLayout {
                    spacing: FishUI.Units.largeSpacing * 2

                    Label {
                        text: qsTr("Background type")
                        leftPadding: FishUI.Units.smallSpacing
                    }

                    TabBar {
                        Layout.fillWidth: true

                        onCurrentIndexChanged: {
                            background.backgroundType = currentIndex
                        }

                        Component.onCompleted: {
                            currentIndex = background.backgroundType
                        }

                        TabButton {
                            text: qsTr("Picture")
                        }

                        TabButton {
                            text: qsTr("Color")
                        }
                    }
                }

    GridView {
        id: _view

        // 避免在属性绑定中使用_view.width，这可能导致布局循环
        property int rowCount: Math.max(1, Math.floor(width / itemWidth))

        Layout.fillWidth: true
        // 使用显式高度计算，避免在count为0时出现除零错误
        implicitHeight: {
            if (count === 0 || rowCount === 0) {
                return 0
            }
            return Math.ceil(count / rowCount) * cellHeight + FishUI.Units.largeSpacing
        }

        visible: background.backgroundType === 0

        clip: true
        model: background.backgrounds
        currentIndex: -1
        interactive: false

        cellHeight: itemHeight
        cellWidth: {
            if (rowCount === 0) return itemWidth
            var availableColumns = Math.floor(width / itemWidth)
            if (availableColumns <= 0) return itemWidth
            var allColumnSize = availableColumns * itemWidth
            var extraSpace = Math.max(width - allColumnSize, 0)
            var extraSpacing = extraSpace / availableColumns
            return itemWidth + Math.floor(extraSpacing)
        }

        property int itemWidth: 180
        property int itemHeight: 127

                    delegate: Item {
                        id: item

                        property bool isSelected: modelData === background.currentBackgroundPath

                        width: GridView.view.cellWidth
                        height: GridView.view.cellHeight
                        scale: 1.0

                        // 移除Behavior动画以减少内存使用
                        // Behavior on scale {
                        //     NumberAnimation {
                        //         duration: 200
                        //         easing.type: Easing.OutSine
                        //     }
                        // }

                        // Preload background
                        Rectangle {
                            anchors.fill: parent
                            anchors.margins: FishUI.Units.largeSpacing
                            radius: FishUI.Theme.bigRadius + FishUI.Units.smallSpacing / 2
                            color: FishUI.Theme.backgroundColor
                            visible: _image.status !== Image.Ready
                        }

                        // Preload image
                        Image {
                            anchors.centerIn: parent
                            width: 32
                            height: width
                            sourceSize: Qt.size(width, height)
                            source: FishUI.Theme.darkMode ? "qrc:/images/dark/picture.svg"
                                                          : "qrc:/images/light/picture.svg"
                            visible: _image.status !== Image.Ready
                        }

                        Rectangle {
                            anchors.fill: parent
                            anchors.margins: FishUI.Units.smallSpacing
                            color: "transparent"
                            radius: FishUI.Theme.bigRadius + FishUI.Units.smallSpacing / 2

                            border.color: FishUI.Theme.highlightColor
                            border.width: _image.status == Image.Ready && isSelected ? 3 : 0

                            Image {
                                id: _image
                                anchors.fill: parent
                                anchors.margins: FishUI.Units.smallSpacing
                                source: "file://" + modelData
                                sourceSize: Qt.size(width, height)
                                fillMode: Image.PreserveAspectCrop
                                asynchronous: true
                                mipmap: false  // 禁用mipmap以减少内存
                                cache: true
                                smooth: false  // 禁用平滑以减少GPU负载
                                opacity: 1.0

                                // 移除Behavior动画
                                // Behavior on opacity {
                                //     NumberAnimation {
                                //         duration: 100
                                //         easing.type: Easing.InOutCubic
                                //     }
                                // }

                                // 暂时禁用OpacityMask以测试崩溃问题
                                // layer.enabled: true
                                // layer.effect: OpacityMask {
                                //     maskSource: Item {
                                //         width: _image.width
                                //         height: _image.height
                                //
                                //         Rectangle {
                                //             anchors.fill: parent
                                //             radius: FishUI.Theme.bigRadius
                                //         }
                                //     }
                                // }
                            }

                            MouseArea {
                                anchors.fill: parent
                                acceptedButtons: Qt.LeftButton
                                hoverEnabled: false  // 禁用hover以减少事件处理

                                onClicked: {
                                    background.setBackground(modelData)
                                }

                                // 移除hover效果以减少动画
                                // onEntered: function() {
                                //     _image.opacity = 0.7
                                // }
                                // onExited: function() {
                                //     _image.opacity = 1.0
                                // }

                                onPressedChanged: {
                                    // 简化按压效果
                                    item.scale = pressed ? 0.97 : 1.0
                                }
                            }
                        }
                    }

                    // calcExtraSpacing 函数已不再需要，因为 cellWidth 现在直接计算
                }

                Item {
                    visible: background.backgroundType === 1
                    height: FishUI.Units.smallSpacing
                }

                Loader {
                    Layout.fillWidth: true
                    height: item ? item.height : 0
                    visible: background.backgroundType === 1
                    sourceComponent: colorView
                }
            }

//            DesktopPreview {
//                Layout.alignment: Qt.AlignHCenter
//                width: 500
//                height: 300
//            }

            Item {
                height: FishUI.Units.largeSpacing
            }
        }
    }

    Component {
        id: colorView

        GridView {
            id: _colorView
            Layout.fillWidth: true

            // 避免在属性绑定中使用_width，这可能导致布局循环
            property int rowCount: Math.max(1, Math.floor(width / cellWidth))

            // 使用显式高度计算，避免在count为0时出现除零错误
            implicitHeight: {
                if (count === 0 || rowCount === 0) {
                    return 0
                }
                return Math.ceil(count / rowCount) * cellHeight + FishUI.Units.largeSpacing
            }

            cellWidth: 50
            cellHeight: 50

            interactive: false
            model: ListModel {}

            property var itemSize: 32

            Component.onCompleted: {
                model.append({"bgColor": "#2B8ADA"})
                model.append({"bgColor": "#4DA4ED"})
                model.append({"bgColor": "#B7E786"})
                model.append({"bgColor": "#F2BB73"})
                model.append({"bgColor": "#EE72EB"})
                model.append({"bgColor": "#F0905A"})
                model.append({"bgColor": "#595959"})
                model.append({"bgColor": "#000000"})
            }

            delegate: Rectangle {
                property bool checked: background.backgroundColor === bgColor
                property color currentColor: bgColor

                width: _colorView.itemSize + FishUI.Units.largeSpacing
                height: width
                color: "transparent"
                radius: width / 2
                border.color: _mouseArea.pressed ? Qt.rgba(currentColor.r,
                                                           currentColor.g,
                                                           currentColor.b, 0.6)
                                                 : Qt.rgba(currentColor.r,
                                                           currentColor.g,
                                                           currentColor.b, 0.4)
                border.width: checked ? 3 : _mouseArea.containsMouse ? 2 : 0

                MouseArea {
                    id: _mouseArea
                    anchors.fill: parent
                    hoverEnabled: true
                    onClicked: background.backgroundColor = bgColor
                }

                Rectangle {
                    width: 32
                    height: width
                    anchors.centerIn: parent
                    color: currentColor
                    radius: width / 2
                }
            }
        }
    }
}
