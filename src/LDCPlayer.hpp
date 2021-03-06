// Copyright (c) 2013-2015 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#ifndef SSVLD_COMPONENTS_PLAYER
#define SSVLD_COMPONENTS_PLAYER

#include "LDDependencies.hpp"
#include "LDGroups.hpp"
#include "LDSensor.hpp"
#include "LDUtils.hpp"

namespace ld
{
    class LDCBlock : public sses::Component
    {
    private:
        int val;
        LDGame& game;
        LDCPhysics& cPhysics;
        Body& body;
        LDCPhysics* parent{nullptr};
        ssvs::Vec2i offset;
        ssvs::BitmapText text;

    public:
        LDCBlock(
            sses::Entity& mE, int mVal, LDGame& mGame, LDCPhysics& mCPhysics)
            : sses::Component{mE}, val(mVal), game(mGame), cPhysics(mCPhysics),
              body(cPhysics.getBody()),
              text{game.getAssets().get<ssvs::BitmapFont>("limeStroked"),
                  ssvu::toStr(val)}
        {
            text.setScale(0.75f, 0.75f);
            text.setTracking(-3);
            body.onResolution += [this](const ResolutionInfo& mRI)
            {
                if(body.hasGroup(LDGroup::BlockFloating)) return;

                if((std::abs(body.getVelocity().x) > 400 &&
                       mRI.resolution.x != 0) ||
                    (std::abs(body.getVelocity().y) > 400 &&
                        mRI.resolution.y != 0))
                    game.getAssets().playSound("bounce.wav");
            };
            body.onPreUpdate += [this]
            {
                // body.setVelocity(ssvs::getCClamped(body.getVelocity(),
                // -800.f,
                // 800.f));

                if(!cPhysics.isInAir())
                {
                    // Ground friction
                    // body.setVelocityX(body.getVelocity().x * 0.9f);

                    //					if(body.getWidth() > 1900)
                    // else
                    //	body.setVelocityX(body.getVelocity().x * 0.98f);

                    // If velocity is very small, set it to 0
                    if(std::abs(body.getVelocity().x) < 5.f)
                        body.setVelocityX(0.f);

                    if(parent == nullptr)
                        body.delGroupsNoResolve(LDGroup::Player);
                }
                text.setString(ssvu::toStr(ssvu::toInt(body.getStress().y)));
            };

            body.onPostUpdate += [this]
            {
                if(body.getStress().y > 10000) getEntity().destroy();
                // Global min/max velocity
                // body.setVelocity(ssvs::getCClamped(body.getVelocity(),
                // -800.f,
                // 800.f));
            };
        }
        inline void update(FT) override
        {
            text.setPosition(toPixels(body.getShape().getVertexNW<int>()) +
                             ssvs::Vec2f{4, 3});

            if(parent != nullptr)
            {
                ssvs::Vec2f v{(parent->getBody().getPosition() + offset) -
                              body.getPosition()};
                if(ssvs::getDistEuclidean(parent->getBody().getPosition(),
                       body.getPosition()) > 1700)
                    dropped();
                else
                    body.setVelocity(v);
            }
        }
        inline void draw() override
        {
            // if(val != -1)
            game.render(text);
        }

        inline void pickedUp(LDCPhysics& mParent)
        {
            game.start10Secs();
            parent = &mParent;
            body.addGroups(LDGroup::BlockFloating);
            body.addGroupsNoResolve(LDGroup::Player);
        }
        inline void dropped(float mHBoost = 1.f, float mVBoost = 1.f)
        {
            parent = nullptr;
            auto newVel(body.getVelocity());
            newVel.x *= mHBoost;
            newVel.y *= mVBoost;
            body.setVelocity(ssvs::getCClamped(newVel, -1000.f, 1000.f));
            body.delGroups(LDGroup::BlockFloating);
        }
        inline void setOffset(const ssvs::Vec2i& mOffset) { offset = mOffset; }
        inline bool hasParent() { return parent != nullptr; }
        inline int getVal() { return val; }
    };

    class LDCPlayer : public sses::Component
    {
    public:
        enum class Action
        {
            Standing,
            Walking,
            Jumping,
            Falling
        };

    private:
        LDGame& game;
        LDCPhysics& cPhysics;
        Body& body;
        Action action;
        bool facingLeft{false}, jumpReady{false};
        float walkSpeed{150.f}, jumpSpeed{520.f};
        bool wasFacingLeft{false};
        float lastTurn{0.f};
        bool wasFacingRight{false};
        float lastJump{0.f}, stepTime{0.f};

        LDSensor blockSensor{cPhysics.getBody(), ssvs::Vec2i{10, 2400}};
        LDCBlock* currentBlock{nullptr};
        sses::EntityStat currentBlockStat;
        Body* lastBlock{nullptr};
        float lastBlockTimer{0.f};

    public:
        LDCPlayer(sses::Entity& mE, LDGame& mGame, LDCPhysics& mCPhysics)
            : sses::Component{mE}, game(mGame), cPhysics(mCPhysics),
              body(cPhysics.getBody())
        {
            blockSensor.getSensor().addGroupsToCheck(LDGroup::Block);

            blockSensor.onDetection += [this](sses::Entity& mDE)
            {
                if(hasBlock() || !game.getIAction()) return;
                if(!mDE.getComponent<LDCPhysics>().getBody().hasGroup(
                       LDGroup::CanBePicked))
                    return;
                auto& block(mDE.getComponent<LDCBlock>());
                block.pickedUp(cPhysics);
                if(currentBlock != nullptr) currentBlock->dropped();
                currentBlock = &block;
                currentBlockStat = currentBlock->getEntity().getStat();
                lastBlock = &mDE.getComponent<LDCPhysics>().getBody();

                game.getAssets().playSound(
                    "pick.wav", ssvs::SoundPlayer::Mode::Abort);
            };

            body.onPreUpdate += [this]
            {
                jumpReady = false;
            };
            body.onPostUpdate += [this]
            {
            };
            body.onDetection += [this](const DetectionInfo&)
            {
            };
            body.onResolution += [this](const ResolutionInfo& mRI)
            {
                // When you get pushed in a floor/ceiling so hard that a fourth
                // of
                // your height is inside, you're considered crushed
                // if(std::abs(mRI.resolution.y) > body.getHeight() / 12.f) {
                // getEntity().destroy(); return; }

                if(lastBlock == nullptr || &mRI.body != lastBlock ||
                    lastBlockTimer <= 0)
                    return;

                // Ignore resolution against last picked block for some time
                mRI.noResolvePosition = mRI.noResolveVelocity = true;
            };

            cPhysics.onResolution += [this](const ssvs::Vec2i&)
            {
                jumpReady = true;
            };
        }
        ~LDCPlayer()
        {
            if(currentBlock == nullptr) return;
            currentBlock->dropped();
        }

        void update(FT mFT) override
        {
            if(currentBlock != nullptr &&
                !getManager().isAlive(currentBlockStat))
                currentBlock = nullptr;

            wasFacingLeft = facingLeft;
            wasFacingRight = !facingLeft;

            ssvs::Vec2i offset{facingLeft ? -1000 : 1000, -600};
            blockSensor.setPosition(
                body.getPosition() + ssvs::Vec2i{offset.x / 2, 300});

            if(game.getIX() == 0)
                move(0, mFT);
            else if(game.getIX() == -1)
                move(-1, mFT);
            else if(game.getIX() == 1)
                move(1, mFT);

            if(game.getIJump() == 1) jump();

            const auto& velocity(body.getVelocity());

            if(velocity.x > 0)
                facingLeft = false;
            else if(velocity.x < 0)
                facingLeft = true;

            if(!cPhysics.isInAir())
            {
                if(velocity.x == 0)
                    action = Action::Standing;
                else if(std::abs(velocity.x) >= walkSpeed)
                    action = Action::Walking;
            }
            else
            {
                if(velocity.y > 0)
                    action = Action::Falling;
                else if(velocity.y < 0)
                    action = Action::Jumping;
            }

            if(facingLeft && !wasFacingLeft) lastTurn = 20.f;
            if(!facingLeft && !wasFacingRight) lastTurn = 20.f;

            if(lastTurn > 0.f) lastTurn -= mFT;
            if(lastJump > 0.f) lastJump -= mFT;

            if(hasBlock())
            {
                lastBlockTimer = 15.f;

                if(!game.getIAction())
                {
                    game.getAssets().playSound(
                        "drop.wav", ssvs::SoundPlayer::Mode::Abort);
                    currentBlock->dropped(
                        ((lastTurn > 0.f) ? lastTurn * 0.12f : 1.f),
                        ((lastJump > 0.f) ? lastJump * 0.12f : 1.f));
                    currentBlock = nullptr;
                    return;
                }
                currentBlock->setOffset(offset);
                if(!currentBlock->hasParent()) currentBlock = nullptr;
            }
            else if(lastBlockTimer > 0)
                lastBlockTimer -= mFT;
        }

        inline void move(int mDirection, FT mFT)
        {
            body.setVelocityX(walkSpeed * mDirection);

            if(mDirection != 0)
            {
                if(stepTime > 0.f)
                    stepTime -= mFT;
                else if(!cPhysics.isInAir())
                {
                    game.getAssets().playSound("step.wav");
                    stepTime = 26.f;
                }
            }
        }
        inline void jump()
        {
            if(cPhysics.isInAir() || lastJump > 0.f) return;
            body.setVelocityY(body.getVelocity().y - jumpSpeed);
            lastJump = 20.f;
            game.getAssets().playSound("jump.wav");
        }

        inline Action getAction() { return action; }
        inline bool isJumpReady() { return jumpReady; }
        inline bool isFacingLeft() { return facingLeft; }
        inline bool hasBlock() { return currentBlock != nullptr; }
    };
}

#endif
