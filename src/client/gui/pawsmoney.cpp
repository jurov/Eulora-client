/*
 * pawsmoney.cpp - Author: Ondrej Hurt
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


// CS INCLUDES
#include <psconfig.h>

// COMMON INCLUDES
#include "util/localization.h"
#include "util/log.h"
#include "util/psxmlparser.h"
#include "rpgrules/psmoney.h"

// PAWS INCLUDES
#include "pawsmoney.h"
#include "paws/pawsmanager.h"
#include "inventorywindow.h"


#define MONEY_FILE_NAME  "money.xml"
#define SLOT_SIZE       48

csRef<iDocumentNode> FindFirstWidget(iDocument * doc);


/**************************************************************************
*
*                        class pawsMoney
*
***************************************************************************/

pawsMoney::pawsMoney()
{
    bitcents   =   NULL;
    denarius     =   NULL;
    argents     =   NULL;
    coppers     =   NULL;
    amount    =   0;
}

bool pawsMoney::Setup(iDocumentNode * node)
{
    csString borderStr;
    
    border = node->GetAttributeValueAsBool("border");
    
    spacing = node->GetAttributeValueAsInt("spacing");
    return CreateGUI();
}

bool pawsMoney::CreateGUI()
{
    csRef<iDocument> doc;
    csRef<iDocumentNode> widgetNode;

    doc = ParseFile(PawsManager::GetSingleton().GetObjectRegistry(), PawsManager::GetSingleton().GetLocalization()->FindLocalizedFile(MONEY_FILE_NAME));
    if (doc == NULL)
    {
        Error2("File %s not found", MONEY_FILE_NAME);
        return false;
    }
    widgetNode = FindFirstWidget(doc);
    if (widgetNode == NULL)
    {
        Error2("Failed to load children from %s", MONEY_FILE_NAME);
        return false;
    }
    
    // This is a weird case because we need to remember the left,top from the original widget spec
    // but the width and height from this one
    csRect rect=this->GetDefaultFrame(); // save them here
    if (!LoadAttributes(widgetNode))
        return false;
    this->SetRelativeFramePos(rect.xmin,rect.ymin); // restore them here

    if ( ! LoadChildren(widgetNode) )
    {
        Error2("Failed to load children from %s", MONEY_FILE_NAME);
        return false;
    }
    
    // SetRelativeFrameSize(GetActualWidth(100), GetActualHeight(100));
    return true;
}

void pawsMoney::SetContainer(int container)
{
    if (!coppers) return;

    coppers->SetContainer( container );
    argents->SetContainer( container );
    denarius->SetContainer( container );
    bitcents->SetContainer( container );
    
    bitcents->SetSlotID( MONEY_BITCENTS );
    denarius->SetSlotID( MONEY_DENARIUS ); 
    argents->SetSlotID( MONEY_ARGENTS ); 
    coppers->SetSlotID( MONEY_COPPERS ); 
}


bool pawsMoney::PostSetup()
{
    bitcents  =  dynamic_cast <pawsSlot*> (FindWidget("BITCents"));
    if (bitcents == NULL)
        return false;

    bitcents->SetEmptyOnZeroCount(false);
    bitcents->PlaceItem("MoneyCircles", "");
    bitcents->SetSlotID( MONEY_BITCENTS );
    //if (border) 
    //    circles->SetBackground("Bulk Item Slot");
            
    denarius = dynamic_cast <pawsSlot*> (FindWidget("Denarius"));
    if (denarius == NULL)
        return false;
    denarius->SetEmptyOnZeroCount(false);                
    denarius->PlaceItem("MoneyOctas", "");
    denarius->SetSlotID( MONEY_DENARIUS );    
    //octas->SetRelativeFramePos(GetActualWidth(SLOT_SIZE+spacing), 0);
    //if (border) 
    //    octas->SetBackground("Bulk Item Slot");
    
    argents =  dynamic_cast <pawsSlot*> (FindWidget("Argents"));
    if (argents == NULL)
        return false;
    argents->SetEmptyOnZeroCount(false);        
    argents->PlaceItem("MoneyHexas", "");
    argents->SetSlotID( MONEY_ARGENTS );   
    //hexas->SetRelativeFramePos(0, GetActualHeight(SLOT_SIZE+spacing));
    //if (border) hexas->SetBackground("Bulk Item Slot");
    
    coppers    =  dynamic_cast <pawsSlot*> (FindWidget("Coppers"));
    if (coppers == NULL)
        return false;
    coppers->SetEmptyOnZeroCount(false);        
    coppers->PlaceItem("MoneyTrias", "");
    coppers->SetSlotID( MONEY_COPPERS );     
    //trias->SetRelativeFramePos(GetActualWidth(SLOT_SIZE+spacing), GetActualHeight(SLOT_SIZE+spacing));
    //if (border) trias->SetBackground("Bulk Item Slot");
    
    return true;
}

void pawsMoney::Draw()
{
    pawsWidget::Draw();
}

void pawsMoney::Drag( bool dragOn )
{
    coppers->SetDrag( dragOn );
    argents->SetDrag( dragOn );
    denarius->SetDrag( dragOn );
    bitcents->SetDrag( dragOn );
}

void pawsMoney::Set(int bitcents, int denarius, int argents, int coppers)
{
    if (!this->coppers) return;

    this->  bitcents  ->StackCount(bitcents);
    this->  denarius    ->StackCount(denarius);
    this->  argents    ->StackCount(argents);
    this->  coppers    ->StackCount(coppers);

    RecalculateAmount();
}

void pawsMoney::Set(const psMoney & money)
{
    Set( money.GetBITCents(), money.GetDenarius(), money.GetArgents(), 
        money.GetCoppers() );
}

void pawsMoney::Set(int coinType, int count)
{
    switch (coinType)
    {
        case MONEY_BITCENTS:  bitcents  -> StackCount(count); break;
        case MONEY_DENARIUS:    denarius    -> StackCount(count); break;
        case MONEY_ARGENTS:    argents    -> StackCount(count); break;
        case MONEY_COPPERS:    coppers    -> StackCount(count); break;
        default: CS_ASSERT(0);
    }

    this->RecalculateAmount();
}

void pawsMoney::Get(int & bitcents, int & denarius, int & argents, int & coppers)
{
    bitcents  =  this->  bitcents  ->StackCount();
    denarius    =  this->  denarius    ->StackCount();
    argents    =  this->  argents    ->StackCount();
    coppers    =  this->  coppers    ->StackCount();
}

int pawsMoney::Get(int coinType)
{
    switch (coinType)
    {
        case MONEY_BITCENTS:  return  bitcents  -> StackCount(); break;
        case MONEY_DENARIUS:    return  denarius    -> StackCount(); break;
        case MONEY_ARGENTS:    return  argents    -> StackCount(); break;
        case MONEY_COPPERS:    return  coppers    -> StackCount(); break;
        default: CS_ASSERT(0);
    }
    return 0;
}

bool pawsMoney::IsNoAmount()
{
    if (amount < 1)
        return true;
    
    return false;
}

void pawsMoney::RecalculateAmount()
{
    amount  = this->coppers->StackCount();
    amount += this->argents->StackCount()   * 100;
    amount += this->denarius->StackCount()   * 10000;
    amount += this->bitcents->StackCount() * 1000000;
}

psMoney pawsMoney::Get()
{
    psMoney money(Get(MONEY_BITCENTS), Get(MONEY_DENARIUS), Get(MONEY_ARGENTS), Get(MONEY_COPPERS));
    return money;
}


void pawsMoney::OnUpdateData(const char* /*dataname*/, PAWSData& value)
{
    psMoney money( value.GetStr() );
    Set( money );            
}
