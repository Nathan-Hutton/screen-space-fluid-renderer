#pragma once
#define _USE_MATH_DEFINES

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <cmath>

#include "cyMatrix.h"
#include "cyTriMesh.h"
#include "cyGL.h"
#include "lodepng.h"

// contains all model data and methods
class Model
{
public:
	struct MeshBuffers
	{
		std::vector<unsigned int> elements;

		std::vector<cy::Vec3f> verts;   //actual vertex coordinates
		std::vector<cy::Vec3f> norms;   //actual normal vectors
		std::vector<cy::Vec2f> texts;   //texture coordinates
	};

private:
	// gl data
	GLuint VAO;			//model vertex array
	GLuint VBO[3];		//position, normal, and texcoord buffers
	GLuint eBuffer;		//model element buffer
	unsigned int elementsLength;

	// positioning data
	cy::Matrix4f modelTrans;
	float xPos, yPos, zPos;
	float xRot, yRot, zRot;

	// obj file data
	const char* objFilename;
	cy::TriMesh* mesh;
	MeshBuffers* buffers;
	cy::GLTexture2D difTex;
	cy::GLTexture2D spcTex;

	// shader data
	cy::GLSLProgram* program;
	//const char* vertFile;
	//const char* fragFile;

	// environment shader data (optional)
	cy::GLTextureCubeMap envTex;

	void calcTrans();
	void prepareBuffers();
	void createElementBuffer();
	void createVertexArray();

public:
	Model();
	~Model();

	void Initialize();	// prepare for rendering based on current meshbuffers
	void Bind();	// bind VAO and element buffer for rendering
	void Unbind();
	void CompileShaders(const char* vertFile, const char* fragFile);

	cy::Matrix4f GetTrans() { calcTrans(); return modelTrans; }
	cy::GLTexture2D GetDif() { return difTex; }
	cy::GLTexture2D GetSpc() { return spcTex; }
	cy::GLTextureCubeMap GetEnv() { return envTex; }
	MeshBuffers* GetBuffers() { return buffers; }
	unsigned int GetLength() { return elementsLength; }
	cy::GLSLProgram* GetProgram() { return program; }
	//cy::Vec3f GetPos() { return cy::Vec3f(xPos, yPos, zPos); }

	void LoadOBJFile(const char* filename);
	void SetVerts(std::vector<float> vertData);
	void SetNorms(std::vector<float> normData);
	void SetTexts(std::vector<float> textData);

	void SetTranslation(float x, float y, float z);		//explicitly sets translation values
	void CenterModel();

	void DifTexSetup();
	void SpcTexSetup();
	void LoadEnvMap(const char* folder, const char** filenames);

	void UpdateRotation(float x, float y, float z);		//adds incoming values to current state

	void SetYUp() { xRot = -M_PI / 2; }
};

