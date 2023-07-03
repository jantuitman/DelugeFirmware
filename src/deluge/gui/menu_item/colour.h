/*
 * Copyright © 2020-2023 Synthstrom Audible Limited
 *
 * This file is part of The Synthstrom Audible Deluge Firmware.
 *
 * The Synthstrom Audible Deluge Firmware is free software: you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program.
 * If not, see <https://www.gnu.org/licenses/>.
*/

#pragma once
#include "gui/menu_item/selection.h"

namespace menu_item {

class Colour final : public Selection {
public:
	Colour(char const* newName = NULL) : Selection(newName) {}
	void readCurrentValue();
	void writeCurrentValue();
	char const** getOptions();
	int getNumOptions();
	void getRGB(uint8_t rgb[3]);
	uint8_t value;
};

extern Colour activeColourMenu;
extern Colour stoppedColourMenu;
extern Colour mutedColourMenu;
extern Colour soloColourMenu;

} // namespace menu_item
