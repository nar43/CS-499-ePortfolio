// SceneManager.cpp (enhanced)
// Fixes:
//  - DestroyGLTextures used glGenTextures mistakenly — replaced with glDeleteTextures
//  - Prevent materials from being appended every frame (m_objectMaterials now initialized once)
//  - Added bounds checking for texture creation and loading
//  - Added additional logging and comments for maintainability

#include "SceneManager.h"

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#endif

#include <glm/gtx/transform.hpp>
#include <iostream>

namespace
{
    const char* g_ModelName = "model";
    const char* g_ColorValueName = "objectColor";
    const char* g_TextureValueName = "objectTexture";
    const char* g_UseTextureName = "bUseTexture";
    const char* g_UseLightingName = "bUseLighting";
    const int MAX_TEXTURE_SLOTS = 16;
}

/* Constructor */
SceneManager::SceneManager(ShaderManager* pShaderManager)
    : m_pShaderManager(pShaderManager),
    m_basicMeshes(new ShapeMeshes()),
    m_loadedTextures(0)
{
    // initialize texture index array
    for (int i = 0; i < MAX_TEXTURE_SLOTS; ++i) {
        m_textureIDs[i].tag = "/0";
        m_textureIDs[i].ID = -1;
    }

    // Setup default materials once
    OBJECT_MATERIAL glassMaterial;
    glassMaterial.ambientColor = glm::vec3(0.4f, 0.4f, 0.4f);
    glassMaterial.ambientStrength = 0.3f;
    glassMaterial.diffuseColor = glm::vec3(0.3f, 0.3f, 0.3f);
    glassMaterial.specularColor = glm::vec3(0.6f, 0.6f, 0.6f);
    glassMaterial.shininess = 85.0;
    glassMaterial.tag = "glass";

    OBJECT_MATERIAL backdropMaterial;
    backdropMaterial.ambientColor = glm::vec3(0.6f, 0.6f, 0.6f);
    backdropMaterial.ambientStrength = 0.6f;
    backdropMaterial.diffuseColor = glm::vec3(0.6f, 0.5f, 0.1f);
    backdropMaterial.specularColor = glm::vec3(0.0f);
    backdropMaterial.shininess = 0.0;
    backdropMaterial.tag = "backdrop";

    // push once
    m_objectMaterials.push_back(glassMaterial);
    m_objectMaterials.push_back(backdropMaterial);
}

/* Destructor */
SceneManager::~SceneManager()
{
    m_pShaderManager = nullptr;
    delete m_basicMeshes;
    m_basicMeshes = nullptr;

    DestroyGLTextures();
}

/* CreateGLTexture: loads image -> creates OpenGL texture -> stores into slot */
bool SceneManager::CreateGLTexture(const char* filename, std::string tag)
{
    if (m_loadedTextures >= MAX_TEXTURE_SLOTS) {
        std::cerr << "ERROR: Max texture slots reached (" << MAX_TEXTURE_SLOTS << ")" << std::endl;
        return false;
    }
    int width = 0, height = 0, channels = 0;
    stbi_set_flip_vertically_on_load(true);

    unsigned char* image = stbi_load(filename, &width, &height, &channels, 0);
    if (!image) {
        std::cerr << "ERROR: Could not load image: " << filename << std::endl;
        return false;
    }

    GLuint textureID = 0;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    // Parameter setup
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    if (channels == 3) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
    }
    else if (channels == 4) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    }
    else {
        std::cerr << "ERROR: Unsupported channels (" << channels << ") in " << filename << std::endl;
        stbi_image_free(image);
        glBindTexture(GL_TEXTURE_2D, 0);
        glDeleteTextures(1, &textureID);
        return false;
    }

    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(image);
    glBindTexture(GL_TEXTURE_2D, 0);

    // register
    m_textureIDs[m_loadedTextures].ID = textureID;
    m_textureIDs[m_loadedTextures].tag = tag;
    ++m_loadedTextures;

    std::cout << "Loaded texture '" << tag << "' (" << filename << ") into slot " << (m_loadedTextures - 1) << std::endl;
    return true;
}

/* Bind all currently loaded textures to texture units (0..N-1) */
void SceneManager::BindGLTextures()
{
    for (int i = 0; i < m_loadedTextures; ++i) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, m_textureIDs[i].ID);
    }
}

/* Properly delete textures (was using glGenTextures incorrectly) */
void SceneManager::DestroyGLTextures()
{
    if (m_loadedTextures <= 0) return;
    std::vector<GLuint> ids;
    ids.reserve(m_loadedTextures);
    for (int i = 0; i < m_loadedTextures; ++i) {
        if (m_textureIDs[i].ID > 0) ids.push_back((GLuint)m_textureIDs[i].ID);
    }
    if (!ids.empty()) {
        glDeleteTextures(static_cast<GLsizei>(ids.size()), ids.data());
    }
    m_loadedTextures = 0;
    for (int i = 0; i < MAX_TEXTURE_SLOTS; ++i) {
        m_textureIDs[i].ID = -1;
        m_textureIDs[i].tag = "/0";
    }
}

/* Find texture ID by tag, returns -1 if not found */
int SceneManager::FindTextureID(std::string tag)
{
    for (int i = 0; i < m_loadedTextures; ++i) {
        if (m_textureIDs[i].tag == tag) return m_textureIDs[i].ID;
    }
    return -1;
}

/* Find texture slot index by tag, returns -1 if not found */
int SceneManager::FindTextureSlot(std::string tag)
{
    for (int i = 0; i < m_loadedTextures; ++i) {
        if (m_textureIDs[i].tag == tag) return i;
    }
    return -1;
}

/* Find material by tag (copies into 'material'), returns true if found */
bool SceneManager::FindMaterial(std::string tag, OBJECT_MATERIAL& material)
{
    for (const auto& m : m_objectMaterials) {
        if (m.tag == tag) {
            material = m;
            return true;
        }
    }
    return false;
}

/* Set transformations & update shader model matrix */
void SceneManager::SetTransformations(glm::vec3 scaleXYZ,
    float XrotationDegrees, float YrotationDegrees, float ZrotationDegrees,
    glm::vec3 positionXYZ)
{
    glm::mat4 model = glm::translate(positionXYZ)
        * glm::rotate(glm::radians(XrotationDegrees), glm::vec3(1.0f, 0.0f, 0.0f))
        * glm::rotate(glm::radians(YrotationDegrees), glm::vec3(0.0f, 1.0f, 0.0f))
        * glm::rotate(glm::radians(ZrotationDegrees), glm::vec3(0.0f, 0.0f, 1.0f))
        * glm::scale(scaleXYZ);

    if (m_pShaderManager) m_pShaderManager->setMat4Value(g_ModelName, model);
}

/* Set a solid color (disables textures in shader) */
void SceneManager::SetShaderColor(float r, float g, float b, float a)
{
    glm::vec4 color(r, g, b, a);
    if (m_pShaderManager) {
        m_pShaderManager->setIntValue(g_UseTextureName, false);
        m_pShaderManager->setVec4Value(g_ColorValueName, color);
    }
}

/* Set texture by tag (tells shader which sampler index to use) */
void SceneManager::SetShaderTexture(std::string textureTag)
{
    if (!m_pShaderManager) return;
    int slot = FindTextureSlot(textureTag);
    if (slot < 0) {
        std::cerr << "WARNING: texture '" << textureTag << "' not found; using unit 0" << std::endl;
        slot = 0;
    }
    m_pShaderManager->setIntValue(g_UseTextureName, true);
    m_pShaderManager->setSampler2DValue(g_TextureValueName, slot);
}

/* Set texture UV scale */
void SceneManager::SetTextureUVScale(float u, float v)
{
    if (m_pShaderManager) m_pShaderManager->setVec2Value("UVscale", glm::vec2(u, v));
}

/* Set material parameters by tag (if found) */
void SceneManager::SetShaderMaterial(std::string materialTag)
{
    OBJECT_MATERIAL mat;
    if (FindMaterial(materialTag, mat)) {
        if (m_pShaderManager) {
            m_pShaderManager->setVec3Value("material.ambientColor", mat.ambientColor);
            m_pShaderManager->setFloatValue("material.ambientStrength", mat.ambientStrength);
            m_pShaderManager->setVec3Value("material.diffuseColor", mat.diffuseColor);
            m_pShaderManager->setVec3Value("material.specularColor", mat.specularColor);
            m_pShaderManager->setFloatValue("material.shininess", mat.shininess);
        }
    }
    else {
        // fallback or no-op
    }
}

/* PrepareScene: load meshes & textures (called once at initialization) */
void SceneManager::PrepareScene()
{
    // load base meshes
    m_basicMeshes->LoadPlaneMesh();
    m_basicMeshes->LoadConeMesh();
    m_basicMeshes->LoadCylinderMesh();
    m_basicMeshes->LoadBoxMesh();
    m_basicMeshes->LoadTorusMesh(); // changed DrawTorusMesh() to LoadTorusMesh() if available

    // create textures (check return values)
    CreateGLTexture("../../Utilities/textures/knife_handle.jpg", "tabletop");
    CreateGLTexture("../../Utilities/textures/abstract.jpg", "lampshade");
    CreateGLTexture("../../Utilities/textures/tilesf2.jpg", "lampbase");
    CreateGLTexture("../../Utilities/textures/backdrop.jpg", "background");
    CreateGLTexture("../../Utilities/textures/cheese_wheel.jpg", "book");
    CreateGLTexture("../../Utilities/textures/gold-seamless-texture.jpg", "cup");
    CreateGLTexture("../../Utilities/textures/stainless.jpg", "laptopscreen");

    // bind to GPU units
    BindGLTextures();
}

/* RenderScene: apply transforms + draw meshes
   Important: don't modify container sizes here; only perform rendering actions. */
void SceneManager::RenderScene()
{
    // Activate lighting in shader
    if (m_pShaderManager) {
        m_pShaderManager->setBoolValue(g_UseLightingName, true);
        m_pShaderManager->setVec3Value("lightSources[0].position", -3.0f, 4.0f, 6.0f);
        m_pShaderManager->setVec3Value("lightSources[0].ambientColor", 0.01f, 0.01f, 0.01f);
        m_pShaderManager->setVec3Value("lightSources[0].diffuseColor", 0.5f, 0.5f, 0.5f);
        m_pShaderManager->setVec3Value("lightSources[0].specularColor", 0.2f, 0.2f, 0.2f);
        m_pShaderManager->setFloatValue("lightSources[0].focalStrength", 32.0f);
        m_pShaderManager->setFloatValue("lightSources[0].specularIntensity", 0.2f);
    }

    // Use local variables for transforms
    glm::vec3 scaleXYZ;
    float XrotationDegrees = 0.0f, YrotationDegrees = 0.0f, ZrotationDegrees = 0.0f;
    glm::vec3 positionXYZ;

    // Draw tabletop
    scaleXYZ = glm::vec3(5.0f, 0.2f, 2.0f);
    float tabletopHeight = 10.0f;
    positionXYZ = glm::vec3(0.0f, tabletopHeight / 2.0f, 5.0f);
    SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
    SetShaderTexture("tabletop");
    SetTextureUVScale(1.0f, 1.0f);
    SetShaderMaterial("glass");
    m_basicMeshes->DrawBoxMesh();

    // Lamp base
    scaleXYZ = glm::vec3(0.3f, 0.05f, 0.3f);
    positionXYZ = glm::vec3(-2.0f, tabletopHeight / 2.0f + 0.15f, 5.0f);
    SetTransformations(scaleXYZ, 0.0f, 0.0f, 0.0f, positionXYZ);
    SetShaderTexture("lampbase");
    SetTextureUVScale(1.0f, 1.0f);
    m_basicMeshes->DrawBoxMesh();

    // Lamp pole
    scaleXYZ = glm::vec3(0.05f, 0.5f, 0.05f);
    positionXYZ = glm::vec3(-2.0f, tabletopHeight / 2.0f + 0.2f, 5.0f);
    SetTransformations(scaleXYZ, 0.0f, 0.0f, 0.0f, positionXYZ);
    SetShaderColor(0.4f, 0.4f, 0.4f, 1.0f);
    m_basicMeshes->DrawCylinderMesh();

    // Lamp shade
    scaleXYZ = glm::vec3(0.3f, 0.1f, 0.3f);
    positionXYZ = glm::vec3(-2.0f, tabletopHeight / 2.0f + 0.6f, 5.0f);
    SetTransformations(scaleXYZ, 0.0f, 0.0f, 0.0f, positionXYZ);
    SetShaderTexture("lampshade");
    SetTextureUVScale(1.0f, 1.0f);
    m_basicMeshes->DrawConeMesh();

    // Cup
    scaleXYZ = glm::vec3(0.2f, 0.5f, 0.2f);
    positionXYZ = glm::vec3(2.0f, tabletopHeight / 2.0f - 0.1f, 5.0f);
    SetTransformations(scaleXYZ, 0.0f, 0.0f, 0.0f, positionXYZ);
    SetShaderTexture("cup");
    m_basicMeshes->DrawCylinderMesh();

    // Laptop base
    scaleXYZ = glm::vec3(1.2f, 0.1f, 0.8f);
    positionXYZ = glm::vec3(0.0f, tabletopHeight / 2.0f + 0.15f, 5.0f);
    SetTransformations(scaleXYZ, 0.0f, 0.0f, 0.0f, positionXYZ);
    SetShaderColor(0.2f, 0.2f, 0.2f, 1.0f);
    m_basicMeshes->DrawBoxMesh();

    // Laptop screen
    scaleXYZ = glm::vec3(1.2f, 0.4f, 0.05f);
    positionXYZ = glm::vec3(0.0f, tabletopHeight / 2.0f + 0.35f, 4.7f);
    SetTransformations(scaleXYZ, 30.0f, 0.0f, 0.0f, positionXYZ);
    SetShaderTexture("laptopscreen");
    SetTextureUVScale(1.0f, 1.0f);
    m_basicMeshes->DrawBoxMesh();

    // Two books
    scaleXYZ = glm::vec3(0.5f, 0.1f, 0.3f);
    positionXYZ = glm::vec3(-1.5f, tabletopHeight / 2.0f + 0.15f, 5.1f);
    SetTransformations(scaleXYZ, 0.0f, 0.0f, 0.0f, positionXYZ);
    SetShaderTexture("book");
    m_basicMeshes->DrawBoxMesh();

    positionXYZ = glm::vec3(-1.5f, tabletopHeight / 2.0f + 0.25f, 5.1f);
    SetTransformations(scaleXYZ, 0.0f, 0.0f, 0.0f, positionXYZ);
    SetShaderTexture("book");
    m_basicMeshes->DrawBoxMesh();

    // Backdrop plane
    scaleXYZ = glm::vec3(20.0f, 1.0f, 20.0f);
    XrotationDegrees = 90.0f;
    positionXYZ = glm::vec3(0.0f, 15.0f, -8.0f);
    SetTransformations(scaleXYZ, XrotationDegrees, 0.0f, 0.0f, positionXYZ);
    SetShaderTexture("background");
    SetTextureUVScale(1.0f, 1.0f);
    SetShaderMaterial("backdrop");
    m_basicMeshes->DrawPlaneMesh();
}
