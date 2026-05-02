/*
 * This file is part of the SylCore Project. See AUTHORS file for Copyright information
 * https://sylcore.org
 * https://www.youtube.com/@DEVSylian
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Affero General Public License as published by the
 * Free Software Foundation; either version 3 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include "DynamicAreaTriggerMgr.h"
#include "Player.h"
#include "Map.h"
#include "GridDefines.h"            // SIZE_OF_GRIDS (usually 32.0f)
#include <cmath>
#include <set>

DynamicAreaTriggerMgr* DynamicAreaTriggerMgr::instance()
{
    static DynamicAreaTriggerMgr mgr;
    return &mgr;
}

// Convert world coordinates to the same cell coordinates used by the map grid.
// SIZE_OF_GRIDS is the edge length of a cell in yards (default: 32.0).
inline DynamicAreaTriggerMgr::CellCoord DynamicAreaTriggerMgr::PositionToCell(float x, float y)
{
    return { int32(std::floor(x / SIZE_OF_GRIDS)), int32(std::floor(y / SIZE_OF_GRIDS)) };
}

void DynamicAreaTriggerMgr::AddTrigger(std::shared_ptr<DynamicAreaTrigger> trigger)
{
    uint32 mapId = trigger->GetMapId();
    auto& data = _mapData[mapId];   // creates MapData if it doesn't exist
    data.allTriggers.push_back(trigger);
    RegisterToCells(data, trigger);
}

void DynamicAreaTriggerMgr::RemoveTrigger(DynamicAreaTrigger* triggerPtr)
{
    uint32 mapId = triggerPtr->GetMapId();
    auto it = _mapData.find(mapId);
    if (it == _mapData.end())
        return;

    MapData& data = it->second;
    auto& vec = data.allTriggers;
    for (auto iter = vec.begin(); iter != vec.end(); ++iter)
    {
        if (iter->get() == triggerPtr)
        {
            UnregisterFromCells(data, *iter);
            vec.erase(iter);
            break;
        }
    }

    // Clean up empty map entry to save memory
    if (vec.empty())
        _mapData.erase(it);
}

void DynamicAreaTriggerMgr::RegisterToCells(MapData& data, std::shared_ptr<DynamicAreaTrigger> trigger)
{
    Position const& pos = trigger->GetPos();
    float radius = trigger->GetRadius();

    // Determine the range of cells the circle touches
    CellCoord minCell = PositionToCell(pos.GetPositionX() - radius, pos.GetPositionY() - radius);
    CellCoord maxCell = PositionToCell(pos.GetPositionX() + radius, pos.GetPositionY() + radius);

    for (int32 x = minCell.x; x <= maxCell.x; ++x)
        for (int32 y = minCell.y; y <= maxCell.y; ++y)
            data.cellIndex[{x, y}].push_back(trigger);
}

void DynamicAreaTriggerMgr::UnregisterFromCells(MapData& data, std::shared_ptr<DynamicAreaTrigger> trigger)
{
    // Removal is infrequent, so we can just scan all cells.
    // For absolute peak performance you could store a reverse mapping, but not needed.
    for (auto& [cell, list] : data.cellIndex)
    {
        auto newEnd = std::remove(list.begin(), list.end(), trigger);
        list.erase(newEnd, list.end());
    }

    // Erase empty cell entries to keep the map small
    for (auto it = data.cellIndex.begin(); it != data.cellIndex.end(); )
    {
        if (it->second.empty())
            it = data.cellIndex.erase(it);
        else
            ++it;
    }
}

/**
 * This is the workhorse. It must be called from Player::UpdatePosition()
 * after the player's grid/cell has been updated.
 *
 * ALGORITHM:
 *  1. Compute the player's current cell.
 *  2. Gather all triggers registered in that cell and the 8 surrounding cells.
 *  3. For each unique trigger, compute 2D distance to the player.
 *  4. Compare with the trigger's stored "inside" set and fire OnPlayerEnter
 *     or OnPlayerLeave when the state changes.
 */
void DynamicAreaTriggerMgr::UpdatePlayerPosition(Player* player)
{
    if (!player->IsInWorld())
        return;

    Map* map = player->GetMap();
    if (!map)
        return;

    uint32 mapId = map->GetId();
    auto it = _mapData.find(mapId);
    if (it == _mapData.end())
        return;  // No triggers on this map

    MapData& data = it->second;

    float px = player->GetPositionX();
    float py = player->GetPositionY();
    CellCoord playerCell = PositionToCell(px, py);

    // Collect all triggers from current cell and the 8 neighbouring cells.
    // Using a set of raw pointers prevents processing the same trigger twice
    // if it spans multiple cells (common for large radii).
    std::set<DynamicAreaTrigger*> processed;
    for (int32 dx = -1; dx <= 1; ++dx)
    {
        for (int32 dy = -1; dy <= 1; ++dy)
        {
            CellCoord neighbour = { playerCell.x + dx, playerCell.y + dy };
            auto cellIt = data.cellIndex.find(neighbour);
            if (cellIt != data.cellIndex.end())
            {
                for (auto& trigger : cellIt->second)
                    processed.insert(trigger.get());
            }
        }
    }

    // For each unique trigger, check inside/outside and fire events
    for (DynamicAreaTrigger* trigger : processed)
    {
        bool inside = trigger->IsPlayerInside(player);
        bool wasInside = trigger->GetInsidePlayers().contains(player->GetGUID());
        // Note: GetInsidePlayers() returns a const set, so contains() is clean.

        if (inside && !wasInside)
            trigger->OnPlayerEnter(player);
        else if (!inside && wasInside)
            trigger->OnPlayerLeave(player);
    }
}

void DynamicAreaTriggerMgr::RemovePlayerFromAll(Player* player)
{
    uint32 mapId = player->GetMapId();
    auto it = _mapData.find(mapId);
    if (it == _mapData.end())
        return;

    for (auto& trigger : it->second.allTriggers)
        trigger->CleanPlayer(player);
}
