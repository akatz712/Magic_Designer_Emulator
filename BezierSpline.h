// these are in common to BezierSpline

typedef struct DIV
{
	double x; // a DIV is saved for repaint It needs the x,y points of 4 touple beziers
	double y;
} DIV ;

#define maxDIVs 143748
// 143748 about 2 MB space
// 121 beziers times 4 numbers per bezier  times optionally 3X plotting
// times optionally 99 rotations minus 4 to be safe.
