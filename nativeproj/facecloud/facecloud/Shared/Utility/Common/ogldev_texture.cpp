/*

	Copyright 2011 Etay Meiri

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <iostream>
#include "ogldev_texture.h"
#include "OSMesaContext.h"

Texture::Texture(GLenum TextureTarget)
{
    m_textureTarget = TextureTarget;
}
Texture::Texture()
{

}
Texture::~Texture()
{
	if (m_textureObj != 0) {
		glDeleteTextures(1, &m_textureObj);
	}
}
bool Texture::LoadFile( const std::string& FileName)
{
	try
	{

		m_fileName = FileName;
		try {
			m_image.read(m_fileName);
			m_image.write(&m_blob, "RGBA");
		}
		catch (Magick::Error& Error) {
			std::cout << "Error loading texture '" << m_fileName << "': " << Error.what() << std::endl;
			return false;
		}

		Gen(m_image);
	}
	catch (...)
	{
		std::cout << "Error loading texture other error '" << m_fileName <<  std::endl;
		return false;
	}

    
    return true;
}
bool Texture::LoadBase64(std::string& base64string)
{
	try {
		m_blob.base64(base64string);
		m_image.read(m_blob);
	}
	catch (Magick::Error& Error) {
		std::cout << "Error loading blob texture '" <<  "': " << Error.what() << std::endl;
		return false;
	}

	Gen(m_image);
}
void Texture::Gen(Magick::Image image)
{
	try
	{
		glGenTextures(1, &m_textureObj);
		glBindTexture(m_textureTarget, m_textureObj);
		glTexImage2D(m_textureTarget, 0, GL_RGBA, m_image.size().width(), m_image.size().height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, m_blob.data());
		glTexParameterf(m_textureTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameterf(m_textureTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glBindTexture(m_textureTarget, 0);
	}
	catch (...)
	{
		OSMesa::Log("Gen Texture Error");
	}
}

void Texture::Bind(GLenum TextureUnit)
{
    glActiveTexture(TextureUnit);
    glBindTexture(m_textureTarget, m_textureObj);
}
GLuint Texture::GetTextureObj()
{
	return m_textureObj;
}


// don't forget to include related head files
void Texture::FromCVMat(GLenum TextureTarget, cv::Mat& image)
{
	m_textureTarget = TextureTarget;
	if (image.empty()) {
		std::cout << "image empty" << std::endl;
	}
	else {
		//glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		//glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
		glGenTextures(1, &m_textureObj);
		glBindTexture(m_textureTarget, m_textureObj);

		//glTexParameteri(m_textureTarget, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		//glTexParameteri(m_textureTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		//// Set texture clamping method
		//glTexParameteri(m_textureTarget, GL_TEXTURE_WRAP_S, GL_CLAMP);
		//glTexParameteri(m_textureTarget, GL_TEXTURE_WRAP_T, GL_CLAMP);

		cv::cvtColor(image, image, CV_RGB2RGBA);

		glTexImage2D(GL_TEXTURE_2D,         // Type of texture
			0,                   // Pyramid level (for mip-mapping) - 0 is the top level
			GL_RGBA,              // Internal colour format to convert to
			image.cols,          // Image width  i.e. 640 for Kinect in standard mode
			image.rows,          // Image height i.e. 480 for Kinect in standard mode
			0,                   // Border width in pixels (can either be 1 or 0)
			GL_RGBA,              // Input image format (i.e. GL_RGB, GL_RGBA, GL_BGR etc.)
			GL_UNSIGNED_BYTE,    // Image data type
			image.ptr());        // The actual image data itself

		glTexParameterf(m_textureTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameterf(m_textureTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glBindTexture(m_textureTarget, 0);
	}
}


void Texture::CloneFromTexture(GLuint tex)
{
	glBindTexture(GL_TEXTURE_2D, tex);
	GLint wtex, htex, comp, rs, gs, bs, as;
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &wtex);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &htex);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, &comp);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_RED_SIZE, &rs);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_GREEN_SIZE, &gs);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_BLUE_SIZE, &bs);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_ALPHA_SIZE, &as);


	OSMesa::Log(OSMesa::format("\nCloneFromTexture c r : %i %i:", wtex, htex));
	long size = 0;
	if (comp == GL_RGB)
	{

		size = wtex * htex * 3;
	}
	else if (comp == GL_RGBA)
	{
		size = wtex * htex * 4;
	}

	unsigned char * data = new unsigned char[size];
	glGetTexImage(GL_TEXTURE_2D, 0, comp, GL_UNSIGNED_BYTE, data);

	m_textureTarget = GL_TEXTURE_2D;
	glGenTextures(1, &m_textureObj);


	glBindTexture(m_textureTarget, m_textureObj);
	//glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, wtex, htex, comp, GL_UNSIGNED_BYTE, data);

	glTexImage2D(GL_TEXTURE_2D,         // Type of texture
		0,                   // Pyramid level (for mip-mapping) - 0 is the top level
		comp,              // Internal colour format to convert to
		wtex,          // Image width  i.e. 640 for Kinect in standard mode
		htex,          // Image height i.e. 480 for Kinect in standard mode
		0,                   // Border width in pixels (can either be 1 or 0)
		comp,              // Input image format (i.e. GL_RGB, GL_RGBA, GL_BGR etc.)
		GL_UNSIGNED_BYTE,    // Image data type
		data);        // The actual image data itself


	glTexParameterf(m_textureTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(m_textureTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(m_textureTarget, 0);


}
void Texture::CloneFromFBOTexture(GLuint tex)
{
	glBindTexture(GL_TEXTURE_2D, tex);
	GLint wtex, htex;
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &wtex);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &htex);


	GLuint fbo;
	glGenFramebuffers(1, &fbo);
	/// bind the FBO
	glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
	/// attach the source texture to the fbo
	glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
		GL_TEXTURE_2D, tex, 0);


	/// bind the destination texture
	glBindTexture(GL_TEXTURE_2D, m_textureObj);
	/// copy from framebuffer (here, the FBO!) to the bound texture
	glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, wtex, htex);
	/// unbind the FBO
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
}