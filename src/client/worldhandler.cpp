/*
 * worldhandler.cpp
 *
 *  Created on: 22 Jul 2015
 *      Author: Diana Coman
 */

#include <psconfig.h>
#include <csgeom/math3d.h>
#include "globals.h"

#include "psclientchar.h"
#include "gui/inventorywindow.h"
#include "pscelclient.h"
#include "gui/pawsslot.h"
#include "util/slots.h"
#include "net/cmdhandler.h"

#include "worldhandler.h"
#include "cmdusers.h"

bool worldHandler::IsInBrain(csString itemName)
{
	pawsSlot* slot = FindInventoryItemSlot(itemName);
	if (slot == NULL)
		return false;
	else
		return (slot->GetID() == PSCHARACTER_SLOT_MIND);
}

pawsSlot* worldHandler::FindInventoryItemSlot(csString itemName)
{
	pawsInventoryWindow* window = (pawsInventoryWindow*)PawsManager::GetSingleton().FindWidget("InventoryWindow");
    pawsSlot* slot = dynamic_cast <pawsSlot*>(window->FindWidget(itemName.GetData()));

    return slot;
}

psInventoryCache::CachedItemDescription* worldHandler::FindItemSlot(csString itemName, bool anywhere)
{
	csArray<psInventoryCache::CachedItemDescription*>::Iterator iter = psengine->GetInventoryCache()->GetSortedIterator();
	psInventoryCache::CachedItemDescription* from = NULL;

	while (iter.HasNext())
	    		    {
	    		        psInventoryCache::CachedItemDescription* slotDesc = iter.Next();
	    		        Notify2(LOG_USER, "CRAFT BOT: item found with name=%s\n", slotDesc->name.GetData());
	    		        if (slotDesc->name.CompareNoCase(itemName))
	    		        {
	    		        	Notify2(LOG_USER, "Found item with containerID = %d\n", slotDesc->containerID);

	    		            if (anywhere || (slotDesc->containerID == CONTAINER_INVENTORY_BULK))
	    		        	//if (slotDesc->containerID == sc)
	    		            {
	    		                from = slotDesc;
	    		                Notify1(LOG_USER, "item found, breaking search\n");
	    		                break;  // We found an item, so breaking the search
	    		            }
	    		        }
	    		    }
	return from;
}

bool worldHandler::MoveItems(int fromContainer, int fromSlot, int toContainer, int toSlot, int quantity)
{
	if (fromContainer == CONTAINER_INVENTORY_BULK)
	{
	  fromSlot -= PSCHARACTER_SLOT_BULK1;
	  fromContainer =  (INVENTORY_SLOT_NUMBER)(fromSlot/100);
	  fromSlot = fromSlot%100;
	}
	psSlotMovementMsg msg( fromContainer, fromSlot, toContainer, toSlot, quantity);
	msg.SendMessage();
	return true;
}

bool worldHandler::Equip(csString itemName)
{
	psInventoryCache::CachedItemDescription* slot = FindItemSlot(itemName, true);
	if (slot == NULL)
		return false;

	if (slot->containerID != CONTAINER_INVENTORY_EQUIPMENT)	//equip only if truly needed
	{
		MoveItems(slot->containerID, slot->slot, CONTAINER_INVENTORY_EQUIPMENT, PSCHARACTER_SLOT_MIND, slot->stackCount);
	}

	return true;
}

bool worldHandler::IsWithinReach(csString name)
{
	GEMClientObject* object = FindItemInWorld(name);

	if (object)
		return true;
	return false;
}

GEMClientObject* worldHandler::FindItemInWorld(csString name)
{
    psCelClient* cel = psengine->GetCelClient();
    csVector3 myPos = cel->GetMainPlayer()->GetPosition();
    GEMClientObject* bestObject = NULL;
    float distance = 0.0f;

    // Find all entities within a certain radius.
    csArray<GEMClientObject*> entities = cel->FindNearbyEntities(cel->GetMainPlayer()->GetSector(),
	                                                                 myPos,
	                                                                 NEARBY_TARGET_MAX_RANGE);

	// Iterate through the entity list looking for one with the right name.
	size_t entityCount = entities.GetSize();

	for (size_t i = 0; i < entityCount; ++i)
	{
	   // Get the next entity, skip if it's me or the starting entity.
	        GEMClientObject* object = entities[i];
	        CS_ASSERT( object );

	        if (!csString(object->GetName()).StartsWith(name, true))
	            continue;
	        csVector3 pos(object->GetPosition());
	        float seDistance = csSquaredDist::PointPoint(myPos, pos);
	        if (!bestObject || seDistance < distance)
	        {
	            bestObject = object;
	            distance = seDistance;
	        }
	 }
    return bestObject;
}

bool worldHandler::Target(csString containerName)
{
	csString cmd = csString("/target ") + containerName;
	psengine->GetCmdHandler()->Execute(cmd.GetData());
	//psengine->GetCharManager()->LockTarget(true);	//lock the bloody target!
	return true;
}

bool worldHandler::OpenTarget()
{
	GEMClientObject *object = NULL;
	object = psengine->GetCharManager()->GetTarget();

	if (object == NULL)
		return false;

	EID id = object->GetEID();

	psViewItemDescription out(id.Unbox(), -1);
	out.SendMessage();

	return true;
}

int worldHandler::GetTargetID()
{
	GEMClientObject *object = NULL;
	object = psengine->GetCharManager()->GetTarget();

	if (!object)
		return -1;

	return object->GetEID().Unbox();
}
bool worldHandler::CloseOpenWindow(csString windowName)
{
	csString cmd = csString("/show ") + windowName;
	psengine->GetCmdHandler()->Execute(cmd.GetData());
	return true;
}
bool worldHandler::CombineContentsInTarget()
{
	psengine->GetCmdHandler()->Execute("/combine");
	return true;
}

bool worldHandler::UseTarget()
{
	psengine->GetCmdHandler()->Execute("/use");
	return true;
}

bool worldHandler::TakeAllFromTarget()
{
	psengine->GetCmdHandler()->Execute("/takeall");
	//psengine->GetCharManager()->LockTarget(false);	//unlock target, just in case
	return true;
}

bool worldHandler::TrainSkillMax(csString skillName)
{
	if (!IsWithinReach(trainerName))
		return false;

	csString cmd = csString("/target ") + trainerName;

	psengine->GetCmdHandler()->Execute(cmd.GetData());
	psengine->GetCmdHandler()->Execute("/train");

	csString commandData;
	commandData.Format("<B NAME=\"%s\" AMOUNT=\"%d\"/>", EscpXML(skillName).GetData(), 100);
	psGUISkillMessage msg(psGUISkillMessage::BUY_SKILL, commandData);
	msg.SendMessage();

	return true;
}

