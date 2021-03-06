/*
    This file is part of DuneMapEditor.
 
    DuneMapEditor is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
 
    DuneMapEditor is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
 
    You should have received a copy of the GNU General Public License
    along with DuneMapEditor.  If not, see <http://www.gnu.org/licenses/>.
*/

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
#include "MissionSettingsWindow.h"
#include "IconsOrder.h"

#ifndef _WIN32
struct POINT
{
    int x, y;
};

#endif

POINT camera = {0,0};
POINT mouse;

double zoom = 1;
bool showgrey = false;
bool showunits = true;
int state = 0;
bool showmask = true;
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
    virtual void mouseMove(Window *sender, QMouseEvent *event) {Q_UNUSED(sender,event);}
    virtual void mousePress(Window *sender, QMouseEvent *event) {}
    virtual void mouseRelease(Window *sender, QMouseEvent *event) {}
    virtual ~MouseTool() {}
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

GLWidget::GLWidget(Window *parent)
    : QGLWidget(desiredFormat(), parent),
    m_program(0)
{
    window = parent;
    connect(&timer, SIGNAL(timeout()), this, SLOT(updateGL()));
    if(format().swapInterval() == -1)
    {
        // V_blank synchronization unavailable
        timer.setInterval(15);
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
    if (img.isNull())
        return 0;

	// allocate buffer
    width = img.width();
    height = img.height();
    data = (unsigned char*)malloc( width * height * 4 );

    QPoint p;
    for (p.rx()=0; p.rx()<width; ++p.rx())
        for (p.ry()=0; p.ry()<height; ++p.ry())
        {
            QColor c = QColor(img.pixel(p));
            int i = p.rx() + p.ry()*width;
            data[i*4] = c.red();
            data[i*4+1] = c.green();
            data[i*4+2] = c.blue();
            data[i*4+3] = c.alpha();
            if (c.red()   >  0xE0
             && c.green() == 0
             && c.blue()  >  0xE0)
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

GLuint LoadGreyTexture( QString filename )
{
	int width, height;
    unsigned char * data;

	// open texture data
    QImage img(filename);
    if (img.isNull())
        return 0;

    // allocate buffer
    width = img.width();
    height = img.height();
    if (width  != 128
     || height != 128)
        return 0;

    data = (unsigned char*)malloc( width * height * 4 );

    QPoint p;
    for (p.rx()=0; p.rx()<width; ++p.rx())
        for (p.ry()=0; p.ry()<height; ++p.ry())
        {
            QColor c = QColor(img.pixel(p));
            int i = p.rx() + p.ry()*width;
            int all = c.red() + c.green() + c.blue();
            all /= 3;
            data[i*4] = all;
            data[i*4+1] = all;
            data[i*4+2] = all;
            data[i*4+3] = 1;

            g[i%width][i/width] = all/255.0;
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

int unitPos(int x, int y)
{
    if (x >= 0 && x <= 0x3F
     && y >= 0 && y <= 0x3F)
        return x + y*0x40;
    return -1;
}

void unitPos(int pos, int *x, int *y)
{
    if (pos >= 0x40*0x40 || pos < 0)
    {
        *x = -1;
        return;
    }
    *x = pos & 0x3F;
    *y = pos >> 6;
}

int unitAtPos(int pos, size_t i=0)
{
    for (; i<Mission.Units.size(); ++i)
        if (Mission.Units[i].pos == pos)
            return i;
    return -1;
}

int unitAt(int x, int y, size_t i=0)
{
    int pos = unitPos(x, y);
    if (pos != -1)
        return unitAtPos(pos, i);
    return -1;
}

int structureAt(int x, int y, size_t i=0)
{
    for (; i<Mission.Structures.size(); ++i)
    {
        DuneMission::Structure &s = Mission.Structures[i];
        DrawInfos &di = StructureDrawInfos[s.id];
        int sx, sy, sw, sh;
        unitPos(s.pos, &sx, &sy);
        sw = di.width/32;
        sh = di.height/32;
        if (sx != -1)
        {
            if (x >= sx && x < sx + sw
             && y >= sy && y < sy + sh)
                return i;
        }
    }
    return -1;
}

int structureAtPos(int pos, size_t i=0)
{
    int x, y;
    unitPos(pos, &x, &y);
    if (x != -1)
        return structureAt(x, y, i);
    return -1;
}

struct Selection
{
    std::vector<int> Structures;
    std::vector<int> Units;

    void clear()
    {
        Structures.clear();
        Units.clear();
    }

    void addStructure(int id)
    {
        for (size_t i=0; i<Structures.size(); ++i)
            if (Structures[i] == id)
                return;
        Structures.push_back(id);
    }

    void removeStructure(int id)
    {
        for (size_t i=0; i<Structures.size(); ++i)
            if (Structures[i] == id)
                Structures.erase(Structures.begin()+i);
    }

    void addUnit(int id)
    {
        for (size_t i=0; i<Units.size(); ++i)
            if (Units[i] == id)
                return;
        Units.push_back(id);
    }

    void removeUnit(int id)
    {
        for (size_t i=0; i<Units.size(); ++i)
            if (Units[i] == id)
                Units.erase(Units.begin()+i);
    }
} CurrentSelection;


void DeleteSelected()
{
    for (size_t i=0; i<CurrentSelection.Structures.size(); ++i)
    {
        int id = CurrentSelection.Structures[i];
        Mission.Structures.erase(Mission.Structures.begin()+id);

        // decrease numbers
        for (size_t j=i+1; j<CurrentSelection.Structures.size(); ++j)
            if (CurrentSelection.Structures[j]>id)
                --CurrentSelection.Structures[j];
    }

    for (size_t i=0; i<CurrentSelection.Units.size(); ++i)
    {
        int id = CurrentSelection.Units[i];
        Mission.Units.erase(Mission.Units.begin()+id);

        // decrease numbers
        for (size_t j=i+1; j<CurrentSelection.Units.size(); ++j)
            if (CurrentSelection.Units[j]>id)
                --CurrentSelection.Units[j];
    }
    CurrentSelection.clear();
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
        for (int y=0; y<duneGround.height; ++y)
		{
            for (int x=0; x<duneGround.width; ++x)
			{
				int id;
				if (state == 2)
                    id = duneGroundNew[x][y];
				else
                    id = duneGround[x][y];
				int idx = id&15;
				int idy = (id>>4);
                glTexCoord2d(tw*(idx+0),tw*(idy+0)); glVertex2d(x,-y);
                glTexCoord2d(tw*(idx+1),tw*(idy+0)); glVertex2d(x+1,-y);
                glTexCoord2d(tw*(idx+1),tw*(idy+1)); glVertex2d(x+1,-(y+1));
                glTexCoord2d(tw*(idx+0),tw*(idy+1)); glVertex2d(x,-(y+1));
			}
		}
	}
	glEnd();

    if (state == 1 && showmask)
	{
		glEnable( GL_BLEND );
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
		glColor4f(1.f,1.f,1.f,0.5f);
		glBegin( GL_QUADS );
        for (int y=0; y<duneGround.theight(); ++y)
		{
            for (int x=0; x<duneGround.twidth(); ++x)
			{
                static int tid[]={0x7F,0x8F,0xBF,0xCF,0x9F};
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
        static struct Houses {double r,g,b;} houses[] = {
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
        for (size_t i = 0; i < Mission.Structures.size(); ++i)
		{
            int x = (Mission.Structures[i].pos&0x3F);
            int y = (Mission.Structures[i].pos/0x40);
            int id = Mission.Structures[i].id;
            Houses &h = houses[Mission.Structures[i].house];
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

        for (size_t i = 0; i < Mission.Units.size(); ++i)
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

        if (radarsAngle&2)
        {
            for (size_t i = 0; i < CurrentSelection.Structures.size(); ++i)
            {
                int x, y;
                int id = CurrentSelection.Structures[i];
                DuneMission::Structure &s = Mission.Structures[id];
                DrawInfos &di = StructureDrawInfos[s.id];
                unitPos(s.pos, &x, &y);
                double tx = 99/512.0;
                double ty = 163/512.0;
                double w = 26/512.0;
                glTexCoord2d(tx  ,ty  ); glVertex2d(x              ,-(y  ));
                glTexCoord2d(tx+w,ty  ); glVertex2d(x+1*di.width/32,-(y  ));
                glTexCoord2d(tx+w,ty+w); glVertex2d(x+1*di.width/32,-(y+1*di.height/32));
                glTexCoord2d(tx  ,ty+w); glVertex2d(x              ,-(y+1*di.height/32));
            }
            for (size_t i = 0; i < CurrentSelection.Units.size(); ++i)
            {
                int x, y;
                int id = CurrentSelection.Units[i];
                unitPos(Mission.Units[id].pos, &x, &y);
                int idx = 6;
                int idy = 4;
                glTexCoord2d(tw*(idx+0),tw*(idy+0)); glVertex2d(x+0.5-0.5,-(y+0.5-0.5));
                glTexCoord2d(tw*(idx+1),tw*(idy+0)); glVertex2d(x+0.5+0.5,-(y+0.5-0.5));
                glTexCoord2d(tw*(idx+1),tw*(idy+1)); glVertex2d(x+0.5+0.5,-(y+0.5+0.5));
                glTexCoord2d(tw*(idx+0),tw*(idy+1)); glVertex2d(x+0.5-0.5,-(y+0.5+0.5));
            }
        }
        double x = (Mission.CursorPos&0x3F)-0.5;
        double y = (Mission.CursorPos/0x40)-0.5;
        int idx = 3;
        int idy = 5;
        glTexCoord2d(tw*(idx+0),tw*(idy+0)); glVertex2d(x+0.5-0.5,-(y+0.5-0.5));
        glTexCoord2d(tw*(idx+1),tw*(idy+0)); glVertex2d(x+0.5+0.5,-(y+0.5-0.5));
        glTexCoord2d(tw*(idx+1),tw*(idy+1)); glVertex2d(x+0.5+0.5,-(y+0.5+0.5));
        glTexCoord2d(tw*(idx+0),tw*(idy+1)); glVertex2d(x+0.5-0.5,-(y+0.5+0.5));
        glEnd();

        // 121 89
        x = (Mission.TacticalPos&0x3F)-121/32.0;
        y = (Mission.TacticalPos/0x40)-89/32.0;
        glDisable( GL_TEXTURE_2D );

        glBegin( GL_LINE_STRIP );
        glVertex2d(x   ,-(y         ));
        glVertex2d(x+10,-(y         ));
        glVertex2d(x+10,-(y+224/32.0));
        glVertex2d(x   ,-(y+224/32.0));
        glVertex2d(x   ,-(y         ));
        glEnd();

        glEnable( GL_TEXTURE_2D );

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

void LoadMap( QString filename )
{
    QFile f(filename);
    if (!f.open(QIODevice::ReadOnly))
        return;
    int size = 1;
    for (; size<256; ++size)
        if (f.size()<= size*size)
            break;
    unsigned char tmp;
    duneGround.Clear();
    if (duneGround.width != size)
        duneGround.resize(size,size);
    for (int y=0; y<duneGround.height; ++y)
        for (int x=0; x<duneGround.width; ++x)
        {
            f.read((char*)&tmp,1);
            duneGround[x][y]=tmp;
            duneGround.SetTileMask(x,y);
        }
    f.close();
}

void SaveMap( QString filename )
{
    QFile f(filename);
    if (!f.open(QIODevice::WriteOnly))
    	return;
    for (int y=0; y<duneGround.height; ++y)
	    for (int x=0; x<duneGround.width; ++x)
             f.write((char*)&duneGround[x][y],1);
    f.close();
}

struct SelectTool : MouseTool
{
    bool move;
    QPointF downPos;
    SelectTool() {move = false;}

    void mousePress(Window *sender, QMouseEvent *event)
    {
        if (event->button() == Qt::LeftButton)
        {
            downPos = worldCursor(sender, event);
            if (!(event->modifiers() & (Qt::ShiftModifier | Qt::ControlModifier | Qt::AltModifier)))
                CurrentSelection.clear();

            int pos = unitPos(downPos.x(),downPos.y());
            if (event->modifiers() & Qt::AltModifier)
            {
                for (int i = structureAtPos(pos); i != -1; i = structureAtPos(pos, i+1))
                    CurrentSelection.removeStructure(i);
                for (int i = unitAtPos(pos); i != -1; i = unitAtPos(pos, i+1))
                    CurrentSelection.removeUnit(i);
            }
            else
            {
                for (int i = structureAtPos(pos); i != -1; i = structureAtPos(pos, i+1))
                    CurrentSelection.addStructure(i);
                for (int i = unitAtPos(pos); i != -1; i = unitAtPos(pos, i+1))
                    CurrentSelection.addUnit(i);
            }
        }
    }

    void mouseMove(Window *sender, QMouseEvent *event)
    {
        if (event->buttons() & Qt::LeftButton)
        {
            QPointF c = worldCursor(sender,event) - downPos;
            int dx = c.x();
            int dy = c.y();
            for (size_t i=0; i<CurrentSelection.Structures.size(); ++i)
            {
                int id = CurrentSelection.Structures[i];
                DuneMission::Structure &s = Mission.Structures[id];
                int x, y;
                unitPos(s.pos,&x,&y);
                s.pos = unitPos(x+dx,y+dy);
            }
            for (size_t i=0; i<CurrentSelection.Units.size(); ++i)
            {
                int id = CurrentSelection.Units[i];
                DuneMission::Unit &u = Mission.Units[id];
                int x, y;
                unitPos(u.pos,&x,&y);
                u.pos = unitPos(x+dx,y+dy);
            }
            downPos.rx() += dx;
            downPos.ry() += dy;
        }
    }
};

struct DrawGround : MouseTool
{
    int id;
    DrawGround(int _id) : id(_id) {ChangeState(1);}
    ~DrawGround(){ChangeState(0);}

    void mousePress(Window *sender, QMouseEvent *event)
    {
        if (event->button() == Qt::LeftButton)
        {
            mouse.x = event->x()-sender->glWidget->x();
            mouse.y = event->y()-sender->glWidget->y();
            if (state == 2)
            {
                duneGround = duneGroundNew;
                ChangeState(0);
            }
        }
    }

    void mouseMove(Window *sender, QMouseEvent *event)
    {
        POINT pos = mouse;
        mouse.x = event->x()-sender->glWidget->x();
        mouse.y = event->y()-sender->glWidget->y();

        if (event->buttons() & Qt::RightButton)
        {
            camera.x -= mouse.x-pos.x;
            camera.y -= mouse.y-pos.y;
        }

        QPointF c = worldCursor(sender);
        double cx = c.rx()*2+0.5;
        double cy = c.ry()*2+0.5;
        if ((event->buttons() & Qt::LeftButton)
         && state == 1
         && duneGround.tin(cx,cy))
        {
            int drawsize = sender->getDrawSize();
            for (int x=0; x<drawsize; ++x)
            for (int y=0; y<drawsize; ++y)
                if (duneGround.tin(cx+x,cy+y))
                {
                    const int xx = cx + x;
                    const int yy = cy + y;
                    duneGround.t(xx, yy) = id;

                    // walk across rhomb
                    const int x1 =  xx / 2;
                    const int y1 =  yy / 2;
                    for (int i1 = -3; i1 <= 3; ++i1)
                    for (int i2 = -3 + abs(i1); abs(i1) + abs(i2) <= 3; ++i2)
                    {
                        const int px = x1 + i2;
                        const int py = y1 + i1;
                        if (duneGround.in(px, py))
                            duneGround.Update(px, py);
                    }
                }
        }
    }
};

struct BuildStructureTool : MouseTool
{
    int id;

    BuildStructureTool(int _id) : id(_id) {}

    void place(Window *sender, QMouseEvent *event)
    {
        QPointF c = worldCursor(sender,event);
        int x = c.x();
        int y = c.y();
        int pos = unitPos(x, y);
        if (pos != -1)
        {
            DrawInfos &di = StructureDrawInfos[id];
            int w = di.width/32;
            int h = di.height/32;
            bool was = false;
            for (int j=0; j<h && !was; ++j)
                for (int i=0; i<w && !was; ++i)
                    if (structureAt(x+i, y+j) != -1
                ||(id > 1 && unitAt(x+i, y+j) != -1))
                            was = true;
            if (!was)
            {
                DuneMission::Structure s;
                s.id = id;
                s.house = sender->getHouseSelected();
                s.flag = 0;
                s.life = sender->getHitPoints();
                s.pos = pos;
                Mission.Structures.push_back(s);
            }
        }
    }

    void mousePress(Window *sender, QMouseEvent *event)
    {
        if (event->button() == Qt::LeftButton)
            place(sender, event);
    }

    void mouseMove(Window *sender, QMouseEvent *event)
    {
        if (event->buttons() & Qt::LeftButton)
            place(sender, event);
    }
};

struct BuildUnitTool : MouseTool
{
    int id;

    BuildUnitTool(int _id) : id(_id) {}

    void place (Window *sender, QMouseEvent *event)
    {
        QPointF c = worldCursor(sender, event);
        int pos = unitPos(c.x(),c.y());
        if (pos != -1
         && unitAtPos(pos) == -1)
        {
            bool was = false;
            for (int i = structureAtPos(pos); i != -1; i = structureAtPos(pos, i+1))
                if (Mission.Structures[i].id > 1) // not Wall
                {
                    was = true;
                    break;
                }
            if (!was)
            {
                DuneMission::Unit s;
                s.id = id;
                s.house = sender->getHouseSelected();
                s.angle = 0;
                s.ai = 0;
                s.life = sender->getHitPoints();
                s.pos = pos;
                Mission.Units.push_back(s);
            }
        }
    }

    void mousePress(Window *sender, QMouseEvent *event)
    {
        if (event->button() == Qt::LeftButton)
            place(sender, event);
    }

    void mouseMove(Window *sender, QMouseEvent *event)
    {
        if (event->buttons() & Qt::LeftButton)
            place(sender, event);
    }
};


Window::Window()
{
    DunemaskInit();
    glWidget = new GLWidget(this);

    setCentralWidget(glWidget);

    setWindowTitle(tr("DuneMapEditor"));
    createMenus();
    createToolbars();

    setMouseTracking(true);
    centralWidget()->setMouseTracking(true);

    radarsTimer = new QTimer(this);
    connect(radarsTimer, SIGNAL(timeout()), this, SLOT(updateRadars()));
    radarsTimer->start(133);

    currentMouseTool = new SelectTool();
    missionSettingsDialog = 0;
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
        Mission.Clear();
    }
}

void Window::openMap()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Choose Map"));
    if (!fileName.isEmpty())
        LoadMap(fileName);
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
        SaveMap(fileName);
}

void Window::saveMission()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save Mission"));
    if (!fileName.isEmpty())
        SaveMission(fileName);
}

void Window::options()
{
    OptionsWindow opts;
    opts.exec();
}

void Window::missionSettings()
{
    if (missionSettingsDialog)
        delete missionSettingsDialog;
    missionSettingsDialog = new MissionSettingsWindow(this);
    missionSettingsDialog->show();
    missionSettingsDialog->raise();
    missionSettingsDialog->activateWindow();
}

void Window::setMapSize()
{
    QDialog dlg;
    dialog = &dlg;
    QVBoxLayout *layout = new QVBoxLayout();
    dlg.setLayout(layout);
    QGridLayout *grid = new QGridLayout();

    grid->addWidget(new QLabel(tr("Width")), 0, 0);
    grid->addWidget(new QLabel(tr("Height")), 0, 1);

    QSpinBox *width = new QSpinBox();
    QSpinBox *height = new QSpinBox();
    width->setMaximum(256);
    height->setMaximum(256);
    width->setValue(duneGround.width);
    height->setValue(duneGround.height);

    grid->addWidget(width, 1, 0);
    grid->addWidget(height, 1, 1);

    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttons,SIGNAL(accepted()),this,SLOT(accept()));
    connect(buttons,SIGNAL(rejected()),this,SLOT(reject()));

    layout->addLayout(grid);
    layout->addWidget(buttons);

    int r = dlg.exec();
    if (r)
    {
        duneGround.resize(width->value(), height->value());
        for (int y=0; y<duneGround.height; ++y)
            for (int x=0; x<duneGround.width; ++x)
                duneGround.SetTileMask(x,y);
    }
}

void Window::accept()
{
    dialog->done(1);
}

void Window::reject()
{
    dialog->done(0);
}

void Window::help()
{
    QMessageBox::about(this, tr("Help"),tr(
"Don't ask stupid questions!!!\nRead FAQ first!"));
}

void Window::about()
{
    QMessageBox::about(this, tr("About"),tr(
"Dune Map Editor by r57shell\n"
"Last Update: 15.07.2020\n"
"For additional info visit: http://elektropage.ru\n"
"Or mail to: r57shell@uralweb.ru\n"));
}

void Window::togglemask()
{
    showmask = showMaskCheckBox->isChecked();
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

    missionSettingsAct = new QAction(tr("&Mission Settings"), this);
    connect(missionSettingsAct, SIGNAL(triggered()), this, SLOT(missionSettings()));

    setMapSizeAct = new QAction(tr("&Set Map Size"), this);
    connect(setMapSizeAct, SIGNAL(triggered()), this, SLOT(setMapSize()));


    optionsMenu = menuBar()->addMenu(tr("&Options"));
    optionsMenu->addAction(optionsAct);
    optionsMenu->addAction(missionSettingsAct);
    optionsMenu->addAction(setMapSizeAct);

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

    mainTools = this->addToolBar(tr("Main"));

    arrowButton = new QToolButton();
    arrowButton->setIcon(QIcon(":/other/arrow.png"));
    arrowButton->setCheckable(true);
    mainTools->addWidget(arrowButton);
    connect(arrowButton, SIGNAL(clicked(bool)), this, SLOT(tool(bool)));

    groundMenu = this->addToolBar(tr("Ground"));

    for (int z=0; z<sizeof(gorder)/sizeof(gorder[0]); ++z)
    {
        int i = gorder[z];
        QString gname = QString(GroundName[z]).toLower();
        groundButton[i] = new QToolButton();
        groundButton[i]->setIcon(QIcon(QString().sprintf(":/ground/%s.png",gname.toLocal8Bit().data())));
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

    showMaskCheckBox = new QCheckBox();
    showMaskCheckBox->setChecked(true);
    connect(showMaskCheckBox, SIGNAL(clicked()), this, SLOT(togglemask()));
    groundMenu->addWidget(showMaskCheckBox);

    houseMenu = this->addToolBar(tr("Houses"));
    for (int i=0; i<5; ++i)
    {
        houseButton[i] = new QToolButton();
        houseButton[i]->setIcon(QIcon(QString().sprintf(":/house/house%d.png",i)));
        houseButton[i]->setCheckable(true);
        houseMenu->addWidget(houseButton[i]);
        connect(houseButton[i], SIGNAL(clicked(bool)), this, SLOT(house(bool)));
    }
    hitPoints = new QSpinBox();
    hitPoints->setMaximum(256);
    hitPoints->setValue(256);
    houseMenu->addWidget(hitPoints);

    structuresMenu = this->addToolBar(tr("Structures"));

    for (int z=0; z<50; ++z)
    {
        structuresButton[z] = NULL;
        unitsButton[z] = NULL;
    }
    for (int z=0; StructuresOrder[z] != -1; ++z)
    {
        int i = StructuresOrder[z];
        structuresButton[i] = new QToolButton();
        structuresButton[i]->setIcon(QIcon(QString().sprintf(":/structures/structure%02d.png",i)));
        structuresButton[i]->setCheckable(true);
        structuresMenu->addWidget(structuresButton[i]);
        connect(structuresButton[i], SIGNAL(clicked(bool)), this, SLOT(tool(bool)));
    }

    unitsMenu = this->addToolBar(tr("Units"));

    for (int z=0; UnitsOrder[z] != -1; ++z)
    {
        int i = UnitsOrder[z];
        unitsButton[i] = new QToolButton();
        unitsButton[i]->setIcon(QIcon(QString().sprintf(":/units/unit%02d.png",i)));
        unitsButton[i]->setCheckable(true);
        unitsMenu->addWidget(unitsButton[i]);
        connect(unitsButton[i], SIGNAL(clicked(bool)), this, SLOT(tool(bool)));
    }
}

void Window::house(bool)
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

void Window::tool(bool)
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
    if (arrowButton == sender)
        select();
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
    arrowButton->setChecked(false);
}

void Window::select()
{
    if (currentMouseTool)
        delete currentMouseTool;
    currentMouseTool = new SelectTool();
    uncheckTools();
    arrowButton->setChecked(true);
}

int Window::getHitPoints()
{
    return hitPoints->value();
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
                    GroundGrey = LoadGreyTexture(fileName);
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
        case  Qt::Key_Delete:
            DeleteSelected();
            break;
		default:
			break;
    }
}

void Window::mousePressEvent(QMouseEvent *event)
{
    if (currentMouseTool)
        currentMouseTool->mousePress(this,event);
    if (event->button() == Qt::RightButton)
    {
        mouse.x = event->x()-glWidget->x();
        mouse.y = event->y()-glWidget->y();
    }
}

void Window::mouseReleaseEvent(QMouseEvent *event)
{
    if (currentMouseTool)
        currentMouseTool->mouseRelease(this,event);
}

void Window::mouseMoveEvent(QMouseEvent* event)
{
    if (currentMouseTool)
        currentMouseTool->mouseMove(this,event);
    POINT pos = mouse;
    mouse.x = event->x()-glWidget->x();
    mouse.y = event->y()-glWidget->y();

    if (event->buttons() & Qt::RightButton)
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

void OptionsWindow::saveOptions()
{
    //settings.animation = animationCheckBox->checkState();
    //settings.coloring = coloringCheckBox->checkState();
    this->close();
}

OptionsWindow::OptionsWindow()
{
    hintsGroupBox = new QGroupBox(tr("Misc"));

    animationCheckBox = new QCheckBox(tr("Animation"));
    animationCheckBox->setChecked(/*settings.animation*/false);
    coloringCheckBox = new QCheckBox(tr("Coloring"));
    coloringCheckBox->setChecked(/*settings.coloring*/false);

    QGridLayout *layout = new QGridLayout;
    layout->addWidget(animationCheckBox, 0, 0);
    layout->addWidget(coloringCheckBox, 1, 0);
    hintsGroupBox->setLayout(layout);

    okButton = new QPushButton(tr("Ok"));
    connect(okButton, SIGNAL(clicked()), this, SLOT(saveOptions()));
    cancelButton = new QPushButton(tr("Cancel"));
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(close()));

    QHBoxLayout *bottomLayout = new QHBoxLayout;
    bottomLayout->addStretch();
    bottomLayout->addWidget(okButton);
    bottomLayout->addWidget(cancelButton);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(hintsGroupBox);
    mainLayout->addLayout(bottomLayout);
    setLayout(mainLayout);

    setWindowTitle(tr("Options"));
}
