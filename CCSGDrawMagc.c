#include <math.h>
#include "CCSGDrawMagc.h"

extern void controlpoints(DIV *); // calculates the bezier controlpoints
extern unsigned int gcd(unsigned int u, unsigned int v);

// Global variables
int linesdrawn;
int pointscnt;

static void makeDIV (DIV *div, double x, double y)
{
	if (linesdrawn < maxDIVs) // will only get design saved up to room
	{
		div[linesdrawn].x = x;
		div[linesdrawn].y = y;
		// end point of previous bezier
		if (linesdrawn > 0)
		{
			div[linesdrawn-1].x = x;
			div[linesdrawn-1].y = y;
		}
		pointscnt++;
		linesdrawn+=4;
	}
}

int draw_magic(magic *mg, DIV *div)
{
	double x,y;
	double xcen, ycen, xpan, ypan;	

	int i;
	double c_angles[10],delta_angles[10];
	int gcdfactor,fastest,segments;
	int loops,loopcnt;
	double deltatheta;

	// calculations done to make a final adjustment to the actual screen being drawn to.
	xcen = ( (mg->magic_area).right + (mg->magic_area).left ) / 2.;
	ycen = ( (mg->magic_area).bottom+ (mg->magic_area).top  ) / 2.;
	xpan = ( (mg->magic_area).right - (mg->magic_area).left ) / 200. / 2.;
	ypan = ( (mg->magic_area).bottom- (mg->magic_area).top  ) / 200. / 2.;

	linesdrawn = 0; // Div segments
	pointscnt = 0; // number of points

	fastest = 0;
	segments = 0;
	gcdfactor = 0;
	for (i=0;i<10;i++) {
		if (mg->m_lengths[i]!=0 && mg->m_circles[i]!=0) {
			segments++;
			if (abs(mg->m_circles[i]) > fastest) fastest = abs(mg->m_circles[i]);
			if (segments == 1) gcdfactor = fastest;
			else gcdfactor = gcd(gcdfactor,abs(mg->m_circles[i]));
		}
	}
	if (segments == 0) return 0;

	deltatheta = 3.0; // as per Larry
	if (mg->numpoints >= 54000) deltatheta = 1.0;
	loops = fastest/gcdfactor*120; if (mg->numpoints >= 54000) loops *= 3; // see if adequate precision

	for (i=0; i<10; i++) {
		if (mg->m_lengths[i]!=0 && mg->m_circles[i]!=0) {
			c_angles[i] = DG_RD((double)(mg->m_angles[i])/10.);
			delta_angles[i] = DG_RD((double)(mg->m_circles[i])*deltatheta/(double)fastest);
		}
	}

	for (loopcnt=0; loopcnt<=loops; loopcnt++) { // extra point to close shape
		x = 0.; y = 0.;
		for (i=0;i<10;i++) {
			if (mg->m_lengths[i]!=0 && mg->m_circles[i]!=0) {
				x += (double)mg->m_lengths[i]*cos(c_angles[i]);
				y += (double)mg->m_lengths[i]*sin(c_angles[i]);
			}
		}
		makeDIV (div, x*xpan + xcen, y*ypan + ycen);
		// increment angles
		for (i=0; i<10; i++) {
			if (mg->m_lengths[i]!=0 && mg->m_circles[i]!=0) {
				c_angles[i] += delta_angles[i];
			}
		}
		if (linesdrawn >= maxDIVs) break; //incomplete design
	}
	controlpoints(div);
	return (linesdrawn);
}
