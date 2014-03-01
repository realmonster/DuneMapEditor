#ifndef _GLWIDGET_H_
#define _GLWIDGET_H_

#include <QMainWindow>
#include <QDialog>
#include <QMenu>
#include <QGLWidget>
#include <QCheckBox>
#include <QGroupBox>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QTimer>
#include <QSignalMapper>
#include <QToolButton>
#include <QSlider>
#include <QSpinBox>

class MissionSettingsWindow;
class Window;

class GLWidget : public QGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    GLWidget(Window *parent);
    ~GLWidget();

    QSize minimumSizeHint() const;
    QSize sizeHint() const;

protected:
    void initializeGL();
    void paintGL();
    void resizeGL(int width, int height);

private:
    Window *window;
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
    int getHitPoints();
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
    void saveMission();
    void updateRadars();
    void tool(bool checked);
    void house(bool checked);
    void options();
    void missionSettings();
    void setMapSize();
    void help();
    void about();
    void accept();
    void reject();

private:
    void createMenus();
    void createToolbars();
    void uncheckTools();
    void buildStructure(int id);
    void buildUnit(int id);
    void drawGround(int type);
    void selectHouse(int id);
    void select();

    int houseSelected;

    QMenu *fileMenu;
    QMenu *optionsMenu;
    QMenu *helpMenu;

    QAction *newMapAct;
    QAction *newMissionAct;
    QAction *openMapAct;
    QAction *openMissionAct;
    QAction *saveMapAct;
    QAction *saveMissionAct;
    QAction *exitAct;
    QAction *optionsAct;
    QAction *missionSettingsAct;
    QAction *setMapSizeAct;
    QAction *helpAct;
    QAction *aboutAct;

    QTimer *radarsTimer;

    QToolBar *mainTools;
    QToolBar *structuresMenu;
    QToolBar *unitsMenu;
    QToolBar *groundMenu;
    QToolBar *houseMenu;

    QSlider *drawSizeSlider;
    QSpinBox *hitPoints;

    QToolButton *arrowButton;
    QToolButton *structuresButton[50];
    QToolButton *unitsButton[50];
    QToolButton *groundButton[5];
    QToolButton *houseButton[5];

    MissionSettingsWindow *missionSettingsDialog;
    QDialog *dialog;
};

class OptionsWindow : public QDialog
{
    Q_OBJECT

public:
    OptionsWindow();

private slots:
    void saveOptions();

private:
    QGroupBox *hintsGroupBox;
    QPushButton *okButton;
    QPushButton *cancelButton;

    QCheckBox *animationCheckBox;
    QCheckBox *coloringCheckBox;
};

#endif // _GLWIDGET_H_
