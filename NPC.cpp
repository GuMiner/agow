#include "Managers\FontManager.h"
#include "Math\MathOps.h"
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
	unsigned int capsuleModel = modelManager->LoadModel("models/npc/capsule");
	if (capsuleModel == 0)
	{
		return false;
	}

	models[Shape::CAPSULE] = capsuleModel;

	unsigned int diamondModel = modelManager->LoadModel("models/npc/diamond");
	if (diamondModel == 0)
	{
		return false;
	}

	models[Shape::DIAMOND] = diamondModel;

	unsigned int cuboidModel = modelManager->LoadModel("models/npc/cuboid");
	if (cuboidModel == 0)
	{
		return false;
	}

	models[Shape::CUBOID] = cuboidModel;
    Logger::Log("Loaded NPC models ", capsuleModel, ", ", cuboidModel, ", and ", diamondModel, ".");

	return true;
}

NPC::NPC(std::string name, std::string description, Shape shape, vec::vec4 color, int startingHealth)
    : name(name), description(description), shape(shape), color(color), health(startingHealth),
      isSelected(false), showInteractionKeys(false), nearFieldCollisionLastFrame(false)
{
    nameString.color = vec::vec3(1.0f) - vec::vec3(color.x, color.y, color.z);
}

void NPC::LoadGraphics(FontManager* fontManager)
{
    nameString.sentenceId = fontManager->CreateNewSentence();
    fontManager->UpdateSentence(nameString.sentenceId, name, 22, nameString.color);
}

void NPC::LoadNpcPhysics(BasicPhysics physics, vec::vec3 startingPosition, float mass)
{
	BasicPhysics::CShape physicalShape = GetPhysicalShape(shape);
	physicalModel.modelId = models[shape];
	physicalModel.rigidBody = physics.GetDynamicBody(physicalShape, VecOps::Convert(startingPosition), mass);

	// NPCs can't rotate from physical interactions.
	physicalModel.rigidBody->setAngularFactor(0.0f);
    
    physics.DynamicsWorld->addRigidBody(physicalModel.rigidBody);

    nearFieldBubble = physics.GetGhostObject(BasicPhysics::CShape::NPC_NEARFIELD_BUBBLE, VecOps::Convert(startingPosition));
    nearFieldBubble->setUserPointer(new UserPhysics(ObjectType::NPC_CLOSEUP, this));
    physics.DynamicsWorld->addRigidBody(nearFieldBubble);
}

void NPC::Update(float gameTime, float elapsedTime)
{
    nameString.posRotMatrix =
        MatrixOps::Translate(vec::vec3(-0.60f, -0.60f, 0.55f)) *
        BasicPhysics::GetBodyMatrix(physicalModel.rigidBody) * 
        MatrixOps::Scale(0.20f, 0.20f, 0.20f) * 
        MatrixOps::Rotate(90.0f, vec::vec3(1.0f, 0.0f, 0.0f));

    // This works as physics updates are last. TODO make this more deterministic.
    if (nearFieldCollisionLastFrame)
    {
        nearFieldCollisionLastFrame = false;
    }
    else if (showInteractionKeys)
    {
        isSelected = false;
        showInteractionKeys = false;
    }

    // TODO figure out why the fixed constraint doesn't seem to properly work.
    nearFieldBubble->setWorldTransform(physicalModel.rigidBody->getWorldTransform());
}

void NPC::Render(FontManager* fontManager, ModelManager* modelManager, const vec::mat4& projectionMatrix)
{
    vec::mat4 mvMatrix = BasicPhysics::GetBodyMatrix(physicalModel.rigidBody);
	modelManager->RenderModel(projectionMatrix, physicalModel.modelId, mvMatrix, color, isSelected);
    
    fontManager->RenderSentence(nameString.sentenceId, projectionMatrix, nameString.posRotMatrix);
}

void NPC::UnloadNpcPhysics(BasicPhysics physics)
{
	physics.DynamicsWorld->removeRigidBody(physicalModel.rigidBody);
	physics.DeleteBody(physicalModel.rigidBody);
}

NPC::~NPC()
{

}

void NPC::CollisionCallback(ObjectType collidingObject)
{
    // This occurs when the NPC's ghost object -- representing their FOV -- is hit.
    if (collidingObject == ObjectType::PLAYER)
    {
        isSelected = true;
        showInteractionKeys = true;
        nearFieldCollisionLastFrame = true;
    }
}
