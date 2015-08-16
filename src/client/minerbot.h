/*
 * minerbot.h
 *      Grundin's enhanced miner (with much code reused from Foxy's craftbot)
 *
 *      It implements /mine command as enhancement for /explore. No user interface yet.
 *
 *      When invoked by /mine without parameters it does following once:
 *      - explores current spot - same as /explore
 *      - if a tiny or small claim is found, and raw material is available, it places it into the claim 
 *              and immediately builds the claim (TODO reading recipe)
 *      - if material is not available or larger claim is found, claim is locked
 *      
 *      Movement, repeated actions, storage or training are not implemented (must be done manually by user).
 *
 */
#ifndef MINER_BOT_HEADER
#define MINER_BOT_HEADER

#include "paws/pawswidget.h"
#include "recipe.h"
#include "botstorage.h"
#include "net/cmdbase.h"

const csString MINERBOT_CLASS = csString("minerBot");

/** A bot for enhanced exploring */
class minerBot : public psClientNetSubscriber, public iCmdSubscriber
{
public:
enum MINERBOT_STATES
{
        ERROR = -1,
        INACTIVE = 0,
        EXPLORING = 1,
        LEARNING_STORAGE = 2,
        BETWEEN_ACTIONS = 3,
        READING_RECIPE = 4,
        RETRIEVING_INGREDIENTS = 5,
        TO_CRAFT = 6,
        CRAFTING = 7,
        TRAINING = 8
};

    minerBot();
    virtual ~minerBot();

    bool PostSetup();
    const char *HandleCommand(const char *cmd);
    
    void HandleMessage( MsgEntry* message );

    //Crafting functions
    bool StartMine();
    bool tryAdvance();

private:

    botStorage storage;
    csString claimName;
    csString scrollName;
    EID markerId;

    MINERBOT_STATES state;
/*    //counter for inventory and storage messages since otherwise it's impossible to know when all the needed items have been updated.
    int msgsInventoryToReceive;
    int msgsItemsNeeded;        //for retrieving the whole list of items + ids from storage

    //flags to navigate the messages since by themselves they are unreliable;
    int exploReceived;
    int modReceived;
    int skillReceived;
//    int trainingExpected;
//    bool trainingSet;


    int attemptsLeft;

    bool inventoryOnly;
*/
    //own utilities
    void OutputMsg(csString msg);
    void GoToState(MINERBOT_STATES newState);
    void GoToNextState();
    //void RecoverFromError();
    //void ResetAllFlags();

    //handlers for each type of message intercepted
    void HandleMode(MsgEntry* message);
    //void HandleGUISkill(MsgEntry* message);
    void HandleSystem(MsgEntry* message);
    void HandleCraftInfo(MsgEntry* message);
    //void HandleGUIStorage(MsgEntry* message);
    //void HandleGUIInventory(MsgEntry* message);
    void HandlePersistItem(MsgEntry* message);

    //crafting steps
    //bool ReadRecipe();
    bool Explore();
    //bool GetStorageContents();
    bool GetIngredients();
    bool Combine();
    bool Craft();
    bool WrapUp();

};

//CREATE_PAWS_FACTORY( minerBot );

#endif

