
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
    void DealDamage(Unit* player, uint32 move_index, uint8 target_index = 0);
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

    Player* player;
    Prisma* prisma;
    Position player_pos;

    Player* target;
    Prisma* target_prisma;
    Position target_pos;

    Position CalculatePivotPoint(Unit* unit1, Unit* unit2);
    CombatCampPosition GenerateCombatCamp(float distance);
    Position GeneratePositionFromPivot(float rand, float distance);
    void MoveUnitToCombat(Unit* unit, Position loc, Position targetPos);

    bool SendPrismaUIInformation();
    bool SendPrismaMoveSetInformation();
    bool SendPrismaTurnInformation(PrismaTurnInformations info);
};
