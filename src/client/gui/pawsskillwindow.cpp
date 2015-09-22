/*
 * pawsskillwindow.cpp
 *
 * Copyright (C) 2003 Atomic Blue (info@planeshift.it, http://www.atomicblue.org)
 *
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation (version 2 of the License)
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */

#include <psconfig.h>
#include <imesh/spritecal3d.h>

#include "globals.h"
#include "pscelclient.h"
#include "clientvitals.h"
#include "../charapp.h"
#include "net/messages.h"
#include "net/clientmsghandler.h"
#include "util/skillcache.h"
#include "util/strutil.h"
#include "util/psxmlparser.h"
#include "util/log.h"

// PAWS INCLUDES
#include "pawsskillwindow.h"
#include "pawschardescription.h"
#include "paws/pawstextbox.h"
#include "paws/pawslistbox.h"
#include "paws/pawsmanager.h"
#include "paws/pawsobjectview.h"
#include "paws/pawsprogressbar.h"
#include "gui/pawscontrolwindow.h"



#define BTN_BUY       100
#define BTN_FILTER    101
#define BTN_BUYLVL    103
#define BTN_STATS    1000 ///< Stats button for the tab panel
//#define BTN_COMBAT   1001 ///< Combat button for the tab panel
#define BTN_MAGIC    1002 ///< Magic button for the tab panel
//#define BTN_JOBS     1003 ///< Jobs button for the tab panel
//#define BTN_VARIOUS  1004 ///< Various button for the tab panel
//#define BTN_FACTION  1005
#define MAX_CAT         6
#define BTN_GATHER   1001 ///< gather button for the tab panel
#define BTN_CRAFT     1003 ///< craft button for the tab panel
#define BTN_FAITH  1004 ///< Faith for the tab panel
#define BTN_LEAD  1005
#define BTN_UTILITY  1009
#define BTN_FIGHT  1008

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

pawsSkillWindow::pawsSkillWindow()
{
    selectedSkill.Clear();
    skillString.Clear();
    skillDescriptions.DeleteAll();
    filter = false;
    train = false;
    foundSelected = false;

    hitpointsMax = 0;
    bloodpointsMax = 0;
    manaMax = 0;
    spiritMax = 0;
//    hitpoints = 0;
//    bloodpoints = 0;
//    mana = 0;
 //   spirit = 0;
    physStaminaMax = 0;
    menStaminaMax = 0;

    factRequest = false;
    charApp = new psCharAppearance(PawsManager::GetSingleton().GetObjectRegistry());
}
pawsSkillWindow::pawsSkillWindow(const pawsSkillWindow& origin)
{

}
pawsSkillWindow::~pawsSkillWindow()
{
    // Delete cached skill description strings
    csHash<psSkillDescription *>::GlobalIterator i = skillDescriptions.GetIterator();
    while (i.HasNext())
    {
        delete i.Next();
    }
    skillDescriptions.DeleteAll();
    delete charApp;
}

bool pawsSkillWindow::PostSetup()
{
    // Setup the Doll
    if ( !SetupDoll() )
    {
        return false;
    }

    xml =  csQueryRegistry<iDocumentSystem > ( PawsManager::GetSingleton().GetObjectRegistry());


    schoSkillList        = (pawsListBox*)FindWidget("SchoSkillList");
    schoSkillDescription = (pawsMultiLineTextBox*)FindWidget("SchoDescription");

    fightSkillList        = (pawsListBox*)FindWidget("FightSkillList");
    fightSkillDescription = (pawsMultiLineTextBox*)FindWidget("FightDescription");

    gatherSkillList        = (pawsListBox*)FindWidget("GatherSkillList");
    gatherSkillDescription = (pawsMultiLineTextBox*)FindWidget("GatherDescription");

    craftSkillList        = (pawsListBox*)FindWidget("CraftSkillList");
    craftSkillDescription = (pawsMultiLineTextBox*)FindWidget("CraftDescription");

    magicSkillList        = (pawsListBox*)FindWidget("MagicSkillList");
    magicSkillDescription = (pawsMultiLineTextBox*)FindWidget("MagicDescription");

    faithSkillList        = (pawsListBox*)FindWidget("FaithSkillList");
    faithSkillDescription = (pawsMultiLineTextBox*)FindWidget("FaithDescription");

    leadSkillList        = (pawsListBox*)FindWidget("LeadSkillList");
    leadSkillDescription = (pawsMultiLineTextBox*)FindWidget("LeadDescription");

    utilitySkillList        = (pawsListBox*)FindWidget("UtilitySkillList");
    utilitySkillDescription = (pawsMultiLineTextBox*)FindWidget("UtilityDescription");






    hpBar = dynamic_cast <pawsProgressBar*> (FindWidget("HPBar"));
    bpBar = dynamic_cast <pawsProgressBar*> (FindWidget("BPBar"));
    manaBar = dynamic_cast <pawsProgressBar*> (FindWidget("ManaBar"));
    spiritBar = dynamic_cast <pawsProgressBar*> (FindWidget("SpiritBar"));
    pysStaminaBar = dynamic_cast <pawsProgressBar*> (FindWidget("PysStaminaBar"));
    menStaminaBar = dynamic_cast <pawsProgressBar*> (FindWidget("MenStaminaBar"));
    experienceBar = dynamic_cast <pawsProgressBar*> (FindWidget("ExperienceBar"));

    hpFrac = dynamic_cast <pawsTextBox*> (FindWidget("HPFrac"));
    bpFrac = dynamic_cast <pawsTextBox*> (FindWidget("BPFrac"));
    manaFrac = dynamic_cast <pawsTextBox*> (FindWidget("ManaFrac"));
    spiritFrac = dynamic_cast <pawsTextBox*> (FindWidget("SpiritFrac"));
    pysStaminaFrac = dynamic_cast <pawsTextBox*> (FindWidget("PysStaminaFrac"));
    menStaminaFrac = dynamic_cast <pawsTextBox*> (FindWidget("MenStaminaFrac"));
    experiencePerc = dynamic_cast <pawsTextBox*> (FindWidget("ExperiencePerc"));

    if ( !hpBar  || !bpBar  || !manaBar ||  !spiritBar ||!pysStaminaBar || !menStaminaBar || !experienceBar
         || !hpFrac  || !manaFrac  || !pysStaminaFrac || !menStaminaFrac || !experiencePerc )
    {
        return false;
    }

    hpBar->SetTotalValue(1);
    bpBar->SetTotalValue(1);
    manaBar->SetTotalValue(1);
    spiritBar->SetTotalValue(1);
    pysStaminaBar->SetTotalValue(1);
    menStaminaBar->SetTotalValue(1);
    experienceBar->SetTotalValue(1);

    currentTab =0;
    previousTab =0;

    psengine->GetMsgHandler()->Subscribe(this, MSGTYPE_GUISKILL);
    psengine->GetMsgHandler()->Subscribe(this, MSGTYPE_FACTION_INFO);

    return true;
}

bool pawsSkillWindow::SetupDoll()
{
    pawsObjectView* widget = dynamic_cast<pawsObjectView*>(FindWidget("Doll"));
    GEMClientActor* actor = psengine->GetCelClient()->GetMainPlayer();
    if (!widget || !actor)
    {
        return true; // doll not wanted, not an error
    }

    // Set the doll view
    while(!widget->View(actor->GetFactName()))
    {
        continue;
    }

    CS_ASSERT(widget->GetObject()->GetMeshObject());

    // Set the charApp.
    widget->SetCharApp(charApp);

    // Register this doll for updates
    widget->SetID(actor->GetEID().Unbox());

    csRef<iSpriteCal3DState> spstate = scfQueryInterface<iSpriteCal3DState> (widget->GetObject()->GetMeshObject());
    if (spstate)
    {
        // Setup cal3d to select random 0 velocity anims
        spstate->SetVelocity(0.0,&psengine->GetRandomGen());
    }

    charApp->SetMesh(widget->GetObject());

    charApp->ApplyTraits(actor->traits);
    charApp->ApplyEquipment(actor->equipment);

    widget->EnableMouseControl(true);

    //return (a && e);
    return true;
}

/*void pawsSkillWindow::HandleFactionMsg(MsgEntry* me)
{
    psFactionMessage factMsg(me);
    csString ratingStr;
    csList<csString> rowEntry;

    if ( factMsg.cmd == psFactionMessage::MSG_FULL_LIST )
    {
        // Flag that we have received our faction information.
        factRequest = true;

        factions.DeleteAll();
        factionList->Clear();

        //Put all factions in the list.
        for ( size_t z = 0; z < factMsg.factionInfo.GetSize(); z++ )
        {
            FactionRating *fact = new FactionRating;
            fact->name = factMsg.factionInfo[z]->faction;
            fact->rating = factMsg.factionInfo[z]->rating;
            factions.Push(fact);

            rowEntry.PushBack(fact->name);
            ratingStr.Format("%d", fact->rating);
            rowEntry.PushBack(ratingStr);
            fact->row = factionList->NewTextBoxRow(rowEntry);
        }
    }
    else if (factRequest)   // ignore MSG_UPDATE if weve not had full list first
    {
        //Put all the faction updates into the gui.
        for ( size_t z = 0; z < factMsg.factionInfo.GetSize(); z++ )
        {
            bool found = false;

            // Find out where that faction is in our list
            size_t idx = 0;
            for (size_t t = 0; t < factions.GetSize(); t++ )
            {
                if ( factions[t]->name == factMsg.factionInfo[z]->faction )
                {
                    factions[t]->rating = factMsg.factionInfo[z]->rating;
                    found = true;
                    idx = t;
                    break;
                }
            }

            // If it was found above then we can just update it.
            if (found)
            {
                pawsListBoxRow* row = factions[idx]->row;
                if (row)
                {
                    pawsTextBox* rank = dynamic_cast <pawsTextBox*> (row->GetColumn(1));
                    if (rank == NULL)
                    {
                        return;
                    }

                    ratingStr.Format("%d", factions[idx]->rating);
                    rank->SetText(ratingStr);
                }
            }
            else
            {
                // We don't know about this faction level yet so add it in.
                FactionRating *fact = new FactionRating;
                fact->name = factMsg.factionInfo[z]->faction;
                fact->rating = factMsg.factionInfo[z]->rating;
                factions.Push(fact);

                rowEntry.PushBack(fact->name);
                ratingStr.Format("%d", fact->rating);
                rowEntry.PushBack(ratingStr);
                fact->row = factionList->NewTextBoxRow(rowEntry);
            }
        }
    }
}
*/
void pawsSkillWindow::HandleMessage( MsgEntry* me )
{
    switch ( me->GetType() )
    {
        /*case MSGTYPE_FACTION_INFO:
        {
            HandleFactionMsg(me);
            break;
        }*/

        case MSGTYPE_GUISKILL:
        {
            psGUISkillMessage incoming(me);

            switch (incoming.command)
            {
                case psGUISkillMessage::SKILL_LIST:
                {
                    bool flush = (train != incoming.trainingWindow) || incoming.openWindow;
                    train=incoming.trainingWindow;
                    if (train)
                    {
                        factRequest = false;
                        factions.DeleteAll();
                        //factionList->Clear();
                    }

                    skillString = "no";
                    if (!IsVisible() && incoming.openWindow)
                    {
                        Show();
                    }
                    skillString = incoming.commandData;
                    skillCache.apply(&incoming.skillCache);

                    int selectedRowIdx = -1;
                    HandleSkillList(&skillCache, incoming.focusSkill, &selectedRowIdx, flush);

                    if (selectedRowIdx > 0)
                    {
                        SelectSkill(selectedRowIdx, incoming.skillCat);
                    }

                    hitpointsMax = incoming.hitpointsMax;

                    bloodpointsMax = incoming.bloodpointsMax;
                    manaMax = incoming.manaMax;
                    spiritMax = incoming.spiritMax;
/*                    hitpoints = incoming.hitpoints;

                    bloodpoints = incoming.bloodpoints;
                    mana = incoming.mana;
                    spirit = incoming.spirit;
*/
                    physStaminaMax = incoming.physStaminaMax;
                    menStaminaMax = incoming.menStaminaMax;

                    break;
                }

                case psGUISkillMessage::DESCRIPTION:
                {
                    HandleSkillDescription(incoming.commandData);
                    break;
                }
            }
        }
    }
}

void pawsSkillWindow::Close()
{
    Hide();


    schoSkillList->Clear();
    fightSkillList->Clear();
    gatherSkillList->Clear();
    craftSkillList->Clear();
    magicSkillList->Clear();
    faithSkillList->Clear();
    leadSkillList->Clear();
    utilitySkillList->Clear();

    skillString.Clear();
    selectedSkill.Clear();
    unsortedSkills.DeleteAll();
}

bool pawsSkillWindow::OnButtonPressed(int /*mouseButton*/, int /*keyModifier*/, pawsWidget* widget)
{
    switch ( widget->GetID() )
    {
        case BTN_FILTER:
        {
            filter = !filter;
            HandleSkillList(&skillCache);
            return true;
        }

        case BTN_BUY:
        {
            BuySkill();
            return true;
        }

        case BTN_BUYLVL:
        {
            BuyMaxSkill();
            return true;
        }

        case BTN_STATS:
        
        case BTN_FIGHT:
        case BTN_UTILITY:
        case BTN_LEAD:
        case BTN_GATHER:
        case BTN_MAGIC:
        case BTN_CRAFT:
        case BTN_FAITH:

        {
            previousTab = currentTab; //Keep the selection if we are hitting the same tab where we are.
            currentTab = widget->GetID() - 1000;
            if (previousTab != currentTab)
            {
                selectedSkill.Clear();
            }
            break;
        }

    }
    return false;
}

void pawsSkillWindow::SelectSkill(int skill, int cat)
{
    if(skill < 0 || skill >= (int)unsortedSkills.GetSize() || cat < 0 || cat > MAX_CAT)
    {
        return;
    }

    //Let's see which category the skill belongs to
    switch(cat)
    {
        case 0: //scho
        {
            schoSkillList->Select(unsortedSkills[skill]);
            break;
        }
    case 1: //Combat skills
        {
            fightSkillList->Select(unsortedSkills[skill]);
            break;
        }
    case 2: //Magic Skills
        {
            utilitySkillList->Select(unsortedSkills[skill]);
            break;
        }
    case 3://Jobs Skills
        {
            leadSkillList->Select(unsortedSkills[skill]);
            break;
        }
    case 4://Various Skills
        {
            gatherSkillList->Select(unsortedSkills[skill]);
            break;
        }
    case 5://Various Skills
        {
            magicSkillList->Select(unsortedSkills[skill]);
            break;
        }
    case 6://Various Skills
        {
            craftSkillList->Select(unsortedSkills[skill]);
            break;
        }
    case 7://Various Skills
        {
            faithSkillList->Select(unsortedSkills[skill]);
            break;
        }
    }
}

void pawsSkillWindow::HandleSkillList(psSkillCache *skills, int selectedNameId, int *rowIdx, bool flush)
{
    if (!flush && !skills->isModified())
        return;

    if (skills->hasRemoved() || unsortedSkills.IsEmpty())
    {
        flush = true;
    }

    if (flush)
    {
        // Clear descriptions

    	schoSkillDescription->SetText("");
        fightSkillDescription->SetText("");
        utilitySkillDescription->SetText("");
        leadSkillDescription->SetText("");
        gatherSkillDescription->SetText("");
        magicSkillDescription->SetText("");
        craftSkillDescription->SetText("");
        faithSkillDescription->SetText("");


        // Clear everything on a flush
    	schoSkillList->Clear();
        fightSkillList->Clear();
        utilitySkillList->Clear();
        leadSkillList->Clear();
        gatherSkillList->Clear();
        magicSkillList->Clear();
        craftSkillList->Clear();
        faithSkillList->Clear();


        unsortedSkills.DeleteAll();
    }

    x = skills->getProgressionPoints();
    foundSelected = false;

    int idx = 0; // Row index counter
    psSkillCacheIter p = skills->iterBegin();
    while (p.HasNext())
    {
        psSkillCacheItem *skill = p.Next();

        if (rowIdx && (int)skill->getNameId() == selectedNameId)
        {
            *rowIdx = idx; // Row index of the selected skill
        }

        if (flush || skills->isModified())
        {
            switch (skill->getCategory()-1)
            {

                case 0://Stats
                {
                    HandleSkillCategory(schoSkillList, "SchoIndicator", "Stats Button", skill, idx, flush);
                    break;
                }
                case 1://Combat skills
                {
                    HandleSkillCategory(fightSkillList, "FightIndicator", "Fight Button", skill, idx, flush);
                    break;
                }
                case 2://Magic skills
                {
                    HandleSkillCategory(utilitySkillList, "UtilityIndicator", "Utility Button", skill, idx, flush);
                    break;
                }
                case 3://Jobs skills
                {
                    HandleSkillCategory(leadSkillList, "LeadIndicator", "Lead Button", skill, idx, flush);
                    break;
                }
                case 4://Various skills
                {
                    HandleSkillCategory(gatherSkillList, "GatherIndicator", "Gather Button", skill, idx, flush);
                    break;
                }
                case 5://Lead skills
                {
                    HandleSkillCategory(magicSkillList, "MagicIndicator", "Magic Button", skill, idx, flush);
                    break;
                }
                case 6://fight skills
                {
                    HandleSkillCategory(craftSkillList, "CraftIndicator", "Craft Button", skill, idx, flush);
                    break;
                }
                case 7://utility skills
                {
                    HandleSkillCategory(faithSkillList, "FaithIndicator", "Faith Button", skill, idx, flush);
                    break;
                }
            }
        }
    }
    
    if (flush)
    {
    	schoSkillList->SetSortedColumn(0);
        fightSkillList->SetSortedColumn(0);
        utilitySkillList->SetSortedColumn(0);
        leadSkillList->SetSortedColumn(0);
        gatherSkillList->SetSortedColumn(0);
        magicSkillList->SetSortedColumn(0);
        craftSkillList->SetSortedColumn(0);
        faithSkillList->SetSortedColumn(0);


       
    }

    if (flush && !foundSelected)
    {
        selectedSkill.Clear();
    }
    
    skills->setRemoved(false);
    skills->setModified(false);
}

void pawsSkillWindow::HandleSkillDescription( csString& description )
{
    /* Example message:
    <DESCRIPTION NAME="Climbing" DESC="This skill enable user to climb." />
    */
    csRef<iDocument> descDoc = xml->CreateDocument();

    const char* error = descDoc->Parse( description );
    if ( error )
    {
        Error2( "Error Parsing Skill Description: %s\n", error );
        return;
    }

    csRef<iDocumentNode> root = descDoc->GetRoot();
    if (!root)
    {
        Error1("No XML root in Skill Description");
        return;
    }
    csRef<iDocumentNode> desc = root->GetNode("DESCRIPTION");
    if(!desc)
    {
        Error1("No <DESCRIPTION> tag in Skill Description");
        return;
    }

    const char *nameStr = "";
    nameStr = desc->GetAttributeValue("NAME");

    const char* descStr = "";
    descStr = desc->GetAttributeValue("DESC");
    if((!descStr) || (!strcmp(descStr,""))|| (!strcmp(descStr,"(null)")))
    {
        descStr = "No description available";
    }
    int skillCategory = desc->GetAttributeValueAsInt("CAT");

    // Add to the cache
    if (nameStr && strcmp(nameStr, "") && strcmp(nameStr, "(null)"))
    {
        csStringID skillNameId = (csStringID)psengine->FindCommonStringId(nameStr);
        if (skillNameId != csInvalidStringID)
        {
            if (skillDescriptions.Contains(skillNameId))
            {
                psSkillDescription *p = skillDescriptions.Get(skillNameId, NULL);
                if (p)
                {
                    p->Update(skillCategory, descStr);
                }
            }
            else
            {
                psSkillDescription *item = new psSkillDescription(skillCategory, descStr);
                skillDescriptions.Put(skillNameId, item);
            }
        }
    }

    switch (skillCategory)
    {
        case 0://Stats
        {
             schoSkillDescription->SetText(descStr);
             break;
        }
        case 1://Combat skills
        {
            fightSkillDescription->SetText( descStr );
            break;
        }
        case 2://Magic skills
        {
            utilitySkillDescription->SetText(descStr);
            break;
        }
        case 3://Jobs skills
        {
            leadSkillDescription->SetText(descStr);
            break;
        }
        case 4://Various skills
        {
            gatherSkillDescription->SetText(descStr);
            break;
        }
        case 5://Factions
        {
            magicSkillDescription->SetText(descStr);
            break;
        }
        case 6://Factions
        {
            craftSkillDescription->SetText(descStr);
            break;
        }
        case 7://Factions
        {
            faithSkillDescription->SetText(descStr);
            break;
        }
    }
}

void pawsSkillWindow::OnNumberEntered(const char* /*name*/, int /*param*/, int number)
{
    csString commandData;

    if(number == -1)
        return;

    if (selectedSkill.IsEmpty())
    {
        PawsManager::GetSingleton().CreateWarningBox("You have to select a skill to buy.");
        return;
    }

    commandData.Format("<B NAME=\"%s\" AMOUNT=\"%d\"/>", EscpXML(selectedSkill).GetData(), number);
    psGUISkillMessage outgoing(psGUISkillMessage::BUY_SKILL, commandData);
    outgoing.SendMessage();
}

void pawsSkillWindow::BuyMaxSkill()
{
    csString commandData;
    if (selectedSkill.IsEmpty())
    {
        PawsManager::GetSingleton().CreateWarningBox("You have to select a skill to buy.");
        return;
    }

    csStringID skillId = psengine->FindCommonStringId(selectedSkill);
    if (skillId == csInvalidStringID)
    {
        PawsManager::GetSingleton().CreateWarningBox("You have to select a skill to buy.");
        return;
    }

    psSkillCacheItem* currSkill = skillCache.getItemBySkillId(skillId);
    unsigned short possibleTraining = currSkill->getKnowledgeCost() - currSkill->getKnowledge();

    if (skillCache.getProgressionPoints() < possibleTraining)
        possibleTraining = skillCache.getProgressionPoints();
        
    //check for 0 pp
/*    if(!possibleTraining)
    {
        if(!skillCache.getProgressionPoints())
            PawsManager::GetSingleton().CreateWarningBox("You don't have PP to train.");
        else
            PawsManager::GetSingleton().CreateWarningBox("You can't train this skill anymore.");
        return;
    }
*/
    commandData.Format("<B NAME=\"%s\" AMOUNT=\"%d\"/>", EscpXML(selectedSkill).GetData(), possibleTraining);
    psGUISkillMessage msg(psGUISkillMessage::BUY_SKILL, commandData);
    msg.SendMessage();
}

void pawsSkillWindow::BuySkill()
{
    if (selectedSkill.IsEmpty())
    {
        PawsManager::GetSingleton().CreateWarningBox("You have to select a skill to buy.");
        return;
    }

    csStringID skillId = psengine->FindCommonStringId(selectedSkill);
    if (skillId == csInvalidStringID)
    {
        PawsManager::GetSingleton().CreateWarningBox("You have to select a skill to buy.");
        return;
    }
    psSkillCacheItem* currSkill = skillCache.getItemBySkillId(skillId);
    if(!currSkill)
    {
        PawsManager::GetSingleton().CreateWarningBox("You have to select a skill to buy.");
        return;
    }
    unsigned short possibleTraining = currSkill->getKnowledgeCost() - currSkill->getKnowledge();
/*
    if (skillCache.getProgressionPoints() < possibleTraining)
        possibleTraining = skillCache.getProgressionPoints();
    
    //check for 0 pp
    if(!possibleTraining)
    {
        if(!skillCache.getProgressionPoints())
            PawsManager::GetSingleton().CreateWarningBox("You don't have PP to train.");
        else
            PawsManager::GetSingleton().CreateWarningBox("You can't train this skill anymore.");
        return;
    }
*/
    pawsNumbersPromptWindow::Create("Training Percent", 0, 1, 100, this, "Training Percent");
/*pawsNumbersPromptWindow * Create(const csString & label,
                                           int number, int minNumber, int maxNumber, 
                                           iOnNumberEnteredAction * action,const char *name, int param=0);

*/
}
////label, number, minNumber, maxNumber, action,name,param);
void pawsSkillWindow::Show()
{
    pawsControlledWindow::Show();

    // Hack set to no when show called because of an incoming skill table.
    if (skillString != "no")
    {
        skillCache.clear(); // Clear the skill cache before the new request
        psGUISkillMessage outgoing(psGUISkillMessage::REQUEST, "");
        outgoing.SendMessage();
    }

    // If this is the first time the window is open then we need to get our
    // full list of faction information.
    if (!train && !factRequest)
    {
        psFactionMessage factionRequest(0, psFactionMessage::MSG_FULL_LIST);
        factionRequest.BuildMsg();
        factionRequest.SendMessage();
    }
}

void pawsSkillWindow::Hide()
{
    psGUISkillMessage outgoing(psGUISkillMessage::QUIT, "");
    outgoing.SendMessage();
    skillCache.clear();
    pawsControlledWindow::Hide();
    train = false;
}


void pawsSkillWindow::OnListAction( pawsListBox* widget, int status )
{
    if (status==LISTBOX_HIGHLIGHTED)
    {
        pawsListBoxRow* row = widget->GetSelectedRow();
        pawsTextBox* skillName = (pawsTextBox*)(row->GetColumn(0));

        selectedSkill.Replace( skillName->GetText() );

        // Try to find cached copy of the description string */
        csStringID skillNameId = (csStringID)psengine->FindCommonStringId(selectedSkill);

        psSkillDescription *desc = NULL;
        if (skillNameId != csInvalidStringID)
        {
            desc = skillDescriptions.Get(skillNameId, NULL);
        }

        if (!desc)
        {
            // Request from the server
            csString commandData;
            commandData.Format("<S NAME=\"%s\" />", EscpXML(selectedSkill).GetData());
            psGUISkillMessage outgoing( psGUISkillMessage::SKILL_SELECTED, commandData);
            outgoing.SendMessage();
        }
        else
        {
            // Use the cached version
            switch (desc->GetCategory())
            {
                case 0://Stats
                {
                    schoSkillDescription->SetText(desc->GetDescription());
                    break;
                }
                case 1://Combat skills
                {
                    fightSkillDescription->SetText(desc->GetDescription());
                    break;
                }
                case 2://Magic skills
                {
                    utilitySkillDescription->SetText(desc->GetDescription());
                    break;
                }
                case 3://Jobs skills
                {
                    leadSkillDescription->SetText(desc->GetDescription());
                    break;
                }
                case 4://Various skills
                {
                    gatherSkillDescription->SetText(desc->GetDescription());
                    break;
                }
                case 5://Factions
                {
                    magicSkillDescription->SetText(desc->GetDescription());
                    break;
                }
                case 6://Factions
                {
                    craftSkillDescription->SetText(desc->GetDescription());
                    break;
                }
                case 7://Factions
                {
                    faithSkillDescription->SetText(desc->GetDescription());
                    break;
                }
            }
        }
    }
}

void pawsSkillWindow::Draw()
{
    psClientVitals* vitals = NULL;
    if (psengine->GetCelClient() && psengine->GetCelClient()->GetMainPlayer() )
    {
        vitals = psengine->GetCelClient()->GetMainPlayer()->GetVitalMgr();
    }

    if (vitals)
    {
        csString buff;
        buff.Format("%1.0f/%i", vitals->GetHP()*hitpointsMax, hitpointsMax);
//printf("skills 912 hpmax  %u \n",  hitpointsMax);
//printf("format statement %1.0f/%i", vitals->GetHP()*hitpointsMax, hitpointsMax);
//printf(" test get HPmod  %g \n\n",vitals->GetHP());
        hpFrac->SetText(buff);

        buff.Format("%1.0f/%i", vitals->GetBP()*bloodpointsMax, bloodpointsMax);
//        buff.Format("%d/%i", bloodpoints*bloodpointsMax, bloodpointsMax);
//printf(" BP %10.2f \n " ,vitals->GetBP()*bloodpointsMax);
//printf("skills 915 bpmax  %u \n",  bloodpointsMax);
//printf("format statement %1.0f/%i", vitals->GetBP()*bloodpointsMax, bloodpointsMax);
//printf(" test get bpmod %g \n\n",vitals->GetBP());
        bpFrac->SetText(buff);

        buff.Format("%1.0f/%i", vitals->GetMana()*manaMax, manaMax);
//printf("skills 922  manaMax %u \n", manaMax);
//printf(" test get manamod  %g \n\n",vitals->GetMana());
        manaFrac->SetText(buff);

        buff.Format("%1.0f/%i", vitals->GetSpirit()*spiritMax, spiritMax);
//printf("skills 922  spirit %u \n", spiritMax);
//printf(" test get spiritmod  %g \n\n",vitals->GetSpirit());
        spiritFrac->SetText(buff);

        buff.Format("%1.0f/%i", vitals->GetPStamina()*physStaminaMax, physStaminaMax);
//printf(" test get spiritmod  %g \n\n",vitals->GetPStamina());
        pysStaminaFrac->SetText(buff);
        buff.Format("%1.0f/%i", vitals->GetMStamina()*menStaminaMax, menStaminaMax);
//printf(" test get spiritmod  %g \n\n",vitals->GetMStamina());
        menStaminaFrac->SetText(buff);
    }

    pawsWidget::Draw();
}

void pawsSkillWindow::FlashTabButton(const char* buttonName, bool flash)
{
/*
    if (buttonName != NULL && FindWidget(buttonName))
    {
        ((pawsButton *) FindWidget(buttonName))->Flash(flash);
    }
*/
}

void pawsSkillWindow::HandleSkillCategory(pawsListBox* tabNameSkillList,
                                          const char* indWidget,
                                          const char* tabName,
                                          psSkillCacheItem *skillInfo,
                                          int &idx, bool flush)
{
  
    long long R = skillInfo->getRank();
    long long Y = skillInfo->getKnowledge();
    long long Z = skillInfo->getPractice();

     // filter out untrained skills FILTER
    if (filter  &&  R==0  &&  Y==0  &&  Z==0)
    {
        return;
    }
    if (R==0)
    {
        return;
    }

    // Get the skill name string from common strings
    csString skillName = psengine->FindCommonString(skillInfo->getNameId());

    if (skillName.IsEmpty())
    {
        Error2("Invalid skill name with Id %d", skillInfo->getNameId());
        return;
    }

    pawsListBoxRow* row = NULL;
    if (flush)
    {
        row = tabNameSkillList->NewRow();
    }
    else
    {
        row = unsortedSkills[idx];
    }

    pawsTextBox* name = dynamic_cast <pawsTextBox*> (row->GetColumn(0));
    if (name == NULL)
    {
        return;
    }

    name->SetText( skillName );

    pawsTextBox* rank = dynamic_cast <pawsTextBox*> (row->GetColumn(1));
    if (rank == NULL)
    {
        return;
    }
    if (skillInfo->getRank() == skillInfo->getActualStat())
    {
        rank->FormatText("%i", skillInfo->getRank());
    }
    else
    {
        rank->FormatText("%i (%i)", skillInfo->getRank(), skillInfo->getActualStat());
    }

    pawsWidget * indCol = row->GetColumn(2);
    if (indCol == NULL)
    {
        return;
    }

    pawsSkillIndicator * indicator = dynamic_cast <pawsSkillIndicator*> (indCol->FindWidget(indWidget));
    if (indicator == NULL)
    {
        return;
    }
    indicator->Set(x, R, Y, skillInfo->getKnowledgeCost(),
                   Z, skillInfo->getPracticeCost());
    Notify8(LOG_USER, "SKILL: skillName:%s progrPoints:%d Rank:%d Knowledge:%d knowCost:%d Practice:%d practCost:%d \n",
            skillName.GetData(), x, R, Y, skillInfo->getKnowledgeCost(), Z, skillInfo->getPracticeCost());
    if (flush)
    {
        unsortedSkills.Push(row);
    }
    else
    {
        unsortedSkills[idx] = row;
    }
    
    if (skillName == selectedSkill)
    {
        tabNameSkillList->Select(row);
        foundSelected = true;
    }

    //If we are training, flash the tab button
//    FlashTabButton(tabName, train);

    ++idx; // Update the row index
}

/************************************************************************************
*
*                           graphical skill indicator
*
**************************************************************************************/

pawsSkillIndicator::pawsSkillIndicator()
{
    x = 0;
    rank = 0;
    y = 0;
    yCost = 0;
    z = 0;
    zCost = 0;

    g2d = PawsManager::GetSingleton().GetGraphics2D();
}

unsigned int pawsSkillIndicator::GetRelCoord(unsigned int pt)
{
    if (yCost + zCost == 0) // divide-by-zero guard
    {
        return 0;
    }
    else
    {

        int returnv = int(screenFrame.Width() * pt / float(yCost+zCost));
//printf("1089 returnnv %d - real cord %d - ycost %d zcost %d \n", returnv, pt, yCost,zCost);
        if(returnv > screenFrame.Width())
        {
            returnv = screenFrame.Width();
        }

        return returnv;
    }
}

void pawsSkillIndicator::DrawSkillProgressBar(int x, int y, int width, int height,
                                int start_r, int start_g, int start_b)
{
    pawsProgressBar::DrawProgressBar(
        csRect(x, y, x+width-1, y+height-1), PawsManager::GetSingleton().GetGraphics3D(), 1,
        start_r, start_g, start_b, 100, 100, 100 );
}

void pawsSkillIndicator::Draw()
{
    csRect sf = screenFrame;

    unsigned int t1;

    if (y < yCost)
    {
        if (GetRelCoord(x) + GetRelCoord(y) < (unsigned int) sf.Width())
        {
            t1 = GetRelCoord(x) + GetRelCoord(y);
        }
        else
        {
            t1 = sf.Width();
        }

        int split = GetRelCoord(yCost);
        DrawSkillProgressBar(sf.xmin, sf.ymin, split, sf.Height(), 180, 180, 30);
        DrawSkillProgressBar(sf.xmin+split, sf.ymin, sf.Width()-split, sf.Height(), 180, 30, 30);
        DrawSkillProgressBar(sf.xmin, sf.ymin, t1, sf.Height()/2, 30, 30, 180);
        DrawSkillProgressBar(sf.xmin, sf.ymin, GetRelCoord(y), sf.Height(), 0, 80, 0);
    }
    else
    {
        int split = GetRelCoord(y+z);
        DrawSkillProgressBar(sf.xmin, sf.ymin, split, sf.Height(), 0, 80, 0);
        DrawSkillProgressBar(sf.xmin+split, sf.ymin, sf.Width()-split, sf.Height(), 180, 30, 30);
    }
    g2d->DrawLine(sf.xmin, sf.ymin, sf.xmax, sf.ymin, 0);
    g2d->DrawLine(sf.xmax, sf.ymin, sf.xmax, sf.ymax, 0);
    g2d->DrawLine(sf.xmin, sf.ymax, sf.xmax, sf.ymax, 0);
    g2d->DrawLine(sf.xmin, sf.ymin, sf.xmin, sf.ymax, 0);
}

void pawsSkillIndicator::Set(long long  x, long long rank, long long y, long long yCost, long long z, long long zCost)
{
//printf("1145  set rank %lld ycost %lld zcost %lld \n",rank,  yCost,zCost);
    this->x = x;
    this->rank = rank;
    //clamp ycost so if someone overtrained (due to training before changes to the training cost)
    //it won't show a wrong progress bar filled status.
    if(y > yCost)
        this->y = yCost;
    else
        this->y = y;
    this->yCost = yCost;
    this->z = z;
    this->zCost = zCost;
}
