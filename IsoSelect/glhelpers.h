#pragma once

#include "stdafx.h"

class AbstractShader;
class VertexShader;
class FragmentShader;
class GeometryShader;
class TessControlShader;
class TessEvaluationShader;
class GPUProgram;

class Shader
{
public: 
	static GLuint LoadShader(GLenum shaderType, const wchar_t *path);
protected:
	GLuint shaderID;
	Shader(GLenum shaderType, const wchar_t *path);
public:
	virtual ~Shader();
	GLuint getId() const;
};

class VertexShader : public Shader
{
public:
	VertexShader(const wchar_t *path);
};

class FragmentShader : public Shader
{
public:
	FragmentShader(const wchar_t *path);
};

class GeometryShader : public Shader
{
public:
	GeometryShader(const wchar_t *path);
};

class TessControlShader : public Shader
{
public:
	TessControlShader(const wchar_t *path);
};

class TessEvaluationShader : public Shader
{
public:
	TessEvaluationShader(const wchar_t *path);
};

class GPUProgram
{
private:
	GLuint programID;
	mutable std::unordered_map<std::string, GLuint> uniformLocations;
	void checkProgram() const;
	GLuint lookupUniform(std::string name) const;
public:
	GPUProgram(const VertexShader& vertexShader, const FragmentShader& fragmentShader);
	GPUProgram(const VertexShader& vertexShader, const FragmentShader& fragmentShader,
		const GeometryShader& geometryShader);
	GPUProgram(const VertexShader& vertexShader, const FragmentShader& fragmentShader,
		const GeometryShader& geometryShader,
		const TessControlShader& tessControlShader,
		const TessEvaluationShader& tessEvaluationShader);
	GPUProgram(const VertexShader& vertexShader, const FragmentShader& fragmentShader,
		const TessControlShader& tessControlShader,
		const TessEvaluationShader& tessEvaluationShader);

	~GPUProgram();

	void bind() const;
	GLuint getId() const;
	//get uniform ID
	GLuint operator[] (const char* name) const;
};

class Texture
{
protected:
	GLenum textureType;
	GLenum textureSlot;
	GLuint textureId;
	Texture(GLenum type, GLenum slot, GLint magfilter, GLint minFilter,
		GLint wrapS, GLint wrapT, GLint wrapR);

public:
	virtual ~Texture();
	void BindToUniform(GLuint location) const;
	GLint getSlotNumber() const;
	GLuint getId() const;
};

class Texture2D : public Texture
{
public:
	Texture2D(GLenum slot, GLint magfilter, GLint minFilter,
		GLint wrapS, GLint wrapT, GLsizei width, GLsizei height,
		GLenum internalFormat, GLenum format, GLenum colorType,
		const GLvoid *data);
};

class Texture3D : public Texture
{
public:
	Texture3D(GLenum slot, GLint magfilter, GLint minFilter,
		GLint wrapS, GLint wrapT, GLint wrapR, 
		GLsizei width, GLsizei height, GLsizei depth,
		GLenum internalFormat, GLenum format, GLenum colorType,
		const GLvoid *data);
};