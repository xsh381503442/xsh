/*!!!The matrix function library is based on the previous work done by Jiachi Zou as shown below!!!!!*/

/*
 * Light Matrix: C code implementation for basic matrix operation
 *
 * Copyright (C) 2017 Jiachi Zou
 *
 * This code is vPortFree software: you can redistribute it and/or modify
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

Modifications is made by Neil. More functions are added to make the
library versatile. In order to improve the quality and functionality
of the code furtherly, team effort is essential, hence every programmer
is welcome to modify, rewrite the functions or add new ones.

*/


#include "algo_MatFun.h"

#define min(a, b) ((a) > (b) ? (b) : (a))
#define equal(a, b)	((a-b)<1e-7f && (a-b)>-(1e-7f))

//Here the concept of class function of cpp is adopted. The private functions
//are called by the public functions below, whereas the public functions are 
//used directly by users.

/************************************************************************/
/*                          Private Function                            */
/************************************************************************/
//Private functions are ONLY used in this file.

/* exchange of the value between a and b */
void swap(int *a, int *b)
{
	int m;
	m = *a;
	*a = *b;
	*b = m;
}

/* permutation, output is det*/
void perm(int list[], int k, int m, int* p, const Mat* mat, float* det)
{
	int i;

	if (k > m) {
		float res = mat->element[0][list[0]];

		for (i = 1; i < mat->row; i++) {
			res *= mat->element[i][list[i]];
		}

		if (*p % 2) {
			//odd is negative
			*det -= res;
		}
		else {
			//even is positive
			*det += res;
		}
	}
	else {
		// if the element is 0, we don't need to calculate the value for this permutation
		if (!equal(mat->element[k][list[k]], 0))
			perm(list, k + 1, m, p, mat, det);
		for (i = k + 1; i <= m; i++)
		{
			if (equal(mat->element[k][list[i]], 0))
				continue;
			swap(&list[k], &list[i]);
			*p += 1;
			perm(list, k + 1, m, p, mat, det);
			swap(&list[k], &list[i]);
			*p -= 1;
		}
	}
}

/* permutation, output is det*/
void perm_DBL(int list[], int k, int m, int* p, const Mat* mat, double* det)
{
	int i;

	if (k > m) {
		float res = mat->element[0][list[0]];

		for (i = 1; i < mat->row; i++) {
			res *= mat->element[i][list[i]];
		}

		if (*p % 2) {
			//odd is negative
			*det -= res;
		}
		else {
			//even is positive
			*det += res;
		}
	}
	else {
		// if the element is 0, we don't need to calculate the value for this permutation
		if (!equal(mat->element[k][list[k]], 0))
			perm_DBL(list, k + 1, m, p, mat, det);
		for (i = k + 1; i <= m; i++)
		{
			if (equal(mat->element[k][list[i]], 0))
				continue;
			swap(&list[k], &list[i]);
			*p += 1;
			perm_DBL(list, k + 1, m, p, mat, det);
			swap(&list[k], &list[i]);
			*p -= 1;
		}
	}
}

/************************************************************************/
/*                           Public Function                            */
/************************************************************************/
//Public functions can be used outside this file.
/* Creation of matrix */

Mat* MatCreate(Mat* mat, int row, int col)
{
	int i;
	//vPortInitialiseBlocks();
	mat->element = (DBL_or_FLT**)pvPortMalloc(row * sizeof(DBL_or_FLT*));
	if (mat->element == NULL) {
		//printf("mat create fail!\n");
		return NULL;
		//exit(EXIT_FAILURE);
	}
	//vPortInitialiseBlocks();
	for (i = 0; i < row; i++) {
		mat->element[i] = (DBL_or_FLT*)pvPortMalloc(col * sizeof(DBL_or_FLT));
		if (mat->element[i] == NULL) {
			int j;
			//printf("mat create fail!\n");
			for (j = 0; j < i; j++)
				vPortFree(mat->element[j]);
			vPortFree(mat->element);
			return NULL;
			//exit(EXIT_FAILURE);
		}
	}

	mat->row = row;
	mat->col = col;
	mat->numel = row * col;

	return mat;
}

/* vPortFree(mat) */
void MatDelete(Mat* mat)
{
	int i;

	for (i = 0; i < mat->row; i++)
		vPortFree(mat->element[i]);
	vPortFree(mat->element);
}

/* mat = val; */
Mat* MatSetVal(Mat* mat, const float* val)
{
	int row, col;

	for (row = 0; row < mat->row; row++) {
		for (col = 0; col < mat->col; col++) {
			mat->element[row][col] = val[col + row * mat->col];
		}
	}

	return mat;
}

/* mat = val; */
Mat* MatSetVal_DBL(Mat* mat, const double* val)
{
	int row, col;

	for (row = 0; row < mat->row; row++) {
		for (col = 0; col < mat->col; col++) {
			mat->element[row][col] = val[col + row * mat->col];
		}
	}

	return mat;
}
/* display the matrix */
void MatDisp(const Mat* mat)
{

#ifdef MAT_LEGAL_CHECKING //Yes, it was defined in the beginning of this file
	if (mat == NULL) {
		return;
	}
#endif
	int row, col;

	//using namespace std;
	printf("Mat %d-by-%d:\n", mat->row, mat->col);
	for (row = 0; row < mat->row; row++) {
//		cout << "[ ";
		for (col = 0; col < mat->col; col++) {
			printf("%.4f\t", mat->element[row][col]);
//			cout << setw(10) << setfill(' ') << setprecision(7) << mat->element[row][col];
		}
//		cout << "] ";
		printf("\n");
	}
}



/* mat = zeros(r,c); */
Mat* MatZeros(Mat* mat)
{
	int row, col;
	DBL_or_FLT temp = 0.0;

	for (row = 0; row < mat->row; row++) {
		for (col = 0; col < mat->col; col++) {
			mat->element[row][col] = temp;
		}
	}

	return mat;
}

/* mat = eye(r,c) */
Mat* MatEye(Mat* mat)
{
	int i;
	DBL_or_FLT temp;
	temp = 1.0;

	MatZeros(mat);
	for (i = 0; i < min(mat->row, mat->col); i++) {
		mat->element[i][i] = temp;
	}

	return mat;
}

/* mat = ones(r,c) */
Mat* MatOnes(Mat * mat)
{
	int row, col;
	DBL_or_FLT temp;
	temp = 1.0;

	for (row = 0; row < mat->row; row++) {
		for (col = 0; col < mat->col; col++) {
			mat->element[row][col] = temp;
		}
	}

	return mat;
}



/* output = input1 + input2 */
Mat* MatAdd(const Mat* input1, const Mat* input2, Mat* output)
{
#ifdef MAT_LEGAL_CHECKING
	//the assertion shall be true, otherwise, the program execution will be aborted
	assert(input1->row == input2->row && input2->row == output->row && input1->col == input2->col && input2->col == output->col);
#endif

	int row, col;
	for (row = 0; row < input1->row; row++) {
		for (col = 0; col < input1->col; col++) {
			output->element[row][col] = input1->element[row][col] + input2->element[row][col];
		}
	}

	return output;
}

/* output = input1 - input2 */
Mat* MatSub(const Mat* input1, const Mat* input2, Mat* output)
{
#ifdef MAT_LEGAL_CHECKING
	assert(input1->row == input2->row && input2->row == output->row && input1->col == input2->col && input2->col == output->col);
#endif

	int row, col;

	for (row = 0; row < input1->row; row++) {
		for (col = 0; col < input1->col; col++) {
			output->element[row][col] = input1->element[row][col] - input2->element[row][col];
		}
	}

	return output;
}

/* output = input1 * input2 */
Mat* MatMul(const Mat* input1, const Mat* input2, Mat* output)
{
#ifdef MAT_LEGAL_CHECKING
	assert(input1->col == input2->row && input1->row == output->row && input2->col == output->col);
#endif
	
	int row, col;
	int i;
	DBL_or_FLT temp;

	for (row = 0; row < output->row; row++) {
		for (col = 0; col < output->col; col++) {
			temp = 0.0;
			for (i = 0; i < input1->col; i++) {
				temp += input1->element[row][i] * input2->element[i][col];
			}
			output->element[row][col] = temp;
		}
	}

	return output;
}

/* output = input^pow */
Mat* MatPower(const Mat * input, int pow, Mat * output)
{
#ifdef MAT_LEGAL_CHECKING
	assert(input->row == input->col && output->row == output->col && input->row == output->row && pow>0);
#endif // MAT_LEGAL_CHECKING
	MatCopy(input,output);//initialize the output to the input
	if (pow > 1)
	{
		Mat temp;
		MatCreate(&temp, output->row, output->col);
		MatCopy(input, &temp);
		for (int i = 1; i < pow; i++)//if pow=1, this line will not be executed
		{
			MatMul(input, &temp, output);
			MatCopy(output, &temp);
		}
		MatDelete(&temp);
	}
	return output;
}

/* output = input' */
Mat* MatTrans(const Mat* input, Mat* output)
{
#ifdef MAT_LEGAL_CHECKING
	assert(input->row == output->col && input->col == output->row);
#endif

	int row, col;
	for (row = 0; row < output->row; row++) {
		for (col = 0; col < output->col; col++) {
			output->element[row][col] = input->element[col][row];
		}
	}

	return output;
}

/* return det(mat) */
double MatDet(const Mat* mat)
{
#ifdef MAT_LEGAL_CHECKING
	if (mat->row != mat->col) {
		printf("err check, not a square matrix for MatDetermine\n");
		MatDisp(mat);
		return 0.0;
	}
#endif

	double det = 0.0;
	int plarity = 0;
	int *list;
	int i;
  //vPortInitialiseBlocks();
	list = (int*)pvPortMalloc(sizeof(int)*mat->col);
	if (list == NULL) {
		printf("pvPortMalloc list fail\n");
		//exit(EXIT_FAILURE);
		perror("pvPortMalloc list fail: ");
		//exit(EXIT_FAILURE); //This is an alternative way to handle the error
		return(-1);
	}
	for (i = 0; i < mat->col; i++)
		list[i] = i;
	
	perm_DBL(list, 0, mat->row - 1, &plarity, mat, &det);
	vPortFree(list);

	return det;
}

/* output = adj(input) */
Mat* MatAdj(const Mat* input, Mat* output)
{
#ifdef MAT_LEGAL_CHECKING
	assert(input->row == input->col && input->row == output->row && input->col == output->col);
#endif

	Mat smat;
	int row, col;
	int i, j, r, c;
	DBL_or_FLT det;

	MatCreate(&smat, input->row - 1, input->col - 1);

	for (row = 0; row < input->row; row++) {
		for (col = 0; col < input->col; col++) {
			r = 0;
			for (i = 0; i < input->row; i++) {
				if (i == row)
					continue;
				c = 0;
				for (j = 0; j < input->col; j++) {
					if (j == col)
						continue;
					smat.element[r][c] = input->element[i][j];
					c++;
				}
				r++;
			}
			det = MatDet(&smat);//automatic data type conversion will probably happen depending on the type of det(float or double)
			if ((row + col) % 2)
				det = -det;
			output->element[col][row] = det;
		}
	}

	MatDelete(&smat);

	return output;
}

/* output = input^(-1) it uses the above "MatAdj" function*/
Mat* MatInv(const Mat* input, Mat* output)
{
#ifdef MAT_LEGAL_CHECKING
	assert(input->row == input->col || input->row == output->row || input->col == output->col);
#endif

	Mat adj_mat;
	DBL_or_FLT det;
	int row, col;

	MatCreate(&adj_mat, input->row, input->col);
	MatAdj(input, &adj_mat);
	det = MatDet(input);

	if (equal(det, 0)) {
		printf("err, determinate is 0 for MatInv\n");
		return NULL;
	}

	for (row = 0; row < input->row; row++) {
		for (col = 0; col < input->col; col++)
			output->element[row][col] = adj_mat.element[row][col] / det;
	}

	MatDelete(&adj_mat);

	return output;
}

/* output = input; */
void MatCopy(const Mat* input, Mat* output)
{
	int row, col;

#ifdef MAT_LEGAL_CHECKING
	if (input->row != output->row || input->col != output->col) {
		printf("err check, mismatched matrix for MatCopy\n");
		MatDisp(input);
		MatDisp(output);
		return;
	}
#endif

	for (row = 0; row < input->row; row++) {
		for (col = 0; col < input->col; col++)
			output->element[row][col] = input->element[row][col];
	}
}

/* output = num * input */
Mat* MatMulNum(const Mat * input, float num, Mat * output)
{
#ifdef MAT_LEGAL_CHECKING
	assert(input->row == output->row || input->col == output->col);
#endif
	for (int row = 0; row < output->row; row++) 
	{
		for (int col = 0; col < output->col; col++)
		{
			output->element[row][col] = num*input->element[row][col];
		}
	}

	return output;
}
/* output = num * input */
Mat* MatMulNum_DBL(const Mat * input, double num, Mat * output)
{
#ifdef MAT_LEGAL_CHECKING
	assert(input->row == output->row || input->col == output->col);
#endif
	for (int row = 0; row < output->row; row++) 
	{
		for (int col = 0; col < output->col; col++)
		{
			output->element[row][col] = num*input->element[row][col];
		}
	}

	return output;
}
/* output = input / num */
Mat* MatDivNum(const Mat * input, float num, Mat * output)
{
#ifdef MAT_LEGAL_CHECKING
	assert(input->row == output->row || input->col == output->col);
#endif

	MatMulNum(input, 1.0f / num, output);
	return output;
}
/* output = input / num */
Mat* MatDivNum_DBL(const Mat * input, double num, Mat * output)
{
#ifdef MAT_LEGAL_CHECKING
	assert(input->row == output->row || input->col == output->col);
#endif

	MatMulNum_DBL(input, 1.0 / num, output);
	return output;
}
/* output = 1. / input */
Mat * MatReciprocal(const Mat * input, Mat * output)
{
#ifdef MAT_LEGAL_CHECKING
	assert(input->row == output->row && input->col == output->col);
#endif // MAT_LEGAL_CHECKING

	DBL_or_FLT temp = 1.0;
	for (int row_i = 0; row_i < input->row; row_i++)
	{
		for (int col_i = 0; col_i < input->col; col_i++)
		{
			output->element[row_i][col_i] = temp / input->element[row_i][col_i];
		}
	}
	return output;
}

/* output = input + num */
Mat* MatAddNum(const Mat * input, float num, Mat * output)
{
#ifdef MAT_LEGAL_CHECKING
	assert(input->row == output->row || input->col == output->col);
#endif
	Mat temp1;
	Mat temp2;
	MatCreate(&temp1, output->row, output->col);
	MatCreate(&temp2, output->row, output->col);
	//convert the num into a matrix
	MatOnes(&temp1);
	MatMulNum(&temp1, num, &temp2);
	MatAdd(input, &temp2, output);
	MatDelete(&temp1);
	MatDelete(&temp2);
	return output;
}

/* output = input + num */
Mat* MatAddNum_DBL(const Mat * input, double num, Mat * output)
{
#ifdef MAT_LEGAL_CHECKING
	assert(input->row == output->row || input->col == output->col);
#endif
	Mat temp1;
	Mat temp2;
	MatCreate(&temp1, output->row, output->col);
	MatCreate(&temp2, output->row, output->col);
	//convert the num into a matrix
	MatOnes(&temp1);
	MatMulNum_DBL(&temp1, num, &temp2);
	MatAdd(input, &temp2, output);
	MatDelete(&temp1);
	MatDelete(&temp2);
	return output;
}
/* output = input - num */
Mat* MatSubNum(const Mat * input, float num, Mat * output)
{
#ifdef MAT_LEGAL_CHECKING
	assert(input->row == output->row || input->col == output->col);
#endif
	MatAddNum(input, -num, output);
	return output;
}

/* output = input - num */
Mat* MatSubNum_DBL(const Mat * input, double num, Mat * output)
{
#ifdef MAT_LEGAL_CHECKING
	assert(input->row == output->row || input->col == output->col);
#endif
	MatAddNum_DBL(input, -num, output);
	return output;
}
/* output = reshape(input) */
Mat* MatReshape(const Mat * input, Mat * output)
{
	//error check (debug)
#ifdef MAT_LEGAL_CHECKING
	assert(input->numel == output->numel);
#endif

	//define a temporary dynamic 1-D array to store the elements of the source array
	//DBL_or_FLT* p = new DBL_or_FLT[input->row*input->col];//C++ style
//	vPortInitialiseBlocks();
	DBL_or_FLT* p = (DBL_or_FLT*)pvPortMalloc(input->numel*sizeof(DBL_or_FLT));//C style
	
	for (int col = 0; col < input->col; col++)
	{
		for (int row = 0; row < input->row; row++)
		{
			p[col*input->row + row] = input->element[row][col];
		}
	}
	
	//assign the values of the dynamic array to the output matrix
	int i = 0;
	for (int col = 0; col < output->col; col++)
	{
		for (int row = 0; row < output->row; row++)
		{
			output->element[row][col] = p[i];
			++i;
		}
	}
	
	//vPortFree memory and return the value
	//delete [] p;//C++ style
	vPortFree(p);
	return output;
}

/* output = input(:,col_i) */
Mat* MatGetCol(const Mat * input, int col_i, Mat * output)
{
	//error check (debug)
#ifdef MAT_LEGAL_CHECKING
	assert(input->row == output->row && output->col == 1);
#endif
	//assignment
	for (int row = 0; row < input->row; row++)
	{
		output->element[row][0] = input->element[row][col_i];
	}
	return output;
}

/* output = input(row_i,:) */
Mat* MatGetRow(const Mat * input, int row_i, Mat * output)
{
	//error check (debug)
#ifdef MAT_LEGAL_CHECKING
	assert(input->col == output->col && output->row == 1);
#endif
	//assignment
	for (int col = 0; col < input->col; col++)
	{
		output->element[0][col] = input->element[row_i][col];
	}
	return output;
}

/* output = input(row_i_0:row_i_f,col_i_0:col_i_f) */
Mat* MatGetSubMat(const Mat * input, int row_i_0, int col_i_0, int row_i_f, int col_i_f, Mat * output)
{
	//error check (debug)
#ifdef MAT_LEGAL_CHECKING
	assert(input->row >= output->row && input->col >= output->col);
	assert(row_i_0 <= row_i_f && col_i_0 <= col_i_f);
	assert(row_i_0 >= 0 && col_i_0 >= 0 && row_i_f <= input->row && col_i_f <= input->col);
	assert(output->row == row_i_f - row_i_0 + 1 && output->col == col_i_f - col_i_0 + 1);
#endif

	//assignment
	int output_i = 0;
	int output_j = 0;
	for (int row = row_i_0; row <= row_i_f; row++)
	{
		for (int col = col_i_0; col <= col_i_f; col++)
		{
			output->element[output_i][output_j] = input->element[row][col];
			++output_j;
		}
		output_j = 0;
		++output_i;
	}
	return output;
}

/* output = [input1, input2] or output = [input1; input2] */
Mat* MatCatMat(const Mat * input1, const Mat * input2, int dim, Mat * output)
{
#ifdef MAT_LEGAL_CHECKING
	assert(dim == 1 || dim == 2);
	if (dim==1)
	{
		assert(input1->row == input2->row && input1->row == output->row && input1->col + input2->col == output->col);
	}
	else
	{
		assert(input1->col == input2->col && input1->col == output->col && input1->row + input2->row == output->row);
	}
#endif // MAT_LEGAL_CHECKING
	if (dim == 1)
	{
		for (int row_i = 0; row_i < output->row; row_i++)
		{
			for (int col_i = 0; col_i < output->col; col_i++)
			{
				if (col_i < input1->col)
				{
					output->element[row_i][col_i] = input1->element[row_i][col_i];
				}
				else
				{
					output->element[row_i][col_i] = input2->element[row_i][col_i - input1->col];
				}
			}
		}
	}
	else
	{
		for (int row_i = 0; row_i < output->row; row_i++)
		{
			if (row_i < input1->row)
			{
				for (int col_i = 0; col_i < output->col; col_i++)
				{
					output->element[row_i][col_i] = input1->element[row_i][col_i];
				}
			}
			else
			{
				for (int col_i = 0; col_i < output->col; col_i++)
				{
					output->element[row_i][col_i] = input2->element[row_i - input1->row][col_i];
				}
			}
		}
	}
	return output;
}

/* output = input1.*input2 */
Mat* MatMulMatElementwise(const Mat * input1, const Mat * input2, Mat * output)
{
	//error check (debug)
#ifdef MAT_LEGAL_CHECKING
	assert(input1->row == output->row && input1->col == output->col && input2->row == output->row && input2->col == output->col);
	assert(input1->row == input2->row && input1->col == input2->col);
#endif

	//calculation
	for (int row = 0; row < output->row; row++)
	{
		for (int col = 0; col < output->col; col++)
		{
			output->element[row][col] = input1->element[row][col] * input2->element[row][col];
		}
	}
	return output;
}

/* output = input1./input2 */
Mat * MatDivMatElementwise(const Mat * input1, const Mat * input2, Mat * output)
{
	//error check (debug)
#ifdef MAT_LEGAL_CHECKING
	assert(input1->row == output->row && input1->col == output->col && input2->row == output->row && input2->col == output->col);
	assert(input1->row == input2->row && input1->col == input2->col);
#endif
	Mat temp;
	MatCreate(&temp, input2->row, input2->col);
	MatReciprocal(input2, &temp);
	return MatMulMatElementwise(input1, &temp, output);
}

/* output=input.^pow  */
Mat* MatMulMatElementwisePower(const Mat * input, int pow, Mat * output)
{
#ifdef MAT_LEGAL_CHECKING
	assert(input->row == output->row && input->col == output->col);
#endif // MAT_LEGAL_CHECKING
	MatCopy(input, output);//initialize the output to the input
	if (pow > 1)
	{
		Mat temp;
		MatCreate(&temp, output->row, output->col);
		MatCopy(input, &temp);
		for (int i = 1; i < pow; i++)//if pow=1, this line will not be executed
		{
			MatMulMatElementwise(input, &temp, output);
			MatCopy(output, &temp);
		}
		MatDelete(&temp);
	}
	return output;
}

/* return length(input) */
int MatLength(const Mat * input)
{
	if (input->row >=input->col)
	{
		return input->row;
	}
	else
	{
		return input->col;
	}
}

/* output = dot(input1,input2) */
double VecDotProduct(const Mat * input1, const Mat * input2)
{
	//error check (debug)
#ifdef MAT_LEGAL_CHECKING
	assert(input1->row == input2->row && input1->col == input2->col);
	assert(input1->row == 1 || input1->col == 1);
#endif // MAT_LEGAL_CHECKING

	//calculation
	double output = 0.0;
	if (input1->row == 1)//row vector
	{
		for (int col = 0; col < input1->col; col++)
		{
			output += input1->element[0][col] * input2->element[0][col];
		}
	}
	else//column vector
	{
		for (int row = 0; row < input1->row; row++)
		{
			output += input1->element[row][0] * input2->element[row][0];
		}
	}
	return output;
}

/* output = cross(input1,input2) */
Mat* VecCrossProduct3D(const Mat * input1, const Mat * input2, Mat * output)
{
	// error check(debug)
#ifdef MAT_LEGAL_CHECKING
	assert(input1->row == input2->row && input1->col == input2->col);
	assert(input1->row == 1 || input1->col == 1);
	assert(input1->numel == 3);
#endif // MAT_LEGAL_CHECKING

	//calculation
	if (input1->row == 1)//row vector
	{
		output->element[0][0] = input1->element[0][1] * input2->element[0][2] - input1->element[0][2] * input2->element[0][1];
		output->element[0][1] = input1->element[0][2] * input2->element[0][0] - input1->element[0][0] * input2->element[0][2];
		output->element[0][2] = input1->element[0][0] * input2->element[0][1] - input1->element[0][1] * input2->element[0][0];
	}
	else//column vector
	{
		output->element[0][0] = input1->element[1][0] * input2->element[2][0] - input1->element[2][0] * input2->element[1][0];
		output->element[1][0] = input1->element[2][0] * input2->element[0][0] - input1->element[0][0] * input2->element[2][0];
		output->element[2][0] = input1->element[0][0] * input2->element[1][0] - input1->element[1][0] * input2->element[0][0];
	}
	return output;
}

/* norm(input) */
double VecMagnitude(const Mat * input)
{
	//debug
	#ifdef MAT_LEGAL_CHECKING
	assert(input->row == 1 || input->col == 1);
	#endif
	double temp = 0;
	//sum the square of each element
	if (input->row == 1)
	{
		for (int col_i = 0; col_i < input->col; col_i++)
		{
			temp += pow((double)input->element[0][col_i], 2.0);
		}
	}
	else//col==1
	{
		for (int row_i = 0; row_i < input->row; row_i++)
		{
			temp += pow((double)input->element[row_i][0], 2.0);
		}
	}
	return sqrt(temp);
}

/* output = sum(input, dim)*/
Mat* MatSum(const Mat * input, int dim, Mat* output)
{
#ifdef MAT_LEGAL_CHECKING
	if (dim==1)
	{
		assert(input->col == output->col && output->row == 1);
	}
	else if (dim==2)
	{
		assert(input->row == output->row && output->col == 1);
	}
	else
	{
		exit(EXIT_FAILURE);
	}
#endif // MAT_LEGAL_CHECKING

	if (dim == 1)
	{
		for (int col=0; col<output->col; col++)
		{
			output->element[0][col] = 0;
			for (int row = 0; row < input->row; row++)
			{
				output->element[0][col] += input->element[row][col];
			}
		}
	}
	else//dim=2
	{
		for (int row= 0; row < output->row; row++)
		{
			output->element[row][0] = 0;
			for (int col = 0; col < input->col; col++)
			{
				output->element[row][0] += input->element[row][col];
			}
		}
	}
	return output;
}

/* output = mean(input, dim) */
Mat* MatMean(const Mat * input, int dim, Mat * output)
{
#ifdef MAT_LEGAL_CHECKING
	if (dim == 1)
	{
		assert(input->col == output->col && output->row == 1);
	}
	else if (dim == 2)
	{
		assert(input->row == output->row && output->col == 1);
	}
	else
	{
		exit(EXIT_FAILURE);
	}
#endif // MAT_LEGAL_CHECKING

	DBL_or_FLT temp = 1.0;
	if (dim == 1)
	{
		Mat temp_mat1;
		MatCreate(&temp_mat1, 1, output->col);
		MatSum(input, 1, &temp_mat1);
		for (int col_i = 0; col_i < output->col; col_i++)
		{
			output->element[0][col_i] = (&temp_mat1)->element[0][col_i] * temp / input->row;
		}
		MatDelete(&temp_mat1);
	}
	else
	{
		Mat temp_mat2;
		MatCreate(&temp_mat2, output->row, 1);
		MatSum(input, 2, &temp_mat2);
		for (int row_i = 0; row_i < output->row; row_i++)
		{
			output->element[row_i][0] = (&temp_mat2)->element[row_i][0] * temp / input->col;
		}
		MatDelete(&temp_mat2);
	}
	return output;
}

/* output = var(input, dim) */
Mat* MatVar(const Mat * input, int w, int dim, Mat * output)
{
#ifdef MAT_LEGAL_CHECKING
	if (dim == 1)
	{
		assert(input->col == output->col && output->row == 1);
	}
	else if (dim == 2)
	{
		assert(input->row == output->row && output->col == 1);
	}
	else
	{
		exit(EXIT_FAILURE);
	}
#endif // MAT_LEGAL_CHECKING

	DBL_or_FLT temp = 1.0;

	Mat temp_mat1;//mu_matrix
	Mat temp_mat2;//input_ij-mu_matrix
	Mat temp_mat3;//temp_mat2.^2
	Mat temp_mat4;//mean(temp_mat3,dim)
	MatCreate(&temp_mat1, output->row, output->col);
	MatCreate(&temp_mat2, input->row, input->col);
	MatCreate(&temp_mat3, input->row, input->col);
	MatCreate(&temp_mat4, output->row, output->col);

	//assign value to temp_mat1
	MatMean(input, dim, &temp_mat1);
	//assign value to temp_mat2
	if (dim == 1)
	{
		for (int row_i = 0; row_i < input->row; row_i++)
		{
			for (int col_i = 0; col_i < input->col; col_i++)
			{
				temp_mat2.element[row_i][col_i] = input->element[row_i][col_i] - temp_mat1.element[0][col_i];
			}
		}
	}
	else//dim==2
	{
		for (int row_i = 0; row_i < input->row; row_i++)
		{
			for (int col_i = 0; col_i < input->col; col_i++)
			{
				temp_mat2.element[row_i][col_i] = input->element[row_i][col_i] - temp_mat1.element[row_i][0];
			}
		}
	}
	//assign value to temp_mat3
	MatMulMatElementwisePower(&temp_mat2, 2, &temp_mat3);
	//assign value to temp_mat4
	MatMean(&temp_mat3, dim, &temp_mat4);
	if (w == 0)//divided by observations-1(N-1)
	{
		if (dim==1)
		{
			if (input->row > 1)//to avoid a 0 denominator
			{
#ifdef DOUBLE_MAT
				MatMulNum_DBL(&temp_mat4, input->row*temp / (input->row - 1), output);
#else
				MatMulNum(&temp_mat4, input->row*temp / (input->row - 1), output);
#endif
			}
			else//input is a row vector, the variance of a single number is 0
			{
				MatZeros(output);
			}
		}
		else//dim==2
		{
			if (input->col > 1)
			{
#ifdef DOUBLE_MAT
				MatMulNum_DBL(&temp_mat4, input->col*temp / (input->col - 1), output);
#else
				MatMulNum(&temp_mat4, input->col*temp / (input->col - 1), output);
#endif
			}
			else//input is a column vector, the variance of a single number is 0
			{
				MatZeros(output);
			}
		}
	}
	else//divided by N
	{
		MatMean(&temp_mat3, dim, output);
	}
	MatDelete(&temp_mat1);
	MatDelete(&temp_mat2);
	MatDelete(&temp_mat3);
	MatDelete(&temp_mat4);
	return output;
}

/* output = std(input, dim) */
Mat* MatStd(const Mat * input, int w, int dim, Mat * output)
{
#ifdef MAT_LEGAL_CHECKING
	if (dim == 1)
	{
		assert(input->col == output->col && output->row == 1);
	}
	else if (dim == 2)
	{
		assert(input->row == output->row && output->col == 1);
	}
	else
	{
		exit(EXIT_FAILURE);
	}
#endif // MAT_LEGAL_CHECKING
	//variance stored in a temp matrix
	Mat temp;
	MatCreate(&temp, output->row, output->col);
	MatVar(input, w, dim, &temp);
	//calculate the standard deviation
	if (dim == 1)
	{
		for (int col_i = 0; col_i < output->col; col_i++)
		{
			output->element[0][col_i] = sqrt(temp.element[0][col_i]);
		}
	}
	else//dim==2
	{
		for (int row_i = 0; row_i < output->row; row_i++)
		{
			output->element[row_i][0] = sqrt(temp.element[row_i][0]);
		}
	}
	return output;
}

/* output = min(input, dim) */
Mat* MatMin(const Mat * input, int dim, Mat * output)
{
#ifdef MAT_LEGAL_CHECKING
	if (dim == 1)
	{
		assert(input->col == output->col && output->row == 1);
	}
	else if (dim == 2)
	{
		assert(input->row == output->row && output->col == 1);
	}
	else
	{
		exit(EXIT_FAILURE);
	}
#endif //MAT_LEGAL_CHECKING
	//comparision and assignment

	if (dim == 1)
	{
		for (int col_i = 0; col_i < input->col; col_i++)
		{
			DBL_or_FLT temp = input->element[0][col_i];
			for (int row_i = 0; row_i < input->row; row_i++)
			{
				if (input->element[row_i][col_i] < temp)
				{
					temp = input->element[row_i][col_i];
				}
			}
			output->element[0][col_i] = temp;
		}
	}
	else//dim==2
	{
		for (int row_i = 0; row_i < input->row; row_i++)
		{
			DBL_or_FLT temp = input->element[row_i][0];
			for (int col_i = 0; col_i < input->col; col_i++)
			{
				if (input->element[row_i][col_i] < temp)
				{
					temp = input->element[row_i][col_i];
				}
			}
			output->element[row_i][0] = temp;
		}
	}
	return output;
}

/* output = max(input, dim) */
Mat* MatMax(const Mat * input, int dim, Mat * output)
{
#ifdef MAT_LEGAL_CHECKING
	if (dim == 1)
	{
		assert(input->col == output->col && output->row == 1);
	}
	else if (dim == 2)
	{
		assert(input->row == output->row && output->col == 1);
	}
	else
	{
		exit(EXIT_FAILURE);
	}
#endif //MAT_LEGAL_CHECKING
	//comparision and assignment

	if (dim == 1)
	{
		for (int col_i = 0; col_i < input->col; col_i++)
		{
			DBL_or_FLT temp = input->element[0][col_i];
			for (int row_i = 0; row_i < input->row; row_i++)
			{
				if (input->element[row_i][col_i] > temp)
				{
					temp = input->element[row_i][col_i];
				}
			}
			output->element[0][col_i] = temp;
		}
	}
	else//dim==2
	{
		for (int row_i = 0; row_i < input->row; row_i++)
		{
			DBL_or_FLT temp = input->element[row_i][0];
			for (int col_i = 0; col_i < input->col; col_i++)
			{
				if (input->element[row_i][col_i] > temp)
				{
					temp = input->element[row_i][col_i];
				}
			}
			output->element[row_i][0] = temp;
		}
	}
	return output;
}

/* output = cov(input1, input2, w), where inputs are vectors */
double MatCov(const Mat * input1, const Mat * input2, int w)
{
#ifdef MAT_LEGAL_CHECKING
	assert(input1->row == input2->row && input1->col == input2->col);//the sizes of the inputs are same
	assert(input1->row == 1 || input1->col == 1);//inputs must be vectors
	assert(w == 0 || w == 1);
#endif

	DBL_or_FLT temp = 1.0;

	Mat temp1;//mu1(scalar)
	Mat temp2;//input1-mu1(vector)
	Mat temp3;//mu2
	Mat temp4;//input-mu2
	if (input1->row == 1)//row vector
	{
		//assign values to the temp matrices
		MatCreate(&temp1, 1, 1);//it's actually a scalar(1-by-1 matrix)
		MatMean(input1, 2, &temp1);
		MatCreate(&temp2, input1->row, input1->col);
#ifdef DOUBLE_MAT
		MatSubNum_DBL(input1, (&temp1)->element[0][0], &temp2);
#else
		MatSubNum(input1, (&temp1)->element[0][0], &temp2);		
#endif
		MatCreate(&temp3, 1, 1);
		MatMean(input2, 2, &temp3);
		MatCreate(&temp4, input2->row, input2->col);
#ifdef DOUBLE_MAT
		MatSubNum_DBL(input2, (&temp3)->element[0][0], &temp4);
#else
		MatSubNum(input2, (&temp3)->element[0][0], &temp4);
#endif
		
		//Sigma(input1-mu1)*(input2-mu2)
		DBL_or_FLT temp5 = VecDotProduct(&temp2, &temp4);
		//vPortFree memory
		MatDelete(&temp1);
		MatDelete(&temp2);
		MatDelete(&temp3);
		MatDelete(&temp4);
		if (w == 0)
		{
			return temp5 * temp / (input1->col - 1);
		}
		else
		{
			return temp5 * temp / input1->col;
		}
	}
	else//col==1
	{
		//assign values to the temp matrices
		MatCreate(&temp1, 1, 1);//it's actually a scalar(1-by-1 matrix)
		MatMean(input1, 1, &temp1);
		MatCreate(&temp2, input1->row, input1->col);
#ifdef DOUBLE_MAT
		MatSubNum_DBL(input1, (&temp1)->element[0][0], &temp2);		
#else
		MatSubNum(input1, (&temp1)->element[0][0], &temp2);		
#endif
		MatCreate(&temp3, 1, 1);
		MatMean(input2, 1, &temp3);
		MatCreate(&temp4, input2->row, input2->col);
#ifdef DOUBLE_MAT
		MatSubNum_DBL(input2, (&temp3)->element[0][0], &temp4);
#else
		MatSubNum(input2, (&temp3)->element[0][0], &temp4);
#endif
		

		//Sigma(input1-mu1)*(input2-mu2)
		DBL_or_FLT temp5 = VecDotProduct(&temp2, &temp4);
		//vPortFree memory
		MatDelete(&temp1);
		MatDelete(&temp2);
		MatDelete(&temp3);
		MatDelete(&temp4);
		if (w == 0)
		{
			return temp5 * temp / (input1->row - 1);
		}
		else
		{
			return temp5 * temp / input1->row;
		}
	}
}

/* output = skewness(input, flag, dim) */
Mat* MatSkewness(const Mat * input, bool flag, int dim, Mat* output)
{
	//error check (debug)
#ifdef MAT_LEGAL_CHECKING
	if (dim == 1)
	{
		assert(input->col == output->col && output->row == 1);
	}
	else if (dim == 2)
	{
		assert(input->row == output->row && output->col == 1);
	}
	else
	{
		exit(EXIT_FAILURE);
	}
	assert(flag == true || flag == false);
#endif //MAT_LEGAL_CHECKING

	DBL_or_FLT temp = 1.0;

	//define temporary matrices
	Mat temp1;//mu(input) a row/column vector (mean value)
	Mat temp2;//a column/row vector of the input
	Mat temp3;//a column/row vector(input-mu_input)
	Mat temp4;//a column/row vector((input-mu_input).^3)
	Mat temp5;//mu(temp4) 1-by-1 matrix
	Mat temp6;//mu(temp4) row/column vector
	Mat temp7;//std(input,1or2) 1-by-1 matrix
	Mat temp8;//std(input,1or2) normalized by n instead of (n-1)
	Mat temp9;//temp8.^3, a row/column vector

	if (dim == 1)
	{
		//row vectors
		//matrices initialization
		MatCreate(&temp1, 1, input->col);
		MatMean(input, dim, &temp1);
		MatCreate(&temp2, input->row, 1);
		MatCreate(&temp3, input->row, 1);
		MatCreate(&temp4, input->row, 1);
		MatCreate(&temp5, 1, 1);
		MatCreate(&temp6, 1, input->col);
		MatZeros(&temp6);
		MatCreate(&temp7, 1, 1);
		MatZeros(&temp7);
		MatCreate(&temp8, 1, input->col);
		MatCreate(&temp9, 1, input->col);

		//calculation
		for (int col_i = 0; col_i < input->col; col_i++)
		{
			//numerator
			MatGetCol(input, col_i, &temp2);
#ifdef DOUBLE_MAT
			MatSubNum_DBL(&temp2, temp1.element[0][col_i], &temp3);
#else
			MatSubNum(&temp2, temp1.element[0][col_i], &temp3);
#endif
			MatMulMatElementwisePower(&temp3, 3, &temp4);
			MatMean(&temp4, dim, &temp5);
			temp6.element[0][col_i] = temp5.element[0][0];
			//denominator
			MatStd(&temp2, 1, dim, &temp7);
			temp8.element[0][col_i] = temp7.element[0][0];
			MatMulMatElementwisePower(&temp8, 3, &temp9);
		}
	}
	else//dim==2
	{
		//column vectors
		//matrices initialization
		MatCreate(&temp1, input->row, 1);
		MatMean(input, dim, &temp1);
		MatCreate(&temp2, 1, input->col);
		MatCreate(&temp3, 1, input->col);
		MatCreate(&temp4, 1, input->col);
		MatCreate(&temp5, 1, 1);
		MatCreate(&temp6, input->row, 1);
		MatZeros(&temp6);
		MatCreate(&temp7, 1, 1);
		MatZeros(&temp7);
		MatCreate(&temp8, input->row, 1);
		MatCreate(&temp9, input->row, 1);

		//calculation
		for (int row_i = 0; row_i < input->row; row_i++)
		{
			//numerator
			MatGetRow(input, row_i, &temp2);
#ifdef DOUBLE_MAT
			MatSubNum_DBL(&temp2, temp1.element[row_i][0], &temp3);
#else
			MatSubNum(&temp2, temp1.element[row_i][0], &temp3);
#endif
			MatMulMatElementwisePower(&temp3, 3, &temp4);
			MatMean(&temp4, dim, &temp5);
			temp6.element[row_i][0] = temp5.element[0][0];
			//denominator
			MatStd(&temp2, 1, dim, &temp7);
			temp8.element[row_i][0] = temp7.element[0][0];
			MatMulMatElementwisePower(&temp8, 3, &temp9);
		}
	}
	
	//vPortFree memory
	MatDelete(&temp1);
	MatDelete(&temp2);
	MatDelete(&temp3);
	MatDelete(&temp4);
	MatDelete(&temp5);
	MatDelete(&temp7);
	MatDelete(&temp8);

	//assignment
	if (flag)
	{
		return MatDivMatElementwise(&temp6, &temp9, output);
	}
	else
	{
		int N;
		Mat temp10;
		MatCreate(&temp10, output->row, output->col);
		if (dim==1)
		{
			N = input->row;
		}
		else
		{
			N = input->col;
		}

		MatDivMatElementwise(&temp6, &temp9, &temp10);
#ifdef DOUBLE_MAT
		return MatMulNum_DBL(&temp10, sqrt(1.0*N*(N - 1)) * temp / (N - 2), output);
#else
		return MatMulNum(&temp10, sqrt(1.0*N*(N - 1)) * temp / (N - 2), output);
#endif
		
	}
}

