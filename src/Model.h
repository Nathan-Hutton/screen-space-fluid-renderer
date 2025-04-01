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
		std::vector<unsigned int> m_elements;

		std::vector<cy::Vec3f> m_verts;   //actual vertex coordinates
		std::vector<cy::Vec3f> m_norms;   //actual normal vectors
		std::vector<cy::Vec2f> m_texts;   //texture coordinates
	};

private:
	// gl data
	GLuint m_VAO;			//model vertex array
	GLuint m_VBO[3];		//position, normal, and texcoord buffers
	GLuint m_eBuffer;		//model element buffer
	unsigned int m_elementsLength;

	// positioning data
	cy::Matrix4f m_modelTrans;
	float m_xPos, m_yPos, m_zPos;
	float m_xRot, m_yRot, m_zRot;

	// obj file data
	const char* m_objFilename;
	cy::TriMesh* m_mesh;
	MeshBuffers* m_buffers;
	cy::GLTexture2D m_difTex;
	cy::GLTexture2D m_spcTex;

	// shader data
	cy::GLSLProgram* m_program;
	//const char* vertFile;
	//const char* fragFile;

	// environment shader data (optional)
	cy::GLTextureCubeMap m_envTex;

	void calcTrans();
	void prepareBuffers();
	void createElementBuffer();
	void createVertexArray();

public:
	Model();
	~Model();

	void Initialize();	// prepare for rendering based on current meshbuffers
	void Bind();	// bind VAO and element buffer for rendering
	void Unbind() const;
	void CompileShaders(const char* vertFile, const char* fragFile);

	cy::Matrix4f GetTrans() { calcTrans(); return m_modelTrans; }
	cy::GLTexture2D GetDif() const { return m_difTex; }
	cy::GLTexture2D GetSpc() const { return m_spcTex; }
	cy::GLTextureCubeMap GetEnv() const { return m_envTex; }
	MeshBuffers* GetBuffers() { return m_buffers; }
	unsigned int GetLength() const { return m_elementsLength; }
	cy::GLSLProgram* GetProgram() { return m_program; }
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

	void SetYUp() { m_xRot = -M_PI / 2; }
};

