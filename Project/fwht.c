///\file fwht.c
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>


#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KRED  "\x1B[31m"
#define KMAG  "\x1B[35m"
#define RESET "\x1B[0m"
/**
 * Helper function for Fast Walsh-Hadamard Transform
 * \param n the length of one side of the padded n by n image in pixels
 * \param src `int[]` containing the data to be transformed
 * \param dst `int[]` that will store the transformed data
 */
void fwht_transform(int n, const int *src, int *dst);
/**
 * Helper function for Fast Walsh-Hadamard Transform
 * \param n the length of one side of the padded n by n image in pixels
 * \param src `double[]` containing the data to be transformed
 * \param dst `double[]` that will store the transformed data
 */
void dbl_fwht_transform(int n, const double *src, double *dst);
/**
 * Performs a Fast Walsh-Hadamard Transform on an input `buff` and returns the output image as a `buff`.
 * \param buffer `buff` containing the image information before the Fast Walsh-Hadamard Transform
 * \param buffname name of the `buff` that will be output
 *  
 * \return `buff` containing the Fast Walsh-Hadamard transformed image data.
 */
struct buff fwht(struct buff buffer, char* buffname){
    // printf("Hellow World!\n");
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

	strcpy(result.imageName, "fwht");
	strcat(result.imageName, ext);
	strcpy(result.name, buffname);
	strcpy(result.status, "true");
	result.has_wht = 2;
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
    // printf(" channels: %d\n dimensions: %d\n", c, dim);
    
    for (int k = 0; k < c; k++) { // for each channel
		for (int d = 0; d < dim; d++) {    // for each column
			//printf("column1\n");
            double temp[dim];
			for (int a = 0; a < dim; a++) { // for each pixel
                temp[a] = *(result.img + (d * c) + k + (a * dim * c));
			}


			dbl_fwht_transform(dim, temp, temp); // perform column-wise fast transform

            for(int a = 0; a < dim; a++){
				*(result.wht + (d * c) + k + (a * dim * c)) = temp[a];
                // *(result.img + (d * c) + k + (a * dim * c)) = temp[a];  // put values back into wht image
            }
        
			//printf("\n");
		}

		/*for (int d = 0; d < dim; d++) {    // for each row
			//printf("column1\n");
            double temp[dim];
			for (int a = 0; a < dim; a++) { // for each pixel
                temp[a] = *(result.img + (a * c) + k + (d * dim * c));
			}

			dbl_fwht_transform(dim, temp, temp); // perform row-wise fast transform

            for(int a = 0; a < dim; a++){
				*(result.wht + (a * c) + k + (d * dim * c)) = temp[a];
                // *(result.img + (d * c) + k + (a * dim * c)) = temp[a];
            }
        
			//printf("\n");
		}*/
	}

	int max = 0;
	int min = 10000000;
	int val = 0;
	for(int i = 0; i < size; i++){
		val = (int)*(result.wht + i);
		if(val > max){ 
			max = val;
		}
		if(min > val){
			min = val;
		}
		else {
			//printf("%f\n", *(temp2 + i));
			*(img.img + i) = (uint8_t)(round(*(img.wht + i)));
		}*/
	}
	// printf("MAX: %d\n", max);
	// printf("MIN: %d\n", min);
	
	// result.img = malloc(size * c);
	//convert temp2 back to a char array image, should already be correct values minus float errors
	for (int i = 0; i < size * c; i++) {
		double scaled_dbl = (*(result.wht + i) - min) / (max - min) * 255;
		int value = (uint8_t)(round(scaled_dbl));
		*(result.img + i) = value;
		//printf("%f\n", *(temp2 + i));
		/*if (*(img.wht + i) > 255) {
			//printf("255\n");
			*(img.img + i) = 255;
		}
		else if (*(img.wht + i) < 0) {
			//printf("0\n");
			*(img.img + i) = 0;
		}
		else {
			//printf("%f\n", *(temp2 + i));
			*(img.img + i) = (uint8_t)(round(*(img.wht + i)));
		}*/
	}
	// printf("MAX: %d\n", max);
	// printf("MIN: %d\n", min);
	
	// result.img = malloc(size * c);
	//convert temp2 back to a char array image, should already be correct values minus float errors
	for (int i = 0; i < size * c; i++) {
		double scaled_dbl = (*(result.wht + i) - min) / (max - min) * 255;
		int value = (uint8_t)(round(scaled_dbl));
		*(result.img + i) = value;
		//printf("%f\n", *(temp2 + i));
		/*if (*(img.wht + i) > 255) {
			//printf("255\n");
			*(img.img + i) = 255;
		}
		else if (*(img.wht + i) < 0) {
			//printf("0\n");
			*(img.img + i) = 0;
	// result.img = malloc(size * c);
	//convert temp2 back to a char array image, should already be correct values minus float errors
	for (int i = 0; i < size * c; i++) {
		double scaled_dbl = (*(result.wht + i) - min) / (max - min) * 255;
		int value = (uint8_t)(round(scaled_dbl));
		*(result.img + i) = value;
		//printf("%f\n", *(temp2 + i));
		/*if (*(img.wht + i) > 255) {
			//printf("255\n");
			*(img.img + i) = 255;
		}
		else if (*(img.wht + i) < 0) {
			//printf("0\n");
			*(img.img + i) = 0;
		}
		else {
			//printf("%f\n", *(temp2 + i));
			*(img.img + i) = (uint8_t)(round(*(img.wht + i)));
		}*/
	}
    return result;
}
/**
 * Inverse function for Fast Walsh-Hadamard Transform
 * \param img `buff` containing the image to be inversely transformed
 * 
 * \return `buff` containing inverse FWHT image
 */
struct buff ifwht(struct buff img) {
	// Inverse FWHT transform, takes an wft image as input and recovers the original image from it.
	// To invert, run the normal transform in reverse, i.e. transform rows then columns

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


	for (int k = 0; k < c; k++) { // for each channel

		/* for (int d = 0; d < dim; d++) {    // for each row
			//printf("column1\n");
            double temp[dim];
			for (int a = 0; a < dim; a++) { // for each pixel
                temp[a] = *(img.wht + (a * c) + k + (d * dim * c));
			}

			dbl_fwht_transform(dim, temp, temp); // perform row-wise fast transform

            for(int a = 0; a < dim; a++){
				*(img.wht + (a * c) + k + (a * dim * c)) = temp[a];
                // *(result.img + (d * c) + k + (a * dim * c)) = temp[a];
            }
        
			//printf("\n");
		} */

		for (int d = 0; d < dim; d++) {    // for each column
			//printf("column1\n");
            double temp[dim];
			for (int a = 0; a < dim; a++) { // for each pixel
                temp[a] = *(img.wht + (d * c) + k + (a * dim * c));
			}


			dbl_fwht_transform(dim, temp, temp); // perform column-wise fast transform

            for(int a = 0; a < dim; a++){
				*(img.wht + (d * c) + k + (a * dim * c)) = temp[a];
                // *(result.img + (d * c) + k + (a * dim * c)) = temp[a];  // put values back into wht image
            }
        
			//printf("\n");
		}
	}

	// find min and max value for scaling
	int max = 0;
	int min = 10000000;
	int val = 0;
	for(int i = 0; i < size; i++){
		val = (int)*(img.wht + i);
		if(val > max){ 
			max = val;
		}
		if(min > val){
			min = val;
		}
	}
	// printf("MAX: %d\n", max);
	// printf("MIN: %d\n", min);
	
	img.img = malloc(size * c);
	//convert temp2 back to a char array image, should already be correct values minus float errors
	for (int i = 0; i < size * c; i++) {
		double scaled_dbl = (*(img.wht + i) - min) / (max - min) * 255;
		int value = (uint8_t)(round(scaled_dbl));
		*(img.img + i) = value;
		//printf("%f\n", *(temp2 + i));
		/*if (*(img.wht + i) > 255) {
			//printf("255\n");
			*(img.img + i) = 255;
		}
		else if (*(img.wht + i) < 0) {
			//printf("0\n");
			*(img.img + i) = 0;
		}
		else {
			//printf("%f\n", *(temp2 + i));
			*(img.img + i) = (uint8_t)(round(*(img.wht + i)));
		}*/
	}
	
	return img;
}

void fwht_transform(int n, const int *src, int *dst)
{
    int adata[n];
    int bdata[n];
    int *a = adata;
    int *b = bdata;
    void *tmp;
    memcpy(a, src, sizeof(int)*n);
    
    // Fast Walsh Hadamard Transform.
    int i, j, s;
    for (i = n>>1; i > 0; i>>=1) {
        for (j = 0; j < n; j++) {
            s = j/i%2;
            b[j]=a[(s?-i:0)+j]+(s?-1:1)*a[(s?0:i)+j];
        }
        tmp = a; a = b; b = tmp;
    }
    
    memcpy(dst, a, sizeof(int)*n);
}

void dbl_fwht_transform(int n, const double *src, double *dst)  // double version for wht image
{
    double adata[n];
    double bdata[n];
    double *a = adata;
    double *b = bdata;
    void *tmp;
    memcpy(a, src, sizeof(double)*n);
    
    // Fast Walsh Hadamard Transform.
    int i, j, s;
    for (i = n>>1; i > 0; i>>=1) {
        for (j = 0; j < n; j++) {
            s = j/i%2;
            b[j]=a[(s?-i:0)+j]+(s?-1:1)*a[(s?0:i)+j];
        }
        tmp = a; a = b; b = tmp;
    }
    
    memcpy(dst, a, sizeof(double)*n);
}

void fwht_normalize(int n, int *src)
{
    int i;
    for (i = 0; i < n; i++) src[i] /= n;
}
double fwht_sum_absolute_difference(int n, int *a, int *b)
{
    long sum = 0;
    int diff;
    int i;
    for (i = 0; i < n; i++) {
        diff = b[i]-a[i];
        sum += diff > 0 ? diff : -diff;
    }
    return sum/(double)n;
}