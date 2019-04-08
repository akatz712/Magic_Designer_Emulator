// This include file defines the structure for a particular design
// created by the magic designer program.
// It must be preceded by including math.h

#include "BezierSpline.h"
#define DG_RD(ANGLE_x) (double)(ANGLE_x*3.14159265/180.)
#define RD_DG(RADIAN_x) (double)(RADIAN_x*180./3.14159265)

typedef struct tagRECT_fromWindows
{
  int left;
  int top;
  int right;
  int bottom;
} RECT_fromWindows;

// integer version of the structure. Since all sizes are relative to center_radius, this should be practical.
// for angles, use 1/10 of a degree.

typedef struct magic
{
	RECT_fromWindows magic_area; // where it is drawn is important info - these are in pixels
	int center_radius;  // the drawing area
	int gearL_radius;
	int gearR_radius;              // small gears radius sizes
	int pegL_radius;
	int pegR_radius;             // distance center to peg
	int gearR_angle;      // this is a fixed gear.
	int pegL_angle;       // this is the angle when shift lever is 70 and pegR angle is 0 (0 is right in polar coordinates)
	int pegR_angle;       // this is 0. However, on the computer different designs result if change it.
	int rotations;          // number of rotations (normally 1)
	// these are the settings the user of the original toy had control over. 
	// However, circle studs would require a change in: gearR_radius, pegR_radius (0), and gearR_angle (or gearL can be replaced by the circle stud)
	int armL_length;      // converted already in GUI
	int armR_length;      // setting letter for each arm
	int gearL_angle;       // converted by GUI from shiftlever 10=285 polar degrees, 70=225.
	int prime;            // when circles intersect do we accept prime? 0 means no. now contains arm adjust*10.
	int clipradius;      // if true, we do not draw outside the circle (more accurate less fun)
	int numpoints;       // legacy - if <= 18000, use Larry's 3.0 delta. if > 18000 use 1.0 delta
} magic ;
