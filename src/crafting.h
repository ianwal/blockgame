#pragma once

#include "src/item.h"

#include <vector>

namespace bg
{

struct CraftingRecipe {
    std::vector<ItemStack> inputs{};
    ItemStack output{};
};

bool canCraft(CraftingRecipe const& recipe, std::vector<ItemStack> const& inputItems);

class CraftingRecipeRegistry
{
public:
    std::vector<CraftingRecipe> const& getAllRecipes() const;

    /// Load the crafting recipe.
    void load(CraftingRecipe recipe);

private:
    std::vector<CraftingRecipe> allRecipes{};
};

} // namespace bg
