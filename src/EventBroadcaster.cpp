#include "EventBroadcaster.h"
#include "iostream"

void EventBroadcaster::registerListener(EventListener* pListener) {
    EventKey EKey = pListener->getKey();
    this->mapListener[EKey].push_back(pListener);
    this->vecListener.push_back(pListener);
}
void EventBroadcaster::unregisterListener(EventListener* pListener) {
    EventKey EKey = pListener->getKey();
    auto& vec = this->mapListener[EKey];
    int nIndex = this->findListener(pListener, vec);
    if (nIndex != -1)
        vec.erase(vec.begin() + nIndex);

    nIndex = this->findListener(pListener);
    if (nIndex != -1)
        this->vecListener.erase(this->vecListener.begin() + nIndex);
}

void EventBroadcaster::unregisterAllListeners() {
    this->vecListener.clear();
    this->mapListener.clear();
}

void EventBroadcaster::broadcast(EventKey EKey, std::unordered_map<std::string, void*> mapParameter) {
    for(int i = 0; i < this->mapListener[EKey].size(); i++) {
        std::cout << this->mapListener[EKey][i]->getListenerOwnerName() << std::endl;
        if(this->mapListener[EKey][i]->isListenerEnabled())
        {
            this->mapListener[EKey][i]->onEventTrigger(mapParameter);
        }
    }
}

void EventBroadcaster::disableOtherListenerExcept(EventListener* pListenerExcluded)
{
    for (EventListener* pListener : this->vecListener)
    {
        if (pListenerExcluded->getKey() != pListener->getKey())
        {
            pListener->setListenerEnabled(false);
        }
    }
}

void EventBroadcaster::enableListener(std::string strName)
{
    for (EventListener* pListener : this->vecListener)
    {
        if (pListener->getListenerOwnerName() == strName)
        {
            pListener->setListenerEnabled(true);
        }
    }
}

void EventBroadcaster::enableAllListeners()
{
    for (EventListener* pListener : this->vecListener)
    {
        pListener->setListenerEnabled(true);
    }
}

int EventBroadcaster::findListener(EventListener* pListener, std::vector<EventListener*> vecListener) {
    if(vecListener.empty())
        vecListener = this->vecListener;

    int nIndex = -1;

    for(int i = 0; i < vecListener.size() && nIndex == -1; i++) {
        if(vecListener[i] == pListener)
            nIndex = i;
    }

    return nIndex;
}

// removes every copy of this pointer from the broadcaster.
// Unlike unregisterListener, it doesn't call getKey(): during a destructor the
// derived class is already gone, so calling a virtual function there would crash.
void EventBroadcaster::removeListener(EventListener* pListener)
{
    for (auto& entry : this->mapListener)
    {
        std::vector<EventListener*>& vec = entry.second;
        vec.erase(std::remove(vec.begin(), vec.end(), pListener), vec.end());
    }
    this->vecListener.erase(std::remove(this->vecListener.begin(), this->vecListener.end(), pListener), this->vecListener.end());
}

// a screen (intro, pause, story...) disables every other listener while it's open,
// so any disabled listener means one is open
bool EventBroadcaster::isBlocking()
{
    for (EventListener* pListener : this->vecListener)
        if (!pListener->isListenerEnabled())
            return true;
    return false;
}

// every listener (PauseScreen, ObjectiveIntro, StoryWindow...) runs this when deleted
EventListener::~EventListener()
{
    if (EventBroadcaster::getInstance())
        EventBroadcaster::getInstance()->removeListener(this);
}

/* * * * * * * * * * * * * * * * * * * * *
 *       SINGLETON-RELATED CONTENT       *
 * * * * * * * * * * * * * * * * * * * * */
EventBroadcaster* EventBroadcaster::P_SHARED_INSTANCE = NULL;
EventBroadcaster::EventBroadcaster() {}
EventBroadcaster::EventBroadcaster(const EventBroadcaster&) {}

void EventBroadcaster::initialize() {
    
    P_SHARED_INSTANCE = new EventBroadcaster();
  
}

void EventBroadcaster::destroy()
{
    delete P_SHARED_INSTANCE;
    P_SHARED_INSTANCE = nullptr;
}

EventBroadcaster* EventBroadcaster::getInstance() {
    return P_SHARED_INSTANCE;
}
/* * * * * * * * * * * * * * * * * * * * */
