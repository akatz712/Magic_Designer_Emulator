#include <math.h>
#include "DrawMagc.h"

extern int circle_circle_intersection(double,double,double,double,double,double,double *,double *,double *,double *);
extern void controlpoints(DIV *); // calculates the bezier controlpoints

// Global variables
int linesdrawn;
int pointscnt;

// shared between main code and sample_point
static double pegL_angle_start, pegR_angle_start;
static double gearL_x, gearL_y, gearR_x, gearR_y;
static float xcen, ycen, xpan, ypan;
static double armL_length_adjusted, armR_length_adjusted;

static double theta, maxtheta, deltatheta, officialdelta;

static int sample_point(magic *mg, double *xret, double *yret) // uses theta
 // returns a flag means 0 is okay, 1 is out of clipping range, 2 is could not resolve
{
	double pegL_angle, pegR_angle, pegL_x, pegL_y, pegR_x, pegR_y;
	double x, y, x_prime, y_prime;
	double r, angl;
	double x_rot, y_rot;

	// find the x,y for the pegs (which are sitting on gears which are rotating) We subtract since gears rotate in the opposite direction.
	pegL_angle = pegL_angle_start - theta * (mg->center_radius/(double)mg->gearL_radius);
	pegR_angle = pegR_angle_start - theta * (mg->center_radius/(double)mg->gearR_radius);
	pegL_x = mg->pegL_radius * cos( DG_RD(pegL_angle) ) + gearL_x;
	pegL_y = mg->pegL_radius * sin( DG_RD(pegL_angle) ) + gearL_y;
	pegR_x = mg->pegR_radius * cos( DG_RD(pegR_angle) ) + gearR_x;
	pegR_y = mg->pegR_radius * sin( DG_RD(pegR_angle) ) + gearR_y;

	// get the relative plotting point.
	if (circle_circle_intersection(			// only draw if circles intersect
				pegR_x, pegR_y, armR_length_adjusted,
				pegL_x, pegL_y, armL_length_adjusted,
				&x, &y, &x_prime, &y_prime))
	{
		if (mg->prime&1) { x=x_prime; y=y_prime; }

		// convert back to polar coordinates to rotate the drawing due to theta rotation of center wheel
		r = sqrt( x * x + y * y );
		angl = atan2( y, x ); // documentation says it fails if both x and y are 0
		x_rot = r * cos(angl - DG_RD(theta));
		y_rot = r * sin(angl - DG_RD(theta));

		// finally change coordinates to pixel frame and plot
		*xret = x_rot * xpan + xcen;
		*yret = y_rot * ypan + ycen;

		if (!(mg->clipradius&1) || r <= mg->center_radius)
			return 0;
		else
			return 1;
	}
	else
	{
		*xret = 0.;
		*yret = 0.;
		return 2;
	}
}

static void binary_search (magic *mg, int direction, double *x, double *y) // changes theta
{ //   with a valid point known, it searches for the state change point from theta back deltatheta, d is a  direction factor either 1 or -1
	double maybex, maybey;
	int i;

	deltatheta /= 2.;
	theta -= deltatheta; // start half way

	for (i=0; i < 9; i++) // loop some arbitray amount of times binary search
	{
		deltatheta /= 2.;
		if (sample_point(mg, &maybex, &maybey) != 0)
		{
			theta -= deltatheta * direction;
		}
		else
		{
			*x = maybex;
			*y = maybey;
			theta += deltatheta * direction;
		}
	}

	deltatheta = officialdelta;
}

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
	double gearL_r, gearR_r;
	double x,y;
	double armadjust;
	
	bool validplotp = false; // if there is no current valid plot point, this forces the DIV to come out
	int loop, loopMAX; // sanity checks
	
	// calculations done to make a final adjustment to the actual screen being drawn to.
	xcen = ( (mg->magic_area).right + (mg->magic_area).left ) / 2.;
	ycen = ( (mg->magic_area).bottom+ (mg->magic_area).top  ) / 2.;
	xpan = ( (mg->magic_area).right - (mg->magic_area).left ) / (float)mg->center_radius / 2.;
	ypan = ( (mg->magic_area).bottom- (mg->magic_area).top  ) / (float)mg->center_radius / 2.;

	linesdrawn = 0; // if returned as 0 is a flag that there was no design
	pointscnt = 0; // number of points since last gap

	// find coordinates of the center of gears. This coordinate system is where the center of the center plate is 0,0.
	gearL_r = mg->gearL_radius + mg->center_radius;
	gearR_r = mg->gearR_radius + mg->center_radius;
	gearL_x = gearL_r * cos( DG_RD(mg->gearL_angle/10.) );
	gearL_y = gearL_r * sin( DG_RD(mg->gearL_angle/10.) );
	gearR_x = gearR_r * cos( DG_RD(mg->gearR_angle/10.) );
	gearR_y = gearR_r * sin( DG_RD(mg->gearR_angle/10.) );
	
	// determine starting angle of left peg - spiralgragh
	// this magic numbers 225 and 315 is because I measured this peg_L angle as 270 at that position, and peg_Rangle as 0, so we need a default value
	pegL_angle_start = mg->pegL_angle/10. + (mg->gearL_angle/10.-225.) * ((mg->center_radius/(double)mg->gearL_radius)+1.);
	pegR_angle_start = mg->pegR_angle/10. + (mg->gearR_angle/10.-315.) * ((mg->center_radius/(double)mg->gearR_radius)+1.);

	// Adjust the arm lengths to reflect what is the width of the arms since they meet in a penholder.
	armadjust = (double)(mg->prime/10);
	armadjust = armadjust*armadjust;
	armL_length_adjusted = sqrt((double)(mg->armL_length)*(double)(mg->armL_length) + armadjust);
	armR_length_adjusted = sqrt((double)(mg->armR_length)*(double)(mg->armR_length) + armadjust);

	maxtheta = mg->rotations * 360.;
	loopMAX = mg->rotations * 1000;
	theta = 0.;
	deltatheta = 3.0; // as per Larry
	if (mg->numpoints >= 54000) deltatheta = 1.0;
	officialdelta = deltatheta;

	for (loop=0; loop<loopMAX; loop++) // change add sanity 1000 times max
	{
		if ( theta > maxtheta ) theta = maxtheta; // in canvas over writing is visible

		validplotp = (sample_point(mg, &x, &y) == 0);

		if (validplotp)
		{
			if ( pointscnt == 0 && theta > 0. ) //  means there was a gap
			{
				binary_search(mg, -1, &x, &y); //replace point
			}
			makeDIV (div, x, y);
		}
		else
		{ //invalid start or gap
			if ( pointscnt > 0 )
			{	// find start of gap between this and previous theta
				x = div[linesdrawn-4].x; // we know previous is a valid point
				y = div[linesdrawn-4].y;
				binary_search(mg, 1, &x, &y); //replace point
				makeDIV (div, x, y);
				controlpoints(div);
			}
		}

		if (theta == maxtheta ) break;
		theta += deltatheta;
	}  // end of for loop of theta
	controlpoints(div);

	return (linesdrawn);
}
