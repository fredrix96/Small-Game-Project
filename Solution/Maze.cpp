#include "Maze.h"

Maze::Maze(string bitmapPath)
{
	this->imageData = nullptr;
	this->path = bitmapPath;
	this->width = 0;
	this->height = 0;
	this->numComponents = 0;
	this->texture = 0;

	LoadBMP(bitmapPath);
}

Maze::~Maze()
{
	stbi_image_free(imageData);
}

Maze::Maze(const Maze& other)
{
	this->path = other.path;
	this->width = other.width;
	this->height = other.height;
	this->numComponents = other.numComponents;

	this->LoadBMP(other.path);
}

Maze & Maze::operator=(const Maze & other)
{
	this->path = other.path;
	this->width = other.width;
	this->height = other.height;
	this->numComponents = other.numComponents;

	this->LoadBMP(other.path);

	return *this;
}

int Maze::GetMazeHeight()
{
	return this->height;
}

int Maze::GetMazeWidth()
{
	return this->width;
}

// Loading the .bmp file and returning the width,height and pointer to the first pixel in the file.
bool Maze::LoadBMP(const std::string & fileName)
{
	bool readingSuccess = true;

	this->imageData = stbi_load(fileName.c_str(), &this->width, &this->height, &this->numComponents, 4);

	if (this->imageData == NULL)
	{
		std::cerr << "Loading failed for bmp: " << fileName << std::endl;
		readingSuccess = false;
	}

	glGenTextures(1, &this->texture);
	glBindTexture(GL_TEXTURE_2D, this->texture);
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Skickar texturen till GPU'n
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, this->width, this->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, this->imageData);

	return readingSuccess;
}

bool Maze::IsWallAtWorld(float x, float y)
{
	bool isAWall = true;

	// NOT NEEDED Transform world coords to texture coords. ( 1 pixel on texture corresponds to 1.0, origo is (0, 0) for both spaces

	glm::vec3 pixel = readPixel(x, y);

	if (pixel == glm::vec3(0.0f, 0.0f, 0.0f))
	{
		isAWall = false;
	}

	return isAWall;
}

// Returns a vector with the rgb value of a pixel
glm::vec3 Maze::readPixel(unsigned int x, unsigned int y)
{
	unsigned int channelCount = 4;

	unsigned char* pixelOffset = this->imageData + (x + this->height * y) * channelCount;
	
	vector<unsigned char> pixel;
	for (int i = 0; i < 3; i++)
	{
		pixel.push_back(pixelOffset[i]);
	}

	return glm::vec3(pixel[0], pixel[1], pixel[2]);
}

void Maze::Bind(unsigned int textureUnit)
{
	if (textureUnit  >= 0 && textureUnit <= 31)
	{
		glActiveTexture(GL_TEXTURE0 + textureUnit);
		glBindTexture(GL_TEXTURE_2D, this->texture);
	}
	else
	{
		std::cout << "[ERROR] Texture could not be bound. Unit not in range[0-31]" << std::endl;
	}
}

void Maze::initiateBuffers()
{
	// create a buffer to hold the results of the transform feedback process.
	glGenBuffers(1, &this->vbo);
	glBindBuffer(GL_ARRAY_BUFFER, this->vbo);

	// allocate space (no data)
	glBufferData(
		GL_ARRAY_BUFFER,
		sizeof(glm::vec3) * 18 * 64 * 64 + sizeof(glm::vec2) * 18 * 64 * 64,		// 884,736 bytes <1Mb
		NULL,							// no data passed
		GL_DYNAMIC_COPY);

	// VAO to draw points
	glGenVertexArrays(1, &this->vao);
	glBindVertexArray(this->vao);

	glGenTransformFeedbacks(1, &this->tbo);
	// create and bind transform feedback object and buffer to write to.
	glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, this->tbo);
	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, this->vbo);
}
void Maze::LoadMaze(const std::string & fileName)
{
	this->imageData = stbi_load(fileName.c_str(), &this->width, &this->height, &this->numComponents, 3);

	if (this->imageData == NULL)
		std::cerr << "Loading failed for texture: " << fileName << std::endl;
	{
	}

	glGenTextures(1, &this->m_texture);
	glBindTexture(GL_TEXTURE_2D, this->m_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_NEAREST);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_NEAREST);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Skickar texturen till GPU'n
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, this->width, this->height, 0, GL_RGB, GL_UNSIGNED_BYTE, this->imageData);
}
void Maze::DrawToBuffer()
{
	// Skip the fragment shader

	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, this->vbo);
	glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, this->tbo);

	glEnable(GL_RASTERIZER_DISCARD);
	// Set the output Layout
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3) + sizeof(glm::vec2), 0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec3) + sizeof(glm::vec2), (const GLvoid*)(sizeof(glm::vec3)));

	// Perform transform feedback
	glBeginTransformFeedback(GL_TRIANGLES);
	glEndTransformFeedback();
	glDrawArrays(GL_POINTS, 0, this->width * this->height);

	// Enable the fragment shader again
	glDisable(GL_RASTERIZER_DISCARD);

	// Something ...
	glFlush();

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
}
void Maze::DrawMaze()
{
	glBindTransformFeedback(GL_TRANSFORM_FEEDBACK_BUFFER, this->tbo);
	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, this->vbo);

	// Set the input Layout
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3) + sizeof(glm::vec2), 0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec3) + sizeof(glm::vec2), (const GLvoid*)(sizeof(glm::vec3)));
	glEnableVertexAttribArray(1);

	glDrawTransformFeedback(GL_TRIANGLES, this->tbo);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
}
	this->tbo = 0;
	this->vbo = 0;
	this->vao = 0;
	glDeleteBuffers(1, &this->tbo);
	glDeleteBuffers(1, &this->vbo);
	glDeleteVertexArrays(1, &this->vao);