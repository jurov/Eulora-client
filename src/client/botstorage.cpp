/*
 * botStorage.cpp
 *
 *  Created on: 23 Jul 2015
 *      Author: Diana Coman
 */

#include "botstorage.h"
#include <psconfig.h>

#include "util/psxmlparser.h"
#include "net/messages.h"
#include "util/log.h"

item::item()
{
	name = csString("");
	id = csString("");
	category = csString("");
	quantity = 0;
}

item::item(csString itemName, csString itemID, int count)
{
	name = itemName;
	id = itemID;
	quantity = count;
}

item::~item()
{
}

/**
 * Class botStorage
 */

botStorage::botStorage()
{
	itemsList = new csHash<item, csString>();
	categoriesList = new csList<csString>();
}

botStorage::~botStorage()
{
	delete itemsList;
	delete categoriesList;
}

void botStorage::EmptyStorage()
{
	itemsList->Empty();
	categoriesList->DeleteAll();
}

bool botStorage::AddItem(csString itemName, csString itemID, int quantity)
{
	item i(itemName, itemID, quantity);
	itemsList->Put(itemName, i);
}

csString botStorage::GetItemID(csString itemName)
{
	item j;
	item i = itemsList->Get(itemName, j);

	return i.getId();
}

int botStorage::Retrieve(csString itemName, int quantityNeeded)
{
	char commandData[200];
	csString escpxml = EscpXML(itemName);

	csArray<item> itemStacks = itemsList->GetAll(itemName);
	if (itemStacks.IsEmpty())
		return 0;

	csArray<item>::Iterator iter = itemStacks.GetIterator();
	int stacksCount = 0;
	while (quantityNeeded>0 && iter.HasNext())
	{
		item i = iter.Next();
		csString itemID = i.getId();

		if (i.getQuantity() > 0)
		{
			int amountOut;
			if (i.getQuantity() < quantityNeeded)
				amountOut = i.getQuantity();
			else amountOut = quantityNeeded;

			quantityNeeded = quantityNeeded - amountOut;
			i.setQuantity(i.getQuantity()-amountOut);

			sprintf(commandData, "<T ID=\"%d\" ITEM=\"%s\" COUNT=\"%d\" ITEM_ID=\"%s\" />",
		            merchantID, escpxml.GetData(), amountOut, itemID.GetData());

			Notify2(LOG_USER, "CRAFT BOT: taking out from storage: %s\n", commandData);
			psGUIStorageMessage outgoing(tradeCmd, commandData);
			outgoing.SendMessage();

			stacksCount = stacksCount + 1;
		}
	}

	return stacksCount;
}

bool botStorage::RetrieveAll(csString itemName)
{
	return false;
}


