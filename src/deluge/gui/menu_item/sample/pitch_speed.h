/*
 * Copyright (c) 2014-2023 Synthstrom Audible Limited
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
#include "gui/ui/sound_editor.h"
#include "model/clip/clip.h"
#include "model/drum/drum.h"
#include "model/drum/kit.h"
#include "model/song/song.h"
#include "processing/sound/sound_drum.h"

namespace menu_item::sample {
class PitchSpeed final : public Selection {
public:
	PitchSpeed(char const* newName = NULL) : Selection(newName) {}

	bool usesAffectEntire() { return true; }

	void readCurrentValue() {
		soundEditor.currentValue = soundEditor.currentSampleControls->pitchAndSpeedAreIndependent;
	}

	void writeCurrentValue() {
		// If affect-entire button held, do whole kit
		if (currentUIMode == UI_MODE_HOLDING_AFFECT_ENTIRE_IN_SOUND_EDITOR && soundEditor.editingKit()) {

			Kit* kit = (Kit*)currentSong->currentClip->output;

			for (Drum* thisDrum = kit->firstDrum; thisDrum; thisDrum = thisDrum->next) {
				if (thisDrum->type == DrumType::SOUND) {
					SoundDrum* soundDrum = (SoundDrum*)thisDrum;
					Source* source = &soundDrum->sources[soundEditor.currentSourceIndex];

					source->sampleControls.pitchAndSpeedAreIndependent = soundEditor.currentValue;
				}
			}
		}

		// Or, the normal case of just one sound
		else {
			soundEditor.currentSampleControls->pitchAndSpeedAreIndependent = soundEditor.currentValue;
		}
	}

	char const** getOptions() {
		static char const* options[] = {"Linked", "Independent", NULL};
		return options;
	}
};
} // namespace menu_item::sample
