/*
 * worldhandler.h
 *
 *  Created on: 22 Jul 2015
 *      Author: Diana Coman
 */

#ifndef WORLDHANDLER_H_
#define WORLDHANDLER_H_

const csString trainerName = csString("Heina Draggenfort");

//handlers for interaction with the game world and original client core
class worldHandler
{
	//auxiliary methods for item handling
public:

	//tests and finder methods
	static bool IsInBrain(csString itemName);
	static bool IsWithinReach(csString name);
    static pawsSlot* FindInventoryItemSlot(csString itemName);
    static psInventoryCache::CachedItemDescription* FindItemSlot(csString itemName, bool anywhere);

    //item action methods
    static bool MoveItems(int fromContainer, int fromSlot, int toContainer, int toSlot, int quantity);
    static bool Equip(csString itemName);

    //containers and world objects targeting/using/combining/opening/properties
    static bool Target(csString containerName);
    static bool OpenTarget();
    static bool CombineContentsInTarget();
    static bool UseTarget();
    static bool TakeAllFromTarget();
    static bool CloseOpenWindow(csString windowName);
    static GEMClientObject* FindItemInWorld(csString name);

    //training
    static bool TrainSkillMax(csString skillName);

    /**
     * Gets the ID of the currently targeted item.
     * Returns -1 if nothing is currently targeted.
     */
    static int GetTargetID();
};



#endif /* WORLDHANDLER_H_ */