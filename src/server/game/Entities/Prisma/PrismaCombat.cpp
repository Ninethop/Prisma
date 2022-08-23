
#include "PrismaCombat.h"

// WILD CONFRONTATION
PrismaCombat::PrismaCombat(Player* _player, Prisma* _prisma)
    : combat_type(PrismaCombatTypes::WILD), player(_player), prisma(nullptr), target(nullptr), target_prisma(_prisma),
    map(_player->GetMap()), weather(PrismaWeathers::CLEAR)
{
    target_prisma->SetPrismaLevel(irand(3, 8)); // always before
    target_prisma->InitializePrisma();

    // if the player doesn't have any prisma, we need to stop the combat
    if (!Prisma::HasPrisma(player))
    {
        // maybe teleport to a nearest safety point ?
        return;
    }

    prisma = Prisma::Invoke(player);
    if (!prisma)
    {
        // can't summon prisma
        return;
    }

    pivot = CalculatePivotPoint(player, target_prisma);
    camp = GenerateCombatCamp(COMBAT_RANGE_FROM_PIVOT);
    player_pos = GeneratePositionFromPivot(angle.player + RAND_OFFSET_WILD_MASTER, COMBAT_RANGE_MASTER_FROM_PIVOT);

    player->SendPrismaData("PRISMA", "ShowPrisma"); // used for display UI

    SendPrismaUIInformation();
    SendPrismaMoveSetInformation();

    MoveUnitToCombat(player, player_pos, camp.target);
    MoveUnitToCombat(target_prisma, camp.target, camp.player);
    MoveUnitToCombat(prisma, camp.player, camp.target);
}

void PrismaCombat::DealDamage(Unit* who, uint32 move_index, uint8 target_index)
{
    // this 3 are always needed
    if (!player || !prisma || !target_prisma)
        return;

    // player deal damage to target_prisma
    if (who->GetGUID() == player->GetGUID())
    {
        PrismaMoveSet* move_set = prisma->GetPrismaMoveSet();
        int32 move_id = *(move_set->GetMovesID() + move_index);
        uint32 move_pp = *(move_set->GetMovesPP() + move_index);

        // move id not exist or move doesn't have enough pp
        if (move_id <= 0 || move_pp == 0)
            return;

        if (!move_set->UseMove(move_index))
            return;

        // player turn
        PrismaUnitTurnData player_turn;
        player_turn.Initialize();
        player_turn.SetDamage(target_prisma, Prisma::CalculateDamage(prisma, target_prisma, move_id, weather, false));
        player_turn.SetSpeed(prisma->GetCalculatedStat().speed, Prisma::MoveUseSpeedPriority(move_id));
        player_turn.played = true;

        turn_data.data.push_back(player_turn);

        // if wild, randomly generate target turn
        if (combat_type == PrismaCombatTypes::WILD)
        {
            PrismaUnitTurnData target_turn;
            target_turn.Initialize();
            target_turn.speed = target_prisma->GetCalculatedStat().speed;
            //uint32 move_id = Prisma::GetRandomMove(target_prisma);
            //DealDamage(target_prisma, move_id, 0);
            target_turn.played = true;

            turn_data.data.push_back(target_turn);
        }

        SendTurn();
        return;
    }

    // target deal damage throught target_prisma to prisma
    if (target)
    {
        if (who->GetGUID() == target->GetGUID() && target_prisma)
        {

            return;
        }
    }

    // target_prisma deal damage to prisma
    if (who->GetGUID() == target_prisma->GetGUID())
    {

    }
}

void PrismaCombat::SendTurn()
{
    if (!turn_data.CanPlayTurn())
        return;

    std::sort(turn_data.data.begin(), turn_data.data.end(), PrismaTurnData::SortBySpeed);
    if (combat_type == PrismaCombatTypes::WILD)
    {
        PrismaTurnInformations _info = PrismaTurnInformations::AGAIN;

        for (PrismaUnitTurnData& turn : turn_data.data)
        {
            if (turn.damage > 0 && turn.damage_target)
            {
                turn.damage_target->ApplyDamage(turn.damage);

                // if target is dead, then stop combat
                if (turn.damage_target->IsPrismaDead())
                {
                    if (turn.damage_target == target_prisma)
                        _info = PrismaTurnInformations::WIN;

                    if (turn.damage_target == prisma)
                        _info = PrismaTurnInformations::LOOSE;

                    break;
                }
            }
        }

        SendPrismaTurnInformation(_info);
        turn_data.Reset();
        return;
    }

    turn_data.Reset();
}

void PrismaCombat::EndCombat()
{
    if (prisma)
    {
        prisma->SavePrismaToDB();
        ObjectGuid::LowType db_guid = prisma->GetSpawnId();
        Prisma::DeleteFromDB(db_guid);
        prisma->CleanupsBeforeDelete();
        delete prisma;
    }

    if (player)
    {
        player->RemoveUnitFlag(UnitFlags::UNIT_FLAG_SERVER_CONTROLLED | UnitFlags::UNIT_FLAG_NON_ATTACKABLE);
    }

    if (target)
    {
        target->RemoveUnitFlag(UnitFlags::UNIT_FLAG_SERVER_CONTROLLED | UnitFlags::UNIT_FLAG_NON_ATTACKABLE);

        // only if target_prisma is non wild
        if (target_prisma)
        {
            target_prisma->SavePrismaToDB();
            ObjectGuid::LowType db_guid = target_prisma->GetSpawnId();
            Prisma::DeleteFromDB(db_guid);
            target_prisma->CleanupsBeforeDelete();
            delete target_prisma;
        }
    }
    else // is wild
    {
        if (target_prisma)
        {
            target_prisma->KillSelf();
            Corpse* target_prisma_corpse = target_prisma->ToCorpse();
            if (target_prisma_corpse)
                target_prisma_corpse->RemoveFromGrid();
        }
    }
}

Position PrismaCombat::CalculatePivotPoint(Unit* unit1, Unit* unit2)
{
    if (unit1->GetMap() != unit2->GetMap())
        return unit1->GetPosition();

    Position pos1 = unit1->GetPosition();
    Position pos2 = unit2->GetPosition();

    float x = (pos1.GetPositionX() + pos2.GetPositionX()) / 2.f;
    float y = (pos1.GetPositionY() + pos2.GetPositionY()) / 2.f;
    float z = unit1->GetMap()->GetGridHeight(x, y);

    return Position(x, y, z);
}

CombatCampPosition PrismaCombat::GenerateCombatCamp(float distance)
{
    CombatCampPosition _camp;
    int count = 0;
    while (true)
    {
        angle.Initialize();
        float px = pivot.GetPositionX() + (distance * cos(angle.player));
        float py = pivot.GetPositionY() + (distance * sin(angle.player));
        _camp.player = { px, py, map->GetGridHeight(px,py) };

        float tx = pivot.GetPositionX() + (distance * cos(angle.target));
        float ty = pivot.GetPositionY() + (distance * sin(angle.target));
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

Position PrismaCombat::GeneratePositionFromPivot(float rand, float distance)
{
    float x = pivot.GetPositionX() + (distance * cos(rand));
    float y = pivot.GetPositionY() + (distance * sin(rand));
    float z = map->GetGridHeight(x, y);

    return Position(x, y, z);

}

void PrismaCombat::MoveUnitToCombat(Unit* unit, Position loc, Position targetPos)
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

/* Show prisma information for player
*   FUNC|FRIEND|ENEMY
*   {data} : level , hp , total_hp , name (, cur_xp, total_xp)
*/
bool PrismaCombat::SendPrismaUIInformation()
{
    if (!target_prisma || !prisma || !player)
        return false;

    std::string data = "";

    //FUNCTION
    data += "Wild|";

    //FRIEND
    data += std::to_string(prisma->GetPrismaLevel()) + ",";
    data += std::to_string(prisma->GetCurrentStamina()) + ",";
    data += std::to_string(prisma->GetCalculatedStat().stamina) + ",";
    data += prisma->GetName() + ",";
    data += std::to_string(prisma->GetPrismaExperience()) + ",";
    data += std::to_string(Prisma::GetRequiredExperienceForNextLevel(prisma->GetPrismaLevel(), PrismaExperienceTypes::MEDIUM_FAST));

    //SEPARATOR
    data += "|";

    //ENEMY
    uint32 target_hp = target_prisma->GetCalculatedStat().stamina;
    data += std::to_string(target_prisma->GetPrismaLevel()) + ",";
    data += std::to_string(target_hp) + ",";    // on wild, target have maximum hp at initialisation
    data += std::to_string(target_hp) + ",";
    data += target_prisma->GetName();

    player->SendPrismaData("PRISMA", data);
    return true;
}

/* Send move set information for player
*   FUNC|MOVE0|MOVE1|MOVE2|MOVE3
*   MOVE = id , name , total_pp, pp
*/
bool PrismaCombat::SendPrismaMoveSetInformation()
{
    if (!prisma || !player)
        return false;

    PrismaMoveSet* _moveSet = prisma->GetPrismaMoveSet();
    const PrismaMoveTemplate* _moves[4];

    std::string data = "";

    //FUNCTION
    data += "MoveSet|";

    //MOVE0 -> 4
    for (int i = 0; i < 4; ++i)
    {
        int32* id = _moveSet->GetMovesID();
        uint32* pp = _moveSet->GetMovesPP();

        if (*(id + i) > 0)
        {
            _moves[i] = sObjectMgr->GetPrismaMoveTemplate(*(id + i));
            if (_moves[i])
            {
                data += std::to_string(i) + ",";
                data += _moves[i]->Name + ",";
                data += std::to_string(_moves[i]->PowerPoints) + ",";
                data += std::to_string(*(pp + i));
            }
            else
            {
                data += "-1,,0,0";
            }
        }

        if (*(id + i) == -1)
        {
            data += "-1,,0,0";
        }

        data += "|";
    }

    player->SendPrismaData("PRISMA", data);
    return true;
}

/* Send information about the current turn to display correctly the UI
*   FUNC|SUB_CALL|DATA1|DATA2
*   AGAIN --> DATA = hp
*   WIN   --> DATA = level, xp, total_xp
*/
bool PrismaCombat::SendPrismaTurnInformation(PrismaTurnInformations info)
{
    std::string data = "";

    //FUNCTION
    data += "TurnInfo|";

    if (info == PrismaTurnInformations::AGAIN)
    {
        data += "Update|";
        data += std::to_string(prisma->GetCurrentStamina()) + "|";
        data += std::to_string(target_prisma->GetCurrentStamina());

        player->SendPrismaData("PRISMA", data);
        return true;
    }

    if (info == PrismaTurnInformations::WIN)
    {
        // need to calculate experience
        data += "Win";

        player->SendPrismaData("PRISMA", data);
        prisma->ApplyExperience(Prisma::GetGainExperience(prisma, target_prisma, false, false, 1, 1.f));
        EndCombat();
        return true;
    }

    return false;
}
