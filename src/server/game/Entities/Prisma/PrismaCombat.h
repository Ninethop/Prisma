
#pragma once

#include "Prisma.h"
#include "Corpse.h"
#include "ScriptMgr.h"
#include "MotionMaster.h"

class Prisma;
class TC_GAME_API PrismaCombat
{
public:
    // WILD CONFRONTATION
    PrismaCombat(Player* player, Prisma* target);
    void UseMove(Unit* player, uint32 move_index, uint8 selection_index = 0);
    void SendTurn();
    void EndCombat();

private:
    PrismaCombatTypes combat_type;
    PrismaTurnData turn_data;
    Map* map;
    Position pivot;
    CombatRandAngle angle;
    CombatCampPosition camp;
    PrismaWeathers weather;

    std::vector<Prisma*> enemy_fields;
    std::vector<Prisma*> friend_fields;

    void AddFriendToFields(Prisma* _prisma) { friend_fields.push_back(_prisma); };
    void AddEnemyToFields(Prisma* _prisma) { enemy_fields.push_back(_prisma); };

    Player* player;
    Prisma* prisma;
    Position player_pos;

    Player* target;
    Prisma* target_prisma;
    Position target_pos;

    void ApplyMoveOnSelectedTarget(PrismaTurnInformation* _info, PrismaUnitTurnData& turn, const PrismaMoveTemplate* _move);
    void ApplySpecificMoveOnSelectedTarget(PrismaTurnInformation* _info, PrismaUnitTurnData& _turn, const PrismaMoveTemplate* _move, Prisma* _target);
    void ApplyMoveOnRandomTarget(PrismaTurnInformation* _info, PrismaUnitTurnData& turn, const PrismaMoveTemplate* _move);
    void ApplySpecificMoveOnRandomTarget(PrismaTurnInformation* _info, PrismaUnitTurnData& turn, const PrismaMoveTemplate* _move, Prisma* _target);
    void ApplyMoveOnAllTarget(PrismaTurnInformation* _info, PrismaUnitTurnData& turn, const PrismaMoveTemplate* _move);
    void ApplySpecificMoveOnAllTarget(PrismaTurnInformation* _info, PrismaUnitTurnData& turn, const PrismaMoveTemplate* _move);

    #define SPECIFIC_MOVE_SELECTED_TARGET           1
    const uint32 SpecificMoveSelectedTarget[SPECIFIC_MOVE_SELECTED_TARGET] = { 0 };

    #define SPECIFIC_MOVE_RANDOM_TARGET             1
    const uint32 SpecificMoveRandomTarget[SPECIFIC_MOVE_RANDOM_TARGET] = { 165 };

    #define SPECIFIC_MOVE_ALL_TARGET                1
    const uint32 SpecificMoveAllTarget[SPECIFIC_MOVE_ALL_TARGET] = { 0 };

    void ApplyNonVolatileStatusFlags(PrismaTurnInformation* _info, PrismaUnitTurnData& turn, const PrismaMoveTemplate* _move);
    void ApplySpecificNonVolatileStatusFlags(PrismaTurnInformation* _info, PrismaUnitTurnData& turn, const PrismaMoveTemplate* _move, std::vector<Prisma*> _status_targets);

    #define SPECIFIC_MOVE_NON_VOLATILE_STATUS       3
    const uint32 SpecificMoveNonVolatileStatus[SPECIFIC_MOVE_NON_VOLATILE_STATUS] = { 161, 374, 375 };

    template <int N>
    bool IsSpecificMove(const PrismaMoveTemplate* _move, const uint32 _array[N])
    {
        for (int i = 0; i < N; ++i)
            if (_array[i] == _move->Entry)
                return true;

        return false;
    }

    Position CalculatePivotPoint(Unit* unit1, Unit* unit2);
    CombatCampPosition GenerateCombatCamp(float distance);
    Position GeneratePositionFromPivot(float rand, float distance);
    void MoveUnitToCombat(Unit* unit, Position loc, Position targetPos);

    bool ShowPrismaUI(Player* _p);
    bool HidePrismaUI(Player* _p);
    bool SendPrismaUIInformation();
    bool SendPrismaMoveSetInformation();
    bool SendPrismaTurnInformation(PrismaTurnInformations info);
};
