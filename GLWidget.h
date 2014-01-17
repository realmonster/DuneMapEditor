#ifndef _GLWIDGET_H_
#define _GLWIDGET_H_

#include <QMainWindow>
#include <QMenu>
#include <QGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QTimer>

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

protected:
	void keyPressEvent(QKeyEvent *event);
    void mousePressEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent* event);
	void wheelEvent(QWheelEvent *event);

private slots:
    void newFile();
    void open();
    void save();
    void print();
	
private:
    void createMenus();
    GLWidget *glWidget;
    QMenu *fileMenu;
	QAction *newAct;
    QAction *openAct;
    QAction *saveAct;
    QAction *printAct;
    QAction *exitAct;
};

#endif // _GLWIDGET_H_
