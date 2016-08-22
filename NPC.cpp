#include "Math\VecOps.h"
#include "NPC.h"

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
	}
}

bool NPC::LoadNpcModels(ModelManager* modelManager)
{
	// TODO
	return true;
}

NPC::NPC(std::string name, std::string description, Shape shape, vec::vec4 color, int startingHealth)
	: name(name), description(description), shape(shape), color(color), health(startingHealth)
{
}

void NPC::LoadNpcPhysics(BasicPhysics physics, vec::vec3 startingPosition, float mass)
{
	BasicPhysics::CShape physicalShape = GetPhysicalShape(shape);
	physicalModel.modelId = models[shape];
	physicalModel.rigidBody = physics.GetDynamicBody(physicalShape, VecOps::Convert(startingPosition), mass);
	physics.DynamicsWorld->addRigidBody(physicalModel.rigidBody);
}

void NPC::Render(const vec::mat4& projectionMatrix, const vec::mat4& mvMatrix)
{
	
}

void NPC::UnloadNpcPhysics(BasicPhysics physics)
{
	physics.DynamicsWorld->removeRigidBody(physicalModel.rigidBody);
	physics.DeleteBody(physicalModel.rigidBody);
}

NPC::~NPC()
{

}
