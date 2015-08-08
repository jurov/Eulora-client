/*
 * craftbot.h
 *	Foxy's Crafting Bot for Eulora.
 *
 *	The bot can be accessed (basically started or stopped) via the /craft command (added to cmdusers.cpp). The bot has its own window in which it will give feedback on its activity and/or any errors that it may encounter.
 *
 *	Usage:
 *	- starting the bot:  /craft quantity recipe_name   (case-insensitive)
 *	- stopping the bot:  /craft stop   (or any other single word/number after /craft)
 *
 *	It does:
 *	- crafting (with any required container and tool) of the specified number of items with the specified recipe
 *	- full training when a new level has been reached during crafting
 *	- automated reading of the recipe and selection of needed container
 *	- automated retrieval from either inventory or storage (if not in inventory) of items needed according to the recipe
 *
 *	It expects (otherwise it will fail in most cases with an error message in its window):
 *	- recipe name to be provided in full (case does not matter though)
 *	- the recipe to pe "precise" meaning NO "between 1 and 7 coarse frangible threads" but rather "3 coarse frangible threads"
 *	- recipes to be either equipped already or in inventory (if they are in inventory, it will equip them)
 *	- the needed tool (if any) to be equipped
 *	- the needed container to be on the ground (hence, not in inventory) and within reach
 *	- if ingredients are to be taken out from storage by the bot, the character should be close enough to the NPC providing storage (so far Heina Draggenfort again)
 *	- if training is desired, the character should also be close enough to the trainer (Heina Draggenfort)
 *
 *	Tips and caveats:
 *	- you can safely switch away from eulora's window while the bot is working provided that you do NOT use for that a key that is set as shortcut to some game command (whether you know it or not). The "tab" key is atm set as default shortcut for "mouse look" so either you change that or you don't use alt-tab for switching away from eulora.
 *	- you can chat while the bot works and in general you can do any other activities that Eulora allows you to do while crafting something. However, checking your skills CAN cause some interference in some cases as it triggers some of the messages that the bot uses as a backup to infer when a job is done.
 *	- clicking on other items might change the target and therefore cause the current work to stop (as it would without the bot). The bot will try to recover from this, but it might remain hung if it doesn't receive any notification. The easiest solution is to simply restart it with another /craft command.
 *	- it won't work with recipes that do not specify a clear number of ingredients but rather allow a range of ingredients
 *
 *
 *	General concept and architecture:
 *	- the bot is basically a state machine advancing from one state to another based on the relevant messages received by Eulora's client.
 *	- the crafting process is modelled as a linear sequence of steps which correspond to the bot's states ,roughly:
 *
 *	     INACTIVE --> LEARNING_STORAGE --> READING_RECIPE --> RETRIEVING_INGREDIENTS --> COMBINING --> CRAFTING --> INACTIVE
 *	     					\					/				/							/	\		/	|
 *	     					 \				   /               /					       /	 \	   /    |
 *	     					  \               /               /							  /       \   /     |
 *	     					   \             /               /							 /         \ /      |
 *	     					    \           /               /						    /           /\ 	    |
 *	     					     \         /               /						   /           /  \     |
 *	     					      \       /               /							  /           /   TRAINING
 *	     					       \     /               /							 /           /
 *	     					        \   /               /							/           /
 *	     							 ERROR------------------------------------------------------
 *
 *  - although I avoided relying on the plain text of system messages sent to the client, some of them had to be used for increased robustness of the bot.
 *  - there are a few additional classes that the bot uses, namely:
 *  		- recipe class for holding the contents of a recipe and parsing it for use by the bot
 *  		- botStorage class for using the storage in game and retrieving needed items
 *  		- worldHandler class for actions (only actions) that require interacting with the game world, hence the client itself. This class is basically there to insulate the bot itself from the client's core code and to allow at the same time minimal changes to the core code. Some of the code in there is actually taken from the client's core code, usually when it was not exposed for use. The worldHandler class works basically as a rudimentary API to Eulora's client.
 *
 *  Created on: 22 Jul 2015
 *      Author: Diana Coman
 */
#ifndef CRAFT_BOT_HEADER
#define CRAFT_BOT_HEADER

#include "paws/pawswidget.h"
#include "recipe.h"
#include "botstorage.h"

enum BOT_STATES
{
	ERROR = -1,
	INACTIVE = 0,
	READING_RECIPE = 1,
	LEARNING_STORAGE = 2,
	BETWEEN_ACTIONS = 3,
	RETRIEVING_INGREDIENTS = 4,
	COMBINING = 5,
	CRAFTING = 6,
	TRAINING = 7
};

const int EXPECTED_MODS = 1;

const csString NOT_IN_RANGE_STORAGE = csString("You are not in range to check your storage with");
const csString RANKING_PREFIX = csString("You've ranked up in");
const csString CONTAINER_WORN = csString("The container is too worn to be useful");
const csString TOOL_WORN = csString("You do not have a good tool");
const csString WORK_STOPPED = csString("You stopped working.");
const csString WRONG_INGREDIENTS = csString("You don't have the right ingredients");
const csString STORAGE_HOLDER = csString("Heina Draggenfort");
const csString NOT_IN_STORAGE = csString("Storage does not have");
const csString GOT_ITEMS = csString("You got ");
const csString WRONG_TARGET_FOR_USE = csString("Only items can be targeted for use");
const csString TRAINING_RECEIVED = csString("You've received some ");


/** A bot for crafting items in Eulora
 */
class craftBot : public pawsWidget, public psClientNetSubscriber
{
public:
    craftBot();
    virtual ~craftBot();

    bool PostSetup();

    void HandleMessage( MsgEntry* message );

    //Crafting functions
    bool StartCraftRun(csString recipeName, int numberItems);
    void StopWorking();

private:
    pawsMessageTextBox* textbox;
    int textcolour;

    int itemsToDo;
    int itemsDone;
    recipe rcp;
    botStorage storage;

    BOT_STATES state;
    //counter for inventory and storage messages since otherwise it's impossible to know when all the needed items have been updated.
    int msgsInventoryToReceive;
    int msgsItemsNeeded;	//for retrieving the whole list of items + ids from storage

    //flags to navigate the messages since by themselves they are unreliable;
    int modReceived;
    int skillReceived;
    int trainingExpected;
    bool trainingSet;


    int attemptsLeft;

    bool inventoryOnly;

    //own utilities
    void OutputMsg(csString msg);
    void GoToState(BOT_STATES newState);
    void GoToNextState();
    void RecoverFromError();
    void ResetAllFlags();

    //handlers for each type of message intercepted
    void HandleMode(MsgEntry* message);
    void HandleGUISkill(MsgEntry* message);
    void HandleSystem(MsgEntry* message);
    void HandleCraftInfo(MsgEntry* message);
    void HandleGUIStorage(MsgEntry* message);
    void HandleGUIInventory(MsgEntry* message);

    //crafting steps
    bool ReadRecipe();
    bool GetStorageContents();
    bool GetIngredients();
    bool Combine();
    bool Craft();
    bool WrapUp();

};

CREATE_PAWS_FACTORY( craftBot );

#endif
