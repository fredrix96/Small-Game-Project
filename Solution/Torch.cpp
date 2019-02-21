#include "Torch.h"

Torch::Torch(Transform transform, Mesh * mesh, Texture * texture, irrklang::ISoundEngine* engine)
	:torchSound("Sounds/torch.wav", true, engine)
Torch::Torch(Transform transform, Mesh * mesh, Texture * texture, PointLightHandler* PLH, glm::vec3 lightColor)
	:particleTexture("Textures/particle.png", "NormalMaps/flat_normal.jpg")
{
	this->texture = texture;
	this->mesh = mesh;
	this->mesh->CreateMesh("ObjectFiles/torch.obj");
	this->transform = transform;
	this->transform.SetScale(glm::vec3(0.1f, 0.1f, 0.1f));
	this->transform.GetRot().x = 0;
	this->transform.GetRot().y = -transform.GetRot().y;
	this->transform.GetRot().z = 0;

	this->particle.SetTexture(&this->particleTexture);


	// Calculate the position of the torchLight
	glm::mat4 scaleMatrix = glm::scale(glm::vec3(0, this->transform.GetScale().y, 0));

	glm::mat4 rotationXMatrix = glm::rotate(this->transform.GetRot().x, glm::vec3(1, 0, 0));
	glm::mat4 rotationYMatrix = glm::rotate(this->transform.GetRot().y, glm::vec3(0, 1, 0));
	glm::mat4 rotationZMatrix = glm::rotate(this->transform.GetRot().z, glm::vec3(0, 0, 1));
	glm::mat4 rotationMatrix = rotationZMatrix * rotationYMatrix * rotationXMatrix;

	glm::mat4 scaleRotMatrix = rotationMatrix * scaleMatrix;

	this->lightPos = this->transform.GetPos() + glm::vec3(scaleRotMatrix * this->lightStartingPos);
	this->torchLight = PLH->CreateLight(this->lightPos, lightColor, 1.0f);

	// Rotate according to camera
	rotationXMatrix = glm::rotate(0.0f,					glm::vec3(1, 0, 0));
	rotationYMatrix = glm::rotate(-transform.GetRot().y,	glm::vec3(0, 1, 0));
	rotationZMatrix = glm::rotate(0.0f,					glm::vec3(0, 0, 1));
	rotationMatrix = rotationZMatrix * rotationYMatrix * rotationXMatrix;

	this->torchSound.SetVolume(0.4);
}

Torch::~Torch()
{
}

void Torch::SetScale(glm::vec3 scale)
{
	this->transform.SetScale(scale);
}

void Torch::SetPos(glm::vec3 pos)
{
	this->transform.SetPos(pos);
}

void Torch::SetRot(glm::vec3 rot)
{
	this->transform.SetRot(rot);
}

void Torch::SetTransform(Transform transform)
{
	this->transform = transform;
}

glm::vec3 & Torch::GetScale()
{
	return this->transform.GetScale();
}

glm::vec3 & Torch::GetPos()
{
	return this->transform.GetPos();
}

glm::vec3 & Torch::GetRot()
{
	return this->transform.GetRot();
}

Transform Torch::GetTransform()
{
	return this->transform;
}

Particle & Torch::GetParticle()
{
	return this->particle;
}

glm::vec3 Torch::GetFirePos()
{
	return this->lightPos;
}

void Torch::BindTexture()
{
	this->texture->Bind(0);
}

void Torch::Draw()
{
	this->mesh->Draw();
}

void Torch::Update(double dt, Transform transform, glm::vec3 camPos, glm::vec3 camForward, glm::vec3 camRight, glm::vec3 camUp, float distFromPlayer)
{
	this->transform.GetRot().y = -transform.GetRot().y;
	torchSound.Play();

	// Update the torch so that it is located in front of the player
	this->GetPos() = camPos
		+ camForward * distFromPlayer
		+ camRight * 0.4f
		+ camUp * -0.5f;

	// Update the lights position (Should be in the correct spot on the torch)
	glm::mat4 scaleMatrix = glm::scale(glm::vec3(0, this->transform.GetScale().y, 0));

	glm::mat4 rotationXMatrix = glm::rotate(this->transform.GetRot().x, glm::vec3(1, 0, 0));
	glm::mat4 rotationYMatrix = glm::rotate(this->transform.GetRot().y, glm::vec3(0, 1, 0));
	glm::mat4 rotationZMatrix = glm::rotate(this->transform.GetRot().z, glm::vec3(0, 0, 1));
	glm::mat4 rotationMatrix = rotationZMatrix * rotationYMatrix * rotationXMatrix;

	glm::mat4 scaleRotMatrix = rotationMatrix * scaleMatrix;

	this->lightPos = this->transform.GetPos() + glm::vec3(scaleRotMatrix * this->lightStartingPos);
	
	this->torchLight->GetPos() = this->lightPos;
}