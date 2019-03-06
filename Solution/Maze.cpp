#include "Maze.h"

Maze::Maze()
{
	this->imageData = nullptr;
	this->path = "";
	this->width = 0;
	this->height = 0;
	this->numComponents = 0;
	this->mazeTexture = 0;

	this->mazeTbo = 0;
	this->mazeVbo = 0;
	this->mazeVao = 0;


	// Set maze position, rotation and scale
	this->transform.SetPos(glm::vec3(0, 0, 0));
	this->transform.SetRot(glm::vec3(0, 0, 0));
	this->transform.SetScale(glm::vec3(scaleXZ, scaleY, scaleXZ));

	// Load wall & floor texture
	this->LoadTextures();

	this->LoadMaze("MazePNG/mazeColorCoded.png");

	// Find the exit
	this->exitPos = this->FindExit();
	this->exitWorldPos = this->TransformToWorldCoords(glm::vec3(exitPos.x, 0, exitPos.y));

	this->InitiateMazeBuffers();

	this->GenerateDrawOrder();
}

Maze::~Maze()
{
	stbi_image_free(imageData);

	glDeleteBuffers(1, &this->mazeTbo);
	glDeleteBuffers(1, &this->mazeVbo);
	glDeleteVertexArrays(1, &this->mazeVao);
}

int Maze::GetMazeHeight()
{
	return this->height;
}

int Maze::GetMazeWidth()
{
	return this->width;
}

glm::vec2 Maze::GetExitPos() const
{
	return this->exitPos;
}

glm::vec3 Maze::GetExitWorldPos() const
{
	return this->exitWorldPos;
}

Transform * Maze::GetTransform()
{
	return &this->transform;
}

glm::vec2 * Maze::GetDrawOrder()
{
	return this->drawOrder;
}

glm::vec3 Maze::TransformToMazeCoords(glm::vec3 pos)
{
	float newX = pos.x;
	float newZ = pos.z;

	// NOT NEEDED Transform world coords to texture coords. ( 1 pixel on texture corresponds to 1.0, origo is (0, 0) for both spaces

	// The maze can be translated
	newX -= this->GetTransform()->GetPos().x;
	newZ -= this->GetTransform()->GetPos().z;

	// The maze can be scaled
	newX /= this->GetTransform()->GetScale().x;
	newZ /= this->GetTransform()->GetScale().z;

	// The walls have a offset, while the maze's center is in the origin (0,0)
	float pixelOffset = 0.5f;
	newX += (this->GetMazeWidth() / 2) + pixelOffset;
	newZ += (this->GetMazeHeight() / 2) + pixelOffset;

	pos.x = newX;
	pos.z = newZ;

	return pos;
}

glm::vec3 Maze::TransformToWorldCoords(glm::vec3 pos)
{
	float newX = pos.x;
	float newZ = pos.z;

	// NOT NEEDED Transform world coords to texture coords. ( 1 pixel on texture corresponds to 1.0, origo is (0, 0) for both spaces

	// The walls have a offset, while the maze's center is in the origin (0,0)
	newX -= (this->GetMazeWidth() / 2);
	newZ -= (this->GetMazeHeight() / 2);

	// The maze can be scaled
	newX *= this->GetTransform()->GetScale().x;
	newZ *= this->GetTransform()->GetScale().z;

	// The maze can be translated
	newX += this->GetTransform()->GetPos().x;
	newZ += this->GetTransform()->GetPos().z;
	
	pos.x = newX;
	pos.z = newZ;

	return pos;
}

unsigned int Maze::GetTileCount()
{
	return (1 + 2 * DRAWDISTANCE)*(1 + 2 * DRAWDISTANCE);
}

bool Maze::IsWallAtWorld(float x, float y)
{
	bool isAWall = true;

	glm::vec3 transformed = this->TransformToMazeCoords(glm::vec3(x, 0.0f, y));
	glm::vec3 pixel = readPixel(transformed.x, transformed.z);
	
	if (pixel == glm::vec3(0.0f, 0.0f, 0.0f) || pixel == glm::vec3(255, 15, 15)) // pixel == glm::vec3(255, 15, 15) is EXIT
	{
		isAWall = false;
	}

	return isAWall;
}

Wall Maze::GetWallType(float x, float y)
{
	Wall type = NO_WALL;
	glm::vec3 transformed = this->TransformToMazeCoords(glm::vec3(x, 0.0f, y));
	glm::vec3 pixel = readPixel(transformed.x, transformed.z);

	if (pixel == glm::vec3(100.0f, 0.0f, 255.0f))
	{
		type = WALL_UP;
	}
	else if (pixel == glm::vec3(100.0f, 255.0f, 0.0f))
	{
		type = WALL_DOWN;
	}
	else if (pixel == glm::vec3(100.0f, 0.0f, 0.0f))
	{
		type = WALL_LEFT;
	}
	else if (pixel == glm::vec3(100.0f, 255.0f, 255.0f))
	{
		type = WALL_RIGHT;
	}
	else if (pixel == glm::vec3(200.0f, 0.0f, 255.0f))
	{
		type = WALL_CORNER_RIGHT_UP;
	}
	else if (pixel == glm::vec3(200.0f, 255.0f, 0.0f))
	{
		type = WALL_CORNER_LEFT_UP;
	}
	else if (pixel == glm::vec3(200.0f, 0.0f, 0.0f))
	{
		type = WALL_CORNER_LEFT_DOWN;
	}
	else if (pixel == glm::vec3(200.0f, 255.0f, 255.0f))
	{
		type = WALL_CORNER_RIGHT_DOWN;
	}
	else if (pixel == glm::vec3(255.0f, 0.0f, 255.0f))
	{
		type = WALL_END_UP;
	}
	else if (pixel == glm::vec3(255.0f, 255.0f, 0.0f))
	{
		type = WALL_END_RIGHT;
	}
	else if (pixel == glm::vec3(255.0f, 0.0f, 0.0f))
	{
		type = WALL_END_DOWN;
	}
	else if (pixel == glm::vec3(255.0f, 255.0f, 255.0f))
	{
		type = WALL_END_LEFT;
	}
	else if (pixel == glm::vec3(50.0f, 0.0f, 0.0f))
	{
		type = WALL_VERTICAL;
	}
	else if (pixel == glm::vec3(50.0f, 255.0f, 255.0f))
	{
		type = WALL_HORIZONTAL;
	}

	return type;
}

void Maze::BindTexture(unsigned int textureUnit)
{
	if (textureUnit >= 0 && textureUnit <= 31)
	{
		glActiveTexture(GL_TEXTURE0 + textureUnit);
		glBindTexture(GL_TEXTURE_2D, this->mazeTexture);
	}
	else
	{
		std::cout << "[ERROR] Texture could not be bound. Unit not in range[0-31]" << std::endl;
	}
}

void Maze::LoadMaze(const std::string & fileName)
{
	this->path = fileName;

	this->imageData = stbi_load(fileName.c_str(), &this->width, &this->height, &this->numComponents, 3);

	if (this->imageData == NULL)
		std::cerr << "Loading failed for texture: " << fileName << std::endl;

	glGenTextures(1, &this->mazeTexture);
	glBindTexture(GL_TEXTURE_2D, this->mazeTexture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	// Skickar texturen till GPU'n
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, this->width, this->height, 0, GL_RGB, GL_UNSIGNED_BYTE, this->imageData);
}

void Maze::DrawMazeToBuffer()
{
	// Skip the fragment shader
	glEnable(GL_RASTERIZER_DISCARD);

	glBindVertexArray(this->mazeVao);
	glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, this->mazeTbo);
	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, this->mazeVbo);

	// Perform transform feedback
	glBeginTransformFeedback(GL_TRIANGLES);
	glDrawArrays(GL_POINTS, 0, (1 + 2 * DRAWDISTANCE)*(1 + 2 * DRAWDISTANCE));
	glEndTransformFeedback();

	// Enable the fragment shader again
	glDisable(GL_RASTERIZER_DISCARD);

	// Something ...
	glFlush();

	// Memory barrier
	glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0);
	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, 0);
	glBindVertexArray(0);
}

void Maze::DrawMaze()
{
	glBindVertexArray(this->mazeVao);
	
	glDrawTransformFeedback(GL_PATCHES, this->mazeTbo);
	
	// Memory barrier
	glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0);
	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, 0);
	glBindVertexArray(0);
}

void Maze::DrawShadows()
{
	glBindVertexArray(this->mazeVao);

	glDrawTransformFeedback(GL_TRIANGLES, this->mazeTbo);

	// Memory barrier
	glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0);
	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, 0);
	glBindVertexArray(0);
}

void Maze::BindMaterial(Shader* shader)
{
	this->floor0Mat->BindMaterialArray(shader, 0);
	this->wall1Mat->BindMaterialArray(shader, 1);
}

void Maze::InitiateMazeBuffers()
{
	// Three walls can be drawn with the same point, hence 18 vertices and not 6
	GLint maxNrOfVertices = 18;

	// mazeVao to draw points
	glGenVertexArrays(1, &this->mazeVao);
	glBindVertexArray(this->mazeVao);

	// Create a buffer to hold the results of the transform feedback process.
	glGenBuffers(1, &this->mazeVbo);
	glBindBuffer(GL_ARRAY_BUFFER, this->mazeVbo);

	// Allocate space (no data)
	glBufferData(
		GL_ARRAY_BUFFER,
		sizeof(glm::vec3)	 * maxNrOfVertices * (1 + 2 * DRAWDISTANCE)*(1 + 2 * DRAWDISTANCE) +	// Position
		sizeof(glm::vec2)	 * maxNrOfVertices * (1 + 2 * DRAWDISTANCE)*(1 + 2 * DRAWDISTANCE) +	// Texcoords
		sizeof(glm::vec3)	 * maxNrOfVertices * (1 + 2 * DRAWDISTANCE)*(1 + 2 * DRAWDISTANCE) +	// Normals
		sizeof(glm::vec3)	 * maxNrOfVertices * (1 + 2 * DRAWDISTANCE)*(1 + 2 * DRAWDISTANCE) +	// Tangents
		sizeof(float)		 * maxNrOfVertices * (1 + 2 * DRAWDISTANCE)*(1 + 2 * DRAWDISTANCE),	// Type
		NULL,							// no data passed
		GL_DYNAMIC_COPY);

	// Set the output Layout
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glEnableVertexAttribArray(3);
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3		 * sizeof(glm::vec3) + sizeof(glm::vec2) + sizeof(float), 0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 3		 * sizeof(glm::vec3) + sizeof(glm::vec2) + sizeof(float), (const GLvoid*)(sizeof(glm::vec3)));
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3		 * sizeof(glm::vec3) + sizeof(glm::vec2) + sizeof(float), (const GLvoid*)(sizeof(glm::vec3) + sizeof(glm::vec2)));
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 3		 * sizeof(glm::vec3) + sizeof(glm::vec2) + sizeof(float), (const GLvoid*)(2 * sizeof(glm::vec3) + sizeof(glm::vec2)));
	glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, 3		 * sizeof(glm::vec3) + sizeof(glm::vec2) + sizeof(float), (const GLvoid*)(3 * sizeof(glm::vec3) + sizeof(glm::vec2)));

	// Create and bind transform feedback object and buffer to write to.
	glGenTransformFeedbacks(1, &this->mazeTbo);
	glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, this->mazeTbo);
	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, this->mazeVbo);

	glBindVertexArray(0);
}

void Maze::GenerateDrawOrder()
{
	int layer = 0;
	int ID = 0;

	if (layer == 0)
	{
		this->drawOrder[ID].x = 0;
		this->drawOrder[ID].y = 0;
		ID++;
		layer++;
	}

	while (layer <= DRAWDISTANCE)
	{
		for (int y = (-layer); y < (1 + (2 * layer)) - layer; y++)
		{
			for (int x = (-layer); x < (1 + (2 * layer)) - layer; x++)
			{
				if (x == 1 - layer && y > (-layer) && y < (2 * layer) - layer)
					x += (1 + 2 * (layer - 1));
				this->drawOrder[ID].x = x;
				this->drawOrder[ID].y = y;
				ID++;
			}
		}
		layer++;
	}
}

void Maze::LoadTextures()
{
	MaterialHandler& MH = MaterialHandler::GetInstance();

	Texture* wallDiffuse = MH.LoadTexture("Textures/wall0/wall0_diffuse.png", "TextureDiffuse");
	Texture* wallNormal = MH.LoadTexture("Textures/wall0/wall0_normal.png", "TextureNormal");
	Texture* wallAmbient = MH.LoadTexture("Textures/wall0/wall0_ambient.png", "TextureAmbient");
	Texture* wallSpecular = MH.LoadTexture("Textures/wall0/wall0_specular.png", "TextureSpecular");
	Texture* wallHeight = MH.LoadTexture("Textures/wall0/wall0_height.png", "TextureHeight");
	float wallShininess = 16.0;

	
	Texture* floorDiffuse = MH.LoadTexture("Textures/floor0/floor0_diffuse.png", "TextureDiffuse");
	Texture* floorNormal = MH.LoadTexture("Textures/floor0/floor0_normal.png", "TextureNormal");
	Texture* floorAmbient = MH.LoadTexture("Textures/floor0/floor0_ambient.png", "TextureAmbient");
	Texture* floorSpecular = MH.LoadTexture("Textures/floor0/floor0_specular.png", "TextureSpecular");
	Texture* floorHeight = MH.LoadTexture("Textures/floor0/floor0_height.png", "TextureHeight");
	float floorShininess = 16.0;

	

	// Wall0
	this->wall1Mat = MH.AddMaterial(wallDiffuse, wallAmbient, wallSpecular, wallNormal, wallHeight, wallShininess, "wall1");

	// Floor0
	this->floor0Mat = MH.AddMaterial(floorDiffuse, floorAmbient, floorSpecular, floorNormal, floorHeight, floorShininess, "floor0");
}

// Returns a vector with the rgb value of a pixel
glm::vec3 Maze::readPixel(unsigned int x, unsigned int y)
{
	unsigned char* pixelOffset = this->imageData + (x + this->width * y) * this->numComponents;

	vector<unsigned char> pixel;
	for (int i = 0; i < 3; i++)
	{
		pixel.push_back(pixelOffset[i]);
	}

	return glm::vec3(pixel[0], pixel[1], pixel[2]);
}

glm::vec2 Maze::FindExit()
{
	// Loop through whole map
	for (int x = 0; x < this->width; x++)
	{
		for (int y = 0; y < this->height; y++)
		{
			if (this->readPixel(x, y) == glm::vec3(255, 15, 15))
			{
				return glm::vec2(x, y);
			}
		}
	}

	// Didn't find exit
	return glm::vec2(-1, -1);
}