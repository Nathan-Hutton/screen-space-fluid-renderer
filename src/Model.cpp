#include "Model.h"
#include <filesystem>
#include <stdlib.h>
#include <linux/limits.h>

using path = std::filesystem::path;

Model::Model() {
	xPos = 0;
	yPos = 0;
	zPos = 0;
    xRot = 0;
    yRot = 0;
    zRot = 0;
	modelTrans = cy::Matrix4f();

	mesh = NULL;
	buffers = new MeshBuffers();
    objFilename = NULL;
    program = new cy::GLSLProgram();
}

Model::~Model() {
    if (mesh) {
        delete mesh;
    }
    if (buffers) {
        delete buffers;
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(3, VBO);
    glDeleteBuffers(1, &eBuffer);
}

void Model::Initialize()
{
    //CompileShaders();
    createElementBuffer();
}

void Model::Bind()
{
    // bind our VAO
    glBindVertexArray(VAO);

    // bind the element buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eBuffer);
}

void Model::Unbind()
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
        printf("realpath failed: No such file or directory: %s\n", vertFile);
    if (realpath(fragFile, absFrag) == nullptr)
        printf("realpath failed: No such file or directory: %s\n", vertFile);

    std::cout << absVert << std::endl;
    std::cout << absFrag << std::endl;

    bool compiled = program->BuildFiles(absVert, absFrag);

    if (!compiled) {
        fprintf(stderr, "Error: could not compile shaders\n");
    } else {
        fprintf(stdout, "Shaders compiled!\n");
    }
}

void Model::calcTrans() {
    cy::Matrix4f rotations = cy::Matrix4f();
    rotations.SetRotationXYZ(xRot, yRot, zRot);

    cy::Matrix4f translations = cy::Matrix4f();
    translations.SetTranslation(cy::Vec3f(xPos, yPos, zPos));

    modelTrans = rotations * translations;
}

void Model::LoadOBJFile(char const* filename) {
    mesh = new cy::TriMesh();

    char objPath[PATH_MAX];
    if (realpath(filename, objPath) == nullptr)
        printf("realpath failed: No such file or directory: %s\n", filename);

    bool loadedMesh = mesh->LoadFromFileObj(objPath, true);
    if (!loadedMesh) {
        fprintf(stderr, "Error: '%s'\n", "unable to load obj file");
        mesh = NULL;
        return;
    }
    fprintf(stdout, "OBJ loaded. # of verticies: '%d'\n", mesh->NV());

    objFilename = filename;

    prepareBuffers();
    DifTexSetup();
    SpcTexSetup();
}

void Model::prepareBuffers()
{
    int faceCount = mesh->NF();

    buffers = new MeshBuffers();

    unsigned int counter = 0;

    // for each face
    for (int i = 0; i < faceCount; i++) {
        cy::TriMesh::TriFace faceVerts = mesh->F(i);
        cy::TriMesh::TriFace faceNorms = mesh->FN(i);
        cy::TriMesh::TriFace faceTexts = mesh->FT(i);

        //for each vertex (we know its always 3)
        for (int j = 0; j < 3; j++) {
            unsigned int vertIdx = faceVerts.v[j];
            unsigned int normIdx = faceNorms.v[j];
            unsigned int textIdx = faceTexts.v[j];

            buffers->verts.push_back(mesh->V(vertIdx));
            buffers->norms.push_back(mesh->VN(normIdx));
            buffers->texts.push_back(cy::Vec2f(mesh->VT(textIdx)));
            buffers->elements.push_back(counter++);
        }
    }

    //return myBuffs;
}

void Model::CenterModel() {
    if (!mesh) {
        return;
    }

    // get bounding box data
    mesh->ComputeBoundingBox();
    cy::Vec3f boxMin = mesh->GetBoundMin();
    cy::Vec3f boxMax = mesh->GetBoundMax();

    xPos = -(boxMax[0] + boxMin[0]) / 2;
    yPos = -(boxMax[1] + boxMin[1]) / 2;
    zPos = -(boxMax[2] + boxMin[2]) / 2;
}

void Model::DifTexSetup() {
    if (!mesh) {
        return;
    }

    // get texture data
    // assuming one material for now bc i'm lazy
    if (mesh->NM() == 0) return;
    cy::TriMesh::Mtl mat = mesh->M(0);

    cy::TriMesh::Str difMap = mat.map_Kd;
    if (!difMap) {
        return;
    }

    path parent = path(objFilename).parent_path();
    path difPath = parent / path(difMap.data);

    // setting up the diffuse texture
    std::vector<unsigned char> png;
    std::vector<unsigned char> image; //the raw pixels
    unsigned width, height;

    //load and decode
    unsigned error = lodepng::load_file(png, difPath.string());
    if (!error) error = lodepng::decode(image, width, height, png);
    //if there's an error, display it
    if (error) std::cout << "decoder error " << error << ": " << lodepng_error_text(error) << std::endl;

    difTex.Initialize();
    difTex.SetImage(&image[0], 4, width, height);
    difTex.BuildMipmaps();
    difTex.SetFilteringMode(GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR);
    difTex.SetWrappingMode(GL_REPEAT, GL_REPEAT);
}

void Model::SpcTexSetup() {
    if (!mesh) {
        return;
    }

    // get texture data
    // assuming one material for now bc i'm lazy
    if (mesh->NM() == 0) return;
    cy::TriMesh::Mtl mat = mesh->M(0);

    cy::TriMesh::Str spcMap = mat.map_Ks;
    if (!spcMap) {
        return;
    }

    path parent = path(objFilename).parent_path();
    path spcPath = parent / path(spcMap.data);

    std::vector<unsigned char> png;
    std::vector<unsigned char> image;
    unsigned width, height;

    //load and decode
    unsigned error = lodepng::load_file(png, spcPath.string());
    if (!error) error = lodepng::decode(image, width, height, png);
    if (error) std::cout << "decoder error " << error << ": " << lodepng_error_text(error) << std::endl;

    spcTex.Initialize();
    spcTex.SetImage(&image[0], 4, width, height);
    spcTex.BuildMipmaps();
    spcTex.SetFilteringMode(GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR);
    spcTex.SetWrappingMode(GL_REPEAT, GL_REPEAT);
}

void Model::createVertexArray()
{
    int size = buffers->verts.size();
    // initialize the vao
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    // initializing the vbos
    glGenBuffers(1, &VBO[0]);   //position buffer
    glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cy::Vec3f) * size, &(buffers->verts[0]), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    glGenBuffers(1, &VBO[1]);   //normals buffer
    glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cy::Vec3f) * size, &(buffers->norms[0]), GL_STATIC_DRAW);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(1);

    glGenBuffers(1, &VBO[2]);   //texCoords buffer
    glBindBuffer(GL_ARRAY_BUFFER, VBO[2]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cy::Vec2f) * size, &(buffers->texts[0]), GL_STATIC_DRAW);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(2);

    // for completeness' sake, let's unbind the vbo and vao
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void Model::createElementBuffer()
{
    createVertexArray();

    elementsLength = buffers->elements.size();

    glGenBuffers(1, &eBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * elementsLength, &(buffers->elements[0]), GL_STATIC_DRAW);
}

void Model::SetVerts(std::vector<float> vertData)
{
    buffers->elements.clear();
    unsigned int count = 0;
    for (size_t i = 0; i < vertData.size(); i += 3) {
        if (i + 1 > vertData.size() || i + 2 > vertData.size()) {
            std::cout << "nuh-uh" << std::endl;
            buffers->verts.clear();
            return;
        }
        buffers->verts.push_back(cy::Vec3(vertData[i], vertData[i + 1], vertData[i + 2]));
        buffers->elements.push_back(count);
        count++;
    }
}
void Model::SetNorms(std::vector<float> normData)
{
    for (size_t i = 0; i < normData.size(); i += 3) {
        if (i + 1 > normData.size() || i + 2 > normData.size()) {
            std::cout << "nuh-uh" << std::endl;
            buffers->norms.clear();
            return;
        }
        buffers->norms.push_back(cy::Vec3(normData[i], normData[i + 1], normData[i + 2]));
    }
}
void Model::SetTexts(std::vector<float> textData)
{
    for (size_t i = 0; i < textData.size(); i += 2) {
        buffers->texts.push_back(cy::Vec2(textData[i], textData[i + 1]));
    }
}

void Model::LoadEnvMap(const char* folder, const char** filenames)
{
    path parent = path(folder);

    envTex.Initialize();
    for (int i = 0; i < 6; i++) {
        //load image from file
        path imgPath = parent / path(filenames[i]);

        std::vector<unsigned char> png;
        std::vector<unsigned char> image;
        unsigned width, height;

        //load and decode
        unsigned error = lodepng::load_file(png, imgPath.string());
        if (!error) error = lodepng::decode(image, width, height, png);
        if (error) std::cout << "decoder error " << error << ": " << lodepng_error_text(error) << std::endl;

        //set image
        envTex.SetImageRGBA((cy::GLTextureCubeMap::Side)i, &image[0], width, height);
    }

    envTex.BuildMipmaps();
    envTex.SetSeamless();
}

void Model::SetTranslation(float x, float y, float z)
{
    xPos = x;
    yPos = y;
    zPos = z;
}

void Model::UpdateRotation(float x, float y, float z)
{
    xRot += x;
    yRot += y;
    zRot += z;
}
