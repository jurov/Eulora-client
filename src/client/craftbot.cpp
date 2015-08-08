/*
 * craftbot.cpp
 *
 *  Created on: 22 Jul 2015
 *      Author: Diana Coman
 */
#include <psconfig.h>
#include "globals.h"

#include "psclientchar.h"
#include "gui/inventorywindow.h"
#include "pscelclient.h"


// PAWS INCLUDES

#include "net/messages.h"
#include "net/clientmsghandler.h"
#include "util/log.h"
#include "net/connection.h"

#include "net/cmdhandler.h"

#include "craftbot.h"
#include "gui/pawsslot.h"
#include "util/slots.h"
#include "worldhandler.h"
#include "util/log.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

craftBot::craftBot()
{
	itemsToDo = 0;
	itemsDone = 0;
	state = INACTIVE;
	textcolour = 61046;
}

bool craftBot::PostSetup()
{
	psengine->GetMsgHandler()->Subscribe(this, MSGTYPE_MODE);
	psengine->GetMsgHandler()->Subscribe(this, MSGTYPE_GUISKILL);
	psengine->GetMsgHandler()->Subscribe(this, MSGTYPE_SYSTEM);
	psengine->GetMsgHandler()->Subscribe(this, MSGTYPE_CRAFT_INFO);
	psengine->GetMsgHandler()->Subscribe(this, MSGTYPE_GUISTORAGE);
	psengine->GetMsgHandler()->Subscribe(this, MSGTYPE_GUIINVENTORY);

	textbox = dynamic_cast<pawsMessageTextBox*> (FindWidget("CraftBotMainText"));
	textbox->Show();
	OutputMsg(csString("Craft bot ready and waiting."));

	return true;
}
craftBot::~craftBot()
{
    psengine->GetMsgHandler()->Unsubscribe(this, MSGTYPE_MODE);
    psengine->GetMsgHandler()->Unsubscribe(this, MSGTYPE_GUISKILL);
    psengine->GetMsgHandler()->Unsubscribe(this, MSGTYPE_SYSTEM);
    psengine->GetMsgHandler()->Unsubscribe(this, MSGTYPE_CRAFT_INFO);
    psengine->GetMsgHandler()->Unsubscribe(this, MSGTYPE_GUISTORAGE);
    psengine->GetMsgHandler()->Unsubscribe(this, MSGTYPE_GUIINVENTORY);
}

void craftBot::OutputMsg(csString msg)
{
	if (textbox != NULL)
	{
		csString out = csString(">") + msg;
		textbox->Show();
		textbox->AddMessage(out.GetData(), textcolour);
	}
}

void craftBot::HandleMessage( MsgEntry* me )
{
	if (state == INACTIVE)
		return;

	switch (me->GetType() )
	{
	case MSGTYPE_MODE:
		{
			HandleMode(me);
			break;
		}
	case MSGTYPE_GUISKILL:
		{
			HandleGUISkill(me);
			break;
		}
	case MSGTYPE_SYSTEM:
		{
			HandleSystem(me);
			break;
		}
	case MSGTYPE_CRAFT_INFO:
		{
			HandleCraftInfo(me);
			break;
		}
	case MSGTYPE_GUISTORAGE:
		{
			HandleGUIStorage(me);
			break;
		}
	case MSGTYPE_GUIINVENTORY:
		{
			HandleGUIInventory(me);
			break;
		}
	}
}

void craftBot::HandleMode(MsgEntry* message)
{
	psModeMessage incoming(message);
	Notify1(LOG_USER, "CRAFT BOT: mod received.\n");
	if (incoming.mode == psModeMessage::PEACE || incoming.mode == psModeMessage::OVERWEIGHT)
	{
		if (state == COMBINING || state == CRAFTING)
			modReceived = modReceived + 1;
		Notify4(LOG_USER, "CRAFT BOT: state = %d peace or overweight mod received and modReceived =%d and skillReceived = %d.\n", state, modReceived, skillReceived);
	}
	if (modReceived >= EXPECTED_MODS && skillReceived>1)
		WrapUp();
}
void craftBot::HandleGUISkill(MsgEntry* message)
{
	psGUISkillMessage incoming(message);

	if (state == COMBINING || state == CRAFTING)
	{
		skillReceived = skillReceived + 1;
		if (modReceived >= EXPECTED_MODS && skillReceived>1)
				WrapUp();
	}
	else if (state == TRAINING)
	{
		trainingExpected = trainingExpected -1;
		if (trainingExpected < 1 && trainingSet)
			GoToNextState();
	}

}

void craftBot::HandleGUIInventory(MsgEntry* message)
{
	if (state == RETRIEVING_INGREDIENTS)
	{
		msgsInventoryToReceive = msgsInventoryToReceive -1;
		if (msgsInventoryToReceive == 0)
			GoToNextState();
	}
}

void craftBot::HandleSystem(MsgEntry* message)
{
	if (state != INACTIVE)// && state != ERROR)
	{
		psSystemMessage incoming(message);
		csString text = incoming.msgline;

		if (text.StartsWith(RANKING_PREFIX))
		{
			//ranked up, need to train, but first pause the crafting
			StopWorking();

			size_t end = text.FindLast('!');
			size_t start = text.FindLast(' ');
			csString skillName = text.Slice(start+1, end-start-1);

			OutputMsg(csString("Ranking message received, so I'll attempt to train."));
			state = TRAINING;
			trainingExpected = trainingExpected + 1;
			trainingSet = false;
			if (!worldHandler::TrainSkillMax(skillName))
			{
				OutputMsg(csString("No trainer found within reach, will just keep crafting."));
				trainingExpected = 0;
				GoToNextState();
			}
		}
		else if (text.StartsWith(NOT_IN_RANGE_STORAGE))
		{
			if (state == LEARNING_STORAGE)
			{
				//storage provider within range max, but not within storage cmd range...
				OutputMsg(csString("Storage provider not in range, will use inventory only."));
				inventoryOnly = true;
				GoToNextState();
			}
		}
		else if (text.StartsWith(TRAINING_RECEIVED) && state==TRAINING)
		{
			trainingSet = true;
			if (trainingExpected < 1)
				GoToNextState();
				//GoToState(COMBINING);
		}
		else if (text.StartsWith(NOT_IN_STORAGE))
		{
			//OutputMsg("Required ingredients not found in storage! Please re-supply and /craft again.");
			StopWorking();
			storage.EmptyStorage();
			state = INACTIVE;

			attemptsLeft = attemptsLeft - 1;
			if (attemptsLeft > 0)
			{
				OutputMsg("Failed to retrieve ingredients from storage! Will try again...");
				GoToNextState();
			}
			else
			{
				OutputMsg("Required ingredients not found in storage! Please re-supply and /craft again.");
				//state = INACTIVE;
			}
		}
		else if (text.StartsWith(WRONG_INGREDIENTS))
		{
			//wrong ingredients usually means there's some mix up of messages, attempt to re-target and recover
			RecoverFromError();
		}
		else if (text.StartsWith(WRONG_TARGET_FOR_USE))
		{
			OutputMsg(csString("Container not available, please move within range of the required container and /craft again."));
			state = ERROR;
		}
		else if (text.StartsWith(CONTAINER_WORN) || text.StartsWith(TOOL_WORN))
		{
			state = ERROR;
			OutputMsg(text);
			StopWorking();
		}
	}
}
void craftBot::HandleCraftInfo(MsgEntry* message)
{
	psMsgCraftingInfo incoming(message);
	csString itemDescription(incoming.craftInfo);

	if (state == READING_RECIPE)
	{
		OutputMsg(itemDescription);
		rcp.ParseRecipe(itemDescription);
		worldHandler::CloseOpenWindow("Read Book");
		GoToNextState();
	}
}
void craftBot::HandleGUIStorage(MsgEntry* message)
{
	if (state != LEARNING_STORAGE)
		return;

	psGUIStorageMessage incoming( message );

	switch (incoming.command)
	{
		case psGUIStorageMessage::STORAGE:
	    {
	        csRef<iDocumentSystem> xml =  csQueryRegistry<iDocumentSystem > ( PawsManager::GetSingleton().GetObjectRegistry());

	        csRef<iDocument> doc = xml->CreateDocument();
	        if(!doc)
	        {
	            OutputMsg(csString("Error in merchant data xml"));
	            state = ERROR;
	            return;
	        }

	        doc->Parse( incoming.commandData );
	        csRef<iDocumentNode> merchant = doc->GetRoot();
	        if(!merchant)
	        {
	        	OutputMsg(csString("Error in merchant data xml"));
	        	state = ERROR;
	            return;
	        }
	        csRef<iDocumentNode> data = merchant->GetNode("STORAGE");
	        if(!data)
	        {
	        	OutputMsg(csString("Error in merchant data xml"));
	        	state = ERROR;
	            return;
	        }

	        storage.SetMerchantID(data->GetAttributeValueAsInt( "ID" ));
	        storage.SetTradeCmd(data->GetAttributeValueAsInt("TRADE_CMD"));

	    	break;
	    }
	    case psGUIStorageMessage::CATEGORIES:
	    {
	    		csRef<iDocumentSystem> xml =  csQueryRegistry<iDocumentSystem> (PawsManager::GetSingleton().GetObjectRegistry());

	    		csRef<iDocument> catList  = xml->CreateDocument();

	    		const char* error = catList->Parse( incoming.commandData );
	    		if ( error )
	    		{
	    			OutputMsg("Error parsing the categories from storage.");
	    			state = ERROR;
	    			return;
	    		}

	    		csRef<iDocumentNode> root = catList->GetRoot();
	    		if (!root)
	    		{
	    			OutputMsg("Error getting the root of categories xml.");
	    			state = ERROR;
	    			return;
	    		}
	    		csRef<iDocumentNode> topNode = root->GetNode("L");
	    		if(!topNode)
	    		{
	    			OutputMsg("Error getting top node of categories xml.");
	    			state = ERROR;
	    			return;
	    		}
	    		csRef<iDocumentNodeIterator> iter = topNode->GetNodes();

	    		msgsItemsNeeded = 0;
	    		while ( iter->HasNext() )
	    		{
	    			csRef<iDocumentNode> category = iter->Next();

	    			if ( !category )
	    			{
	    				OutputMsg("Improper category found in xml.");
	    				state = ERROR;
	    				return;
	    			}
	    			storage.GetCategoriesList()->PushBack(category->GetAttributeValue("NAME"));
	    			msgsItemsNeeded = msgsItemsNeeded + 1;
	    		}
	    		OutputMsg(csString("Done retrieving categories for storage."));

	    		//for each category, ask the server for list of items...
	    		csList<csString>::Iterator catIter(*storage.GetCategoriesList());

	    		while (catIter.HasNext())
	    		{
	    			csString commandData;
	    			commandData.Format("<C ID=\"%d\" CATEGORY=\"%s\" />", storage.GetMerchantID(),
	                                       EscpXML(catIter.Next()).GetData());

	    			psGUIStorageMessage outgoing(psGUIStorageMessage::CATEGORY, commandData);
	    			outgoing.SendMessage();
	    		}

	    	break;
	    }
	    case psGUIStorageMessage::ITEMS:
	    {
	        csRef<iDocumentSystem> xml =  csQueryRegistry<iDocumentSystem> (PawsManager::GetSingleton().GetObjectRegistry());

	        csRef<iDocument> itemList  = xml->CreateDocument();

	        const char* error = itemList->Parse( incoming.commandData );
	        if ( error )
	        {
	            OutputMsg(csString("Error parsing items from storage."));
	            state = ERROR;
	            return;
	        }

	        csRef<iDocumentNode> root = itemList->GetRoot();
	        if(!root)
	        {
	            OutputMsg(csString("No root element in items xml."));
	            state = ERROR;
	            return;
	        }
	        csRef<iDocumentNode> topNode = root->GetNode("L");
	        if (!topNode)
	        {
	            OutputMsg(csString("No top node element in items xml."));
	            state = ERROR;
	            return;
	        }
	        csRef<iDocumentNodeIterator> iter = topNode->GetNodes();

	        while ( iter->HasNext() )
	        {
	            csRef<iDocumentNode> item = iter->Next();

	            csString name = item->GetAttributeValue("NAME");
	            csString id = item->GetAttributeValue("ID");
	            int quantity = atoi(item->GetAttributeValue("COUNT"));
	            storage.AddItem(name, id, quantity);
	        }
	     //   OutputMsg(csString("Learnt all items and categories from storage."));
	        msgsItemsNeeded = msgsItemsNeeded - 1;
	        if (msgsItemsNeeded == 0)	//we are done, going to next stage
	        	GoToNextState();

	    	break;
	    }
	    case psGUIStorageMessage::MONEY:
	    {
	    	break;
	    }
	}
}




bool craftBot::StartCraftRun(csString recipeName, int numberItems)
{
	Show();
	itemsToDo = numberItems;
	itemsDone = 0;
	attemptsLeft = 3;

	rcp.setRecipeName(recipeName);
	storage.EmptyStorage();
	GoToNextState();

	return true;
}

void craftBot::ResetAllFlags()
{
    int msgsInventoryToReceive = 0;
    int msgsItemsNeeded = 0;	//for retrieving the whole list of items + ids from storage

    //flags to navigate the messages since by themselves they are unreliable;
    int modReceived = false;
    int skillReceived = 0;
    int trainingExpected = 0;
    bool trainingSet = false;
}

void craftBot::StopWorking()
{
	worldHandler::Target(rcp.GetContainerName());
	worldHandler::OpenTarget();
	worldHandler::TakeAllFromTarget();
	ResetAllFlags();
	OutputMsg(csString("Bot stopped working."));
	state = INACTIVE;
}

bool craftBot::ReadRecipe()
{
	//find and equip recipe
	if (!worldHandler::IsInBrain(rcp.GetRecipeName()))
	{
		if (!worldHandler::Equip(rcp.GetRecipeName()))
		{
			OutputMsg(csString("Recipe not found! Please have your recipes in the inventory or already equipped."));
			GoToState(ERROR);
			return false;
		}
		OutputMsg(csString("Recipe equipped."));
	}
	else OutputMsg(csString("Recipe found in brain."));

	//read recipe
	state = READING_RECIPE;
	psViewItemDescription out(CONTAINER_INVENTORY_EQUIPMENT, PSCHARACTER_SLOT_MIND);
	out.SendMessage();
	OutputMsg(csString("Reading the recipe..."));
	return true;
}
bool craftBot::GetStorageContents()
{
	if (!worldHandler::IsWithinReach(STORAGE_HOLDER))
	{
		OutputMsg(csString("Storage holder not found. Will try to work with items in inventory only."));
		state = LEARNING_STORAGE;
		inventoryOnly = true;
		GoToNextState();
		return false;
	}

	inventoryOnly = false;
	csString cmd = csString("/target ") + STORAGE_HOLDER;
	psengine->GetCmdHandler()->Execute(cmd.GetData());

	state = LEARNING_STORAGE;
	psengine->GetCmdHandler()->Execute("/storage");
	psGUIStorageMessage exchange(psGUIStorageMessage::REQUEST,"<R TYPE=\"WITHDRAW\"/>");
	exchange.SendMessage();
	return true;
}
bool craftBot::Combine()
{
	if (itemsToDo > itemsDone)
		{
			char out[200];
			sprintf(out, "Done %d items, %d left to do.", itemsDone, itemsToDo-itemsDone);
			OutputMsg(csString(out));

			if (!worldHandler::Target(rcp.GetContainerName()))
			{
				OutputMsg(csString("Could not find container within reach!"));
				state = ERROR;
				return false;
			}

			if (!worldHandler::OpenTarget())
			{
				OutputMsg(csString("Could not open the container!"));
				state = ERROR;
				return false;
			}

			int toContainer = worldHandler::GetTargetID();
			if (toContainer < 0)
			{
				OutputMsg(csString("Lost target on container, please /craft again if the container is truly available and within reach."));
				state = ERROR;
				return false;
			}

			OutputMsg(csString("Moving ingredients to container for crafting"));
			//move ingredients from inventory to container
			//first check for any bundles

			psInventoryCache::CachedItemDescription* from = worldHandler::FindItemSlot(rcp.GetBundleName(), false);
			if (from)
			{
				worldHandler::MoveItems(from->containerID, from->slot, toContainer, 0, 1);
				state = COMBINING;
				modReceived = 0;
				skillReceived = 0;
				GoToNextState();
				return true;
			}
			csHash<int, csString>::GlobalIterator iterIngredients(rcp.GetIngredientsList()->GetIterator());

			int nextEmptySlot = 0;
			if (!iterIngredients.HasNext())
			{
				OutputMsg(csString("Empty ingredients list!"));
				state = ERROR;
				return false;
			}
			while (iterIngredients.HasNext())
				{
					csString itemName;
					int quantity = iterIngredients.Next(itemName);

					//psInventoryCache::CachedItemDescription*
					from = worldHandler::FindItemSlot(itemName, false);
					if (!from || from->stackCount < quantity)
					{
						if (!from)
						{
							OutputMsg(csString("Item not yet found in inventory! Will try again a bit later."));
							worldHandler::Target(rcp.GetContainerName());
							worldHandler::TakeAllFromTarget();
							state = RETRIEVING_INGREDIENTS;
							msgsInventoryToReceive = 1;

						}
						else OutputMsg(csString("Not enough items found! Please re-supply your storage and/or inventory and /craft again."));

						return false;
					}
					else
						worldHandler::MoveItems(from->containerID, from->slot, toContainer, nextEmptySlot, quantity);
					nextEmptySlot = nextEmptySlot + 1;
				}
			OutputMsg(csString("Done with ingredients. Starting to combine."));
			state = COMBINING;
			worldHandler::CombineContentsInTarget();
			//just to make sure it doesn't hang, ask for a retarget and use too
			modReceived = 0;
			skillReceived = 0;
			GoToNextState();
		}
	else
	{
		OutputMsg(csString("All items done, switching to inactive state."));
		state = INACTIVE;
		return false;
	}
	return true;
}
bool craftBot::Craft()
{
	if (!worldHandler::Target(rcp.GetContainerName()))
	{
		state = ERROR;
		return false;
	}
	state = CRAFTING;
	if (!worldHandler::UseTarget())
	{
		state = ERROR;
		return false;
	}
	OutputMsg(csString("Crafting..."));
	return true;
}

void craftBot::GoToState(BOT_STATES newState)
{

	state = BETWEEN_ACTIONS;
	switch (newState)
	{
		case READING_RECIPE:
		{
			ReadRecipe();
			break;
		}
		case LEARNING_STORAGE:
		{
			GetStorageContents();
			break;
		}
		case RETRIEVING_INGREDIENTS:
		{
			GetIngredients();
			break;
		}
		case COMBINING:
		{
			Combine();
			break;
		}
		case CRAFTING:
		{
			Craft();
			break;
		}
		default:
		{
			StopWorking();
		}
	}
}
bool craftBot::GetIngredients()
{
	if (itemsDone >= itemsToDo)
	{
		OutputMsg(csString("Finished craft run, switching to inactive."));
		state = INACTIVE;
		return true;
	}

	OutputMsg(csString("Getting all needed ingredients to inventory..."));
	state = RETRIEVING_INGREDIENTS;

	msgsInventoryToReceive = 0;

	csHash<int, csString>::GlobalIterator iterIngredients(rcp.GetIngredientsList()->GetIterator());

	//check first for any bundles, to make those first
	psInventoryCache::CachedItemDescription* from = worldHandler::FindItemSlot(rcp.GetBundleName(), false);
	if (from)
	{
		OutputMsg(csString("Bundle found in inventory - will craft that first."));
		GoToNextState();
		return true;
	}

	while (iterIngredients.HasNext())
		{
			csString itemName;
			int quantity = iterIngredients.Next(itemName);

			//look first in inventory
			OutputMsg(itemName);
			//psInventoryCache::CachedItemDescription*
			from = worldHandler::FindItemSlot(itemName, false);

			if (!from || from->stackCount < quantity)
				{
					if (!inventoryOnly)
					{
						OutputMsg(csString("Item not found in inventory or not enough items in inventory, will try to get them from storage."));

						int quantityNeeded = quantity;
						if (from)
							quantityNeeded = quantity - from->stackCount;
						int retrieved = storage.Retrieve(itemName, quantityNeeded);

						if (retrieved == 0)
						{
							OutputMsg(csString("Items not found in storage either, please re-supply and /craft again."));
							StopWorking();
							return true;
						}
						msgsInventoryToReceive = msgsInventoryToReceive + retrieved;
					}
					else
					{
						OutputMsg(csString("Item not found in inventory and storage not accessible. Please move next to a storage provider or get the ingredients in your inventory."));
						StopWorking();
					}
				}
		}

	if (msgsInventoryToReceive == 0)	//everything was already in the inventory, no need to wait
	{
		OutputMsg(csString("Everything found in inventory, starting to craft."));
		GoToNextState();
	}
	return true;
}

bool craftBot::WrapUp()
{
	Notify1(LOG_USER, "CRAFT BOT: wrapping up\n");
	itemsDone = itemsDone + 1;
	worldHandler::Target(rcp.GetContainerName());
	worldHandler::OpenTarget();
	worldHandler::TakeAllFromTarget();
	GoToState(RETRIEVING_INGREDIENTS);
}

void craftBot::RecoverFromError()
{
	state = BETWEEN_ACTIONS;
	worldHandler::Target(rcp.GetContainerName());
	worldHandler::TakeAllFromTarget();
	ResetAllFlags();

	GoToState(COMBINING);
}

void craftBot::GoToNextState()
{
	BOT_STATES nextState;

	switch (state)
	{
	case INACTIVE:
	{
		nextState = LEARNING_STORAGE;
		break;
	}
	case LEARNING_STORAGE:
	{
		nextState = READING_RECIPE;
		break;
	}
	case READING_RECIPE:
	{
		nextState = RETRIEVING_INGREDIENTS;
		break;
	}
	case RETRIEVING_INGREDIENTS:
	{
		nextState = COMBINING;
		break;
	}
	case COMBINING:
	{
		nextState = CRAFTING;
		break;
	}
	case TRAINING:
	{
		nextState = COMBINING;
		break;
	}
	default:
		nextState = ERROR;
	}
	GoToState(nextState);
}
