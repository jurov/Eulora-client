/*
 * recipe.h
 *
 *  Created on: 15 Jul 2015
 *      Author: Diana Coman
 */

#ifndef RECIPE_H_
#define RECIPE_H_

#include "globals.h"


class recipe {
public:

	recipe();
	virtual ~recipe();

	bool HasTool();
	void ParseRecipe(csString recipeText);

	int GetQuantity(csString ingredient);

	const csString& GetBundleName() const {
		return bundleName;
	}

	const csString& GetContainerName() const {
		return containerName;
	}

	csHash<int, csString>* GetIngredientsList() const {
		return ingredientsList;
	}

	const csString& GetItemName() const {
		return itemName;
	}

	const csString& GetRecipeName() const {
		return recipeName;
	}

	const csString& GetToolName() const {
		return toolName;
	}

	const csString& getRecipeName() const {
		return recipeName;
	}

	void setRecipeName(const csString& recipeName) {
		this->recipeName = recipeName;
	}

private:

	csHash<int, csString>* ingredientsList;
	csString itemName;
	csString bundleName;
	csString recipeName;
	csString containerName;
	csString toolName;

	void ParseIngredients(csString ingredients);
	void ParseItemName(csString recipeText);
	void ParseContainerAndTool(csString recipeText);
	csString GetTextBetweenWords(csString str, csString wordBefore, csString wordAfter);
	int NextPosInString(csString str, csString separator, int startPos);
};

#endif /* RECIPE_H_ */