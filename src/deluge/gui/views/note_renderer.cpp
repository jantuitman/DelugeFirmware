#include "gui/views/note_renderer.h"
#include "gui/views/timeline_view.h"
#include "model/note/note_row.h"
#include "model/note/note.h"
#include "util/functions.h"
#include <string.h>

NoteRenderer noteRenderer;

NoteRenderer::NoteRenderer() {


}

/*
void NoteRenderer::recalculateColours() {

}

void NoteRenderer::recalculateColour(uint8_t yDisplay) {

}
*/


// this function was previously in noterow but now has a noterow as parameter.
// the color parameters have been removed.
void NoteRenderer::renderNoteRow(NoteRow* noteRow, TimelineView* editorScreen, uint8_t* image, uint8_t occupancyMask[], bool overwriteExisting,
                    uint32_t effectiveRowLength, bool allowNoteTails, int renderWidth, int32_t xScroll,
                    uint32_t xZoom, int xStartNow, int xEnd, bool drawRepeats, int clipColourOffset) {

	if (overwriteExisting) {
		memset(image, 0, renderWidth * 3);
		if (occupancyMask) memset(occupancyMask, 0, renderWidth);
	}

	if (noteRow->hasNoNotes()) return;

	int32_t squareEndPos[MAX_IMAGE_STORE_WIDTH];
	int32_t searchTerms[MAX_IMAGE_STORE_WIDTH];

	int whichRepeat = 0;

	int xEndNow;

	do {
		// Start by presuming we'll do all the squares now... though we may decide in a second to do a smaller number now, and come back for more batches
		xEndNow = xEnd;

		// For each square we might do now...
		for (int square = xStartNow; square < xEndNow; square++) {

			// Work out its endPos...
			int32_t thisSquareEndPos =
			    editorScreen->getPosFromSquare(square + 1, xScroll, xZoom) - effectiveRowLength * whichRepeat;

			// If we're drawing repeats and the square ends beyond end of Clip...
			if (drawRepeats && thisSquareEndPos > effectiveRowLength) {

				// If this is the first square we're doing now, we can take a shortcut to skip forward some repeats
				if (square == xStartNow) {
					int numExtraRepeats = (uint32_t)(thisSquareEndPos - 1) / effectiveRowLength;
					whichRepeat += numExtraRepeats;
					thisSquareEndPos -= numExtraRepeats * effectiveRowLength;
				}

				// Otherwise, we'll come back in a moment to do the rest of the repeats - just record that we want to stop rendering before this square until then
				else {
					xEndNow = square;
					break;
				}
			}

			squareEndPos[square - xStartNow] = thisSquareEndPos;
		}

		memcpy(searchTerms, squareEndPos, sizeof(squareEndPos));

		noteRow->notes.searchMultiple(searchTerms, xEndNow - xStartNow);

		int squareStartPos =
		    editorScreen->getPosFromSquare(xStartNow, xScroll, xZoom) - effectiveRowLength * whichRepeat;

		uint8_t rowDefaultColour[3];
		uint8_t rowDefaultBlurColour[3];
		uint8_t rowDefaultTailColour[3];
		uint8_t noteColour[3];
		uint8_t noteBlurColour[3];
		uint8_t noteTailColour[3];
        getNoteColourFromY(noteRow->y, clipColourOffset, rowDefaultColour);
        getBlurColour(rowDefaultBlurColour, rowDefaultColour);
        getTailColour(rowDefaultTailColour, rowDefaultColour);

		for (int xDisplay = xStartNow; xDisplay < xEndNow; xDisplay++) {
			if (xDisplay != xStartNow) squareStartPos = squareEndPos[xDisplay - xStartNow - 1];
			int i = searchTerms[xDisplay - xStartNow];

			Note* note = noteRow->notes.getElement(i - 1); // Subtracting 1 to do "LESS"

			uint8_t* pixel = image + xDisplay * 3;

			if (note) {
				getNoteSpecificColours(noteRow->y, clipColourOffset,  note, rowDefaultColour, rowDefaultBlurColour, rowDefaultTailColour, noteColour, noteBlurColour, noteTailColour);
			}

			// If Note starts somewhere within square, draw the blur colour
			if (note && note->pos > squareStartPos) {
				pixel[0] = noteBlurColour[0];
				pixel[1] = noteBlurColour[1];
				pixel[2] = noteBlurColour[2];
				if (occupancyMask) occupancyMask[xDisplay] = 64;
			}

			// Or if Note starts exactly on square...
			else if (note && note->pos == squareStartPos) {
				pixel[0] = noteColour[0];
				pixel[1] = noteColour[1];
				pixel[2] = noteColour[2];
				if (occupancyMask) occupancyMask[xDisplay] = 64;
			}

			// Draw wrapped notes
			else if (!drawRepeats || whichRepeat) {
				bool wrapping = (i == 0); // Subtracting 1 to do "LESS"
				if (wrapping) note = noteRow->notes.getLast();
				int noteEnd = note->pos + note->length;
				if (wrapping) noteEnd -= effectiveRowLength;
				if (noteEnd > squareStartPos && allowNoteTails) {
					pixel[0] = noteTailColour[0];
					pixel[1] = noteTailColour[1];
					pixel[2] = noteTailColour[2];
					if (occupancyMask) occupancyMask[xDisplay] = 64;
				}
			}
		}

		xStartNow = xEndNow;
		whichRepeat++;
		// TODO: if we're gonna repeat, we probably don't need to do the multi search again...
	} while (
	    xStartNow
	    != xEnd); // This will only do another repeat if we'd modified xEndNow, which can only happen if drawRepeats

}


/**
* gets the note color for pitch y.
* 
* applies the colorschemes.
* 
* Classic: applies sine waves to the r,g,b, components for nice gradients.
* 
* OctavePiano: renders 'black key' notes different from 'white key' notes,
* and gives each octave a different color. within octaves colors are constant.
* red and green are not used to full extend so that they can be used to display
* accidental transposes.
* 
* Blue: renders a blue gradient. This is for super visible accidentals in red and green.
* 
*
*
*
*
*/
void NoteRenderer::getNoteColourFromY(int yNote,int clipColourOffset, uint8_t rgb[]) {
	int colorScheme = runtimeFeatureSettings.get(RuntimeFeatureSettingType::ColorScheme);
	if (colorScheme == RuntimeFeatureStateColorScheme::Classic) {
		// TODO: replace with the old function since the new color schemes are resolved
		// in this function
		hueToRGBWithColorScheme((yNote + clipColourOffset) * -8 / 3, rgb,colorScheme);
	}
	else if (colorScheme == RuntimeFeatureStateColorScheme::OctavePiano) {
    	int octaveOffset = ((yNote/12) + clipColourOffset) % 12;
    	int offsetWithinOctave = yNote % 12;
		int octaveRGB[][3] = {
	  		{ 16,  0, 32 },
	  		{  0, 16, 32 },
	  		{ 16, 16, 0  },
	  		{  0,  0, 32 },
	  		{ 32,  0, 48 },
	  		{  0, 32, 48 },
            { 32, 32, 0  },
            {  0,  0, 48 },
            { 48,  0, 64 },
            {  0, 48, 64 },
            { 48, 48, 0  },
            {  0,  0, 64 }
	  	};	      

	 	int blackKeys[] = {0,1,0,1,0,0,1,0,1,0,1,0};

		rgb[0] = octaveRGB[octaveOffset][0] >> blackKeys[offsetWithinOctave];
		rgb[1] = octaveRGB[octaveOffset][1] >> blackKeys[offsetWithinOctave];
		rgb[2] = octaveRGB[octaveOffset][2] >> blackKeys[offsetWithinOctave];      
	}
	else {
		// colorSchem == RuntimeFeatureStateColorScheme::BluePiano
		// like a piano we use dark keys and light keys.
		// all tints are blue.
		// octaves are a gradient	
		int octaveOffset = (yNote/12) + clipColourOffset;
		int offsetWithinOctave = yNote % 12;
		int blackKeys[] = {0,1,0,1,0,0,1,0,1,0,1,0};
		rgb[0] = blackKeys[offsetWithinOctave] * 8;
		rgb[1] = blackKeys[offsetWithinOctave] * 8;
		rgb[2] = (16 + ((octaveOffset * 3 )% 16) + offsetWithinOctave * 2 ) >> blackKeys[offsetWithinOctave];      
	}

}

void NoteRenderer::getNoteSpecificColours(int y,
	                                     int clipColourOffset,
	                                     Note* note, 
	                                     uint8_t rowDefaultColour[],
	                                     uint8_t rowDefaultBlurColour[],
	                                     uint8_t rowDefaultTailColour[],
	                                     uint8_t  noteColour[],
                                         uint8_t  noteBlurColour[],
                                         uint8_t  noteTailColour[]) {
	// copy either the defaults or tranpose
	int transpose = note->getAccidentalTranspose();
	int colorScheme = runtimeFeatureSettings.get(RuntimeFeatureSettingType::ColorScheme);
	if ( transpose == 0) {
		for(int i=0; i<3 ; i++) {
			noteColour[i] = rowDefaultColour[i]; 
			noteBlurColour[i] = rowDefaultBlurColour[i];
            noteTailColour[i] = rowDefaultTailColour[i];
		}
	}
	else {
		getNoteColourFromY(y + transpose, clipColourOffset, noteColour);
	    if (colorScheme != RuntimeFeatureStateColorScheme::Classic) {
	    	if (transpose > 0) {
	    		noteColour[0] = 0;
	    		noteColour[1] = 64; // set green color for raised notes.
	    		noteColour[2] = 0;
	    	}
	    	else { // set red color for lowered notes.
	    		noteColour[0] = 64;
	    		noteColour[1] = 0;
	    		noteColour[2] = 0;
	    	}
	    }

	    getBlurColour(noteBlurColour, noteColour);
	    getTailColour(noteTailColour, noteColour);  	
	}
}
