/// \file wht.c
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>


#define KYEL  "\x1B[33m" ///< Yellow terminal text color
#define KBLU  "\x1B[34m" ///< Blue terminal text color
#define KRED  "\x1B[31m" ///< Red terminal text color
#define KMAG  "\x1B[35m" ///< Magenta terminal text color
#define RESET "\x1B[0m"  ///< Reset terminal text color to default

struct buff spectrum(struct buff buffer) {

	// put data into origImg.dat
	FILE *origImgFP;
	origImgFP = fopen("origImg.dat", "w");
	int row, col;
	for (row = 0; row < buffer.height; row++ ){

		for(col = 0; col < buffer.width; col++ ) {
			if (buffer.channels == 3){
				int avg = 0;
				avg += (int)buffer.img[3*((row*buffer.width) + col)];
				avg += (int)buffer.img[3*((row*buffer.width) + col) + 1];
				avg += (int)buffer.img[3*((row*buffer.width) + col) + 2];
				avg = avg / 3;

				fprintf(origImgFP, "%d\t", avg);
			}
			else {
				fprintf(origImgFP, "%d\t", (int)buffer.img[(row*buffer.width) + col]);
			}
		}
		fprintf(origImgFP, "\n");
	}
	fclose(origImgFP);


	// put data into transdormedImage.dat
	FILE *transformedImgFP;
	transformedImgFP = fopen("transformedImg.dat", "w");
	for (row = 0; row < buffer.height; row++ ){

		for(col = 0; col < buffer.width; col++ ) {
			fprintf(transformedImgFP, "%f\t", buffer.wht[(row*buffer.width) + col]);
		}
		fprintf(transformedImgFP, "\n");
	}
	fclose(transformedImgFP);


}