#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <setjmp.h>
#include "gd.h"

struct buff
{
	char imageName[32];
	char name[30];
	char status[30];
	unsigned char *img;
	int width, height, channels;
	gdImagePtr imrgb;
	int isLibgd;
};

struct template
{
	char name[30];
	float** mask;
	float** mask_tp; // modified mask for target pixel placement
	int tpRow;
	int tpColumn;
};

#include "read.c"
#include "write.c"
#include "arithmetics.c"
#include "brighten.c"
#include "edge.c"
#include "hadamard.c"
#include "pad.c"
#include "display.c"
#include "histoEQ.c"
#include "rotation.c"
#include "blurr.c"
#include "sharpen.c"
#define KGRN "\x1B[32m"
#define KYEL "\x1B[33m"
#define KBLU "\x1B[34m"
#define KRED "\x1B[31m"
#define KMAG "\x1B[35m"
#define RESET "\x1B[0m"

void addBuffer(struct buff buffer, struct buff *buffers, int *buffCount);
struct buff buffSearch(char *buffName, struct buff *buffers, int buffCount);
void printBuffer(struct buff *buffer, int buffCount);
void printMenu();
char *get_filename_ext(const char *filename);
int check_types(char *ext, char *file_types[]);

int main(int argc, char **argv)
{
	printf(KGRN "Welcome to the UNIX Image Manipulation tool.\n");
	printf("Type \"" KYEL "menu" KGRN "\"  to view the list of commands or \"" KYEL "list" KGRN "\" to view your buffers.\n\n" RESET);

	struct buff buffers[10];
	int buffCount = 0;

	struct template T;

	char p[100];
	gets(p);

	while (1)
	{
		char *command;
		char *imageName;
		char *buffName;
		char *amount;

		command = strtok(p, " ");

		if (strcmp(command, "menu") == 0)
		{
			printMenu();
		}
		else if (strcmp(command, "read") == 0)
		{
			imageName = strtok(NULL, " ");
			if (strlen(imageName) > 14)
			{
				printf(KRED "Error: " RESET "Image name too long. Image + extension must be shorter than 14 characters.\n");
			}
			else
			{
				char *ext = get_filename_ext(imageName);
				char *file_types[4] = {"jpeg", "gif", "tiff", "png"}; //ALLOW NEW FILE TYPES HERE
				int approved = check_types(ext, file_types);
				if (approved != 1)
				{
					printf(KRED "Error: " RESET "Image file type is not approved.\n");
					exit(1);
				}
				strtok(NULL, " ");
				buffName = strtok(NULL, " ");
				struct buff temp = readToBuff(imageName, buffName);
				addBuffer(temp, buffers, &buffCount);
			}
		}
		else if (strcmp(command, "write") == 0)
		{
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
				writeToImage(buffSearch(buffName, buffers, buffCount), imageName);
				printf(KYEL "Done writing!\n\n" RESET);
			}
		}
		else if (strcmp(command, "list") == 0)
		{
			printBuffer(buffers, buffCount);
		}
		else if (strcmp(command, "brighten") == 0)
		{
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
		else if (strcmp(command, "hadamard") == 0)
		{
			buffName = strtok(NULL, " ");
			strtok(NULL, " ");
			imageName = strtok(NULL, " ");
			if (strlen(imageName) > 14)
			{
				printf(KRED "Error: " RESET "Image name too long. Image + extension must be shorter than 14 characters.\n\n");
			}
			else
			{
				
				addBuffer(hadamard(buffSearch(buffName, buffers, buffCount), imageName), buffers, &buffCount);
			}
		}
		else if (strcmp(command, "pad") == 0)
		{
			buffName = strtok(NULL, " ");
			strtok(NULL, " ");
			imageName = strtok(NULL, " ");
			if (strlen(imageName) > 14)
			{
				printf(KRED "Error: " RESET "Image name too long. Image + extension must be shorter than 14 characters.\n\n");
			}
			else
			{
				
				addBuffer(pad(buffSearch(buffName, buffers, buffCount), imageName), buffers, &buffCount);
			}
		}
		else if (strcmp(command, "darken") == 0)
		{
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
			buffName = strtok(NULL, " ");
			char tem[10] = "temp.png";

			struct buff temp = buffSearch(buffName, buffers, buffCount);
			if (strcmp(temp.status, "false") != 0)
			{
				writeToImage(temp, tem);
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
			buffName = strtok(NULL, " ");
			char* radius = strtok(NULL, " ");
			char* sigma = strtok(NULL, " ");
			struct buff temp = blurr(buffSearch(buffName, buffers, buffCount), atoi(radius), atof(sigma));
			addBuffer(temp, buffers, &buffCount);
		}
		else if((strcmp(command, "sharpen") == 0))
		{
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
		else											
		{												
			printf(KRED "Error: " RESET "Command not found or not supported, please type menu for list of commands.\n");
		}

		gets(p);
	}
}

void printMenu()
{
	printf(KGRN "\n----- Commands -----\n");
	printf(KBLU "Exit Program: " RESET "\"quit\"\n");
	printf(KBLU "List Buffers: " RESET "\"list\"\n");
	printf(KBLU "Show Image in Buffer: " RESET "\"display <buffer-name>\"\n");
	printf(KBLU "Input Image: " RESET "\"read <image-name> into <new-buffer-name>\"\n");
	printf(KBLU "Output Image: " RESET "\"write <buffer-name> into <new-image-name>\"\n");

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
	printf(KBLU "Template: " RESET "\"define_template <template-name> <template-structure> = <(x,y)>\"\n");
	printf(KBLU "Convolve_Template: " RESET "\"convolve_template <buff-name> <template-name>\"\n");
	printf(KBLU "Convolution: " RESET "\"convolve3x3 <template> <buffer-name>\" template = [a b c][d e f][g h i] integers\n");
	printf(KBLU "Sharpen: " RESET "\"sharpen <low/high> <buffer>\"\n\n");
	printf(KBLU "Pad: " RESET "\"pad <buffer> into <new-buffer-name>\"\n\n");

}

void printBuffer(struct buff *buffers, int buffCount)
{
	printf(KGRN "\n----- Buffers -----\n" RESET);
	for (int i = 0; i < buffCount; i++)
	{
		printf("%s " KBLU "contains" RESET " %s\n", buffers[i].name, buffers[i].imageName);
	}
	printf("\n");
}

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
	}

	printf(KYEL "New buffer added\n\n" RESET);
}

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

char *get_filename_ext(const char *filename)
{
	char *dot = strrchr(filename, '.');
	if (!dot || dot == filename)
		return "";
	return dot + 1;
}
int check_types(char *ext, char *file_types[])
{
	for (int i = 0; i < 4; i++)
	{
		int temp = strcmp(ext, file_types[i]);
		if (temp == 0)
			return 1;
	}
	return 0;
}
