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

// integer version of the structure. 
// for angles, use 1/10 of a degree.

typedef struct magic
{
	RECT_fromWindows magic_area; // where it is drawn is important info - these are in pixels
	int rotations;          // number of rotations (normally 1)
	int clipradius;      // if true, we do not draw outside the circle (more accurate less fun)
	int numpoints;       // legacy - if <= 18000, use Larry's 3.0 delta. if > 18000 use 1.0 delta
	int m_circles[10];
	int m_angles[10];
	int m_lengths[10];
} magic ;
