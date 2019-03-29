/* this code is based on the following site: http://www.codeproject.com/KB/graphics/BezierSpline.aspx
License: The Code Project Open License (CPOL)

Draw a Smooth Curve through a Set of 2D Points with Bezier Primitives
By Oleg V. Polikarpotchkin, Peter Lee

Calculate piecewise Bezier curve control points to make it a spline
*/

#include "BezierSpline.h"
#include <stdlib.h>
extern int linesdrawn;
extern int pointscnt;

	/// <summary>
	/// Solves a tridiagonal system for one of coordinates (x or y)
	/// of first Bezier control points.
	/// </summary>
	/// <param name="rhs">Right hand side vector.</param>
	/// <returns>Solution vector.</returns>

static void GetFirstControlPoints(DIV *div,double *rhs,int xory)
{
	int n, index, i;
	double *tmp;  // Temp workspace.
	double b;

	n = pointscnt-1;
	tmp = (double *) malloc(n*sizeof(double));
	index = linesdrawn - pointscnt*4 + 1; // +1 is because it is first control point

	b = 2.0;

	if (xory == 0)
	{
		div[index].x = rhs[0] / b; //0th element
	}
	else
	{
		div[index].y = rhs[0] / b;
	}
	for (i = 1; i < n; i++) // Decomposition and forward substitution.
	{
		tmp[i] = 1 / b;
		b = (i < n - 1 ? 4.0 : 3.5) - tmp[i];
		if (xory == 0)
		{
			div[index+i*4].x = (rhs[i] - div[index+i*4-4].x) / b; //ith element
		}
		else
		{
			div[index+i*4].y = (rhs[i] - div[index+i*4-4].y) / b;
		}
	}
	for (i = 1; i < n; i++)
	{
		if (xory == 0)
		{
			div[index+ (n - i - 1)*4].x -= tmp[n - i] * div[index+ (n - i)*4].x; // Backsubstitution.
		}
		else
		{
			div[index+ (n - i - 1)*4].y -= tmp[n - i] * div[index+ (n - i)*4].y;
		}
	}
	free(tmp);
}

	/// <summary>
	/// Get open-ended Bezier Spline Control Points.
	/// </summary>
	/// <param name="knots">Input Knot Bezier spline points.</param>
	/// <param name="firstControlPoints">Output First Control points
	/// array of knots.Length - 1 length.</param>
	/// <param name="secondControlPoints">Output Second Control points
	/// array of knots.Length - 1 length.</param>
	/// <exception cref="ArgumentNullException"><paramref name="knots"/>
	/// parameter must be not null.</exception>
	/// <exception cref="ArgumentException"><paramref name="knots"/>
	/// array must contain at least two points.</exception>

void controlpoints(DIV *div) // this routine will fill control points.
{
	int n, index, i;
	double *rhs;

	if (pointscnt < 2) return;

	n = pointscnt-1;
	rhs = (double *) malloc(n*sizeof(double));
	index = linesdrawn - pointscnt*4;

	// Special case: Bezier curve should be a straight line. this was missing causing a bug.
	if (pointscnt == 2)
	{
		// 3P1 = 2P0 + P3
		div[index+1].x = (2 * div[index].x + div[index+4].x) / 3.0;
		div[index+1].y = (2 * div[index].y + div[index+4].y) / 3.0;
		// P2 = 2P1 - P0
		div[index+2].x = 2 * div[index+1].x - div[index].x;
		div[index+2].y = 2 * div[index+1].y - div[index].y;
	}
	else
	{

	// Set right hand side X values
	for (i = 1; i < n - 1; ++i)
		rhs[i] = 4 * div[index+i*4].x + 2 * div[index+i*4+4].x;
	rhs[0] = div[index].x + 2 * div[index+4].x;
	rhs[n - 1] = (8 * div[index+ n*4 - 4].x + div[index+ n*4].x) / 2.0;
	// Get first control points X-values
	GetFirstControlPoints(div,rhs,0);

	// Set right hand side Y values
	for (i = 1; i < n - 1; ++i)
		rhs[i] = 4 * div[index+i*4].y + 2 * div[index+i*4+4].y;
	rhs[0] = div[index].y + 2 * div[index+4].y;
	rhs[n - 1] = (8 * div[index+ n*4 - 4].y + div[index+ n*4].y) / 2.0;
	// Get first control points Y-values
	GetFirstControlPoints(div,rhs,1);
	free(rhs);

	// Fill output arrays.
	for (i = 0; i < n; ++i)
	{
		// First control point
		// Second control point
		if (i < n - 1)
		{
			div[index+i*4+2].x = 2 * div[index+i*4+4].x - div[index+i*4+5].x; // +5 1st conrtol point
			div[index+i*4+2].y = 2 * div[index+i*4+4].y - div[index+i*4+5].y;
		}
		else
		{
			div[index+i*4+2].x = (div[index+ n*4].x + div[index+ n*4 - 3].x) / 2; // -3 1st conrol point
			div[index+i*4+2].y = (div[index+ n*4].y + div[index+ n*4 - 3].y) / 2;
		}
	}
	}
	div[index+ n*4 +1].x = -10000.0; // special illegal values so everything has a value
	div[index+ n*4 +1].y = -10000.0;
	div[index+ n*4 +2].x = -10000.0;
	div[index+ n*4 +2].y = -10000.0;
	pointscnt = 0;
}
