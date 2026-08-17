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



//// [2026 port] Asset path resolution.
//// The 2007 code fopen()ed bare filenames, so the game only ran with the working directory
//// set to the folder holding the .sgi files. This tries, in order:
////   1. the name as given (preserves the old behaviour)
////   2. $TANK_ASSETS/<name>
////   3. assets/<name>          (running from the project root)
////   4. ../assets/<name>       (running from build/)
static FILE *OpenAsset(const char *fileName)
{
    char buf[1024];
    const char *dir;
    FILE *f;

    if ((f = fopen(fileName, "rb")) != NULL) return f;

    dir = getenv("TANK_ASSETS");
    if (dir && *dir) {
	snprintf(buf, sizeof(buf), "%s/%s", dir, fileName);
	if ((f = fopen(buf, "rb")) != NULL) return f;
    }

    snprintf(buf, sizeof(buf), "assets/%s", fileName);
    if ((f = fopen(buf, "rb")) != NULL) return f;

    snprintf(buf, sizeof(buf), "../assets/%s", fileName);
    if ((f = fopen(buf, "rb")) != NULL) return f;

    return NULL;
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
    // [2026 port] Was: perror(fileName); exit(1);
    // Now searches a few asset locations and returns NULL on failure so the caller can
    // substitute a fallback instead of killing the process. See OpenAsset() above.
    if ((image->file = OpenAsset(fileName)) == NULL) {
	free(image);
	return NULL;
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

#include "Platform.h"

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


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////
////	[2026 port] Texture loading was 16 copy-pasted blocks that each called exit(1) if a file
////	was missing. sky1.sgi is NOT present in the 2007 archive, so the original code exited
////	before opening a window. This version keeps the exact same load order and the same
////	globals, but degrades gracefully:
////		- a missing file falls back to a named substitute, if one is given
////		- otherwise it falls back to a generated magenta/black checkerboard
////	so an incomplete asset set produces obviously-wrong textures instead of a dead process.
////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static unsigned int *MakePlaceholderTexture(void)
{
	const int   size = 256;
	const int   cell = 32;
	unsigned int *px = (unsigned int *)malloc(size * size * sizeof(unsigned int));
	int u, v;

	if (!px) return NULL;

	for (v = 0; v < size; v++)
		for (u = 0; u < size; u++)
		{
			int on = (((u / cell) + (v / cell)) & 1);
			/* RGBA, little-endian byte order matches the rgbatorgba() layout above */
			px[v * size + u] = on ? 0xFFFF00FFu : 0xFF000000u;
		}

	iw = size;
	ih = size;
	return px;
}

static unsigned int *LoadTexture(const char *filename, int *comps, unsigned int *substitute)
{
	unsigned int *tex = read_texture(filename, &iw, &ih, comps);

	if (tex) return tex;

	if (substitute)
	{
		fprintf(stderr, "warning: %s not found -- reusing an already-loaded texture in its place\n", filename);
		return substitute;
	}

	fprintf(stderr, "warning: %s not found -- using a placeholder checkerboard\n", filename);
	return MakePlaceholderTexture();
}

void create_textures(GLsizei *w,  GLsizei *h,
		      GLsizei *padW, GLsizei *padH, int *comps)
{
	(void)w; (void)h; (void)padW; (void)padH;	/* never used by the original either */

	start1    = LoadTexture(start1_filename,    comps, NULL);
	start2    = LoadTexture(start2_filename,    comps, NULL);
	start3    = LoadTexture(start3_filename,    comps, NULL);
	start4    = LoadTexture(start4_filename,    comps, NULL);

	minitank1 = LoadTexture(minitank1_filename, comps, NULL);
	green1    = LoadTexture(green1_filename,    comps, NULL);
	red1      = LoadTexture(red1_filename,      comps, NULL);
	orange1   = LoadTexture(orange1_filename,   comps, NULL);
	yellow1   = LoadTexture(yellow1_filename,   comps, NULL);
	sand1     = LoadTexture(sand1_filename,     comps, NULL);

	/* sky2 is loaded first so it can stand in for the missing sky1.sgi */
	sky2      = LoadTexture(sky2_filename,      comps, NULL);
	sky1      = LoadTexture(sky1_filename,      comps, sky2);

	wall1     = LoadTexture(wall1_filename,     comps, NULL);
	wall2     = LoadTexture(wall2_filename,     comps, NULL);
	steel1    = LoadTexture(steel1_filename,    comps, NULL);
	steel2    = LoadTexture(steel2_filename,    comps, NULL);

	TextureWidth  = iw;
	TextureHeight = ih;
}
