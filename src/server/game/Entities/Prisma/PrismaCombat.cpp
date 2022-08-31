
#include "PrismaCombat.h"

// WILD CONFRONTATION
PrismaCombat::PrismaCombat(Player* _player, Prisma* _prisma)
    : combat_type(PrismaCombatTypes::WILD), player(_player), prisma(nullptr), target(nullptr), target_prisma(_prisma),
    map(_player->GetMap()), weather(PrismaWeathers::CLEAR)
{
    target_prisma->SetPrismaLevel(irand(3, 8)); // always before
    target_prisma->InitializePrisma();

    // Add enemy prisma monster to fields vector ( for combat use )
    AddEnemyToFields(target_prisma);

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

    // Add friend prisma monster to fields vector ( for combat use )
    AddFriendToFields(prisma);

    // generate center of the scene
    pivot = CalculatePivotPoint(player, target_prisma);
    // generate camp position, with a random angle for player camp
    // and the invert for the enemy camp, prisma monster are moved exactly
    // where camp player & enemy are generated
    camp = GenerateCombatCamp(COMBAT_RANGE_FROM_PIVOT);
    // generate the player position, near to the camp player position ( near prisma monster )
    player_pos = GeneratePositionFromPivot(angle.player + RAND_OFFSET_WILD_MASTER, COMBAT_RANGE_MASTER_FROM_PIVOT);

    // Send UI information
    ShowPrismaUI(player);
    SendPrismaUIInformation();
    SendPrismaMoveSetInformation();

    // Move all the unit to the generated position
    MoveUnitToCombat(player, player_pos, camp.target);
    MoveUnitToCombat(target_prisma, camp.target, camp.player);
    MoveUnitToCombat(prisma, camp.player, camp.target);
}

void PrismaCombat::UseMove(Unit* who, uint32 move_index, uint8 selection_index)
{
    // this 3 are always needed
    if (!player || !prisma || !target_prisma)
        return;

    // player deal damage to target_prisma
    if (who->GetGUID() == player->GetGUID())
    {
        PrismaMoveSet move_set = prisma->GetPrismaMoveSet();
        int32 move_id = *(move_set.GetMovesID() + move_index);
        uint32 move_pp = *(move_set.GetMovesPP() + move_index);

        // move id not exist or move doesn't have enough pp
        if (move_id <= 0 || move_pp == 0)
            return;

        // if player have shoot a bad move, but can choose a good one
        // , then he need to choose again
        if (!move_set.CanUseMove(move_index) && move_set.CanUseMoves())
            return;

        // player turn
        PrismaUnitTurnData player_turn;
        player_turn.Initialize(prisma, true, move_id, selection_index, friend_fields, enemy_fields);
        prisma->InitializeTurnInformation();
        player_turn.played = true;

        turn_data.data.push_back(player_turn);

        // if wild, randomly generate target turn
        if (combat_type == PrismaCombatTypes::WILD)
        {
            PrismaUnitTurnData target_turn;
            target_turn.Initialize(target_prisma, false, target_prisma->GetPrismaMoveSet().GetRandomMoveId(), 0, enemy_fields, friend_fields);
            target_prisma->InitializeTurnInformation();
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

    // sort all prisma turn by speed
    std::sort(turn_data.data.begin(), turn_data.data.end(), PrismaTurnData::SortBySpeed);

    if (combat_type == PrismaCombatTypes::WILD)
    {
        //PrismaTurnInformations _info = PrismaTurnInformations::AGAIN;
        PrismaTurnInformation _info = { PrismaTurnInformations::AGAIN, std::vector<Prisma*>() };

        for (PrismaUnitTurnData& _turn : turn_data.data)
        {
            // this is used for Client information
            _turn.self->SetMoveSpeed(_turn.FinalSpeed());

            // this happen when a prisma is dead before he can attack, and when the combat
            // isn't stopped because there are another prisma alive in the team
            if (_turn.self->IsPrismaDead())
                continue;

            _info.targets.clear();

            const PrismaMoveTemplate* _move = sObjectMgr->GetPrismaMoveTemplate(_turn.move_id);
            if (!_move)
            {
                const PrismaMoveTemplate _secure_move_ = { 165, 0, "Struggle", PrismaTypes::NORMAL, PrismaMoveCategories::PHYSICAL,
                1, 50, 100, 0.f, 0, PrismaMoveSelectionTypes::RANDOM_TARGET };

                ApplyMoveOnRandomTarget(&_info, _turn, &_secure_move_);
                continue;
            }

            // remove 1 pp
            _turn.self->UseMove(_turn.move_id);

            // Need to store in PrismaTurnInfo, the move as failed
            uint32 rand_accuracy = urand(0, 100);
            if (rand_accuracy > _move->Accuracy)
            {
                _turn.self->SetMoveFailed();
                continue;
            }

            TC_LOG_INFO("prisma", "PrimaCombat Log >> %s use %s (%u)", _turn.self->GetName(), _move->Name, _move->Entry);

            switch (_move->SelectionType)
            {
            case PrismaMoveSelectionTypes::SELECTED_TARGET:
                ApplyMoveOnSelectedTarget(&_info, _turn, _move);
                break;
            case PrismaMoveSelectionTypes::RANDOM_TARGET:
                ApplyMoveOnRandomTarget(&_info, _turn, _move);
                break;
            case PrismaMoveSelectionTypes::ALL_TARGET:
                ApplyMoveOnAllTarget(&_info, _turn, _move);
                break;
            default:
                break;
            }

            ApplyNonVolatileStatusFlags(&_info, _turn, _move);

            if (_info.next == PrismaTurnInformations::WIN || _info.next == PrismaTurnInformations::LOOSE)
                break;
        }

        SendPrismaTurnInformation(_info.next);
        turn_data.Reset();
        return;
    }

    turn_data.Reset();
}

/*****************************
        SELECTED TARGET
*****************************/
void PrismaCombat::ApplyMoveOnSelectedTarget(PrismaTurnInformation* _info, PrismaUnitTurnData& _turn, const PrismaMoveTemplate* _move)
{
    if (_turn.enemies.size() == 0)
        return;

    if (_turn.selection >= _turn.enemies.size())
        _turn.selection = 0;

    auto _target = _turn.enemies[_turn.selection];
    _info->targets.push_back(_target);

    if (IsSpecificMove<SPECIFIC_MOVE_SELECTED_TARGET>(_move, SpecificMoveSelectedTarget))
    {
        ApplySpecificMoveOnSelectedTarget(_info, _turn, _move, _target);
        return;
    }

    uint32 damage = Prisma::CalculateDamage(_turn.self, _target, _turn.move_id, weather, false);
    _target->ApplyDamage(damage);

    if (_target->IsPrismaDead())
        _turn.is_friend ? _info->next = PrismaTurnInformations::WIN : _info->next = PrismaTurnInformations::LOOSE;
}

void PrismaCombat::ApplySpecificMoveOnSelectedTarget(PrismaTurnInformation* _info, PrismaUnitTurnData& _turn, const PrismaMoveTemplate* _move, Prisma* _target)
{

}

/****************************
        RANDOM TARGET
*****************************/
void PrismaCombat::ApplyMoveOnRandomTarget(PrismaTurnInformation* _info, PrismaUnitTurnData& _turn, const PrismaMoveTemplate* _move)
{
    if (_turn.enemies.size() == 0)
        return;

    uint32 rand = urand(0, _turn.enemies.size() - 1);
    auto _target = _turn.enemies[rand];
    _info->targets.push_back(_target);

    if (IsSpecificMove<SPECIFIC_MOVE_RANDOM_TARGET>(_move, SpecificMoveRandomTarget))
    {
        ApplySpecificMoveOnRandomTarget(_info, _turn, _move, _target);
        return;
    }

    if (_turn.enemies.size() == 0)
        return;

}

void PrismaCombat::ApplySpecificMoveOnRandomTarget(PrismaTurnInformation* _info, PrismaUnitTurnData& _turn, const PrismaMoveTemplate* _move, Prisma* _target)
{
    if (_move->Entry == 165)    // STRUGGLE
    {
        uint32 damage = Prisma::CalculateDamage(_turn.self, _target, _turn.move_id, weather, false);

        _target->ApplyDamage(damage);
        _turn.self->ApplyDamage((damage / 2));

        if (_target->IsPrismaDead() && _turn.self->IsPrismaDead())
        {
            _info->next = PrismaTurnInformations::EQUALITY;
        }
        else if (_target->IsPrismaDead() && !_turn.self->IsPrismaDead())
        {
            if (_turn.is_friend) _info->next = PrismaTurnInformations::WIN;
            else _info->next = PrismaTurnInformations::LOOSE;
        }
        else if (!_target->IsPrismaDead() && _turn.self->IsPrismaDead())
        {
            if (_turn.is_friend) _info->next = PrismaTurnInformations::LOOSE;
            else _info->next = PrismaTurnInformations::WIN;
        }
        else
        {
            _info->next = PrismaTurnInformations::AGAIN;
        }

        return;
    }
}

/****************************
        ALL TARGETS
*****************************/
void PrismaCombat::ApplyMoveOnAllTarget(PrismaTurnInformation* _info, PrismaUnitTurnData& _turn, const PrismaMoveTemplate* _move)
{
    if (_turn.enemies.size() == 0)
        return;

    if (IsSpecificMove<SPECIFIC_MOVE_ALL_TARGET>(_move, SpecificMoveAllTarget))
    {
        ApplySpecificMoveOnAllTarget(_info, _turn, _move);
        return;
    }

    for (auto target : _turn.enemies)
    {
        uint32 damage = Prisma::CalculateDamage(_turn.self, target, _turn.move_id, weather, false);
        target->ApplyDamage(damage);

        if (target->IsPrismaDead())
        {
            if (_turn.is_friend)
            {
                if (target->IsLast())
                {
                    _info->next = PrismaTurnInformations::WIN;
                }
            }
            else
            {
                if (_turn.self->IsLast())
                {
                    _info->next = PrismaTurnInformations::LOOSE;
                }
            }
        }
    }
}

void PrismaCombat::ApplySpecificMoveOnAllTarget(PrismaTurnInformation* _info, PrismaUnitTurnData& _turn, const PrismaMoveTemplate* _move)
{

}

/****************************
     NON VOLATILE STATUS
*****************************/
void PrismaCombat::ApplyNonVolatileStatusFlags(PrismaTurnInformation* _info, PrismaUnitTurnData& _turn, const PrismaMoveTemplate* _move)
{
    if (_move->NonVolatileStatusFlags == 0 || _info->targets.size() == 0)
        return;

    // only take prisma who don't have any `non volatile status`
    std::vector<Prisma*> status_targets;
    for (auto _t : _info->targets)
        if (_t->GetNonVolatileStatusFlags() == 0)
            status_targets.push_back(_t);

    if (status_targets.size() == 0)
        return;

    if (IsSpecificMove<SPECIFIC_MOVE_NON_VOLATILE_STATUS>(_move, SpecificMoveNonVolatileStatus))
    {
        ApplySpecificNonVolatileStatusFlags(_info, _turn, _move, status_targets);
        return;
    }

    std::vector<PrismaNonVolatileStatus> status;
    for (double i = double(PrismaNonVolatileStatus::BURN); i < double(PrismaNonVolatileStatus::SLEEP); ++i)
        if ((_move->NonVolatileStatusFlags & uint32(std::pow(2.0, i))) != 0)
            status.push_back(PrismaNonVolatileStatus(int(i)));

    if (status.size() == 0)
        return;

    float rand = frand(0.f, 100.f);
    if (rand < _move->ProbabilityNVSF)
    {
        PrismaNonVolatileStatus _status;

        if (status.size() == 1)
        {
            _status = status[0];
        }
        else
        {
            uint32 random_status = urand(0, status.size() - 1);
            _status = status[random_status];
        }

        for (auto target : status_targets)
            target->AddNonVolatileStatus(_status);
    }
}

void PrismaCombat::ApplySpecificNonVolatileStatusFlags(PrismaTurnInformation* _info, PrismaUnitTurnData& _turn, const PrismaMoveTemplate* _move, std::vector<Prisma*> _status_targets)
{
    if (_move->Entry == 161) // TRI ATTACK
    {
        for (int i = int(PrismaNonVolatileStatus::BURN); i < int(PrismaNonVolatileStatus::PARALYSIS); ++i)
        {
            float rand = frand(0.f, 100.f);
            if (rand < _move->ProbabilityNVSF)
            {
                for (auto target : _status_targets)
                    target->AddNonVolatileStatus(PrismaNonVolatileStatus(i));
            }
        }

        return;
    }

    if (_move->Entry == 374) // FLING ...
    {

        return;
    }

    if (_move->Entry == 375) // PSYCHO SHIFT
    {
        if (_turn.self->GetNonVolatileStatusFlags() > 0)
        {
            for (auto target : _status_targets)
            {
                uint32 flags = target->GetNonVolatileStatusFlags();
                flags |= _turn.self->GetNonVolatileStatusFlags();
                target->SetNonVolatileStatusFlags(flags);
            }
        }

        return;
    }
}

void PrismaCombat::EndCombat()
{
    if (prisma)
    {
        if (prisma->IsPrismaDead())
        {
            // need to teleport player to health center
        }

        prisma->SavePrismaToDB();
        prisma->DespawnOrUnsummon(Milliseconds(0));
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
            target_prisma->DespawnOrUnsummon(Milliseconds(0));
        }
    }
    else // is wild
    {
        if (target_prisma)
        {
            if (target_prisma->IsPrismaDead())
            {
                target_prisma->KillSelf();
                //target_prisma->CleanupBeforeRemoveFromMap(true);
            }
            else
            {
                target_prisma->SetReactState(ReactStates::REACT_AGGRESSIVE);
                target_prisma->ResumeMovement();
                //delete target_prisma; // ???
            }
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

    angle.Initialize();
    for (double delta = 0; delta < M_PI; delta += 0.2)
    {
        angle.player += float(delta);
        angle.target += float(delta);

        float px = pivot.GetPositionX() + (distance * cos(angle.player));
        float py = pivot.GetPositionY() + (distance * sin(angle.player));
        _camp.player = { px, py, map->GetGridHeight(px,py) };

        float tx = pivot.GetPositionX() + (distance * cos(angle.target));
        float ty = pivot.GetPositionY() + (distance * sin(angle.target));
        _camp.target = { tx, ty, map->GetGridHeight(tx,ty) };

        if (map->isInLineOfSight(
            _camp.player.GetPositionX(), _camp.player.GetPositionY(), _camp.player.GetPositionZ(),
            _camp.target.GetPositionX(), _camp.target.GetPositionY(), _camp.target.GetPositionZ(),
            0, LineOfSightChecks::LINEOFSIGHT_ALL_CHECKS, VMAP::ModelIgnoreFlags::Nothing))
        {
            break;
        }
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

/* Show prisma UI for player */
bool PrismaCombat::ShowPrismaUI(Player* _p)
{
    if (!_p)
        return false;

    PrismaMessageData message;
    message << "ShowPrisma";

    _p->SendPrismaData(message);
    return true;
}

/* Hide prisma UI for player */
bool PrismaCombat::HidePrismaUI(Player* _p)
{
    if (!_p)
        return false;

    PrismaMessageData message;
    message << "HidePrisma";

    _p->SendPrismaData(message);
    return true;
}

/* Show prisma information for player
*   FUNC|FRIEND|ENEMY
*   {data} : level , hp , total_hp , name (, cur_xp, total_xp)
*/
bool PrismaCombat::SendPrismaUIInformation()
{
    if (!target_prisma || !prisma || !player)
        return false;

    PrismaMessageData message;
    message << "Wild" << ";";

    //FRIEND
    message << prisma->GetPrismaLevel() << ",";
    message << prisma->GetCurrentStamina() << ",";
    message << prisma->GetCalculatedStat().stamina << ",";
    message << prisma->GetName() << ",";
    message << prisma->GetPrismaExperience() << ",";
    message << Prisma::GetRequiredExperienceForNextLevel(prisma->GetPrismaLevel(), PrismaExperienceTypes::MEDIUM_FAST);

    //SEPARATOR
    message << ";";

    //ENEMY
    uint32 target_hp = target_prisma->GetCalculatedStat().stamina;
    message << target_prisma->GetPrismaLevel() << ",";
    message << target_hp << ",";
    message << target_hp << ",";
    message << target_prisma->GetName();

    player->SendPrismaData(message);
    return true;
}

/* Send move set information for player
*   FUNC|MOVE0|MOVE1|MOVE2|MOVE3
*   MOVE = index , name , total_pp , pp, type
*/
bool PrismaCombat::SendPrismaMoveSetInformation()
{
    if (!prisma || !player)
        return false;

    PrismaMoveSet _moveSet = prisma->GetPrismaMoveSet();
    const PrismaMoveTemplate* _moves[4];

    PrismaMessageData message;
    message << "MoveSet" << ";";

    //MOVE0 -> 4
    for (int i = 0; i < 4; ++i)
    {
        int32* id = _moveSet.GetMovesID();
        uint32* pp = _moveSet.GetMovesPP();

        if (*(id + i) > 0)
        {
            _moves[i] = sObjectMgr->GetPrismaMoveTemplate(*(id + i));
            if (_moves[i])
            {
                message << i << ",";
                message << _moves[i]->Name << ",";
                message << _moves[i]->PowerPoints << ",";
                message << *(pp + i) << ",";
                message << int32(_moves[i]->Type);
            }
            else
            {
                message << "-1,,0,0,-1";
            }
        }
        else
        {
            message << "-1,,0,0,-1";
        }

        message << ";";
    }

    player->SendPrismaData(message);
    return true;
}

/* Send information about the current turn to display correctly the UI
*   FUNC|SUB_CALL|DATA1|DATA2
*   AGAIN --> DATA = hp, flag0, flag1, flag2, speed, failed, critical
*   WIN   --> DATA = level, xp, total_xp
*/
bool PrismaCombat::SendPrismaTurnInformation(PrismaTurnInformations info)
{
    PrismaMessageData message;
    message << "TurnInfo" << ";";

    if (info == PrismaTurnInformations::AGAIN)
    {
        message << "Update" << ";";

        message << prisma->GetCurrentStamina() << ",";
        message << prisma->GetNonVolatileStatusFlags() << ",";
        message << prisma->GetVolatileStatusFlags() << ",";
        message << prisma->GetVolatileCombatStatusFlags() << ",";
        message << prisma->MoveSpeed() << ",";
        message << (prisma->MoveFailed() ? "1" : "0") << ",";
        message << (prisma->MoveCritical() ? "1" : "0");

        message << ";";

        message << target_prisma->GetCurrentStamina() << ",";
        message << target_prisma->GetNonVolatileStatusFlags() << ",";
        message << target_prisma->GetVolatileStatusFlags() << ",";
        message << target_prisma->GetVolatileCombatStatusFlags() << ",";
        message << target_prisma->MoveSpeed() << ",";
        message << (target_prisma->MoveFailed() ? "1" : "0") << ",";
        message << (target_prisma->MoveCritical() ? "1" : "0");

        player->SendPrismaData(message);
        return true;
    }

    if (info == PrismaTurnInformations::WIN)
    {
        message << "Win";
        player->SendPrismaData(message);

        prisma->ApplyExperience(Prisma::GetGainExperience(prisma, target_prisma, false, false, 1, 1.f));
        EndCombat();
        return true;
    }

    if (info == PrismaTurnInformations::LOOSE)
    {
        message << "Loose";
        player->SendPrismaData(message);

        EndCombat();
        return true;
    }

    return false;
}
