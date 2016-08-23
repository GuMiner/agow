#include "Math\VecOps.h"
#include "Utils\Logger.h"
#include "NPC.h"
#include <iostream>

std::map<NPC::Shape, unsigned int> NPC::models;

BasicPhysics::CShape NPC::GetPhysicalShape(Shape shape)
{
	switch (shape)
	{
	case Shape::CAPSULE:
		return BasicPhysics::CShape::NPC_CAPSULE;
	case Shape::DIAMOND:
		return BasicPhysics::CShape::NPC_DIAMOND;
	case Shape::CUBOID:
		return BasicPhysics::CShape::NPC_CUBOID;
    default:
        return BasicPhysics::CShape::NPC_CAPSULE;
	}
}

bool NPC::LoadNpcModels(ModelManager* modelManager)
{
	unsigned int capsuleModel = modelManager->LoadModel("models/npc_capsule");
	if (capsuleModel == 0)
	{
		return false;
	}

	models[Shape::CAPSULE] = capsuleModel;

	unsigned int diamondModel = modelManager->LoadModel("models/npc_diamond");
	if (diamondModel == 0)
	{
		return false;
	}

	models[Shape::DIAMOND] = diamondModel;

	unsigned int cuboidModel = modelManager->LoadModel("models/npc_cuboid");
	if (cuboidModel == 0)
	{
		return false;
	}

	models[Shape::CUBOID] = cuboidModel;
    Logger::Log("Loaded NPC models ", capsuleModel, ", ", cuboidModel, ", and ", diamondModel, ".");

	return true;
}

NPC::NPC(std::string name, std::string description, Shape shape, vec::vec4 color, int startingHealth)
	: name(name), description(description), shape(shape), color(color), health(startingHealth), isSelected(false)
{
}

void NPC::LoadNpcPhysics(BasicPhysics physics, vec::vec3 startingPosition, float mass)
{
	BasicPhysics::CShape physicalShape = GetPhysicalShape(shape);
	physicalModel.modelId = models[shape];
	physicalModel.rigidBody = physics.GetDynamicBody(physicalShape, VecOps::Convert(startingPosition), mass);

	// NPCs can't rotate by default.
	physicalModel.rigidBody->setAngularFactor(0.0f);

	physics.DynamicsWorld->addRigidBody(physicalModel.rigidBody);
}

void NPC::Render(ModelManager* modelManager, const vec::mat4& projectionMatrix)
{
    vec::mat4 mvMatrix = BasicPhysics::GetBodyMatrix(physicalModel.rigidBody);
	modelManager->RenderModel(projectionMatrix, physicalModel.modelId, mvMatrix, color, isSelected);
}

void NPC::UnloadNpcPhysics(BasicPhysics physics)
{
	physics.DynamicsWorld->removeRigidBody(physicalModel.rigidBody);
	physics.DeleteBody(physicalModel.rigidBody);
}

NPC::~NPC()
{

}
