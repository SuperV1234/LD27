// Copyright (c) 2013-2015 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#ifndef SSVLD_COMPONENTS_PHYSICS
#define SSVLD_COMPONENTS_PHYSICS

#include "LDSensor.hpp"
#include "LDDependencies.hpp"

namespace ld
{
    class LDGame;

    class LDCPhysics : public sses::Component
    {
    private:
        static constexpr int crushedMax{3}, crushedTolerance{1};

        World& world;
        Body& body;
        ssvs::Vec2i lastResolution;
        bool affectedByGravity{true};
        int crushedLeft{0}, crushedRight{0}, crushedTop{0}, crushedBottom{0};
        int maxVelocityY{1000};
        ssvs::Vec2f gravityForce{0, 25};
        LDSensor groundSensor;

    public:
        ssvu::Delegate<void(sses::Entity&)> onDetection;
        ssvu::Delegate<void(const ssvs::Vec2i&)> onResolution;

        LDCPhysics(sses::Entity& mE, World& mWorld, bool mIsStatic,
            const ssvs::Vec2i& mPosition, const ssvs::Vec2i& mSize,
            bool mAffectedByGravity = true);
        inline ~LDCPhysics() { body.destroy(); }


        inline void update(FT) override
        {
            if(affectedByGravity && body.getVelocity().y < maxVelocityY)
                body.applyAccel(gravityForce);
        }

        inline void setAffectedByGravity(bool mAffectedByGravity)
        {
            affectedByGravity = mAffectedByGravity;
        }

        inline World& getWorld() const { return world; }
        inline Body& getBody() const { return body; }
        inline const ssvs::Vec2i& getPos() const { return body.getPosition(); }
        inline const ssvs::Vec2i& getLastResolution() const
        {
            return lastResolution;
        }
        inline bool isAffectedByGravity() const { return affectedByGravity; }
        inline bool isCrushedLeft() const
        {
            return crushedLeft > crushedTolerance;
        }
        inline bool isCrushedRight() const
        {
            return crushedRight > crushedTolerance;
        }
        inline bool isCrushedTop() const
        {
            return crushedTop > crushedTolerance;
        }
        inline bool isCrushedBottom() const
        {
            return crushedBottom > crushedTolerance;
        }
        inline int getCrushedLeft() const { return crushedLeft; }
        inline int getCrushedRight() const { return crushedRight; }
        inline int getCrushedTop() const { return crushedTop; }
        inline int getCrushedBottom() const { return crushedBottom; }
        inline bool isInAir()
        {
            return !groundSensor.isActive();
        } // std::abs(body.getShape().getY() - body.getOldShape().getY()) >=
          // std::abs(body.getLastResolution().y) + 10.f; }
    };
}

#endif
