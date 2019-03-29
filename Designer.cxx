#include <stdio.h>
#include <math.h>

#include <FL/Fl.H>
#include <FL/Fl_Menu.H>
#include <FL/Fl_Radio_Round_Button.H>
#include <FL/Fl_Repeat_Button.H>
#include <FL/Fl_Simple_Counter.H>
#include <FL/Fl_Output.H>
#include <FL/Fl_Counter.H>
#include <FL/Fl_Color_Chooser.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Native_File_Chooser.H>

#include <FL/Fl_Slider.H>
#include <FL/Fl_Check_Button.H>
#include <FL/fl_ask.H>

#define DEPTH 4

#include "DrawMagc.h"

extern int draw_magic(magic *, DIV *);

#include "armsplace1.xpm"
#include "armsplace2.xpm"
#include "armsplace3.xpm"
#include "armsplace4.xpm"
#include "armsplace5.xpm"
#include "armsplace6.xpm"
#include "pdfx.xpm"

#ifdef linux
#include "icon.xbm" // needed to place -Wno-narrowing into compiling for this
Pixmap p; //for window icon
#endif

// stuff for PDF output ----------------------------------------
#include <setjmp.h>
#include <hpdf.h>

jmp_buf env;

#ifdef HPDF_DLL
void  __stdcall
#else
void
#endif
error_handler  (HPDF_STATUS   error_no,
                HPDF_STATUS   detail_no,
                void         *user_data)
{
    printf ("ERROR: error_no=%04X, detail_no=%u\n", (HPDF_UINT)error_no,
                (HPDF_UINT)detail_no);
    longjmp(env, 1);
}

HPDF_Doc pdf;

static int hpdf_init() {
	pdf = HPDF_New (error_handler, NULL);
	if (!pdf) {
	    printf ("ERROR: cannot create pdf object.\n");
	    return 1;
	}

	if (setjmp(env)) {
	    HPDF_Free (pdf);
	    return 1;
	}
	/* set compression mode */
	HPDF_SetCompressionMode (pdf, HPDF_COMP_ALL);
	return 0;
}
//----------------------------------------------------------------------------

//---------------------------------------------------------------------------
/*
 * A Seed Fill Algorithm
 * by Paul Heckbert
 * from "Graphics Gems", Academic Press, 1990
 *
 * user provides pixelread() and pixelwrite() routines
 */

/*
 * fill.c : simple seed fill program
 * Calls pixelread() to read pixels, pixelwrite() to write pixels.
 *
 * Paul Heckbert	13 Sept 1982, 28 Jan 1987
 */

typedef int Pixel;		/* 1-channel frame buffer assumed */

typedef struct {short y, xl, xr, dy;} Segment;
/*
 * Filled horizontal segment of scanline y for xl<=x<=xr.
 * Parent segment was on line y-dy.  dy=1 or -1
 */

#define MAX 10000		/* max depth of stack */
Segment *stack, *sp;	/* stack of filled segments */

#define PUSH(Y, XL, XR, DY)	/* push new segment on stack */ \
    if (sp<stack+MAX && Y+(DY)>=0 && Y+(DY)<=height) \
    {sp->y = Y; sp->xl = XL; sp->xr = XR; sp->dy = DY; sp++;}

#define POP(Y, XL, XR, DY)	/* pop segment off stack */ \
    {sp--; Y = sp->y+(DY = sp->dy); XL = sp->xl; XR = sp->xr;}

//------------------------------------------------------------------------
int width, height;
int futurewidth, futureheight, left, top;

static void Timer_CB(void*);
bool inresize = false;
int Mode = 0;
bool jagg = false;
bool suite = true;
bool bloom_mode = false;
int bloom_idx = 0;
bool slow_mode = false;
int slow_idx = 0;

#define current 7999
DIV *DIV_buffer_ptr;
magic **magics;
DIV **beziers;
int *segs;
int *lcolor;
int *lwidth;
int *Larm;
int *Rarm;
int *Shift;
int *Armsplacement;
int designsCount;
int selected;
int backcolor;

unsigned char *image_array;   // image buffer
unsigned char *image_double;
Pixel *pixels;

Fl_Window *win, *buttons;
Fl_Button *bCorner;
Fl_Output *centerXtext,*centerYtext,*radiusXtext,*radiusYtext,*angleRtext, *traces, *selectedt, *rots, *shiftlever_e, *message;
Fl_Counter *cmorph;
Fl_Native_File_Chooser fnfc;
Fl_Radio_Round_Button *Larmb[18];
Fl_Radio_Round_Button *Rarmb[18];
Fl_Button *armsplacementb;
Fl_Slider *shiftlever;
Fl_Radio_Round_Button *Widthb[4];
Fl_Button *bColor;
Fl_Check_Button *bClip,*bShape,*bFine,*bPrime;
Fl_Counter *cLPeg, *cRPeg, *cLArm, *cRArm, *cLGearA, *cRGearA, *cLPegA, *cRPegA, *cArmAdjust;
Fl_Simple_Counter *cCenter, *cLGear, *cRGear;
Fl_Pixmap *armp[6], *PDFimage;

static int l,t,r,b;
static float radiusX, radiusY, centerX, centerY;
static int rotateL, rotateR;
static char s[140];

/*******************************************************/

extern void agg_drawc(int, int, int, DIV *, int, int, int, int, int);
extern void agg_drawrect(int, int, int, int, int, int);
extern void agg_drawellipse(int, int, int, int, int, int);
extern void agg_clearAll(int);
extern void agg_attach(unsigned char *, int, int, int);
extern void agg_setantialias(double);

void agg_draw(int x) {
	int clipshape = magics[x]->clipradius&0x3;
	if (clipshape == 0x2) 
	{ agg_drawrect(lcolor[x], magics[x]->magic_area.left, magics[x]->magic_area.top, magics[x]->magic_area.right, magics[x]->magic_area.bottom, magics[x]->numpoints%3600);
	 return; }
	if (clipshape == 0x3) 
	{ agg_drawellipse(lcolor[x], magics[x]->magic_area.left, magics[x]->magic_area.top, magics[x]->magic_area.right, magics[x]->magic_area.bottom, magics[x]->numpoints%3600);
	 return; }
	agg_drawc(lwidth[x],lcolor[x],segs[x],beziers[x], magics[x]->magic_area.left, magics[x]->magic_area.top, magics[x]->magic_area.right, magics[x]->magic_area.bottom, magics[x]->numpoints%3600);
}

void agg_this() {
	if (suite || Mode == 6) memcpy(image_array,image_double,width*height*DEPTH);
	else agg_clearAll(-1);
	if (segs[current] > 0 && Mode != 6) agg_draw(current);
}

void agg_refresh() {
	agg_clearAll(backcolor);
	for (int i=0;i<designsCount;i++) {
		if (segs[i] > 0) agg_draw(i);
		if (i%100 == 99) { win->redraw(); Fl::check(); }
	}
	memcpy(image_double,image_array,width*height*DEPTH);
	agg_this();
}

bool agg_bloom() {
	if (bloom_idx == 0) agg_clearAll(backcolor);
	for (;bloom_idx<designsCount;bloom_idx++) {
		if (segs[bloom_idx] > 0) {
			agg_draw(bloom_idx);
			bloom_idx++;
			return true;
		}
	}
	return false;
}

bool agg_slow() {
	agg_clearAll(-1);
	slow_idx+=4;
	agg_drawc(lwidth[current],lcolor[current],slow_idx,beziers[current], magics[current]->magic_area.left, magics[current]->magic_area.top, magics[current]->magic_area.right, magics[current]->magic_area.bottom, magics[current]->numpoints%3600);
	return (slow_idx<segs[current]);
}

/************************************************************************/

	const int Advanced_default_all[6][12] = {{6000,500,1000,0,750,10500,10500,2350,1800,0,1350,1},
		{6000,500,1000,0,750,10500,10500,3150,1800,0,1350,1},
		{6000,500,500,0,0,10500,10500,0,1800,0,0,1},
		{6000,1000,1000,750,750,10500,10500,3150,2350,450,1350,1},
		{6000,1000,500,750,0,10500,10500,0,2350,450,0,1},
		{6000,1000,500,750,0,10500,10500,0,3150,450,0,1}};
	const int Advanced_shift_all[6] = {11, 0, 0, 12, 12, 0}; // 12 L 11 R Gear angle
	int MGSmagic[26];

void processMGSmagic(magic * magic_ptr) {
	if (MGSmagic[24] != 0) {
		if (MGSmagic[20] == 0) MGSmagic[20] = 4; // arms placement default
		for (int i=4; i<16; i++) MGSmagic[i] = Advanced_default_all[MGSmagic[20]-1][i-4];
		MGSmagic[16] = 6251;
		int temp = 2950 - MGSmagic[19]*10; // shift
		int temp2 = Advanced_shift_all[MGSmagic[20]-1];
		if (temp2 > 0) MGSmagic[temp2] = temp;
		MGSmagic[9] = 7000 + (10 - (MGSmagic[17]-64)) * 500; // Larm
		MGSmagic[10] = 7000 + (10 - (MGSmagic[18]-64)) * 500; // Rarm
		MGSmagic[24] = 0;
	}
	if (MGSmagic[25] < 54000) MGSmagic[25] %= 3600; // this legacy not explicit but found 3600/7200 in old files or 18000/54000 in newer files
	if (MGSmagic[17] < 65 || MGSmagic[17] > 82) MGSmagic[17] = 90; // Z is used for out of bounds arms
	if (MGSmagic[18] < 65 || MGSmagic[18] > 82) MGSmagic[18] = 90;
//	this.rect[0] = (float)MGSmagic[0];
//	this.rect[1] = (float)MGSmagic[1];
//	this.rect[2] = (float)MGSmagic[2];
//	this.rect[3] = (float)MGSmagic[3];
	magic_ptr->magic_area.left = MGSmagic[0];
	magic_ptr->magic_area.top = MGSmagic[1];
	magic_ptr->magic_area.right = MGSmagic[2];
	magic_ptr->magic_area.bottom = MGSmagic[3];
	magic_ptr->center_radius = MGSmagic[4];
	magic_ptr->gearL_radius = MGSmagic[5];
	magic_ptr->gearR_radius = MGSmagic[6];
	magic_ptr->pegL_radius = MGSmagic[7];
	magic_ptr->pegR_radius = MGSmagic[8];
	magic_ptr->gearR_angle = MGSmagic[11];
	magic_ptr->pegL_angle = MGSmagic[13];
	magic_ptr->pegR_angle = MGSmagic[14];
	magic_ptr->rotations = MGSmagic[15];
	magic_ptr->armL_length = MGSmagic[9];
	magic_ptr->armR_length = MGSmagic[10];
	magic_ptr->gearL_angle = MGSmagic[12];
	magic_ptr->prime = MGSmagic[16];
	magic_ptr->clipradius = MGSmagic[21];
	magic_ptr->numpoints = MGSmagic[25];
}

void add_design (int index, int larm, int rarm, int shift, int armp, int c, int w, magic* mg) {
	Larm[index] = larm;
	Rarm[index] = rarm;
	Shift[index] = shift;
	Armsplacement[index] = armp;
	lcolor[index] = c;
	lwidth[index] = w;
	magics[index] = mg;
	segs[index] = draw_magic(magics[index], DIV_buffer_ptr);
	beziers[index] = (DIV *)malloc(sizeof(DIV)*segs[index]+8);
	memcpy(beziers[index],DIV_buffer_ptr,sizeof(DIV)*segs[index]+8);
}

void update_design () {
	int prev_segs = segs[current];
	free(beziers[current]);
	segs[current] = draw_magic(magics[current], DIV_buffer_ptr);
	beziers[current] = (DIV *)malloc(sizeof(DIV)*segs[current]+8);
	memcpy(beziers[current],DIV_buffer_ptr,sizeof(DIV)*segs[current]+8);
	if (segs[current] == 0 && prev_segs > 0) message->value("The current design cannot be drawn.");
	if (segs[current] > 0 && prev_segs == 0) message->value("");
}

int loadMGS(const char * filename) {
	FILE *in;
	int ret,count=0;
	in = fl_fopen(filename, "r");
	fscanf(in,"{%d}\n",&backcolor); // is in 0BGR -> RGBFF
	while(true) {
		ret = fscanf(in,"{%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d}\n",
		&MGSmagic[0],&MGSmagic[1],&MGSmagic[2],&MGSmagic[3],&MGSmagic[4],&MGSmagic[5],&MGSmagic[6],&MGSmagic[7],&MGSmagic[8],&MGSmagic[9],
		&MGSmagic[10],&MGSmagic[11],&MGSmagic[12],&MGSmagic[13],&MGSmagic[14],&MGSmagic[15],&MGSmagic[16],&MGSmagic[17],&MGSmagic[18],&MGSmagic[19],
		&MGSmagic[20],&MGSmagic[21],&MGSmagic[22],&MGSmagic[23],&MGSmagic[24],&MGSmagic[25]);
		if (ret != 26) break; // eof error or corrupt file
		magic * mg = new magic;
		processMGSmagic(mg);
		add_design(designsCount, MGSmagic[17], MGSmagic[18], MGSmagic[19], MGSmagic[20], MGSmagic[22], MGSmagic[23], mg);
		designsCount++; count++;
	}
	fclose(in);
	return count;
}

void saveShape(int x) { //6000,500,500,0,0,9500,8000,0,1800,0,0,1,10001,69,72,60,3, - circle to match shape
	if ((magics[x]->clipradius&0x2) == 0x2) {
		magics[x]->center_radius = 6000;
		magics[x]->gearL_radius = 500;
		magics[x]->gearR_radius = 500;
		magics[x]->pegL_radius = 0;
		magics[x]->pegR_radius = 0;
		magics[x]->armL_length = 9500;
		magics[x]->armR_length = 8000;
		magics[x]->gearR_angle = 0;
		magics[x]->gearL_angle = 1800;
		magics[x]->pegL_angle = 0;
		magics[x]->pegR_angle = 0;
		magics[x]->rotations = 1;
		magics[x]->prime = 10001;
		Larm[x] = 69;
		Rarm[x] = 72;
		Shift[x] = 60;
		Armsplacement[x] = 3;
	}
}

void saveMGS(const char * filename,int start, int end) {
	FILE *out;
	out = fl_fopen(filename, "w"); 
	fprintf(out,"{%d}\n",backcolor&0x00FFFFFF);
	for (int i=start-1;i<end;i++)
			fprintf(out,"{%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d}\n",
				magics[i]->magic_area.left,
				magics[i]->magic_area.top,
				magics[i]->magic_area.right,
				magics[i]->magic_area.bottom,
				magics[i]->center_radius,
				magics[i]->gearL_radius,
				magics[i]->gearR_radius,
				magics[i]->pegL_radius,
				magics[i]->pegR_radius,
				magics[i]->armL_length,
				magics[i]->armR_length,
				magics[i]->gearR_angle,
				magics[i]->gearL_angle,
				magics[i]->pegL_angle,
				magics[i]->pegR_angle,
				magics[i]->rotations,
				magics[i]->prime,
				Larm[i],
				Rarm[i],
				Shift[i],
				Armsplacement[i],
				magics[i]->clipradius,
				lcolor[i],
				lwidth[i],
				0,
				magics[i]->numpoints);
	fclose(out);
}

void savePDF(const char * filename) {

	if (hpdf_init()) return; // sets compression
	HPDF_Page page_1;
	page_1 = HPDF_AddPage (pdf);
	HPDF_Page_SetWidth (page_1, width);
	HPDF_Page_SetHeight (page_1, height);
	int c = backcolor&0x00FFFFFF;
    HPDF_Page_SetRGBFill (page_1, (c&0xFF)/255.0F, ((c>>8)&0xFF)/255.0F, ((c>>16)&0xFF)/255.0F);
    HPDF_Page_Rectangle (page_1, 0, 0, width, height);
    HPDF_Page_Fill (page_1);

	for (int i=0;i<designsCount;i++) {
		if (segs[i] > 0) {
			c = lcolor[i]; //design.getMGScolor();
			if (((magics[i]->clipradius)&0x2) == 0x2) {
			    HPDF_Page_SetRGBFill (page_1, (c&0xFF)/255.0F, ((c>>8)&0xFF)/255.0F, ((c>>16)&0xFF)/255.0F);
				float left = (float) magics[i]->magic_area.left; // Need to test shapes which are rotated.
				float top = (float) magics[i]->magic_area.top;
				float right = (float) magics[i]->magic_area.right;
				float bottom = (float) magics[i]->magic_area.bottom;
				float temp; //test rotated shape
				if (left > right) { temp = right; right = left; left = temp; }
				if (top > bottom) { temp = bottom; bottom = top; top = temp; }
				if (((magics[i]->clipradius)&0x1) == 0x1) { // ellipse
					float KAPPA = 0.552F; // 4.0 * ((Math.sqrt(2) - 1) / 3.0)
					float rx = (right-left)/2.0F;
					float ry = (bottom-top)/2.0F;
					float cx = left+rx;
					float cy = top+ry;
					float H = (float) height;
				    HPDF_Page_MoveTo (page_1, cx, H - (cy - ry));
					HPDF_Page_CurveTo(page_1, cx + (KAPPA * rx), H - (cy - ry), cx + rx, H - (cy - (KAPPA * ry)), cx + rx, H - (cy));
					HPDF_Page_CurveTo(page_1, cx + rx, H - (cy + (KAPPA * ry)), cx + (KAPPA * rx), H - (cy + ry), cx, H - (cy + ry));
					HPDF_Page_CurveTo(page_1, cx - (KAPPA * rx), H - (cy + ry), cx - rx, H - (cy + (KAPPA * ry)), cx - rx, H - (cy));
					HPDF_Page_CurveTo(page_1, cx - rx, H - (cy - (KAPPA * ry)), cx - (KAPPA * rx), H - (cy - ry), cx, H - (cy - ry));
				    HPDF_Page_Fill (page_1);
				} else {
				    HPDF_Page_Rectangle (page_1, left, height-bottom, right-left, bottom-top);
				    HPDF_Page_Fill (page_1);
				}
			} else {
			    HPDF_Page_SetRGBStroke (page_1, (c&0xFF)/255.0F, ((c>>8)&0xFF)/255.0F, ((c>>16)&0xFF)/255.0F);
			    HPDF_Page_SetLineWidth (page_1, (float) lwidth[i]); //need to test jagged and smooth with different widths
				DIV *Divs;
				Divs = beziers[i];
				for (int j=0; j< segs[i]; j+=4) {
					if (Divs[j+1].x > -10000.0) {
					    HPDF_Page_MoveTo (page_1, Divs[j].x, height - Divs[j].y);
						HPDF_Page_CurveTo(page_1, Divs[j+1].x, height - Divs[j+1].y, Divs[j+2].x, height - Divs[j+2].y, Divs[j+3].x, height - Divs[j+3].y);
					    HPDF_Page_Stroke (page_1);
					}
				}
			}
		}
	}
	HPDF_SaveToFile (pdf, filename);
	HPDF_Free (pdf);
}

void default_shape() {
	magic * magic_ptr = new magic;
//	this.rect[0] = scrW/2.F - scrm/2.F; // =(mg->magic_area).left
//	this.rect[1] = scrH/2.F + scrm/2.F; //  =(mg->magic_area).top
//	this.rect[2] = scrW/2.F + scrm/2.F; //  =(mg->magic_area).right
//	this.rect[3] = scrH/2.F - scrm/2.F; //  =(mg->magic_area).bottom
	int m = width;
	if (height < width) m = height;
	int Larm = 67; //C
	int Rarm = 67;
	int shift = 60;
	int armp = 4;
	magic_ptr->magic_area.left = (width - m)/2;
	magic_ptr->magic_area.top = (height + m)/2;
	magic_ptr->magic_area.right = (width + m)/2;
	magic_ptr->magic_area.bottom = (height - m)/2;
	magic_ptr->center_radius = 6000;
	magic_ptr->gearL_radius = 1000;
	magic_ptr->gearR_radius = 1000;
	magic_ptr->pegL_radius = 750;
	magic_ptr->pegR_radius = 750;
	magic_ptr->gearR_angle = 3150;
	magic_ptr->pegL_angle = 450;
	magic_ptr->pegR_angle = 1350;
	magic_ptr->rotations = 1;
	magic_ptr->armL_length = 7000 + (10 - (Larm-64)) * 500;
	magic_ptr->armR_length = 7000 + (10 - (Rarm-64)) * 500;
	magic_ptr->gearL_angle = 2950 - shift*10;
	magic_ptr->prime = 6251;
	magic_ptr->clipradius = 0;
	magic_ptr->numpoints = 0;
	add_design(current, Larm, Rarm, shift, armp, 0x00000060, 2, magic_ptr);
}

void selectDesign() {
	segs[current] = segs[selected];
	lcolor[current] = lcolor[selected];
	lwidth[current] = lwidth[selected];
	Larm[current] = Larm[selected];
	Rarm[current] = Rarm[selected];
	Shift[current] = Shift[selected];
	Armsplacement[current] = Armsplacement[selected];
	memcpy(magics[current],magics[selected],sizeof(magic));
	free(beziers[current]);
	beziers[current] = (DIV *)malloc(sizeof(DIV)*segs[selected]+8);
	memcpy(beziers[current],beziers[selected],sizeof(DIV)*segs[selected]+8);
}

int closestDesign(int xpos, int ypos) { // Concern with rotated designs
	int i,j,n, des = -1;
	double xx, yy, diffx, diffy, diffsofar = 10000.0;
	DIV *Divs;
	xx = (double)xpos;
	yy = (double)ypos;
	for(j = 0; j < designsCount; j++) {
		n = segs[j];
		Divs = beziers[j];
		for(i = 0; i < n; i++) {
			diffx = Divs[i].x - xx;
			if (diffx < 0) diffx = 0 - diffx;
			diffy = Divs[i].y - yy;
			if (diffy < 0) diffy = 0 - diffy;
			diffx = diffx + diffy;
			if (diffx < diffsofar) { diffsofar = diffx; des = j; }
			if (diffsofar < 2.0) return des;
		}
	}
	return des;
}

void copyDesign(int index) {
	magics[index] = new magic;
	memcpy(magics[index],magics[current],sizeof(magic));
	beziers[index] = (DIV *)malloc(sizeof(DIV)*segs[current]+8);
	memcpy(beziers[index],beziers[current],sizeof(DIV)*segs[current]+8);
	segs[index] = segs[current];
	lcolor[index] = lcolor[current];
	lwidth[index] = lwidth[current];
	Larm[index] = Larm[current];
	Rarm[index] = Rarm[current];
	Shift[index] = Shift[current];
	Armsplacement[index] = Armsplacement[current];
}

bool anyChanges() {
	if (lcolor[selected] != lcolor[current]) return true;
	if (lwidth[selected] != lwidth[current]) return true;
	if (memcmp(magics[selected],magics[current],sizeof(magic)) != 0) return true;
	return false;
}

int isMorphallowed() {
	if (magics[selected]->center_radius != magics[current]->center_radius ||
		magics[selected]->gearL_radius != magics[current]->gearL_radius ||
		magics[selected]->gearR_radius != magics[current]->gearR_radius ||
		magics[selected]->rotations != magics[current]->rotations) return 3; // cannot morph between different sidedness
	if ((magics[selected]->prime&0x1) != (magics[current]->prime&0x1) ||
		magics[selected]->clipradius != magics[current]->clipradius ||
		(magics[selected]->numpoints>=54000) != (magics[current]->numpoints>=54000) ||
		lwidth[selected] != lwidth[current]) return 4; // cannot morph on different discrete values
	return 0;
}

static float deltared, deltagreen, deltablue, deltaarmadjust, deltaangleR;
static float delta12[12];
static int deltaLarm, deltaRarm, deltaShift;

void makedeltas(int morph_num) {
	deltared = ((float)(lcolor[current]&0xFF) - (float)(lcolor[selected]&0xFF))/morph_num;
	deltagreen = ((float)((lcolor[current]>>8)&0xFF) - (float)((lcolor[selected]>>8)&0xFF))/morph_num;
	deltablue = ((float)((lcolor[current]>>16)&0xFF) - (float)((lcolor[selected]>>16)&0xFF))/morph_num;
	delta12[0] = ((float)magics[current]->magic_area.left - (float)magics[selected]->magic_area.left)/morph_num;
	delta12[1] = ((float)magics[current]->magic_area.top - (float)magics[selected]->magic_area.top)/morph_num;
	delta12[2] = ((float)magics[current]->magic_area.right - (float)magics[selected]->magic_area.right)/morph_num;
	delta12[3] = ((float)magics[current]->magic_area.bottom - (float)magics[selected]->magic_area.bottom)/morph_num;
	delta12[4] = ((float)magics[current]->pegL_radius - (float)magics[selected]->pegL_radius)/morph_num;
	delta12[5] = ((float)magics[current]->pegR_radius - (float)magics[selected]->pegR_radius)/morph_num;
	delta12[6] = ((float)magics[current]->armL_length - (float)magics[selected]->armL_length)/morph_num;
	delta12[7] = ((float)magics[current]->armR_length - (float)magics[selected]->armR_length)/morph_num;
	delta12[8] = ((float)magics[current]->gearR_angle - (float)magics[selected]->gearR_angle)/morph_num;
	delta12[9] = ((float)magics[current]->gearL_angle - (float)magics[selected]->gearL_angle)/morph_num;
	delta12[10] = ((float)magics[current]->pegL_angle - (float)magics[selected]->pegL_angle)/morph_num;
	delta12[11] = ((float)magics[current]->pegR_angle - (float)magics[selected]->pegR_angle)/morph_num;
	deltaarmadjust = ((float)(magics[current]->prime/10) - (float)(magics[selected]->prime/10))/morph_num;
	deltaangleR = ((float)(magics[current]->numpoints%3600) - (float)(magics[selected]->numpoints%3600))/morph_num;
	if (Larm[current] == Larm[selected]) deltaLarm = Larm[selected]; else deltaLarm = 90; // Z
	if (Rarm[current] == Rarm[selected]) deltaRarm = Rarm[selected]; else deltaRarm = 90;
	if (Shift[current] == Shift[selected]) deltaShift = Shift[selected]; else deltaShift = 0;
}

void applydeltas(int factor) {
	lcolor[current] = ((int)round((float)(lcolor[selected]&0xFF)+deltared*factor)) |
		((int)round((float)((lcolor[selected]>>8)&0xFF)+deltagreen*factor)<<8) |
		((int)round((float)((lcolor[selected]>>16)&0xFF)+deltablue*factor)<<16);
	magics[current]->magic_area.left = (int)round((float)magics[selected]->magic_area.left + delta12[0]*factor);
	magics[current]->magic_area.top = (int)round((float)magics[selected]->magic_area.top + delta12[1]*factor);
	magics[current]->magic_area.right = (int)round((float)magics[selected]->magic_area.right + delta12[2]*factor);
	magics[current]->magic_area.bottom = (int)round((float)magics[selected]->magic_area.bottom + delta12[3]*factor);
	magics[current]->pegL_radius = (int)round((float)magics[selected]->pegL_radius + delta12[4]*factor);
	magics[current]->pegR_radius = (int)round((float)magics[selected]->pegR_radius + delta12[5]*factor);
	magics[current]->armL_length = (int)round((float)magics[selected]->armL_length + delta12[6]*factor);
	magics[current]->armR_length = (int)round((float)magics[selected]->armR_length + delta12[7]*factor);
	magics[current]->gearR_angle = (int)round((float)magics[selected]->gearR_angle + delta12[8]*factor);
	magics[current]->gearL_angle = (int)round((float)magics[selected]->gearL_angle + delta12[9]*factor);
	magics[current]->pegL_angle = (int)round((float)magics[selected]->pegL_angle + delta12[10]*factor);
	magics[current]->pegR_angle = (int)round((float)magics[selected]->pegR_angle + delta12[11]*factor);
	magics[current]->prime = ((int)round((float)(magics[selected]->prime/10) + deltaarmadjust*factor)) *10 + (magics[current]->prime&0x1);
	magics[current]->numpoints = ((int)round((float)(magics[selected]->numpoints%3600) + deltaangleR*factor)) + ((magics[current]->numpoints>=54000)?54000:0);
	Larm[current] = deltaLarm; Rarm[current] = deltaRarm; Shift[current] = deltaShift;
}

/************************************************************************/
/*
 * fill: set the pixel at (x,y) and all of its 4-connected neighbors
 * with the same pixel value to the new pixel value nv.
 * A 4-connected neighbor is a pixel above, below, left, or right of a pixel.
 */

void fill(int x, int y, Pixel nv) /* seed point */ /* new pixel value */
{
    int l, x1, x2, dy;
    Pixel ov;	/* old pixel value */

    ov = pixels[y*width+x];		/* read pv at seed point */
    if (ov==nv || x<0 || x>width || y<0 || y>height) return;
    PUSH(y, x, x, 1);			/* needed in some cases */
    PUSH(y+1, x, x, -1);		/* seed segment (popped 1st) */

    while (sp>stack) {
	/* pop segment off stack and fill a neighboring scan line */
	POP(y, x1, x2, dy);
	/*
	 * segment of scan line y-dy for x1<=x<=x2 was previously filled,
	 * now explore adjacent pixels in scan line y
	 */
	for (x=x1; x>=0 && pixels[y*width+x]==ov; x--)
	    pixels[y*width+x] = nv;
	if (x>=x1) goto skip;
	l = x+1;
	if (l<x1) PUSH(y, l, x1-1, -dy);		/* leak on left? */
	x = x1+1;
	do {
	    for (; x<=width && pixels[y*width+x]==ov; x++)
		pixels[y*width+x] = nv;
	    PUSH(y, l, x-1, dy);
	    if (x>x2+1) PUSH(y, x2+1, x-1, -dy);	/* leak on right? */
skip:	    for (x++; x<=x2 && pixels[y*width+x]!=ov; x++);
	    l = x;
	} while (x<=x2);
    }
}
//----------------------------------------------------------------------------------------
unsigned int gcd(unsigned int u, unsigned int v)
{
  int shift;
 
  /* GCD(0,v) == v; GCD(u,0) == u, GCD(0,0) == 0 */
  if (u == 0) return v;
  if (v == 0) return u;
 
  /* Let shift := lg K, where K is the greatest power of 2
        dividing both u and v. */
  for (shift = 0; ((u | v) & 1) == 0; ++shift) {
         u >>= 1;
         v >>= 1;
  }
 
  while ((u & 1) == 0)
    u >>= 1;
 
  /* From here on, u is always odd. */
  do {
       /* remove all factors of 2 in v -- they are not common */
       /*   note: v is not zero, so while will terminate */
       while ((v & 1) == 0)  /* Loop X */
           v >>= 1;
 
       /* Now u and v are both odd. Swap if necessary so u <= v,
          then set v = v - u (which is even). For bignums, the
          swapping is just pointer movement, and the subtraction
          can be done in-place. */
       if (u > v) {
         unsigned int t = v; v = u; u = t;}  // Swap u and v.
       v = v - u;                       // Here v >= u.
     } while (v != 0);
 
  /* restore common factors of 2 */
  return u << shift;
}
//------------------------------------------------------------------------------

void calc_arms (int flag) {
	if (Armsplacement[current] == 4) {
		if (flag == 1) magics[current]->armL_length =
		 (int)((magics[current]->center_radius + magics[current]->gearL_radius) + (10 - (Larm[current]-64)) * (float)(magics[current]->center_radius)/12.0 + 0.5);
		if (flag == 2) magics[current]->armR_length =
		 (int)((magics[current]->center_radius + magics[current]->gearR_radius) + (10 - (Rarm[current]-64)) * (float)(magics[current]->center_radius)/12.0 + 0.5);
	} else {
		if (flag == 1) magics[current]->armL_length = 7000 + (10 - (Larm[current]-64)) * 500;
		if (flag == 2) magics[current]->armR_length = 7000 + (10 - (Rarm[current]-64)) * 500;
	}
}

void Gear_cb(Fl_Widget*, void *) {
	int center, left, right, v, factor;
	center = cCenter->value();
	left = cLGear->value();
	right = cRGear->value();
	v = left*right/gcd(left, right);
	v = v/gcd(v, center);
	if (v > 99)	v = 99;
	factor = (int)(7000.0 / (center + (float)(left+right)/2.0));
	magics[current]->center_radius = center * factor;
	magics[current]->gearL_radius = left * factor;
	magics[current]->gearR_radius = right * factor;
	magics[current]->rotations = v;
	sprintf(s,"%d",magics[current]->rotations);
	rots->value(s);
	if (Larm[current] == 90) { Larm[current] = 67; Larmb[Larm[current]-65]->setonly(); } // reset to defaults
	if (Rarm[current] == 90) { Rarm[current] = 67; Rarmb[Rarm[current]-65]->setonly(); }
	calc_arms(1);
	calc_arms(2);
	cLArm->value(magics[current]->armL_length);
	cRArm->value(magics[current]->armR_length);
	update_design();
	agg_this();
	win->redraw();
}

void showC() {
	l = magics[current]->magic_area.left;
	t = magics[current]->magic_area.top;
	r = magics[current]->magic_area.right;
	b = magics[current]->magic_area.bottom;
	sprintf(s,"%d",(int) round((l+r)/2.F));
	centerXtext->value(s);
	sprintf(s,"%d",(int) round((t+b)/2.F));
	centerYtext->value(s);
}

void showR() {
	l = magics[current]->magic_area.left;
	t = magics[current]->magic_area.top;
	r = magics[current]->magic_area.right;
	b = magics[current]->magic_area.bottom;
	sprintf(s,"%d",(int) round(abs(l-r)/2.F));
	radiusXtext->value(s);
	sprintf(s,"%d",(int) round(abs(t-b)/2.F));
	radiusYtext->value(s);
}

void showT() {
	sprintf(s,"%d",designsCount);
	traces->value(s);
	sprintf(s,"%d",selected+1);
	selectedt->value(s);
}

void adjust_bCorner() {
	int alignment = FL_ALIGN_INSIDE;
	l = magics[current]->magic_area.left;
	t = magics[current]->magic_area.top;
	r = magics[current]->magic_area.right;
	b = magics[current]->magic_area.bottom;
	if (l < r) alignment |= FL_ALIGN_LEFT;
	else  alignment |= FL_ALIGN_RIGHT;
	if (t < b) alignment |= FL_ALIGN_TOP;
	else  alignment |= FL_ALIGN_BOTTOM;
	bCorner->align(alignment);
	bCorner->redraw_label();
}

void show_gears() {
	int g, temp;
	if (Armsplacement[current] == 4) {
		cCenter->show();
		cLGear->show();
		cRGear->show();
	} else {
		cCenter->hide();
		cLGear->hide();
		cRGear->hide();
		return;
	}
	g = gcd(magics[current]->gearL_radius,magics[current]->gearR_radius);
	g = gcd(g,magics[current]->center_radius);
	temp = magics[current]->center_radius/g; if (temp>13 || temp < 1) temp = 6;
	cCenter->value(temp);
	temp = magics[current]->gearL_radius/g; if (temp>13 || temp < 1) temp = 1;
	cLGear->value(temp);
	temp = magics[current]->gearR_radius/g; if (temp>13 || temp < 1) temp = 1;
	cRGear->value(temp);
}

void set_controls() { // this will have to deal with advanced values for arms/shift
	int j;
	showC();
	showR();
	sprintf(s,"%d",magics[current]->numpoints%3600);
	angleRtext->value(s);
	adjust_bCorner();
	if (Larm[current] != 90) Larmb[Larm[current]-65]->setonly();
	else for (j=0; j<18; j++) Larmb[j]->clear();
	if (Rarm[current] != 90) Rarmb[Rarm[current]-65]->setonly();
	else for (j=0; j<18; j++) Rarmb[j]->clear();
	if (Shift[current] != 0) shiftlever->value(Shift[current]);
	sprintf(s,"%d",Shift[current]);
	shiftlever_e->value(s);
	Widthb[lwidth[current]-1]->setonly();
	bColor->color(fl_rgb_color(lcolor[current] & 0xFF, (lcolor[current]>>8) & 0xFF, (lcolor[current]>>16) & 0xFF));
	bColor->labelcolor(fl_contrast(bColor->labelcolor(),bColor->color()));
	bColor->redraw();
	armsplacementb->image(armp[Armsplacement[current]-1]);
	armsplacementb->redraw();
	bClip->value(magics[current]->clipradius&0x1);
	bShape->value(magics[current]->clipradius&0x2);
	bFine->value(magics[current]->numpoints >= 54000);
	bPrime->value(magics[current]->prime&0x1);
	show_gears();
	cLPeg->value(magics[current]->pegL_radius);
	cRPeg->value(magics[current]->pegR_radius);
	cLArm->value(magics[current]->armL_length);
	cRArm->value(magics[current]->armR_length);
	cLGearA->value(magics[current]->gearL_angle);
	cRGearA->value(magics[current]->gearR_angle);
	cLPegA->value(magics[current]->pegL_angle);
	cRPegA->value(magics[current]->pegR_angle);
	cArmAdjust->value(magics[current]->prime/10);
	sprintf(s,"%d",magics[current]->rotations);
	rots->value(s);
}

void Add_CB(Fl_Widget*, void *) {
	int ret,morph;
	if (!suite) { message->value("Suite must be visible to add designs."); return; }
	if (Mode == 6) { message->value("Cannot add designs in paint mode."); return; }
	if (segs[current] == 0) { message->value("The current design cannot be drawn."); return; }
	if (designsCount == 0 || selected == -1) morph = 1;
	else {
		morph = cmorph->value();
		if (!anyChanges()) { message->value("Nothing to Add."); return; }
	}
	if (morph == 1) {
		copyDesign(designsCount);
		saveShape(designsCount);
		memcpy(image_double,image_array,width*height*DEPTH);
		designsCount++;
		message->value("Added design to the suite.");
		selected = designsCount-1;
		showT();
	} else {
		ret = isMorphallowed();
		if (ret == 3) { message->value("Cannot morph between different sidedness."); return; }
		if (ret == 4) { message->value("Cannot morph on different discrete values."); return; }
		makedeltas(morph);
		for (int i=1; i<=morph; i++) {
			applydeltas(i);
			update_design();
			agg_this();
			copyDesign(designsCount);
			saveShape(designsCount);
			memcpy(image_double,image_array,width*height*DEPTH);
			designsCount++;
		}
		sprintf(s,"Added %d morphed designs to the suite.", morph); message->value(s);
		selected = designsCount-1;
		showT();
		win->redraw();
	}
}

void Load_CB(Fl_Widget*, void *) {
	int flag;
	fnfc.title("Append a file to the Design suite");
	fnfc.type(Fl_Native_File_Chooser::BROWSE_FILE);
	fnfc.filter("MGS Files\t*.{MGS,mgs}");
	//fnfc.directory("/var/tmp");           // default directory to use

	switch ( fnfc.show() ) {
		case -1:
		sprintf(s,"ERROR: %s", fnfc.errmsg()); message->value(s); break;  // ERROR
		case  1:
		break;  // CANCEL
		default:
		flag = loadMGS(fnfc.filename());
		agg_refresh(); // now provides feedback.
		if (flag>0) { sprintf(s,"Loaded %d from: %s",flag, fl_filename_name(fnfc.filename())); message->value(s); }
		win->redraw();
		showT();
		break;  // FILE CHOSEN
	}
}

void Save_CB(Fl_Widget*, void *) {
	int start, end;
	if (designsCount == 0) { message->value("The design suite is empty."); return; }
	if (selected > -1) {
		start = selected + 1;
		end = start + cmorph->value() - 1;
		if (end > designsCount) end = designsCount;
		switch ( fl_choice("Do you want to save slice(%d-%d) of the suite or the entire thing", "Slice", "Entire", 0, start, end) ) {
			case 0:  // Yes
			break;
			case 1:  // No (default)
			start = 1;
			end = designsCount;
			break;
		}
	}
	else {
		start = 1;
		end = designsCount;
	}

	fnfc.title("Save the Design suite to a file");
	fnfc.type(Fl_Native_File_Chooser::BROWSE_SAVE_FILE);
	fnfc.filter("MGS Files\t*.{MGS,mgs}");
	fnfc.preset_file("File.MGS");
	fnfc.options(Fl_Native_File_Chooser::SAVEAS_CONFIRM|Fl_Native_File_Chooser::USE_FILTER_EXT|Fl_Native_File_Chooser::NEW_FOLDER );

	switch ( fnfc.show() ) {
		case -1:
		sprintf(s,"ERROR: %s", fnfc.errmsg()); message->value(s); break;  // ERROR
		case  1:
		break;  // CANCEL
		default:
		saveMGS(fnfc.filename(),start,end);
		sprintf(s,"Saved (%d-%d): %s",start,end,fl_filename_name(fnfc.filename())); message->value(s);
		break;  // FILE CHOSEN
	}
}

void PDF_CB(Fl_Widget*, void *) {
	if (designsCount == 0) { message->value("The design suite is empty."); return; }
	fnfc.title("Create a PDF file showing the Design suite");
	fnfc.type(Fl_Native_File_Chooser::BROWSE_SAVE_FILE);
	fnfc.filter("PDF Files\t*.pdf");
	fnfc.preset_file("File.pdf");
	fnfc.options(Fl_Native_File_Chooser::SAVEAS_CONFIRM|Fl_Native_File_Chooser::USE_FILTER_EXT|Fl_Native_File_Chooser::NEW_FOLDER );

	switch ( fnfc.show() ) {
		case -1:
		sprintf(s,"ERROR: %s", fnfc.errmsg()); message->value(s); break;  // ERROR
		case  1:
		break;  // CANCEL
		default:
		savePDF(fnfc.filename());
		sprintf(s,"Saved: %s", fl_filename_name(fnfc.filename())); message->value(s);
		break;  // FILE CHOSEN
	}
}

void Delete_CB(Fl_Widget*, void *) { // tbd: delete what was just added
	int count;
	if (designsCount == 0) { message->value("The design suite is empty."); return; }
	count = cmorph->value();
	if (count > designsCount) count = designsCount;
	switch ( fl_choice("Are you sure you want to delete designs (%d-%d)?", "Yes", "No", 0, designsCount-count+1, designsCount) ) {
		case 0:  // Yes
		for (int i=1; i<=count; i++) {
			designsCount--;
			delete magics[designsCount];
			free(beziers[designsCount]);
			segs[designsCount] = 0;
		}
		agg_refresh();
		win->redraw();
		if (selected >= designsCount) selected = -1;
		showT();
		sprintf(s,"Deleted %d designs.",count);
		message->value(s);
		case 1:  return; // No (default)
	}
}

static void Timer_Bloom_CB(void*) {
	if (agg_bloom())
		Fl::repeat_timeout(0.05, Timer_Bloom_CB,(void*)0);
	else {
		bloom_mode = false;
		bloom_idx = 0;
		agg_this();
		buttons->show();
	}
	win->redraw();
}

void Bloom_CB(Fl_Widget*, void *) {
	bloom_mode = true;
	bloom_idx = 0;
	buttons->hide();
	Fl::add_timeout(0.05, Timer_Bloom_CB,(void*)0);
}

void Kill_Bloom_CB(Fl_Widget*, void *) {
	Fl::remove_timeout(Timer_Bloom_CB);
	bloom_mode = false;
	bloom_idx = 0;
	agg_this();
	buttons->show();
	win->redraw();
}

static void Timer_Slow_CB(void*) {
	if (agg_slow())
		Fl::repeat_timeout(0.2, Timer_Slow_CB,(void*)0);
	else {
		slow_mode = false;
		slow_idx = 0;
		agg_this();
		buttons->show();
	}
	win->redraw();
}

void Slow_CB(Fl_Widget*, void *) {
	slow_mode = true;
	slow_idx = 0;
	buttons->hide();
	Fl::add_timeout(0.05, Timer_Slow_CB,(void*)0);
}

void Kill_Slow_CB(Fl_Widget*, void *) {
	Fl::remove_timeout(Timer_Slow_CB);
	slow_mode = false;
	slow_idx = 0;
	agg_this();
	buttons->show();
	win->redraw();
}

void Color_CB(Fl_Widget*, void *) {
	uchar r,g,b;
	r = lcolor[current] & 0xFF;
	g = (lcolor[current]>>8) & 0xFF;
	b = (lcolor[current]>>16) & 0xFF;
	if (!fl_color_chooser("Pick a line color:",r,g,b,1)) return;
	lcolor[current] = b<<16|g<<8|r;
	bColor->color(fl_rgb_color(lcolor[current] & 0xFF, (lcolor[current]>>8) & 0xFF, (lcolor[current]>>16) & 0xFF));
	bColor->labelcolor(fl_contrast(bColor->labelcolor(),bColor->color()));
	agg_this();
	win->redraw();
}

void width_cb(Fl_Widget *, void* lw) {
//	lwidth[current] = (int) lw;
lwidth[current] = *((int*)(&lw));
	agg_this();
	win->redraw();
}

const char *modeslabel[7] = {"Move","Zoom","Bwarp","Ewarp","Rotate","Select","Paint"};

void setwindowtitle(int mode) {
	char title[80];
	sprintf(title,"Magic Designer FLTK+AGG2D %dx%d mode: %s",width,height,modeslabel[mode]);
	win->copy_label(title);	
}

void setMode_cb(Fl_Widget *, void* mod) {
//	Mode = (int) mod;
	Mode = *((int*)(&mod));
	agg_this();
	setwindowtitle(Mode);
	win->redraw(); // needed to remove current design
}

void Circle_cb(Fl_Widget *, void*) {
	l = magics[current]->magic_area.left;
	t = magics[current]->magic_area.top;
	r = magics[current]->magic_area.right;
	b = magics[current]->magic_area.bottom;
	radiusX = abs(l-r)/2.F;
	radiusY = abs(t-b)/2.F;
	centerX = (l+r)/2.F;
	centerY = (t+b)/2.F;
	if (radiusX < radiusY) radiusY = radiusX;
	else radiusX = radiusY;
	if (l > r) radiusX = 0.F - radiusX;
	if (t < b) radiusY = 0.F - radiusY;
	magics[current]->magic_area.left = (int) round(centerX - radiusX);
	magics[current]->magic_area.top = (int) round(centerY + radiusY);
	magics[current]->magic_area.right = (int) round(centerX + radiusX);
	magics[current]->magic_area.bottom = (int) round(centerY - radiusY);
	update_design();
	agg_this();
	showR();
	win->redraw();
}


void Corner_cb(Fl_Widget *, void*) {
	int temp;
	l = magics[current]->magic_area.left;
	t = magics[current]->magic_area.top;
	r = magics[current]->magic_area.right;
	b = magics[current]->magic_area.bottom;
	if ((l > r && t < b) || (l < r && t > b) ) {
		temp = t;
		magics[current]->magic_area.top = b;
		magics[current]->magic_area.bottom = temp;
	} else {
		temp = l;
		magics[current]->magic_area.left = r;
		magics[current]->magic_area.right = temp;
	}
	update_design();
	agg_this();
	adjust_bCorner();
	win->redraw();
}

void ULRD_cb(Fl_Widget *, void* d) { // adjust for morph
	int dir, ud, lr;
//	dir = (int)d;
dir = *((int*)(&d));
	int morph = cmorph->value();
	if (dir == 0) { ud = -morph; lr = 0; }
	if (dir == 1) { ud = 0; lr = -morph; }
	if (dir == 2) { ud = 0; lr = morph; }
	if (dir == 3) { ud = morph; lr = 0; }
	switch(Mode) {
		case 0: // Move
		l = magics[current]->magic_area.left;
		t = magics[current]->magic_area.top;
		r = magics[current]->magic_area.right;
		b = magics[current]->magic_area.bottom;
		radiusX = abs(l-r)/2.F;
		radiusY = abs(t-b)/2.F;
		centerX = (l+r)/2.F;
		centerY = (t+b)/2.F;
		centerX += lr;
		centerY += ud;
		if (centerX > 4000.F || centerX < -4000.F) return;
		if (centerY > 4000.F || centerY < -4000.F) return;
		if (l > r) radiusX = 0.F - radiusX;
		if (t < b) radiusY = 0.F - radiusY;
		magics[current]->magic_area.left = (int) round(centerX - radiusX);
		magics[current]->magic_area.top = (int) round(centerY + radiusY);
		magics[current]->magic_area.right = (int) round(centerX + radiusX);
		magics[current]->magic_area.bottom = (int) round(centerY - radiusY);
		update_design();
		agg_this();
		showC();
		win->redraw();
		return;

		case 1: // Zoom
		l = magics[current]->magic_area.left;
		t = magics[current]->magic_area.top;
		r = magics[current]->magic_area.right;
		b = magics[current]->magic_area.bottom;
		radiusX = abs(l-r)/2.F;
		radiusY = abs(t-b)/2.F;
		centerX = (l+r)/2.F;
		centerY = (t+b)/2.F;
		radiusX += lr;
		radiusY += ud;
		if (radiusX < 5.F || radiusY < 5.F) return;
		if (radiusX > 4000.F || radiusY > 4000.F) return;
		if (l > r) radiusX = 0.F - radiusX;
		if (t < b) radiusY = 0.F - radiusY;
		magics[current]->magic_area.left = (int) round(centerX - radiusX);
		magics[current]->magic_area.top = (int) round(centerY + radiusY);
		magics[current]->magic_area.right = (int) round(centerX + radiusX);
		magics[current]->magic_area.bottom = (int) round(centerY - radiusY);
		update_design();
		agg_this();
		showR();
		win->redraw();
		return;

		case 2: // WarpB
		l = magics[current]->magic_area.left;
		t = magics[current]->magic_area.top;
		l += lr;
		t += ud;
		if (l > 4000 || l < -4000) return;
		if (t > 4000 || t < -4000) return;
		magics[current]->magic_area.left = l;
		magics[current]->magic_area.top = t;
		update_design();
		agg_this();
		adjust_bCorner();
		showC();
		showR();
		win->redraw();
		return;

		case 3: // WarpE
		r = magics[current]->magic_area.right;
		b = magics[current]->magic_area.bottom;
		r += lr;
		b += ud;
		if (r > 4000 || r < -4000) return;
		if (b > 4000 || b < -4000) return;
		magics[current]->magic_area.right = r;
		magics[current]->magic_area.bottom = b;
		update_design();
		agg_this();
		adjust_bCorner();
		showC();
		showR();
		win->redraw();
		return;

		case 4: // Rotate
		if (ud == 0) return; // only uses UD
		r = magics[current]->numpoints%3600 - ud;
		if (r >= 3600) r -= 3600;
		if (r < 0) r += 3600; 
		magics[current]->numpoints = ((magics[current]->numpoints>=54000)?54000:0) + r;
		update_design();
		agg_this();
		sprintf(s,"%d",r);
		angleRtext->value(s);
		win->redraw();
		return;

		case 5: // Select
		if (ud == 0) return; // only uses UD
		if (designsCount < 2) return;
		selected = (selected-ud)%designsCount;
		if (selected < 0) selected += designsCount;
		selectDesign();
		agg_this();
		set_controls();
		showT();
		win->redraw();
		return;
	}
}

void leftArm_cb(Fl_Widget *, void* arm) { // need to add stuff for arm placement/sidedness
//	Larm[current] = (int) arm;
Larm[current] = *((int*)(&arm));
	calc_arms(1);
	cLArm->value(magics[current]->armL_length);
	update_design();
	agg_this();
	win->redraw();
}

void rightArm_cb(Fl_Widget *, void* arm) {
//	Rarm[current] = (int) arm;
Rarm[current] = *((int*)(&arm));
	calc_arms(2);
	cRArm->value(magics[current]->armR_length);
	update_design();
	agg_this();
	win->redraw();
}

bool placeShift(int temp) {
	int temp2 = Advanced_shift_all[Armsplacement[current]-1];
	if (temp2 == 12) {
		magics[current]->gearL_angle = temp;
		cLGearA->value(magics[current]->gearL_angle);
	}
	if (temp2 == 11) {
		magics[current]->gearR_angle = temp; // note: this is not really the right gear. The right/left is determined by the arm placement.
		cRGearA->value(magics[current]->gearR_angle);
	}
	return (temp2 != 0);
}

void shiftlever_cb(Fl_Widget *, void*) { // need to add stuff for arm placement
	int temp = (int)(shiftlever->value());
	if (temp == Shift[current]) return;
	Shift[current] = temp;
	sprintf(s,"%d",Shift[current]);
	shiftlever_e->value(s);
	if (placeShift(2950 - (Shift[current] * 10))) {
		update_design();
		agg_this();
		win->redraw();
	}
}

void Clip_cb(Fl_Widget *, void*) {
	if (bClip->value()) magics[current]->clipradius = (magics[current]->clipradius&0xFFFFFFFE)|0x1;
	else magics[current]->clipradius = magics[current]->clipradius&0xFFFFFFFE;
	update_design();
	agg_this();
	win->redraw();
}

void Shape_cb(Fl_Widget *, void*) {
	if (bShape->value()) magics[current]->clipradius = (magics[current]->clipradius&0xFFFFFFFD)|0x2;
	else magics[current]->clipradius = magics[current]->clipradius&0xFFFFFFFD;
	agg_this();
	win->redraw();
}

void Fine_cb(Fl_Widget *, void*) {
	magics[current]->numpoints %= 3600;
	if (bFine->value()) magics[current]->numpoints += 54000;
	update_design();
	agg_this();
	win->redraw();
}

void Prime_cb(Fl_Widget *, void*) {
	if (bPrime->value()) magics[current]->prime = (magics[current]->prime&0xFFFFFFFE)|0x1;
	else magics[current]->prime = magics[current]->prime&0xFFFFFFFE;
	update_design();
	agg_this();
	win->redraw();
}

void Jagg_cb(Fl_Widget *w, void*) {
	jagg = (((Fl_Check_Button *)w)->value());
	if (jagg) agg_setantialias(0.01);
	else agg_setantialias(1.0);
	agg_refresh();
	win->redraw();
}

void Suite_cb(Fl_Widget *w, void*) {
	suite = (((Fl_Check_Button *)w)->value());
	agg_this();
	win->redraw();
}

void armsp_cb(Fl_Widget *, void*) { // need to add logic if advanced parameters
	Armsplacement[current]++;
	if (Armsplacement[current] > 6) Armsplacement[current] = 1;
	armsplacementb->image(armp[Armsplacement[current]-1]);
	magics[current]->center_radius = Advanced_default_all[Armsplacement[current]-1][0];
	magics[current]->gearL_radius = Advanced_default_all[Armsplacement[current]-1][1];
	magics[current]->gearR_radius = Advanced_default_all[Armsplacement[current]-1][2];
	magics[current]->pegL_radius = Advanced_default_all[Armsplacement[current]-1][3];
	magics[current]->pegR_radius = Advanced_default_all[Armsplacement[current]-1][4];
	magics[current]->gearR_angle = Advanced_default_all[Armsplacement[current]-1][7];
	magics[current]->pegL_angle = Advanced_default_all[Armsplacement[current]-1][9];
	magics[current]->pegR_angle = Advanced_default_all[Armsplacement[current]-1][10];
	magics[current]->rotations = Advanced_default_all[Armsplacement[current]-1][11];
	magics[current]->gearL_angle = Advanced_default_all[Armsplacement[current]-1][8];
	magics[current]->prime = 6251;
	show_gears();
	sprintf(s,"%d",magics[current]->rotations);
	rots->value(s);
	if (Larm[current] == 90) { Larm[current] = 67; Larmb[Larm[current]-65]->setonly(); } // reset to defaults
	if (Rarm[current] == 90) { Rarm[current] = 67; Rarmb[Rarm[current]-65]->setonly(); }
	if (Shift[current] == 0) { Shift[current] = 60; shiftlever->value(Shift[current]); sprintf(s,"%d",Shift[current]); shiftlever_e->value(s); }
	calc_arms(1);
	calc_arms(2);
	cLPeg->value(magics[current]->pegL_radius);
	cRPeg->value(magics[current]->pegR_radius);
	cLArm->value(magics[current]->armL_length);
	cRArm->value(magics[current]->armR_length);
	cLGearA->value(magics[current]->gearL_angle);
	cRGearA->value(magics[current]->gearR_angle);
	cLPegA->value(magics[current]->pegL_angle);
	cRPegA->value(magics[current]->pegR_angle);
	cArmAdjust->value(magics[current]->prime/10);
	placeShift(2950 - (Shift[current] * 10));
	update_design();
	agg_this();
	win->redraw();
}

void cLPeg_cb(Fl_Widget *, void*) {
	magics[current]->pegL_radius = cLPeg->value();
	update_design();
	agg_this();
	win->redraw();
}

void cRPeg_cb(Fl_Widget *, void*) {
	magics[current]->pegR_radius = cRPeg->value();
	update_design();
	agg_this();
	win->redraw();
}

void cLArm_cb(Fl_Widget *, void*) {
	magics[current]->armL_length = cLArm->value();
	Larm[current] = 90;
	for (int j=0; j<18; j++) Larmb[j]->clear();
	update_design();
	agg_this();
	win->redraw();
}

void cRArm_cb(Fl_Widget *, void*) {
	magics[current]->armR_length = cRArm->value();
	Rarm[current] = 90;
	for (int j=0; j<18; j++) Rarmb[j]->clear();
	update_design();
	agg_this();
	win->redraw();
}

void cLGearA_cb(Fl_Widget *, void*) {
	if (cLGearA->value() == 3600) cLGearA->value(0);
	if (cLGearA->value() < 0) cLGearA->value(3600);
	magics[current]->gearL_angle = cLGearA->value();
	Shift[current] = 0;
	shiftlever_e->value("0");
	update_design();
	agg_this();
	win->redraw();
}

void cRGearA_cb(Fl_Widget *, void*) {
	if (cRGearA->value() == 3600) cRGearA->value(0);
	if (cRGearA->value() < 0) cRGearA->value(3600);
	magics[current]->gearR_angle = cRGearA->value();
	Shift[current] = 0;
	shiftlever_e->value("0");
	update_design();
	agg_this();
	win->redraw();
}

void cLPegA_cb(Fl_Widget *, void*) {
	magics[current]->pegL_angle = cLPegA->value();
	update_design();
	agg_this();
	win->redraw();
}

void cRPegA_cb(Fl_Widget *, void*) {
	magics[current]->pegR_angle = cRPegA->value();
	update_design();
	agg_this();
	win->redraw();
}

void cArmAdjust_cb(Fl_Widget *, void*) {
	magics[current]->prime = cArmAdjust->value()*10 + (magics[current]->prime&0x1);
	update_design();
	agg_this();
	win->redraw();
}

// Callback: when user picks 'Quit'
void quit_cb(Fl_Widget*, void*) {
	if (Fl::event()==FL_SHORTCUT && Fl::event_key()==FL_Escape) return; // ignore Escape
	exit(0);
}

Fl_Menu_Item rclick_menu[] = {
	{ "Add to Design Suite",   0, Add_CB,  (void*)0 },
	{ "Load Design Suite",   0, Load_CB,  (void*)0 },
	{ "Save Design Suite",   0, Save_CB,  (void*)0 },
	{ "Design Suite as a PDF",   0, PDF_CB,  (void*)0 },
	{ "Delete/Undo Designs", 0, Delete_CB, (void*)0 },
	{ "Start Bloom Show", 0, Bloom_CB,  (void*)0 },
	{ "Start Slow Draw", 0, Slow_CB,  (void*)0 },
	{ "Done with program",   0, quit_cb,  (void*)0 },
	{ 0 }
};

Fl_Menu_Item rclick_inBloom_menu[] = {
	{ "Kill Bloom Show (left click to pause/restart)", 0, Kill_Bloom_CB,  (void*)0 },
	{ "Done with program",   0, quit_cb,  (void*)0 },
	{ 0 }
};

Fl_Menu_Item rclick_inSlow_menu[] = {
	{ "Kill Slow Draw (left click to pause/restart)", 0, Kill_Slow_CB,  (void*)0 },
	{ "Done with program",   0, quit_cb,  (void*)0 },
	{ 0 }
};

/************************************************************************/

// WINDOW CLASS TO HANDLE DRAWING IMAGE
class MyWindow : public Fl_Window { // public Fl_Double_Window {
	// FLTK DRAW METHOD
	void draw() {
		if (inresize) return;
		fl_draw_image((const uchar*)image_array, 0, 0, width, height, DEPTH, width*DEPTH);
	}

	void resize(int X, int Y, int W, int H) {
		futurewidth = W;
		futureheight = H;
		left = X;
		top = Y;
		Fl::remove_timeout(Timer_CB,(void*)0);
		if (H!= height || W!= width) {
			inresize = true;
			Fl::add_timeout(1.0, Timer_CB,(void*)0);              // setup a timer
			// the timer is to guess that the user moved it in a second.
		}
	}

	public:
	// CTOR
	MyWindow(int x, int y, int w, int h, const char *name=0) : Fl_Window(x,y,w,h,name) {
		end();
	}

	int handle(int e) {
		int X = Fl::event_x();
		int Y = Fl::event_y();
		int temp;
		float factor;
		switch (e) {
			case FL_RELEASE:
				// RIGHT MOUSE RELEASED? Mask it from Fl_Input
				if ( Fl::event_button() == FL_RIGHT_MOUSE ) {
					return(1);          // (tells caller we handled this event)
				}
				break;
			case FL_PUSH:
				// RIGHT MOUSE PUSHED? Popup menu on right click
				const Fl_Menu_Item *m;
				if ( Fl::event_button() == FL_RIGHT_MOUSE ) {
					if (bloom_mode) { m = rclick_inBloom_menu->popup(X, Y, 0, 0, 0); }
					else { if (slow_mode) m = rclick_inSlow_menu->popup(X, Y, 0, 0, 0);
					else m = rclick_menu->popup(X, Y, 0, 0, 0); }
					if ( m ) m->do_callback(0, m->user_data());
					return(1);          // (tells caller we handled this event)
				}
				if (bloom_mode) { // bloom mode supercedes all others and this should toggle pause.
					if (Fl::has_timeout(Timer_Bloom_CB)) Fl::remove_timeout(Timer_Bloom_CB);
					else Fl::add_timeout(0.05, Timer_Bloom_CB,(void*)0);
					return (1);
				}
				if (slow_mode) { // slow mode supercedes all others and this should toggle pause.
					if (Fl::has_timeout(Timer_Slow_CB)) Fl::remove_timeout(Timer_Slow_CB);
					else Fl::add_timeout(0.2, Timer_Slow_CB,(void*)0);
					return (1);
				}
				if (Mode == 5) { // Select
					temp = closestDesign(X,Y);
					if (temp >= 0) {
						selected = temp;
						selectDesign();
						agg_this();
						set_controls();
						showT();
						redraw();
					}
					return (1);
				}
				if (Mode == 6) { // Paint
					fill(X,Y,lcolor[current]);
					agg_this();
					redraw();
					return (1);
				}
			case FL_DRAG: // mouse down with button - it is left since right was processed already
				if (bloom_mode || slow_mode) return(1);
			switch (Mode) {

				case 0: // Move
				l = magics[current]->magic_area.left;
				t = magics[current]->magic_area.top;
				r = magics[current]->magic_area.right;
				b = magics[current]->magic_area.bottom;
				radiusX = abs(l-r)/2.F;
				radiusY = abs(t-b)/2.F;
				centerX = X;
				centerY = Y;
				if (l > r) radiusX = 0.F - radiusX;
				if (t < b) radiusY = 0.F - radiusY;
				magics[current]->magic_area.left = (int) round(centerX - radiusX);
				magics[current]->magic_area.top = (int) round(centerY + radiusY);
				magics[current]->magic_area.right = (int) round(centerX + radiusX);
				magics[current]->magic_area.bottom = (int) round(centerY - radiusY);
				update_design();
				agg_this();
				showC();
				redraw();
				return(1);

				case 1: // Zoom
				l = magics[current]->magic_area.left;
				t = magics[current]->magic_area.top;
				r = magics[current]->magic_area.right;
				b = magics[current]->magic_area.bottom;
				radiusX = abs(l-r)/2.F;
				radiusY = abs(t-b)/2.F;
				centerX = (l+r)/2.F;
				centerY = (t+b)/2.F;
				factor = sqrt((X-centerX)*(X-centerX)+(Y-centerY)*(Y-centerY))/((radiusX+radiusY)/2.F);
				if ((radiusX < 10.F || radiusY < 10.F) && factor < 1.F) return(1);
				radiusX *= factor;
				radiusY *= factor;
				if (l > r) radiusX = 0.F - radiusX;
				if (t < b) radiusY = 0.F - radiusY;
				magics[current]->magic_area.left = (int) round(centerX - radiusX);
				magics[current]->magic_area.top = (int) round(centerY + radiusY);
				magics[current]->magic_area.right = (int) round(centerX + radiusX);
				magics[current]->magic_area.bottom = (int) round(centerY - radiusY);
				update_design();
				agg_this();
				showR();
				redraw();
				return(1);

				case 2: // WarpB
				magics[current]->magic_area.left = (int) X;
				magics[current]->magic_area.top = (int) Y;
				update_design();
				agg_this();
				adjust_bCorner();
				showC();
				showR();
				redraw();
				return(1);

				case 3: // WarpE
				magics[current]->magic_area.right = (int) X;
				magics[current]->magic_area.bottom = (int) Y;
				update_design();
				agg_this();
				adjust_bCorner();
				showC();
				showR();
				redraw();
				return(1);

				case 4: // Rotate
				l = magics[current]->magic_area.left;
				t = magics[current]->magic_area.top;
				r = magics[current]->magic_area.right;
				b = magics[current]->magic_area.bottom;
				centerX = (l+r)/2.F;
				centerY = (t+b)/2.F;
				temp = (int) (RD_DG(atan2(Y-centerY, X-centerX))*10.F)+1799;
				magics[current]->numpoints = ((magics[current]->numpoints>=54000)?54000:0) + temp;
				update_design();
				agg_this();
				sprintf(s,"%d",temp);
				angleRtext->value(s);
				redraw();
				return(1);

				default:
				return(1);
			}
		}
		return(Fl_Window::handle(e));    // let Fl_... handle all other events
	}

};

void startwin() {
	Fl::visual(FL_RGB);
	image_array = new unsigned char[width*height*DEPTH];  // image buffer
	image_double = new unsigned char[width*height*DEPTH];
	pixels = (Pixel *)image_double;
	agg_attach(image_array, width, height, width * DEPTH);
	if (jagg) agg_setantialias(0.01);
	else agg_setantialias(1.0);
	agg_refresh();
	win = new MyWindow(left,top,width,height,"");
	setwindowtitle(0);
	win->resizable(win);
	win->callback(quit_cb,(void *)0);
	inresize = false;
#ifdef linux
	win->icon((const void*)p);
#endif
	win->show();
}

static void Timer_CB(void*) {              // resize timer callback
	Fl::remove_timeout(Timer_CB);
	width = futurewidth;
	height = futureheight;
	delete [] image_array;
	delete [] image_double;
	delete win;
	startwin();
}

/************************************************************************/

int main(int argc, char **argv) {

	int XX, YY, HH, WW;
	Fl::screen_work_area(XX,YY,WW,HH);
	if (argc > 2) {
		width = atoi(argv[1]);
		height = atoi(argv[2]);
	} else {
		width = WW-310-10;
		height = HH-24; //Fl_Window::decorated_h();
	}
	left = XX;
	top= YY+20; //Fl_Window::decorated_h();

#ifdef linux
fl_open_display(); // needed if display has not been previously opened
p = XCreateBitmapFromData(fl_display, DefaultRootWindow(fl_display),
                                 icon_bits, icon_width, icon_height);
#endif

	// allocate stuff
	stack = new Segment[MAX]; /* stack of filled segments */
	sp = stack;
	DIV_buffer_ptr = new DIV[maxDIVs];
	magics = new magic *[current+1];
	beziers = new DIV *[current+1];
	segs = new int[current+1];
	lcolor = new int[current+1];
	lwidth = new int[current+1];
	Larm = new int[current+1];
	Rarm = new int[current+1];
	Shift = new int[current+1];
	Armsplacement = new int[current+1];

	designsCount = 0;
	selected = -1; // means no designs
	backcolor = -1; // White
	default_shape();
	startwin();

	const char *armlabel[18] = {"A","B","C","D","E","F","G","H","I","J","K","L","M","N","O","P","Q","R"};
	const char *widthlabel[4] = {"1","2","3","4"};

	buttons = new Fl_Window(width+10,top,310,524,"Magic Parameters");
	buttons->callback(quit_cb,(void *)0);
	message = new Fl_Output(0,504,310,20);

	Fl_Group *Modes = new Fl_Group(0,0,64,140);
	Fl_Button *bMove = new Fl_Radio_Round_Button(0, 0, 64, 20, modeslabel[0]);
	bMove->callback(setMode_cb,(void *)0);
	bMove->setonly();
	bMove->labelcolor(FL_WHITE);
	Fl_Button *bZoom = new Fl_Radio_Round_Button(0, 20, 64, 20, modeslabel[1]);
	bZoom->callback(setMode_cb,(void *)1);
	bZoom->labelcolor(FL_WHITE);
	Fl_Button *bWarpB = new Fl_Radio_Round_Button(0, 40, 64, 20, modeslabel[2]);
	bWarpB->callback(setMode_cb,(void *)2);
	bWarpB->labelcolor(FL_WHITE);
	Fl_Button *bWarpE = new Fl_Radio_Round_Button(0, 60, 64, 20, modeslabel[3]);
	bWarpE->callback(setMode_cb,(void *)3);
	bWarpE->labelcolor(FL_WHITE);
	Fl_Button *bRotate = new Fl_Radio_Round_Button(0, 80, 64, 20, modeslabel[4]);
	bRotate->callback(setMode_cb,(void *)4);
	bRotate->labelcolor(FL_WHITE);
	Fl_Button *bSelect = new Fl_Radio_Round_Button(0, 100, 64, 20, modeslabel[5]);
	bSelect->callback(setMode_cb,(void *)5);
	bSelect->labelcolor(FL_WHITE);
	Fl_Button *bPaint = new Fl_Radio_Round_Button(0, 120, 64, 20, modeslabel[6]);
	bPaint->callback(setMode_cb,(void *)6);
	bPaint->labelcolor(FL_WHITE);
	Modes->end();
	Modes->box(FL_UP_BOX);
	Modes->color(FL_BLUE);
	Modes->tooltip("Mouse Modes");

	Fl_Button *bCircle = new Fl_Button(0, 140, 64, 16, "Circle");
	bCircle->callback(Circle_cb,(void *)0);
	bCircle->labelcolor(FL_WHITE);
	bCircle->color(FL_BLUE);
	bCircle->tooltip("Remove Warp");

	bCorner = new Fl_Button(0, 156, 64, 64, "B");
	bCorner->callback(Corner_cb,(void *)0);
	bCorner->labelcolor(FL_WHITE);
	bCorner->labelsize(24);
	bCorner->color(FL_BLUE);
	bCorner->tooltip("Rotate by corners");

	Fl_Repeat_Button *Upb = new Fl_Repeat_Button(12,220,40,20,"U");
	Upb->callback(ULRD_cb,(void *)0);
	Upb->labelcolor(FL_WHITE);
	Upb->color(FL_BLUE);
	Upb->tooltip("Fine adjust current Mode");
	Fl_Repeat_Button *Leftb = new Fl_Repeat_Button(0,240,32,20,"L");
	Leftb->callback(ULRD_cb,(void *)1);
	Leftb->labelcolor(FL_WHITE);
	Leftb->color(FL_BLUE);
	Leftb->tooltip("Fine adjust current Mode");
	Fl_Repeat_Button *Rightb = new Fl_Repeat_Button(32,240,32,20,"R");
	Rightb->callback(ULRD_cb,(void *)2);
	Rightb->labelcolor(FL_WHITE);
	Rightb->color(FL_BLUE);
	Rightb->tooltip("Fine adjust current Mode");
	Fl_Repeat_Button *Downb = new Fl_Repeat_Button(12,260,40,20,"D");
	Downb->callback(ULRD_cb,(void *)3);
	Downb->labelcolor(FL_WHITE);
	Downb->color(FL_BLUE);
	Downb->tooltip("Fine adjust current Mode");

	centerXtext = new Fl_Output(18,280,46,16,"C:");
	centerYtext = new Fl_Output(18,296,46,16);
	radiusXtext = new Fl_Output(22,312,42,16,"Rx");
	radiusYtext = new Fl_Output(22,328,42,16,"Ry");
	angleRtext = new Fl_Output(22,344,42,16,"Ra");
	centerXtext->tooltip("Center X coordinate");
	centerYtext->tooltip("Center Y coordinate");
	radiusXtext->tooltip("Radius horizontal size");
	radiusYtext->tooltip("Radius vertical size");
	angleRtext->tooltip("Frame Rotation angle");
	centerXtext->clear_visible_focus();
	centerYtext->clear_visible_focus();
	radiusXtext->clear_visible_focus();
	radiusYtext->clear_visible_focus();
	angleRtext->clear_visible_focus();

	traces = new Fl_Output(20,388,42,16,"T:");
	traces->color(fl_rgb_color(64,192,255));
	traces->tooltip("#Designs in suite");
	traces->clear_visible_focus();
	selectedt = new Fl_Output(80,388,42,16, "S:");
	selectedt->color(fl_rgb_color(64,192,255));
	selectedt->tooltip("Selected design (used as base in morph)");
	selectedt->clear_visible_focus();

	Fl_Group *LArm = new Fl_Group(66,0,32,360);
	for (int i=0;i<18;i++) {
		Larmb[i] = new Fl_Radio_Round_Button(66,i*20,32,20,armlabel[i]);
		Larmb[i]->callback(leftArm_cb,(void *)(i+65));
	}
	LArm->end();
	LArm->box(FL_UP_BOX);
	LArm->color(FL_YELLOW);
	LArm->tooltip("Left Arm");

	Fl_Group *RArm = new Fl_Group(100,0,32,360);
	for (int i=0;i<18;i++) {
		Rarmb[i] = new Fl_Radio_Round_Button(100,i*20,32,20,armlabel[i]);
		Rarmb[i]->callback(rightArm_cb,(void *)(i+65));
	}
	RArm->end();
	RArm->box(FL_UP_BOX);
	RArm->color(FL_YELLOW);
	RArm->tooltip("Right Arm");

	shiftlever = new Fl_Slider(FL_VERT_NICE_SLIDER,134,20,32,240,"Shift");
	shiftlever->bounds(10.0,70.0);
	shiftlever->align(FL_ALIGN_TOP);
	shiftlever->color(FL_CYAN);
	shiftlever->callback(shiftlever_cb,(void*)0);
	shiftlever_e = new Fl_Output(134,260,32,16);
	shiftlever->tooltip("Shift Lever");
	shiftlever_e->tooltip("Shift Lever");
	shiftlever_e->clear_visible_focus();

	cmorph = new Fl_Counter(0,404,150,20);
	cmorph->range(1,999);
	cmorph->step(1,50);
	cmorph->value(1);
	cmorph->tooltip("Used for morph designs and fine positioning.");

	Fl_Button *bAdd = new Fl_Button(0, 424, 50, 40, "Add");
	bAdd->callback(Add_CB,(void *)0);
	bAdd->tooltip("Add current design or morph to Design Suite");
	Fl_Button *bLoad = new Fl_Button(50, 424, 50, 40, "Load");
	bLoad->callback(Load_CB,(void *)0);
	bLoad->tooltip("Load designs from a file to the Design Suite");
	Fl_Button *bSave = new Fl_Button(50, 464, 50, 40, "Save");
	bSave->callback(Save_CB,(void *)0);
	bSave->tooltip("Save the Design Suite to a file.");
	Fl_Button *bDelete = new Fl_Button(0, 464, 50, 40, "Delete");
	bDelete->callback(Delete_CB,(void *)0);
	bDelete->tooltip("Delete/Undo designs");
	Fl_Button *bBloom = new Fl_Button(100, 464, 50, 40, "Bloom");
	bBloom->callback(Bloom_CB,(void *)0);
	bBloom->tooltip("Start Bloom Show.\nUse Mouse on Window to left Pause/ right Kill.");
	Fl_Button *bSlow = new Fl_Button(100, 424, 50, 40, "Slow");
	bSlow->callback(Slow_CB,(void *)0);
	bSlow->tooltip("Emulate a design being drawn.\nUse Mouse on Window to left Pause/ right Kill.");
	PDFimage = new Fl_Pixmap(pdfx);
	Fl_Button *bPDF = new Fl_Button(132,276,48,48);
	bPDF->image(PDFimage);
	bPDF->callback(PDF_CB,(void *)0);
	bPDF->tooltip("Create a PDF file of scalable smooth curves.");

	Fl_Group *Width = new Fl_Group(170,0,64,40);
	for (int i=0; i<4; i++) {
		Widthb[i] = new Fl_Radio_Round_Button(170+((i/2)*32),(i%2)*20,32,20,widthlabel[i]);
		Widthb[i]->callback(width_cb,(void *)(i+1));
	}
	Width->end();
	Width->box(FL_UP_BOX);
	Width->color(fl_rgb_color(255,192,64));
	Width->tooltip("Line Width 1=thin, 2=bold 3=thick 4=very thick");

	bColor = new Fl_Button(170, 40, 64, 60, "Color");
	bColor->callback(Color_CB,(void *)0);
	bColor->tooltip("Line or Paper Color");

	bClip = new Fl_Check_Button(234,0,72,20,"Clip");
	bClip->callback(Clip_cb,(void *)0);
	bClip->box(FL_UP_BOX);
	bClip->color(fl_rgb_color(255,192,64));
	bClip->tooltip("Check to emulate circular paper boundary.");
	bShape = new Fl_Check_Button(234,20,72,20,"Paper");
	bShape->callback(Shape_cb,(void *)0);
	bShape->box(FL_UP_BOX);
	bShape->color(fl_rgb_color(255,192,64));
	bShape->tooltip("Check to emulate circular or rectangular paper.");
	bFine = new Fl_Check_Button(234,40,72,20,"3X plot");
	bFine->callback(Fine_cb,(void *)0);
	bFine->box(FL_UP_BOX);
	bFine->labelcolor(FL_WHITE);
	bFine->color(FL_BLUE);
	bFine->tooltip("Check if design curve does not seem smooth.");
	Fl_Check_Button *bJagg = new Fl_Check_Button(234,60,72,20,"Jagged");
	bJagg->callback(Jagg_cb,(void *)0);
	bJagg->box(FL_UP_BOX);
	bJagg->color(fl_rgb_color(64,192,255));
	bJagg->tooltip("Nice curves for scaling or jagged lines for painting.");
	Fl_Check_Button *bSuite = new Fl_Check_Button(234,80,72,20,"Suite");
	bSuite->callback(Suite_cb,(void *)0);
	bSuite->box(FL_UP_BOX);
	bSuite->value(true);
	bSuite->color(fl_rgb_color(64,192,255));
	bSuite->tooltip("Un-Check to hide Suite of designs.");

	armp[0] = new Fl_Pixmap(armsplace1);
	armp[1] = new Fl_Pixmap(armsplace2);
	armp[2] = new Fl_Pixmap(armsplace3);
	armp[3] = new Fl_Pixmap(armsplace4);
	armp[4] = new Fl_Pixmap(armsplace5);
	armp[5] = new Fl_Pixmap(armsplace6);
	armsplacementb = new Fl_Button(166,118,144,144);
	armsplacementb->callback(armsp_cb,(void *)0);
	armsplacementb->tooltip("Click to cycle to the next arms placement (there are 6).\nAlso used to reset values below.");

	cCenter = new Fl_Simple_Counter(220,264,90,20);
	cCenter->color(fl_rgb_color(192,128,192));
	cCenter->range(1,13);
	cCenter->step(1);
	cCenter->callback(Gear_cb,(void *)0);
	cCenter->tooltip("Center gear turn table size radio");
	cLGear = new Fl_Simple_Counter(220,284,90,20);
	cLGear->color(fl_rgb_color(192,128,192));
	cLGear->range(1,13);
	cLGear->step(1);
	cLGear->callback(Gear_cb,(void *)0);
	cLGear->tooltip("Left gear size radio");
	cRGear = new Fl_Simple_Counter(220,304,90,20);
	cRGear->color(fl_rgb_color(192,128,192));
	cRGear->range(1,13);
	cRGear->step(1);
	cRGear->callback(Gear_cb,(void *)0);
	cRGear->tooltip("Right gear size radio");
	rots = new Fl_Output(198,264,22,20,"Rs");
	rots->color(fl_rgb_color(192,128,192));
	rots->tooltip("Number of rotations to complete the design.");
	rots->clear_visible_focus();

	cLPeg = new Fl_Counter(160,324,150,20);
	cLPeg->color(fl_rgb_color(192,128,192));
	cLPeg->range(0,65000);
	cLPeg->step(1,50);
	cLPeg->callback(cLPeg_cb,(void *)0);
	cLPeg->tooltip("Distance from left gear center to peg.");
	cRPeg = new Fl_Counter(160,344,150,20);
	cRPeg->color(fl_rgb_color(192,128,192));
	cRPeg->range(0,65000);
	cRPeg->step(1,50);
	cRPeg->callback(cRPeg_cb,(void *)0);
	cRPeg->tooltip("Distance from right gear center to peg.");
	cLArm = new Fl_Counter(160,364,150,20);
	cLArm->color(fl_rgb_color(192,128,192));
	cLArm->range(0,65000);
	cLArm->step(1,50);
	cLArm->callback(cLArm_cb,(void *)0);
	cLArm->tooltip("Length of arm from left peg to pen.");
	cRArm = new Fl_Counter(160,384,150,20);
	cRArm->color(fl_rgb_color(192,128,192));
	cRArm->range(0,65000);
	cRArm->step(1,50);
	cRArm->callback(cRArm_cb,(void *)0);
	cRArm->tooltip("Length of arm from right peg to pen.");
	cLGearA = new Fl_Counter(160,404,150,20);
	cLGearA->color(fl_rgb_color(192,128,192));
	cLGearA->range(0,3600);
	cLGearA->step(1,50);
	cLGearA->callback(cLGearA_cb,(void *)0);
	cLGearA->tooltip("Angle position of left gear; East=0;counterclockwise.");
	cRGearA = new Fl_Counter(160,424,150,20);
	cRGearA->color(fl_rgb_color(192,128,192));
	cRGearA->range(0,3600);
	cRGearA->step(1,50);
	cRGearA->callback(cRGearA_cb,(void *)0);
	cRGearA->tooltip("Angle position of right gear; East=0;counterclockwise.");
	cLPegA = new Fl_Counter(160,444,150,20);
	cLPegA->color(fl_rgb_color(192,128,192));
	cLPegA->range(0,65000);
	cLPegA->step(1,50);
	cLPegA->callback(cLPegA_cb,(void *)0);
	cLPegA->tooltip("A reference starting angle position for the left peg.");
	cRPegA = new Fl_Counter(160,464,150,20);
	cRPegA->color(fl_rgb_color(192,128,192));
	cRPegA->range(0,65000);
	cRPegA->step(1,50);
	cRPegA->callback(cRPegA_cb,(void *)0);
	cRPegA->tooltip("A reference starting angle position for the right peg.");
	cArmAdjust = new Fl_Counter(160,484,150,20);
	cArmAdjust->color(fl_rgb_color(192,128,192));
	cArmAdjust->range(0,65000);
	cArmAdjust->step(1,50);
	cArmAdjust->callback(cArmAdjust_cb,(void *)0);
	cArmAdjust->tooltip("Adjustment factor because arms do not meet at a point.");
	bPrime = new Fl_Check_Button(180,304,40,20,"Pri");
	bPrime->callback(Prime_cb,(void *)0);
	bPrime->box(FL_UP_BOX);
	bPrime->color(fl_rgb_color(192,128,192));
	bPrime->tooltip("Un-Check to emulate putting pen arms on upside down.");

	message->clear_visible_focus();

	showT();
	set_controls();
	buttons->end();
#ifdef linux
	buttons->icon((const void*)p);
#endif
	buttons->show();

	return Fl::run();
}
