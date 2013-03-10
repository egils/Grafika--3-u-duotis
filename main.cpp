#include <iostream>
#include <GL/glut.h>
#include "lib/Matrices.h"
#include "lib/arcball.h"

using namespace std;

#define M_PI 3.14159265358979323846

#define C0 0.559016994374947424102293417183
#define C1 0.904508497187473712051146708591
#define C2 1.46352549156242113615344012577

float zNear=1.0, zFar=100.0;
float aspect = 1;

static int height = 600;
static int width = 600;

const vec eye( 0.0f, 0.0f, 0.0f );
const float SPHERE_RADIUS = 5.0f;
const vec up( 0.0f, 0.0f, 0.0f );

Vector3 axis;
double angle = 0.0;
double maxAngle = 0.0;
bool animation = false;
double step = 0;
const int steps = 200;

Matrix4 rotateMatrix(
    1, 0, 0, 0,
    0, 1, 0, 0,
    0, 0, 1, 0,
    0, 0, 0, 1
);

static float vertices[][3] = {
    {  C1, 0.0,  C2}, //1
    {  C1, 0.0, -C2}, //2
    { -C1, 0.0,  C2}, //3
    { -C1, 0.0, -C2}, //4
    {  C2,  C1, 0.0}, //5
    {  C2, -C1, 0.0}, //6
    { -C2,  C1, 0.0}, //7
    { -C2, -C1, 0.0}, //8
    { 0.0,  C2,  C1}, //9
    { 0.0,  C2, -C1}, //10
    { 0.0, -C2,  C1}, //11
    { 0.0, -C2, -C1}, //12
    { 0.0,  C0,  C2}, //13
    { 0.0,  C0, -C2}, //14
    { 0.0, -C0,  C2}, //15
    { 0.0, -C0, -C2}, //16
    {  C2, 0.0,  C0}, //17
    {  C2, 0.0, -C0}, //18
    { -C2, 0.0,  C0}, //19
    { -C2, 0.0, -C0}, //20
    {  C0,  C2, 0.0}, //21
    {  C0, -C2, 0.0}, //22
    { -C0,  C2, 0.0}, //23
    { -C0, -C2, 0.0}, //24
    {  C1,  C1,  C1}, //25
    { C1,  C1, -C1}, // 26
    {  C1, -C1,  C1}, // 27
    {  C1, -C1, -C1}, // 28
    { -C1,  C1,  C1}, // 29
    { -C1,  C1, -C1}, // 30
    { -C1, -C1,  C1}, // 31
    { -C1, -C1, -C1} //32
};

static int faces[30][4] = {
    {  0, 12,  2, 14 },
    {  0, 14, 10, 26 },
    {  0, 26,  5, 16 },
    {  0, 16,  4, 24 },
    {  0, 24,  8, 12 },
    { 12,  8, 28,  2 },
    {  2, 28,  6, 18 },
    {  2, 18,  7, 30 },
    {  2, 30, 10, 14 },
    { 10, 30,  7, 23 },
    { 10, 23, 11, 21 },
    { 10, 21,  5, 26 },
    {  5, 21, 11, 27 },
    {  5, 27,  1, 17 },
    {  5, 17,  4, 16 },
    {  4, 17,  1, 25 },
    {  4, 25,  9, 20 },
    {  4, 20,  8, 24 },
    {  8, 20,  9, 22 },
    {  8, 22,  6, 28 },
    {  6, 22,  9, 29 },
    {  6, 29,  3, 19 },
    {  6, 19,  7, 18 },
    {  7, 19,  3, 31 },
    {  7, 31, 11, 23 },
    { 11, 31,  3, 15 },
    { 11, 15,  1, 27 },
    {  1, 15,  3, 13 },
    {  1, 13,  9, 25 },
    {  9, 13,  3, 29 }
};

const int lineLength = 2;

void drawAxis(int i) {
    glBegin(GL_LINES);

    float *v = vertices[i];

    glVertex3f(v[0] * lineLength, v[1] * lineLength, v[2] * lineLength);
    glVertex3f(v[0] * -lineLength, v[1] * -lineLength, v[2] * -lineLength);

    glEnd();
}

void drawAxis() {
    glLineWidth(5);

    glColor3d(1, 0, 0);
    drawAxis(5);

    glColor3d(0, 1, 0);
    drawAxis(10);

    glColor3d(0, 0, 1);
    drawAxis(31);
}

static float colors[32][3];

double fabs(double a) {
    return a > 0? a : -a;
}

void randomizeColors() {
    srand(time(NULL));
    for (int x = 0; x < 32; x++) {
        colors[x][0] = (float) rand() / RAND_MAX;
        colors[x][1] = (float) rand() / RAND_MAX;
        colors[x][2] = (float) rand() / RAND_MAX;
    }
}

void getVectorNormal(GLfloat v1[], GLfloat v2[], GLfloat v3[], GLfloat *norm_x, GLfloat *norm_y, GLfloat *norm_z) {
    GLfloat Qx, Qy, Qz, Px, Py, Pz;

    Qx = v2[0] - v1[0];
    Qy = v2[1] - v1[1];
    Qz = v2[2] - v1[2];
    Px = v3[0] - v1[0];
    Py = v3[1] - v1[1];
    Pz = v3[2] - v1[2];

    *norm_x = Py * Qz - Pz * Qy;
    *norm_y = Pz * Qx - Px * Qz;
    *norm_z = Px * Qy - Py * Qx;
}


void drawCylinder(float x1, float y1, float z1, float x2,float y2, float z2, float radius,int subdivisions,GLUquadricObj *quadric) {
    float vx = x2-x1;
    float vy = y2-y1;
    float vz = z2-z1;
    float ax,rx,ry,rz;
    float len = sqrt( vx*vx + vy*vy + vz*vz );

    glPushMatrix();
    glTranslatef( x1,y1,z1 );
    if (fabs(vz) < 0.0001){
        glRotatef(90, 0,1,0);
        ax = 57.2957795*-atan( vy / vx );
        if (vx < 0)
        {
                ax = ax + 180;
        }

        rx = 1;
        ry = 0;
        rz = 0;
    } else {
        ax = 57.2957795*acos( vz/ len );
        if (vz < 0.0)
        {
                ax = -ax;
        }

        rx = -vy*vz;
        ry = vx*vz;
        rz = 0;
    }

    glRotatef(ax, rx, ry, rz);
    gluQuadricOrientation(quadric,GLU_OUTSIDE);
    gluCylinder(quadric, radius, radius, len, subdivisions, 1);
    glPopMatrix();
}

void drawShape() {
    GLfloat norm_x, norm_y, norm_z;
    GLUquadricObj *quadric = NULL;

    for (int x = 0; x < 30; x++) {
        glBegin(GL_POLYGON);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        getVectorNormal(vertices[faces[x][0]], vertices[faces[x][1]], vertices[faces[x][2]], &norm_x, &norm_y, &norm_z);
        glNormal3f(-norm_x, -norm_y, -norm_z);
        glColor3fv(colors[x]);
        for (int v = 0; v < 4; v++)
            glVertex3fv(vertices[faces[x][v]]);
        glEnd();
    }
    
    /* draw spheres */
    glColor3f(1, 1, 1);
    for (int i = 0; i < 32; i++) {
        glPushMatrix();

        glTranslatef(vertices[i][0], vertices[i][1], vertices[i][2]);
        glutSolidSphere(0.1, 10, 10);

        glPopMatrix();
    }

    int v1, v2;

    for (int i = 0; i < 30; i++){
        for (int j = 0; j < 3; j++){
            v1 = j;
            v2 = j + 1;

            quadric = gluNewQuadric();
            gluQuadricNormals(quadric, GLU_SMOOTH);
            drawCylinder(
                vertices[faces[i][v1]][0],
                vertices[faces[i][v1]][1], 
                vertices[faces[i][v1]][2],
                vertices[faces[i][v2]][0], 
                vertices[faces[i][v2]][1],
                vertices[faces[i][v2]][2], 0.03, 10, quadric);
        }

        quadric = gluNewQuadric();
        gluQuadricNormals(quadric, GLU_SMOOTH);
        drawCylinder(
            vertices[faces[i][0]][0], 
            vertices[faces[i][0]][1], 
            vertices[faces[i][0]][2],
            vertices[faces[i][3]][0], 
            vertices[faces[i][3]][1],
            vertices[faces[i][3]][2], 0.03, 10, quadric);
        gluDeleteQuadric(quadric);

    }
}

void draw() {
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(64.0, aspect, zNear, zFar);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glTranslatef(0.0, 0.0, -5.3);

    arcball_rotate();
    glMultMatrixf(rotateMatrix.get());

    drawShape();
    drawAxis();

    glutSwapBuffers();
}

void resize(int w, int h) {
    height = h;
    width = w;

    glViewport(0, 0, w, h);
}

void keyboard(unsigned char c, int x, int y) {
    draw();

    if (animation)
        return;

    float *a;

    switch (c) {
        case 'r':
        case 'e':
            maxAngle = 120;
            a = vertices[5];
            break;
        case 'g':
        case 'f':
            maxAngle = 72;
            a = vertices[10];
            break;
        case 'b':
        case 'v':
            maxAngle = 180;
            a = vertices[31];
            break;
        default:
          return;
    }

    axis.set(a[0], a[1], a[2]);
    axis.normalize();

    step = (double)maxAngle / steps;

    if (c == 'e' || c == 'f' || c == 'v')
            step *= -1;

    animation = true;
    angle = 0;
}

void mouse_handler(int button, int state, int x, int y) {
    if ( state == GLUT_DOWN ) {
        int invert_y = (height - y) - 1;
        arcball_start(width - x, height - invert_y);
    }
}

void mouse_motion_handler(int x, int y) {
    int invert_y = (height - y) - 1;
    arcball_move(width - x, height - invert_y);

    glutPostRedisplay();
}

void animationFunc() {
    if (!animation)
        return;

    if (maxAngle < angle) {
        animation = false;
        return;
    }

    angle += fabs(step);

    rotateMatrix.rotate(step, axis);

    glutPostRedisplay();
}

void init() {
    randomizeColors();
    arcball_reset();

    glutDisplayFunc(draw);
    glutReshapeFunc(resize);
    glutKeyboardFunc(keyboard);
    glutMouseFunc(mouse_handler);
    glutMotionFunc(mouse_motion_handler);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glutIdleFunc(animationFunc);

    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

    arcball_setzoom( SPHERE_RADIUS, eye, up );

    draw();
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(600, 600);
    glutInitWindowPosition(-1, -1);
    glutCreateWindow("Egidijaus Lukausko (PS-2) 3 ir 4 uzduociu programa");

    glEnable(GL_LIGHTING);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_COLOR_MATERIAL);
    //apsvietimas
    GLfloat light_position1[] = {-1, 0, 0, 0};
    GLfloat light_color1[] = {1.0f, 1.0f, 1.0f, 1.0f};
    glLightfv(GL_LIGHT0, GL_POSITION, light_position1);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_color1);
    glLightfv(GL_LIGHT0, GL_SHININESS, light_color1);
    glEnable(GL_LIGHT0);

    GLfloat light_position2[] = {1, 0, 0, 0};
    GLfloat light_color2[] = {1.0f, 1.0f, 1.0f, 1.0f};
    GLfloat light1_ambient[] = { 0, 0, 0, 1.0 };
    GLfloat light1_diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
    GLfloat light1_specular[] = { 1.0, 1.0, 1.0, 1.0 };
    GLfloat light1_position[] = { -2.0, 2.0, 1.0, 1.0 };
    GLfloat spot_direction[] = { -1.0, -1.0, 0.0 };

    glLightfv(GL_LIGHT1, GL_POSITION, light_position2);
    glLightfv(GL_LIGHT1, GL_SPECULAR, light_color2);
    glLightfv(GL_LIGHT1, GL_SHININESS, light_color2);

    glLightfv(GL_LIGHT1, GL_AMBIENT, light1_ambient);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, light1_diffuse);
    glLightf(GL_LIGHT1, GL_CONSTANT_ATTENUATION, 1);
    glLightf(GL_LIGHT1, GL_LINEAR_ATTENUATION, 0);
    glLightf(GL_LIGHT1, GL_QUADRATIC_ATTENUATION, 0);

    glLightf(GL_LIGHT1, GL_SPOT_CUTOFF, 180);
    glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, spot_direction);
    glLightf(GL_LIGHT1, GL_SPOT_EXPONENT, 2.0);

    glEnable(GL_LIGHT1);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

    glutDisplayFunc(init);
    glutMainLoop();
    return 0;
}


