#include "src/item.h"

#include "src/app_context.h"
#include "src/assertr.h"

#include <format>
#include <string>
#include <string_view>

namespace bg
{

std::string to_string(ItemType itemType)
{
    return App::get().itemRegistry.getName(itemType);
}

ItemType ItemRegistry::getType(std::string const& name)
{
    for (auto const& [itemType, itemName] : names) {
        if (name == itemName) {
            return itemType;
        }
    }
    assertr(false, std::format("No item type found for name: {}", name));
    return {};
}

std::string ItemRegistry::getName(ItemType type)
{
    for (auto const& [itemType, itemName] : names) {
        if (type == itemType) {
            return itemName;
        }
    }

    assertr(false, std::format("No name found for item type: {}", std::to_underlying(type)));
    return {};
}

std::string ItemRegistry::getPrettyName(ItemType type)
{
    for (auto const& [itemType, itemPrettyName] : prettyNames) {
        if (type == itemType) {
            return itemPrettyName;
        }
    }

    assertr(false, std::format("No pretty name found for item type: {}", std::to_underlying(type)));
    return {};
}

void ItemRegistry::load(std::uint16_t id, std::string const& name, std::string const& prettyName)
{
    auto const type = static_cast<ItemType>(id);
    assertr(!names.contains(type), name);
    assertr(!prettyNames.contains(type), name);

    names[type]       = name;
    prettyNames[type] = prettyName;
}

} // namespace bg
