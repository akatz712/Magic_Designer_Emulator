// all of the agg code is being separated so I can focus on performance
// and use any GUI toolkit

#include "agg2d/agg2d.h"

// for the DIV structure
#include "BezierSpline.h"
/*******************************************************/
// Agg2D::Color replaced with agg::rgba8
Agg2D m_graphics;

void agg_drawc(int width, int color, int segs, DIV *Divs, int l, int t, int r, int b, int a) {
	float centerX, centerY;
	centerX = (l+r)/2.F;
	centerY = (t+b)/2.F;
	if (a>0) {
		m_graphics.translate(-centerX,-centerY);
		m_graphics.rotate(m_graphics.deg2Rad((double) a/10.F));
		m_graphics.translate(centerX,centerY);
	} 
	m_graphics.lineWidth(double(width));
	m_graphics.lineColor(agg::rgba8((color>>16) & 0xFF, (color>>8) & 0xFF, color & 0xFF, 0xFF));  // is in 0BGR -> B,G,R,FF
	m_graphics.resetPath();
	for(int i = 0; i < segs; i+=4)
	{
		if (Divs[i+1].x > -10000.0) {
			m_graphics.moveTo(Divs[i].x, Divs[i].y);
			m_graphics.cubicCurveTo(Divs[i+1].x, Divs[i+1].y, Divs[i+2].x, Divs[i+2].y, Divs[i+3].x, Divs[i+3].y);
		}
	}
	m_graphics.drawPath(Agg2D::StrokeOnly);
	m_graphics.resetTransformations();
}

void agg_drawrect(int color, int l, int t, int r, int b, int a) {
	float centerX, centerY;
	centerX = (l+r)/2.F;
	centerY = (t+b)/2.F;
	if (a>0) {
		m_graphics.translate(-centerX,-centerY);
		m_graphics.rotate(m_graphics.deg2Rad((double) a/10.F));
		m_graphics.translate(centerX,centerY);
	} 
	m_graphics.fillColor(agg::rgba8((color>>16) & 0xFF, (color>>8) & 0xFF, color & 0xFF, 0xFF));  // is in 0BGR -> B,G,R,FF
	m_graphics.resetPath();
	m_graphics.moveTo(l, t);// B
	m_graphics.lineTo(l, b);
	m_graphics.lineTo(r, b);// E
	m_graphics.lineTo(r, t);
	m_graphics.closePolygon();
	m_graphics.drawPath(Agg2D::FillOnly);
	m_graphics.resetTransformations();
}

void agg_drawellipse(int color, int l, int t, int r, int b, int a) {
	float radiusX, radiusY, centerX, centerY;
	radiusX = abs(l-r)/2.F;
	radiusY = abs(t-b)/2.F;
	centerX = (l+r)/2.F;
	centerY = (t+b)/2.F;
	if (a>0 && radiusX != radiusY) {
		m_graphics.translate(-centerX,-centerY);
		m_graphics.rotate(m_graphics.deg2Rad((double) a/10.F));
		m_graphics.translate(centerX,centerY);
	} 
	m_graphics.fillColor(agg::rgba8((color>>16) & 0xFF, (color>>8) & 0xFF, color & 0xFF, 0xFF));  // is in 0BGR -> B,G,R,FF
	m_graphics.resetPath();
	m_graphics.addEllipse(centerX,centerY,radiusX,radiusY, Agg2D::CW); // let agg calculate how many lines to do.
	m_graphics.drawPath(Agg2D::FillOnly);
	m_graphics.resetTransformations();
}

void agg_clearAll(int color) {
	m_graphics.clearAll(agg::rgba8((color>>16) & 0xFF, (color>>8) & 0xFF, color & 0xFF, 0xFF));  // is in 0BGR -> RGBFF
}

void agg_attach(unsigned char *buf, int w, int h, int stride) {
	m_graphics.attach(buf, w, h, stride);
}

void agg_setantialias(double v) {
	m_graphics.antiAliasGamma(v);
}
