#include "Math\PhysicsOps.h"
#include "Utils\Logger.h"
#include "PhysicsDebugDrawer.h"

PhysicsDebugDrawer::PhysicsDebugDrawer()
    : debugMode(DebugDrawModes::DBG_NoDebug)
{
}

bool PhysicsDebugDrawer::LoadBasics(ShaderManager* shaderManager)
{
    // Load our shader program to custom-render grass.
    if (!shaderManager->CreateShaderProgram("lineRender", &programId))
    {
        Logger::LogError("Failed to load the line rendering shader; cannot continue.");
        return false;
    }

    projMatrixLocation = glGetUniformLocation(programId, "projMatrix");

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, &positionBuffer);
    glGenBuffers(1, &colorBuffer);

    return true;
}

PhysicsDebugDrawer::~PhysicsDebugDrawer()
{
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &positionBuffer);
    glDeleteBuffers(1, &colorBuffer);

    glDeleteProgram(programId);
}

void PhysicsDebugDrawer::drawLine(const btVector3& from, const btVector3& to, const btVector3& color)
{
    // Draw a line with the 'from' position being slightly darker.
    glm::vec3 glmColor = glm::vec3(color.x(), color.y(), color.z());
    lines.positions.push_back(glm::vec3(from.x(), from.y(), from.z()));
    lines.colors.push_back(glmColor * 0.80f);

    lines.positions.push_back(glm::vec3(to.x(), to.y(), to.z()));
    lines.colors.push_back(glmColor);
}

void PhysicsDebugDrawer::drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color)
{
    // TODO implement.
}

void PhysicsDebugDrawer::reportErrorWarning(const char* warningString)
{
    Logger::LogError("Debug draw warning: ", warningString, ".");
}

void PhysicsDebugDrawer::draw3dText(const btVector3& location, const char* textString)
{
    // TODO implement
}

void PhysicsDebugDrawer::setDebugMode(int debugMode)
{
    this->debugMode = debugMode;
}

int PhysicsDebugDrawer::getDebugMode() const
{
    return debugMode;
}

bool PhysicsDebugDrawer::ShouldRender() const
{
    return debugMode != DebugDrawModes::DBG_NoDebug;
}

void PhysicsDebugDrawer::Reset()
{
    lines.positions.clear();
    lines.colors.clear();
}

void PhysicsDebugDrawer::Render(const glm::mat4 & projectionMatrix)
{
    // Render the lines.
    glUseProgram(programId);
    glBindVertexArray(vao);

    lines.TransferPositionToOpenGl(positionBuffer);
    lines.TransferColorToOpenGl(colorBuffer);

    glUniformMatrix4fv(projMatrixLocation, 1, GL_FALSE, &projectionMatrix[0][0]);
    glDrawArrays(GL_LINES, 0, lines.positions.size());
}
