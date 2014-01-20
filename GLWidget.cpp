#include <QtGui>
#include <QtWidgets>
#include <QtOpenGL>

#include <math.h>
#include <time.h>

#include "GLWidget.h"
#include "Dunes.h"
#include "DuneStructures.h"
#include "DuneGround.h"
#include "MissionParser.h"

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
//int drawtype = 0;
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

struct MouseTool
{
    virtual void mouseMove(Window *sender, QMouseEvent *event) {}
    virtual void mousePress(Window *sender, QMouseEvent *event) {}
    virtual void mouseRelease(Window *sender, QMouseEvent *event) {}
};

void ChangeState(int _state)
{
	if (state == _state)
		return;
	state = _state;
}

QPointF worldCursor(Window *sender)
{
    QPointF r;
    r.setX((camera.x+mouse.x-(sender->glWidget->width()/2))/32.0/zoom);
    r.setY((camera.y+mouse.y-(sender->glWidget->height()/2))/32.0/zoom);
    return r;
}

QPointF worldCursor(Window *sender, QMouseEvent *event)
{
    QPointF r;
    mouse.x = event->x()-sender->glWidget->x();
    mouse.y = event->y()-sender->glWidget->y();
    r.setX((camera.x+mouse.x-(sender->glWidget->width()/2))/32.0/zoom);
    r.setY((camera.y+mouse.y-(sender->glWidget->height()/2))/32.0/zoom);
    return r;
}

DuneGround duneGround(64,64);
DuneGround duneGroundNew(64,64);

Ground g(2*64+1,2*64+1);
GLuint GroundTiles;
GLuint GroundGrey;
GLuint StructuresTexture;

int radarsAngle;

MouseTool *currentMouseTool = NULL;

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
    /*double cx = (camera.x+mouse.x-(width()/2))/32.0/zoom*2+0.5;
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
    //}*/

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
        double pi = qAcos(-1.0);
        static struct {double r,g,b;} houses[] = {
            {1.0f, 0.7f, 0.7f},
            {0.7f, 0.8f, 1.0f},
            {0.7f, 1.0f, 0.7f},
            {0.8f, 0.8f, 0.8f},
            {0.8f, 0.6f, 0.8f},
        };
            ;
        glColor4f(1.f,1.f,1.f,1.f);
        glEnable( GL_BLEND );
        glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

		glBindTexture( GL_TEXTURE_2D, StructuresTexture );
		glBegin( GL_QUADS );
        for (int i = 0; i < Mission.Structures.size(); ++i)
		{
            int x = (Mission.Structures[i].pos&0x3F);
            int y = (Mission.Structures[i].pos/0x40);
            int id = Mission.Structures[i].id;
            auto &h = houses[Mission.Structures[i].house];
			DrawInfos si = StructureDrawInfos[id];
            //if (si.colored)
                glColor4f(h.r,h.g,h.b,1.f);
            //else
            //    glColor4f(1.f,1.f,1.f,1.f);
            glTexCoord2d((si.x+       0)/512.0,(si.y+        0)/512.0); glVertex2d(x,            -y);
            glTexCoord2d((si.x+si.width)/512.0,(si.y+        0)/512.0); glVertex2d(x+si.width/32,-y);
            glTexCoord2d((si.x+si.width)/512.0,(si.y+si.height)/512.0); glVertex2d(x+si.width/32,-(y+si.height/32));
            glTexCoord2d((si.x+       0)/512.0,(si.y+si.height)/512.0); glVertex2d(x,            -(y+si.height/32));

            if (!si.colored)
            {
                int idx = 6+Mission.Structures[i].house;
                int idy = 8;
                double a = (radarsAngle)*pi/180;
                y += (si.height/32)-1;
                glTexCoord2d(tw/2*(idx+0),tw*(idy+0)/2); glVertex2d(x+0.25-0.25*cos(a)+0.25*sin(a),-(y+0.75-0.25*sin(a)-0.25*cos(a)));
                glTexCoord2d(tw/2*(idx+1),tw*(idy+0)/2); glVertex2d(x+0.25+0.25*cos(a)+0.25*sin(a),-(y+0.75+0.25*sin(a)-0.25*cos(a)));
                glTexCoord2d(tw/2*(idx+1),tw*(idy+1)/2); glVertex2d(x+0.25+0.25*cos(a)-0.25*sin(a),-(y+0.75+0.25*sin(a)+0.25*cos(a)));
                glTexCoord2d(tw/2*(idx+0),tw*(idy+1)/2); glVertex2d(x+0.25-0.25*cos(a)-0.25*sin(a),-(y+0.75-0.25*sin(a)+0.25*cos(a)));
            }
		}
		glEnd();

        glColor4f(1.f,1.f,1.f,1.f);

        glBindTexture( GL_TEXTURE_2D, StructuresTexture );
		glBegin( GL_QUADS );

        for (int i = 0; i < Mission.Units.size(); ++i)
		{
            int x = (Mission.Units[i].pos&0x3F);
            int y = (Mission.Units[i].pos/0x40);
            int id = Mission.Units[i].id;
            if (id == 0x19)
                id = 0x12;
			int idx = id&15;
            int idy = (id>>4)+6+Mission.Units[i].house*2;
            double a = (Mission.Units[i].angle/32)*pi/4;
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

struct DrawGround : MouseTool
{
    int id;
    DrawGround(int _id) : id(_id) {}

    void mousePress(Window *sender, QMouseEvent *event)
    {
        switch (event->button())
        {
            case Qt::LeftButton:
                mouse.x = event->x()-sender->glWidget->x();
                mouse.y = event->y()-sender->glWidget->y();
                if (state == 2)
                {
                    duneGround = duneGroundNew;
                    ChangeState(0);
                }
                mouse1down = true;
                break;
        }
    }
    void mouseRelease(Window *sender, QMouseEvent *event)
    {
        switch (event->button())
        {
            case Qt::LeftButton:
                mouse1down = false;
                break;
        }
    }

    void mouseMove(Window *sender, QMouseEvent *event)
    {
        POINT pos = mouse;
        mouse.x = event->x()-sender->glWidget->x();
        mouse.y = event->y()-sender->glWidget->y();

        if (mouse2down)
        {
            camera.x -= mouse.x-pos.x;
            camera.y -= mouse.y-pos.y;
        }

        QPointF c = worldCursor(sender);
        double cx = c.rx()*2+0.5;
        double cy = c.ry()*2+0.5;
        if (state == 1 && duneGround.tin(cx,cy) && mouse1down/*(GetKeyState(VK_LBUTTON) & 0x80)*/)
        {
            int drawsize = sender->getDrawSize();
            for (int x=0; x<drawsize; ++x)
            for (int y=0; y<drawsize; ++y)
                if (duneGround.tin(cx+x,cx+y))
                    duneGround.Draw(cx+x,cy+y,id);
        }
    }
};

struct BuildStructureTool : MouseTool
{
    int id;

    BuildStructureTool(int _id) : id(_id) {}
    void mousePress(Window *sender, QMouseEvent *event)
    {
        switch (event->button())
        {
            case Qt::LeftButton:
            {
                QPointF c = worldCursor(sender,event);
                int x = c.x();
                int y = c.y();
                if (x >= 0 && x <= 0x3F
                 && y >= 0 && y <= 0x3F)
                {
                    DuneMission::Structure s;
                    s.id = id;
                    s.house = sender->getHouseSelected();
                    s.flag = 0;
                    s.life = 0x100;
                    s.pos = x + y * 0x40;
                    Mission.Structures.push_back(s);
                }
            }
                break;
        }
    }
};

struct BuildUnitTool : MouseTool
{
    int id;

    BuildUnitTool(int _id) : id(_id) {}
    void mousePress(Window *sender, QMouseEvent *event)
    {
        switch (event->button())
        {
            case Qt::LeftButton:
            {
                QPointF c = worldCursor(sender, event);
                int x = c.x();
                int y = c.y();
                if (x >= 0 && x <= 0x3F
                 && y >= 0 && y <= 0x3F)
                {
                    DuneMission::Unit s;
                    s.id = id;
                    s.house = sender->getHouseSelected();
                    s.angle = 0;
                    s.ai = 0;
                    s.life = 0x100;
                    s.pos = x + y * 0x40;
                    Mission.Units.push_back(s);
                }
            }
                break;
        }
    }
};


Window::Window()
{
    DunemaskInit();
    glWidget = new GLWidget;

    setCentralWidget(glWidget);

    setWindowTitle(tr("DuneMapEditor"));
    createMenus();
    createToolbars();

    setMouseTracking(true);
    centralWidget()->setMouseTracking(true);

    radarsTimer = new QTimer(this);
    connect(radarsTimer, SIGNAL(timeout()), this, SLOT(updateRadars()));
    radarsTimer->start(133);

    currentMouseTool = new DrawGround(0);
    selectHouse(0);
}

void Window::newMap()
{
    QMessageBox msgBox;
    msgBox.setText("DuneMapEditor");
    msgBox.setInformativeText(tr("Do you really want to clear map?"));
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::No);
    int ret = msgBox.exec();
    if(ret == QMessageBox::Yes)
        duneGround.Clear();
}

void Window::newMission()
{
    QMessageBox msgBox;
    msgBox.setText("DuneMapEditor");
    msgBox.setInformativeText(tr("Do you really want to clear mission?"));
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::No);
    int ret = msgBox.exec();
    if(ret == QMessageBox::Yes)
    {
        Mission.Units.clear();
        Mission.Structures.clear();
    }
}

void Window::openMap()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Choose Map"));
    if (!fileName.isEmpty())
        LoadMap(fileName.toLocal8Bit().data());
}

void Window::openMission()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Choose Mission"));
    if (!fileName.isEmpty())
        LoadMission(fileName);
}

void Window::saveMap()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save Map"));
    if (!fileName.isEmpty())
        SaveMap(fileName.toLocal8Bit().data());
}

void Window::saveMission()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save Mission"));
    if (!fileName.isEmpty())
        SaveMission(fileName);
}

void Window::options()
{
}

void Window::help()
{
}

void Window::about()
{
    QMessageBox::about(this, tr("About"), tr("Здесь должен быть пафосный текст"));
}

void Window::createMenus()
{
    newMapAct = new QAction(tr("&New Map"), this);
    newMapAct->setShortcuts(QKeySequence::New);
    newMapAct->setStatusTip(tr("Create a new map"));
    connect(newMapAct, SIGNAL(triggered()), this, SLOT(newMap()));

    newMissionAct = new QAction(tr("New Mission"), this);
    newMissionAct->setStatusTip(tr("Create a new map"));
    connect(newMissionAct, SIGNAL(triggered()), this, SLOT(newMission()));

    openMapAct = new QAction(tr("&Open Map"), this);
    openMapAct->setShortcuts(QKeySequence::Open);
    openMapAct->setStatusTip(tr("Open an existing map"));
    connect(openMapAct, SIGNAL(triggered()), this, SLOT(openMap()));

    openMissionAct = new QAction(tr("Open Mission"), this);
    openMissionAct->setStatusTip(tr("Open an existing mission"));
    connect(openMissionAct, SIGNAL(triggered()), this, SLOT(openMission()));

    saveMapAct = new QAction(tr("&Save Map"), this);
    saveMapAct->setShortcuts(QKeySequence::Save);
    saveMapAct->setStatusTip(tr("Save the map to disk"));
    connect(saveMapAct, SIGNAL(triggered()), this, SLOT(saveMap()));

    saveMissionAct = new QAction(tr("Save Mission"), this);
    saveMissionAct->setStatusTip(tr("Save the mission to disk"));
    connect(saveMissionAct, SIGNAL(triggered()), this, SLOT(saveMission()));

    exitAct = new QAction(tr("E&xit"), this);
    exitAct->setShortcuts(QKeySequence::Quit);
    exitAct->setStatusTip(tr("Exit the application"));
    connect(exitAct, SIGNAL(triggered()), this, SLOT(close()));

    fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(newMapAct);
    fileMenu->addAction(newMissionAct);
    fileMenu->addAction(openMapAct);
    fileMenu->addAction(openMissionAct);
    fileMenu->addAction(saveMapAct);
    fileMenu->addAction(saveMissionAct);
    fileMenu->addSeparator();
    fileMenu->addAction(exitAct);

    optionsAct = new QAction(tr("&Options"), this);
    optionsAct->setStatusTip(tr("Options"));
    connect(optionsAct, SIGNAL(triggered()), this, SLOT(options()));

    optionsMenu = menuBar()->addMenu(tr("&Options"));
    optionsMenu->addAction(optionsAct);

    helpAct = new QAction(tr("&Help"), this);
    helpAct->setStatusTip(tr("Help"));
    connect(helpAct, SIGNAL(triggered()), this, SLOT(help()));

    aboutAct = new QAction(tr("&About"), this);
    aboutAct->setStatusTip(tr("Help"));
    connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));

    helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(helpAct);
    helpMenu->addAction(aboutAct);
}

void Window::createToolbars()
{
    int gorder[] = {
        DuneGround::Dust,
        DuneGround::Ground,
        DuneGround::SpiceLow,
        DuneGround::SpiceHigh,
        DuneGround::Dune,
    };

    char* gname[] = {
        "dust",
        "ground",
        "spicelow",
        "spicehigh",
        "dune",
    };

    groundMenu = this->addToolBar(tr("Ground"));

    for (int z=0; z<sizeof(gorder)/sizeof(gorder[0]); ++z)
    {
        int i = gorder[z];
        groundButton[i] = new QToolButton();
        groundButton[i]->setIcon(QIcon(QString().sprintf(":/ground/%s.png",gname[z])));
        groundButton[i]->setCheckable(true);
        groundMenu->addWidget(groundButton[i]);
        connect(groundButton[i], SIGNAL(clicked(bool)), this, SLOT(tool(bool)));
    }

    drawSizeSlider = new QSlider();
    drawSizeSlider->setRange(1,10);
    drawSizeSlider->setTickInterval(1);
    drawSizeSlider->setOrientation(Qt::Horizontal);
    drawSizeSlider->setMinimumWidth(70);
    drawSizeSlider->setMaximumWidth(100);
    groundMenu->addWidget(drawSizeSlider);

    houseMenu = this->addToolBar(tr("Houses"));
    for (int i=0; i<5; ++i)
    {
        houseButton[i] = new QToolButton();
        houseButton[i]->setIcon(QIcon(QString().sprintf(":/house/house%d.png",i)));
        houseButton[i]->setCheckable(true);
        houseMenu->addWidget(houseButton[i]);
        connect(houseButton[i], SIGNAL(clicked(bool)), this, SLOT(house(bool)));
    }

    int order[] = {
         0, // platform x1
         1, // platform x4
         8, // CY
         9, // Windtrap
        12, // Refinery
        7,  // Barracks
        10, // Barracks
        18, // Radar
        17, // Spice Silo
         3, // Venchile
         4, // Venchile
        14, // Wall
        15, // Turret
        16, // R-Turret
        13, // Repair
         5, // Hi-Tech
        11, // Starport
         2, // Palace
        };

    structuresMenu = this->addToolBar(tr("Structures"));

    for (int z=0; z<50; ++z)
    {
        structuresButton[z] = NULL;
        unitsButton[z] = NULL;
    }
    for (int z=0; z<sizeof(order)/sizeof(order[0]); ++z)
    {
        int i = order[z];
        structuresButton[i] = new QToolButton();
        structuresButton[i]->setIcon(QIcon(QString().sprintf(":/structures/structure%02d.png",i)));
        structuresButton[i]->setCheckable(true);
        structuresMenu->addWidget(structuresButton[i]);
        connect(structuresButton[i], SIGNAL(clicked(bool)), this, SLOT(tool(bool)));
    }

    int uorder[] = {
         4, // Solder
         2, // Infantry
         5, // Trooper
         3, // Troopers
        13, // Trike
        14, // Raider Trike
        15, // Quad
        16, // Harvester
        17, // MCV
         9, // Tank
        10, // Siege Tank
         7, // Launcher
         8, // Deviator
        12, // Sonic
        11, // Devastator
         6, // ?
         0, // Carryall
         1, // Thopter
        25, // Sandworm
        };

    unitsMenu = this->addToolBar(tr("Units"));

    for (int z=0; z<sizeof(uorder)/sizeof(uorder[0]); ++z)
    {
        int i = uorder[z];
        unitsButton[i] = new QToolButton();
        unitsButton[i]->setIcon(QIcon(QString().sprintf(":/units/unit%02d.png",i)));
        unitsButton[i]->setCheckable(true);
        unitsMenu->addWidget(unitsButton[i]);
        connect(unitsButton[i], SIGNAL(clicked(bool)), this, SLOT(tool(bool)));
    }
}

void Window::house(bool checked)
{
    QObject *sender = QObject::sender();
    for (int i=0; i<5; ++i)
    {
        if (houseButton[i] == sender)
            selectHouse(i);
    }
}

void Window::selectHouse(int id)
{
    houseSelected = id;
    for (int i=0; i<5; ++i)
        houseButton[i]->setChecked(id == i);
}

int Window::getHouseSelected()
{
    return houseSelected;
}

void Window::tool(bool checked)
{
    QObject *sender = QObject::sender();
    for (int i=0; i<50; ++i)
    {
        if (structuresButton[i] == sender)
            buildStructure(i);
        if (unitsButton[i] == sender)
            buildUnit(i);
    }
    for (int i=0; i<5; ++i)
        if (groundButton[i] == sender)
            drawGround(i);
}

void Window::uncheckTools()
{
    for (int i=0; i<50; ++i)
    {
        if (structuresButton[i])
            structuresButton[i]->setChecked(false);
        if (unitsButton[i])
            unitsButton[i]->setChecked(false);
    }
    for (int i=0; i<5; ++i)
        if (groundButton[i])
            groundButton[i]->setChecked(false);
}

void Window::buildStructure(int id)
{
    if (currentMouseTool)
        delete currentMouseTool;
    currentMouseTool = new BuildStructureTool(id);
    uncheckTools();
    structuresButton[id]->setChecked(true);
}

void Window::buildUnit(int id)
{
    if (currentMouseTool)
        delete currentMouseTool;
    currentMouseTool = new BuildUnitTool(id);
    uncheckTools();
    unitsButton[id]->setChecked(true);
}

void Window::drawGround(int type)
{
    if (currentMouseTool)
        delete currentMouseTool;
    uncheckTools();
    currentMouseTool = new DrawGround(type);
    groundButton[type]->setChecked(true);
}

int Window::getDrawSize()
{
    return drawSizeSlider->value();
}

void Window::updateRadars()
{
    radarsAngle += 45;
    if (radarsAngle == 360)
        radarsAngle = 0;
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
            Window::openMap();
			break;
		case  Qt::Key_M:
            Window::openMission();
			break;
		case  Qt::Key_S:
            Window::saveMap();
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
        case  Qt::Key_1:
            drawGround(0);
			break;
        case  Qt::Key_2:
            drawGround(1);
			break;
        case  Qt::Key_3:
            drawGround(2);
			break;
        case  Qt::Key_4:
            drawGround(3);
			break;
        case  Qt::Key_5:
            drawGround(4);
			break;
		case  Qt::Key_Q:
            drawSizeSlider->setValue(drawSizeSlider->value()-1);
			break;
		case  Qt::Key_W:
            drawSizeSlider->setValue(drawSizeSlider->value()+1);
			break;
		case  Qt::Key_C:
            Window::newMap();
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
    if (currentMouseTool)
        currentMouseTool->mousePress(this,event);
    switch(event->button())
    {
        case Qt::RightButton:
            mouse.x = event->x()-glWidget->x();
            mouse.y = event->y()-glWidget->y();
            mouse2down = true;
            break;
    }
}

void Window::mouseReleaseEvent(QMouseEvent *event)
{
    if (currentMouseTool)
        currentMouseTool->mouseRelease(this,event);
    switch (event->button())
    {
        case Qt::RightButton:
            mouse2down = false;
            break;
        default:
            break;
    }
}

void Window::mouseMoveEvent(QMouseEvent* event)
{
    if (currentMouseTool)
        currentMouseTool->mouseMove(this,event);
    POINT pos = mouse;
    mouse.x = event->x()-glWidget->x();
    mouse.y = event->y()-glWidget->y();
    //this->statusBar()->showMessage(QString().sprintf("(%d,%d)",mouse.x,mouse.y));

    if (mouse2down)
    {
        camera.x -= mouse.x-pos.x;
        camera.y -= mouse.y-pos.y;
    }
}

void Window::wheelEvent(QWheelEvent *event)
{
    QPoint p = this->glWidget->mapFromGlobal(QCursor::pos());

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
