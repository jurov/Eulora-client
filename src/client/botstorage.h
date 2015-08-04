/*
 * botStorage.h
 *
 *  Created on: 23 Jul 2015
 *      Author: Diana Coman
 */

#ifndef BOTSTORAGE_H_
#define BOTSTORAGE_H_

#include "globals.h"

#include <csutil/hash.h>
#include <csutil/list.h>

class item
{
public:
	item();
	item(csString itemName, csString itemID, int count);
	~item();

	const csString& getCategory() const {
		return category;
	}

	void setCategory(const csString& catItem) {
		category = catItem;
	}

	const csString& getId() const {
		return id;
	}

	void setId(const csString& idItem) {
		id = idItem;
	}

	const csString& getName() const {
		return name;
	}

	void setName(const csString& nameItem) {
		name = nameItem;
	}

	int getQuantity() const {
		return quantity;
	}

	void setQuantity(int quantity) {
		this->quantity = quantity;
	}

protected:
	csString name;
	csString id;
	csString category;
	int quantity;
};

class botStorage
{
public:
	botStorage();
	~botStorage();

	//categories
	csList<csString>* GetCategoriesList()
	{
		return categoriesList;
	}
	void SetMerchantID(int id)
	{
		merchantID = id;
	}
	int GetMerchantID()
	{
		return merchantID;
	}
	int GetTradeCmd()
	{
		return tradeCmd;
	}
	void SetTradeCmd(int tradeCommandID)
	{
		tradeCmd = tradeCommandID;
	}

	//items
	csString GetItemID(csString itemName);	//returns the id of the first stack in storage matching the given name
	int Retrieve(csString itemName, int quantityNeeded);	//returns the number of stacks it had to retrieve to make the amount requested
	bool RetrieveAll(csString itemName);

	bool AddItem(csString itemName, csString itemID, int quantity);
	void EmptyStorage();

protected:
	csList<csString>* categoriesList;
	csHash<item, csString>* itemsList;
	int merchantID;
	int tradeCmd;
};



#endif /* BOTINVENTORY_H_ */
