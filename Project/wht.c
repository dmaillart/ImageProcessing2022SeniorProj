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

signed char* hadamard(int n);
signed char* hadamard_recursive(int i, int n, signed char* temp);
void print_matrix(signed char* n, int length);
void print_matrixf(double* n, int length);
void print_matrixu(unsigned char* n, int length);


/** 
 * Generate natural-ordering Hadamard matrix of size (2^n)x(2^n). 
 * \param n log2 of the length or width of the input image (with zero padding), and the number of iterations required to generate the Hadamard matrix.
 *
 * \return `char*` Hadamard matrix data
 */
signed char* hadamard(int n) {
	//dim = width of the final matrix, and of the input image. Add dim to ptr to get next row
	int dim = 1 << n;		
	size_t size = dim * dim;
	//printf("size = %d\n", size);
	//pointer to output matrix, an array of chars which is the same length as the input buffer.img. Signed because the matrix has 1 and -1 only
	signed char* ptr = malloc(size);	
	if (ptr == NULL) {
		printf(KRED"Error: "RESET"Unable to allocate memory for the matrix.\n");
		exit(1);
	}

	// Create temporary matrix for Hadamard(1)
	signed char* temp = malloc(4);
	*(temp) = 1;
	*(temp + 1) = 1;
	*(temp + 2) = 1;
	*(temp + 3) = -1;

	// call recursive function to generate the matrix
	int i = 1;	// i is log2 of the length of each row in temp
	ptr = hadamard_recursive(i, n, temp);
	return ptr;
	
}

/**
 * Helper function for `hadamard()`. Recursively generates the natural-ordered Hadamard matrix of size (2^n)x(2^n)
 * \param i i is log2 of the length of each row in temp
 * \param n log2 of the length or width of the input image (with zero padding), and the number of iterations required to generate the Hadamard matrix.
 * \param temp temporary matrix for Hadamard(1)
 *
 * \return `char*` Hadamard matrix data
 */
signed char* hadamard_recursive(int i, int n, signed char* temp) {
	// iterate i by 1 each recursion until i = n
	
	// End case, i == n, so temp is already the finished matrix.
	if (i == n) {
		return temp;
	}
	// dim is the length of temp and 1/2 the length of the next iteration's temp
	// dim = 2^i
	int dim = 1 << i;
	//size = size of temp in memory = 1/4 of next iteration
	int size = dim * dim;
	//printf("%d\n", dim);
	//temp2 is the next iteration's temp matrix, of double the dimensions of temp
	//printf("Calling malloc(%d):\n", 4 * size);
	signed char* temp2 = malloc(4 * size);

	//offsets define the current position in the temp and temp2 matrices.
	int offset = 0;
	int offset2 = 0;

	// Top half of the output is the input repeated twice
	for (int row = 0; row < dim; row++) {
		for (int col = 0; col < dim; col++) {
			//printf("%d   ", col);
			*(temp2 + offset2 + col) = *(temp + offset + col);
			*(temp2 + offset2 + col + dim) = *(temp + offset + col);
		}
		offset += dim;
		offset2 += dim * 2;
		//printf("\n");
	}
	offset = 0;
	// Bottom half is the input repeated once positive and once negative
	for (int row = 0; row < dim; row++) {
		for (int col = 0; col < dim; col++) {
			//printf("%d   ", col);
			*(temp2 + offset2 + col) = *(temp + offset + col);
			*(temp2 + offset2 + col + dim) = -1 * (*(temp + offset + col));
		}
		offset += dim;
		offset2 += dim * 2;
		//printf("\n");
	}

	// Clear the temp matrix from memory, it is no longer useful
	//printf("%p\n", (void*)temp);
	free(temp);

	// iterate i and go into recursion.
	i++;
	return hadamard_recursive(i, n, temp2);
}

/**
 * Performs Walsh-Hadamard transform onto buffer `buff`.
 * \param buffer Source image buffer
 * \param buffname Name of resultant buffer
 *
 * \return transformed `buff`
 */
struct buff wht(struct buff buffer, char* buffname) {
	struct buff result;
	int w = buffer.width;
	int h = buffer.height;
	int c = buffer.channels;

	struct buff img = buffer;

	char* ext;
	ext = strstr(img.imageName, ".png");
	if (ext == NULL)
		ext = strstr(img.imageName, ".jpg");
	if (ext == NULL)
		ext = strstr(img.imageName, ".tiff");
	if (ext == NULL)
		ext = strstr(img.imageName, ".gif");	// Use extension of a for extension of result

	strcpy(result.imageName, "wht");
	strcat(result.imageName, ext);
	strcpy(result.name, buffname);
	strcpy(result.status, "true");
	result.has_wht = 1;
	result.isLibgd = 0;

	// Get the size of the square image: dim = 2^n
	int n = (int)ceil(log2(fmax(w, h)));
	int dim = 1 << n;

	result.height = dim;
	result.width = dim;
	//printf("dim = %d\n", dim);
	result.channels = c;

	int size = c * dim * dim; // number of pixels * number of channels
	result.img = malloc(size);
	result.wht = malloc(size * sizeof(double)); // wht image is in doubles, which are 8 bytes.
	result.whtimg = malloc(size);

	// Copy img into result with 0s for padding
	for (int k = 0; k < c; k++) {
		for (int i = 0; i < dim; i++) {
			for (int j = 0; j < dim*c; j+=c) {
				if (i >= h || j >= w * c) {
					*(result.img + j + (i * dim * c) + k) = 0;
				}
				else {
					*(result.img + j + (i * dim * c) + k) = *(img.img + j + (i * w * c) + k);
				}
			}
		}
	}
	//printf("zero padding complete\n");
	// Generate Hadamard matrix
	signed char* H = hadamard(n);
	//printf("Hadamard matrix complete\n");
	// 2D Hadamard Transform
	// For Hadamard matrix H and image i, the transform is (1/2^2n)HiH

	// Transform the columns as column vectors multiplied by H: (1/2^n)iH
	//printf("First transform: columns\n");
	for (int k = 0; k < c; k++) {
		for (int d = 0; d < dim; d++) {
			//printf("column1\n");
			for (int a = 0; a < dim; a++) {
				*(result.wht + (a * c) + k + (d * dim * c)) = 0;
				//printf("pixel: %d\n", (a * c) + k + (d * dim * c));
				for (int b = 0; b < dim; b++) {
					*(result.wht + (a * c) + k + (d * dim * c)) = *(result.wht + (a * c) + k + (d * dim * c)) + ((double)(*(result.img + (a * c) + k + (b * dim * c))) * (double)(*(H + b + (d * dim)))) / (double)dim;
					
					//printf("%f * %f = %f\n", (double)(*(result.img + (a * c) + k + (b * dim * c))), (double)(*(H + b + (d * dim))), (double)(*(result.img + (a * c) + k + (b * dim * c))) * (double)(*(H + b + (d * dim))));
					//printf("%f\t/\t%f\t=\t%f\n", ((double)(*(result.img + (a * c) + k + (b * dim * c))) * (double)(*(H + b + (d * dim)))), (double)dim, ((double)(*(result.img + (a * c) + k + (b * dim * c))) * (double)(*(H + b + (d * dim)))) / (double)dim);
				}
				//printf("%f\t", *(result.wht + (a * c) + k + (d * dim * c)));
				//printf("\n");
			}
			//printf("\n");
		}
	}
	//printf("columns transformed\n");

	double* temp;
	temp = malloc(size*sizeof(double));

	// Transform result.wht rows with (1/2^n)Hi
	// swapped a and b in formulas to switch rows and columns
	//printf("Second transform: rows\n");
	for (int k = 0; k < c; k++) {
		for (int d = 0; d < dim; d++) {
			for (int a = 0; a < dim; a++) {
				*(temp + (a * c) + k + (d * dim * c)) = 0;
				//printf("pixel: %d\n", (a * c) + k + (d * dim * c));
				for (int b = 0; b < dim; b++) {
					*(temp + (a * c) + k + (d * dim * c)) = *(temp + (a * c) + k + (d * dim * c)) + ((*(result.wht + (b * c) + k + (d * dim * c))) * (double)(*(H + a + (b * dim)))) / (double)dim;
					//printf("%f * %d = %f\n", *(result.wht + (b * c) + k + (d * dim * c)), *(H + a + (b * dim)), ((*(result.wht + (b * c) + k + (d * dim * c))) * (*(H + a + (b * dim)))));
				}
				//printf("%f\t", *(temp + (a * c) + k + (d * dim * c)));
				//printf("\n");
			}
			//printf("\n");
		}
	}
	//printf("rows transformed\n");
	
	/*print_matrix(H, dim);
	printf("\n");
	print_matrixu(result.img, dim);
	printf("\n");
	print_matrixf(result.wht, dim);
	printf("\n"); 
	print_matrixf(temp, dim);
	printf("\n"); */
	// Free the old wht that we just transformed, and assign it to the temp where we stored the finished transform
	free(result.wht);
	result.wht = temp;

	//// Scale wht values to fit in 0-255 for a displayable image
	//// Skip pixel 0 because it usually has a much higher value that throws off the average
	//// above comment is redacted because whtimg code is obsolete and being removed
	//double top = result.wht[0];
	//double bot = result.wht[0];
	//for (int i = 2; i < size; i++) {
	//	//printf("%f\n", *(result.wht + i));
	//	if (*(result.wht + i) > top) {
	//		top = *(result.wht + i);
	//	}
	//	else if (*(result.wht + i) < bot) {
	//		bot = *(result.wht + i);
	//	}
	//}
	////printf("top: %f\n", top);
	////printf("bot: %f\n", bot);
	//double diff = top - bot;
	//double ratio = 255 / diff;
	////printf("diff: %f\n", diff);
	//
	//if (result.wht[0] > top) {
	//	result.whtimg[0] = 255;
	//	for (int i = 1; i < size; i++) {
	//		*(result.whtimg + i) = (uint8_t)((*(result.wht + i) - bot) * ratio);
	//		//printf("%d\t", *(result.whtimg + i));
	//		//if ((i + 1) % 4 == 0) {
	//		//	printf("\n");
	//		//}
	//	}
	//}
	//else {
	//	for (int i = 0; i < size; i++) {
	//		*(result.whtimg + i) = (uint8_t)((*(result.wht + i) - bot) * ratio);
	//		//printf("%d\t", *(result.whtimg + i));
	//		//if ((i + 1) % 4 == 0) {
	//		//	printf("\n");
	//		//}
	//	}
	//}
	//for (int i = 1; i < size; i++) {
	//	*(result.whtimg + i) = (uint8_t)((*(result.wht + i) - bot) * ratio);
	//	//printf("%d\t", *(result.whtimg + i));
	//	//if ((i + 1) % 4 == 0) {
	//	//	printf("\n");
	//	//}
	//}
	////printf("%p\n", result.whtimg);
	////for (int k = 0; k < c; k++) {
	////	for (int i = 0; i < dim; i++) {
	////		for (int j = 0; j < dim * c; j += c) {
	////			*(result.whtimg + j + (i * dim * c) + k) = (uint8_t)((*(result.wht + k + ) - bot) * ratio);
	////		}
	////	}
	////}
	////printf("returning wht buffer\n");
	//printf("whtimg complete\n");
	return result;
}


/** 
 * Inverse Hadamard transform, takes an wft image as input and recovers the original image from it.
 * \param img Source wht buffer
 * 
 * \return `buff` original image
 */
struct buff iwht(struct buff img) {
	// Inverse Hadamard transform, takes an wft image as input and recovers the original image from it.
	// To invert, run the normal transform in reverse, i.e. transform rows then columns. (H*A)*H instead of H*(A*H)
	struct buff result;
	int w = img.width;
	int h = img.height;
	int c = img.channels;
	//printf("iwht\n");
	int n = (int)ceil(log2(fmax(w, h)));
	int dim = 1 << n;
	int size = dim * dim;
	if (w != h || dim != w) {
		printf(KRED "ERROR: " RESET "WHT data dimensions are not square and a power of 2.");
		return img;
	}

	signed char* H = hadamard(n); // Get Hadamard Matrix

	double* temp;
	double* temp2;
	temp = malloc(size * sizeof(double) * c);
	temp2 = malloc(size * sizeof(double) * c);

	//print_matrixf(img.wht, 4);
	//printf("\n");

    // Transform result.wht rows with (1/2^n)Hi
	// swapped a and b in formulas to switch rows and columns
	//printf("Second transform: rows\n");
	for (int k = 0; k < c; k++) {
		for (int d = 0; d < dim; d++) {
			for (int a = 0; a < dim; a++) {
				*(temp + (a * c) + k + (d * dim * c)) = 0;
				//printf("pixel: %d\n", (a * c) + k + (d * dim * c));
				for (int b = 0; b < dim; b++) {
					*(temp + (a * c) + k + (d * dim * c)) = *(temp + (a * c) + k + (d * dim * c)) + ((*(img.wht + (b * c) + k + (d * dim * c))) * (double)(*(H + a + (b * dim))));// / (double)dim;
					//printf("%f * %d = %f\n", *(img.wht + (b * c) + k + (d * dim * c)), *(H + a + (b * dim)), ((*(img.wht + (b * c) + k + (d * dim * c))) * (*(H + a + (b * dim)))));
				}
				//printf("%f\t", *(temp + (a * c) + k + (d * dim * c)));
				//printf("\n");
			}
			//printf("\n");
		}
	}
	//printf("rows transformed\n");

	// Transform the columns as column vectors multiplied by H: (1/2^n)iH
	//printf("First transform: columns\n");
	for (int k = 0; k < c; k++) {
		for (int d = 0; d < dim; d++) {
			//printf("column1\n");
			for (int a = 0; a < dim; a++) {
				*(temp2 + (a * c) + k + (d * dim * c)) = 0;
				//printf("pixel: %d\n", (a * c) + k + (d * dim * c));
				for (int b = 0; b < dim; b++) {
					*(temp2 + (a * c) + k + (d * dim * c)) = *(temp2 + (a * c) + k + (d * dim * c)) + ((double)(*(temp + (a * c) + k + (b * dim * c))) * (double)(*(H + b + (d * dim))));// / (double)dim;

					//printf("%f * %f = %f\n", (double)(*(temp + (a * c) + k + (b * dim * c))), (double)(*(H + b + (d * dim))), (double)(*(temp + (a * c) + k + (b * dim * c))) * (double)(*(H + b + (d * dim))));
					//printf("%f\t/\t%f\t=\t%f\n", ((double)(*(result.img + (a * c) + k + (b * dim * c))) * (double)(*(H + b + (d * dim)))), (double)dim, ((double)(*(result.img + (a * c) + k + (b * dim * c))) * (double)(*(H + b + (d * dim)))) / (double)dim);
				}
				//printf("%f\t", *(result.wht + (a * c) + k + (d * dim * c)));
				//printf("\n");
			}
			//printf("\n");
		}
	}
	//printf("columns transformed\n");
	
	img.img = malloc(size * c);
	//convert temp2 back to a char array image, should already be correct values minus float errors
	for (int i = 0; i < size * c; i++) {
		//printf("%f\n", *(temp2 + i));
		if (*(temp2 + i) > 255) {
			//printf("255\n");
			*(img.img + i) = 255;
		}
		else if (*(temp2 + i) < 0) {
			//printf("0\n");
			*(img.img + i) = 0;
		}
		else {
			//printf("%f\n", *(temp2 + i));
			*(img.img + i) = (uint8_t)(round(*(temp2 + i)));
		}
	}
	free(temp);
	free(temp2);
	
	return img;
}



/**
 * Prints the `signed` format Hadamard matrix.
 * \param n `signed` Hadamard matrix
 * \param length Length of matrix
 */
void print_matrix(signed char* n, int length) {
	printf("Printing Hadamard Matrix of Length %d:\n", length);
	signed int pixel = 0;
	for (int i = 0; i < length; i++) {
		for (int j = 0; j < length; j++) {
			pixel = *(n + (j + (i * length)));
			//printf("%d ", j + (i * length));
			if (pixel == 1) {
				printf(" 1 ");
			}
			else if(pixel == -1){
				printf(" - ");
			}
			else {
				printf(" %d ", pixel);
			}
		}
		printf("\n");
	}
}

/**
 * Prints the `unsigned` format Hadamard matrix.
 * \param n `unsigned` Hadamard matrix
 * \param length Length of matrix
 */
void print_matrixu(unsigned char* n, int length) {
	for (int i = 0; i < length; i++) {
		for (int j = 0; j < length; j++) {
			printf("%d\t", n[j + i * length]);
		}
		printf("\n");
	}
}

/**
 * Prints the `double` format Hadamard matrix.
 * \param n `double` Hadamard matrix
 * \param length Length of matrix
 */
void print_matrixf(double* n, int length) {
	for (int i = 0; i < length; i++) {
		for (int j = 0; j < length; j++) {
			printf("%f\t", n[j + i*length]);
		}
		printf("\n");
	}
}
