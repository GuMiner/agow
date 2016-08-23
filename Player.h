#pragma once
#include <SFML\System.hpp>
#include <SFML\Window.hpp>
#include <Bullet\btBulletDynamicsCommon.h>
#include "Config\KeyBindingConfig.h"
#include "Managers\RegionManager.h"
#include "Math\Vec.h"

class Player
{
    public:
        Player();
        void InputUpdate();

        const vec::vec3& GetViewPosition() const;
        const vec::quaternion& GetViewOrientation() const;

        void Warp(RegionManager* regionManager, btDynamicsWorld* world, const vec::vec2 mapPos);

    private:
        // Where the viewer is currently oriented on-screen.
        vec::vec3 viewPosition;
        vec::quaternion viewOrientation;

        sf::Vector2i lastMousePos;
};
