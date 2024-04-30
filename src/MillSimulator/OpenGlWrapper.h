/***************************************************************************
 *   Copyright (c) 2024 Shai Seger <shaise at gmail>                       *
 *                                                                         *
 *   This file is part of the FreeCAD CAx development system.              *
 *                                                                         *
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Library General Public           *
 *   License as published by the Free Software Foundation; either          *
 *   version 2 of the License, or (at your option) any later version.      *
 *                                                                         *
 *   This library  is distributed in the hope that it will be useful,      *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU Library General Public License for more details.                  *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this library; see the file COPYING.LIB. If not,    *
 *   write to the Free Software Foundation, Inc., 59 Temple Place,         *
 *   Suite 330, Boston, MA  02111-1307, USA                                *
 *                                                                         *
 ***************************************************************************/

#ifndef __openglwrapper_h__
#define __openglwrapper_h__
#ifdef QT_OPENGL_LIB
#include "DlgCAMSimulator.h"
extern QOpenGLContext* gOpenGlContext;
extern CAMSimulator::OpenGLWindow* gWindow;
#define glGenBuffers gWindow->glGenBuffers
#define glBindBuffer gWindow->glBindBuffer
#define glBufferData gWindow->glBufferData
#define glGenVertexArrays gWindow->glGenVertexArrays
#define glBindVertexArray gWindow->glBindVertexArray
#define glEnableVertexAttribArray gWindow->glEnableVertexAttribArray
#define glVertexAttribPointer gWindow->glVertexAttribPointer
#define glShaderSource gWindow->glShaderSource
#define glCompileShader gWindow->glCompileShader
#define glAttachShader gWindow->glAttachShader
#define glLinkProgram gWindow->glLinkProgram
#define glGetProgramiv gWindow->glGetProgramiv
#define glGetUniformLocation gWindow->glGetUniformLocation
#define glGetError gWindow->glGetError
#define glEnable gWindow->glEnable
#define glColorMask gWindow->glColorMask
#define glCullFace gWindow->glCullFace
#define glDepthFunc gWindow->glDepthFunc
#define glStencilFunc gWindow->glStencilFunc
#define glStencilOp gWindow->glStencilOp
#define glDepthMask gWindow->glDepthMask
#define glDisable gWindow->glDisable
#define glMatrixMode gWindow->glMatrixMode
#define glUseProgram gWindow->glUseProgram
#define glDrawElements gWindow->glDrawElements
#define glDeleteVertexArrays gWindow->glDeleteVertexArrays
#define glUniformMatrix4fv gWindow->glUniformMatrix4fv
#define glUniform3fv gWindow->glUniform3fv
#define glUniform1i gWindow->glUniform1i
#define glCreateShader gWindow->glCreateShader
#define glCreateProgram gWindow->glCreateProgram
#define glDeleteBuffers gWindow->glDeleteBuffers
#define glActiveTexture gWindow->glActiveTexture
#define glBindTexture gWindow->glBindTexture
#define glGenTextures gWindow->glGenTextures
#define glTexParameteri gWindow->glTexParameteri
#define glTexImage2D gWindow->glTexImage2D
#define glDeleteTextures gWindow->glDeleteTextures
#else
#include "GL/glew.h"
#endif // HAVE_OPENGL_EXT

#endif // !__openglwrapper_h__
