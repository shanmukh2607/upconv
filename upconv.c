#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define INROWS 16
#define INCOLS 16
#define CHANNELS 3

#define NUM_FILTERS 16
#define WSIZE 3

#define STRIDE 1

#define UPROWS 2*INROWS + 1
#define UPCOLS 2*INCOLS + 1
#define OUTROWS (2*INROWS + 1 - WSIZE)/STRIDE + 1
#define OUTCOLS (2*INCOLS + 1 - WSIZE)/STRIDE + 1

int zero_multiplier_count = 0;
int count = 0;

void conv2D_layer(int output[][OUTCOLS],int input[][UPCOLS], int inrows, int incols, int filter[WSIZE][WSIZE], int size_filter, int stride, int padsize){
    // convolution (*) => op = img (*) filter 
    
    // output size
    int outrows = (inrows - size_filter + 2*padsize)/stride + 1;
    int outcols = (incols - size_filter + 2*padsize)/stride + 1;
    // printf("%d %d\n", outrows, outcols);

    int ii, jj, acc;

    //*** convolution loop ***//
    // (i,j) indices for output map
    for(int i=0; i<outrows; i++){
        for(int j=0; j<outcols; j++){
            // compute input map top-left start index pair for op's (i,j)
            ii = i*stride;
            jj = j*stride;

            // if stride clips out of image area

            if((ii > inrows - size_filter) || (jj > incols - size_filter)){
                output[i][j] = 0;
                continue; // go to next pair of (i,j) in loop
            }

            // convolve
            acc=0;
            for(int k=0; k<size_filter; k++){
                for(int l=0; l<size_filter;l++){
                    if(input[ii + k][jj + l] == 0) zero_multiplier_count += 1;
                    acc += filter[k][l] * input[ii + k][jj + l];
                    count += 1;
                }
            }
            output[i][j] = acc;
        }
    }
}

void accumulator(int acc[][OUTCOLS] , int temp[][OUTCOLS]){
    for(int i=0; i< OUTROWS; i++){
        for(int j=0; j<OUTCOLS; j++){
            acc[i][j] += temp[i][j];
        }
    }
}

// Upconvolution Code

void Upconv2D_layer(int output[][OUTCOLS], int input[][INCOLS], int inrows, int incols, int filter[WSIZE][WSIZE], int size_filter, int stride, int fillval){
    // Upfill : Constant fill strategy (Typically Zero fill)
    int upimg[UPROWS][UPCOLS];

    
    for(int i=0; i< UPROWS; i++){
        for(int j=0; j< UPCOLS; j++){
            if((i%2==1) && (j%2==1)) upimg[i][j] = input[(i-1)/2][(j-1)/2];
            else upimg[i][j] = fillval;
        }
    }

    conv2D_layer(output, upimg, UPROWS, UPCOLS, filter, size_filter, stride, 0);
}


int main(int argc, char *argv[]){

    // volatile uint64_t *time = 0xBFF8;   // addr of of reg in ShaktiC that stores clk cycles

    // volatile uint64_t start1 = *time;

    int input[CHANNELS][INROWS][INCOLS] = {{{-77 ,30 ,-13 ,-59 ,0 ,43 ,89 ,-44 ,-67 ,-105 ,75 ,32 ,110 ,8 ,-115 ,5} ,{72 ,-6 ,-106 ,30 ,-102 ,-79 ,12 ,1 ,9 ,104 ,67 ,63 ,69 ,64 ,-127 ,89} ,{-68 ,-36 ,21 ,35 ,-25 ,-58 ,-22 ,-111 ,33 ,108 ,14 ,-76 ,-91 ,37 ,25 ,24} ,{-73 ,-68 ,-26 ,13 ,87 ,27 ,24 ,20 ,54 ,-104 ,-115 ,9 ,-12 ,17 ,-29 ,41} ,{39 ,-50 ,-52 ,50 ,32 ,22 ,25 ,-11 ,7 ,-45 ,82 ,-86 ,-33 ,-126 ,-113 ,76} ,{20 ,51 ,-35 ,-76 ,-11 ,-87 ,82 ,-113 ,-106 ,103 ,-90 ,-91 ,-43 ,-92 ,-57 ,-37} ,{-80 ,-68 ,-82 ,-88 ,88 ,60 ,-21 ,-127 ,10 ,-5 ,-20 ,34 ,-47 ,-51 ,-120 ,-69} ,{29 ,-37 ,-31 ,-109 ,123 ,106 ,87 ,63 ,-90 ,-76 ,-119 ,-50 ,81 ,-67 ,-80 ,-85} ,{100 ,-51 ,6 ,-22 ,12 ,119 ,6 ,-91 ,-9 ,23 ,21 ,-70 ,29 ,-103 ,119 ,-24} ,{84 ,-26 ,121 ,-93 ,117 ,54 ,-86 ,-108 ,-16 ,-90 ,124 ,9 ,-116 ,-118 ,-28 ,33} ,{-97 ,-94 ,99 ,-95 ,-66 ,124 ,5 ,66 ,78 ,89 ,-93 ,-101 ,3 ,37 ,-7 ,-107} ,{-84 ,18 ,-28 ,43 ,-103 ,19 ,-22 ,94 ,115 ,15 ,-24 ,91 ,-102 ,-58 ,-12 ,55} ,{-78 ,52 ,57 ,23 ,-57 ,121 ,-87 ,-54 ,-17 ,-12 ,1 ,64 ,-40 ,-37 ,124 ,97} ,{87 ,67 ,74 ,109 ,-80 ,-44 ,-90 ,80 ,114 ,86 ,-80 ,94 ,9 ,73 ,17 ,82} ,{-88 ,35 ,-62 ,-32 ,-114 ,-8 ,-84 ,125 ,-28 ,65 ,66 ,-48 ,79 ,38 ,5 ,27} ,{-92 ,43 ,56 ,28 ,-56 ,-56 ,-43 ,52 ,59 ,107 ,-82 ,-83 ,92 ,25 ,-45 ,-32} },
{{-55 ,-62 ,10 ,94 ,62 ,-91 ,-101 ,-69 ,-31 ,99 ,-96 ,124 ,109 ,18 ,76 ,73} ,{69 ,-59 ,-12 ,-94 ,-114 ,-89 ,-94 ,116 ,58 ,123 ,52 ,-6 ,80 ,-31 ,106 ,-121} ,{98 ,42 ,51 ,104 ,-115 ,5 ,76 ,52 ,71 ,100 ,-95 ,42 ,-83 ,-18 ,81 ,-32} ,{-56 ,107 ,43 ,-86 ,-118 ,-118 ,-56 ,75 ,57 ,113 ,-119 ,-91 ,125 ,125 ,-78 ,-40} ,{-113 ,63 ,96 ,125 ,-58 ,-67 ,82 ,29 ,91 ,-8 ,11 ,-110 ,76 ,82 ,87 ,-123} ,{-40 ,13 ,-100 ,-58 ,-27 ,115 ,-12 ,1 ,20 ,87 ,-62 ,-83 ,-46 ,-99 ,127 ,7} ,{122 ,-96 ,-34 ,-77 ,115 ,-14 ,113 ,47 ,-9 ,-103 ,-107 ,-96 ,-42 ,72 ,-106 ,-112} ,{5 ,93 ,43 ,-30 ,27 ,-77 ,68 ,25 ,-48 ,44 ,-44 ,78 ,124 ,-93 ,22 ,67} ,{70 ,67 ,1 ,53 ,53 ,35 ,83 ,-123 ,-124 ,-15 ,-60 ,-104 ,116 ,60 ,-100 ,107} ,{-123 ,-61 ,-82 ,-77 ,-30 ,-6 ,-19 ,-54 ,109 ,-17 ,-119 ,-61 ,22 ,-100 ,100 ,-108} ,{-23 ,7 ,-65 ,-82 ,-105 ,-54 ,98 ,-91 ,-88 ,-90 ,-119 ,-36 ,6 ,-97 ,57 ,102} ,{7 ,-18 ,-43 ,100 ,-127 ,73 ,-83 ,120 ,55 ,16 ,-107 ,56 ,93 ,34 ,109 ,-76} ,{-28 ,110 ,-123 ,77 ,-94 ,-3 ,-121 ,121 ,-49 ,115 ,-124 ,-50 ,9 ,84 ,12 ,9} ,{98 ,-32 ,45 ,69 ,-9 ,49 ,58 ,96 ,94 ,46 ,-36 ,-103 ,73 ,-116 ,12 ,-22} ,{-70 ,40 ,109 ,47 ,-70 ,102 ,60 ,59 ,-6 ,0 ,-48 ,-76 ,-116 ,119 ,78 ,115} ,{-122 ,-88 ,122 ,24 ,-19 ,-93 ,-8 ,57 ,125 ,-36 ,81 ,-122 ,19 ,48 ,124 ,-91} },
{{24 ,108 ,-71 ,10 ,63 ,14 ,26 ,115 ,-103 ,72 ,-124 ,-113 ,69 ,47 ,4 ,-84} ,{0 ,25 ,62 ,-100 ,-128 ,118 ,34 ,15 ,73 ,-126 ,69 ,30 ,-12 ,60 ,111 ,115} ,{-91 ,46 ,-58 ,127 ,103 ,-19 ,-118 ,2 ,-24 ,-99 ,55 ,49 ,97 ,-84 ,28 ,66} ,{109 ,21 ,30 ,21 ,71 ,-82 ,72 ,-96 ,-9 ,81 ,-76 ,-5 ,-88 ,-61 ,-112 ,43} ,{91 ,-57 ,94 ,-79 ,98 ,23 ,-18 ,120 ,-64 ,73 ,17 ,6 ,-96 ,-47 ,20 ,58} ,{126 ,89 ,51 ,-11 ,20 ,-92 ,104 ,-122 ,-55 ,79 ,-59 ,-10 ,77 ,-11 ,8 ,-127} ,{-116 ,-21 ,39 ,-17 ,-123 ,-99 ,-4 ,91 ,-2 ,-14 ,118 ,98 ,-25 ,-27 ,51 ,-119} ,{-41 ,72 ,26 ,4 ,-93 ,-33 ,-71 ,-26 ,-107 ,-82 ,-7 ,-90 ,-99 ,-43 ,-121 ,82} ,{-41 ,-54 ,-3 ,-89 ,-107 ,5 ,-69 ,-100 ,-79 ,90 ,41 ,-83 ,-81 ,-116 ,103 ,0} ,{87 ,-85 ,82 ,33 ,28 ,17 ,-126 ,-78 ,62 ,42 ,82 ,-103 ,5 ,-54 ,-6 ,-13} ,{58 ,-50 ,-9 ,-53 ,-53 ,-33 ,-40 ,-13 ,-115 ,-48 ,10 ,118 ,-97 ,85 ,-8 ,17} ,{-28 ,63 ,87 ,120 ,103 ,-112 ,-47 ,-61 ,73 ,43 ,124 ,37 ,-14 ,65 ,-91 ,-2} ,{-68 ,-65 ,34 ,-122 ,-82 ,122 ,-73 ,-86 ,-67 ,-9 ,-86 ,51 ,-69 ,90 ,30 ,114} ,{69 ,64 ,54 ,-50 ,32 ,-31 ,-13 ,47 ,-84 ,40 ,-118 ,98 ,87 ,22 ,36 ,52} ,{-36 ,91 ,13 ,-95 ,-94 ,-101 ,18 ,-5 ,-94 ,42 ,81 ,115 ,-52 ,60 ,27 ,56} ,{32 ,-105 ,102 ,87 ,11 ,77 ,12 ,69 ,86 ,-79 ,-115 ,-41 ,69 ,-124 ,-17 ,-50} }
};
    // volatile uint64_t end1 = *time;

    // printf("Input Initialization : %ld",(end1-start1)); // Or alternate way to print it out
    

    // volatile uint64_t start2 = *time;
    int filter1[NUM_FILTERS][CHANNELS][WSIZE][WSIZE] = 
{{{{-8 ,-6 ,-2 } ,{-6 ,-3 ,-5 } ,{4 ,-1 ,6 } },{{4 ,1 ,5 } ,{7 ,-3 ,2 } ,{-5 ,-7 ,4 } },{{6 ,-8 ,3 } ,{3 ,2 ,-4 } ,{6 ,-7 ,1 } }},
{{{-6 ,2 ,-4 } ,{6 ,6 ,-1 } ,{1 ,6 ,2 } },{{1 ,2 ,-8 } ,{-6 ,2 ,-5 } ,{6 ,-8 ,-2 } },{{-5 ,-3 ,-4 } ,{3 ,-1 ,-2 } ,{-2 ,7 ,-1 } }},
{{{2 ,-6 ,1 } ,{-4 ,1 ,-8 } ,{-7 ,4 ,3 } },{{1 ,4 ,3 } ,{-8 ,-1 ,3 } ,{-2 ,0 ,0 } },{{4 ,6 ,6 } ,{-6 ,-1 ,2 } ,{1 ,-7 ,1 } }},
{{{0 ,2 ,-5 } ,{6 ,-3 ,2 } ,{4 ,-4 ,-1 } },{{5 ,-8 ,-6 } ,{1 ,-8 ,0 } ,{3 ,-1 ,0 } },{{-3 ,-7 ,1 } ,{5 ,-3 ,-1 } ,{-4 ,0 ,1 } }},
{{{3 ,6 ,-2 } ,{6 ,-6 ,-3 } ,{-8 ,0 ,-3 } },{{-7 ,-4 ,-2 } ,{-6 ,-8 ,6 } ,{7 ,-7 ,6 } },{{1 ,-2 ,-3 } ,{4 ,-7 ,-3 } ,{-4 ,6 ,-7 } }},
{{{-4 ,-3 ,3 } ,{-2 ,6 ,-2 } ,{-3 ,-4 ,7 } },{{-3 ,-4 ,-1 } ,{4 ,5 ,-2 } ,{-6 ,5 ,0 } },{{-8 ,-1 ,-4 } ,{1 ,-7 ,1 } ,{1 ,-7 ,7 } }},
{{{4 ,-5 ,3 } ,{5 ,-3 ,-5 } ,{-2 ,-6 ,4 } },{{-2 ,4 ,0 } ,{-3 ,7 ,7 } ,{-8 ,-3 ,3 } },{{-5 ,5 ,-6 } ,{-6 ,-8 ,-4 } ,{-7 ,-2 ,-2 } }},
{{{-2 ,2 ,-6 } ,{2 ,-3 ,-8 } ,{2 ,2 ,6 } },{{-2 ,6 ,-7 } ,{-8 ,-3 ,4 } ,{7 ,6 ,-8 } },{{-3 ,-1 ,4 } ,{-8 ,4 ,6 } ,{3 ,-1 ,1 } }},
{{{-3 ,-4 ,-4 } ,{-3 ,7 ,7 } ,{-5 ,-1 ,-5 } },{{7 ,-8 ,-5 } ,{-3 ,-2 ,6 } ,{1 ,5 ,-2 } },{{-1 ,-6 ,-7 } ,{4 ,-3 ,4 } ,{2 ,-8 ,-7 } }},
{{{-4 ,5 ,-7 } ,{-1 ,-5 ,-2 } ,{-7 ,-4 ,7 } },{{2 ,-2 ,4 } ,{4 ,6 ,-8 } ,{6 ,0 ,4 } },{{4 ,1 ,-6 } ,{-6 ,7 ,-3 } ,{4 ,2 ,1 } }},
{{{-4 ,2 ,5 } ,{7 ,-8 ,2 } ,{-6 ,1 ,5 } },{{-6 ,0 ,3 } ,{7 ,1 ,5 } ,{1 ,2 ,7 } },{{5 ,-4 ,-3 } ,{1 ,4 ,6 } ,{5 ,-3 ,-6 } }},
{{{-5 ,-1 ,-2 } ,{-3 ,1 ,0 } ,{-3 ,1 ,0 } },{{-8 ,2 ,7 } ,{-5 ,4 ,4 } ,{-5 ,3 ,2 } },{{2 ,4 ,-2 } ,{6 ,3 ,-3 } ,{0 ,-4 ,0 } }},
{{{-8 ,7 ,-3 } ,{-7 ,-4 ,3 } ,{-2 ,0 ,1 } },{{-3 ,-5 ,1 } ,{7 ,-7 ,0 } ,{5 ,-1 ,-6 } },{{3 ,-7 ,4 } ,{0 ,-4 ,-8 } ,{7 ,0 ,-7 } }},
{{{-2 ,4 ,1 } ,{-4 ,-2 ,-4 } ,{-8 ,1 ,-8 } },{{2 ,7 ,2 } ,{1 ,3 ,7 } ,{-2 ,-6 ,0 } },{{6 ,3 ,0 } ,{0 ,0 ,1 } ,{0 ,7 ,6 } }},
{{{-4 ,-3 ,-5 } ,{5 ,7 ,3 } ,{-6 ,1 ,0 } },{{-3 ,-1 ,6 } ,{5 ,1 ,-8 } ,{7 ,-7 ,-3 } },{{-1 ,0 ,-2 } ,{-7 ,5 ,-8 } ,{-4 ,6 ,2 } }},
{{{-7 ,6 ,1 } ,{-2 ,7 ,-3 } ,{-3 ,0 ,-8 } },{{-5 ,-5 ,3 } ,{4 ,1 ,6 } ,{3 ,-4 ,-6 } },{{-7 ,2 ,-3 } ,{4 ,-3 ,-5 } ,{-3 ,5 ,-4 } }}
};
    // volatile uint64_t end2 = *time;
    // printf("Weight Declaration: %ld",(end2-start2)); // Or alternate way to print it out


    // Output pointer and size var declaration
    int op1[NUM_FILTERS][OUTROWS][OUTCOLS];  // total conv op
    int opc[OUTROWS][OUTCOLS];  // ptr to hold op computed for conv of a channel in loop
    int outrows = (2* INROWS + 1 - WSIZE)/STRIDE + 1;
    int outcols = (2* INCOLS + 1 - WSIZE)/STRIDE + 1;


    // volatile uint64_t start3 = *time;       // Upconv computation time
    // Upconvolution call

    for(int nf = 0; nf< NUM_FILTERS; nf++){
        Upconv2D_layer(op1[nf], input[0], INROWS, INCOLS, filter1[nf][0], WSIZE, STRIDE, 0); // fill value = 0;

        // Iterate over channels:
        for(int c=1; c< CHANNELS; c++){


            // Upconvolution call
            Upconv2D_layer(opc, input[c], INROWS, INCOLS, filter1[nf][c], WSIZE, STRIDE, 0); // fill value = 0;

            // accumulate opc to op1
            accumulator(op1[nf], opc);
        }

    }
    printf("Redundant muls: %d\n",zero_multiplier_count);
    printf("Total muls: %d\n", count);
    //  volatile uint64_t end3 = *time;
    // printf("Upconv Computation : %ld",(end3-start3)); // Or alternate way to print it out

    // for(int c=0; c< NUM_FILTERS; c++){
    //     for(int i=0; i< outrows; i++){
    //         for(int j =0; j< outcols; j++){
    //             printf("%d ",op1[c][i][j]);
    //         }
    //         printf("\n");
    //     }
    //     printf("\n");
    // }

    // printf("Total cycles : %ld",(end3 - start1));       // Total cycles

    return 0;
}




