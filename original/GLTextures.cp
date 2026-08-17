//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////		
////	File:			GLTextures.cp																															
////	Author:			Silicon Graphics, Ed Martin
////	Description:	Texture loading code. Came with the Xcode free samples
////					Can only work from 256x256 pixel sgi images.
////
////	Contents:		
////					1) Section written entirely by Silicon Graphics
////
////					2) Functions used by tank to load samples (written by Ed Martin)
////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////
////					1) Section written entirely by Silicon Graphics
////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct _ImageRec {
    unsigned short imagic;
    unsigned short type;
    unsigned short dim;
    unsigned short xsize, ysize, zsize;
    unsigned int min, max;
    unsigned int wasteBytes;
    char name[80];
    unsigned long colorMap;
    FILE *file;
    unsigned char *tmp, *tmpR, *tmpG, *tmpB;
    unsigned long rleEnd;
    unsigned int *rowStart;
    int *rowSize;
} ImageRec;

static void
ImageClose(ImageRec *image) {
    fclose(image->file);
    free(image->tmp);
    free(image->tmpR);
    free(image->tmpG);
    free(image->tmpB);
    free(image);
}


void
bwtorgba(unsigned char *b,unsigned char *l,int n) {
    while(n--) {
	l[0] = *b;
	l[1] = *b;
	l[2] = *b;
	l[3] = 0xff;
	l += 4; b++;
    }
}

void
latorgba(unsigned char *b, unsigned char *a,unsigned char *l,int n) {
    while(n--) {
	l[0] = *b;
	l[1] = *b;
	l[2] = *b;
	l[3] = *a;
	l += 4; b++; a++;
    }
}

void
rgbtorgba(unsigned char *r,unsigned char *g,unsigned char *b,unsigned char *l,int n) {
    while(n--) {
	l[0] = r[0];
	l[1] = g[0];
	l[2] = b[0];
	l[3] = 0xff;
	l += 4; r++; g++; b++;
    }
}

void
rgbatorgba(unsigned char *r,unsigned char *g,unsigned char *b,unsigned char *a,unsigned char *l,int n) {
    while(n--) {
	l[0] = r[0];
	l[1] = g[0];
	l[2] = b[0];
	l[3] = a[0];
        l += 4; r++; g++; b++; a++;
    }
}

static void
ConvertShort(unsigned short *array, long length) {
    unsigned b1, b2;
    unsigned char *ptr;

    ptr = (unsigned char *)array;
    while (length--) {
	b1 = *ptr++;
	b2 = *ptr++;
	*array++ = (b1 << 8) | (b2);
    }
}
static void
ConvertLong(unsigned *array, long length) {
    unsigned b1, b2, b3, b4;
    unsigned char *ptr;

    ptr = (unsigned char *)array;
    while (length--) {
	b1 = *ptr++;
	b2 = *ptr++;
	b3 = *ptr++;
	b4 = *ptr++;
	*array++ = (b1 << 24) | (b2 << 16) | (b3 << 8) | (b4);
    }
}

static void
ImageGetRow(ImageRec *image, unsigned char *buf, int y, int z) {
    unsigned char *iPtr, *oPtr, pixel;
    int count;

    if ((image->type & 0xFF00) == 0x0100) {
	fseek(image->file, (long) image->rowStart[y+z*image->ysize], SEEK_SET);
	fread(image->tmp, 1, (unsigned int)image->rowSize[y+z*image->ysize],
	      image->file);

	iPtr = image->tmp;
	oPtr = buf;
	for (;;) {
	    pixel = *iPtr++;
	    count = (int)(pixel & 0x7F);
	    if (!count) {
		return;
	    }
	    if (pixel & 0x80) {
		while (count--) {
		    *oPtr++ = *iPtr++;
		}
	    } else {
		pixel = *iPtr++;
		while (count--) {
		    *oPtr++ = pixel;
		}
	    }
	}
    } else {
	fseek(image->file, 512+(y*image->xsize)+(z*image->xsize*image->ysize),
	      SEEK_SET);
	fread(buf, 1, image->xsize, image->file);
    }
}



static ImageRec *ImageOpen(const char *fileName)
{
    union {
	int testWord;
	char testByte[4];
    } endianTest;
    ImageRec *image;
    int swapFlag;
    int x;

    endianTest.testWord = 1;
    if (endianTest.testByte[0] == 1) {
	swapFlag = 1;
    } else {
	swapFlag = 0;
    }

    image = (ImageRec *)malloc(sizeof(ImageRec));
    if (image == NULL) {
	fprintf(stderr, "Out of memory!\n");
	exit(1);
    }
    if ((image->file = fopen(fileName, "rb")) == NULL) {
	perror(fileName);
	exit(1);
    }

    fread(image, 1, 12, image->file);

    if (swapFlag) {
	ConvertShort(&image->imagic, 6);
    }

    image->tmp = (unsigned char *)malloc(image->xsize*256);
    image->tmpR = (unsigned char *)malloc(image->xsize*256);
    image->tmpG = (unsigned char *)malloc(image->xsize*256);
    image->tmpB = (unsigned char *)malloc(image->xsize*256);
    if (image->tmp == NULL || image->tmpR == NULL || image->tmpG == NULL ||
	image->tmpB == NULL) {
	fprintf(stderr, "Out of memory!\n");
	exit(1);
    }

    if ((image->type & 0xFF00) == 0x0100) {
	x = image->ysize * image->zsize * sizeof(unsigned);
	image->rowStart = (unsigned *)malloc(x);
	image->rowSize = (int *)malloc(x);
	if (image->rowStart == NULL || image->rowSize == NULL) {
	    fprintf(stderr, "Out of memory!\n");
	    exit(1);
	}
	image->rleEnd = 512 + (2 * x);
	fseek(image->file, 512, SEEK_SET);
	fread(image->rowStart, 1, x, image->file);
	fread(image->rowSize, 1, x, image->file);
	if (swapFlag) {
	    ConvertLong(image->rowStart, x/(int)sizeof(unsigned));
	    ConvertLong((unsigned *)image->rowSize, x/(int)sizeof(int));
	}
    }
    return image;
}
unsigned *
read_texture(const char *name, int *width, int *height, int *components){

    unsigned *base, *lptr;
    unsigned char *rbuf, *gbuf, *bbuf, *abuf;
    ImageRec *image;
    int y;

    image = ImageOpen(name);
    
    if(!image)
	return NULL;
    (*width)=image->xsize;
    (*height)=image->ysize;
    (*components)=image->zsize;
    base = (unsigned *)malloc(image->xsize*image->ysize*sizeof(unsigned));
    rbuf = (unsigned char *)malloc(image->xsize*sizeof(unsigned char));
    gbuf = (unsigned char *)malloc(image->xsize*sizeof(unsigned char));
    bbuf = (unsigned char *)malloc(image->xsize*sizeof(unsigned char));
    abuf = (unsigned char *)malloc(image->xsize*sizeof(unsigned char));
    if(!base || !rbuf || !gbuf || !bbuf)
      return NULL;
    lptr = base;
    for(y=0; y<image->ysize; y++) {
	if(image->zsize>=4) {
	    ImageGetRow(image,rbuf,y,0);
	    ImageGetRow(image,gbuf,y,1);
	    ImageGetRow(image,bbuf,y,2);
	    ImageGetRow(image,abuf,y,3);
	    rgbatorgba(rbuf,gbuf,bbuf,abuf,(unsigned char *)lptr,image->xsize);
	    lptr += image->xsize;
	} else if(image->zsize==3) {
	    ImageGetRow(image,rbuf,y,0);
	    ImageGetRow(image,gbuf,y,1);
	    ImageGetRow(image,bbuf,y,2);
	    rgbtorgba(rbuf,gbuf,bbuf,(unsigned char *)lptr,image->xsize);
	    lptr += image->xsize;
	} else if(image->zsize==2) {
	    ImageGetRow(image,rbuf,y,0);
	    ImageGetRow(image,abuf,y,1);
	    latorgba(rbuf,abuf,(unsigned char *)lptr,image->xsize);
	    lptr += image->xsize;
	} else {
	    ImageGetRow(image,rbuf,y,0);
	    bwtorgba(rbuf,(unsigned char *)lptr,image->xsize);
	    lptr += image->xsize;
	}
    }
    ImageClose(image);
    free(rbuf);
    free(gbuf);
    free(bbuf);
    free(abuf);

    return (unsigned *) base;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////
////					2) Functions used by tank to load samples (written by Ed Martin)
////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <GLUT/glut.h>

#include "GameConstants.cp"

const char start1_filename[]	= "start1.sgi"	 ;
const char start2_filename[]	= "start2.sgi"	 ;
const char start3_filename[]	= "start3.sgi"	 ;
const char start4_filename[]	= "start4.sgi"	 ;

const char minitank1_filename[]	= "minitank1.sgi";
const char green1_filename[]	= "green1.sgi"	 ;
const char red1_filename[]		= "red1.sgi"	 ;
const char yellow1_filename[]	= "yellow1.sgi"	 ;
const char orange1_filename[]	= "orange1.sgi"	 ;
const char sky1_filename[]		= "sky1.sgi"	 ;
const char sky2_filename[]		= "sky2.sgi"	 ;
const char sand1_filename[]		= "sand1.sgi"	 ;
const char wall1_filename[]		= "wall1.sgi"	 ;
const char wall2_filename[]		= "wall2.sgi"	 ;
const char steel1_filename[]	= "steel1.sgi"	 ;
const char steel2_filename[]	= "steel2.sgi"	 ;

void create_textures(GLsizei *w,  GLsizei *h, 
		      GLsizei *padW, GLsizei *padH, int *comps)
{
	start1 = read_texture(start1_filename, &iw, &ih, comps);
			if (!start1) 
			{
				fprintf(stderr, "Could not open %s\n", start1_filename);
				exit(1);
			}
	start2 = read_texture(start2_filename, &iw, &ih, comps);
			if (!start2) 
			{
				fprintf(stderr, "Could not open %s\n", start2_filename);
				exit(1);
			}
	start3 = read_texture(start3_filename, &iw, &ih, comps);
			if (!start3) 
			{
				fprintf(stderr, "Could not open %s\n", start3_filename);
				exit(1);
			}
	start4 = read_texture(start4_filename, &iw, &ih, comps);
			if (!start4) 
			{
				fprintf(stderr, "Could not open %s\n", start4_filename);
				exit(1);
			}

	minitank1 = read_texture(minitank1_filename, &iw, &ih, comps);
			if (!minitank1) 
			{
				fprintf(stderr, "Could not open %s\n", minitank1_filename);
				exit(1);
			}
	green1 = read_texture(green1_filename, &iw, &ih, comps);
			if (!green1) 
			{
				fprintf(stderr, "Could not open %s\n", green1_filename);
				exit(1);
			}
	red1 = read_texture(red1_filename, &iw, &ih, comps);
			if (!red1) 
			{
				fprintf(stderr, "Could not open %s\n", red1_filename);
				exit(1);
			}
	orange1 = read_texture(orange1_filename, &iw, &ih, comps);
			if (!orange1) 
			{
				fprintf(stderr, "Could not open %s\n", orange1_filename);
				exit(1);
			}
	yellow1 = read_texture(yellow1_filename, &iw, &ih, comps);
			if (!yellow1) 
			{
				fprintf(stderr, "Could not open %s\n", yellow1_filename);
				exit(1);
			}
	sand1 = read_texture(sand1_filename, &iw, &ih, comps);
			if (!sand1) 
			{
				fprintf(stderr, "Could not open %s\n", sand1_filename);
				exit(1);
			}
	sky1 = read_texture(sky1_filename, &iw, &ih, comps);
			if (!sky1) 
			{
				fprintf(stderr, "Could not open %s\n", sky1_filename);
				exit(1);
			}
	sky2 = read_texture(sky2_filename, &iw, &ih, comps);
			if (!sky2) 
			{
				fprintf(stderr, "Could not open %s\n", sky2_filename);
				exit(1);
			}
	wall1 = read_texture(wall1_filename, &iw, &ih, comps);
			if (!wall1) 
			{
				fprintf(stderr, "Could not open %s\n", wall1_filename);
				exit(1);
			}
	wall2 = read_texture(wall2_filename, &iw, &ih, comps);
			if (!wall2) 
			{
				fprintf(stderr, "Could not open %s\n", wall2_filename);
				exit(1);
			}
	steel1 = read_texture(steel1_filename, &iw, &ih, comps);
			if (!steel1) 
			{
				fprintf(stderr, "Could not open %s\n", steel1_filename);
				exit(1);
			}	
	steel2 = read_texture(steel2_filename, &iw, &ih, comps);
			if (!steel2) 
			{
				fprintf(stderr, "Could not open %s\n", steel2_filename);
				exit(1);
			}	
	
	TextureWidth = iw;	
	TextureHeight = ih;

}