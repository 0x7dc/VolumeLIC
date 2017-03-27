/*----------------------------------------------------*/
/* mainApp.cpp - Volume Rendering Visualization       */
/*                                                    */
/* sunch_nal, 2010                                    */
/* Institute of Meteorology                           */
/* PLA University of Science and Technology           */
/*----------------------------------------------------*/

#include <stdio.h>
#include <iostream.h>
#include <math.h>
#include <GL/glew.h>
#include <GL/glut.h>
#include <Cg/cg.h>
#include <Cg/cgGL.h>

//**定义常量-------------------------------------------/
#define NPIX      400
#define pName     "data\\output_hpf_6.dat"
//**---------------------------------------------------/

//**变量声明-------------------------------------------/
int   time = 0, timebase=0, frame = 0;

int   row = 128;
int   col = 128;
int   lyr = 128;
int   SliceNum = 300;

int   mouse_x = 0;  	    //鼠标位置
int   mouse_y = 0;
int   mouse_left_right = 0; //无->0 | 左键->1 | 右键->2
float rotateX = 0;		    //旋转大小
float rotateY = 0;
float zoom_Sc = 1.3f;       //缩放比例

float coord_x = 0.5f;
float coord_y = 0.5f;
float coord_z = 0.5f;

float*  pVecData;           //矢量数据

static CGcontext cgContext;
static CGprofile cgVertexProfile;
static CGprofile cgFragmtProfile;
static CGprogram cgVertexShader;
static CGprogram cgFragmtShader;

static CGparameter cgVolume;
static CGparameter cgVertexModelViewInvs;
static CGparameter cgVertexModelViewProj;
//**---------------------------------------------------/

//**绘制三维线框---------------------------------------------------------------------/
void Draw3DEnv()
{
	glPushMatrix();
	glEnable(GL_POLYGON_SMOOTH);

	glLineWidth(1.0f);
	glColor3f(0.0f, 0.7f, 0.0f);
	glBegin(GL_LINE_LOOP);
	glVertex3f(coord_x, coord_y,  coord_z);
	glVertex3f(coord_x, coord_y, -coord_z);
	glVertex3f(-coord_x, coord_y, -coord_z);
	glVertex3f(-coord_x, coord_y, coord_z);
	glVertex3f(coord_x, coord_y,  coord_z);
	glVertex3f(coord_x, -coord_y, coord_z);
	glVertex3f(coord_x, -coord_y, -coord_z);
	glVertex3f(-coord_x, -coord_y, -coord_z);
	glVertex3f(-coord_x, -coord_y, coord_z);
	glVertex3f(coord_x, -coord_y, coord_z);
	glEnd();
	glBegin(GL_LINES);
	glVertex3f(-coord_x, coord_y, coord_z);
	glVertex3f(-coord_x, -coord_y, coord_z);
	glVertex3f(-coord_x, coord_y, -coord_z);
	glVertex3f(-coord_x, -coord_y, -coord_z);
	glVertex3f(coord_x, coord_y, -coord_z);
	glVertex3f(coord_x, -coord_y, -coord_z);
	glEnd();
	glPopMatrix();
}
//-----------------------------------------------------------------------------------/

//**绘制函数-------------------------------------------------------------------------/
void Display()
{	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glPushMatrix();
	
	glPushMatrix();
    glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(0, 0, 2.9, 0, 0, 0, 0, 1, 0);
	glRotatef(rotateX, 1.0f, 0.0f, 0.0f);
    glRotatef(rotateY, 0.0f, 0.0f, 1.0f);
    glScalef(zoom_Sc, zoom_Sc, zoom_Sc);
	Draw3DEnv();
	glPopMatrix();
    
	glPushMatrix();
 	glMatrixMode(GL_TEXTURE);
    glLoadIdentity();
 	glRotatef(rotateX, 1.0f, 0.0f, 0.0f);
  	glRotatef(rotateY, 0.0f, 0.0f, 1.0f);
  	glScalef(zoom_Sc, zoom_Sc, zoom_Sc);
 	glTranslatef(-0.5f, -0.5f, -0.5f);
	
	cgGLBindProgram(cgVertexShader);
	cgGLEnableProfile(cgVertexProfile);
	cgGLBindProgram(cgFragmtShader);
	cgGLEnableProfile(cgFragmtProfile);
	cgGLSetStateMatrixParameter(cgVertexModelViewInvs,
		                        CG_GL_TEXTURE_MATRIX,
								CG_GL_MATRIX_INVERSE);
	
	cgGLSetStateMatrixParameter(cgVertexModelViewProj,
		                        CG_GL_MODELVIEW_PROJECTION_MATRIX,
								CG_GL_MATRIX_IDENTITY);
	cgUpdateProgramParameters(cgVertexShader);
	cgGLEnableTextureParameter(cgVolume);
	
	glEnable(GL_BLEND);
	glBegin(GL_QUADS);
	for (int i = 0; i < SliceNum; i++)
	{
		float tempZ = -1.0f + 2 * (float)i / (SliceNum - 1);//0.0;//
		glVertex3f(-1.0f, 1.0f, tempZ);
		glVertex3f(1.0f, 1.0f, tempZ);
		glVertex3f(1.0f, -1.0f, tempZ);
		glVertex3f(-1.0f, -1.0f, tempZ);
	}
	glEnd();
	glDisable(GL_BLEND);

	cgGLDisableProfile(cgFragmtProfile);
	cgGLDisableProfile(cgVertexProfile);
	cgGLDisableTextureParameter(cgVolume);
	
	glPopMatrix();

	//计算刷新率
	frame++;
	time = glutGet(GLUT_ELAPSED_TIME);
	
	if (time - timebase > 1000)
	{
		float fps = frame*1000.0f / (time-timebase);
	 	timebase = time;		
		frame = 0;

		cout<<"刷新率："<<fps<<endl; 
	}

	glutSwapBuffers();
}
//**---------------------------------------------------------------------------------/

//**生成3D Texture-------------------------------------------------------------------/
void Gen3DTexture(void) 
{
	int i = 0;
	int size = row * col * lyr;
	FILE* fp;
	//1 读取矢量数据
	pVecData = new float[row * col * lyr * 3];
	fp = fopen("data\\tornado0.dat", "rb");
	fread(pVecData, sizeof(float), size * 3, fp);  
	//2 读输出数据
	unsigned char* pData = new unsigned char[size];
	fp = fopen(pName, "rb");
	fread(pData, sizeof(unsigned char), size, fp);
	//3 设置3D纹理
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glBindTexture(GL_TEXTURE_3D, 666);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);//GL_REPEAT
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	/////////////////////////////////////////////
	unsigned char* imgData = new unsigned char[size * 4];
	for (i = 0; i < size; i++)
	{
// 		if (pData[i] > 100)
// 		{
// 			imgData[i * 4 + 0] = pData[i];
// 			imgData[i * 4 + 1] = pData[i];
// 			imgData[i * 4 + 2] = pData[i];
// 		} 
// 		else
// 		{
// 			imgData[i * 4 + 0] = 0;
// 			imgData[i * 4 + 1] = 0;
// 			imgData[i * 4 + 2] = 0;
// 		}

		imgData[i * 4 + 0] = pData[i];
		imgData[i * 4 + 1] = pData[i];
		imgData[i * 4 + 2] = pData[i];
		
		float x = pVecData[i * 3 + 0];
		float y = pVecData[i * 3 + 1];
		float z = pVecData[i * 3 + 2];
		double vectMag = sqrt(x*x + y*y + z*z);
		if (vectMag < 0.85f && vectMag > 0.75f)//0.7-0.8
		{
			imgData[i * 4 + 3] = 200;
		}
		else if (vectMag < 0.5f && vectMag > 0.45f)
		{
			imgData[i * 4 + 3] = 20;//20;
		}
		else
		{
			imgData[i * 4 + 3] = 0;
		}
	}
	/////////////////////////////////////////////
    
	//4 生成3D纹理
	//glTexImage3DEXT(GL_TEXTURE_3D_EXT, 0, GL_INTENSITY, row, col, lyr, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, pData);
	glTexImage3DEXT(GL_TEXTURE_3D_EXT, 0, GL_RGBA, row, col, lyr, 0, GL_RGBA, GL_UNSIGNED_BYTE, imgData/*pData*/);
    //glEnable(GL_TEXTURE_3D);
	//glShadeModel(GL_FLAT);
	
	//5 设置纹理坐标
	float xPlane[4] = {1.0, 0.0, 0.0, 0.0};
	float yPlane[4] = {0.0, 1.0, 0.0, 0.0};
	float zPlane[4] = {0.0, 0.0, 1.0, 0.0};
	glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
	glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
	glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
	glTexGenfv(GL_S, GL_EYE_PLANE, xPlane);
	glTexGenfv(GL_T, GL_EYE_PLANE, yPlane);
	glTexGenfv(GL_R, GL_EYE_PLANE, zPlane);
	//glTexGenfv(GL_S, GL_OBJECT_PLANE, xPlane);
	//glTexGenfv(GL_T, GL_OBJECT_PLANE, yPlane);
	//glTexGenfv(GL_R, GL_OBJECT_PLANE, zPlane);

	//glEnable(GL_TEXTURE_GEN_S);
    //glEnable(GL_TEXTURE_GEN_T);
	//glEnable(GL_TEXTURE_GEN_R);

	//glEnable(GL_CULL_FACE);
    //glFrontFace(GL_CW);
	//glCullFace(GL_BACK);
	//glEnable(GL_DEPTH_TEST);
	
	//6 OpenGl设置
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//7 其他
	delete[] pData;
	pData = NULL;
	fclose(fp);
}
//**---------------------------------------------------------------------------------/

//**显示窗口函数---------------------------------------------------------------------/
void ReShape(int w, int h)
{
	double aspect_ratio = (double)w / (double)h;
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0f, aspect_ratio, 0.1f, 500.0f); 

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(0, 0, 3, 0, 0, 0, 0, 1, 0);

	//glMatrixMode(GL_TEXTURE);
    //glLoadIdentity();
	
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);	
}
//**---------------------------------------------------------------------------------/

//**鼠标单击函数---------------------------------------------------------------------/
void MouseClick(int button, int state, int x, int y) 
{ 
	mouse_x = x;
	mouse_y = y;

	if (state == GLUT_DOWN)
	{
		if (button == GLUT_LEFT_BUTTON)
		{
			mouse_left_right = 1;
		} 
		else if(button == GLUT_RIGHT_BUTTON)
		{
			mouse_left_right = 2;
		}
	} 
	else
	{
		mouse_left_right = 0;
	}
}
//**---------------------------------------------------------------------------------/

//**鼠标移动函数---------------------------------------------------------------------/
void MouseMove(int x, int y) 
{
	int tempValue = 0;
	
	if (mouse_left_right == 1)
	{
		//鼠标左键按下
		tempValue = x - mouse_x;
		rotateY += tempValue;

		tempValue = y - mouse_y;
		rotateX += tempValue;
	}
	else if (mouse_left_right == 2)
	{
		//鼠标右键按下
		tempValue = y - mouse_y;
		zoom_Sc += tempValue * 0.01f;
	}
	//记录当前点的坐标
	mouse_x = x;
	mouse_y = y;
	glutPostRedisplay();//刷新窗口
}
//**---------------------------------------------------------------------------------/

//**主函数---------------------------------------------------------------------------/
int main(int argc, char** argv) 
{	
	//glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_SINGLE);
    glutInitWindowSize(NPIX, NPIX);
	glutInitWindowPosition(300, 150);
    glutCreateWindow("3D Texture VolRen Visualization ...");

	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
	    /* Problem: glewInit failed, something is seriously wrong. */
	    //fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
		int aa = 0;
	}

	Gen3DTexture();

	//初始化cg
	cgContext =cgCreateContext();
	cgGLSetDebugMode(CG_FALSE);
	cgSetParameterSettingMode(cgContext, CG_DEFERRED_PARAMETER_SETTING);

	cgVertexProfile = cgGLGetLatestProfile(CG_GL_VERTEX);
	cgGLSetOptimalOptions(cgVertexProfile);
	cgVertexShader = cgCreateProgramFromFile(cgContext,
		                                     CG_SOURCE,
											 "vertex_shader.cg",
											 cgVertexProfile,
											 "vertex_shader",
											 NULL);
	cgGLLoadProgram(cgVertexShader);
	cgVertexModelViewInvs = cgGetNamedParameter(cgVertexShader, "modelViewInvs");
	cgVertexModelViewProj = cgGetNamedParameter(cgVertexShader, "modelViewProj");

    cgFragmtProfile = cgGLGetLatestProfile(CG_GL_FRAGMENT);
	cgGLSetOptimalOptions(cgFragmtProfile);
	cgFragmtShader = cgCreateProgramFromFile(cgContext, 
										     CG_SOURCE,
											 "fragmt_shader.cg",
											 cgFragmtProfile,
											 "fragmt_shader",
											 NULL);
	cgGLLoadProgram(cgFragmtShader);
	cgVolume = cgGetNamedParameter(cgFragmtShader, "volume");
	cgGLSetTextureParameter(cgVolume, 666);

    glutDisplayFunc(Display);
	glutMouseFunc(MouseClick);
	glutMotionFunc(MouseMove);
    glutReshapeFunc(ReShape);
    glutMainLoop();

    return 0;
}
//**---------------------------------------------------------------------------------/

