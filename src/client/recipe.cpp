/*
 * recipe.cpp
 *
 *  Created on: 15 Jul 2015
 *      Author: Diana Coman
 */

#include <psconfig.h>

#include "recipe.h"
#include "util/strutil.h"
#include "util/log.h"


recipe::recipe()
{
	itemName = csString("");
	bundleName = csString("");
	recipeName = csString("");
	containerName = csString("");
	toolName = csString("");

	ingredientsList = new csHash<int, csString>();
}

recipe::~recipe()
{
	delete ingredientsList;
}

bool recipe::HasTool()
{
	return !toolName.CompareNoCase("");
}

int recipe::GetQuantity(csString ingredient)
{
	return ingredientsList->Get(ingredient, 0);
}

void recipe::ParseIngredients(csString ingredients)
{
	csString separator(", ");
	size_t pos, nextPos, length;
	csString ingredientName;
	int ingredientCount;
	csString currText;

	pos = 0;

	while (pos < ingredients.Length())
	{
		while (ingredients.GetAt(pos) == ' ') pos++;   //skip leading spaces if any

		nextPos = NextPosInString(ingredients, separator, pos);
		length = nextPos - pos;
		currText = ingredients.Slice(pos, length);

		ingredientCount = atoi(currText.GetData());
		if (ingredientCount == 0)
		{
			//no ingredient count specified, so everything is ingredient name
			ingredientCount = 1;
			ingredientName = currText;
		}
		else
		{
			pos = currText.Find(" ");
			length = currText.Length()-pos-2;
			ingredientName = currText.Slice(pos+1, length);
		}

		pos = nextPos + separator.Length();

		ingredientsList->Put(ingredientName, ingredientCount);

	}

}

int recipe::NextPosInString(csString str, csString separator, int startPos)
{
	if (startPos >= str.Length())
		return str.Length();

	int nextPos = str.Find(separator, startPos);
	if (nextPos > str.Length())
		nextPos = str.Length();

	return nextPos;
}

csString recipe::GetTextBetweenWords(csString str, csString wordBefore, csString wordAfter)
{
	size_t pos1 = str.Find(wordBefore);
	if (pos1 > str.Length()) return csString("");

	size_t pos2 = str.Find(wordAfter, pos1);
	if (pos1 > str.Length()) return csString("");

	size_t start = pos1 + wordBefore.Length();
	size_t length = pos2 - start;

	return str.Slice(start, length);
}

void recipe::ParseRecipe(csString recipeText)
{
	ingredientsList->Empty();

	csString ingredients = GetTextBetweenWords(recipeText, csString("Combine "), csString(", into "));
	ParseIngredients(ingredients);

	bundleName = GetTextBetweenWords(recipeText, csString(", into "), csString("."));

	csString beforeWord = bundleName + csString(" into");
	itemName = GetTextBetweenWords(recipeText, beforeWord, csString(" using"));

	csString lastPart = GetTextBetweenWords(recipeText, csString("using"), csString("."));

	size_t pos = lastPart.Find("with");
	if (pos>lastPart.Length())
	{
		toolName = "";
		pos = lastPart.Find(".");
		containerName = lastPart.Slice(0, pos);
	}
	else
	{
		containerName = lastPart.Slice(0, pos);
		toolName = GetTextBetweenWords(lastPart, csString("with a "), csString("."));
	}

}