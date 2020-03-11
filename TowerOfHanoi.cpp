
#include<stdio.h>
#include<GL/glut.h>
#include<GL/gl.h>
#include<GL/glu.h>
#include<iostream>
#include<math.h>
#include <fstream>
#include<string.h>
#include<ctime>
using namespace std;
char t[2]={'0'};
char buffer[40];
#define WORD unsigned short
#define DWORD unsigned long
#define LONG long
int time_counter=0;
void DrawText(GLint x, GLint y, char* s, GLfloat r, GLfloat g, GLfloat b)
{
    int lines;
    char* p;
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    //Change to Ortho
    glOrtho(0.0, glutGet(GLUT_WINDOW_WIDTH), 
            0.0, glutGet(GLUT_WINDOW_HEIGHT), -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glColor3f(r,g,b);
    glRasterPos2i(x, y);
    for(p = s, lines = 0; *p; p++) {
        if (*p == '\n') {
            lines++;
            glRasterPos2i(x, y-(lines*500));
        }
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *p);
    }
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

void output(int x, int y, char *string)
{
      int len, i;

           glRasterPos2f(x,y);
           len=(int) strlen(string);
           for (i = 0; i < len; i++)
{
             glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24,string[i]);
}
}

void drawstring(int x, int y, char *string,void *font)
{
      int len, i;

           glRasterPos2f(x,y);
           len=(int) strlen(string);
           for (i = 0; i < len; i++)
{
             glutBitmapCharacter(font,string[i]);
}
}
bool running=0;
class disk
{
    private:
        float radius;
        float c1, c2, c3;  //color of disk
    public:
        void setRadius(float r) {radius = r;}
        void setColor(float co1, float co2, float co3) {c1 = co1; c2 = co2; c3 = co3;}
        float getRadius() {return radius;}
        float getC1() {return c1;}
        float getC2() {return c2;}
        float getC3() {return c3;}
};
GLint WindowWidth = 1240;
GLint WindowHeight = 680;
//to indicate rotate angle (board)
GLfloat rtri = 0.0, Rlight = 0.0, rtrBoard = 0.0;
//Variables used in options (menu)
GLboolean menu_light = false;
GLboolean menu_texture = false;
GLboolean menu_solve = false;
GLboolean menu_restart = false;
GLboolean menu_surface = false;
GLboolean menu_restart_view = false;
GLboolean menu_details = false;

//Indicator of intro animation
GLboolean Gintro = false;
//Used in displaying details
GLint min_steps;
GLint no_steps;
//Time counter
time_t tstart = time(0), tend, tcount;
struct tm *current;
//Buffer for string conversions
//Light details
GLfloat fAmbLight[] = { 0.9f, 0.9f, 0.9f, 0.0f };
GLfloat fDiffLight[] = { 1.0f, 1.0f, 1.0f, 0.0f };
GLfloat fSpecLight[] = { 0.9f, 0.9f, 0.9f, 0.0f };
GLfloat lightPos[] = {1.67f, 1.0f,-2.0f, 1.0f };
GLfloat border = 1; // used in light_move()

//Handle information about disk on each peg
GLint *peg_A, *peg_B, *peg_C;
//Game level, number of disk
GLint no_of_disk;

//Camera set up point
//  This is the change the camera set up point after initial rotation
GLfloat camera_x = 0,
        camera_y = 4,
        camera_z = 14;
//Camera set up point for intro
GLfloat intro_camera_x = 0,
        intro_camera_y = 0,
//intro_camera_z = 12.25;
        intro_camera_z = 0;

GLdouble posX, posY, posZ, //Handle mouse position in 3d space
         ghostX, ghostY; //position of animated disk (ghost disk)

//Approximation of round objects
const GLfloat ROUND = 30;

//Hight from reflective surface
GLfloat HEIGHT = 0.0f;

//Details of the disk movement
GLint from = 0, to = 0;

//Mouse movement
GLfloat moveX = 0.0;

//Quadric - cylinders, spheres
GLUquadricObj *IDquadric;

//Mouse button is holed
bool lbuttonDown = false;

// Check if you win
bool win() {
    output(-21,172,(char *)"---->");
	output(-21,163,(char *)"<----");
	glColor3f(0.0,0.0,1.0);
	output(185,160,(char *)"TIME REMAINING : ");
	drawstring(190,130,(char *)"HURRY UP",GLUT_BITMAP_HELVETICA_18);
	glColor3f(1,0,0);
	drawstring(190,140,(char *)"Time is running out",GLUT_BITMAP_HELVETICA_18);
	sprintf(t,"%d",no_steps);
    output(240,160,t);
    glutPostRedisplay();
    
    int count = 0;
    //Calculate who many disk you have on peg_C
    for(int i = 0; i < no_of_disk; i++) {
        if (peg_C[i] == i+1) count += 1;
    }
    //If all disk are on "C" you win
    if (count == no_of_disk) {
        return true;
    }else return false;
}
// Initialize game details
void init_hanoi() {
    //First clean up
    delete [] peg_A;
    delete [] peg_B;
    delete [] peg_C;
    //Allocate tables plus one cell to store index of the top disk
    peg_A = new int[no_of_disk + 1];
    peg_B = new int[no_of_disk + 1];
    peg_C = new int[no_of_disk + 1];
    for (int i = 0; i < no_of_disk; i++) {
    //Put all disk at the first peg and clear others
        peg_A[i] = i+1;
        peg_B[i] = 0;
        peg_C[i] = 0;
    }
    //Put the proper flag at the end of the table
    peg_A[no_of_disk] = no_of_disk;
    peg_B[no_of_disk] = 0;
    peg_C[no_of_disk] = 0;

    //Init variables used in Details
    no_steps = 0;
    min_steps = pow(2, no_of_disk) - 1;
    if (lbuttonDown) tstart = time(0);
}

void GetOGLPos(int x, int y)
{
    GLint viewport[4];
    GLdouble modelview[16];
    GLdouble projection[16];
    GLfloat winX, winY, winZ;
    glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
    glGetDoublev(GL_PROJECTION_MATRIX, projection);
    glGetIntegerv(GL_VIEWPORT, viewport);
    winX = (float)x;
    winY = (float)viewport[3] - (float)y;
    glReadPixels(x, int(winY), 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &winZ);
    gluUnProject(winX, winY, winZ, modelview, projection, viewport, &posX, &posY, &posZ);
}

// Move peg from S-source to D-destination peg
bool move_it(int *S, int *D)
{
    int S_index, D_index;
    if (S[no_of_disk] == 0)
    {
        return false;
    }
    else
    {
        S_index = S[no_of_disk] - 1;
        //Peg is empty so we can put disk on it
        if ((D[no_of_disk] == 0))
        {
            D[0] = S[S_index];
            S[S_index] = 0;
            S[no_of_disk] -= 1;
            D[no_of_disk] += 1;
            no_steps +=1; //Count this move
            return true;
        }
        else D_index = D[no_of_disk];
        //Check if possible to move
        if (S[S_index] > D[D_index - 1])
        {
            D[D_index] = S[S_index];
            S[S_index] = 0;
            S[no_of_disk] -= 1;
            D[no_of_disk] += 1;
            no_steps +=1; //Count this move
            return true;
        }
        else
        {
            //You cant put bigger on smaller
            return false;
        }
    }
}

void mouse(int button, int state, int x, int y)
{
    //Use only left mouse button to drag and drop disk
    if (button == GLUT_LEFT_BUTTON)
    {
        if (state == GLUT_DOWN)
        {
            lbuttonDown = true;
            GetOGLPos(x,y);
            //Check what peg you "drag"
            if (posX > -4.0 and posX < -1.34 and posY > 0) from = 1; //peg_A
            if (posX > -1.3 and posX < 1.32 and posY > 0) from = 2; //peg_B
            if (posX > 1.32 and posX < 4 and posY > 0) from = 3; //peg_C
            if (win())
            {
                //Go to next level, Put limit of 13 disk
                //It fit nice on pegs
                if (no_of_disk < 13) no_of_disk += 1;
                //Initialise new level
                init_hanoi();
                //go back to normal view - stop spinning
                rtri = 0;
            }
        }
        else
        {
            lbuttonDown = false;
            GetOGLPos(x,y);
            //If drop disk on peg A
            if (posX > -4.0 and posX < -1.34 and posY > 0)
            {
                to = 1; //peg_A
                switch (from) {
                    case 1:
                        break;
                    case 2:
                        move_it(peg_B, peg_A);
                        break;
                    case 3:
                        move_it(peg_C, peg_A);
                        break;
                }
            }
            //If drop disk on peg B
            if (posX > -1.3 and posX < 1.32 and posY > 0)
            {
                to = 2; //peg_B
                switch (from) {
                    case 1:
                        move_it(peg_A, peg_B);
                        break;
                    case 2:
                        break;
                    case 3:
                        move_it(peg_C, peg_B);
                        break;
                }
            }
            //If drop disk on peg C
            if (posX > 1.32 and posX < 4 and posY > 0)
            {
                to = 3; //peg_C
                switch (from)
                {
                    case 1:
                        move_it(peg_A, peg_C);
                        break;
                    case 2:
                        move_it(peg_B, peg_C);
                        break;
                    case 3:
                        break;
                }
            }
        }
    }
}

//Mouse motion
void motion(int x, int y)
{
    if (lbuttonDown)
    {
        GetOGLPos(x, y);
    }
}

// Defining key action
void num_disk(int value)
{
    no_of_disk = value;
    //Initialise with new number
    init_hanoi();
    tstart = time(0);
    glutPostRedisplay();
}

// Called to draw disk
// Depends of parameter type it draws disk with different textures
void draw_disk (GLfloat radius, GLfloat x, GLfloat h, GLboolean type, float c1, float c2, float c3)
{
    if (type)
    {
        glPushMatrix();
        glColor3f(c1, c2, c3);
        glRotatef(90, 1.0f, 0.0, 0.0);
        glTranslatef(x, 0.0, -h -0.6);
        //Main part of the disk
        gluCylinder(IDquadric,radius,radius,0.30f,ROUND, ROUND);
        //Top cover of the disk
        gluDisk(IDquadric,0.125f,radius,ROUND,ROUND);
        //Bottom circle of the disk - not really visible; so we don't have to draw it
        glPopMatrix();
    }
    else
    {
        glPushMatrix();
        glRotatef(90, 1.0f, 0.0, 0.0);
        glTranslatef(x, 0.0, -h -0.4);
        glColor3f(c1, c2, c3);
        //Main part of the disk
        //glEnable(GL_TEXTURE_2D);
        //glBindTexture(GL_TEXTURE_2D, nTexture[0]);
        gluCylinder(IDquadric,radius,radius,0.30f,ROUND, ROUND);
        gluDisk(IDquadric, 0.125f, radius, ROUND, ROUND);
        //Bottom not visible
        glPopMatrix();
    }
}

// Called to draw peg
void draw_peg(GLfloat x)
{
    glColor3f(1.0, 1.0, 1.0);
    glPushMatrix();
    glTranslatef(x,0,0);
    glRotatef(270.0, 1.0f, 0.0f, 0.0f);
    gluCylinder(IDquadric, 0.15f, //base radius
            0.1f, //top radius
            4.0f, //height
            ROUND, //approximation
            ROUND //...
            );
    glTranslatef(0,0,4);
    gluSphere(IDquadric,0.1f,ROUND,ROUND);
    glPopMatrix();
}

// Called to draw game board
void draw_board() {
   
    // The base board
    glColor3f(0.0, 0.0, 0.0);
    glBegin(GL_QUADS); // Start Drawing Hanoi board
    glNormal3f(0.0, 1.0, 0.0); //Use to point proper sides for lighting
    glTexCoord2f(1.0f, 1.0f); glVertex3f( 4.0f, 0.15f,-2.0f); // Top Right Of The Quad (Top)
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-4.0f, 0.15f,-2.0f); // Top Left Of The Quad (Top)
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-4.0f, 0.15f, 2.0f); // Bottom Left Of The Quad (Top)
    glTexCoord2f(1.0f, 0.0f); glVertex3f( 4.0f, 0.15f, 2.0f); // Bottom Right Of The Quad (Top)

    glNormal3f(0.0, -1.0, 0.0);
    glTexCoord2f(1.0f, 1.0f); glVertex3f( 4.0f,-0.15f, 2.0f); // Top Right Of The Quad (Bottom)
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-4.0f,-0.15f, 2.0f); // Top Left Of The Quad (Bottom)
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-4.0f,-0.15f,-2.0f); // Bottom Left Of The Quad (Bottom)
    glTexCoord2f(1.0f, 0.0f); glVertex3f( 4.0f,-0.15f,-2.0f); // Bottom Right Of The Quad (Bottom)
    glNormal3f(0.0, 0.0, 1.0);
    glTexCoord2f(0.0f, 0.0f); glVertex3f( 4.0f, 0.15f, 2.0f); // Top Right Of The Quad (Front)

    glTexCoord2f(1.0f, 0.0f); glVertex3f(-4.0f, 0.15f, 2.0f); // Top Left Of The Quad (Front)
    glTexCoord2f(1.0f, 1.0f); glVertex3f(-4.0f,-0.15f, 2.0f); // Bottom Left Of The Quad (Front)
    glTexCoord2f(0.0f, 1.0f); glVertex3f( 4.0f,-0.15f, 2.0f); // Bottom Right Of The Quad (Front)
    glNormal3f(0.0, 0.0, -1.0);
    glTexCoord2f(1.0f, 0.0f); glVertex3f( 4.0f,-0.15f,-2.0f); // Bottom Left Of The Quad (Back)
    glTexCoord2f(1.0f, 1.0f); glVertex3f(-4.0f,-0.15f,-2.0f); // Bottom Right Of The Quad (Back)
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-4.0f, 0.15f,-2.0f); // Top Right Of The Quad (Back)
    glTexCoord2f(0.0f, 0.0f); glVertex3f( 4.0f, 0.15f,-2.0f); // Top Left Of The Quad (Back)
    glNormal3f(-1.0, 0.0, 0.0);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-4.0f, 0.15f, 2.0f); // Top Right Of The Quad (Left)
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-4.0f, 0.15f,-2.0f); // Top Left Of The Quad (Left)
    glTexCoord2f(1.0f, 1.0f); glVertex3f(-4.0f,-0.15f,-2.0f); // Bottom Left Of The Quad (Left)
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-4.0f,-0.15f, 2.0f); // Bottom Right Of The Quad (Left)
    glNormal3f(1.0, 0.0, 0.0);
    glTexCoord2f(1.0f, 0.0f); glVertex3f( 4.0f, 0.15f,-2.0f); // Top Right Of The Quad (Right)
    glTexCoord2f(1.0f, 1.0f); glVertex3f( 4.0f, 0.15f, 2.0f); // Top Left Of The Quad (Right)
    glTexCoord2f(0.0f, 1.0f); glVertex3f( 4.0f,-0.15f, 2.0f); // Bottom Left Of The Quad (Right)
    glTexCoord2f(0.0f, 0.0f); glVertex3f( 4.0f,-0.15f,-2.0f); // Bottom Right Of The Quad (Right)
    glEnd(); // Done Drawing
}

//Functions draws all disks in the game radius depends of numbers of disk in the game
void draw_all_disks ()
{
    GLfloat h = 0; //height position
    GLfloat r, r_max = 1.1; //declare radius var
    GLfloat r_step = 0.9 / no_of_disk; //calculate difference in radius between disks
    int i, index;
    float rValues[no_of_disk];
    float colors[no_of_disk][3];
    float c1 = 0.1, c2 = 0.0, c3 = 0.0;
    float col1, col2, col3;
    for(i=0; i<no_of_disk; i++)
    {
        rValues[i] = r_max - i * r_step;
        c1 = c1 + 0.3;
        c2 = c2 + 0.2;
        c3 = c3 + 0.25;

        if(c1 > 1.0)
            c1 = 0.1;
        if(c2 > 1.0)
            c2 = 0.1;
        if(c3 > 1.0)
            c3 = 0.1;
        colors[i][0] = c1;
        colors[i][1] = c2;
        colors[i][2] = c3;
    }
    for (i = 0; i < no_of_disk; i++)
    {
        //draw on peg A
        if (peg_A[i] != 0) {
            //Calculate radius of the disk
            r = r_max - ((peg_A[i] - 1) * r_step);
            index = peg_A[i] - 1;
            col1 = colors[index][0]; col2 = colors[index][1]; col3 = colors[index][2];
            draw_disk(r, -2.5, h, menu_texture, col1, col2, col3);
        }
        //draw on peg B
        if (peg_B[i] != 0) {
            //Calculate radius of the disk
            r = r_max - ((peg_B[i] - 1) * r_step);
            index = peg_B[i] - 1;
            col1 = colors[index][0]; col2 = colors[index][1]; col3 = colors[index][2];
            draw_disk(r, 0.0, h, menu_texture, col1, col2, col3);
        }
        //draw on peg C
        if (peg_C[i] != 0) {
            //Calculate radius of the disk
            r = r_max - ((peg_C[i] - 1) * r_step);
            index = peg_C[i] - 1;
            col1 = colors[index][0]; col2 = colors[index][1]; col3 = colors[index][2];
            draw_disk(r, 2.5, h, menu_texture, col1, col2, col3);
        }
        h += 0.3; //Move it up
    }
}
// To move light stripe slowly
void light_move()
{
    //Move just on y and z axis
    lightPos[1] = 8.5;
    lightPos[2] = -15.75;
    //Set up range of movement
    if (border == 1)
    {
        lightPos[0] += 0.1;
        if (lightPos[0] > 20.0) border = 0;
    }
    else
    {
        lightPos[0] -= 0.1;
        if (lightPos[0] < -20.0) border = 1;
    }
}
//Function combine all objets that have to be draw
void Draw_Hanoi ()
{
    glPushMatrix();
    draw_board(); //main board
    draw_peg(0); //first peg
    draw_peg(-2.5); //second peg
    draw_peg(2.5); //third peg
    draw_all_disks(); //draw disks
    glPopMatrix();
}

//Function calculate camera position for intro animation purpose
bool intro() {
    if (intro_camera_x < 3.5)
intro_camera_x *= 0;
   // if (intro_camera_x < 8.5) intro_camera_x += 0.8;
    if (intro_camera_y > 6) {
    //if (intro_camera_y > 1) {
        intro_camera_y *= 0;
        return true;
    } else return false;
}

// Called to draw scene
void RenderScene(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glLoadIdentity(); //Re-set the view
    glEnable(GL_NORMALIZE);
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
    //If flag is set up show light source
    if (!intro())
        gluLookAt(camera_x, camera_y, camera_z, // look from camera XYZ
                0, 0, 0, // look at the origin
                0, 1, 0 // positive Y up vector
                );
    else gluLookAt(intro_camera_x, intro_camera_y, intro_camera_z, 0, 0, 0, 0, 1, 0);
    glScaled(1.3,1.3,1.3); //Make it a bit bigger
    //glRotatef(rtri,0.0f,1.0f,0.0f); //Rotate if Win
    Draw_Hanoi(); //Draw board with disks
    if (menu_surface) HEIGHT = 0.2;
    else HEIGHT = 0;
    //glTranslatef(0.0f, -HEIGHT-0.3, 0.0f); //Place on surface
    glEnable(GL_STENCIL_TEST); //Enable using the stencil buffer
    glColorMask(0, 0, 0, 0); //Disable drawing colors to the screen
    glDisable(GL_DEPTH_TEST); //Disable depth testing
    glStencilFunc(GL_ALWAYS, 1, 1); //Make the stencil test always pass
    //Make pixels in the stencil buffer be set to 1 when the stencil test passes
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    glColorMask(1, 1, 1, 1); //Enable drawing colours to the screen
    glEnable(GL_DEPTH_TEST); //Enable depth testing
    //Make the stencil test pass only when the pixel is 1 in the stencil buffer
    glStencilFunc(GL_EQUAL, 1, 1);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP); //Make the stencil buffer not change
    //Draw Game board, reflected vertically, at all pixels where the stencil buffer is 1
    glPushMatrix();
    glScalef(1, -1, 1);
    glTranslatef(0, HEIGHT, 0);
    Draw_Hanoi();
    glPopMatrix();
    glDisable(GL_STENCIL_TEST); //Disable stencil buffer
    //Blend the floor onto the screen
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    glColor4f(1, 1, 1, 0.3f);
    //Draw mirrored plane
    glDisable(GL_BLEND);
    //If left button is pressed render move tracking disk
    if (lbuttonDown) {
        glPushMatrix();
        glEnable (GL_BLEND);
        glDepthMask (GL_FALSE);
        glBlendFunc (GL_SRC_ALPHA, GL_ONE);
        //Move if you point the game board
        if (posX > -4 && posX < 4) {
            if (posZ > -2 && posZ < 2) {
                glTranslatef(posX, 1.0, posZ);
                glColor4f(1, 1, 1, 0.5f);
            }
        }
        glDepthMask (GL_TRUE);
        glDisable (GL_BLEND);
        glPopMatrix();
    }
    //If you win display a message
    if (!win()) {
    	time_counter++;
        sprintf(buffer, "Your moves: %d", no_steps);
        DrawText((glutGet(GLUT_WINDOW_WIDTH) /2) - 140, glutGet(GLUT_WINDOW_HEIGHT) /2 - 140, buffer, 1.0, 1.0, 1.0);
        sprintf(buffer, "Time: %d", time_counter);
        DrawText((glutGet(GLUT_WINDOW_WIDTH) /2) - 140, glutGet(GLUT_WINDOW_HEIGHT) /2 - 160, buffer, 1.0, 1.0, 1.0);
    }
    else
    {
		glDisable(GL_LIGHTING);
    	sprintf(buffer, "Total moves: %d", no_steps);
        DrawText((glutGet(GLUT_WINDOW_WIDTH) /2) - 140, glutGet(GLUT_WINDOW_HEIGHT) /2 - 140, buffer, 1.0, 1.0, 1.0);
        sprintf(buffer, "Time Taken by you: %d", time_counter);
        DrawText((glutGet(GLUT_WINDOW_WIDTH) /2) - 140, glutGet(GLUT_WINDOW_HEIGHT) /2 - 160, buffer, 1.0, 1.0, 1.0);
    }
    //If required display dame details
    if (win()) {
    	if(running==0){
    	char data[10000];
    	
    	cout<<"YOU win"<<endl;
        cout<<"Your moves: "<< no_steps<<endl;

        //Display time
        current = localtime(&tcount);
        cout<<"Time: "<<current->tm_hour<<current->tm_min<<current->tm_sec<<endl;
        
        std::ofstream outfile;
	
  		outfile.open("test.txt", std::ios_base::app); // append instead of overwrite
  		outfile <<"\nMoves\t"<<"Time\t"<<"No of Disk"<<endl;
  		outfile <<no_steps<<"\t"<<current->tm_hour<<current->tm_min<<current->tm_sec<<"\t"<<no_of_disk <<endl ;
    }
    running=1;
    }
    glColor3f(1.0, 1.0, 10.0);
    glutSwapBuffers();
}

// Change viewing volume and viewport. Called when window is resized
void ChangeSize(int w, int h) {
    GLfloat fAspect;
    // Prevent a divide by zero
    if(h == 0) h = 1;
    // Set Viewport to window dimensions
    glViewport(0, 0, w, h);
    fAspect = (GLfloat)w/(GLfloat)h;
    // Reset coordinate system
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    // Produce the perspective projection
    gluPerspective(60.0f, fAspect, 1.0, 400.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

// Called by GLUT library when idle
void TimerFunction(int value) {
    //If you show light object it will spin
    Rlight +=1.2f;
    //If you win current level board will spin until mouse button pressed
    if (win()) {
        rtri+=1.2f;
        menu_solve = false;
        lightPos[0] = 0.42;
        lightPos[1] = 9.75;
        lightPos[2] = -22;
    }
    if (!win()) tend = time(0);
    tcount = difftime(tend, tstart);
    //Redraw the scene with new coordinates
    glutPostRedisplay();
    glutTimerFunc(1,TimerFunction, 1);
}
// Setup the rendering state
void SetupRC(void) {
    char tmp[50];
    //glShadeModel(GL_SMOOTH); //Enables Smooth Shading
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f); //Background
    IDquadric=gluNewQuadric(); //Create A Pointer To The Quadric Object
    gluQuadricNormals(IDquadric, GLU_SMOOTH); //Create Smooth Normals
    gluQuadricTexture(IDquadric, GL_TRUE); //Create Texture Coords
    no_of_disk = 7; //Game start level
    init_hanoi(); //Initialize data
    glClearDepth(1.0f); //Depth Buffer Setup
    glEnable(GL_DEPTH_TEST); //Enables Depth Testing
    glDepthFunc(GL_LEQUAL); //The Type Of Depth Test To Do
}

// Main program entry point
int main(int argc, char* argv[]) {
    GLint sub;
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowSize(WindowWidth, WindowHeight);
    glutInitWindowPosition(0, 0); //Place window on left top corner
    glutCreateWindow("CG ASSIGNMENT TOWER OF HANOI");
    SetupRC();
    glutReshapeFunc(ChangeSize);
    glutDisplayFunc(RenderScene);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);
    glutTimerFunc(33, TimerFunction, 1);
    glutMainLoop();
}

