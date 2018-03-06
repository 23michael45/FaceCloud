#include <gl\glew.h>
#include <gl\freeglut.h>
#include <stdlib.h>
#include <stdio.h>

#pragma comment(lib, "glew32.lib")

GLuint imageFBO;
GLuint imageID;
GLuint depthTextureID;
void SetFrameBufferObject(int fbowidth, int fboheight)
{
	// ����FBO����
	glGenFramebuffersEXT(1, &imageFBO);
	// ����FBO
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, imageFBO);

	// ��������
	glGenTextures(1, &imageID);
	// ��������
	glBindTexture(GL_TEXTURE_2D, imageID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, fbowidth, fboheight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, imageID, 0);

	//
	glGenRenderbuffersEXT(1, &depthTextureID);
	glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, depthTextureID);
	glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT, fbowidth, fboheight);
	glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, depthTextureID);

	GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
	if (status != GL_FRAMEBUFFER_COMPLETE_EXT)
		printf("FBO Initialization Failed.");

	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
}

int IMAGE_WIDTH = 250;
int IMAGE_HEIGHT = 250;

void GenImage()
{
	glPushMatrix();
	glViewport(0, 0, IMAGE_WIDTH, IMAGE_HEIGHT);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glMatrixMode(GL_MODELVIEW);

	glShadeModel(GL_SMOOTH);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, imageFBO);
	glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glBegin(GL_POLYGON);
	glColor3f(1.0, 0.0, 0.0); glVertex3f(-1.0, -1.0, 0.0);
	glColor3f(0.0, 1.0, 0.0); glVertex3f(-1.0, 1.0, 0.0);
	glColor3f(0.0, 0.0, 1.0); glVertex3f(1.0, 1.0, 0.0);
	glColor3f(1.0, 0.0, 0.0); glVertex3f(1.0, -1.0, 0.0);
	glEnd();

	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	glPopMatrix();
}

static GLuint texName;

void display(void)
{
	// ������Ⱦ����
	GenImage();

	glViewport(0, 0, 250, 250);
	glClear(GL_COLOR_BUFFER_BIT);
	glEnable(GL_TEXTURE_2D);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
	//�л���Ļ
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	// ����������Ⱦ������Ӱ��
	glBindTexture(GL_TEXTURE_2D, imageID);
	// ����Mipmap(����ϸ�ڲ�)
	glGenerateMipmapEXT(GL_TEXTURE_2D);

	// ��������Ӱ��
	glBegin(GL_QUADS);
	glTexCoord2f(1.0, 0.0); glVertex3f(-1.0, -1.0, 0.0);
	glTexCoord2f(0.0, 1.0); glVertex3f(-1.0, 1.0, 0.0);
	glTexCoord2f(1.0, 1.0); glVertex3f(1.0, 1.0, 0.0);
	glTexCoord2f(1.0, 0.0); glVertex3f(1.0, -1.0, 0.0);
	glEnd();

	glutSwapBuffers();
	glDisable(GL_TEXTURE_2D);
}


int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(250, 250);
	glutInitWindowPosition(100, 100);
	glutCreateWindow(argv[0]);
	glewInit();
	glutDisplayFunc(display);
	// ���ô���������Ļ
	SetFrameBufferObject(IMAGE_WIDTH, IMAGE_HEIGHT);

	glutMainLoop();
	return 0;
}

