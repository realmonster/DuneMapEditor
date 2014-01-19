#ifndef _GLWIDGET_H_
#define _GLWIDGET_H_

#include <QMainWindow>
#include <QMenu>
#include <QGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QTimer>
#include <QSignalMapper>
#include <QToolButton>
#include <QSlider>

class GLWidget : public QGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    GLWidget(QWidget *parent = 0);
    ~GLWidget();

    QSize minimumSizeHint() const;
    QSize sizeHint() const;

protected:
    void initializeGL();
    void paintGL();
    void resizeGL(int width, int height);

private:
    GLuint m_posAttr;
    GLuint m_colAttr;
    GLuint m_matrixUniform;

    QOpenGLShaderProgram *m_program;
    QTimer timer;
};

class Window : public QMainWindow
{
    Q_OBJECT

public:
    Window();
    int getDrawSize();
    int getHouseSelected();
    GLWidget *glWidget;

protected:
	void keyPressEvent(QKeyEvent *event);
    void mousePressEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent* event);
	void wheelEvent(QWheelEvent *event);

private slots:
    void newMap();
    void newMission();
    void openMap();
    void openMission();
    void saveMap();
    void updateRadars();
    void tool(bool checked);
    void house(bool checked);

private:
    void createMenus();
    void createToolbars();
    void uncheckTools();
    void buildStructure(int id);
    void buildUnit(int id);
    void drawGround(int type);
    void selectHouse(int id);

    int houseSelected;

    QMenu *fileMenu;
    QToolBar *structuresMenu;
    QToolBar *unitsMenu;
    QToolBar *groundMenu;
    QToolBar *houseMenu;
    QAction *newMapAct;
    QAction *newMissionAct;
    QAction *openMapAct;
    QAction *openMissionAct;
    QAction *saveMapAct;
    QAction *exitAct;
    QTimer *radarsTimer;
    QSlider *drawSizeSlider;

    QToolButton *structuresButton[50];
    QToolButton *unitsButton[50];
    QToolButton *groundButton[5];
    QToolButton *houseButton[5];
};

#endif // _GLWIDGET_H_
