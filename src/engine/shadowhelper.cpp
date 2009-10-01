/***************************************************************************
 *   Copyright 2007 Jani Huhtanen <jani.huhtanen@tut.fi>
 *   Copyright 2009 Stefan Majewsky <majewsky@gmx.net>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
***************************************************************************/

#include "shadowhelper.h"

#include <cmath>
#include <QImage>
#include <QPainter>

// Exponential blur, Jani Huhtanen, 2006
//
template<int aprec, int zprec>
inline void blurinner(unsigned char *bptr, int &zR, int &zG, int &zB, int &zA, int alpha);

template<int aprec,int zprec>
inline void blurrow( QImage & im, int line, int alpha);

template<int aprec, int zprec>
inline void blurcol( QImage & im, int col, int alpha);

/*
*  expblur(QImage &img, int radius)
*
*  In-place blur of image 'img' with kernel
*  of approximate radius 'radius'.
*
*  Blurs with two sided exponential impulse
*  response.
*
*  aprec = precision of alpha parameter
*  in fixed-point format 0.aprec
*
*  zprec = precision of state parameters
*  zR,zG,zB and zA in fp format 8.zprec
*/
template<int aprec,int zprec>
void expblur( QImage &img, int radius )
{
  if(radius<1)
    return;

  /* Calculate the alpha such that 90% of
     the kernel is within the radius.
     (Kernel extends to infinity)
  */
  int alpha = (int)((1<<aprec)*(1.0f-std::exp(-2.3f/(radius+1.f))));

  for(int row=0;row<img.height();row++)
  {
    blurrow<aprec,zprec>(img,row,alpha);
  }

  for(int col=0;col<img.width();col++)
  {
    blurcol<aprec,zprec>(img,col,alpha);
  }
  return;
}

template<int aprec, int zprec>
inline void blurinner(unsigned char *bptr, int &zR, int &zG, int &zB, int &zA, int alpha)
{
  int R,G,B,A;
  R = *bptr;
  G = *(bptr+1);
  B = *(bptr+2);
  A = *(bptr+3);

  zR += (alpha * ((R<<zprec)-zR))>>aprec;
  zG += (alpha * ((G<<zprec)-zG))>>aprec;
  zB += (alpha * ((B<<zprec)-zB))>>aprec;
  zA += (alpha * ((A<<zprec)-zA))>>aprec;

  *bptr =     zR>>zprec;
  *(bptr+1) = zG>>zprec;
  *(bptr+2) = zB>>zprec;
  *(bptr+3) = zA>>zprec;
}

template<int aprec,int zprec>
inline void blurrow( QImage & im, int line, int alpha)
{
  int zR,zG,zB,zA;

  QRgb *ptr = (QRgb *)im.scanLine(line);

  zR = *((unsigned char *)ptr    )<<zprec;
  zG = *((unsigned char *)ptr + 1)<<zprec;
  zB = *((unsigned char *)ptr + 2)<<zprec;
  zA = *((unsigned char *)ptr + 3)<<zprec;

  for(int index=1; index<im.width(); index++)
  {
    blurinner<aprec,zprec>((unsigned char *)&ptr[index],zR,zG,zB,zA,alpha);
  }
  for(int index=im.width()-2; index>=0; index--)
  {
    blurinner<aprec,zprec>((unsigned char *)&ptr[index],zR,zG,zB,zA,alpha);
  }
 

}

template<int aprec, int zprec>
inline void blurcol( QImage & im, int col, int alpha)
{
  int zR,zG,zB,zA;

  QRgb *ptr = (QRgb *)im.bits();
  ptr+=col;

  zR = *((unsigned char *)ptr    )<<zprec;
  zG = *((unsigned char *)ptr + 1)<<zprec;
  zB = *((unsigned char *)ptr + 2)<<zprec;
  zA = *((unsigned char *)ptr + 3)<<zprec;

  for(int index=im.width(); index<(im.height()-1)*im.width(); index+=im.width())
  {
    blurinner<aprec,zprec>((unsigned char *)&ptr[index],zR,zG,zB,zA,alpha);
  }

  for(int index=(im.height()-2)*im.width(); index>=0; index-=im.width())
  {
    blurinner<aprec,zprec>((unsigned char *)&ptr[index],zR,zG,zB,zA,alpha);
  }

}

QPixmap Palapeli::createShadow(const QPixmap& source, int radius)
{
	QPixmap pix(source.size());
	pix.fill(Qt::black);

	QPixmap shadowPixmap(source);
	QPainter px(&shadowPixmap);
	px.setCompositionMode(QPainter::CompositionMode_SourceAtop);
	px.drawPixmap(0, 0, pix);
	px.end();

	QImage shadowImage(QSize(source.width() + 2 * radius, source.height() + 2 * radius), QImage::Format_ARGB32_Premultiplied);
	shadowImage.fill(0x00000000); //transparent
	px.begin(&shadowImage);
	px.drawPixmap(QPoint(radius, radius), shadowPixmap);
	px.end();

	expblur<16,7>(shadowImage, radius);
	return QPixmap::fromImage(shadowImage);
}
