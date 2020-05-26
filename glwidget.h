#pragma once

#include <QGLWidget>
#include <QKeyEvent>
#include <QMainWindow>
#include <QtOpenGL>
#include <cmath>
#include <vector>
using namespace std;
#include "objects.h"
#include "point.h"

class arcBall;

class GLWidget : public QGLWidget {
  Q_OBJECT

public:
  GLWidget(QWidget *parent = 0);
  Objects *objects;

  void loadTextures();
  void setOrtho();
  void initializeGL();
  void paintGL();
  void resizeGL(int w, int h);
  void keyPressEvent(QKeyEvent *event);
  void wheelEvent(QWheelEvent *event);
  void mousePressEvent(QMouseEvent *event);
  void mouseMoveEvent(QMouseEvent *event);
  void mouseReleaseEvent(QMouseEvent *event);
  void drawAxes();
  void drawGrid(GLfloat *colour, int nGridlines);
  void drawHuman();
  void drawChest();
  void drawHead();
  void drawEyes();
  void drawHat();
  void drawRightUpperArm();
  void drawLeftUpperArm();
  void drawLeftShoulder();
  void drawLeftElbow();
  void drawLeftForeArm();
  void drawLeftHand();
  void drawRightShoulder();
  void drawRightElbow();
  void drawRightForeArm();
  void drawRightHand();
  void drawRightHip();
  void drawRightThigh();
  void drawRightKnee();
  void drawRightShin();
  void drawRightFoot();
  void drawLeftHip();
  void drawLeftThigh();
  void drawLeftKnee();
  void drawLeftShin();
  void drawLeftFoot();
  void computeJointPositions();
  void animate(int time);
  virtual ~GLWidget();
public slots:
  void resetArc();
  void resetPos();
  void resetZoom();

protected:
  arcBall *arc;
  bool resetArcBall;
  bool axes;
  bool grid;
  bool wframe;
  bool animation;

  // Animation parameters
  int time;
  int nframes;

  Point body_pos;
  float body_angle;

  float joint_radius;
  float limb_length;

  // model position
  float posn_x, posn_z, bodyTheta;
  float left_arm, right_arm;
  float left_leg, right_leg;

  bool arc_max;

  GLuint textures[10];

  int nGridlines;

  // the viewing area dimensions
  int viewWidth;
  int viewHeight;

  // the zoom factor
  float zoomFactor = 5.f;

  float ortho = 5.0f;
  float orthoNear = -100.0f;
  float orthoFar = 100.0f;

  bool drag;
  bool pull;
  float pullX = 0.0;
  float pullY = 0.0;
  float moveX = 0.0;
  float moveY = 0.0;

public slots:
  void idle();
  void saveImage();
};
