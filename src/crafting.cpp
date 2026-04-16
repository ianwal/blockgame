#include "src/crafting.h"

#include <vector>

namespace bg
{

std::vector<CraftingRecipe> const& CraftingRecipeRegistry::getAllRecipes() const
{
    return allRecipes;
}

void CraftingRecipeRegistry::load(CraftingRecipe recipe)
{
    allRecipes.push_back(recipe);
}

} // namespace bg
