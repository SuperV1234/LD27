// Copyright (c) 2013-2015 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#include <unordered_map>

#include "LDFactory.hpp"
#include "LDGroups.hpp"

#include "LDCPhysics.hpp"
#include "LDCRender.hpp"
#include "LDCPlayer.hpp"
#include "LDCPlayerAnimation.hpp"

using namespace std;
using namespace sf;
using namespace ssvu;
using namespace ssvu::FileSystem;
using namespace ssvs;
using namespace ssvsc;
using namespace ssvsc::Utils;
using namespace sses;

namespace ld
{
    Sprite LDFactory::getSpriteFromTile(
        const std::string& mTextureId, const IntRect& mTextureRect) const
    {
        return {assets.get<Texture>(mTextureId), mTextureRect};
    }
    void LDFactory::emplaceSpriteFromTile(LDCRender& mCDraw,
        const std::string& mTextureId, const sf::IntRect& mTextureRect) const
    {
        mCDraw.emplaceSprite(assets.get<Texture>(mTextureId), mTextureRect);
    }

    Entity& LDFactory::createWall(const Vec2i& mPos)
    {
        auto& result(manager.createEntity());
        auto& cPhysics(result.createComponent<LDCPhysics>(
            world, true, mPos, Vec2i{3200, 3200}));
        auto& cRender(
            result.createComponent<LDCRender>(game, cPhysics.getBody()));

        Body& body(cPhysics.getBody());
        body.addGroups(LDGroup::Solid);
        body.addGroupsToCheck(LDGroup::Solid);
        body.setVelTransferMultX(1.f);
        body.setVelTransferMultY(1.f);

        emplaceSpriteFromTile(cRender, "worldTiles.png",
            assets.tilesetWorld((getRndI(0, 100) < 75)
                                    ? Vec2u{1, 0}
                                    : Vec2u(3 + getRndI(0, 2), 0)));
        body.setStressMult(0.f);
        cRender.setScaleWithBody(false);

        return result;
    }
    Entity& LDFactory::createBlockBase(
        const Vec2i& mPos, const Vec2i& mSize, int mVal)
    {
        auto& result(manager.createEntity());
        result.addGroups(LDGroup::Block);
        auto& cPhysics(
            result.createComponent<LDCPhysics>(world, false, mPos, mSize));
        result.createComponent<LDCRender>(game, cPhysics.getBody());
        result.createComponent<LDCBlock>(mVal, game, cPhysics);

        Body& body(cPhysics.getBody());
        body.addGroups(LDGroup::Solid, LDGroup::Block);
        body.addGroupsToCheck(LDGroup::Solid);
        body.addGroupsNoResolve(LDGroup::BlockFloating);
        return result;
    }
    Entity& LDFactory::createBlock(const Vec2i& mPos, int mVal)
    {
        auto& result(createBlockBase(mPos, {1600, 1600}, mVal));
        auto& cPhysics(result.getComponent<LDCPhysics>());
        auto& cRender(result.getComponent<LDCRender>());
        Body& body(cPhysics.getBody());

        body.addGroups(LDGroup::CanBePicked);
        body.setRestitutionX(0.3f);
        body.setRestitutionY(0.3f);
        body.setMass(1.f);
        body.setVelTransferMultX(0.6f);

        emplaceSpriteFromTile(
            cRender, "worldTiles.png", assets.tilesetWorld(0, 0));

        if(mVal > -1)
        {
            if(colorMap.find(mVal) == colorMap.end())
                colorMap[mVal] = Color(
                    getRndI(0, 255), getRndI(0, 255), getRndI(0, 255), 255);
            cRender[0].setColor(colorMap[mVal]);
        }

        return result;
    }
    Entity& LDFactory::createBlockBig(const Vec2i& mPos, int mVal)
    {
        auto& result(createBlockBase(mPos, {2800, 2800}, mVal));
        auto& cPhysics(result.getComponent<LDCPhysics>());
        auto& cRender(result.getComponent<LDCRender>());
        Body& body(cPhysics.getBody());

        // cPhysics.setAffectedByGravity(false);
        // body.onPreUpdate += [&]{ body.setPosition(body.getPosition() -
        // Vec2i{-0,
        // 0}); body.setVelocityX(80); };
        // body.onPostUpdate += [&]{ body.setPosition(body.getPosition() -
        // Vec2i{-0,
        // 0}); body.setVelocityX(80); };
        body.setRestitutionX(0.3f);
        body.setRestitutionY(0.3f);
        body.setMass(10.f);
        body.setVelTransferMultX(0.1f);
        body.setVelTransferMultY(0.1f);

        emplaceSpriteFromTile(
            cRender, "worldTiles.png", assets.tilesetWorld(1, 1));

        if(mVal > -1)
        {
            if(colorMap.find(mVal) == colorMap.end())
                colorMap[mVal] = Color(
                    getRndI(0, 255), getRndI(0, 255), getRndI(0, 255), 255);
            cRender[0].setColor(colorMap[mVal]);
        }

        return result;
    }
    Entity& LDFactory::createBlockBall(const Vec2i& mPos, int mVal)
    {
        auto& result(createBlockBase(mPos, {1000, 1000}, mVal));
        auto& cPhysics(result.getComponent<LDCPhysics>());
        auto& cRender(result.getComponent<LDCRender>());
        Body& body(cPhysics.getBody());

        body.addGroups(LDGroup::CanBePicked);
        body.setRestitutionX(0.8f);
        body.setRestitutionY(0.8f);
        body.setMass(0.6f);
        body.setVelTransferMultX(0.01f);

        emplaceSpriteFromTile(
            cRender, "worldTiles.png", assets.tilesetWorld(7, 0));

        if(mVal > -1)
        {
            if(colorMap.find(mVal) == colorMap.end())
                colorMap[mVal] = Color(
                    getRndI(0, 255), getRndI(0, 255), getRndI(0, 255), 255);
            cRender[0].setColor(colorMap[mVal]);
        }

        return result;
    }
    Entity& LDFactory::createBlockRubberH(const Vec2i& mPos, int mVal)
    {
        auto& result(createBlockBase(mPos, {1600, 1600}, mVal));
        auto& cPhysics(result.getComponent<LDCPhysics>());
        auto& cRender(result.getComponent<LDCRender>());
        Body& body(cPhysics.getBody());

        body.addGroups(LDGroup::CanBePicked);
        body.setRestitutionX(0.8f);
        body.setRestitutionY(0.3f);
        body.setMass(0.8f);

        emplaceSpriteFromTile(
            cRender, "worldTiles.png", assets.tilesetWorld(9, 0));

        if(mVal > -1)
        {
            if(colorMap.find(mVal) == colorMap.end())
                colorMap[mVal] = Color(
                    getRndI(0, 255), getRndI(0, 255), getRndI(0, 255), 255);
            cRender[0].setColor(colorMap[mVal]);
        }

        return result;
    }
    Entity& LDFactory::createBlockRubberV(const Vec2i& mPos, int mVal)
    {
        auto& result(createBlockBase(mPos, {1600, 1600}, mVal));
        auto& cPhysics(result.getComponent<LDCPhysics>());
        auto& cRender(result.getComponent<LDCRender>());
        Body& body(cPhysics.getBody());

        body.addGroups(LDGroup::CanBePicked);
        body.setRestitutionX(0.3f);
        body.setRestitutionY(0.8f);
        body.setMass(0.8f);

        emplaceSpriteFromTile(
            cRender, "worldTiles.png", assets.tilesetWorld(10, 0));

        if(mVal > -1)
        {
            if(colorMap.find(mVal) == colorMap.end())
                colorMap[mVal] = Color(
                    getRndI(0, 255), getRndI(0, 255), getRndI(0, 255), 255);
            cRender[0].setColor(colorMap[mVal]);
        }

        return result;
    }

    Entity& LDFactory::createPlayer(const Vec2i& mPos)
    {
        auto& result(manager.createEntity());
        result.addGroups(LDGroup::Player);
        auto& cPhysics(result.createComponent<LDCPhysics>(
            world, false, mPos, Vec2i{800, 2700}));
        auto& cRender(
            result.createComponent<LDCRender>(game, cPhysics.getBody()));
        auto& cPlayer(result.createComponent<LDCPlayer>(game, cPhysics));
        result.createComponent<LDCPlayerAnimation>(
            assets.tilesetChar, cRender, cPlayer);

        Body& body(cPhysics.getBody());
        body.addGroups(LDGroup::Solid, LDGroup::Player);
        body.addGroupsToCheck(LDGroup::Solid);
        body.addGroupsNoResolve(LDGroup::BlockFloating);
        body.setRestitutionX(0.f);
        body.setRestitutionY(0.f);
        body.setMass(1.f);

        cRender.emplaceSprite(assets.get<Texture>("charTiles.png"));
        cRender.emplaceSprite(assets.get<Texture>("charTiles.png"));
        cRender.setScaleWithBody(false);

        result.setDrawPriority(-1000);
        return result;
    }

    Entity& LDFactory::createReceiver(const Vec2i& mPos, int mVal)
    {
        auto& result(manager.createEntity());
        auto& cPhysics(result.createComponent<LDCPhysics>(
            world, false, mPos, Vec2i{1600, 1600}));
        cPhysics.setAffectedByGravity(false);
        auto& cRender(
            result.createComponent<LDCRender>(game, cPhysics.getBody()));
        result.createComponent<LDCBlock>(mVal, game, cPhysics);

        Body& body(cPhysics.getBody());
        body.addGroupsToCheck(LDGroup::Block);
        body.setResolve(false);

        body.onDetection += [this, mVal](const DetectionInfo& mDI)
        {
            if(!mDI.body.hasGroup(LDGroup::Block)) return;
            auto& entity(*static_cast<Entity*>(mDI.userData));

            if(mVal == -1 || (mVal == entity.getComponent<LDCBlock>().getVal()))
            {
                game.getAssets().playSound(
                    "recv.wav", SoundPlayer::Mode::Override);
                entity.destroy();
                this->game.refresh10Secs();
            }
        };

        emplaceSpriteFromTile(
            cRender, "worldTiles.png", assets.tilesetWorld(5, 0));

        if(mVal > -1)
        {
            if(colorMap.find(mVal) == colorMap.end())
                colorMap[mVal] = Color(
                    getRndI(0, 255), getRndI(0, 255), getRndI(0, 255), 255);
            cRender[0].setColor(colorMap[mVal]);
        }

        return result;
    }

    Entity& LDFactory::createTele(const Vec2i& mPos)
    {
        auto& result(manager.createEntity());
        auto& cPhysics(result.createComponent<LDCPhysics>(
            world, false, mPos, Vec2i{1600, 100}));
        cPhysics.setAffectedByGravity(false);
        auto& cRender(
            result.createComponent<LDCRender>(game, cPhysics.getBody()));

        Body& body(cPhysics.getBody());
        body.addGroupsToCheck(LDGroup::Player);
        body.setResolve(false);

        body.onDetection += [this](const DetectionInfo& mDI)
        {
            if(!mDI.body.hasGroup(LDGroup::Player)) return;

            if(manager.getEntityCount(LDGroup::Block) == 0)
            {
                game.nextLevel();
                game.getAssets().playSound(
                    "tele.wav", SoundPlayer::Mode::Override);
            }
        };

        emplaceSpriteFromTile(
            cRender, "worldTiles.png", assets.tilesetWorld(6, 0));
        return result;
    }
    Entity& LDFactory::createLift(const Vec2i& mPos, const Vec2f& mVel)
    {
        auto& result(manager.createEntity());
        result.addGroups(LDGroup::Block);
        auto& cPhysics(result.createComponent<LDCPhysics>(
            world, false, mPos, Vec2i{3200, 1800}));
        auto& cRender(
            result.createComponent<LDCRender>(game, cPhysics.getBody()));

        Body& body(cPhysics.getBody());
        body.addGroups(LDGroup::Solid);
        body.addGroupsToCheck(LDGroup::Solid);
        body.addGroupsNoResolve(LDGroup::BlockFloating);
        cPhysics.setAffectedByGravity(false);
        body.onPreUpdate += [&, mVel]
        {
            body.setVelocity(mVel * 2.f);
        };
        body.setRestitutionX(0.3f);
        body.setRestitutionY(0.3f);
        body.setMass(10.f);
        body.setVelTransferMultX(1.f);
        body.setVelTransferMultY(1.f);

        emplaceSpriteFromTile(
            cRender, "worldTiles.png", assets.tilesetWorld(0, 1));
        return result;
    }
}
