#include "stdafx.h"
#include "glhelpers.h"

GLuint Shader::LoadShader(GLenum shaderType, const wchar_t *path)
{
	GLuint id = glCreateShader(shaderType);

	std::string shaderCode;
	std::ifstream shaderStream(path, std::ios::in);
	if (shaderStream.is_open()){
		std::string Line = "";
		while (std::getline(shaderStream, Line))
			shaderCode += "\n" + Line;
		shaderStream.close();
	}
	else{
		printf("Impossible to open %ls. Are you in the right directory ?\n", path);
		getchar();
		return 0;
	}

	printf("Compiling shader : %ls\n", path);

	const char *sourcePointer = shaderCode.c_str();
	glShaderSource(id, 1, &sourcePointer, NULL);
	glCompileShader(id);

	GLint result = GL_FALSE;
	GLint infoLogLength;

	glGetShaderiv(id, GL_COMPILE_STATUS, &result);
	glGetShaderiv(id, GL_INFO_LOG_LENGTH, &infoLogLength);
	if (infoLogLength > 0){
		char *errorMessage = new char[infoLogLength + 1];
		glGetShaderInfoLog(id, infoLogLength, NULL, errorMessage);
		printf("%s\n", errorMessage);
		delete errorMessage;
	}

	return id;
}

Shader::Shader(GLenum shaderType, const wchar_t *path)
{
	shaderID = LoadShader(shaderType, path);
}

Shader::~Shader()
{
	glDeleteShader(shaderID);
}

GLuint Shader::getId() const
{
	return shaderID;
}

VertexShader::VertexShader(const wchar_t *path) : Shader(GL_VERTEX_SHADER, path){}

FragmentShader::FragmentShader(const wchar_t *path) : Shader(GL_FRAGMENT_SHADER, path){}

GeometryShader::GeometryShader(const wchar_t *path) : Shader(GL_GEOMETRY_SHADER, path){}

TessControlShader::TessControlShader(const wchar_t *path) : Shader(GL_TESS_CONTROL_SHADER, path){}

TessEvaluationShader::TessEvaluationShader(const wchar_t *path) : Shader(GL_TESS_EVALUATION_SHADER, path){}

void GPUProgram::checkProgram() const
{
	GLint result = GL_FALSE;
	GLint infoLogLength;
	glGetProgramiv(programID, GL_LINK_STATUS, &result);
	glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &infoLogLength);
	if (infoLogLength > 0){
		char *errorMessage = new char[infoLogLength + 1];
		glGetProgramInfoLog(programID, infoLogLength, NULL, errorMessage);
		printf("%s\n", errorMessage);
		delete errorMessage;
	}
}

GPUProgram::GPUProgram(const VertexShader& vertexShader, const FragmentShader& fragmentShader)
{
	programID = glCreateProgram();
	glAttachShader(programID, vertexShader.getId());
	glAttachShader(programID, fragmentShader.getId());
	glLinkProgram(programID);
	checkProgram();
}

GPUProgram::GPUProgram(const VertexShader& vertexShader, const FragmentShader& fragmentShader,
	const GeometryShader& geometryShader)
{
	programID = glCreateProgram();
	glAttachShader(programID, vertexShader.getId());
	glAttachShader(programID, fragmentShader.getId());
	glAttachShader(programID, geometryShader.getId());
	glLinkProgram(programID);
	checkProgram();
}

GPUProgram::GPUProgram(const VertexShader& vertexShader, const FragmentShader& fragmentShader,
	const GeometryShader& geometryShader,
	const TessControlShader& tessControlShader,
	const TessEvaluationShader& tessEvaluationShader)
{
	programID = glCreateProgram();
	glAttachShader(programID, vertexShader.getId());
	glAttachShader(programID, fragmentShader.getId());
	glAttachShader(programID, geometryShader.getId());
	glAttachShader(programID, tessControlShader.getId());
	glAttachShader(programID, tessEvaluationShader.getId());
	glLinkProgram(programID);
	checkProgram();
}

GPUProgram::GPUProgram(const VertexShader& vertexShader, const FragmentShader& fragmentShader,
	const TessControlShader& tessControlShader,
	const TessEvaluationShader& tessEvaluationShader)
{
	programID = glCreateProgram();
	glAttachShader(programID, vertexShader.getId());
	glAttachShader(programID, fragmentShader.getId());
	glAttachShader(programID, tessControlShader.getId());
	glAttachShader(programID, tessEvaluationShader.getId());
	glLinkProgram(programID);
	checkProgram();
}

GPUProgram::~GPUProgram()
{
	glDeleteProgram(programID);
}

void GPUProgram::bind() const
{
	glUseProgram(programID);
}

GLuint GPUProgram::getId() const
{
	return programID;
}

GLuint GPUProgram::lookupUniform(std::string name) const
{
	if (uniformLocations.find(name) == uniformLocations.end())
		uniformLocations[name] = glGetUniformLocation(programID, name.c_str());
	return uniformLocations[name];
}

GLuint GPUProgram::operator[] (std::string&& name) const
{
	return lookupUniform(std::forward<std::string>(name));
}

Texture::Texture(GLenum type, GLenum slot, GLint magfilter, GLint minFilter,
	GLint wrapS, GLint wrapT, GLint wrapR)
{
	textureType = type;
	textureSlot = slot;
	glActiveTexture(textureSlot);
	glGenTextures(1, &textureId);
	glBindTexture(textureType, textureId);
	glTexParameteri(textureType, GL_TEXTURE_MIN_FILTER, minFilter);
	glTexParameteri(textureType, GL_TEXTURE_MAG_FILTER, magfilter);
	glTexParameteri(textureType, GL_TEXTURE_WRAP_S, wrapS);
	glTexParameteri(textureType, GL_TEXTURE_WRAP_T, wrapT);
	glTexParameteri(textureType, GL_TEXTURE_WRAP_R, wrapR);
}

Texture::~Texture()
{
	glDeleteTextures(1, &textureId);
}

void Texture::BindToUniform(GLuint location) const
{
	glActiveTexture(textureSlot);
	glBindTexture(textureType, textureId);
	glUniform1i(location, getSlotNumber());
}

GLint Texture::getSlotNumber() const
{
	return textureSlot - GL_TEXTURE0;
}

GLuint Texture::getId() const
{
	return textureId;
}

Texture2D::Texture2D(GLenum slot, GLint magfilter, GLint minFilter,
	GLint wrapS, GLint wrapT, GLsizei width, GLsizei height,
	GLenum internalFormat, GLenum format, GLenum colorType,
	const GLvoid *data) : Texture(GL_TEXTURE_2D, slot, magfilter, minFilter, wrapS, wrapT,
	GL_CLAMP_TO_EDGE)
{
	glTexImage2D(slot, 0, internalFormat, width, height, 0, format, colorType, data);
}

Texture3D::Texture3D(GLenum slot, GLint magfilter, GLint minFilter,
	GLint wrapS, GLint wrapT, GLint wrapR,
	GLsizei width, GLsizei height, GLsizei depth,
	GLenum internalFormat, GLenum format, GLenum colorType,
	const GLvoid *data) : Texture(GL_TEXTURE_3D, slot, magfilter, minFilter, wrapS, wrapT,
	wrapR)
{
	glTexImage3D(slot, 0, internalFormat, width, height, depth, 0, format, colorType, data);
}