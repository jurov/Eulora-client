/*
 * minerbot.cpp
 *
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

#include "minerbot.h"
#include "util/slots.h"
#include "worldhandler.h"
#include "util/log.h"

const csString NOT_IN_RANGE_STORAGE = csString("You are not in range to check your storage with");
const csString NOT_SUCCESSFUL = csString("You were not successful.");
const csString TOOL_WORN = csString("This tool is too worn to be useful");
const csString TOO_TIRED = csString("TODO");
const csString WORK_STOPPED = csString("You stopped working.");
const csString WRONG_INGREDIENTS = csString("You don't have the right ingredients");
const csString STORAGE_HOLDER = csString("Heina Draggenfort");
const csString NOT_IN_STORAGE = csString("Storage does not have");
const csString GOT_ITEM = csString("You got a");
const csString PLACED_ITEM = csString("You placed an");
const csString WRONG_TARGET_FOR_USE = csString("Only items can be targeted for use");
const csString TRAINING_RECEIVED = csString("You've received some ");
const csString ITEM_KEY = csString("Key");
const csString EXP_MARKER = csString("Exploration Marker");
const csString YOU_EXPLORE = csString("You start to explore");

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

minerBot::minerBot()
{
        markerId = 0;
        scrollName = (const char*)NULL;
        claimName = (const char*)NULL;
        state = INACTIVE;
        PostSetup();
        Debug1(LOG_USER,0,"MINER BOT init");
}

bool minerBot::PostSetup()
{
        psengine->GetMsgHandler()->Subscribe(this, MSGTYPE_MODE);
        //psengine->GetMsgHandler()->Subscribe(this, MSGTYPE_GUISKILL);
        psengine->GetMsgHandler()->Subscribe(this, MSGTYPE_SYSTEM);
        psengine->GetMsgHandler()->Subscribe(this, MSGTYPE_CRAFT_INFO);
        //psengine->GetMsgHandler()->Subscribe(this, MSGTYPE_GUISTORAGE);
        //psengine->GetMsgHandler()->Subscribe(this, MSGTYPE_GUIINVENTORY);
        psengine->GetMsgHandler()->Subscribe(this, MSGTYPE_PERSIST_ITEM);

        
        return true;
}
minerBot::~minerBot()
{
    Debug1(LOG_USER,0,"MINER BOT destruct");
    psengine->GetMsgHandler()->Unsubscribe(this, MSGTYPE_MODE);
    //psengine->GetMsgHandler()->Unsubscribe(this, MSGTYPE_GUISKILL);
    psengine->GetMsgHandler()->Unsubscribe(this, MSGTYPE_SYSTEM);
    psengine->GetMsgHandler()->Unsubscribe(this, MSGTYPE_CRAFT_INFO);
    //psengine->GetMsgHandler()->Unsubscribe(this, MSGTYPE_GUISTORAGE);
    //psengine->GetMsgHandler()->Unsubscribe(this, MSGTYPE_GUIINVENTORY);
    psengine->GetMsgHandler()->Unsubscribe(this, MSGTYPE_PERSIST_ITEM);
}

void minerBot::OutputMsg(csString msg)
{
        /*if (textbox != NULL)
        {
                csString out = csString(">") + msg;
                textbox->Show();
                textbox->AddMessage(out.GetData(), textcolour);
        }*/
}

const char* minerBot::HandleCommand(const char* cmd)
{
    if(csString(cmd).Compare("_frame_")){
      if(state != INACTIVE){
        tryAdvance();
      }
      
    }else{
      if(csString(cmd).StartsWith("/mine"))
      {
        Debug2(LOG_USER,0,"MINER BOT handleCommand: %s", cmd);
        StartMine();
      }
    }
}


void minerBot::HandleMessage( MsgEntry* me )
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
        case MSGTYPE_PERSIST_ITEM:
              {
                  HandlePersistItem(me);
                  break;
              }

        }
}

void minerBot::HandleMode(MsgEntry* message)
{
  if(state != INACTIVE){
        psModeMessage incoming(message);
        Notify1(LOG_USER, "MINER BOT: mod received.\n");
        if (incoming.mode == psModeMessage::PEACE || incoming.mode == psModeMessage::OVERWEIGHT)
        {
                Notify2(LOG_USER, "MINER BOT: state = %d peace or overweight\n", state);
                if (state == CRAFTING){
                  worldHandler::TakeAllFromTarget();
                  GoToNextState();
                }
            
        }
  }
}

bool minerBot::tryAdvance()
{
  switch(state){
    case EXPLORING:
    {
            //Debug4(LOG_USER,0,"MINER BOT tryAdvance %d %s %s", markerId, scrollName.GetData(), claimName.GetData());
        if (markerId != 0 && !scrollName.IsEmpty() && !claimName.IsEmpty()) 
        {
          if(worldHandler::OpenTargetEID(markerId)){
            Debug4(LOG_USER,0,"MINER BOT switch to retrieving %d %s %s", markerId, scrollName.GetData(), claimName.GetData());
          
            GoToNextState();
          }else{
            Debug1(LOG_USER,0,"MINER BOT opentarget failed");
          }
        }
      break;
    }
    case TO_CRAFT:
    {
            Debug1(LOG_USER,0,"MINER BOT tryAdvance TO_CRAFT");
            bool res;
            if (claimName.StartsWith("Tiny")){
              res = worldHandler::UseTarget();
            }else{
              res = worldHandler::CombineContentsInTarget();
            }
            if (res){
              state = CRAFTING;
            }
    }
  }
  return true;
  
}

void minerBot::HandleSystem(MsgEntry* message)
{
        if (state != INACTIVE)// && state != ERROR)
        {
                psSystemMessage incoming(message);
                csString text = incoming.msgline;
                if (state == EXPLORING){
                    if(text == NOT_SUCCESSFUL or text == TOOL_WORN or text == TOO_TIRED){
                      WrapUp();
                    }else if(text.StartsWith(GOT_ITEM)){
                        size_t end = text.FindLast('!');
                        size_t start = GOT_ITEM.Length();                                                                                                                                                                                                                                                                                                                                                                        
                        csString thingName = text.Slice(start+1, end-start-1);
                        if (thingName != ITEM_KEY and thingName.EndsWith("Ennumeration")){
                          scrollName = thingName;
                          Debug2(LOG_USER,0,"MINER BOT Got: %s", thingName.GetData());
                        }else{
                          Debug2(LOG_USER,0,"MINER BOT msg Ignored: %s", text.GetData());
                        }
                        
                    } else if(text.StartsWith(PLACED_ITEM)){
                        size_t end = text.FindLast('!');
                        size_t start = PLACED_ITEM.Length();                                                                                                                                                                                                                                                                                                                                                                        
                        csString thingName = text.Slice(start+1, end-start-1);
                        if (thingName.StartsWith("Tiny") || thingName.StartsWith("Small")){
                          claimName = thingName;
                          Debug2(LOG_USER,0,"MINER BOT Placed: %s", thingName.GetData());
                        }else{
                          Debug2(LOG_USER,0,"MINER BOT item Ignored: %s", thingName.GetData());
                          WrapUp();
                        }
                        //GoToNextState();
                    }
                }
        }
}
void minerBot::HandleCraftInfo(MsgEntry* message)
{
}

void minerBot::HandlePersistItem(MsgEntry* me)
{
    if(state == EXPLORING){
       psPersistItem incoming(me, psengine->GetNetManager()->GetConnection()->GetAccessPointers());
       me->Reset();//otherwise GEM fails to process and show the claim, wtf
       if (incoming.name.EndsWith(EXP_MARKER)){
         if(markerId == 0){
            markerId = incoming.eid;
            Debug2(LOG_USER,0, "MINER BOT: got marker: %d",markerId);
         }else{
            Notify1(LOG_USER, "MINER BOT: marker confusion, aborting\n");
            WrapUp();
         }
       }
    }
}


bool minerBot::StartMine()
{
  Explore();
}

bool minerBot:: Explore()
{
      psWorkCmdMessage work("/explore");
      work.SendMessage();
      state = EXPLORING;
}

bool minerBot::Craft()
{
        Debug2(LOG_USER,0,"MINER BOT Craft: %s",claimName.GetData());
        state = TO_CRAFT;
        if (worldHandler::TargetEID(markerId))
        {
          return true;
        }
        Debug2(LOG_USER,0,"MINER BOT Target failed: %s",claimName.GetData());
        WrapUp();
        return false;
}

bool minerBot::Combine()
{
//TODO small claims
}


void minerBot::GoToState(MINERBOT_STATES newState)
{

        state = BETWEEN_ACTIONS;
        switch (newState)
        {
/*                case READING_RECIPE:
                {
                        ReadRecipe();
                        break;
                }*/
                case EXPLORING:
                {
                        Explore();
                        break;
                }
                case RETRIEVING_INGREDIENTS:
                {
                        GetIngredients();
                        break;
                }
                case TO_CRAFT:{
                  Craft();
                  break;
                }
                case CRAFTING:
                {
                  //nothing here, action deferred
//                        if (Craft())
                          break;
                }
                default:
                {
                        WrapUp();
                }
        }
}
bool minerBot::GetIngredients()
{
    Debug1(LOG_USER,0,"MINER BOT retrieving ingredients");
    state = RETRIEVING_INGREDIENTS;

    OutputMsg(csString("Getting all needed ingredients to inventory..."));
    if (!worldHandler::IsInBrain(scrollName))
    {
            if (!worldHandler::Equip(scrollName))
            {
                    Notify1(LOG_USER,"Recipe not found! Please have your recipes in the inventory or already equipped.");
                    WrapUp();
                    return false;
            }
    }
    Debug1(LOG_USER,0,"MINER BOT Recipe equipped.");

    csString itemName;
    int quantity = 0;
    if(claimName.StartsWith("Tiny")){
        itemName = csString("Little Bit O' Nothing");
        quantity = 1;
    }else if(claimName.StartsWith("Small")){
        itemName = csString("Coarse Frangible Thread");
        quantity = 7;
    }else{
        Notify1(LOG_USER, "MINER BOT: Unknown claim size");
        WrapUp();
        return false;
    }

    //look first in inventory
    OutputMsg(itemName);
    psInventoryCache::CachedItemDescription* from = worldHandler::FindItemSlot(itemName, false);

    if (!from || from->stackCount < quantity)
    {
        Notify1(LOG_USER, "MINER BOT: Item not found in inventory");
        WrapUp();
        return false;
    }
    
    Debug1(LOG_USER,0,"MINER BOT: Item found")
    
    worldHandler::MoveItems(from->containerID, from->slot, markerId.Unbox(), 0, quantity);

    OutputMsg(csString("Everything found in inventory, starting to craft."));
    GoToNextState();
    return true;
}

bool minerBot::WrapUp()
{
        Notify1(LOG_USER, "MINER BOT: wrapping up\n");
        claimName = (const char*)NULL;
        scrollName = (const char*)NULL;
        markerId = 0;
        state = INACTIVE;
}

void minerBot::GoToNextState()
{
        MINERBOT_STATES nextState;

        switch (state)
        {
        case INACTIVE:
        {
                nextState = EXPLORING;
                break;
        }
        case EXPLORING:
        {
                nextState = RETRIEVING_INGREDIENTS;
                break;
        }
        case RETRIEVING_INGREDIENTS:
        {
                nextState = TO_CRAFT;
                break;
        }
        case TO_CRAFT:
        {
                nextState = CRAFTING;
                break;
        }
        case CRAFTING:
        {
                nextState = INACTIVE;
                break;
        }
        default:
        {
                nextState = ERROR;
        }
        }
        Notify3(LOG_USER, "MINER BOT: state: %d nextState: %d",state,nextState);
        GoToState(nextState);
}

