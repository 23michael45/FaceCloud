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

#ifndef TEXTURE_H
#define	TEXTURE_H

#include <string>

#include <GL/glew.h>
#include <ImageMagick-6/Magick++.h>
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/objdetect.hpp"
#include <opencv2/imgproc/imgproc.hpp> 
#include <opencv2/core/core.hpp> 
class Texture
{
public:
    Texture(GLenum TextureTarget);
	Texture();
	~Texture();
	bool LoadFile(const std::string& FileName);
	bool LoadBase64(std::string& base64string);

    void Bind(GLenum TextureUnit);
	GLuint GetTextureObj();

	void FromCVMat(GLenum TextureTarget, cv::Mat& image);


	void CloneFromTexture(GLuint tex);
	void CloneFromFBOTexture(GLuint tex);
private:
    std::string m_fileName;
    GLenum m_textureTarget;
    GLuint m_textureObj;
    Magick::Image m_image;
    Magick::Blob m_blob;

	void Gen(Magick::Image image);


};


#endif	/* TEXTURE_H */

