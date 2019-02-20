#include "Model.h"

Model::Model(std::string path, bool gammaCorrection) : gammaCorrection(gammaCorrection)
{
	this->LoadModel(path);
}

Model::~Model()
{
	// Delete loaded textures
	for (Texture* t : this->loadedTextures)
	{
		delete t;
	}

	// Delete all the meshes
	for (Mesh* m : this->meshes)
	{
		delete m;
	}
}

void Model::Draw(Shader* shader)
{
	for (Mesh* m : this->meshes)
	{
		m->BindTextures(shader);
		m->Bind();
		m->Draw();
	}
}

void Model::LoadModel(std::string path)
{
	Assimp::Importer importer;
	// aiProcess_Triangulate makes sure that all verticies are connected as triangles
	// aiProcess_GenNormals generate normals if they are missing
	// aiProcess_CalcTangentSpace calculates normals in tangent space
	const aiScene *scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenUVCoords | aiProcess_GenNormals | aiProcess_CalcTangentSpace | aiProcess_FlipUVs);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		std::cout << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
		return;
	}
	this->directoryPath = path.substr(0, path.find_last_of('/'));

	ProcessNode(scene->mRootNode, scene);
}

void Model::ProcessNode(aiNode * node, const aiScene * scene)
{
	// Go through all meshes and fill array of them
	for (unsigned int i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
		meshes.push_back(ProcessMesh(mesh, scene));
	}
	// Then go through children
	for (unsigned int i = 0; i < node->mNumChildren; i++)
	{
		ProcessNode(node->mChildren[i], scene);
	}
}

Mesh * Model::ProcessMesh(aiMesh * mesh, const aiScene * scene)
{
	// Temp variables
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	std::vector<Texture*> textures;

	// Get Vertex Attributes
	for (unsigned int i = 0; i < mesh->mNumVertices; i++)
	{
		// Temp storage
		Vertex vertex;

		// Position
		glm::vec3 vector;
		vector.x = mesh->mVertices[i].x;
		vector.y = mesh->mVertices[i].y;
		vector.z = mesh->mVertices[i].z;
		vertex.Position = vector;

		// Texture
		if (mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
		{
			glm::vec2 vec;
			vec.x = mesh->mTextureCoords[0][i].x;
			vec.y = mesh->mTextureCoords[0][i].y;
			vertex.UVCoords = vec;
		}
		else
			vertex.UVCoords = glm::vec2(0.0f, 0.0f);

		// Normal
		vector.x = mesh->mNormals[i].x;
		vector.y = mesh->mNormals[i].y;
		vector.z = mesh->mNormals[i].z;
		vertex.Normal = vector;

		// Tangent IF crash here, probably dont got uv coords on model
		vector.x = mesh->mTangents[i].x;
		vector.y = mesh->mTangents[i].y;
		vector.z = mesh->mTangents[i].z;
		vertex.Tangent = vector;

		// Bitangent
		vector.x = mesh->mBitangents[i].x;
		vector.y = mesh->mBitangents[i].y;
		vector.z = mesh->mBitangents[i].z;
		vertex.Bitangent = vector;

		vertices.push_back(vertex);
	}

	// Get Indices
	for (unsigned int i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];
		for (unsigned int j = 0; j < face.mNumIndices; j++)
			indices.push_back(face.mIndices[j]);
	}

	// Get Material
	if (mesh->mMaterialIndex >= 0)
	{
		aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
		// Load all texture types
		std::vector<Texture*> diffuseMaps = LoadMaterialTextures(material, aiTextureType_DIFFUSE, "TextureDiffuse");
		std::vector<Texture*> ambientMaps = LoadMaterialTextures(material, aiTextureType_AMBIENT, "TextureAmbient");
		std::vector<Texture*> specularMaps = LoadMaterialTextures(material, aiTextureType_SPECULAR, "TextureSpecular");
		std::vector<Texture*> normalMaps = LoadMaterialTextures(material, aiTextureType_HEIGHT, "TextureNormal");
		//std::vector<Texture*> heightMaps = LoadMaterialTextures(material, aiTextureType_HEIGHT, "TextureHeight");

		// If they dont have a specific texture, add default // TODO: Don't load default textures to all models
		if (diffuseMaps.size() == 0)
			textures.push_back(this->LoadTexture("Textures/default_diffuse.png", "TextureDiffuse"));
		if (ambientMaps.size() == 0)
			textures.push_back(this->LoadTexture("Textures/default_ambient.png", "TextureAmbient"));
		if (specularMaps.size() == 0)
			textures.push_back(this->LoadTexture("Textures/default_specular.png", "TextureSpecular"));
		if (normalMaps.size() == 0)
			textures.push_back(this->LoadTexture("Textures/default_normal.png", "TextureNormal"));

		// TODO: Fix heightmap
		//if (heightMaps.size() == 0)
		//	textures.push_back(new Texture("Textures/default_height.png", "TextureHeight"));

		// Combine them
		textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
		textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
		textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
		textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
		//textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());
	}
	else
	{
		// Add default textures to it
		textures.push_back(this->LoadTexture("Textures/default_diffuse.png", "TextureDiffuse"));
		textures.push_back(this->LoadTexture("Textures/default_normal.png", "TextureNormal"));
		textures.push_back(this->LoadTexture("Textures/default_specular.png", "TextureSpecular"));
		textures.push_back(this->LoadTexture("Textures/default_ambient.png", "TextureAmbient"));
		textures.push_back(this->LoadTexture("Textures/default_height.png", "TextureHeight"));
	}

	Mesh* temp = new Mesh(vertices, indices, textures);
	return temp;
}

std::vector<Texture*> Model::LoadMaterialTextures(aiMaterial * mat, aiTextureType type, std::string typeName)
{
	std::vector<Texture*> textures;
	for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
	{
		aiString str;
		mat->GetTexture(type, i, &str); // Get texture name
		textures.push_back(this->LoadTexture((this->directoryPath + '/' + str.C_Str()).c_str(), typeName));
	}
	return textures;
}

Texture* Model::LoadTexture(const char* path, std::string type)
{
	bool skipLoad = false;
	for (unsigned int j = 0; j < this->loadedTextures.size(); j++)
	{
		// Remove the directory path of the texture
		std::string loadedTexturePath = this->loadedTextures[j]->GetPath();
		// Check if texture is already loaded
		if (std::strcmp(loadedTexturePath.data(), path) == 0)
		{
			// Push the loaded texture in to the meshs texture
			return this->loadedTextures[j];
			skipLoad = true;
			break;
		}
	}
	if (!skipLoad)
	{
		// If texture hasn't been loaded already, load it
		Texture* texture = new Texture(path, type, this->gammaCorrection);
		this->loadedTextures.push_back(texture); // Add to loaded textures
		return texture;
	}
}