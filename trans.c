/* 
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */ 
#include <stdio.h>
#include "cachelab.h"

// Names: Chris Couto and Adam Beckwith  
// ID #1: ccouto@cs.hmc.edu
// ID #2: abeckwith@cs.hmc.edu

int is_transpose(int M, int N, int A[N][M], int B[M][N]);

/* 
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started. 
 */ 

/* 
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            B[j][i] = A[i][j];
        }
    }    
}

/**
    * \brief Transposes between A and B arrays in 8x8 blocks, mindful of diagonal entries 
    * \param startX  X-value for top left int in 8x8 square
    * \param startY  Y-value for top left int in 8x8 square
    * \param N       Number of rows in A (number of columns in transposed B)
    * \param M       Number of columns in A (number of rows in transposed B)
    * \param A       Original matrix
    * \param B       Transposed matrix
**/
void scan_box_8x8(int startX, int startY, int N, int M, int A[N][M], int B[M][N]) {
    int i, j, temp;
    // goes through all ints in the 8x8 block
    for (j = startY; j < startY + 8; ++j) {
        for (i = startX; i < startX + 8; ++i) {
            // checks if int is on diagonal
            if (i != j) {
                B[i][j] = A[j][i];
            } else {
                temp = A[j][i];
            }  
        }
        // confirm we have a diagonal entry to transpose
        if (startY == startX) {
            B[j][j] = temp;
        }
    }
}

/**
    * \brief Transposes between A and B arrays in 8x8 blocks, mindful of diagonal entries 
    *        Identical to scan_box_8x8 except it checks whether the entry exists in the matrix before transposing
    * \param startX  X-value for top left int in 8x8 square
    * \param startY  Y-value for top left int in 8x8 square
    * \param N       Number of rows in A (number of columns in transposed B)
    * \param M       Number of columns in A (number of rows in transposed B)
    * \param A       Original matrix
    * \param B       Transposed matrix
**/
void scan_box_8x8_check(int startX, int startY, int N, int M, int A[N][M], int B[M][N]) {
    int i, j, temp;
    // goes through all ints in the 8x8 block
    for (j = startY; j < startY + 8; ++j) {
        for (i = startX; i < startX + 8; ++i) {
            // only transposes if the element is present in A
            if (i < M && j < N) { 
                // checks if int is on diagonal
                if (i != j) {
                    B[i][j] = A[j][i];
                } else {
                    temp = A[j][i];
                }  
            }
        }
        // confirm we have a diagonal entry to transpose
        if (startY == startX) {
            B[j][j] = temp;
        }
    }
}

/**
    * \brief Chooses starting points for 8x8 blocks and passes them to helper that carries out transposing
    *        Designed for 32x32 matrix
    * \param N       Number of rows in A (number of columns in transposed B)
    * \param M       Number of columns in A (number of rows in transposed B)
    * \param A       Original matrix
    * \param B       Transposed matrix
**/
char transpose_32x32_desc[] = "Does 32x32 matrix with 8x8 blocks and adjustments for diagonals";
void transpose_32x32(int M, int N, int A[N][M], int B[M][N])
{
    int x, y;
        // obtain all top-left positions for 8x8 blocks and pass them to helper
        for (x = 0; x < M; x+=8) {
            for (y = 0; y < N; y+=8) {
                scan_box_8x8(x, y, N, M, A, B);
            }
        }
    }

/**
    * \brief Chooses starting points for 8x8 blocks and passes them to helper that carries out transposing
    *        Designed for 61x67 matrix
    * \param N       Number of rows in A (number of columns in transposed B)
    * \param M       Number of columns in A (number of rows in transposed B)
    * \param A       Original matrix
    * \param B       Transposed matrix
**/
char transpose_61x67_desc[] = "Doing 61x67";
void transpose_61x67(int M, int N, int A[N][M], int B[M][N])
{
    int x, y;
    // obtain all top-left positions for 8x8 blocks and pass them to helper
    for (x = 0; x < M; x+=8) { 
        for (y = 0; y < N; y+=8) {
            scan_box_8x8_check(x, y, N, M, A, B);
        }
    }

}

/* 
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded. 
 */
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
    if (M == 32 && N == 32) {
        transpose_32x32(M, N, A, B);
    }
    if (M == 61 && N == 67) {
        transpose_61x67(M, N, A, B);
    }
    if (M == 64 && N == 64) {
        transpose_32x32(M, N, A, B);
    }

}

/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions()
{
    /* Register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc); 

    /* Register any additional transpose functions */
    registerTransFunction(trans, trans_desc); 

    registerTransFunction(transpose_32x32, transpose_32x32_desc);

    registerTransFunction(transpose_61x67, transpose_61x67_desc);

}

/* 
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; ++j) {
            if (A[i][j] != B[j][i]) {
                return 0;
            }
        }
    }
    return 1;
}
