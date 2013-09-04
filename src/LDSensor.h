// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#ifndef SSVLD_SENSOR
#define SSVLD_SENSOR

#include "LDDependencies.h"
#include "LDGroups.h"

namespace ld
{
	class LDGame;
	class LDCPhysics;

	class LDSensor
	{
		private:
			ssvsc::Body& parent;
			ssvs::Vec2i position;
			ssvsc::Sensor& sensor;
			bool active{false};

		public:
			ssvu::Delegate<void(sses::Entity&)> onDetection;

			LDSensor(ssvsc::Body& mParent, const ssvs::Vec2i& mSize) : parent(mParent), position(parent.getPosition()), sensor(parent.getWorld().createSensor(position, mSize))
			{
				sensor.addGroup(LDGroup::Sensor);

				sensor.onPreUpdate += [this]{ active = false; sensor.setPosition(position); };
				sensor.onDetection += [this](const ssvsc::DetectionInfo& mDI)
				{
					if(&mDI.body == &parent) return;
					active = true;

					if(mDI.userData == nullptr) return;
					auto& entity(*static_cast<sses::Entity*>(mDI.userData));
					onDetection(entity);
				};
			}
			~LDSensor() { sensor.destroy(); }

			void setPosition(const ssvs::Vec2i& mPos) { position = mPos; }

			inline ssvsc::Sensor& getSensor()	{ return sensor; }
			inline bool isActive() const		{ return active; }
	};
}

#endif
