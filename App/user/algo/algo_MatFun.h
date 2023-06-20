#ifndef ALGO_MAT_FUN_H_
#define ALGO_MAT_FUN_H_

/*!!!The matrix function library is based on the previous work done by Jiachi Zou as shown below!!!!!*/

/*
 * Light Matrix: C code implementation for basic matrix operation
 *
 * Copyright (C) 2017 Jiachi Zou
 *
 * This code is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This code is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with code.  If not, see <http:#www.gnu.org/licenses/>.
 */

 /*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

 /*
 Copyright (C) 2018 Neil

 Modifications is made by Neil and renamed as "MatFun".

 -The matrix are 0 based which complies with the C convention.
 -The type of the matrix is float throughout the library. To use other types, 
  modifications shall be made.
  -Vectors are treated as 1 dimension matrix(row vector or column vector)

 More functions are added to make the library versatile. In order to improve 
 the quality and functionality of the code furtherly, team effort is essential, 
 hence every programmer is welcome to modify, rewrite the functions or add new ones.

 */

#ifdef __cplusplus
extern "C" {
#endif
	
//uncomment it for error handling
//#define MAT_LEGAL_CHECKING 

//to use anonymous unions
#pragma anon_unions

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include "FreeRTOS.h"



#ifdef DOUBLE_MAT//double matrix
typedef double DBL_or_FLT;
#else
typedef float DBL_or_FLT;
#endif // DOUBLE_MAT

//create a double/float matrix(structure)
typedef struct {
	unsigned short row, col, numel;
	DBL_or_FLT **element;
}Mat;


//Here is an alternative way of defining the matrix of multiple data types
typedef struct
{
	unsigned short row, col, numel;
	union
	{
		double **element_DBL;
		float **element_FLT;
	};//anonymous union
}Mat_Alternative;


/*---------------------------Matrix functions---------------------------------------*/
/*
  The type-specifier Mat* is used through the function signatures because a pointer(-to-Mat) is required to modify the value
  The type-qualifier "const" is used to indicates the value of the corresponding argument shall not be altered
  
  **************NOMENCLATURE********************
  mat: a matrix
  row: the total number of rows
  col: the total number of columns
  row_i: the ith row(0-based index); 
  column_i: the ith column(0-based index)
  input_N: the Nth input
  output: the output
  num: a scalar
  w: weight
  dim: along the dimension
  **********************************************
  
  */
//Part 1: declaration & definition-------------------------------------------------------------------------------------------------------
Mat* MatCreate(Mat* mat, int row, int col);//allocate the memory for the matrix
void MatDelete(Mat* mat);//deallocates the memory previously allocated
//template <class T> //it causes an error, why?
Mat* MatSetVal(Mat* mat, const float* val);//set float value in "val" array to the matrix
Mat* MatSetVal_DBL(Mat* mat, const double* val);//(OVERLOADED FUNCTION)set double value in "val" array to the matrix
void MatDisp(const Mat* mat);//display the matrix
//---------------------------------------------------------------------------------------------------------------------------------------


//Part 2: special matrix-----------------------------------------------------------------------------------------------------------------
Mat* MatZeros(Mat* mat);//mat = zeros(r,c)*, set all the elements of the matrix to be 0
Mat* MatEye(Mat* mat);//set a square matrix to be identity matrix, whereas the diagonal(from (1,1)) of a non-square matrix is set to be 1
Mat* MatOnes(Mat* mat);//set all the elements of the matrix to be 1
//---------------------------------------------------------------------------------------------------------------------------------------


//Part 3: matrix calculation-------------------------------------------------------------------------------------------------------------
Mat* MatAdd(const Mat* input1, const Mat* input2, Mat* output);//matrix addition
Mat* MatSub(const Mat* input1, const Mat* input2, Mat* output);//matrix subtraction
Mat* MatMul(const Mat* input1, const Mat* input2, Mat* output);//matrix multiplication
Mat* MatPower(const Mat* input, int pow, Mat* output);//power of the matrix(to avoid complex number, power is an integer)
Mat* MatTrans(const Mat* input, Mat* output);//matrix transpose
double MatDet(const Mat* mat);//determinant of a matrix	 
Mat* MatAdj(const Mat* input, Mat* output);//adjoint matrix
Mat* MatInv(const Mat* input, Mat* output);//inverse of a matrix
void MatCopy(const Mat* input, Mat* output);//copy the source matrix, "input", to the destination matrix, "output"
//----------------------------------------------------------------------------------------------------------------------------------------


//Part 4: matrix-scalar calculation
Mat* MatMulNum(const Mat * input, float num, Mat * output);//multiply matrix by a scalar(output=input*num)
Mat* MatMulNum_DBL(const Mat * input, double num, Mat * output);//(OVERLOADED FUNCTION)multiply matrix by a scalar(output=input*num)
Mat* MatDivNum(const Mat * input, float num, Mat * output);//matrix divided by a scalar(output=input/num)
Mat* MatDivNum_DBL(const Mat * input, double num, Mat * output);//(OVERLOADED FUNCTION)matrix divided by a scalar(output=input/num)
Mat* MatReciprocal(const Mat* input, Mat* output);//reciprocal(output=1./input)
Mat* MatAddNum(const Mat * input, float num, Mat * output);//add a scalar to a matrix(output=input+num)
Mat* MatAddNum_DBL(const Mat * input, double num, Mat * output);//(OVERLOADED FUNCTION)add a scalar to a matrix(output=input+num)
Mat* MatSubNum(const Mat * input, float num, Mat * output);//subtract a scalar from a matrix(output=input-num)
Mat* MatSubNum_DBL(const Mat * input, double num, Mat * output);//(OVERLOADED FUNCTION)subtract a scalar from a matrix(output=input-num)
Mat* MatReshape(const Mat* input, Mat* output);//reshape the input matrix into a given size defined by output matrix along the column(as defined in matlab)
Mat* MatGetCol(const Mat* input, int col_i, Mat* output);//get a column vector from the matrix, col_i is the 0-based index
Mat* MatGetRow(const Mat* input, int row_i, Mat* output);//get a row vector from the matrix, row_i is the 0-based index
Mat* MatGetSubMat(const Mat* input, int row_i_0, int col_i_0, int row_i_f, int col_i_f, Mat* output);//get the submatrix from the upper-left(i_0) to the lower-right(i_f) elements of the input matrix
Mat* MatCatMat(const Mat* input1, const Mat* input2, int dim, Mat* output);//combine 2 block matrices to a new matrix. dim=1, horizontally; dim=2, vertically
Mat* MatMulMatElementwise(const Mat* input1, const Mat* input2, Mat* output);//element-wise multiplication of 2 matrices(output=input1.*input2)
Mat* MatDivMatElementwise(const Mat* input1, const Mat* input2, Mat* output);//element-wise division(output=input1./input2)
Mat* MatMulMatElementwisePower(const Mat* input, int pow, Mat* output);//element-wise power
int MatLength(const Mat* input);//the larger number of row and column as defined in Matlab


//Part 5: vector calculation
double VecDotProduct(const Mat* input1, const Mat* input2);//dot product(input vectors should have the same dimension)
Mat* VecCrossProduct3D(const Mat* input1, const Mat* input2, Mat* output);//cross product(input vectors should have the same dimension)
double VecMagnitude(const Mat* input);//the magnitude of a vector


//Part 6: statistics
Mat* MatSum(const Mat* input, int dim, Mat* output);//sum of a matrix along dimension "dim"
Mat* MatMean(const Mat* input, int dim, Mat* output);//mean value
Mat* MatVar(const Mat* input, int w, int dim, Mat* output);//variance, w=0, /(N-1); w=1, /N
Mat* MatStd(const Mat* input, int w, int dim, Mat* output);//standard deviation, w=0, /(N-1); w=1, /N
Mat* MatMin(const Mat* input, int dim, Mat* output);//matrix minimum
Mat* MatMax(const Mat* input, int dim, Mat* output);//matrix maximum
double MatCov(const Mat* input1, const Mat* input2, int w);//covariance
Mat* MatSkewness(const Mat* input, bool flag, int dim, Mat* output);//skewness
/*---------------------------------------------------------------------------------*/
#ifdef __cplusplus
} // closing brace for extern "C"
#endif

#endif



