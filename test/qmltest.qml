/*
 *   Copyright 2014 by Marco Martin <mart@kde.org>

 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.1
import QtQuick.Layouts 1.0
import org.kde.experimental.sprinter 0.1
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.extras 2.0 as PlasmaExtras

ColumnLayout {
    width: 300
    height: 500
    PlasmaComponents.TextField {
        id: searchField
        Layout.fillWidth: true
    }
    PlasmaExtras.ScrollArea {
        Layout.fillWidth: true
        Layout.fillHeight: true
        ListView {
            id: view

            model: QuerySession {
                query: searchField.text
            }
            delegate: PlasmaComponents.Label {
                text: model.Title
                width: view.width
            }
        }
    }
}
