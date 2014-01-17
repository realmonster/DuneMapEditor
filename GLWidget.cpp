#include <QtGui>
#include <QtWidgets>
#include <QtOpenGL>

#include <math.h>
#include <time.h>

#include "GLWidget.h"
#include "Dunes.h"
#include "DuneStructures.h"

#ifndef _WIN32
struct POINT
{
    int x, y;
};

#endif

POINT camera = {0,0};
POINT mouse;
bool mouse1down = false;
bool mouse2down = false;

double zoom = 1;
bool showgrey = false;
bool showunits = true;
int state = 0;
int drawtype = 0;
int drawsize = 1;
bool drawinverse = false;

struct Ground
{
	int width;
	int height;
	double *data;
	Ground(int _width, int _height)
	{
		width = _width;
		height = _height;
		data = (double *)malloc(width*height*sizeof(double));
		for (int x=0; x<width; ++x)
			for (int y=0; y<height; ++y)
				(*this)[x][y] = 0;
	}

	bool in(int x, int y)
	{
		if (x<0
		 || y<0
		 || x>=width
		 || y>=height)
			return false;
		return true;
	}

	double *operator [](int x)
	{
		return data+x*height;
	}

	~Ground()
	{
		if (data)
			free(data);
	}
};

struct DuneGround
{
	enum Types {Dust, Ground, SpiceLow, SpiceHigh, Dune};
	int width;
	int height;
	unsigned char *data;
	unsigned char *types;
	DuneGround(int _width, int _height)
	{
		width = _width;
		height = _height;
		data = (unsigned char *)malloc(width*height*sizeof(unsigned char));
		types = (unsigned char *)malloc(twidth()*theight()*sizeof(unsigned char));
		Clear();
	}

	~DuneGround()
	{
		if (data)
			free(data);
		if (types)
			free(types);
	}

	DuneGround(const DuneGround & g)
	{
		data = 0;
		types = 0;
		width = 0;
		height = 0;
		(*this) = g;
	}

	const DuneGround & operator = (const DuneGround &g)
	{
		if (width != g.width
		 || height != g.height)
		{
			if (data)
				free(data);
			if (types)
				free(types);
			data = (unsigned char *)malloc(g.width*g.height*sizeof(unsigned char));
			types = (unsigned char *)malloc(g.twidth()*g.theight()*sizeof(unsigned char));
		}
		width = g.width;
		height = g.height;
		memcpy(data,g.data,width*height*sizeof(unsigned char));
		memcpy(types,g.types,twidth()*theight()*sizeof(unsigned char));
		return (*this);
	}

	void Clear()
	{
		for (int x=0; x<width; ++x)
			for (int y=0; y<height; ++y)
				(*this)[x][y] = 0xB0;
		for (int x=0; x<twidth(); ++x)
			for (int y=0; y<theight(); ++y)
				this->t(x,y) = Dust;
	}

	inline int twidth() const
	{
		return width*2+1;
	}

	inline int theight() const
	{
		return height*2+1;
	}

	inline bool tin(int x, int y) const
	{
		return (x>=0 && y>=0 && x<twidth() && y<theight());
	}

	inline bool in(int x, int y) const
	{
		return (x>=0 && y>=0 && x<width && y<height);
	}

	void Draw(int x, int y, unsigned char type)
	{
	    //   0   1   2
	    // 0 1 2 3 4 5 6
		this->t(x,y) = type;
		if (in(x/2, y/2))
			Correct(x/2, y/2, type);
		if (in(x/2-1, y/2))
			Correct(x/2-1,y/2,type);
		if (in(x/2, y/2-1))
			Correct(x/2,y/2-1,type);
		if (in(x/2-1, y/2-1))
			Correct(x/2-1,y/2-1,type);
	}

	void Correct(int x, int y, unsigned char type_draw)
	{
		int k = 0;
		if (type_draw != Dust)
			k = GetK(x,y,type_draw);
		int allowed = 0;
		int replace = Dust;
		if (type_draw == Ground)
			allowed = ((1<<Ground) | (1<<Dust));
		if (type_draw == SpiceLow)
			allowed = ((1<<SpiceLow) | (1<<SpiceHigh) | (1<<Dust));
		if (type_draw == SpiceHigh)
		{
			allowed = ((1<<SpiceLow) | (1<<SpiceHigh));
			replace = SpiceLow;
		}
		int fmask = make_dunemask(0,1,0,1,1,1,0,1,0);
		if (type_draw != Dune && k != 0)
		{
			for (int i=0; i<3; ++i)
				for (int j=0; j<3; ++j)
					if ((~allowed)&(1<<t(i+x*2,j+y*2))
					&& (((fmask & (1<<(i+j*3))) || type_draw == SpiceHigh)
					|| t(i+x*2,j+y*2) == Dune) )
						Draw(i+x*2,j+y*2,replace);
		}
		if (type_draw == Dune && k != 0)
		{
			for (int i=0; i<3; ++i)
				for (int j=0; j<3; ++j)
					if ((fmask & (1<<(i+j*3)))
					 &&	t(i+x*2,j+y*2) != Dune
					 && t(i+x*2,j+y*2) != Dust)
						Draw(i+x*2,j+y*2,Dust);
			/*int k = GetK(x,y,Dune);
			int mask = dunemask[k];
			for (int i=0; i<3; ++i)
				for (int j=0; j<3; ++j)
				{
					if ((mask & (1<<(i+j*3)))
					 && t(i+x*2,j+y*2) != Dune)
						Draw(i+x*2,j+y*2,Dune);
					if (!(mask & (1<<(i+j*3)))
					 && t(i+x*2,j+y*2) == Dune)
						Draw(i+x*2,j+y*2,Dust);
				}*/
		}
		for (int i=-1; i<2; ++i)
			for (int j=-1; j<2; ++j)
				if (in(x+i,y+j))
					Update(x+i, y+j);
	}

	int GetK(int x, int y, unsigned char type)
	{
		int mask = 0;
		int k = 0;
		for (int i=0; i<3; ++i)
			for (int j=0; j<3; ++j)
				if (t(i+x*2,j+y*2) == type
				|| (type==SpiceLow && t(i+x*2,j+y*2) == SpiceHigh))
					mask |= 1<<(i+j*3);
		//1    2   4
		//8   16  32
		//64 128 256
		if ( type == Dune )
		{
			k = 0;
			int min = 10;
			for (int i=0; i<256; ++i)
				if (dunemask[i] != -1)
				{
					int d = DunemaskDist((dunemask[i] & DUNE_MASK), (mask & DUNE_MASK));
					if (((dunemask[i] & DUNE_MASK) & (mask & DUNE_MASK)) == (mask & DUNE_MASK)
					 && d < min)
					{
						k = i;
						min = d;
					}
				}
			for (int i=0; i<256; ++i)
				if (dunemask[i] != -1)
				{
					if (((dunemask[k] & DUNE_MASK)|(mask&16)) == (dunemask[i] & DUNE_MASKMID))
						k = i;
				}
			if (k == 0x9C && (mask & 2))
				k = 0x60;
			if (dunemask[k] == 0)
				k = 0;
		}
		else
		{
			if ( (mask&(1+2+4)) == (1+2+4))
				k++;
			if ( (mask&(4+32+256)) == (4+32+256))
				k |= 2;
			if ( (mask&(64+128+256)) == (64+128+256))
				k |= 4;
			if ( (mask&(1+8+64)) == (1+8+64))
				k |= 8;

			if ( type == Ground && k == 0 && (mask & 16)) // point
				k = 16;
			if ( k == 3 && !(mask & 16))
				k = 17;
			if ( k == 6 && !(mask & 16))
				k = 18;
			if ( k == 12 && !(mask & 16))
				k = 20;
			if ( k == 9 && !(mask & 16))
				k = 19;
		}
		return k;
	}
	void Update(int x, int y)
	{
		int types_mask = 0;
		for (int type = 0; type < 5; ++type)
		{
			if (type != Dune)
			{
				if (GetK(x,y,type))
					types_mask |= 1<<type;
				continue;
			}
			bool was = false;
			for (int i=0; i<3; ++i)
				for (int j=0; j<3; ++j)
					if (t(i+x*2,j+y*2) == type)
						was = true;
		    if (was)
				types_mask |= 1<<type;
		}

		int k = 0;
		int mask = 0;
		int id = 0xB0;
		if (types_mask & (1<<Ground))
		{
			k = GetK(x,y,Ground);
			id = 0x80+k;
			if (k == 0)
				id = 0xB0;
			if ( k == 16) // point
				id = 0x80;
			if ( k == 17)
				id = 0x3C;
			if ( k == 18)
				id = 0x3D;
			if ( k == 20)
				id = 0x3F;
			if ( k == 19)
				id = 0x3E;
		}

		if (types_mask & (1<<SpiceLow))
		{
			k = GetK(x,y,SpiceLow);
			id = 0xB0+k;
			if ( k == 17)
				id = 0x40;
			if ( k == 18)
				id = 0x41;
			if ( k == 20)
				id = 0x43;
			if ( k == 19)
				id = 0x42;
		}

		if (types_mask & (1<<SpiceHigh))
		{
			k = GetK(x,y,SpiceHigh);
			id = 0xC0+k;
			if ( k == 17 )
				id = 0x44;
			if ( k == 18 )
				id = 0x45;
			if ( k == 20 )
				id = 0x47;
			if ( k == 19 )
				id = 0x46;
		}
		if (types_mask & (1<<Dune))
		{
			k = GetK(x,y,Dune);
			if (k)
				id = k;
			//id = 0x9F;
		}
		(*this)[x][y] = id;
	}

	void SetTileMask(int x, int y)
	{
		int id = (*this)[x][y];
		int type = Dust;
		int mask = 0;
		int k = 0;
		if (id > 0x80 && id <= 0x8F)
		{
			type = Ground; k = id-0x80;
		}
		if (id == 0x80)
		{
			type = Ground; k = 16;
		}
		if (id == 0x3C)
		{
			type = Ground; k = 17;
		}
		if (id == 0x3D)
		{
			type = Ground; k = 18;
		}
		if (id == 0x3F)
		{
			type = Ground; k = 20;
		}
		if (id == 0x3E)
		{
			type = Ground; k = 19;
		}

		if (id == 0x80)
		{
			type = Ground; k = 16;
		}

		if (id > 0xB0 && id <= 0xBF)
		{
			type = SpiceLow; k = id-0xB0;
		}

		if (id == 0x40)
		{
			type = SpiceLow; k = 17;
		}
		if (id == 0x41)
		{
			type = SpiceLow; k = 18;
		}
		if (id == 0x43)
		{
			type = SpiceLow; k = 20;
		}
		if (id == 0x42)
		{
			type = SpiceLow; k = 19;
		}

		if (id > 0xC0 && id <= 0xCF)
		{
			type = SpiceHigh; k = id-0xC0;
		}

		if (id == 0x44)
		{
			type = SpiceHigh; k = 17;
		}
		if (id == 0x45)
		{
			type = SpiceHigh; k = 18;
		}
		if (id == 0x47)
		{
			type = SpiceHigh; k = 20;
		}
		if (id == 0x46)
		{
			type = SpiceHigh; k = 19;
		}

		if (type != Dust)
		{
			if (k<16)
			{
				mask = 0;
				if (k & 1)
					mask |= (1+2+4);
				if (k & 2)
					mask |= (4+32+256);
				if (k & 4)
					mask |= (64+128+256);
				if (k & 8)
					mask |= (1+8+64);
				if (k != 1
				 && k != 2
				 && k != 4
				 && k != 8)
					mask |= 16;
			}
			else
			{
				if (k == 16)
					mask = 16;
				if (k == 17)
					mask = make_dunemask(1,1,1,0,0,1,0,0,1);
				if (k == 18)
					mask = make_dunemask(0,0,1,0,0,1,1,1,1);
				if (k == 20)
					mask = make_dunemask(1,0,0,1,0,0,1,1,1);
				if (k == 19)
					mask = make_dunemask(1,1,1,1,0,0,1,0,0);
			}
		}

		for (int i=0; i<256; ++i)
			if (dunemask[i] != -1 && i == id)
			{
				mask = dunemask[i];
				type = Dune;
			}
		int a = type;
		int b = Dust;
		if (type == SpiceHigh)
			b = SpiceLow;
		for (int i=0; i<3; ++i)
			for (int j=0; j<3; ++j)
				if (mask & (1<<(i+j*3)))
				{
					if (!(type == SpiceLow && t(i+x*2,j+y*2) == SpiceHigh))
						t(i+x*2,j+y*2) = a;
				}
				else
				{
					if (b == SpiceLow && t(i+x*2,j+y*2) != SpiceLow)
						t(i+x*2,j+y*2) = b;
				}
	}

	unsigned char *operator [](int x)
	{
		return data+x*height;
	}

	unsigned char & t(int x,int y)
	{
		return types[y*twidth()+x];
	}
};

struct DuneUnit
{
	short house;
	short id;
	short life;
	short pos;
	short angle;
	short ai;
};

struct DuneStructure
{
	short flag;
	short house;
	short id;
	short life;
	short pos;
};

void ChangeState(int _state)
{
	if (state == _state)
		return;
	state = _state;
}

DuneGround duneGround(64,64);
DuneGround duneGroundNew(64,64);
std::vector<DuneUnit> Units;
std::vector<DuneStructure> Structures;

Ground g(2*64+1,2*64+1);
GLuint GroundTiles;
GLuint GroundGrey;
GLuint StructuresTexture;

QGLFormat desiredFormat()
{
    QGLFormat fmt;
    fmt.setSwapInterval(1);
    return fmt;
}

GLWidget::GLWidget(QWidget *parent)
    : QGLWidget(desiredFormat(), parent),
    m_program(0)
{
    connect(&timer, SIGNAL(timeout()), this, SLOT(updateGL()));
    if(format().swapInterval() == -1)
    {
        // V_blank synchronization unavailable
        timer.setInterval(1);
    }
    else
    {
        // V_blank synchronization available
        timer.setInterval(0);
    }
    timer.start();
}

GLWidget::~GLWidget()
{
}

QSize GLWidget::minimumSizeHint() const
{
    return QSize(50, 50);
}

QSize GLWidget::sizeHint() const
{
    return QSize(400, 400);
}

GLuint MakeGreyTexture()
{
	unsigned char *data = (unsigned char*)malloc( (g.width-1) * (g.height-1) * 3 );

	for (int x=0; x<g.width-1; ++x)
	{
		for (int y=0; y<g.height-1; ++y)
		{
			for (int i=0; i<3; ++i)
				data[(x + y*(g.width-1))*3 + i] = g[x][y]*255;
		}
	}

	GLuint texture;
	glGenTextures( 1, &texture );

	glBindTexture( GL_TEXTURE_2D, texture );

	glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );

	// when texture area is small, bilinear filter the closest MIP map
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );

	// when texture area is large, bilinear filter the first MIP map
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );

	// if wrap is true, the texture wraps over at the edges (repeat)
	//       ... false, the texture ends at the edges (clamp)
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );

	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, g.width-1, g.height-1, 0, GL_RGB, GL_UNSIGNED_BYTE, data );

	free(data);

	return texture;
}

// load a 512x512 24 bit RGB .BMP file as a texture
GLuint LoadTextureRAW( const char * filename, int wrap )
{
	GLuint texture;
	int width, height;
	unsigned char * data;

	// open texture data
    QImage img(filename);

	// allocate buffer
    width = img.width();
    height = img.height();
    data = (unsigned char*)malloc( width * height * 4 );

    QPoint p;
    for (p.rx()=0; p.rx()<width; ++p.rx())
        for (p.ry()=0; p.ry()<height; ++p.ry())
        {
            QColor c = QColor(img.pixel(p));
            int i = p.rx()+p.ry()*width;
            data[i*4] = c.red();
            data[i*4+1] = c.green();
            data[i*4+2] = c.blue();
            data[i*4+3] = c.alpha();
            if (c.red()>0xE0
             && c.green() == 0
             && c.blue()>0xE0)
                data[i*4+3] = 0;
        }

	// allocate a texture name
	glGenTextures( 1, &texture );

	// select our current texture
	glBindTexture( GL_TEXTURE_2D, texture );

	// select modulate to mix texture with color for shading
	glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );

	// when texture area is small, bilinear filter the closest MIP map
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );

	// when texture area is large, bilinear filter the first MIP map
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );

	// if wrap is true, the texture wraps over at the edges (repeat)
	//       ... false, the texture ends at the edges (clamp)
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
				   wrap ? GL_REPEAT : GL_CLAMP );
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
				   wrap ? GL_REPEAT : GL_CLAMP );

    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, width,
    height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data );

	// free buffer
	free( data );

	return texture;

}

GLuint LoadGreyTexture( const char * filename )
{
	GLuint texture;
	int width, height;
	unsigned char * data;
	FILE * file;
	FILE *f;
	unsigned char tmp;
	int i;

	// open texture data
	file = fopen( filename, "rb" );
	if ( file == NULL ) return 0;

	// allocate buffer
	width = 128;
	height = 128;
	data = (unsigned char*)malloc( width * height * 4 );

	// read texture data

	fseek( file, 0x36, SEEK_SET);
	fread( data, width * height * 3, 1, file );
	fclose( file );

	for (i=0; i<width*height; ++i)
	{
		int all = 0;
		for (int j=0; j<3; ++j)
			all += data[i*3+j];
		all /= 3;
		g[i%width][width - 1 - i/width] = all/255.0;
	}

	free(data);

	return MakeGreyTexture();
}

double randf()
{
	return rand()/double(RAND_MAX);
}

// Midpoint displacement
void GenerateGroundRecursive(Ground &g, int x, int y, int width, int height)
{
	if (width <= 1 || height <= 1)
		return;
	double a,b,c,d;
	a = g[x      ][y      ];
	b = g[x+width][y      ];
	c = g[x      ][y+height];
	d = g[x+width][y+height];

	g[(x + (x + width))/2][ y                  ] = (a + b)/2;
	g[ x                 ][(y + (y + height))/2] = (a + c)/2;
	g[(x + (x + width))/2][ y + height         ] = (c + d)/2;
	g[ x + width         ][(y + (y + height))/2] = (b + d)/2;

	double z = (a+b+c+d)/4+((-1+randf()*2)*width/64/2);
	if (z > 1)
		z = 1;
	if (z < 0)
		z = 0;
	g[(x + (x + width))/2][(y + (y + height))/2] = z;

	GenerateGroundRecursive(g, x          , y           , width/2, height/2);
	GenerateGroundRecursive(g, x + width/2, y           , width/2, height/2);
	GenerateGroundRecursive(g, x          , y + height/2, width/2, height/2);
	GenerateGroundRecursive(g, x + width/2, y + height/2, width/2, height/2);
}

void LocalPick(int x, int y, int type, double z, bool inverse)
{
	/*static std::vector<bool> was(129*129);
	if (was.size() != duneGround.width * duneGround.height)
		was.resize(duneGround.width * duneGround.height);

	was.assign(was.size(), false);*/
	static bool was[129][129];
	memset(was,0,sizeof(was));

	duneGroundNew = duneGround;

	static int X[(2*64+1)*(2*64+1)];
	static int Y[(2*64+1)*(2*64+1)];
	int s = 0,e = 0;
	X[e] = x;
	Y[e] = y;
	was[x][y] = true;
	++e;
	while (s < e)
	{
		for (int i=-1; i<2; ++i)
			for (int j=-1; j<2; ++j)
				if (duneGround.tin(X[s]+i, Y[s]+j)
				 && !was[X[s]+i][Y[s]+j]
				 && ((!inverse && g[X[s]+i][Y[s]+j]>=z) || (inverse && g[X[s]+i][Y[s]+j]<=z) ))
				{
					X[e] = X[s]+i;
					Y[e] = Y[s]+j;
					was[X[e]][Y[e]] = true;
					duneGroundNew.Draw(X[e],Y[e],type);
					++e;
				}
		++s;
	}
	//char str[50];
	//sprintf(str,"%d %d < %d", s, e, (2*64+1)*(2*64+1));
	//MessageBox(NULL,"Oo",str,MB_OK);
}

void RangeGround(double z)
{
	for (int x=0; x<64; ++x)
	{
		for (int y=0; y<64; ++y)
		{
			int mask = 0;
			for (int i=0; i<3; ++i)
				for (int j=0; j<3; ++j)
					if (g[i+x*2][j+y*2]>z)
						mask |= 1<<(i+j*3);
			int k = 0;
			//1    2   4
			//8   16  32
			//64 128 256
			if ( (mask&(1+2+4)) == (1+2+4))
				k++;
			if ( (mask&(4+32+256)) == (4+32+256))
				k |= 2;
			if ( (mask&(64+128+256)) == (64+128+256))
				k |= 4;
			if ( (mask&(1+8+64)) == (1+8+64))
				k |= 8;
			int id = 0x80+k;
			if ( k == 0)
				id = 0xB0;
			if ( k == 0 && (mask & 16))
				id = 0x80;
			if ( k == 3 && !(mask & 16))
				id = 0x3C;
			if ( k == 6 && !(mask & 16))
				id = 0x3D;
			if ( k == 12 && !(mask & 16))
				id = 0x3F;
			if ( k == 9 && !(mask & 16))
				id = 0x3E;

			duneGround[x][y] = id;
		}
	}
}

// Diamond Square algorithm
void GenerateGroundCycle(Ground &g, int width)
{
	for (int step = width; step >= 1; step/=2)
	{
		int step2 = step/2;
		for (int y=step2; y<width; y+=step)
		{
			for (int x=step2; x<width; x+=step)
			{
				g[x][y] = (g[x-step2][y-step2]
						+ g[x+step2][y-step2]
						+ g[x-step2][y+step2]
						+ g[x+step2][y+step2])/4 + ((-1+randf()*2)*step2/64);
				if (g[x][y]>1)
					g[x][y]=1;
				if (g[x][y]<0)
					g[x][y]=0;
			}
		}

		for (int z=0; z<2; ++z)
		{
			for (int y=z*step2; y<width; y+=step)
			{
				for (int x=(1-z)*step2; x<width; x+=step)
				{
					double h = 0;
					int n = 0;
					if (g.in(x-step2,y))
					{
						h += g[x-step2][y];
						++n;
					}
					if (g.in(x+step2,y))
					{
						h += g[x+step2][y];
						++n;
					}
					if (g.in(x,y-step2))
					{
						h += g[x][y-step2];
						++n;
					}
					if (g.in(x,y+step2))
					{
						h += g[x][y+step2];
						++n;
					}
					h /= n;
					h += ((-1+randf()*2)*step2/64);
					if (h>1)
						h=1;
					if (h<0)
						h=0;
					g[x][y] = h;
				}
			}
		}
	}
}

GLuint GenerateGround()
{
	g[        0][         0] = randf();
	g[g.width-1][         0] = randf();
	g[        0][g.height-1] = randf();
	g[g.width-1][g.height-1] = randf();

	//GenerateGroundRecursive(g, 0, 0, g.width-1, g.height-1);
	GenerateGroundCycle(g, g.width-1);

	return MakeGreyTexture();
}

void GLWidget::initializeGL()
{
    // load our texture
    GroundTiles = LoadTextureRAW(":/images/tex1.png", 1);
    StructuresTexture = LoadTextureRAW(":/images/structures1.png", 1);
	srand(time(0));
	GroundGrey = GenerateGround();
}

void GLWidget::paintGL()
{
	// OpenGL animation code goes here
	//RECT rc;
	//GetClientRect(hWnd,&rc);
	double cx = (camera.x+mouse.x-(width()/2))/32.0/zoom*2+0.5;
	double cy = (camera.y+mouse.y-(height()/2))/32.0/zoom*2+0.5;
	if (state == 2 && duneGround.tin(cx,cy))
	{
		int x = cx;
		int y = cy;
		double n = 0;
		double z = 0;
		for (int i=0; i<2; ++i)
			for (int j=0; j<2; ++j)
				if ( duneGround.tin(x+i,y+j) )
				{
					double w = (1-(cx-x-i))*(1-(cy-y-j));
					n += w;
					z += w*g[x+i][y+j];
				}
		z /= n;
		LocalPick(cx,cy,drawtype,z,drawinverse);//RangeGround(g[(int)(cx*2)][(int)(cy*2)]);*/
	}

	glViewport(0, 0, width(), height());

	glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );
    glClear(GL_COLOR_BUFFER_BIT);

	glMatrixMode( GL_PROJECTION );
    glLoadIdentity();

	double scale = 1/32.0/zoom;
	glScalef(64.0/width()*zoom,64.0/height()*zoom,1);
	glTranslatef( -camera.x*scale, camera.y*scale, 0);
	//glOrtho(0, 1, 0, 1, -1, 1);

	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();

	// setup texture mapping
	glEnable( GL_TEXTURE_2D );
	glBindTexture( GL_TEXTURE_2D, GroundTiles );

	glPushMatrix();
	glBegin( GL_QUADS );

	double tw = 1.0/16;
	//else
	{
		for (int i=0; i<duneGround.width; ++i)
		{
			for (int j=0; j<duneGround.height; ++j)
			{
				int id;
				if (state == 2)
					id = duneGroundNew[j][i];
				else
					id = duneGround[j][i];
				int idx = id&15;
				int idy = (id>>4);
                glTexCoord2d(tw*(idx+0),tw*(idy+0)); glVertex2d(j,-i);
                glTexCoord2d(tw*(idx+1),tw*(idy+0)); glVertex2d(j+1,-i);
                glTexCoord2d(tw*(idx+1),tw*(idy+1)); glVertex2d(j+1,-(i+1));
                glTexCoord2d(tw*(idx+0),tw*(idy+1)); glVertex2d(j,-(i+1));
			}
		}
	}
	glEnd();

	if (state == 1)
	{
		glEnable( GL_BLEND );
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
		glColor4f(1.f,1.f,1.f,0.5f);
		glBegin( GL_QUADS );
		for (int y=0; y<duneGround.theight(); ++y)
		{
			for (int x=0; x<duneGround.twidth(); ++x)
			{
				static int tid[]={0xB0,0x8F,0xC0,0xCF,0x9F};
				int id = duneGround.t(x,y);
				id = tid[id];
				int idx = id&15;
				int idy = (id>>4);
                glTexCoord2d(tw*(idx+0),tw*(idy+0)); glVertex2d(x*0.5-0.25,-(y*0.5-0.25));
                glTexCoord2d(tw*(idx+1),tw*(idy+0)); glVertex2d(x*0.5+0.5-0.25,-(y*0.5-0.25));
                glTexCoord2d(tw*(idx+1),tw*(idy+1)); glVertex2d(x*0.5+0.5-0.25,-(y*0.5+0.5-0.25));
                glTexCoord2d(tw*(idx+0),tw*(idy+1)); glVertex2d(x*0.5-0.25,-(y*0.5+0.5-0.25));
			}
		}
		glEnd();
		glDisable( GL_BLEND );
	}

	if (showunits)
	{
		glBindTexture( GL_TEXTURE_2D, StructuresTexture );
		glBegin( GL_QUADS );
		for (int i = 0; i < Structures.size(); ++i)
		{
			int x = (Structures[i].pos&0x3F);
			int y = (Structures[i].pos/0x40);
			int id = Structures[i].id;
			DrawInfos si = StructureDrawInfos[id];
            glTexCoord2d((si.x+       0)/512.0,(si.y+        0)/512.0); glVertex2d(x,            -y);
            glTexCoord2d((si.x+si.width)/512.0,(si.y+        0)/512.0); glVertex2d(x+si.width/32,-y);
            glTexCoord2d((si.x+si.width)/512.0,(si.y+si.height)/512.0); glVertex2d(x+si.width/32,-(y+si.height/32));
            glTexCoord2d((si.x+       0)/512.0,(si.y+si.height)/512.0); glVertex2d(x,            -(y+si.height/32));
		}
		glEnd();


        glEnable( GL_BLEND );
        glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
        glBindTexture( GL_TEXTURE_2D, StructuresTexture );
		glBegin( GL_QUADS );

        double pi = qAcos(-1.0);
		for (int i = 0; i < Units.size(); ++i)
		{
			int x = (Units[i].pos&0x3F);
			int y = (Units[i].pos/0x40);
			int id = Units[i].id;
            if (id == 0x19)
                id = 0x12;
			int idx = id&15;
            int idy = (id>>4)+6;
            double a = (Units[i].angle/32)*pi/4;
            glTexCoord2d(tw*(idx+0),tw*(idy+0)); glVertex2d(x+0.5-0.5*cos(a)+0.5*sin(a),-(y+0.5-0.5*sin(a)-0.5*cos(a)));
            glTexCoord2d(tw*(idx+1),tw*(idy+0)); glVertex2d(x+0.5+0.5*cos(a)+0.5*sin(a),-(y+0.5+0.5*sin(a)-0.5*cos(a)));
            glTexCoord2d(tw*(idx+1),tw*(idy+1)); glVertex2d(x+0.5+0.5*cos(a)-0.5*sin(a),-(y+0.5+0.5*sin(a)+0.5*cos(a)));
            glTexCoord2d(tw*(idx+0),tw*(idy+1)); glVertex2d(x+0.5-0.5*cos(a)-0.5*sin(a),-(y+0.5-0.5*sin(a)+0.5*cos(a)));
		}
		glEnd();
        glDisable( GL_BLEND );
	}

	if (showgrey)
	{
		glBindTexture( GL_TEXTURE_2D, GroundGrey );
		glBegin( GL_QUADS );
			  glTexCoord2d(0,0); glVertex2d(0,-0);
			  glTexCoord2d(1,0); glVertex2d(64,-0);
			  glTexCoord2d(1,1); glVertex2d(64,-64);
			  glTexCoord2d(0,1); glVertex2d(0,-64);
		glEnd();
	}

	glPopMatrix();
}

void GLWidget::resizeGL(int width, int height)
{
    /*
    int side = qMin(width, height);
    glViewport((width - side) / 2, (height - side) / 2, side, side);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-0.5, +0.5, -0.5, +0.5, 4.0, 15.0);
    glMatrixMode(GL_MODELVIEW);
    */
}

void FreeTexture( GLuint texture )
{
	glDeleteTextures( 1, &texture );
}

void LoadMap( const char * filename )
{
    FILE *f = fopen(filename, "rb");
    if (!f)
        return;
    unsigned char tmp;
    duneGround.Clear();
    for (int y=0; y<duneGround.height; ++y)
        for (int x=0; x<duneGround.width; ++x)
        {
            fread(&tmp,1,1,f);
            duneGround[x][y]=tmp;
            duneGround.SetTileMask(x,y);
        }
    fclose(f);
}

void SaveMap( const char * filename )
{
    FILE *f = fopen(filename,"wb");
    if (!f)
    	return;
    for (int y=0; y<duneGround.height; ++y)
	    for (int x=0; x<duneGround.width; ++x)
	         fwrite(&duneGround[x][y],1,1,f);
    fclose(f);
}

void LoadMission( const char * filename )
{
	FILE *f = fopen(filename, "rb");
	if (!f)
		return;
	//FILE *log = fopen("log.txt","w");
	unsigned char buff[20];
	Units.clear();
	Structures.clear();
	DuneUnit unit;
	DuneStructure structure;
	for (;;)
	{
		unsigned char cmd;
		unsigned char subcmd;
		if (!fread(&cmd,1,1,f))
			break;
		fread(&subcmd,1,1,f);
		//fprintf(log,"cmd = %d, subcmd = %d, offset = %X\n",cmd,subcmd,ftell(f)-2);
		if (cmd & 0x80)
			break;
		switch(cmd)
		{
			// Settings
			case 0:
				switch(subcmd)
				{
					// LosePicture
					case 0:
					// WinPicture
					case 1:
					// BriefPicture
					case 2:
						fread(buff,1,2,f);
						fseek(f,(buff[0]<<8)|buff[1],SEEK_CUR);
						break;

					// TimeOut
					case 3:
					// MapScale
					case 4:
					// CursorPos
					case 5:
					// TacticalPos
					case 6:
					// LoseFlags
					case 7:
					// WinFlags
					case 8:
						fread(buff,1,2,f);
						break;
				}
				break;
			// MAP
			case 1:
				switch(subcmd)
				{
					// Bloom
					case 'B':
					// Field
					case 'F':
						fread(buff,1,2,f);
						fseek(f,((buff[0]<<8)|buff[1])*2,SEEK_CUR);
						break;
					case 'S':
						fread(buff,1,2,f);
						//sprintf(buff,"%d",((buff[0]<<8)|buff[1]));
						//MessageBox(NULL,buff,"Seed",MB_OK);
						break;
				}
				break;
			// Harkonnen
			case 2:
			// Atreides
			case 3:
			// Ordos
			case 4:
			// Fremen
			case 5:
				switch(subcmd)
				{
					// Quota
					case 'Q':
					// Credits
					case 'C':
					// Brain
					case 'B':
					// MaxUnits
					case 'M':
						fread(buff,1,2,f);
						//sprintf(buff,"%d(%X)%c",cmd,ftell(f),subcmd);
						//MessageBox(NULL,buff,"House",MB_OK);
						break;
				}
				break;
			// Starport (subcmd = unit)
			case 6:
				fread(buff,1,2,f);
				break;
			// Teams ( subcmd = team id )
			case 7:
				fseek(f,5*2,SEEK_CUR);
				break;
			// Units ( subcmd = unk )
			case 8:
				fread(buff,1,6*2,f);
				unit.house = (buff[ 0]<<8) | buff[ 1];
				unit.id    = (buff[ 2]<<8) | buff[ 3];
				unit.life  = (buff[ 4]<<8) | buff[ 5];
				unit.pos   = (buff[ 6]<<8) | buff[ 7];
				unit.angle = (buff[ 8]<<8) | buff[ 9];
				unit.ai    = (buff[10]<<8) | buff[12];
				Units.push_back(unit);
				break;
			// Structures ( subcmd = unk )
			case 9:
				if (subcmd == 'G')
				{
					fread(buff,1,3*2,f);
					structure.pos  = (buff[ 0]<<8) | buff[ 1];
					structure.house = (buff[ 2]<<8) | buff[ 3];
					structure.id    = (buff[ 4]<<8) | buff[ 5];
				}
				else
				{
					fread(buff,1,5*2,f);
					structure.flag  = (buff[ 0]<<8) | buff[ 1];
					structure.house = (buff[ 2]<<8) | buff[ 3];
					structure.id    = (buff[ 4]<<8) | buff[ 5];
					structure.life  = (buff[ 6]<<8) | buff[ 7];
					structure.pos   = (buff[ 8]<<8) | buff[ 9];
				}
				Structures.push_back(structure);
				break;
			// Reinforcements
			case 10:
				fread(buff,1,4*2,f);
				break;
		}
	}
	fclose(f);
	//fclose(log);
	//sprintf((char*)buff,"%d",Units.size());
	//MessageBox(NULL,(char*)buff,"Units.size()",MB_OK);
}

Window::Window()
{
    glWidget = new GLWidget;

    setCentralWidget(glWidget);

    setWindowTitle(tr("DuneGroundEdit"));
    createMenus();

    setMouseTracking(true);
    centralWidget()->setMouseTracking(true);
}

void Window::newFile()
{
}

void Window::open()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Choose Map"));
    if (!fileName.isEmpty())
        LoadMap(fileName.toLocal8Bit().data());
}

void Window::save()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save Map"));
    if (!fileName.isEmpty())
        SaveMap(fileName.toLocal8Bit().data());
}

void Window::print()
{
}

void Window::createMenus()
{
    newAct = new QAction(tr("&New"), this);
    newAct->setShortcuts(QKeySequence::New);
    newAct->setStatusTip(tr("Create a new file"));
    connect(newAct, SIGNAL(triggered()), this, SLOT(newFile()));

    openAct = new QAction(tr("&Open..."), this);
    openAct->setShortcuts(QKeySequence::Open);
    openAct->setStatusTip(tr("Open an existing file"));
    connect(openAct, SIGNAL(triggered()), this, SLOT(open()));

    saveAct = new QAction(tr("&Save"), this);
    saveAct->setShortcuts(QKeySequence::Save);
    saveAct->setStatusTip(tr("Save the document to disk"));
    connect(saveAct, SIGNAL(triggered()), this, SLOT(save()));

    printAct = new QAction(tr("&Print..."), this);
    printAct->setShortcuts(QKeySequence::Print);
    printAct->setStatusTip(tr("Print the document"));
    connect(printAct, SIGNAL(triggered()), this, SLOT(print()));

    exitAct = new QAction(tr("E&xit"), this);
    exitAct->setShortcuts(QKeySequence::Quit);
    exitAct->setStatusTip(tr("Exit the application"));
    connect(exitAct, SIGNAL(triggered()), this, SLOT(close()));

    fileMenu = menuBar()->addMenu(tr("&File"));
	fileMenu->addAction(newAct);
    fileMenu->addAction(openAct);
    fileMenu->addAction(saveAct);
    fileMenu->addAction(printAct);
    fileMenu->addSeparator();
    fileMenu->addAction(exitAct);
}

void Window::keyPressEvent(QKeyEvent *event)
{
    switch (event->key())
    {
		case Qt::Key_Escape:
			if (state == 0)
				exit(0);
			else
				ChangeState(0);
			break;
		case Qt::Key_O:
            {
                Window::open();
            }
			break;
		case  Qt::Key_M:
            {
                QString fileName = QFileDialog::getOpenFileName(this, tr("Choose Mission"));
                if (!fileName.isEmpty())
                    LoadMission(fileName.toLocal8Bit().data());
            }
			break;
		case  Qt::Key_S:
            {
                Window::save();
            }
			break;
		case  Qt::Key_G:
            {
                QString fileName = QFileDialog::getOpenFileName(this, tr("Choose Grey Bitmap"));
                if (!fileName.isEmpty())
                {
                    FreeTexture(GroundGrey);
                    GroundGrey = LoadGreyTexture(fileName.toLocal8Bit().data());
                }
            }
			break;
		case  Qt::Key_R:
			FreeTexture(GroundGrey);
			GroundGrey = GenerateGround();
			break;
		case  Qt::Key_E:
			if (state == 2)
				ChangeState(1);
			else
				ChangeState(state^1);
			break;
		case  Qt::Key_F:
			if (state == 2)
				ChangeState(0);
			else
				ChangeState(2);
			break;
		case  Qt::Key_I:
			drawinverse = !drawinverse;
			break;
		case  Qt::Key_0:
			drawtype = 0;
			break;
		case  Qt::Key_1:
			drawtype = 1;
			break;
		case  Qt::Key_2:
			drawtype = 2;
			break;
		case  Qt::Key_3:
			drawtype = 3;
			break;
		case  Qt::Key_4:
			drawtype = 4;
			break;
		case  Qt::Key_Q:
			--drawsize;
			if (drawsize == 0)
				drawsize = 1;
			break;
		case  Qt::Key_W:
			++drawsize;
			break;
		case  Qt::Key_C:
            {
                QMessageBox msgBox;
                msgBox.setText("DuneGroundEditor");
                msgBox.setInformativeText("Do you really want to clear map?");
                msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
                msgBox.setDefaultButton(QMessageBox::No);
                int ret = msgBox.exec();
                if(ret == QMessageBox::Yes)
                    duneGround.Clear();
            }
			break;
		case  Qt::Key_Space:
			showgrey = !showgrey;
			break;
		case  Qt::Key_U:
			showunits = !showunits;
			break;
		default:
			break;
    }
}

void Window::mousePressEvent(QMouseEvent *event)
{
	switch (event->button())
    {
		case Qt::LeftButton:
			mouse.x = event->x();
			mouse.y = event->y();
			if (state == 2)
			{
				duneGround = duneGroundNew;
				ChangeState(0);
			}
            mouse1down = true;
			break;
		case Qt::RightButton:
			mouse.x = event->x();
			mouse.y = event->y();
			mouse2down = true;
			break;
		default:
			break;
	}
}

void Window::mouseReleaseEvent(QMouseEvent *event)
{
	switch (event->button())
    {
		case Qt::LeftButton:
            mouse1down = false;
			break;
		case Qt::RightButton:
			mouse2down = false;
			break;
		default:
			break;
	}
}

void Window::mouseMoveEvent(QMouseEvent* event)
{
	POINT pos = mouse;
	mouse.x = event->pos().x();
	mouse.y = event->pos().y();

	if (mouse2down)
	{
		camera.x -= mouse.x-pos.x;
		camera.y -= mouse.y-pos.y;
	}

	double cx = ((camera.x+mouse.x-(width()/2))/32.0/zoom)*2+0.5;
	double cy = ((camera.y+mouse.y-(height()/2))/32.0/zoom)*2+0.5;
	if (state == 1 && duneGround.tin(cx,cy) && mouse1down/*(GetKeyState(VK_LBUTTON) & 0x80)*/)
	{
		for (int x=0; x<drawsize; ++x)
		for (int y=0; y<drawsize; ++y)
			if (duneGround.tin(cx+x,cx+y))
				duneGround.Draw(cx+x,cy+y,drawtype);
	}
}

void Window::wheelEvent(QWheelEvent *event)
{
	QPoint p = this->mapFromGlobal(QCursor::pos());

	mouse.x = p.x();
	mouse.y = p.y();
	short zDelta = event->delta();
	if (zDelta > 0)
	{
		zoom *= 1.2;
		camera.x *= 1.2;//(camera.x + mouse.x) * (zoom/(zoom-0.1)) - mouse.x;
		camera.y *= 1.2;//(camera.y + mouse.y) * (zoom/(zoom-0.1)) - mouse.y;
	}
	else
	{
		zoom /= 1.2;
		camera.x /= 1.2;
		camera.y /= 1.2;
	}

    event->accept();
}
