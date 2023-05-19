#include <stdio.h>
#include <stdlib.h>

int** pad_image(int** img, int rows, int cols, int padsize){
    // Usually padding size = (filter size - 1)/2 to ensure every pixel in image is at the center of some convolution
    // However this code treats pad and filter size independent
    int rows_pad = rows + 2*padsize;
    int cols_pad = cols + 2*padsize;
    int i, j;

    // memory declaration for padded image
    int** padded_img = (int **)malloc(rows_pad*sizeof(int*));
    for(int k = 0; k< rows_pad; k++){
        padded_img[k] = (int *)malloc(cols_pad*sizeof(int));
    }
    
    
    // initializing padded_img to zero
    for (i = 0; i < rows_pad; i++){
        for(j = 0; j< cols_pad; j++){
            padded_img[i][j] = 0;
        }
    }

    // cp img to padded_img
    for (i = padsize; i < rows_pad - padsize; i++){
        for(j = padsize; j< cols_pad - padsize; j++){
            padded_img[i][j] = img[i - padsize][j - padsize];
        }
    }
    return padded_img;
}


int** conv2D_layer(int **input, int inrows, int incols, int **filter, int size_filter, int stride, int padsize){
    int inrows_pad = inrows + 2*padsize;
    int incols_pad = incols + 2*padsize;
    int **input_padded; // ptr to padded image.

    // padding
    input_padded = pad_image(input, inrows, incols, padsize);

    // convolution (*) => op = img (*) filter 
    
    // output size
    int outrows = (inrows - size_filter + 2*padsize)/stride + 1;
    int outcols = (incols - size_filter + 2*padsize)/stride + 1;
    // printf("%d %d\n", outrows, outcols);

    // allocate memory for output
    int** output = (int**) malloc(outrows * sizeof(int*));
    for(int i = 0; i < outrows; i++) {
        output[i] = (int*) malloc(outcols * sizeof(int));
    }

    int ii, jj, acc;

    //*** convolution loop ***//
    // (i,j) indices for output map
    for(int i=0; i<outrows; i++){
        for(int j=0; j<outcols; j++){
            // compute input map top-left start index pair for op's (i,j)
            ii = i*stride;
            jj = j*stride;

            // if stride clips out of image area

            if((ii > inrows_pad - size_filter) || (jj > incols_pad - size_filter)){
                output[i][j] = 0;
                continue; // go to next pair of (i,j) in loop
            }

            // convolve
            acc=0;
            for(int k=0; k<size_filter; k++){
                for(int l=0; l<size_filter;l++){
                    acc += filter[k][l] * input_padded[ii + k][jj + l];
                }
            }
            output[i][j] = acc;
        }
    }
    return output;
}

void accumulator(int** acc, int** temp, int r, int c){
    for(int i=0; i< r; i++){
        for(int j=0; j<c; j++){
            acc[i][j] += temp[i][j];
        }
    }
}

// Upconvolution Code

int** upsize(int** img, int rows, int cols, int fillval){
    int uprows = 2*rows + 1;
    int upcols = 2*cols + 1;
    int** upimg = (int **)malloc(uprows * sizeof(int*));
    for(int k =0; k< uprows; k++) upimg[k] = (int *)malloc(upcols*sizeof(int));
    
    for(int i=0; i< uprows; i++){
        for(int j=0; j< upcols; j++){
            if((i%2==1) && (j%2==1)) upimg[i][j] = img[(i-1)/2][(j-1)/2];
            else upimg[i][j] = fillval;
        }
    }
}

int** Upconv2D_layer(int **input, int inrows, int incols, int **filter, int size_filter, int stride, int fillval){
    // Upfill : Constant fill strategy (Typically Zero fill)
    int uprows = 2*inrows + 1;
    int upcols = 2*incols + 1;
    int** upimg = (int **)malloc(uprows * sizeof(int*));
    for(int k =0; k< uprows; k++) upimg[k] = (int *)malloc(upcols*sizeof(int));
    
    for(int i=0; i< uprows; i++){
        for(int j=0; j< upcols; j++){
            if((i%2==1) && (j%2==1)) upimg[i][j] = input[(i-1)/2][(j-1)/2];
            else upimg[i][j] = fillval;
        }
    }

    return conv2D_layer(upimg, uprows, upcols, filter, size_filter, stride, 0);
}


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
    int channels, inrows, incols, i, j;
    int** input;

    // read input sizes & declare memory
    fscanf(inFp, "%d %d %d", &channels, &inrows, &incols);
    input = (int **)malloc(inrows*sizeof(int*));
    for(int k =0; k< inrows;k++){
        input[k] = (int *)malloc(incols*sizeof(int));
    }

    // Read weights sizes & declare memory
    FILE* w1fp = fopen("weights1.txt","rb");
    int channels2, wsize;
    int** filter1;

    fscanf(w1fp,"%d %d", &channels2, &wsize);
    filter1 = (int **)malloc(wsize*sizeof(int*));
    for(int k =0; k< wsize; k++) filter1[k] = (int *)malloc(wsize*sizeof(int));

    // convolution parameters
    int stride, padsize;
    stride = 1;
    padsize = 1;

    // Output pointer and size var declaration
    int** op1;  // ptr to total conv op
    int** opc;  // ptr to hold op computed for conv of a channel in loop

    if(channels != channels2) {printf("Filter not compatible to input\n"); return 1;}

    // 1st channel:
    // read 1st channel ip
    for(i=0; i< inrows; i++){
        for(j=0; j<incols; j++){
            fscanf(inFp, "%d", &input[i][j]);
        }
    }

    // for(i=0; i<inrows;i++){
    //     for(j=0; j<incols;j++){
    //         printf("%d ",input[i][j]);
    //     }
    // }

    // read 1st channel filter into array filter1
    for(i=0; i<wsize;i++){
        for(j=0; j<wsize;j++){
            fscanf(w1fp, "%d", &filter1[i][j]);
        }
    }
    
    // // convolution call
    // op1 = conv2D_layer(input, inrows, incols, filter1, wsize, stride, padsize);
    // // outsize
    // int outrows = (inrows - wsize + 2*padsize)/stride + 1;
    // int outcols = (incols - wsize + 2*padsize)/stride + 1;

    // Upconvolution call
    op1 = Upconv2D_layer(input, inrows, incols, filter1, wsize, stride, 0); // fill value = 0;
    int outrows = (2* inrows + 1 - wsize)/stride + 1;
    int outcols = (2* incols + 1 - wsize)/stride + 1;


    
    
    // printf("%d %d", outrows, outcols);

    // Iterate over channels:
    for(int c=1; c< channels; c++){
        // read c-th channel input
        for(i=0; i< inrows; i++){
            for(j=0; j<incols; j++){
                fscanf(inFp, "%d", &input[i][j]);
            }
        }    

        // read c-th channel filter into array filter1
        for(i=0; i<wsize;i++){
            for(j=0; j<wsize;j++){
                fscanf(w1fp, "%d", &filter1[i][j]);
            }
        }

        // // convolution call
        // opc = conv2D_layer(input, inrows, incols, filter1, wsize, stride, padsize);

        // Upconvolution call
        opc = Upconv2D_layer(input, inrows, incols, filter1, wsize, stride, 0); // fill value = 0;

        // accumulate opc to op1
        accumulator(op1, opc, outrows, outcols);
    }

    // close the input file ptr
    fclose(inFp);
    fclose(w1fp);

    // printing into file
    fprintf(outFp,"%d %d\n",outrows, outcols);
    for(int i =0; i< outrows; i++){
        for(int j=0; j<outcols; j++){
            fprintf(outFp,"%d ",op1[i][j]);
        }
        fprintf(outFp,"\n");
    } 

    fclose(outFp); 
    free(input);
    free(filter1);
    free(op1);
    // free(opc); 
    return 0;
}




