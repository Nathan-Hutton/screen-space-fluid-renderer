#include "Model.h"
#include <filesystem>
#include <stdlib.h>
#include <linux/limits.h>

using path = std::filesystem::path;

Model::Model() {
	m_xPos = 0;
	m_yPos = 0;
	m_zPos = 0;
    m_xRot = 0;
    m_yRot = 0;
    m_zRot = 0;
	m_modelTrans = cy::Matrix4f();

	m_mesh = NULL;
	m_buffers = new MeshBuffers();
    m_objFilename = NULL;
    m_program = new cy::GLSLProgram();
}

Model::~Model() {
    if (m_mesh) {
        delete m_mesh;
    }
    if (m_buffers) {
        delete m_buffers;
    }

    glDeleteVertexArrays(1, &m_VAO);
    glDeleteBuffers(3, m_VBO);
    glDeleteBuffers(1, &m_eBuffer);
}

void Model::Initialize()
{
    //CompileShaders();
    createElementBuffer();
}

void Model::Bind()
{
    // bind our VAO
    glBindVertexArray(m_VAO);

    // bind the element buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_eBuffer);
}

void Model::Unbind() const
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void Model::CompileShaders(const char* vertFile, const char* fragFile)
{
    char absVert[PATH_MAX];
    char absFrag[PATH_MAX];

    if (realpath(vertFile, absVert) == nullptr)
        printf("realpath failed in Model::CompileShader(): No such file or directory: %s\n", vertFile);
    if (realpath(fragFile, absFrag) == nullptr)
        printf("realpath failed in Model::CompileShader(): No such file or directory: %s\n", vertFile);

    //std::cout << absVert << std::endl;
    //std::cout << absFrag << std::endl;

    const bool compiled = m_program->BuildFiles(absVert, absFrag);

    if (!compiled)
        fprintf(stderr, "Error: could not compile shaders\n");
}

void Model::calcTrans() {
    cy::Matrix4f rotations = cy::Matrix4f();
    rotations.SetRotationXYZ(m_xRot, m_yRot, m_zRot);

    cy::Matrix4f translations = cy::Matrix4f();
    translations.SetTranslation(cy::Vec3f(m_xPos, m_yPos, m_zPos));

    m_modelTrans = rotations * translations;
}

void Model::LoadOBJFile(char const* filename) {
    m_mesh = new cy::TriMesh();

    char objPath[PATH_MAX];
    if (realpath(filename, objPath) == nullptr)
        printf("realpath failed in Model::LoadObjFile(): No such file or directory: %s\n", filename);

    bool loadedMesh = m_mesh->LoadFromFileObj(objPath, true);
    if (!loadedMesh) {
        fprintf(stderr, "Error: '%s'\n", "unable to load obj file");
        m_mesh = NULL;
        return;
    }
    //fprintf(stdout, "OBJ loaded. # of verticies: '%d'\n", m_mesh->NV());

    m_objFilename = filename;

    prepareBuffers();
    DifTexSetup();
    SpcTexSetup();
}

void Model::prepareBuffers()
{
    const int faceCount = m_mesh->NF();

    m_buffers = new MeshBuffers();

    unsigned int counter = 0;

    // for each face
    for (int i = 0; i < faceCount; i++) {
        const cy::TriMesh::TriFace m_faceVerts = m_mesh->F(i);
        const cy::TriMesh::TriFace m_faceNorms = m_mesh->FN(i);
        const cy::TriMesh::TriFace m_faceTexts = m_mesh->FT(i);

        //for each vertex (we know its always 3)
        for (int j = 0; j < 3; j++) {
            const unsigned int vertIdx = m_faceVerts.v[j];
            const unsigned int normIdx = m_faceNorms.v[j];
            const unsigned int textIdx = m_faceTexts.v[j];

            m_buffers->m_verts.push_back(m_mesh->V(vertIdx));
            m_buffers->m_norms.push_back(m_mesh->VN(normIdx));
            m_buffers->m_texts.push_back(cy::Vec2f(m_mesh->VT(textIdx)));
            m_buffers->m_elements.push_back(counter++);
        }
    }

    //return myBuffs;
}

void Model::CenterModel() {
    if (!m_mesh) {
        return;
    }

    // get bounding box data
    m_mesh->ComputeBoundingBox();
    const cy::Vec3f boxMin = m_mesh->GetBoundMin();
    const cy::Vec3f boxMax = m_mesh->GetBoundMax();

    m_xPos = -(boxMax[0] + boxMin[0]) / 2;
    m_yPos = -(boxMax[1] + boxMin[1]) / 2;
    m_zPos = -(boxMax[2] + boxMin[2]) / 2;
}

void Model::DifTexSetup() {
    if (!m_mesh)
        return;

    // get texture data
    // assuming one material for now bc i'm lazy
    if (m_mesh->NM() == 0) return;
    const cy::TriMesh::Mtl mat = m_mesh->M(0);

    cy::TriMesh::Str difMap = mat.map_Kd;
    if (!difMap)
        return;

    const path parent = path(m_objFilename).parent_path();
    const path difPath = parent / path(difMap.data);

    // setting up the diffuse texture
    std::vector<unsigned char> png;
    std::vector<unsigned char> image; //the raw pixels
    unsigned width, height;

    //load and decode
    unsigned error = lodepng::load_file(png, difPath.string());
    if (!error) error = lodepng::decode(image, width, height, png);
    //if there's an error, display it
    if (error) std::cout << "decoder error " << error << ": " << lodepng_error_text(error) << std::endl;

    m_difTex.Initialize();
    m_difTex.SetImage(&image[0], 4, width, height);
    m_difTex.BuildMipmaps();
    m_difTex.SetFilteringMode(GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR);
    m_difTex.SetWrappingMode(GL_REPEAT, GL_REPEAT);
}

void Model::SpcTexSetup() {
    if (!m_mesh)
        return;

    // get texture data
    // assuming one material for now bc i'm lazy
    if (m_mesh->NM() == 0) return;
    const cy::TriMesh::Mtl mat = m_mesh->M(0);

    cy::TriMesh::Str spcMap = mat.map_Ks;
    if (!spcMap)
        return;

    const path parent = path(m_objFilename).parent_path();
    const path spcPath = parent / path(spcMap.data);

    std::vector<unsigned char> png;
    std::vector<unsigned char> image;
    unsigned width, height;

    //load and decode
    unsigned error = lodepng::load_file(png, spcPath.string());
    if (!error) error = lodepng::decode(image, width, height, png);
    if (error) std::cout << "decoder error " << error << ": " << lodepng_error_text(error) << std::endl;

    m_spcTex.Initialize();
    m_spcTex.SetImage(&image[0], 4, width, height);
    m_spcTex.BuildMipmaps();
    m_spcTex.SetFilteringMode(GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR);
    m_spcTex.SetWrappingMode(GL_REPEAT, GL_REPEAT);
}

void Model::createVertexArray()
{
    const size_t size = m_buffers->m_verts.size();
    // initialize the vao
    glGenVertexArrays(1, &m_VAO);
    glBindVertexArray(m_VAO);

    // initializing the vbos
    glGenBuffers(1, &m_VBO[0]);   //position buffer
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cy::Vec3f) * size, &(m_buffers->m_verts[0]), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    glGenBuffers(1, &m_VBO[1]);   //normals buffer
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cy::Vec3f) * size, &(m_buffers->m_norms[0]), GL_STATIC_DRAW);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(1);

    glGenBuffers(1, &m_VBO[2]);   //texCoords buffer
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO[2]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cy::Vec2f) * size, &(m_buffers->m_texts[0]), GL_STATIC_DRAW);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(2);

    // for completeness' sake, let's unbind the vbo and vao
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void Model::createElementBuffer()
{
    createVertexArray();

    m_elementsLength = m_buffers->m_elements.size();

    glGenBuffers(1, &m_eBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_eBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * m_elementsLength, &(m_buffers->m_elements[0]), GL_STATIC_DRAW);
}

void Model::SetVerts(std::vector<float> vertData)
{
    m_buffers->m_elements.clear();
    unsigned int count = 0;
    for (size_t i = 0; i < vertData.size(); i += 3) {
        if (i + 1 > vertData.size() || i + 2 > vertData.size()) {
            m_buffers->m_verts.clear();
            return;
        }
        m_buffers->m_verts.push_back(cy::Vec3(vertData[i], vertData[i + 1], vertData[i + 2]));
        m_buffers->m_elements.push_back(count);
        count++;
    }
}
void Model::SetNorms(std::vector<float> normData)
{
    for (size_t i = 0; i < normData.size(); i += 3) {
        if (i + 1 > normData.size() || i + 2 > normData.size()) {
            m_buffers->m_norms.clear();
            return;
        }
        m_buffers->m_norms.push_back(cy::Vec3(normData[i], normData[i + 1], normData[i + 2]));
    }
}
void Model::SetTexts(std::vector<float> textData)
{
    for (size_t i = 0; i < textData.size(); i += 2) {
        m_buffers->m_texts.push_back(cy::Vec2(textData[i], textData[i + 1]));
    }
}

void Model::LoadEnvMap(const char* folder, const char** filenames)
{
    const path parent = path(folder);

    m_envTex.Initialize();
    for (int i = 0; i < 6; i++) {
        //load image from file
        const path imgPath = parent / path(filenames[i]);

        std::vector<unsigned char> png;
        std::vector<unsigned char> image;
        unsigned width, height;

        //load and decode
        unsigned error = lodepng::load_file(png, imgPath.string());
        if (!error) error = lodepng::decode(image, width, height, png);
        if (error) std::cout << "decoder error " << error << ": " << lodepng_error_text(error) << std::endl;

        //set image
        m_envTex.SetImageRGBA((cy::GLTextureCubeMap::Side)i, &image[0], width, height);
    }

    m_envTex.BuildMipmaps();
    m_envTex.SetSeamless();
}

void Model::SetTranslation(float x, float y, float z)
{
    m_xPos = x;
    m_yPos = y;
    m_zPos = z;
}

void Model::UpdateRotation(float x, float y, float z)
{
    m_xRot += x;
    m_yRot += y;
    m_zRot += z;
}
