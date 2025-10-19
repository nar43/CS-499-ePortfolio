///////////////////////////////////////////////////////////////////////////////
// shadermanager.cpp
// ============
// manage the loading and rendering of 3D scenes
//
//  AUTHOR: Brian Battersby - SNHU Instructor / Computer Science
//	Created for CS-330-Computational Graphics and Visualization, Nov. 1st, 2023
///////////////////////////////////////////////////////////////////////////////

#include "SceneManager.h"

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#endif

#include <glm/gtx/transform.hpp>

// declaration of global variables
namespace
{
	const char* g_ModelName = "model";
	const char* g_ColorValueName = "objectColor";
	const char* g_TextureValueName = "objectTexture";
	const char* g_UseTextureName = "bUseTexture";
	const char* g_UseLightingName = "bUseLighting";
}

/***********************************************************
 *  SceneManager()
 *
 *  The constructor for the class
 ***********************************************************/
SceneManager::SceneManager(ShaderManager *pShaderManager)
{
	m_pShaderManager = pShaderManager;
	m_basicMeshes = new ShapeMeshes();

	// initialize the texture collection
	for (int i = 0; i < 16; i++)
	{
		m_textureIDs[i].tag = "/0";
		m_textureIDs[i].ID = -1;
	}
	m_loadedTextures = 0;
}

/***********************************************************
 *  ~SceneManager()
 *
 *  The destructor for the class
 ***********************************************************/
SceneManager::~SceneManager()
{
	// clear the allocated memory
	m_pShaderManager = NULL;
	delete m_basicMeshes;
	m_basicMeshes = NULL;
	// destroy the created OpenGL textures
	DestroyGLTextures();
}

/***********************************************************
 *  CreateGLTexture()
 *
 *  This method is used for loading textures from image files,
 *  configuring the texture mapping parameters in OpenGL,
 *  generating the mipmaps, and loading the read texture into
 *  the next available texture slot in memory.
 ***********************************************************/
bool SceneManager::CreateGLTexture(const char* filename, std::string tag)
{
	int width = 0;
	int height = 0;
	int colorChannels = 0;
	GLuint textureID = 0;

	// indicate to always flip images vertically when loaded
	stbi_set_flip_vertically_on_load(true);

	// try to parse the image data from the specified image file
	unsigned char* image = stbi_load(
		filename,
		&width,
		&height,
		&colorChannels,
		0);

	// if the image was successfully read from the image file
	if (image)
	{
		std::cout << "Successfully loaded image:" << filename << ", width:" << width << ", height:" << height << ", channels:" << colorChannels << std::endl;

		glGenTextures(1, &textureID);
		glBindTexture(GL_TEXTURE_2D, textureID);

		// set the texture wrapping parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		// set texture filtering parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// if the loaded image is in RGB format
		if (colorChannels == 3)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
		// if the loaded image is in RGBA format - it supports transparency
		else if (colorChannels == 4)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
		else
		{
			std::cout << "Not implemented to handle image with " << colorChannels << " channels" << std::endl;
			return false;
		}

		// generate the texture mipmaps for mapping textures to lower resolutions
		glGenerateMipmap(GL_TEXTURE_2D);

		// free the image data from local memory
		stbi_image_free(image);
		glBindTexture(GL_TEXTURE_2D, 0); // Unbind the texture

		// register the loaded texture and associate it with the special tag string
		m_textureIDs[m_loadedTextures].ID = textureID;
		m_textureIDs[m_loadedTextures].tag = tag;
		m_loadedTextures++;

		return true;
	}

	std::cout << "Could not load image:" << filename << std::endl;

	// Error loading the image
	return false;
}

/***********************************************************
 *  BindGLTextures()
 *
 *  This method is used for binding the loaded textures to
 *  OpenGL texture memory slots.  There are up to 16 slots.
 ***********************************************************/
void SceneManager::BindGLTextures()
{
	for (int i = 0; i < m_loadedTextures; i++)
	{
		// bind textures on corresponding texture units
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, m_textureIDs[i].ID);
	}
}

/***********************************************************
 *  DestroyGLTextures()
 *
 *  This method is used for freeing the memory in all the
 *  used texture memory slots.
 ***********************************************************/
void SceneManager::DestroyGLTextures()
{
	for (int i = 0; i < m_loadedTextures; i++)
	{
		glGenTextures(1, &m_textureIDs[i].ID);
	}
}

/***********************************************************
 *  FindTextureID()
 *
 *  This method is used for getting an ID for the previously
 *  loaded texture bitmap associated with the passed in tag.
 ***********************************************************/
int SceneManager::FindTextureID(std::string tag)
{
	int textureID = -1;
	int index = 0;
	bool bFound = false;

	while ((index < m_loadedTextures) && (bFound == false))
	{
		if (m_textureIDs[index].tag.compare(tag) == 0)
		{
			textureID = m_textureIDs[index].ID;
			bFound = true;
		}
		else
			index++;
	}

	return(textureID);
}

/***********************************************************
 *  FindTextureSlot()
 *
 *  This method is used for getting a slot index for the previously
 *  loaded texture bitmap associated with the passed in tag.
 ***********************************************************/
int SceneManager::FindTextureSlot(std::string tag)
{
	int textureSlot = -1;
	int index = 0;
	bool bFound = false;

	while ((index < m_loadedTextures) && (bFound == false))
	{
		if (m_textureIDs[index].tag.compare(tag) == 0)
		{
			textureSlot = index;
			bFound = true;
		}
		else
			index++;
	}

	return(textureSlot);
}

/***********************************************************
 *  FindMaterial()
 *
 *  This method is used for getting a material from the previously
 *  defined materials list that is associated with the passed in tag.
 ***********************************************************/
bool SceneManager::FindMaterial(std::string tag, OBJECT_MATERIAL& material)
{
	if (m_objectMaterials.size() == 0)
	{
		return(false);
	}

	int index = 0;
	bool bFound = false;
	while ((index < m_objectMaterials.size()) && (bFound == false))
	{
		if (m_objectMaterials[index].tag.compare(tag) == 0)
		{
			bFound = true;
			material.ambientColor = m_objectMaterials[index].ambientColor;
			material.ambientStrength = m_objectMaterials[index].ambientStrength;
			material.diffuseColor = m_objectMaterials[index].diffuseColor;
			material.specularColor = m_objectMaterials[index].specularColor;
			material.shininess = m_objectMaterials[index].shininess;
		}
		else
		{
			index++;
		}
	}

	return(true);
}

/***********************************************************
 *  SetTransformations()
 *
 *  This method is used for setting the transform buffer
 *  using the passed in transformation values.
 ***********************************************************/
void SceneManager::SetTransformations(
	glm::vec3 scaleXYZ,
	float XrotationDegrees,
	float YrotationDegrees,
	float ZrotationDegrees,
	glm::vec3 positionXYZ)
{
	// variables for this method
	glm::mat4 modelView;
	glm::mat4 scale;
	glm::mat4 rotationX;
	glm::mat4 rotationY;
	glm::mat4 rotationZ;
	glm::mat4 translation;

	// set the scale value in the transform buffer
	scale = glm::scale(scaleXYZ);
	// set the rotation values in the transform buffer
	rotationX = glm::rotate(glm::radians(XrotationDegrees), glm::vec3(1.0f, 0.0f, 0.0f));
	rotationY = glm::rotate(glm::radians(YrotationDegrees), glm::vec3(0.0f, 1.0f, 0.0f));
	rotationZ = glm::rotate(glm::radians(ZrotationDegrees), glm::vec3(0.0f, 0.0f, 1.0f));
	// set the translation value in the transform buffer
	translation = glm::translate(positionXYZ);

	modelView = translation * rotationX * rotationY * rotationZ * scale;

	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setMat4Value(g_ModelName, modelView);
	}
}

/***********************************************************
 *  SetShaderColor()
 *
 *  This method is used for setting the passed in color
 *  into the shader for the next draw command
 ***********************************************************/
void SceneManager::SetShaderColor(
	float redColorValue,
	float greenColorValue,
	float blueColorValue,
	float alphaValue)
{
	// variables for this method
	glm::vec4 currentColor;

	currentColor.r = redColorValue;
	currentColor.g = greenColorValue;
	currentColor.b = blueColorValue;
	currentColor.a = alphaValue;

	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setIntValue(g_UseTextureName, false);
		m_pShaderManager->setVec4Value(g_ColorValueName, currentColor);
	}
}

/***********************************************************
 *  SetShaderTexture()
 *
 *  This method is used for setting the texture data
 *  associated with the passed in ID into the shader.
 ***********************************************************/
void SceneManager::SetShaderTexture(
	std::string textureTag)
{
	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setIntValue(g_UseTextureName, true);

		int textureID = -1;
		textureID = FindTextureSlot(textureTag);
		m_pShaderManager->setSampler2DValue(g_TextureValueName, textureID);
	}
}

/***********************************************************
 *  SetTextureUVScale()
 *
 *  This method is used for setting the texture UV scale
 *  values into the shader.
 ***********************************************************/
void SceneManager::SetTextureUVScale(float u, float v)
{
	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setVec2Value("UVscale", glm::vec2(u, v));
	}
}

/***********************************************************
 *  SetShaderMaterial()
 *
 *  This method is used for passing the material values
 *  into the shader.
 ***********************************************************/
void SceneManager::SetShaderMaterial(
	std::string materialTag)
{
	if (m_objectMaterials.size() > 0)
	{
		OBJECT_MATERIAL material;
		bool bReturn = false;

		bReturn = FindMaterial(materialTag, material);
		if (bReturn == true)
		{
			m_pShaderManager->setVec3Value("material.ambientColor", material.ambientColor);
			m_pShaderManager->setFloatValue("material.ambientStrength", material.ambientStrength);
			m_pShaderManager->setVec3Value("material.diffuseColor", material.diffuseColor);
			m_pShaderManager->setVec3Value("material.specularColor", material.specularColor);
			m_pShaderManager->setFloatValue("material.shininess", material.shininess);
		}
	}
}

/**************************************************************/
/*** STUDENTS CAN MODIFY the code in the methods BELOW for  ***/
/*** preparing and rendering their own 3D replicated scenes.***/
/*** Please refer to the code in the OpenGL sample project  ***/
/*** for assistance.                                        ***/
/**************************************************************/




/***********************************************************
 *  PrepareScene()
 *
 *  This method is used for preparing the 3D scene by loading
 *  the shapes, textures in memory to support the 3D scene 
 *  rendering
 ***********************************************************/
void SceneManager::PrepareScene()
{
	// only one instance of a particular mesh needs to be
	// loaded in memory no matter how many times it is drawn
	// in the rendered 3D scene

	m_basicMeshes->LoadPlaneMesh();
	m_basicMeshes->LoadConeMesh();
	m_basicMeshes->LoadCylinderMesh();
	m_basicMeshes->LoadBoxMesh();
	m_basicMeshes->DrawTorusMesh();

	CreateGLTexture("../../Utilities/textures/knife_handle.jpg","tabletop");
	CreateGLTexture("../../Utilities/textures/abstract.jpg", "lampshade");
	CreateGLTexture("../../Utilities/textures/tilesf2.jpg", "lampbase");
	CreateGLTexture("../../Utilities/textures/backdrop.jpg", "background");
	CreateGLTexture("../../Utilities/textures/cheese_wheel.jpg", "book");
	CreateGLTexture("../../Utilities/textures/gold-seamless-texture.jpg", "cup");
	CreateGLTexture("../../Utilities/textures/stainless.jpg", "laptopscreen");
	


	BindGLTextures();
}

/***********************************************************
 *  RenderScene()
 *
 *  This method is used for rendering the 3D scene by 
 *  transforming and drawing the basic 3D shapes
 ***********************************************************/
void SceneManager::RenderScene()
{
	// Define lighting properties
	OBJECT_MATERIAL glassMaterial;
	glassMaterial.ambientColor = glm::vec3(0.4f, 0.4f, 0.4f);
	glassMaterial.ambientStrength = 0.3f;
	glassMaterial.diffuseColor = glm::vec3(0.3f, 0.3f, 0.3f);
	glassMaterial.specularColor = glm::vec3(0.6f, 0.6f, 0.6f);
	glassMaterial.shininess = 85.0;
	glassMaterial.tag = "glass";

	m_objectMaterials.push_back(glassMaterial);

	OBJECT_MATERIAL backdropMaterial;
	backdropMaterial.ambientColor = glm::vec3(0.6f, 0.6f, 0.6f);
	backdropMaterial.ambientStrength = 0.6f;
	backdropMaterial.diffuseColor = glm::vec3(0.6f, 0.5f, 0.1f);
	backdropMaterial.specularColor = glm::vec3(0.0f, 0.0f, 0.0f);
	backdropMaterial.shininess = 0.0;
	backdropMaterial.tag = "backdrop";

	// Send lighting information to the shader
	m_pShaderManager->setBoolValue(g_UseLightingName, true);

	m_pShaderManager->setVec3Value("lightSources[0].position", -3.0f, 4.0f, 6.0f);
	m_pShaderManager->setVec3Value("lightSources[0].ambientColor", 0.01f, 0.01f, 0.01f);
	m_pShaderManager->setVec3Value("lightSources[0].diffuseColor", 0.5f, 0.5f, 0.5f);
	m_pShaderManager->setVec3Value("lightSources[0].specularColor", 0.2f, 0.2f, 0.2f);
	m_pShaderManager->setFloatValue("lightSources[0].focalStrength", 32.0f);
	m_pShaderManager->setFloatValue("lightSources[0].specularIntensity", 0.2f);

	// declare the variables for the transformations
	glm::vec3 scaleXYZ;
	float XrotationDegrees = 0.0f;
	float YrotationDegrees = 0.0f;
	float ZrotationDegrees = 0.0f;
	glm::vec3 positionXYZ;

	/*** Set needed transformations before drawing the basic mesh.  ***/
	/*** This same ordering of code should be used for transforming ***/
	/*** and drawing all the basic 3D shapes.						***/
	/******************************************************************/
	// Set the XYZ scale for the desk top
	scaleXYZ = glm::vec3(5.0f, 0.2f, 2.0f); // Thin tabletop

	// Set the XYZ rotation for the desk top
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// Set the height for the tabletop
	float tabletopHeight = 10.0f; // Height for the tabletop
	positionXYZ = glm::vec3(0.0f, tabletopHeight / 2.0f, 5.f); // Positioned above ground

	// Set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	//SetShaderColor(0.7f, 0.6f, 0.5f, 1.0f); // Light brown for the wood top
	SetShaderTexture("tabletop");
	SetTextureUVScale(1.0, 1.0);
	SetShaderMaterial("glass");
	m_basicMeshes->DrawBoxMesh(); // Draw the tabletop
	/******************************************************************/

	// Draw the lamp base
	scaleXYZ = glm::vec3(0.3f, 0.05f, 0.3f); // Small base
	positionXYZ = glm::vec3(-2.0f, tabletopHeight / 2.0f + 0.15f, 5.0f); // Positioned to the left above the tabletop

	// Set the transformations into memory for the lamp base
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);

	// Set the color for the lamp base to orange
	//SetShaderColor(1.0f, 0.5f, 0.0f, 1.0f); // Orange color for the base
	SetShaderTexture("lampbase");
	SetTextureUVScale(1.0, 1.0);
	m_basicMeshes->DrawBoxMesh(); // Draw the lamp base

	// Draw the lamp pole
	scaleXYZ = glm::vec3(0.05f, 0.5f, 0.05f); // Half the length of the pole
	positionXYZ = glm::vec3(-2.0f, tabletopHeight / 2.0f + 0.2f, 5.0f); // Positioned higher above the base
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderColor(0.4f, 0.4f, 0.4f, 1.0f); // Gray for the pole
	m_basicMeshes->DrawCylinderMesh(); // Draw the lamp pole

	// Draw the lamp shade
	scaleXYZ = glm::vec3(0.3f, 0.1f, 0.3f); // Wide shade
	positionXYZ = glm::vec3(-2.0f, tabletopHeight / 2.0f + 0.6f, 5.0f); // Positioned higher above the pole on the left
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	//SetShaderColor(1.0f, 1.0f, 0.8f, 1.0f); // Light color for the shade
	SetShaderTexture("lampshade");
	SetTextureUVScale(1.0, 1.0);
	m_basicMeshes->DrawConeMesh(); // Draw the lamp shade

	/******************************************************************/
	// Draw the cup on the tabletop

	// Set the XYZ scale for the pencil cup holder
	scaleXYZ = glm::vec3(0.2f, 0.5f, 0.2f); // Width, height, depth of the cup holder

	// Set the XYZ position for the pencil cup holder
	// Positioned above the tabletop
	positionXYZ = glm::vec3(2.0f, tabletopHeight / 2.0f - 0.1f, 5.0f); // Adjusted Y to be above the tabletop

	// Set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// Set the color for the pencil cup holder to blue
	//SetShaderColor(0.0f, 0.0f, 1.0f, 1.0f); // RGB for blue color
	SetShaderTexture("cup");
	SetTextureUVScale(1.0, 1.0);

	// Draw the mesh with transformation values
	m_basicMeshes->DrawCylinderMesh(); // Draw pencil cup holder
	/******************************************************************/
//  Draw the laptop base (body)
	scaleXYZ = glm::vec3(1.2f, 0.1f, 0.8f); // Laptop body dimensions
	positionXYZ = glm::vec3(0.0f, tabletopHeight / 2.0f + 0.15f, 5.0f); // Positioned above the tabletop

	// Set the transformations into memory for the laptop body
	SetTransformations(scaleXYZ, 0.0f, 0.0f, 0.0f, positionXYZ);

	// Set the color for the laptop body
	SetShaderColor(0.2f, 0.2f, 0.2f, 1.0f); // Dark gray for the laptop body
	m_basicMeshes->DrawBoxMesh(); // Draw the laptop body

	// Draw the laptop screen (half-closed)
	scaleXYZ = glm::vec3(1.2f, 0.4f, 0.05f); // Laptop screen dimensions
	positionXYZ = glm::vec3(0.0f, tabletopHeight / 2.0f + 0.35f, 4.7f); // Increased Y position for better alignment

	// Set the transformations into memory for the laptop screen
	SetTransformations(scaleXYZ, 30.0f, 0.0f, 0.0f, positionXYZ); // Tilted 30 degrees for half-closed effect

	// Set the color for the laptop screen
	//SetShaderColor(0.1f, 0.1f, 0.1f, 1.0f); // Dark gray for the screen
	SetShaderTexture("laptopscreen");
	SetTextureUVScale(1.0, 1.0);
	m_basicMeshes->DrawBoxMesh(); // Draw the laptop screen
	/******************************************************************/
	// Draw the first book (bottom)
	scaleXYZ = glm::vec3(0.5f, 0.1f, 0.3f); // Dimensions for the first book
	positionXYZ = glm::vec3(-1.5f, tabletopHeight / 2.0f + 0.15f, 5.1f); // Raised slightly higher

	// Set the transformations for the first book
	SetTransformations(scaleXYZ, 0.0f, 0.0f, 0.0f, positionXYZ);

	// Set the color for the first book
	//SetShaderColor(0.8f, 0.2f, 0.2f, 1.0f); // Red color for the first book
	SetShaderTexture("book");
	SetTextureUVScale(1.0, 1.0);
	m_basicMeshes->DrawBoxMesh(); // Draw the first book

	// Draw the second book (top)
	scaleXYZ = glm::vec3(0.5f, 0.1f, 0.3f); // Dimensions for the second book
	positionXYZ = glm::vec3(-1.5f, tabletopHeight / 2.0f + 0.25f, 5.1f); // Positioned directly above the first book

	// Set the transformations for the second book
	SetTransformations(scaleXYZ, 0.0f, 0.0f, 0.0f, positionXYZ);

	// Set the color for the second book
	//SetShaderColor(0.2f, 0.8f, 0.2f, 1.0f); // Green color for the second book
	SetShaderTexture("book");
	SetTextureUVScale(1.0, 1.0);
	m_basicMeshes->DrawBoxMesh(); // Draw the second book

	/******************************************************************/
	// set the XYZ scale for the mesh
	scaleXYZ = glm::vec3(20.0f, 1.0f, 20.0f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 90.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(0.0f, 15.0f, -8.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetShaderTexture("background");
	SetTextureUVScale(1.0, 1.0);
	SetShaderMaterial("backdrop");

	// draw the mesh with transformation values - this plane is used for the backdrop
	m_basicMeshes->DrawPlaneMesh();


}
