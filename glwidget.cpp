#include "glwidget.h"
#include "arcBall.h"
#include "point.h"
#include "vector.h"
#include <QDebug>
//#include <cmath>

static GLfloat light_position[] = {0.0, 100.0, 100.0, 0.0};

// basic colours
static GLfloat black[] = {0.0, 0.0, 0.0, 1.0};
static GLfloat white[] = {1.0, 1.0, 1.0, 1.0};
static GLfloat grey[] = {0.5, 0.5, 0.5, 1.0};

// primary colours
static GLfloat red[] = {1.0, 0.0, 0.0, 1.0};
static GLfloat green[] = {0.0, 1.0, 0.0, 1.0};
static GLfloat blue[] = {0.0, 0.0, 1.0, 1.0};

// secondary colours
static GLfloat yellow[] = {1.0, 1.0, 0.0, 1.0};
static GLfloat magenta[] = {1.0, 0.0, 1.0, 1.0};
static GLfloat cyan[] = {0.0, 1.0, 1.0, 1.0};

// other colours
static GLfloat orange[] = {1.0, 0.5, 0.0, 1.0};
static GLfloat brown[] = {0.5, 0.25, 0.0, 1.0};
static GLfloat dkgreen[] = {0.0, 0.5, 0.0, 1.0};
static GLfloat pink[] = {1.0, 0.6f, 0.6f, 1.0};

GLWidget::GLWidget(QWidget *parent) : QGLWidget(parent) {
  setFocusPolicy(Qt::StrongFocus);
  resetArcBall = false;
  axes = true;
  grid = true;
  wframe = false;
  animation = false;
  nGridlines = 3;
  arc = new arcBall();
  objects = new Objects(this);
  viewWidth = this->width();
  viewHeight = this->height();

  joint_radius = 0.1f;
  limb_length = 0.6f;

  time = 0;
  nframes = 600;

  body_pos = Point(2.0f, 1.7f, 0.0f);
  body_angle = 0.0f;

  left_arm = 0.0f; right_arm = 0.0f;
  left_leg = 0.0f; right_leg = 0.0f;

  arc_max = false;

  this->animate(time);

  QTimer *timer = new QTimer(this);
  timer->start(0);
  connect(timer, SIGNAL(timeout()), this, SLOT(idle()));
}

// TO ADD NEW TEXTURES:
// 1. Add jpg to startup files folder
// 2. Add a QFile::copy to mainwindow.cpp to copy the file to the desktop
// 3. Add the filename to the resources.qrc file in a plain text editor (e.g. notepad)
// 4. Rebuild
void GLWidget::loadTextures() {
  glMatrixMode(GL_TEXTURE);
  glLoadIdentity();
  // load a texture
  glGenTextures(10, textures);
  if (textures[0] == 0)
    qDebug() << "invalid texture names";

  QImage jupiter;
  jupiter.load(QDir::homePath() + "/Desktop/suntexture.png");
  QImage glFormattedImage = QGLWidget::convertToGLFormat(jupiter);
  if (!glFormattedImage.isNull()) {
    qDebug() << "Texture read correctly...";
    // Set the texture as texture 0 for later access
    glBindTexture(GL_TEXTURE_2D, textures[0]);
    // tell OpenGL which texture you want to use
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, glFormattedImage.width(),
                 glFormattedImage.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 glFormattedImage.bits());
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
  } else {
    qDebug() << "Image 1 would not load";
  }

  QImage night_sky;
  night_sky.load(QDir::homePath() + "/Desktop/rickandmorty.jpg");
  glFormattedImage = QGLWidget::convertToGLFormat(night_sky);
  if (!glFormattedImage.isNull()) {
    qDebug() << "Texture read correctly...";
    // Set the texture as texture 1 for later access
    glBindTexture(GL_TEXTURE_2D, textures[1]);
    // tell OpenGL which texture you want to use
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, glFormattedImage.width(),
                 glFormattedImage.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 glFormattedImage.bits());
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
  } else {
    qDebug() << "Image 2 would not load";
  }

  glMatrixMode(GL_MODELVIEW);
}

void GLWidget::setOrtho() {
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  double aspectRatio = static_cast<double>(viewWidth) / static_cast<double>(viewHeight);
  if (aspectRatio > 1.0) {
    glOrtho(static_cast<GLdouble>(-ortho), static_cast<GLdouble>(ortho),
            static_cast<GLdouble>(-ortho) / aspectRatio, static_cast<GLdouble>(ortho) / aspectRatio, -100, 100);
  } else {
    glOrtho(static_cast<GLdouble>(-ortho) * aspectRatio, static_cast<GLdouble>(ortho) * aspectRatio, static_cast<GLdouble>(-ortho),
            static_cast<GLdouble>(ortho), -100, 100);
  }
  glMatrixMode(GL_MODELVIEW);
}

void GLWidget::initializeGL() {
  glClearColor(0.0, 0.0, 0.0, 0.0);
  setOrtho();
  glShadeModel(GL_SMOOTH); // use flat lighting
  glLightfv(GL_LIGHT0, GL_POSITION, light_position); // specify the position of the light
  glEnable(GL_LIGHT0);       // switch light #0 on
  glEnable(GL_LIGHTING);     // switch lighting on
  glEnable(GL_DEPTH_TEST);   // make sure depth buffer is switched on
  glEnable(GL_NORMALIZE);    // normalize normal vectors for safety
  loadTextures();
}

void GLWidget::paintGL() {
  // clear all pixels
  glClearColor(0.0, 0.0, 0.0, 0.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  // draw a quad to display a night sky
  // Not updated with pullX and pullY so that it is not rotated/translated by the user
  glPushMatrix();
  glScalef(12,10,10);
  glTranslatef(0,0,-5);
  glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, white);
  glBindTexture(GL_TEXTURE_2D, textures[1]);
  glEnable(GL_TEXTURE_2D);
  objects->drawQuad();
  glDisable(GL_TEXTURE_2D);
  glPopMatrix();

  GLfloat mNow[16];
  glTranslatef(pullX, pullY, 0.0);
  glGetFloatv(GL_MODELVIEW_MATRIX, mNow);
  arc->getMatrix(mNow);
  glLoadMatrixf(mNow);

  if (axes) {
    drawAxes();
  }
  if (grid) {
    drawGrid(magenta, nGridlines);
  }

  // Draw a sphere to display jupiter
  glPushMatrix();
  glTranslatef(-8.0f, 5.0f, 0.0f);
  glRotatef(-90, 1.0, 0.0, 0.0);
  glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, white);
  glBindTexture(GL_TEXTURE_2D, textures[0]);
  glEnable(GL_TEXTURE_2D);
  objects->drawSphere(3.0f, 64, 64);
  //glTranslatef(10.0f, -5.0f, 0.0f);
  glDisable(GL_TEXTURE_2D);
  glPopMatrix();

  if (wframe) {             // switch for wireframe, this is a hack
    glDisable(GL_LIGHTING); // disable lighting
    glColor4fv(red);        // set a consistant colour for the model
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // switch to line mode
    glPushMatrix();
    drawHuman(); // draw the model
    glPopMatrix();
    glPolygonMode(GL_FRONT_AND_BACK,
                  GL_FILL); // fill triangles for the rest of rendering
    glEnable(GL_LIGHTING);  // enable lighting for the rest of the rendering
  } else {
    // now do the drawing
    glPushMatrix();
    drawHuman();
    glPopMatrix();
  }
}

void GLWidget::resizeGL(int w, int h) {
  viewWidth = w;
  viewHeight = h;
  glViewport(0, 0, viewWidth, viewHeight);
  setOrtho();
  this->updateGL();
}

void GLWidget::keyPressEvent(QKeyEvent *event) {
  switch (event->key()) {

  case Qt::Key_1:
    axes = !axes;
    this->updateGL();
    break;
  case Qt::Key_2:
    grid = !grid;
    this->updateGL();
    break;
  case Qt::Key_3:
    wframe = !wframe;
    this->updateGL();
    break;
  case Qt::Key_4:
    animation = !animation;
    this->updateGL();
    break;
  case '[':
    if (nGridlines > 1)
      nGridlines--;
    this->updateGL();
    break;
  case ']':
    nGridlines++;
    this->updateGL();
    break;
  case Qt::Key_Q:
    qApp->exit();
    break;
  default:
    event->ignore();
    break;
  }
}

void GLWidget::wheelEvent(QWheelEvent *event) {
  float numDegrees = (static_cast<float>(event->delta()) / 8.0f);
  float numSteps = numDegrees / (180.0f * (1.0f / ortho));
  ortho -= numSteps;
  if (ortho <= 0.0f) {
    ortho = 0.0001f;
  }
  viewWidth = this->width();
  viewHeight = this->height();
  setOrtho();
  this->updateGL();
}

void GLWidget::mousePressEvent(QMouseEvent *event) {
  if (event->button() == Qt::LeftButton) {
    arc->startBall(event->x(), event->y(), this->width(), this->height());
    drag = true;
  }
  if (event->button() == Qt::RightButton) {
    moveX = static_cast<float>(event->x());
    moveY = static_cast<float>(event->y());
    pull = true;
  }
}

void GLWidget::mouseMoveEvent(QMouseEvent *event) {
  if (event->buttons() == Qt::LeftButton) {
    if (drag == true) {
      arc->updateBall(event->x(), event->y(), this->width(), this->height());
      this->updateGL();
    }
  }
  if (pull == true) {
    viewHeight = this->height();
    viewWidth = this->width();
    float aspectRatio = static_cast<float>(viewWidth) / static_cast<float>(viewHeight);
    if (aspectRatio < 1.0f) {
      pullX += (static_cast<float>(event->x()) - moveX) /
               (static_cast<float>(this->width()) /
               ((ortho * 2.0f) * aspectRatio));
      pullY -= (static_cast<float>(event->y()) - moveY) /
               (static_cast<float>(this->height()) / (ortho * 2.0f));
    } else {
      pullX += (static_cast<float>(event->x()) - moveX) /
               (static_cast<float>(this->width()) / (ortho * 2.0f));
      pullY -= (static_cast<float>(event->y()) - moveY) /
               (static_cast<float>(this->height()) / ((ortho * 2.0f) / aspectRatio));
    }
    moveX = static_cast<float>(event->x());
    moveY = static_cast<float>(event->y());
    this->updateGL();
  }
}

void GLWidget::mouseReleaseEvent(QMouseEvent *event) {
  if (event->button() == Qt::LeftButton) {
    drag = false;
    this->updateGL();
  }
  if (event->button() == Qt::RightButton) {
    pull = false;
    this->updateGL();
  }
}

void GLWidget::resetArc() {
  resetArcBall = !resetArcBall;
  arc->reset();
  this->updateGL();
}

void GLWidget::resetPos() {
  pullX = 0.0;
  pullY = 0.0;
  this->updateGL();
}

void GLWidget::resetZoom() {
  viewWidth = this->width();
  viewHeight = this->height();
  ortho = zoomFactor;
  setOrtho();
  this->updateGL();
}

void GLWidget::drawAxes() {
  glMatrixMode(GL_MODELVIEW); // switch to modelview
  glPushMatrix();             // push a matrix on the stack

  glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, grey); // set colour
  glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, grey); // but they do emit

  glBegin(GL_LINES); // start drawing lines
  glVertex3f(0.0, 0.0, 0.0);
  glVertex3f(1.0, 0.0, 0.0); // x-axis
  glVertex3f(0.0, 0.0, 0.0);
  glVertex3f(0.0, 1.0, 0.0); // y-axis
  glVertex3f(0.0, 0.0, 0.0);
  glVertex3f(0.0, 0.0, 1.0); // z-axis
  glEnd();

  glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, black); // set reflective colour

  glPushMatrix();                 // push a matrix on the stack
  glTranslatef(0.0, 0.0, 1.0);    // move to end of z axis
  objects->drawCone(0.02f, 0.1f, 32);   // draw a solid cone for the z-axis
  glRotatef(90.0, 1.0, 0.0, 0.0); // rotate to draw in z=0 plane
  glTranslatef(0.1f, 0.0, 0.0);    // move away slightly
  glScalef(0.001f, 0.001f, 0.001f);  // scale down
  renderText(0, 0, 0, "Z");
  glPopMatrix(); // pop matrix off stack

  glPushMatrix();              // push a matrix on the stack
  glTranslatef(0.0, 1.0, 0.0); // move to end of y axis
  glRotatef(-90.0, 1.0, 0.0, 0.0);
  objects->drawCone(0.02f, 0.1f, 32);   // draw a solid cone for the y-axis
  glRotatef(90.0, 1.0, 0.0, 0.0); // rotate to draw in y=0 plane
  glTranslatef(0.1f, 0.0, 0.0);    // move away slightly
  glScalef(0.001f, 0.001f, 0.001f);  // scale down
  renderText(0, 0, 0, "Y");
  glPopMatrix(); // pop matrix off stack

  glPushMatrix();              // push a matrix on the stack
  glTranslatef(1.0, 0.0, 0.0); // move to end of x axis
  glRotatef(90.0, 0.0, 1.0, 0.0);
  objects->drawCone(0.02f, 0.1f, 32);   // draw a solid cone for the x-axis
  glRotatef(90.0, 0.0, 1.0, 0.0); // rotate to draw in x=0 plane
  glTranslatef(0.0, 0.1f, 0.0);    // move away slightly
  glScalef(0.001f, 0.001f, 0.001f);  // scale down
  renderText(0, 0, 0, "X");
  glPopMatrix(); // pop matrix off stack

  glPopMatrix(); // pop matrix off stack

  // stop emitting, otherwise other objects will emit the same colour
  glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, black);
}

// function to draw grid on z = 0 plane
void GLWidget::drawGrid(GLfloat *colour, int nGridlines) {
  int x, z;
  // edges don't reflect
  glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, black);
  glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, colour); // but they do emit
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  for (x = -nGridlines; x <= nGridlines; x++) { // for each x
    if (x % 12)
      glLineWidth(1.0);
    else
      glLineWidth(2.0);
    glBegin(GL_LINES);
    glVertex3i(x, 0, -nGridlines);
    glVertex3i(x, 0, +nGridlines);
    glEnd();
  }                                             // for each x
  for (z = -nGridlines; z <= nGridlines; z++) { // for each y
    if (z % 12)
      glLineWidth(1.0);
    else
      glLineWidth(2.0);
    glBegin(GL_LINES);
    glVertex3i(-nGridlines, 0, z);
    glVertex3i(+nGridlines, 0, z);
    glEnd();
  } // for each y
  glLineWidth(1.0);
  glPopMatrix();
  // stop emitting, otherwise other objects will emit the same colour
  glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, black);
}

void GLWidget::idle() {
  if (animation == true) {
    if (time < nframes)
      time++;
    else
      time = 0;
    animate(time);
  }
  this->updateGL();
}

void GLWidget::drawHuman() { // drawHuman()

  /**
      TO DO: draw an articultated human character, you can use this
      member function or model the human using as a class.

      Criteria:

      The human model you create must have :

      1. two arms with elbow joints
      2. two legs with knee joints
      3. a body comprised of at least a pelvis and torso
      4. a neck

      You may also create as many other characters for your scene as you wish
  */

  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, grey);
  glPushMatrix();
    // pelvis
    //glTranslatef(0.0f, 2.0f, 0.0f);
    glTranslatef(body_pos.x, body_pos.y, body_pos.z);
    glRotatef(body_angle, 0.0f, 1.0f, 0.0f);
    objects->drawSphere(0.4f, 32, 32);
    //glTranslatef(-body_pos.x, -body_pos.y, -body_pos.z);

    this->drawChest();
  glPopMatrix();
}

void GLWidget::drawChest() {
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, cyan);
  glPushMatrix();
    glTranslatef(0.0f, 0.5f, 0.0f);
    //  chest
    glRotatef(90.0f, -1.0f, 0.0f, 0.0f);
    objects->drawCylinder(0.4f, 1.0f, 8);
    glRotatef(-90.0f, -1.0f, 0.0f, 0.0f);

    // head
    this->drawHead();

    // Shoulders
    this->drawRightShoulder();
    this->drawLeftShoulder();

    // Thighs
    this->drawRightHip();
    this->drawLeftHip();
  glPopMatrix();
}

void GLWidget::drawHead() {
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, yellow);
  glPushMatrix();
    glTranslatef(0.0f, 0.8f, 0.0f);
    //objects->drawSphere(0.4f, 32, 32);
    glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
    objects->drawCylinder(0.2f, 2.0f, 32);
    glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);

    // Hat
    this->drawHat();
    // Eyes
    this->drawEyes();
  glPopMatrix();
}

void GLWidget::drawHat() {
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, black);
  glPushMatrix();
    glTranslatef(0.0f, 1.2f, 0.0f);
    glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
    objects->drawCylinder(0.2f, 0.4f, 32);
    glTranslatef(0.0f, 0.0f, 0.2f);
    objects->drawCylinder(0.3f, 0.1f, 32);
    glTranslatef(0.0f, 0.0f, -0.2f);
    glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
  glPopMatrix();
}

void GLWidget::drawEyes() {
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, white);
  glPushMatrix();
    glTranslatef(0.15f, 0.4f, 0.2f);
    objects->drawSphere(0.15f, 32, 32);
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, black);
    glPushMatrix();
       glTranslatef(0.0f, 0.0f, 0.12f);
      objects->drawSphere(0.05f, 32, 32);
    glPopMatrix();

    glTranslatef(-0.3f, 0.0f, 0.0f);
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, white);
    objects->drawSphere(0.15f, 32, 32);
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, black);
    glPushMatrix();
      glTranslatef(0.0f, 0.0f, 0.12f);
      objects->drawSphere(0.05f, 32, 32);
    glPopMatrix();

    glTranslatef(0.3f, 0.0f, 0.0f);
  glPopMatrix();
}

void GLWidget::drawLeftShoulder() {
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, cyan);
  glPushMatrix();
    glTranslatef(-0.5f, 0.4f, 0.0f);
    glRotatef(left_arm, 1.0f, 0.0f, 0.0f);
    objects->drawSphere(joint_radius, 32, 32);

    // Left Upper Arm
    this->drawLeftUpperArm();
  glPopMatrix();
}

void GLWidget::drawLeftUpperArm() {
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, yellow);
  glPushMatrix();
    glTranslatef(-0.1f, -0.3f, 0.0f);
    glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
    glRotatef(-20.0f, 0.0f, 1.0f, 0.0f);
    objects->drawCylinder(0.1f, limb_length, 32);

    // Left Elbow
    this->drawLeftElbow();
  glPopMatrix();
}

void GLWidget::drawLeftElbow() {
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, yellow);
  glPushMatrix();
      glTranslatef(0.0f, 0.0f, 0.3f);
      glRotatef(-135.0f, 1.0f, 0.0f, 0.0f);
      objects->drawSphere(joint_radius, 32, 32);

      // Left forearm
      this->drawLeftForeArm();
  glPopMatrix();
}

void GLWidget::drawLeftForeArm() {
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, yellow);
  glPushMatrix();
    glTranslatef(0.0f, -0.35f, 0.0f);
    glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
    objects->drawCylinder(0.1f, limb_length, 32);

    // Left Hand
    this->drawLeftHand();
  glPopMatrix();
}

void GLWidget::drawLeftHand() {
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, yellow);
  glPushMatrix();
    glTranslatef(0.0f, 0.0f, 0.4f);
    objects->drawSphere(0.15f, 32, 32);
  glPopMatrix();
}

void GLWidget::drawRightShoulder() {
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, cyan);
  glPushMatrix();
    glTranslatef(0.5f, 0.4f, 0.0f);
    glRotatef(right_arm, 1.0f, 0.0f, 0.0f);
    objects->drawSphere(joint_radius, 32, 32);

    // Right Upper Arm
    this->drawRightUpperArm();
  glPopMatrix();
}

void GLWidget::drawRightUpperArm() {
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, yellow);
  glPushMatrix();
    glTranslatef(0.1f, -0.3f, 0.0f);
    glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
    glRotatef(20.0f, 0.0f, 1.0f, 0.0f);
    objects->drawCylinder(0.1f, limb_length, 32);

    // Right Elbow
    this->drawRightElbow();
  glPopMatrix();
}

void GLWidget::drawRightElbow() {
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, yellow);
  glPushMatrix();
      glTranslatef(0.0f, 0.0f, 0.3f);
      glRotatef(-135.0f, 1.0f, 0.0f, 0.0f);
      objects->drawSphere(joint_radius, 32, 32);

      // Right Fore Arm
      this->drawRightForeArm();
  glPopMatrix();
}

void GLWidget::drawRightForeArm() {
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, yellow);
  glPushMatrix();
    glTranslatef(0.0f, -0.35f, 0.0f);
    glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
    objects->drawCylinder(0.1f, limb_length, 32);

    // Right Hand
    this->drawRightHand();
  glPopMatrix();
}

void GLWidget::drawRightHand() {
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, yellow);
  glPushMatrix();
    glTranslatef(0.0f, 0.0f, 0.4f);
    objects->drawSphere(0.15f, 32, 32);
  glPopMatrix();
}

void GLWidget::drawRightHip() {
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, grey);
  glPushMatrix();
    glTranslatef(0.3f, -0.6f, 0.0f);
    glRotatef(right_leg, 0.0f, 0.0f, 0.0f);
    objects->drawSphere(joint_radius, 32, 32);

    // Right Thigh
    this->drawRightThigh();
  glPopMatrix();
}

void GLWidget::drawRightThigh() {
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, grey);
  glPushMatrix();
    glTranslatef(0.0f, -0.4f, 0.0f);
    glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
    objects->drawCylinder(0.1f, limb_length, 32);
    glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);

    // Right Knee
    this->drawRightKnee();
  glPopMatrix();
}

void GLWidget::drawRightKnee() {
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, yellow);
  glPushMatrix();
    glTranslatef(0.0f, -0.4f, 0.0f);
    glRotatef(abs(right_leg), 0.0f, 0.0f, 0.0f);
    objects->drawSphere(joint_radius, 32, 32);

    // Right Shin
    this->drawRightShin();
  glPopMatrix();
}

void GLWidget::drawRightShin() {
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, yellow);
  glPushMatrix();
    glTranslatef(0.0f, -0.4f, 0.0f);
    glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
    objects->drawCylinder(0.1f, limb_length, 32);
    glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);

    // Right Foot
    this->drawRightFoot();
  glPopMatrix();
}

void GLWidget::drawRightFoot() {
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, brown);
  glPushMatrix();
    glTranslatef(0.0f, -0.3f, 0.1f);
    objects->drawCylinder(0.1f, 0.3f, 32);
  glPopMatrix();
}

void GLWidget::drawLeftHip() {
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, grey);
  glPushMatrix();
    glTranslatef(-0.3f, -0.6f, 0.0f);
    glRotatef(left_leg, 0.0f, 0.0f, 0.0f);
    objects->drawSphere(joint_radius, 32, 32);

    // Left Thigh
    this->drawLeftThigh();
  glPopMatrix();
}

void GLWidget::drawLeftThigh() {
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, grey);
  glPushMatrix();
    glTranslatef(0.0f, -0.4f, 0.0f);
    glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
    objects->drawCylinder(0.1f, limb_length, 32);
    glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);

    // Left Knee
    this->drawLeftKnee();
  glPopMatrix();
}

void GLWidget::drawLeftKnee() {
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, yellow);
  glPushMatrix();
    glTranslatef(0.0f, -0.4f, 0.0f);
    glRotatef(abs(left_leg), 0.0f, 0.0f, 0.0f);
    objects->drawSphere(joint_radius, 32, 32);

    // Left Shin
    this->drawLeftShin();
  glPopMatrix();
}

void GLWidget::drawLeftShin() {
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, yellow);
  glPushMatrix();
    glTranslatef(0.0f, -0.4f, 0.0f);
    glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
    objects->drawCylinder(0.1f, limb_length, 32);
    glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);

    // Left Foot
    this->drawLeftFoot();
  glPopMatrix();
}

void GLWidget::drawLeftFoot() {
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, brown);
  glPushMatrix();
    glTranslatef(0.0f, -0.3f, 0.1f);
    objects->drawCylinder(0.1f, 0.3f, 32);
  glPopMatrix();
}

void GLWidget::animate(int time) {
  // Compute position
  for (int timeStep = time; timeStep < nframes; timeStep += 5) {
    this->time = time;
    float theta = (static_cast<float>(time) / 600.0f) * 2.0f * static_cast<float>(M_PI);
    //qDebug() << "Theta: " << theta;
    posn_x = cos(theta);
    posn_z = sin(theta);
    body_pos.x = 2*posn_x;
    body_pos.z = 2*posn_z;

    body_angle = 90.0f + (atan(posn_x / posn_z) * 180.0f / static_cast<float>(M_PI));
    if (posn_z > 0) body_angle += 180.0f;

    this->computeJointPositions();
  }
}

void GLWidget::computeJointPositions() {
  if (right_arm > 45.0f) {
      arc_max = true;
  }

  if (right_arm < -45.0f) {
      arc_max = false;
  }

  if (arc_max) {
    right_arm -= 0.01f;
    left_arm += 0.01f;

    right_leg += 0.01f;
    left_leg -= 0.01f;
  } else {
    right_arm += 0.01f;
    left_arm -= 0.01f;

    left_leg += 0.01f;
    right_leg -= 0.01f;
  }
}

void GLWidget::saveImage() {
  QImage image = this->grabFrameBuffer();
  image.save("your_image.jpeg", "jpeg");
  if (!image.isNull()) {
    qDebug() << "Image saved successfully";
  }
}

GLWidget::~GLWidget() {
  delete objects;
  delete arc;
}
