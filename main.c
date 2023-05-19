#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]){
    char *inFile = argv[1];
    char *outFile = argv[2];
    FILE *inFp, *outFp;
    // File pointer for file containing input data
	inFp = fopen(inFile, "rb");
	if (inFp == NULL)
	{
		printf("\n We have null pointer \n");
	}

    // file ptr to write output
	outFp = fopen(outFile, "wb");
	if (outFp == NULL)
	{
		printf("\n We have null pointer \n");
	}

    // Input and size vars declaration
    int inrows, incols, i, j;
    int** input;

    // read in sizes
    fscanf(inFp, "%d %d", &inrows, &incols);
    input = (int **)malloc(inrows*sizeof(int*));
    for(int k =0; k< inrows;k++){
        input[k] = (int *)malloc(incols*sizeof(int));
    }

    // read in data
    for(i=0; i< inrows; i++){
        for(j=0; j<incols; j++){
            fscanf(inFp, "%d", &input[i][j]);
        }
    }

    // close the input file ptr
    fclose(inFp);

}