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

void fwht_transform(int n, const int *src, int *dst);


/** 
 * The fast walsh-hadamard transform computes the walsh-hadamard transform with in O(nlogn).
 * This function prepares the rows and columns to pass to another function, fwht_transform,
 * which computes the transform.
 * \param buffer the buffer which contains the image going to get transformed
 * \param bufname the name of the buffer which contains the image going to get transformed
 * 
 * \return buff of the transformed image
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
    // printf(" channels: %d\n dimensions: %d\n", c, dim);
    
    for (int k = 0; k < c; k++) { // for each channel
		for (int d = 0; d < dim; d++) {    // for each column
			//printf("column1\n");
            int temp[dim];
			for (int a = 0; a < dim; a++) { // for each pixel
                temp[a] = *(result.img + (d * c) + k + (a * dim * c));
                
                // printf("%d\n", temp[a]);
				//*(result.wht + (a * c) + k + (d * dim * c)) = 0;
				// printf("pixel: %d\n", (a * c) + k + (d * dim * c));
				//for (int b = 0; b < dim; b++) {
					//*(result.wht + (a * c) + k + (d * dim * c)) = *(result.wht + (a * c) + k + (d * dim * c)) + ((double)(*(result.img + (a * c) + k + (b * dim * c))) * (double)(*(H + b + (d * dim)))) / (double)dim;
					
					//printf("%f * %f = %f\n", (double)(*(result.img + (a * c) + k + (b * dim * c))), (double)(*(H + b + (d * dim))), (double)(*(result.img + (a * c) + k + (b * dim * c))) * (double)(*(H + b + (d * dim))));
					//printf("%f\t/\t%f\t=\t%f\n", ((double)(*(result.img + (a * c) + k + (b * dim * c))) * (double)(*(H + b + (d * dim)))), (double)dim, ((double)(*(result.img + (a * c) + k + (b * dim * c))) * (double)(*(H + b + (d * dim)))) / (double)dim);
				//}
				//printf("%f\t", *(result.wht + (a * c) + k + (d * dim * c)));
				//printf("\n");
			}


			fwht_transform(dim, temp, temp); // perform column-wise fast transform

            for(int a = 0; a < dim; a++){
                *(result.img + (d * c) + k + (a * dim * c)) = temp[a];
            }
        
			//printf("\n");
		}

		for (int d = 0; d < dim; d++) {    // for each row
			//printf("column1\n");
            int temp[dim];
			for (int a = 0; a < dim; a++) { // for each pixel
                temp[a] = *(result.img + (a * c) + k + (d * dim * c));
			}

			fwht_transform(dim, temp, temp); // perform row-wise fast transform

            for(int a = 0; a < dim; a++){
                *(result.img + (d * c) + k + (a * dim * c)) = temp[a];
            }
        
			//printf("\n");
		}
	}
    return result;
}

/** 
 * This function takes in a 
 * \param n this is the dimension of the square image matrix
 * \param src an array of values from a row of the original image which will be transformed
 * \param dst the row array of the results of the transform
 */
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

/** 
 * This function does a basic normalization over values in an int array 
 * \param n this is the value to divide everything by
 * \param src this is the array of values to be normalized
 */
void fwht_normalize(int n, int *src)
{
    int i;
    for (i = 0; i < n; i++) src[i] /= n;
}

/** 
 * This function calculates an average of absolute differences over two arrays.
 * \param n this is the length of the two arrays
 * \param a this is the first int array
 * \param a this is the second int array
 */
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