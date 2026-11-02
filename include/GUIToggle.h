#pragma once
#include "AComponent.h"
#include "EnumEventKey.h"
#include "EventBroadcaster.h"
#include <string>

class GUIToggle : public AComponent
{
private:
	EventKey EKey;
public:
	GUIToggle(EventKey EKey);
	~GUIToggle();

	void perform() override;

	EventKey getKey() const;
	void setKey(EventKey EKey);

	//events a button can open, by the name stored in level files
	struct ClickOption
	{
		const char* name;
		EventKey key;
	};

	static const ClickOption OPTIONS[];
	static const int OPTION_COUNT;
	static const char* nameOf(EventKey key);
	static bool keyOf(const std::string& name, EventKey& outKey);
};

