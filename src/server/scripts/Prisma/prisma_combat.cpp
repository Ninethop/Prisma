
#include "ScriptMgr.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "Creature.h"
#include "Log.h"
#include "DatabaseEnv.h"
#include "Prisma.h"

class PrismaCombat : public PlayerScript
{
public:
	PrismaCombat()
        : PlayerScript("PrismaCombat")
    { }

    void OnPrismaKill(Player* player, Prisma* prisma) override
    {
        int xp_gain = Prisma::GetGainExperience(player->GetLevel(), prisma->GetLevel(), 64, true, false, 1, 1.f);
        int xp_to_levelup = Prisma::GetRequiredExperienceForNextLevel(player->GetLevel(), PrismaExperienceTypes::MEDIUM_FAST);

        TC_LOG_INFO("prisma", "PRI: You have gained %u experience", xp_gain);
        TC_LOG_INFO("prisma", "PRI: You need %u experience to level up", xp_to_levelup);
    }

    void OnEnterCombat(Player* player, Unit* target) override
    {
        // initialize wild combat only if target is a simple prisma
        if (target->IsPrisma())
        {
            Prisma* prisma = target->ToPrisma();
            prisma->SetLevel(irand(5,20));
            prisma->InitializePrisma();

            TC_LOG_INFO("prisma", "%s, against %s (%u)!", player->GetName(), target->GetName(), target->GetLevel());

            InitializeWildCombat(player, prisma);

            TC_LOG_INFO("prisma", "%s stat = stamina:%u - attack:%u - defense:%u - special_attack:%u - special_defense:%u - speed:%u", prisma->GetName(),
                prisma->CalculateStat(PrismaStats::STAMINA), prisma->CalculateStat(PrismaStats::ATTACK), prisma->CalculateStat(PrismaStats::DEFENSE),
                prisma->CalculateStat(PrismaStats::SPECIAL_ATTACK), prisma->CalculateStat(PrismaStats::SPECIAL_DEFENSE), prisma->CalculateStat(PrismaStats::SPEED));
            TC_LOG_INFO("prisma", "%s Individual Values = stamina:%u - attack:%u - defense:%u - special_attack:%u - special_defense:%u - speed:%u", prisma->GetName(),
                prisma->GetStatIV(PrismaStats::STAMINA), prisma->GetStatIV(PrismaStats::ATTACK), prisma->GetStatIV(PrismaStats::DEFENSE),
                prisma->GetStatIV(PrismaStats::SPECIAL_ATTACK), prisma->GetStatIV(PrismaStats::SPECIAL_DEFENSE), prisma->GetStatIV(PrismaStats::SPEED));
            TC_LOG_INFO("prisma", "%s Nature = %s", prisma->GetName(), prisma->GetNatureName());
            TC_LOG_INFO("prisma", "%s Characteristic = %s", prisma->GetName(), prisma->GetCharacteristic());
            TC_LOG_INFO("prisma", "%s Gender = %s", prisma->GetName(), prisma->GetGenderName());
        }

    }

    void OnLeaveCombat(Player* player) override
    {
        RemovePrisma(player->GetGUID());
        ClearPlayerFlagCombat(player);

        TC_LOG_INFO("prisma", "%s leave the combat!", player->GetName());
    }

private:
    /*
    * All player prisma are stored here, maybe need to change this to Summon instead of Creature
    * Maybe need to move this to src/game/Entites/Prisma/PrismaData.h
    */
    //std::unordered_map<uint32, Creature*> _prismaStore;
    std::unordered_map<ObjectGuid, Prisma*> _prismaStorage;

    /*
    * simple confrontation with 2 trainer, without prisma
    */
    void InitializeConfrontation(Player* player, Unit* target)
    {
        Map* map = player->GetMap();
        Position pivot = CalculatePivotPoint(player, target);
        Position new_player_pos;
        Position new_target_pos;
        CombatRandAngle angle;
        CombatCampPosition camp = GenerateCombatCamp(pivot, map, angle, CONFRONTATION_RANGE);

        MoveUnitToCombat(player, camp.player, camp.target);
        MoveUnitToCombat(target, camp.target, camp.player);
    }

    /*
    * confrontation from 2 trainer, with 1 prisma for each
    */
    void InitializeCombat(Unit* player, Unit* target, Unit* prisma_player, Unit* prisma_target)
    {

    }

    /*
    * confrontation from player and his prisma and an wild prisma
    */
    void InitializeWildCombat(Player* player, Prisma* target)
    {
        uint32 prisma_id = Prisma::GetTeamID(player);
        if (prisma_id == 0) // player doesn't have any prisma
            return;

        Map* map = player->GetMap();
        ObjectGuid player_guid = player->GetGUID();
        Position pivot = CalculatePivotPoint(player, target);
        CombatRandAngle angle;
        CombatCampPosition camp = GenerateCombatCamp(pivot, map, angle, COMBAT_RANGE_FROM_PIVOT);
        Position player_pos = GeneratePositionFromPivot(pivot, map, angle.player + (RAND_OFFSET_WILD_MASTER / 4.f), COMBAT_RANGE_MASTER_FROM_PIVOT);

        _prismaStorage[player_guid] = Prisma::Invoke(player, prisma_id);
        if (!_prismaStorage[player_guid]) // error prisma doesn't exist
        {
            TC_LOG_INFO("prisma", "Prisma with id:%u can't be spawn", prisma_id);
            return;
        }

        MoveUnitToCombat(player, player_pos, camp.target);
        MoveUnitToCombat(target, camp.target, camp.player);
        MoveUnitToCombat(_prismaStorage[player_guid], camp.player, camp.target);
    }

    Position CalculatePivotPoint(Unit* entity1, Unit* entity2)
    {
        if (entity1->GetMap() != entity2->GetMap())
            return entity1->GetPosition();

        Position entity1_pos = entity1->GetPosition();
        Position entity2_pos = entity2->GetPosition();

        float x = (entity1_pos.GetPositionX() + entity2_pos.GetPositionX()) / 2.f;
        float y = (entity1_pos.GetPositionY() + entity2_pos.GetPositionY()) / 2.f;
        float z = entity1->GetMap()->GetGridHeight(x, y);

        return Position(x, y, z);
    }

    CombatCampPosition GenerateCombatCamp(Position pivot, Map* map, CombatRandAngle& rand, float distance)
    {
        CombatCampPosition _camp;
        int count = 0;
        while (true)
        {
            rand.Initialize();
            float px = pivot.GetPositionX() + (distance * cos(rand.player));
            float py = pivot.GetPositionY() + (distance * sin(rand.player));
            _camp.player = { px, py, map->GetGridHeight(px,py) };

            float tx = pivot.GetPositionX() + (distance * cos(rand.target));
            float ty = pivot.GetPositionY() + (distance * sin(rand.target));
            _camp.target = { tx, ty, map->GetGridHeight(tx,ty) };

            if (map->isInLineOfSight(
                _camp.player.GetPositionX(), _camp.player.GetPositionY(), _camp.player.GetPositionZ(),
                _camp.target.GetPositionX(), _camp.target.GetPositionY(), _camp.target.GetPositionZ(),
                0, LineOfSightChecks::LINEOFSIGHT_ALL_CHECKS, VMAP::ModelIgnoreFlags::Nothing) ||
                count > MAX_ITERATION_ON_ENTER_COMBAT) // avoid freeze
            {
                break;
            }

            count++;
        }

        return _camp;
    }

    Position GeneratePositionFromPivot(Position pivot, Map* map, float rand, float distance)
    {
        float x = pivot.GetPositionX() + (distance * cos(rand));
        float y = pivot.GetPositionY() + (distance * sin(rand));
        float z = map->GetGridHeight(x, y);

        return Position(x, y, z);
    }

    void MoveUnitToCombat(Unit* unit, Position loc, Position targetPos)
    {
        unit->SetUnitFlag(UnitFlags::UNIT_FLAG_NON_ATTACKABLE);
        if (Creature* creature = unit->ToCreature())
        {
            creature->SetReactState(ReactStates::REACT_PASSIVE);
            creature->PauseMovement();
        }
        else if (Player* player = unit->ToPlayer())
        {
            player->SetUnitFlag(UnitFlags::UNIT_FLAG_SERVER_CONTROLLED);
        }
        unit->GetMotionMaster()->MovePoint(0, loc, true, loc.GetRelativeAngle(targetPos));
    }

    void RemovePrisma(ObjectGuid guid)
    {
        if (Prisma* prisma = _prismaStorage[guid])
        {
            ObjectGuid::LowType db_guid = prisma->GetSpawnId();
            Prisma::DeleteFromDB(db_guid);
            prisma->CleanupsBeforeDelete();
            _prismaStorage.erase(guid);
        }
        else
        {
            TC_LOG_INFO("prisma", "no in storage");
        }
    }

    void ClearPlayerFlagCombat(Player* player)
    {
        player->RemoveUnitFlag(UnitFlags::UNIT_FLAG_SERVER_CONTROLLED | UnitFlags::UNIT_FLAG_NON_ATTACKABLE);
    }
};

void AddSC_prisma_combat()
{
	new PrismaCombat();
}
