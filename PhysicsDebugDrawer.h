#pragma once
#include <Bullet\LinearMath\btIDebugDraw.h>
#include <GL\glew.h>
#include <glm\mat4x4.hpp>
#include "Managers\ShaderManager.h"
#include "Utils\Vertex.h"

class PhysicsDebugDrawer : public btIDebugDraw
{
    GLuint programId;
    GLuint vao;
    
    GLuint positionBuffer;
    GLuint colorBuffer;

    GLuint projMatrixLocation;

    universalVertices lines;
    int debugMode;

public:
    PhysicsDebugDrawer();
    bool LoadBasics(ShaderManager* shaderManager);
    ~PhysicsDebugDrawer();

    // Defines how to handle Bullet physics debug drawing.
    virtual void drawLine(const btVector3& from, const btVector3& to, const btVector3& color) override;
    virtual void drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color) override;
    virtual void reportErrorWarning(const char* warningString) override;
    virtual void draw3dText(const btVector3& location, const char* textString) override;
    virtual void setDebugMode(int debugMode) override;
    virtual int getDebugMode() const override;

    bool ShouldRender() const;

    // Resets for another render frame.
    void Reset();

    // Renders
    void Render(const glm::mat4& projectionMatrix);
};

