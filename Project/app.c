/// \file app.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <setjmp.h>
#include "gd.h"

struct buff
{
	char imageName[32];         ///< string name of the image loaded into buffer
	char name[30];              ///< string name of the buffer
	char status[30];            ///< string status of the buffer 
	unsigned char *img;         ///< image data loaded by `stb_image.h`
	int width,                  ///< width of image in pixels
        height,                 ///< height of image in pixels
        channels;               ///< number of RGBA channels within pixels
	gdImagePtr imrgb;           ///< image data loaded by `libgd`
	int isLibgd;                ///< signifies whether image has been loaded by `libgd`
	double* wht;                ///< stores the walsh-hadamard 
	unsigned char* whtimg;      ///< data for walsh-hadamard transformed image
	int has_wht;                ///< signifies whether image has been transformed using Walsh-Hadamard 
};

struct template
{
	char name[30];      ///< template name
	float** mask;       ///< masking matrix
	float** mask_tp;    ///< modified mask for target pixel placement
	int tpRow;          ///< target pixel number of rows
	int tpColumn;       ///< target pixel number of columns
};

#include "read.c"
#include "write.c"
#include "arithmetics.c"
#include "brighten.c"
#include "edge.c"
#include "display.c"
#include "histoEQ.c"
#include "rotation.c"
#include "blurr.c"
#include "sharpen.c"
#include "grayscale.c" //included in wht.c already
#include "spectrum.c"
#include "spectrum_log10.c"
#include "wht.c"		 //included in read.c already
#include "fwht.c"

#define KGRN "\x1B[32m"     ///< Escape code for green terminal text 
#define KYEL "\x1B[33m"     ///< Escape code for yellow terminal text
#define KBLU "\x1B[34m"     ///< Escape code for blue terminal text
#define KRED "\x1B[31m"     ///< Escape code for red terminal text
#define KMAG "\x1B[35m"     ///< Escape code for magenta terminal text
#define RESET "\x1B[0m"     ///< Resets terminal text color to default (white)

void addBuffer(struct buff buffer, struct buff *buffers, int *buffCount);
struct buff buffSearch(char *buffName, struct buff *buffers, int buffCount);
void printBuffer(struct buff *buffer, int buffCount);
void printMenu();
char *get_filename_ext(const char *filename);
int check_types(char *ext, char *file_types[]);
int count_characters(const char* str, char character);

int main(int argc, char **argv)
{
	printf(KGRN "Welcome to the UNIX Image Manipulation tool.\n");
	printf("Type \"" KYEL "menu" KGRN "\"  to view the list of commands or \"" KYEL "list" KGRN "\" to view your buffers.\n\n" RESET);

	struct buff buffers[10];
	int buffCount = 0;

	struct template T;

	char p[100];

	printf("UIMT> ");
	fgets(p, 99, stdin);
	p[strlen(p) - 1] = '\0';

	while (1)
	{
		char *command;
		char *imageName;
		char *buffName;
		char *amount;
		char pCopy[100];
		strcpy(pCopy, p);
		int spaceCount = count_characters(pCopy, ' ');
		if (strlen(p) == 0) {
			strcpy(p, "XXXXX");
		}
		command = strtok(p, " ");
		if (strcmp(command, "menu") == 0)
		{
			printMenu();
		}
		else if (strcmp(command, "read") == 0)
		{
			if (spaceCount != 3) {
				printf(KRED "Error: " RESET "Incorrect syntax on Read command.\n       Use the syntax: read <file name> into <buffer name>\n");
				p[0] = '\0';
				continue;
			}
			imageName = strtok(NULL, " ");
			if (strlen(imageName) > 14)
			{
				printf(KRED "Error: " RESET "Image name too long. Image + extension must be shorter than 14 characters.\n");
			}
			else
			{
				char *ext = get_filename_ext(imageName);
				char *file_types[5] = {"jpeg", "gif", "tiff", "png", "wht"}; //ALLOW NEW FILE TYPES HERE
				int approved = check_types(ext, file_types);
				if (approved != 1)
				{
					printf(KRED "Error: " RESET "Image file type is not approved.\n");
					exit(1);
				}
				strtok(NULL, " ");
				buffName = strtok(NULL, " ");
				struct buff temp = readToBuff(imageName, buffName);
				if (temp.has_wht == 1) {
					temp = iwht(temp);
					whtHistEQ(&temp);
					//temp.img = temp.whtimg;
				}
				addBuffer(temp, buffers, &buffCount);
			}
		}
		else if (strcmp(command, "write") == 0)
		{
			if (spaceCount != 3) {
				printf(KRED "Error: " RESET "Incorrect syntax on Write command.\n       Use the syntax: write <buffer name> to <file name>\n");
				p[0] = '\0';
				continue;
			}
			buffName = strtok(NULL, " ");
			strtok(NULL, " ");
			imageName = strtok(NULL, " ");
			if (strlen(imageName) > 14)
			{
				printf(KRED "Error: " RESET "Image name too long. Image + extension must be shorter than 14 characters.\n");
			}
			else
			{
				printf(KYEL "\nWriting %s into %s...\n" RESET, buffName, imageName);
				writeToImage(buffSearch(buffName, buffers, buffCount), imageName, 0);
				printf(KYEL "Done writing!\n\n" RESET);
			}
		}
		else if (strcmp(command, "write_wht") == 0)
		{
			if (spaceCount != 3) {
				printf(KRED "Error: " RESET "Incorrect syntax on write_wht command.\n       Use the syntax: write_wht <buffer name> to <file name>\n");
				p[0] = '\0';
				continue;
			}
			buffName = strtok(NULL, " ");
			strtok(NULL, " ");
			imageName = strtok(NULL, " ");
			if (strlen(imageName) > 14)
			{
				printf(KRED "Error: " RESET "Image name too long. Image + extension must be shorter than 14 characters.\n");
			}
			else
			{
				printf(KYEL "\nWriting %s into %s...\n" RESET, buffName, imageName);
				writeToImage(buffSearch(buffName, buffers, buffCount), imageName, 1);
				printf(KYEL "Done writing!\n\n" RESET);
			}
		}
		else if (strcmp(command, "list") == 0)
		{
			printBuffer(buffers, buffCount);
		}
		else if (strcmp(command, "brighten") == 0)
		{
			if (spaceCount != 5) {
				printf(KRED "Error: " RESET "Incorrect syntax on brighten command.\n       Use the syntax: brighten <buffer name> into <new buffer name> by <value between 0 and 255>\n");
				p[0] = '\0';
				continue;
			}
			buffName = strtok(NULL, " ");
			strtok(NULL, " ");
			imageName = strtok(NULL, " ");
			if (strlen(imageName) > 14)
			{
				printf(KRED "Error: " RESET "Image name too long. Image + extension must be shorter than 14 characters.\n\n");
			}
			else
			{
				strtok(NULL, " ");
				amount = strtok(NULL, " ");
				if (amount != NULL){
					addBuffer(brighten(buffSearch(buffName, buffers, buffCount), imageName, true, atoi(amount)), buffers, &buffCount);
				}
				else {
					printf(KRED "Error: " RESET "Brightening routine not written in correct format.\n\n");
				}
			}
		}
		else if (strcmp(command, "darken") == 0)
		{
			if (spaceCount != 5) {
					printf(KRED "Error: " RESET "Incorrect syntax on darken command.\n       Use the syntax: darken <buffer name> into <new buffer name> by <value between 0 and 255>\n");
				p[0] = '\0';
				continue;
			}
			buffName = strtok(NULL, " ");
			strtok(NULL, " ");
			imageName = strtok(NULL, " ");
			if (strlen(imageName) > 14)
			{
				printf(KRED "Error: " RESET "Image name too long. Image + extension must be shorter than 14 characters.\n");
			}
			else
			{
				strtok(NULL, " ");
				amount = strtok(NULL, " ");
				if (amount != NULL){
					addBuffer(brighten(buffSearch(buffName, buffers, buffCount), imageName, false, atoi(amount)), buffers, &buffCount);
				}
				else {
					printf(KRED "Error: " RESET "Darkening routine not written in correct format.\n");
				}
			}
		}
		else if (strcmp(command, "display") == 0)
		{
			if (spaceCount != 1) {
				printf(KRED "Error: " RESET "Incorrect syntax on display command.\n       Use the syntax: display <buffer name>\n");
				p[0] = '\0';
				continue;
			}
			buffName = strtok(NULL, " ");
			char tem[10] = "temp.png";

			struct buff temp = buffSearch(buffName, buffers, buffCount);
			if (strcmp(temp.status, "false") != 0)
			{
				writeToImage(temp, tem, 0);
				displayImage(tem, argc, argv);
			}

			remove(tem);
		}
		else if (strcmp(command, "display_wht") == 0)
		{
			if (spaceCount != 1) {
				printf(KRED "Error: " RESET "Incorrect syntax on display_wht command.\n       Use the syntax: display_wht <buffer name>\n");
				p[0] = '\0';
				continue;
			}
			buffName = strtok(NULL, " ");
			char tem[10] = "temp.png";

			struct buff temp = buffSearch(buffName, buffers, buffCount);
			if (temp.has_wht == 0) {
				printf(KRED "Error: " RESET "Buffer does not contain a wht image.\n");
			}
			else if (strcmp(temp.status, "false") != 0)
			{
				writeToImage(temp, tem, 1);
				displayImage(tem, argc, argv);
			}

			remove(tem);
		}
		else if (strcmp(command, "quit") == 0)
		{
			break;
		}
		else if (strcmp(command, "horizontal") == 0 || strcmp(command, "vertical") == 0 || strcmp(command, "combined") == 0)
		{
			if (spaceCount != 4) {
				printf(KRED "Error: " RESET "Incorrect syntax on edge detection command.\n       Use the syntax: <horizontal/vertical/combined> <kirsch/prewitt/sobel> <buffer-name> into <new-buffer-name>\n");
				p[0] = '\0';
				continue;
			}
			char *type = strtok(NULL, " ");
			buffName = strtok(NULL, " ");
			strtok(NULL, " ");
			char* resultBuffName = strtok(NULL, " ");
		
			struct buff temp = detectEdge(command, type, buffSearch(buffName, buffers, buffCount));
			strcpy(temp.name, resultBuffName);
			temp.isLibgd = 1;
			addBuffer(temp, buffers, &buffCount);
			
		}
		else if (strcmp(command, "addition") == 0 || strcmp(command, "subtraction") == 0 || strcmp(command, "division") == 0 || strcmp(command, "multiplication") == 0)
		{
			if (spaceCount != 6) {
				printf(KRED "Error: " RESET "Incorrect syntax on arithmetic command.\n       Use the syntax: <addition/subtraction/multiplication/division> : <new buffer name> = <buffer 1> <+-*/> <buffer 2>\n");
				p[0] = '\0';
				continue;
			}
			strtok(NULL, " ");
			buffName = strtok(NULL, " ");
			strtok(NULL, " ");
			char *buff1 = strtok(NULL, " ");
			char *cmd = strtok(NULL, " ");
			char *buff2 = strtok(NULL, " ");
			if (strcmp(cmd, "+") == 0)
			{
				addBuffer(add(buffSearch(buff1, buffers, buffCount), buffSearch(buff2, buffers, buffCount), buffName),
						  buffers, &buffCount);
			}
			else if (strcmp(cmd, "-") == 0)
			{
				addBuffer(subtract(buffSearch(buff1, buffers, buffCount), buffSearch(buff2, buffers, buffCount), buffName),
						  buffers, &buffCount);
			}
			else if (strcmp(cmd, "*") == 0)
			{
				addBuffer(multiply(buffSearch(buff1, buffers, buffCount), buffSearch(buff2, buffers, buffCount), buffName),
						  buffers, &buffCount);
			}
			else if (strcmp(cmd, "/") == 0)
			{
				addBuffer(divide(buffSearch(buff1, buffers, buffCount), buffSearch(buff2, buffers, buffCount), buffName),
						  buffers, &buffCount);
			}
			else
			{
				printf(KRED "Error:" RESET " Arithmetic syntax incorrect\n");
			}
		}
		else if ((strcmp(command, "histeq") == 0))
		{
			if (spaceCount != 3) {
				printf(KRED "Error: " RESET "Incorrect syntax on histeq command.\n       Use the syntax: histeq <buffer-name> into <new-buffer-name>\n");
				p[0] = '\0';
				continue;
			}
			buffName = strtok(NULL, " ");
			strtok(NULL, " ");
			char *resultBuffName = strtok(NULL, " ");

			struct buff temp = buffSearch(buffName, buffers, buffCount);
			temp = histogramEqualization(temp, command);
			
			strcpy(temp.name, resultBuffName);
			addBuffer(temp, buffers, &buffCount);
		}
		else if ((strcmp(command, "flip") == 0))
		{
			if (spaceCount != 2) {
				printf(KRED "Error: " RESET "Incorrect syntax on flip command.\n       Use the syntax: flip <vertical/horizontal> <buffer-name>\n");
				p[0] = '\0';
				continue;
			}
			char *rot = strtok(NULL, " ");
			if ((strcmp(rot, "vertical") == 0))
			{
				buffName = strtok(NULL, " ");
				struct buff temp = buffSearch(buffName, buffers, buffCount);
				temp = verticalFlip(temp);
				addBuffer(temp, buffers, &buffCount);
			}
			else if (strcmp(rot, "horizontal") == 0)
			{
				buffName = strtok(NULL, " ");
				struct buff temp = buffSearch(buffName, buffers, buffCount);
				temp = horizontalFlip(temp);
				addBuffer(temp, buffers, &buffCount);
			}
			else
			{
				printf(KRED "Error: " RESET "Invalid mirroring!\n");
			}
		}
		else if ((strcmp(command, "rotate") == 0))
		{
			if (spaceCount != 3) {
				printf(KRED "Error: " RESET "Incorrect syntax on rotate command.\n       Use the syntax: rotate <buffer-name> by <degrees between -360 and 360>\n");
				p[0] = '\0';
				continue;
			}
			buffName = strtok(NULL, " ");
			strtok(NULL, " ");
			char* degree = strtok(NULL, " ");

			float degrees;
			sscanf(degree, "%f", &degrees);

			if(degrees>=(float)360 || degrees <=(float)-360)
				printf("Error: degree of rotation is out of limits.\n\n");
			else
				addBuffer(rotate(buffSearch(buffName,buffers, buffCount), degrees), buffers, &buffCount);
		}
		else if((strcmp(command, "blurr") == 0))
		{
			if (spaceCount != 3) {
				printf(KRED "Error: " RESET "Incorrect syntax on blur command.\n       Use the syntax: blurr <buffer> <radius> <sigma>\n");
				p[0] = '\0';
				continue;
			}
			buffName = strtok(NULL, " ");
			char* radius = strtok(NULL, " ");
			char* sigma = strtok(NULL, " ");
			struct buff temp = blurr(buffSearch(buffName, buffers, buffCount), atoi(radius), atof(sigma));
			addBuffer(temp, buffers, &buffCount);
		}
		else if((strcmp(command, "sharpen") == 0))
		{
			if (spaceCount != 2) {
				printf(KRED "Error: " RESET "Incorrect syntax on sharpen command.\n       Use the syntax: sharpen <low/high> <buffer>\n");
				p[0] = '\0';
				continue;
			}
			char* choice = strtok(NULL, " ");
			if (!(strcmp(choice, "low") == 0) && !(strcmp(choice, "high") == 0)){
				printf(KRED "Error: " RESET "Invalid sharpening command.\n");
			}
			else {
				buffName = strtok(NULL, " ");
				struct buff temp = sharpen(buffSearch(buffName, buffers, buffCount), choice);
				addBuffer(temp, buffers, &buffCount);
			}
		}

		else if((strcmp(command, "define_template") == 0))
		{
			struct template temp;
			char* name = strtok(NULL, " ");
			strcpy(temp.name, name);
			char* coords = strtok(NULL, " ");
			char* coord;
			temp.mask = (float **)malloc(2 * sizeof(float*));
			for(int row = 0; row < 2; row++)
				temp.mask[row] = (float *)malloc(2 * sizeof(float)); // Hard coded 2x2 matrix
			int i = 0;
			int j = 0;
			while(!(strcmp(coords, "tp") == 0)) {
				if(j == 0) {
					coord = &coords[1];
					temp.mask[i][j] = atof(coord); // If on the left side, input will be [<coord> (we ignore the square bracket)
				}
				else if(j == 1) // Harded coded 1, should be number of columns
					coord = &coords[0];
					temp.mask[i][j] = atof(coord); // If on the right side, input will be <coord>] (we ignore the square bracket)
				if(j == 0)
					j++;
				else if(j == 1) {
					i++;
					j = 0;
				}
				coords = strtok(NULL, " ");
			}

			
			char* equal_sign = strtok(NULL, " ");
			temp.mask_tp = (float **)malloc(3 * sizeof(float*));
			for(int row = 0; row < 3; row++)
				temp.mask_tp[row] = (float *)malloc(3 * sizeof(float)); // Hard coded 3x3 matrix
			char* tp_coords = strtok(NULL, " ");
			coord = &tp_coords[1];
			temp.tpRow = atof(coord);
			coord = &tp_coords[3];
			temp.tpColumn = atof(coord);
			if((temp.tpRow == 0) && (temp.tpColumn == 0)) {
				temp.mask_tp[1][1] = temp.mask[0][0];			 	// If matrix is  
				temp.mask_tp[1][2] = temp.mask[0][1]; 				// [1 2]
				temp.mask_tp[2][1] = temp.mask[1][0]; 				// [3 4], and target pixel is (0,0) (=1, top left), modify matrix to be
				temp.mask_tp[2][2] = temp.mask[1][1]; 				// [0 0 0] 
			}														// [0 1 2]
			else if((temp.tpRow == 0) && (temp.tpColumn == 1)) {	// [0 3 4], such that a normal 3x3 convlution will output in the middle
				temp.mask_tp[1][0] = temp.mask[0][0];				// which is the target pixel, added 0s will not change the result of the convolution
				temp.mask_tp[1][1] = temp.mask[0][1];				// just a way to set the target pixel to the middle
				temp.mask_tp[2][0] = temp.mask[1][0];
				temp.mask_tp[2][1] = temp.mask[1][1];
			}
			else if((temp.tpRow == 1) && (temp.tpColumn == 0)) {
				temp.mask_tp[0][1] = temp.mask[0][0];
				temp.mask_tp[0][2] = temp.mask[0][1];
				temp.mask_tp[1][1] = temp.mask[1][0];
				temp.mask_tp[1][2] = temp.mask[1][1];
			}
			else if((temp.tpRow == 1) && (temp.tpColumn == 1)) {
				temp.mask_tp[0][0] = temp.mask[0][0];
				temp.mask_tp[0][1] = temp.mask[0][1];
				temp.mask_tp[1][0] = temp.mask[1][0];
				temp.mask_tp[1][1] = temp.mask[1][1];
			}
			T=temp;
		}
		

		else if((strcmp(command, "convolve3x3") == 0))
		{
			if (spaceCount != 8) {
				printf(KRED "Error: " RESET "Incorrect syntax on convolve3x3 command.\n       Use the syntax: convolve3x3 <template> <buffer-name>\n       with template in the form [a b c][d e f][g h i] all integers\n");
				p[0] = '\0';
				continue;
			}
			char* row1, *row2, *row3;
			row1 = strtok(NULL, "]");
			row2 = strtok(NULL, "]");
			row3 = strtok(NULL, "]");
			buffName = strtok(NULL, " ");	
			struct buff temp = convolve3X3(buffSearch(buffName, buffers, buffCount), row1, row2, row3);
			addBuffer(temp, buffers, &buffCount);
		}
		else if((strcmp(command, "convolve_template") == 0))
		{
			if (spaceCount != 2) {
				printf(KRED "Error: " RESET "Incorrect syntax on convolve_template command.\n       Use the syntax: convolve_template <buff-name> <template-name>\n");
				p[0] = '\0';
				continue;
			}
			buffName = strtok(NULL, " ");
			char* templateName = strtok(NULL, " ");

			struct buff temp = (buffSearch(buffName, buffers, buffCount));

			struct buff out = temp;

			char input[100];
    		printf("Enter name of buffer: \n");
   			gets(input);
			strcpy(out.name,input);

			float mask[3][3] = {
				{T.mask_tp[0][0], T.mask_tp[0][1], T.mask_tp[0][2]},
				{T.mask_tp[1][0], T.mask_tp[1][1], T.mask_tp[1][2]},
				{T.mask_tp[2][0], T.mask_tp[2][1], T.mask_tp[2][2]}
			};


			gdImageConvolution(out.imrgb, mask, 1.0,0.0);

			out.isLibgd=1;

			addBuffer(out, buffers, &buffCount);
			
		}
		else if ((strcmp(command, "test_hadamard") == 0)) {
			print_matrix(hadamard(1), 2);
			printf("\n");
			print_matrix(hadamard(2), 4);
			printf("\n");
			print_matrix(hadamard(3), 8);
			printf("\n");
			print_matrix(hadamard(4), 16);
			printf("\n");
			print_matrix(hadamard(5), 32);
			printf("\n");
		}
		else if ((strcmp(command, "details") == 0)) {
			for (int i = 0; i < buffCount; i++) {
				printf("Buffer: %s\n", buffers[i].name);
				printf("Channels: %d\n", buffers[i].channels);
				printf("Height: %d\n", buffers[i].height);
				printf("Width: %d\n", buffers[i].width);
				printf("has_wht: %d\n", buffers[i].has_wht);
				printf("img_ptr: %p\n", buffers[i].img);
				printf("whtimg_ptr: %p\n", buffers[i].whtimg);
				printf("\n");
			}
		}
		else if ((strcmp(command, "wht") == 0)) {  
			if (spaceCount != 3) {
				printf(KRED "Error: " RESET "Incorrect syntax on wht command.\n       Use the syntax: wht <buffer> into <new buffer>\n");
				p[0] = '\0';
				continue;
			}
			imageName = strtok(NULL, " ");
			strtok(NULL, " ");
			buffName = strtok(NULL, " ");
			struct buff temp = wht(buffSearch(imageName, buffers, buffCount), buffName);
			whtHistEQ(&temp);
			addBuffer(temp, buffers, &buffCount);
		}
		else if ((strcmp(command, "spectrum") == 0)) {

			if (spaceCount != 1) {
				printf(KRED "Error: " RESET "Incorrect syntax on spectrum command.\n       Use the syntax: spectrum <buffer>\n");
				p[0] = '\0';
				continue;
			}
			imageName = strtok(NULL, " ");
			struct buff temp = buffSearch(imageName, buffers, buffCount);
			if (temp.has_wht == 0){
				printf(KRED "Error: this buffer does not contain wht data\n");
				continue;
			}

			spectrum(temp);
			
		}
		else if ((strcmp(command, "spectrum_log10") == 0)) {

			if (spaceCount != 1) {
				printf(KRED "Error: " RESET "Incorrect syntax on spectrum command.\n       Use the syntax: spectrum <buffer>\n");
				p[0] = '\0';
				continue;
			}
			imageName = strtok(NULL, " ");
			struct buff temp = buffSearch(imageName, buffers, buffCount);
			if (temp.has_wht == 0){
				printf(KRED "Error: this buffer does not contain wht data\n");
				continue;
			}

			spectrum_log10(temp);
		}
		
		else if ((strcmp(command, "fwht") == 0)) {
			if (spaceCount != 3) {
				printf(KRED "Error: " RESET "Incorrect syntax on fwht command.\n       Use the syntax: fwht <buffer> into <new buffer>\n");
				p[0] = '\0';
				continue;
			}
			imageName = strtok(NULL, " ");
			strtok(NULL, " ");
			buffName = strtok(NULL, " ");
			struct buff temp = grayscale(buffSearch(imageName, buffers, buffCount), buffName);
			temp = fwht(temp, buffName);
			// struct buff temp = fwht(buffSearch(imageName, buffers, buffCount), buffName);
			// whtHistEQ(&temp);
			addBuffer(temp, buffers, &buffCount);
		}
		else if ((strcmp(command, "grayscale") == 0)) {
			if (spaceCount != 3) {
				printf(KRED "Error: " RESET "Incorrect syntax on grayscale command.\n       Use the syntax: grayscale <buffer> into <new buffer>\n");
				p[0] = '\0';
				continue;
			}
			imageName = strtok(NULL, " ");
			strtok(NULL, " ");
			buffName = strtok(NULL, " ");
			struct buff temp = grayscale(buffSearch(imageName, buffers, buffCount), buffName);
			addBuffer(temp, buffers, &buffCount);
		}
		else if ((strcmp(command, "XXXXX") == 0)) {
			// do nothing! this is if the user types Enter only
		}
		else							
		{												
			printf(KRED "Error: " RESET "Command not found or not supported, please type menu for list of commands.\n");
		}
		printf("UIMT> ");
		fgets(p, 99, stdin);
		p[strlen(p) - 1] = '\0';
	}
}

/**
 * Prints a list of all usable commands and their input formats.
 */
void printMenu()
{
	printf(KGRN "\n----- Commands -----\n");
	printf(KBLU "Exit Program: " RESET "\"quit\"\n");
	printf(KBLU "List Buffers: " RESET "\"list\"\n");
	printf(KBLU "Show Image in Buffer: " RESET "\"display <buffer-name>\"\n");
	printf(KBLU "Input Image from file: " RESET "\"read <image-name> into <new-buffer-name>\"\n");
	printf(KBLU "Output Image to file: " RESET "\"write <buffer-name> into <new-image-name>\"\n");

	printf(KBLU "Addition: " RESET "\"addition : <new-buffer-name> = <buffer1> + <buffer2>\"\n");
	printf(KBLU "Subtraction: " RESET "\"subtraction : <new-buffer-name> = <buffer1> - <buffer2>\"\n");
	printf(KBLU "Multiplication " RESET "\"multiplication : <new-buffer-name> = <buffer1> * <buffer2>\"\n");
	printf(KBLU "Division: " RESET "\"division : <new-buffer-name> = <buffer1> / <buffer2>\"\n");
	printf(KBLU "Brighten: " RESET "\"brighten <buffer-name> into <new-buffer-name> by <value between 0 and 255>\"\n");
	printf(KBLU "Darken: " RESET "\"darken <buffer-name> into <new-buffer-name> by <value between 0 and 255>\"\n");

	printf(KBLU "Edge Detection: " RESET "\"<horizontal/vertical/combined> <kirsch/prewitt/sobel> <buffer-name> into <new-buffer-name>\"\n");
	printf(KBLU "Histogram Equalization: " RESET "\"histeq <buffer-name> into <new-buffer-name>\"\n");
	printf(KBLU "Flip: " RESET "\"flip <vertical/horizontal> <buffer-name>\"\n");
	printf(KBLU "Rotation: " RESET "\"rotate <buffer-name> by <degrees>\" where degrees exists in (-360, 360) \n");
	printf(KBLU "Blurring: " RESET "\"blurr <buffer> <radius> <sigma>\"\n");	
	printf(KBLU "Sharpen: " RESET "\"sharpen <low/high> <buffer>\"\n\n");
	printf(KBLU "Template: " RESET "\"define_template <template-name> <template-structure> = <(x,y)>\"\n");
	printf(KBLU "Convolve_Template: " RESET "\"convolve_template <buff-name> <template-name>\"\n");
	printf(KBLU "Convolution: " RESET "\"convolve3x3 <template> <buffer-name>\" template = [a b c][d e f][g h i] integers\n");
	printf(KBLU "Grayscale: " RESET "\"grayscale <buffer-name> into <new-buuffer-name>\"\n");
	printf("\n");
	printf(KBLU "Hadamard Transform (WHT): " RESET "\"wht <buffer-name> into <new-buffer-name>\"\n");
	printf(KBLU "Spectrum: " RESET "\"spectrum <buffer-name>\"\n");
	printf(KBLU "Spectrum with log10 Scaling: " RESET "\"spectrum_log10 <buffer-name>\"\n");
	printf(KBLU "Display WHT image: " RESET "\"display_wht <buffer-name>\"\n");
	printf(KBLU "Output WHT image to file: " RESET "\"write_wht <file-name> into <new-image-name>\"\n");
	printf(KBLU "Fast Walsh-Hadamard Transform (FWHT): " RESET "\"fwht <buffer-name> into <new-buffer-name>\"\n");
	printf("\n");
	printf(KBLU "Display all buffer details: " RESET "\"details\"\n");

}


/**
 * Prints a list of all current buffer names, as well as the image names within them.
 * \param buffers List of buffers in memory
 * \param buffCount Number of buffers in memory
 */
void printBuffer(struct buff *buffers, int buffCount)
{
	printf(KGRN "\n----- Buffers -----\n" RESET);
	for (int i = 0; i < buffCount; i++)
	{
		printf("%s " KBLU "contains" RESET " %s\n", buffers[i].name, buffers[i].imageName);
	}
	printf("\n");
}

/**
 * Adds a buffer to the buffer list.
 * \param buffer Buffer to be added to list
 * \param buffers List of buffers in memory
 * \param buffCount Number of buffers in list
 */
void addBuffer(struct buff buffer, struct buff *buffers, int *buffCount)
{
	int k = -1;
	for (int i = 0; i < *buffCount; i++)
	{
		if (strcmp(buffers[i].name, buffer.name) == 0)
		{
			k = i;
			break;
		}
	}
	if (k == -1)
	{
		buffers[*buffCount].img = buffer.img;
		strcpy(buffers[*buffCount].imageName, buffer.imageName);
		strcpy(buffers[*buffCount].name, buffer.name);
		buffers[*buffCount].width = buffer.width;
		buffers[*buffCount].height = buffer.height;
		buffers[*buffCount].channels = buffer.channels;
		buffers[*buffCount].imrgb = buffer.imrgb;
		buffers[*buffCount].isLibgd = buffer.isLibgd;
		buffers[*buffCount].has_wht = buffer.has_wht;
		buffers[*buffCount].wht = buffer.wht;
		buffers[*buffCount].whtimg = buffer.whtimg;
		(*buffCount)++;
	}
	else
	{
		buffers[k].img = buffer.img;
		strcpy(buffers[k].imageName, buffer.imageName);
		strcpy(buffers[k].name, buffer.name);
		buffers[k].width = buffer.width;
		buffers[k].height = buffer.height;
		buffers[k].channels = buffer.channels;
		buffers[k].imrgb = buffer.imrgb;
		buffers[k].isLibgd = buffer.isLibgd;
		buffers[k].has_wht = buffer.has_wht;
		buffers[k].wht = buffer.wht;
		buffers[k].whtimg = buffer.whtimg;
	}

	printf(KYEL "New buffer added\n\n" RESET);
}

/**
 * Searches for a buffer, by name, from the buffer list.
 * \param buffName String name of the buffer to be found
 * \param buffers List of buffers in memory
 * \param int buffCount Number of buffers in list
 *
 * \return requested buffer
 */
struct buff buffSearch(char *buffName, struct buff *buffers, int buffCount)
{
	for (int i = 0; i < buffCount; i++)
	{
		if (strcmp(buffers[i].name, buffName) == 0)
		{
			return buffers[i];
		}
	}
	printf(KRED "Error:" RESET " buffer not found.\n");

	struct buff temp;
	strcpy(temp.status, "false");
	return temp;
}

/**
 * Gets the extension of a requested file, by name.
 * \param filename: String name of file, with extension
 *
 * \return string of file extension
 */
char *get_filename_ext(const char *filename)
{
	char *dot = strrchr(filename, '.');
	if (!dot || dot == filename)
		return "";
	return dot + 1;
}

/**
 * Validates that a file is of acceptable filetype.
 * \param ext String value of file extension
 * \param file_types Pointer to array of accepted filetypes
 *
 * \return 0 if invalid, 1 if valid
 */
int check_types(char *ext, char *file_types[])
{
	for (int i = 0; i < 5; i++)
	{
		int temp = strcmp(ext, file_types[i]);
		if (temp == 0)
			return 1;
	}
	return 0;
}

/**
 * Counts the number of occurenced of `character` in a string. 
 * \param str String to be evaluated
 * \param character Character to count for
 *
 * \return int number of characters
 */
int count_characters(const char* str, char character)
{
	//this is to avoid strtok segfault errors.
	// Count the number of spaces in pCopy and see if it matches the expectated input
	//https://stackoverflow.com/questions/4235519/counting-number-of-occurrences-of-a-char-in-a-string-in-c
	const char* c = str;
	int count = 0;
	
	do {
		if (*c == character)
			count++;
		
	} while (*(c++));

	return count;
}
