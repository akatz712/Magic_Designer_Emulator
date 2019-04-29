// these are in common to BezierSpline

typedef struct DIV
{
	double x; // a DIV is saved for repaint It needs the x,y points of 4 touple beziers
	double y;
} DIV ;

#define maxDIVs 400000
// around 6mb design.

