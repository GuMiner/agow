#include <glm/gtc/matrix_transform.hpp>
#include "Managers\FontManager.h"
#include "Math\PhysicsOps.h"
#include "Utils\Logger.h"
#include "DialogPane.h"
#include "NPC.h"

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

NPC::NPC(std::string name, std::string description, Shape shape, glm::vec4 color, int startingHealth)
    : name(name), description(description), shape(shape), model(), health(startingHealth), startingHealth(startingHealth),
      showInteractionKeys(false), nearFieldCollisionLastFrame(false), selectionChange(false)
{
    model.color = color;
    model.selected = false;

    nameString.color = glm::vec3(1.0f) - glm::vec3(color.x, color.y, color.z);
    interactionString.color = nameString.color * 1.10f;
}

void NPC::LoadGraphics(FontManager* fontManager)
{
    nameString.sentenceId = fontManager->CreateNewSentence();
    fontManager->UpdateSentence(nameString.sentenceId, name, 22, nameString.color);

    interactionString.sentenceId = fontManager->CreateNewSentence();
    fontManager->UpdateSentence(interactionString.sentenceId, "Converse [C]", 22, interactionString.color);
}

void NPC::LoadNpcPhysics(BasicPhysics physics, glm::vec3 startingPosition, float mass)
{
    BasicPhysics::CShape physicalShape = GetPhysicalShape(shape);
    model.modelId = models[shape];
    model.body = physics.GetDynamicBody(physicalShape, PhysicsOps::Convert(startingPosition), mass);

    // NPCs can't rotate from physical interactions.
    model.body->setAngularFactor(0.0f);
    
    physics.DynamicsWorld->addRigidBody(model.body);

    nearFieldBubble = physics.GetGhostObject(BasicPhysics::CShape::NPC_NEARFIELD_BUBBLE, PhysicsOps::Convert(startingPosition));
    nearFieldBubble->setUserPointer(new TypedCallback<UserPhysics::ObjectType>(UserPhysics::ObjectType::NPC_CLOSEUP, this));
    physics.DynamicsWorld->addRigidBody(nearFieldBubble);
}

bool NPC::Converse(DialogPane* dialogPane)
{
    // This model is selected.
    if (showInteractionKeys)
    {
        // TODO -- this should integrate with the story graph (to setup next).
        dialogPane->QueueText(StyleText(GetName(), GetDescription(), StyleText::Effect::NORMAL));
        return true;
    }
    
    return false;
}

std::string NPC::GetName() const
{
    return name;
}

glm::vec3 NPC::GetPosition() const
{
    return BasicPhysics::GetBodyPosition(model.body);
}

std::string NPC::GetDescription() const
{
    return description;
}

bool NPC::CanKill() const
{
    return health != INVULNERABLE;
}

bool NPC::IsAlive() const
{
    return !CanKill() || health > 0;
}

void NPC::Update(float gameTime, float elapsedTime)
{
    nameString.posRotMatrix =
        glm::translate(glm::mat4(), glm::vec3(-0.60f, -0.60f, 0.55f)) *
        BasicPhysics::GetBodyMatrix(model.body) *
        glm::scale(glm::mat4(), glm::vec3(0.20f)) *
        glm::rotate(glm::mat4(), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    interactionString.posRotMatrix = glm::translate(glm::mat4(), glm::vec3(0.0f, 0.0f, -0.20f)) * nameString.posRotMatrix;

    // This works as physics updates are last. TODO make this more deterministic.
    if (nearFieldCollisionLastFrame)
    {
        nearFieldCollisionLastFrame = false;
    }
    else if (showInteractionKeys)
    {
        model.selected = false;
        selectionChange = true;
        showInteractionKeys = false;
    }

    // TODO figure out why the fixed constraint doesn't seem to properly work.
    nearFieldBubble->setWorldTransform(model.body->getWorldTransform());
}

void NPC::Render(FontManager* fontManager, ModelManager* modelManager, const glm::mat4& projectionMatrix)
{
    if (selectionChange)
    {
        model.body->setActivationState(ACTIVE_TAG);
    }
    
    modelManager->RenderModel(projectionMatrix, &model);
    fontManager->RenderSentence(nameString.sentenceId, projectionMatrix, nameString.posRotMatrix);

    if (showInteractionKeys)
    {
        fontManager->RenderSentence(interactionString.sentenceId, projectionMatrix, interactionString.posRotMatrix);
    }
}

void NPC::UnloadNpcPhysics(BasicPhysics physics)
{
    delete nearFieldBubble->getUserPointer();
    physics.DynamicsWorld->removeRigidBody(model.body);
    physics.DeleteBody(model.body, false);
}

NPC::~NPC()
{

}

void NPC::Callback(UserPhysics::ObjectType collidingObject, void* callbackSpecificData)
{
    // This occurs when the NPC's ghost object -- representing their FOV -- is hit.
    if (collidingObject == UserPhysics::ObjectType::PLAYER)
    {
        model.selected = true;
        selectionChange = true;
        showInteractionKeys = true;
        nearFieldCollisionLastFrame = true;
    }
}
