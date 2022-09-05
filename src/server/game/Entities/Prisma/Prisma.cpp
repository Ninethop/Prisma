
#include "Prisma.h"

Prisma::Prisma(bool isWorldObject)
    : Creature(isWorldObject), owner(nullptr), _iv_generated(false), _nature_generated(false), _gender_generated(false),
    _is_loaded_from_guid(false), m_guid(0), m_id(0), m_experience(0), m_item(-1), m_level(1), m_current_stamina(0),
    m_status_volatile_flags(0), m_status_non_volatile_flags(0), m_status_volatile_combat_flags(0),
    m_turn_volatile(-1), m_turn_non_volatile(-1), m_turn_volatile_combat(-1)
{ }

void Prisma::InitializePrisma()
{
    m_id = GetEntry() - PRISMA_TEMPLATE_RESERVED_MIN;

    // this also generate characteristic
    if (!IndividualValueIsGenerated())
        GenerateIndividualValue();

    // initialize EV value to 0 for all
    {
        _ev.Init();
    }

    if (!NatureIsGenerated())
        GenerateNature();

    if (!GenderIsGenerated())
        GenerateGender();

    {
        _move.Init();
    }

    GenerateCalculatedStat();
    GenerateMoveSet();
    m_current_stamina = _calculatedStat.stamina;
}

bool Prisma::InitializePrismaFromGuid(uint32 guid)
{
    const PrismaData* data = sObjectMgr->GetPrismaData(guid);
    if (data)
    {
        m_guid = guid;
        m_id = data->ID;
        _gender.gender = uint8(data->Gender);
        _nature.nature = uint8(data->Nature);
        m_level = data->Level;
        m_experience = data->Experience;
        m_item = data->Item;
        m_current_stamina = data->CurrentStamina;
        m_status_non_volatile_flags = data->StatusFlags;
        // individual value
        _iv.stamina = data->IVStamina;
        _iv.attack = data->IVAttack;
        _iv.defense = data->IVDefense;
        _iv.special_attack = data->IVSpecialAttack;
        _iv.special_defense = data->IVSpecialDefense;
        _iv.speed = data->IVSpeed;
        GenerateCharacteristic();
        // effort value
        _ev.stamina = data->EVStamina;
        _ev.attack = data->EVAttack;
        _ev.defense = data->EVDefense;
        _ev.special_attack = data->EVSpecialAttack;
        _ev.special_defense = data->EVSpecialDefense;
        _ev.speed = data->EVSpeed;
        // move
        _move.move0 = data->Move0;
        _move.pp_move0 = data->PP_Move0;
        _move.move1 = data->Move1;
        _move.pp_move1 = data->PP_Move1;
        _move.move2 = data->Move2;
        _move.pp_move2 = data->PP_Move2;
        _move.move3 = data->Move3;
        _move.pp_move3 = data->PP_Move3;

        _iv_generated = true;
        _nature_generated = true;
        _gender_generated = true;
        GenerateCalculatedStat();

        _is_loaded_from_guid = true;
        return true;
    }

    return false;
}

uint16 Prisma::CalculateStat(PrismaStats _stat)
{
    const PrismaTemplate* _template = sObjectMgr->GetPrismaTemplate(GetPrismaEntry());
    if (!_template)
    {
        if (_stat == PrismaStats::STAMINA) return MIN_STAMINA_VALUE;
        else return MIN_STAT_VALUE;
    }

    if (_stat == PrismaStats::STAMINA)
    {
        uint16 numerator = ((2 * _template->Stamina) + _iv.stamina + (_ev.stamina / 4)) * m_level;
        uint16 result = (numerator / 100) + (m_level + 10);

        (result < MIN_STAMINA_VALUE) ? result = MIN_STAMINA_VALUE : NULL;
        return result;
    }

    uint16 base_value, iv_value, ev_value;
    float nature_factor = 1.f + _nature.GetFactor(_stat);

    if (_stat == PrismaStats::ATTACK)
    {
        base_value = _template->Attack;
        iv_value = _iv.attack;
        ev_value = _ev.attack;
    }
    else if (_stat == PrismaStats::DEFENSE)
    {
        base_value = _template->Defense;
        iv_value = _iv.defense;
        ev_value = _ev.defense;
    }
    else if (_stat == PrismaStats::SPECIAL_ATTACK)
    {
        base_value = _template->SpecialAttack;
        iv_value = _iv.special_attack;
        ev_value = _ev.special_attack;
    }
    else if (_stat == PrismaStats::SPECIAL_DEFENSE)
    {
        base_value = _template->SpecialDefense;
        iv_value = _iv.special_defense;
        ev_value = _ev.special_defense;
    }
    else // speed
    {
        base_value = _template->Speed;
        iv_value = _iv.speed;
        ev_value = _ev.speed;
    }

    uint16 numerator = (2 * base_value + iv_value + (ev_value / 4)) * m_level;
    uint16 result = ((numerator / 100) + 5) * nature_factor;

    (result < MIN_STAT_VALUE) ? result = MIN_STAT_VALUE : NULL;
    return result;
}

uint32 Prisma::CalculateDamage(Prisma* attacker, Prisma* target, uint32 move_id, PrismaWeathers weather, bool is_second_strike)
{
    if (!attacker || !target || move_id == 0)
        return 0;
    const PrismaMoveTemplate* move_template = sObjectMgr->GetPrismaMoveTemplate(move_id);
    if (!move_template)
        return 0;
    // START BASE

    float damage = (2.f * float(target->GetPrismaLevel()) / 5.f) + 2.f;
    damage *= float(move_template->BasePower);
    uint16 AttackStat;
    uint16 DefenseStat;
    if (move_template->Category == PrismaMoveCategories::PHYSICAL)
    {
        AttackStat = attacker->GetCalculatedStat().attack;
        DefenseStat = target->GetCalculatedStat().defense;
    }
    else
    {
        AttackStat = attacker->GetCalculatedStat().special_attack;
        DefenseStat = target->GetCalculatedStat().special_defense;
    }

    damage *= float(float(AttackStat) / float(DefenseStat));
    damage /= 50.f;
    damage += 2.f;
    // END BASE

    // SINGLE OR MULTI TARGET
    damage *= (move_template->SelectionType == PrismaMoveSelectionTypes::SELECTED_TARGET) ? 1.f : 0.75f;

    // SECOND STRIKE
    damage *= (is_second_strike) ? 0.25f : 1.f;

    // WEATHER: increase
    if ((weather == PrismaWeathers::RAIN && move_template->Type == PrismaTypes::WATER) ||
        (weather == PrismaWeathers::HARSH_SUNLIGHT && move_template->Type == PrismaTypes::FIRE))
    {
        damage *= 1.5f;
    }

    // WEATHER: decrease
    if ((weather == PrismaWeathers::RAIN && move_template->Type == PrismaTypes::FIRE) ||
        (weather == PrismaWeathers::HARSH_SUNLIGHT && move_template->Type == PrismaTypes::WATER))
    {
        damage *= 0.5f;
    }

    // TODO: add check is target can avoid the crit damage
    if (frand(0.f, 100.f) < move_template->CritRate)
    {
        damage *= 1.5f;
        attacker->SetMoveCritical();
    }

    // RANDOM DECREASE
    damage *= frand(0.85f, 1.f);

    // STAB
    const PrismaTemplate* attacker_template = sObjectMgr->GetPrismaTemplate(attacker->GetPrismaEntry());
    if (attacker_template)
    {
        if ((move_template->Type == PrismaTypes(attacker_template->Type1) ||
            move_template->Type == PrismaTypes(attacker_template->Type2)) &&
            move_template->Type != PrismaTypes::TYPELESS)
        {
            damage *= 1.5f;
        }
    }

    // TYPE
    const PrismaTemplate* target_template = sObjectMgr->GetPrismaTemplate(target->GetPrismaEntry());
    if (target_template)
    {
        if (target_template->Type1 >= int32(PrismaTypes::TYPELESS) &&
            target_template->Type1 < NUM_MAX_PRISMA_TYPE &&
            target_template->Type2 >= int32(PrismaTypes::TYPELESS) &&
            target_template->Type2 < NUM_MAX_PRISMA_TYPE)
        {
            damage *= GetMoveCoefficient(move_template->Type,
                PrismaTypes(target_template->Type1), PrismaTypes(target_template->Type2));
        }
    }

    // BURN: reduce from an half if attacker is burn and move is physical
    if (attacker->HasNonVolatileStatus(PrismaNonVolatileStatus::BURN) &&
        move_template->Category == PrismaMoveCategories::PHYSICAL)
    {
        damage *= 0.5f;
    }

    /* Here can be add extra feature to move specificity */
    {

    }

    return uint32(damage);
}

void Prisma::ApplyDamage(uint32 damage)
{
    if (damage >= m_current_stamina)
    {
        m_current_stamina = 0;
        return;
    }

    m_current_stamina -= damage;
}

void Prisma::ApplyExperience(uint32 exp)
{
    m_experience += exp;

    while (m_experience >= Prisma::GetRequiredExperienceForNextLevel(m_level, PrismaExperienceTypes::MEDIUM_FAST))
    {
        m_experience -= Prisma::GetRequiredExperienceForNextLevel(m_level, PrismaExperienceTypes::MEDIUM_FAST);
        LevelUp();
    }

    // PUSH NEW EXPERIENCE
    {
        PrismaMessageData message;
        message << "UpdateExperience" << ";";
        message << m_experience << ",";
        message << Prisma::GetRequiredExperienceForNextLevel(m_level, PrismaExperienceTypes::MEDIUM_FAST);
        owner->SendPrismaData(message);
    }
}

void Prisma::LevelUp()
{
    m_level++;

    // PUSH NEW LEVEL
    {
        PrismaMessageData message;
        message << "LevelUp" << ";";
        message << m_level;
        owner->SendPrismaData(message);
    }
}

bool Prisma::IsLast()
{
    if (!owner)
        return true;

    // need to check is this prisma is the last one ( all the other are dead )
    return true;
}

void Prisma::RestoreStamina()
{
    m_current_stamina = GetCalculatedStat().stamina;
}

void Prisma::RestoreMove()
{
    for (int m = 0; m < 4; m++)
    {
        int move_id = *(_move.GetMovesID() + m);
        PrismaMoveTemplate const* move_template = sObjectMgr->GetPrismaMoveTemplate(move_id);

        if (move_template)
            _move.SetMove(m, move_id, move_template->PowerPoints);
    }
}

void Prisma::RestoreStatus()
{
    m_status_non_volatile_flags = 0;
    m_turn_non_volatile = -1;

    m_status_volatile_flags = 0;
    m_turn_volatile = -1;

    m_status_volatile_combat_flags = 0;
    m_turn_volatile_combat = -1;
}

void Prisma::InitializeTurnInformation()
{
    m_turn_speed = 0;
    m_turn_failed = false;
    m_turn_critical = false;
}

int8 Prisma::MoveSpeedPriority(int32 move_id)
{
    if (move_id <= 0)
        return int8(0);

    const PrismaMoveTemplate* move_template = sObjectMgr->GetPrismaMoveTemplate(move_id);
    if (!move_template)
        return int8(0);

    return move_template->SpeedPriority;
}

/*
    SAVE PRISMA INTO `prisma` TABLE
*/
void Prisma::SavePrismaToDB()
{
    if (!m_guid)
        m_guid = Prisma::GenerateGUID();

    PrismaData& data = sObjectMgr->NewOrExistPrismaData(m_guid);

    data.GUID = m_guid;
    data.ID = GetPrismaEntry();
    data.Gender = _gender.GetGender();
    data.Nature = _nature.GetNature();
    data.Level = m_level;
    data.Experience = m_experience;
    data.Item = m_item;
    data.CurrentStamina = m_current_stamina;
    data.StatusFlags = m_status_non_volatile_flags;
    data.IVStamina = _iv.stamina;
    data.IVAttack = _iv.attack;
    data.IVDefense = _iv.defense;
    data.IVSpecialAttack = _iv.special_attack;
    data.IVSpecialDefense = _iv.special_defense;
    data.IVSpeed = _ev.speed;
    data.EVStamina = _ev.stamina;
    data.EVAttack = _ev.attack;
    data.EVDefense = _ev.defense;
    data.EVSpecialAttack = _ev.special_attack;
    data.EVSpecialDefense = _ev.special_defense;
    data.EVSpeed = _ev.speed;
    data.Move0 = _move.move0;
    data.PP_Move0 = _move.pp_move0;
    data.Move1 = _move.move1;
    data.PP_Move1 = _move.pp_move1;
    data.Move2 = _move.move2;
    data.PP_Move2 = _move.pp_move2;
    data.Move3 = _move.move3;
    data.PP_Move3 = _move.pp_move3;

    PrismaDatabaseTransation trans = PrismaDatabase.BeginTransaction();
    PrismaDatabasePreparedStatement* stmt = PrismaDatabase.GetPreparedStatement(PRISMA_DEL_PRISMA);
    stmt->setUInt32(0, m_guid);
    trans->Append(stmt);

    uint8 index = 0;
    stmt = PrismaDatabase.GetPreparedStatement(PRISMA_INS_PRISMA);
    stmt->setUInt32(index++, data.GUID);
    stmt->setUInt32(index++, data.ID);
    stmt->setUInt32(index++, uint32(data.Gender));
    stmt->setUInt32(index++, uint32(data.Nature));
    stmt->setUInt32(index++, data.Level);
    stmt->setUInt32(index++, data.Experience);
    stmt->setInt32(index++, data.Item);
    stmt->setUInt32(index++, data.CurrentStamina);
    stmt->setUInt32(index++, data.StatusFlags);
    stmt->setUInt32(index++, data.IVStamina);
    stmt->setUInt32(index++, data.IVAttack);
    stmt->setUInt32(index++, data.IVDefense);
    stmt->setUInt32(index++, data.IVSpecialAttack);
    stmt->setUInt32(index++, data.IVSpecialDefense);
    stmt->setUInt32(index++, data.IVSpeed);
    stmt->setUInt32(index++, data.EVStamina);
    stmt->setUInt32(index++, data.EVAttack);
    stmt->setUInt32(index++, data.EVDefense);
    stmt->setUInt32(index++, data.EVSpecialAttack);
    stmt->setUInt32(index++, data.EVSpecialDefense);
    stmt->setUInt32(index++, data.EVSpeed);
    stmt->setInt32(index++, data.Move0);
    stmt->setUInt32(index++, data.PP_Move0);
    stmt->setInt32(index++, data.Move1);
    stmt->setUInt32(index++, data.PP_Move1);
    stmt->setInt32(index++, data.Move2);
    stmt->setUInt32(index++, data.PP_Move2);
    stmt->setInt32(index++, data.Move3);
    stmt->setUInt32(index++, data.PP_Move3);
    trans->Append(stmt);

    PrismaDatabase.CommitTransaction(trans);
}

void Prisma::SetPrismaLevel(uint32 level, bool update)
{
    m_level = level;
    if (update)
        SetLevel(m_level);
}

void Prisma::AddNonVolatileStatus(PrismaNonVolatileStatus _status)
{
    m_status_non_volatile_flags |= uint32(std::pow(2.0, double(_status)));
}

void Prisma::RemoveNonVolatileStatus(PrismaNonVolatileStatus _status)
{
    m_status_non_volatile_flags &= ~(uint32(std::pow(2.0, double(_status))));
}

bool Prisma::HasNonVolatileStatus(PrismaNonVolatileStatus _status)
{
    return (m_status_non_volatile_flags & uint32(std::pow(2.0, double(_status)))) != 0;
}

void Prisma::AddVolatileStatus(PrismaVolatileStatus _status)
{
    m_status_volatile_flags |= uint32(std::pow(2.0, double(_status)));
}

void Prisma::RemoveVolatileStatus(PrismaVolatileStatus _status)
{
    m_status_volatile_flags &= ~(uint32(std::pow(2.0, double(_status))));
}

bool Prisma::HasVolatileStatus(PrismaVolatileStatus _status)
{
    return (m_status_volatile_flags & uint32(std::pow(2.0, double(_status)))) != 0;
}

void Prisma::AddVolatileCombatStatus(PrismaVolatileCombatStatus _status)
{
    m_status_volatile_combat_flags |= uint32(std::pow(2.0, double(_status)));
}

void Prisma::RemoveVolatileCombatStatus(PrismaVolatileCombatStatus _status)
{
    m_status_volatile_combat_flags &= ~(uint32(std::pow(2.0, double(_status))));
}

bool Prisma::HasVolatileCombatStatus(PrismaVolatileCombatStatus _status)
{
    return (m_status_volatile_combat_flags & uint32(std::pow(2.0, double(_status)))) != 0;
}

bool Prisma::HasPrisma(Player* player)
{
    // first select all the player team
    PrismaDatabasePreparedStatement* stmt = PrismaDatabase.GetPreparedStatement(PRISMA_SEL_PLAYER_PRISMA);
    stmt->setUInt32(0, player->GetGUID());
    PreparedQueryResult result = PrismaDatabase.Query(stmt);

    if (result)
    {
        for (int i = 0; i < 6; ++i)
        {
            if ((*result)[i].GetUInt32() > 0)
                return true;
        }
    }

    return false;
}

uint32 Prisma::GenerateGUID()
{
    return ObjectMgr::GeneratePrismaGuid();
}

Prisma* Prisma::Invoke(Player* owner, uint8 num)
{
    uint32 id = Prisma::GetTeamID(owner, num);
    uint32 guid = Prisma::GetTeamGUID(owner, num);
    Map* map = owner->GetMap();
    Prisma* prisma = new Prisma();
    if (!prisma->Create(map->GenerateLowGuid<HighGuid::Unit>(), map, owner->GetPhaseMaskForSpawn(), PRISMA_TEMPLATE_RESERVED_MIN + id, *owner))
    {
        delete prisma;
        return nullptr;
    }

    prisma->SaveToDB(map->GetId(), (1 << map->GetSpawnMode()), owner->GetPhaseMaskForSpawn());
    ObjectGuid::LowType db_guid = prisma->GetSpawnId();
    prisma->CleanupsBeforeDelete();
    delete prisma;

    prisma = new Prisma();
    if (!prisma->InitializePrismaFromGuid(guid))
    {
        // guid doesn't exist
        delete prisma;
        return nullptr;
    }

    if (!prisma->LoadFromDB(db_guid, map, true, true))
    {
        // can't found prisma in `creature` db
        delete prisma;
        return nullptr;
    }
    sObjectMgr->AddCreatureToGrid(db_guid, sObjectMgr->GetCreatureData(db_guid));
    prisma->SetOwner(owner);
    return prisma;
}

uint32 Prisma::GetTeamID(Player* player, uint8 num)
{
    if (num > 5) num = 0;

    // first select all the player team
    PrismaDatabasePreparedStatement* stmt = PrismaDatabase.GetPreparedStatement(PRISMA_SEL_PLAYER_PRISMA);
    stmt->setUInt32(0, player->GetGUID());
    PreparedQueryResult result = PrismaDatabase.Query(stmt);

    if (result)
    {
        // if guid == 0, then player doesn't have the desired prisma
        uint32 prisma_guid = (*result)[num].GetUInt32();
        if (prisma_guid == 0)
            return 0;

        // second select the id from the guid in `prisma_template` table
        PrismaDatabasePreparedStatement* stmt_id = PrismaDatabase.GetPreparedStatement(PRISMA_SEL_PRISMA_ID_FROM_GUID);
        stmt_id->setUInt32(0, prisma_guid);
        PreparedQueryResult result_id = PrismaDatabase.Query(stmt_id);

        if (result_id)
            return (*result_id)[0].GetUInt32();

    }

    return 0;
}

uint32 Prisma::GetTeamGUID(Player* player, uint8 num)
{
    if (num > 5) num = 0;

    // first select all the player team
    PrismaDatabasePreparedStatement* stmt = PrismaDatabase.GetPreparedStatement(PRISMA_SEL_PLAYER_PRISMA);
    stmt->setUInt32(0, player->GetGUID());
    PreparedQueryResult result = PrismaDatabase.Query(stmt);

    if (result)
        return (*result)[num].GetUInt32();

    return 0;
}

void Prisma::GenerateIndividualValue()
{
    // STAMINA
    while (true)
    {
        _iv.stamina = irand(IV_MIN_VALUE, IV_MAX_VALUE);
        if (_iv.stamina >= 0 && _iv.stamina <= 31)
            break;
    }

    // ATTACK
    while (true)
    {
        _iv.attack = irand(IV_MIN_VALUE, IV_MAX_VALUE);
        if (_iv.attack >= 0 && _iv.attack <= 31)
            break;
    }

    // DEFENSE
    while (true)
    {
        _iv.defense = irand(IV_MIN_VALUE, IV_MAX_VALUE);
        if (_iv.defense >= 0 && _iv.defense <= 31)
            break;
    }

    // SPECIAL ATTACK
    while (true)
    {
        _iv.special_attack = irand(IV_MIN_VALUE, IV_MAX_VALUE);
        if (_iv.special_attack >= 0 && _iv.special_attack <= 31)
            break;
    }

    // SPECIAL DEFENSE
    while (true)
    {
        _iv.special_defense = irand(IV_MIN_VALUE, IV_MAX_VALUE);
        if (_iv.special_defense >= 0 && _iv.special_defense <= 31)
            break;
    }

    // SPEED
    while (true)
    {
        _iv.speed = irand(IV_MIN_VALUE, IV_MAX_VALUE);
        if (_iv.speed >= 0 && _iv.speed <= 31)
            break;
    }

    GenerateCharacteristic();
    _iv_generated = true;
}

void Prisma::GenerateCharacteristic()
{
    std::vector<PrismaStats> max = _iv.GetMax();
    if (max.size() == 0) // error
        return;

    int index = 0;
    if (max.size() > 1)
        index = irand(0, max.size() - 1);

    PrismaStats _stat = max[index];
    _characteristic.data = ((uint8)(_stat) * (NUM_MAX_PRISMA_STAT - 1)) + (_iv.GetStat(_stat) % 5);
}

void Prisma::GenerateNature()
{
    _nature.Set(PrismaNatures(irand(0, NUM_MAX_PRISMA_NATURE - 1)));
    _nature_generated = true;
}

void Prisma::GenerateGender()
{
    auto _template = sObjectMgr->GetPrismaTemplate(GetPrismaEntry());

    if (!_template || _template->GenderFactor < 0)
    {
        _gender.Set(PrismaGenders::NO_GENDER);
        _gender_generated = true;
        return;
    }

    // if frand >= GenderFactor, then female otherwise male
    if (frand(0.f, 1.f) >= _template->GenderFactor) _gender.Set(PrismaGenders::FEMALE);
    else _gender.Set(PrismaGenders::MALE);

    _gender_generated = true;
}

std::vector<std::string> Prisma::SplitData(const std::string& data, const std::string& delimiter, bool add)
{
    std::string _data = data;
    if (add) _data += delimiter;
    std::vector<std::string> splitter;

    size_t pos = 0;
    while ((pos = _data.find(delimiter)) != std::string::npos)
    {
        splitter.push_back(_data.substr(0, pos));
        _data.erase(0, pos + delimiter.length());
    }

    return splitter;
}

void Prisma::GenerateCalculatedStat()
{
    _calculatedStat.stamina            = CalculateStat(PrismaStats::STAMINA);
    _calculatedStat.attack             = CalculateStat(PrismaStats::ATTACK);
    _calculatedStat.defense            = CalculateStat(PrismaStats::DEFENSE);
    _calculatedStat.special_attack     = CalculateStat(PrismaStats::SPECIAL_ATTACK);
    _calculatedStat.special_defense    = CalculateStat(PrismaStats::SPECIAL_DEFENSE);
    _calculatedStat.speed              = CalculateStat(PrismaStats::SPEED);

    _currentStat = _calculatedStat;
}

void Prisma::UseMove(uint32 id)
{
    int32 index = _move.GetIndex(id);

    switch (index)
    {
    default:
        break;
    case 0:
        if (_move.pp_move0 > 0)
            _move.pp_move0 -= 1;
        break;
    case 1:
        if (_move.pp_move1 > 0)
            _move.pp_move1 -= 1;
        break;
    case 2:
        if (_move.pp_move2 > 0)
            _move.pp_move2 -= 1;
        break;
    case 3:
        if (_move.pp_move3 > 0)
            _move.pp_move3 -= 1;
        break;
    }
}

void Prisma::GenerateMoveSet()
{
    // contain all moves possible for prisma for current level
    std::vector<uint32> move_list;
    for (uint32 l = 0; l < m_level; ++l)
    {
        auto level_list = sObjectMgr->GetPrismaMoveSet(m_id, l);
        if (level_list)
        {
            for (auto& move : *level_list)
                move_list.push_back(move);
        }
    }

    if (move_list.size() == 0)
        return;

    float percent = 100;
    std::vector<uint32> move_used;
    int n_move = (move_list.size() > 4) ? PRISMA_MAX_MOVE : move_list.size();
    for (int i = 0; i < n_move; ++i)
    {
        float calculated_percent = percent + (float(m_level) * 0.5f);
        if (calculated_percent > 100.f)
            calculated_percent = 100.f;

        float rand = frand(0.f, 100.f);

        if (rand > calculated_percent)
            break;

        // generate a move
        {
            uint32 rand_move;
            while (true)
            {
                rand_move = urand(0, move_list.size() - 1);

                if (move_used.size() == 0)
                    break;

                bool found = false;
                for (int y = 0; y < move_used.size(); ++y)
                    if (move_used[y] == move_list[rand_move])
                        found = true;

                if (!found)
                    break;
            }

            uint32 move_id = move_list[rand_move];
            move_used.push_back(move_id);

            const PrismaMoveTemplate* move_template = sObjectMgr->GetPrismaMoveTemplate(move_id);
            if (!move_template)
            {
                TC_LOG_INFO("prisma", "Can't load move template %u, prisma: %s have %u moves loaded", move_id, GetName(), i);
                break;
            }

            _move.SetMove(i, move_id, move_template->PowerPoints);

            // DEBUG
            TC_LOG_INFO("prisma", "%s have loaded move %s (%u), %f percent for %f", GetName(), move_template->Name, move_id, rand, calculated_percent);
        }

        percent *= 0.5f;
    }
}

float Prisma::GetMoveCoefficient(PrismaTypes move, PrismaTypes target_type)
{
    switch (move)
    {
    default:
    case PrismaTypes::TYPELESS:
        return 1.f;
        break;
    case PrismaTypes::NORMAL:
        return GetCoefficientTypeNormal(target_type);
        break;
    case PrismaTypes::FIRE:
        return GetCoefficientTypeFire(target_type);
        break;
    case PrismaTypes::WATER:
        return GetCoefficientTypeWater(target_type);
        break;
    case PrismaTypes::LIGHTNING:
        return GetCoefficientTypeLightning(target_type);
        break;
    case PrismaTypes::GRASS:
        return GetCoefficientTypeGrass(target_type);
        break;
    case PrismaTypes::ELECTRIC:
        return GetCoefficientTypeElectric(target_type);
        break;
    case PrismaTypes::ICE:
        return GetCoefficientTypeIce(target_type);
        break;
    case PrismaTypes::FIGHTING:
        return GetCoefficientTypeFighting(target_type);
        break;
    case PrismaTypes::POISON:
        return GetCoefficientTypePoison(target_type);
        break;
    case PrismaTypes::GROUND:
        return GetCoefficientTypeGround(target_type);
        break;
    case PrismaTypes::FLYING:
        return GetCoefficientTypeFlying(target_type);
        break;
    case PrismaTypes::PSYCHIC:
        return GetCoefficientTypePsychic(target_type);
        break;
    case PrismaTypes::BUG:
        return GetCoefficientTypeBug(target_type);
        break;
    case PrismaTypes::ROCK:
        return GetCoefficientTypeRock(target_type);
        break;
    case PrismaTypes::GHOST:
        return GetCoefficientTypeGhost(target_type);
        break;
    case PrismaTypes::DARK:
        return GetCoefficientTypeDark(target_type);
        break;
    case PrismaTypes::DRAGON:
        return GetCoefficientTypeDragon(target_type);
        break;
    case PrismaTypes::STEEL:
        return GetCoefficientTypeSteel(target_type);
        break;
    case PrismaTypes::FAIRY:
        return GetCoefficientTypeFairy(target_type);
        break;
    }
}

float Prisma::GetMoveCoefficient(PrismaTypes move, PrismaTypes tMove1, PrismaTypes tMove2)
{
    return (GetMoveCoefficient(move, tMove1) * GetMoveCoefficient(move, tMove2));
}

float Prisma::GetCoefficientTypeNormal(PrismaTypes against)
{
    switch (against)
    {
    case PrismaTypes::GHOST:
        return 0.0f;
        break;
    case PrismaTypes::ROCK:
    case PrismaTypes::STEEL:
        return 0.5f;
        break;
    default:
        return 1.f;
        break;
    }
}

float Prisma::GetCoefficientTypeFire(PrismaTypes against)
{
    switch (against)
    {
    case PrismaTypes::FIRE:
    case PrismaTypes::WATER:
    case PrismaTypes::ROCK:
    case PrismaTypes::DRAGON:
        return 0.5f;
        break;
    case PrismaTypes::GRASS:
    case PrismaTypes::ICE:
    case PrismaTypes::BUG:
    case PrismaTypes::STEEL:
        return 2.f;
        break;
    default:
        return 1.f;
        break;
    }
}

float Prisma::GetCoefficientTypeWater(PrismaTypes against)
{
    switch (against)
    {
    case PrismaTypes::WATER:
    case PrismaTypes::GRASS:
    case PrismaTypes::DRAGON:
        return 0.5f;
        break;
    case PrismaTypes::FIRE:
    case PrismaTypes::GROUND:
    case PrismaTypes::ROCK:
        return 2.f;
        break;
    default:
        return 1.f;
        break;
    }
}

float Prisma::GetCoefficientTypeGrass(PrismaTypes against)
{
    switch (against)
    {
    case PrismaTypes::FIRE:
    case PrismaTypes::GRASS:
    case PrismaTypes::POISON:
    case PrismaTypes::FLYING:
    case PrismaTypes::BUG:
    case PrismaTypes::DRAGON:
    case PrismaTypes::STEEL:
        return 0.5f;
        break;
    case PrismaTypes::WATER:
    case PrismaTypes::GROUND:
    case PrismaTypes::ROCK:
        return 2.f;
        break;
    default:
        return 1.f;
        break;
    }
}

float Prisma::GetCoefficientTypeElectric(PrismaTypes against)
{
    switch (against)
    {
    case PrismaTypes::GROUND:
        return 0.f;
        break;
    case PrismaTypes::ELECTRIC:
    case PrismaTypes::GRASS:
    case PrismaTypes::DRAGON:
        return 0.5f;
        break;
    case PrismaTypes::WATER:
    case PrismaTypes::FLYING:
        return 2.f;
        break;
    default:
        return 1.f;
        break;
    }
}

float Prisma::GetCoefficientTypeIce(PrismaTypes against)
{
    switch (against)
    {
    case PrismaTypes::FIRE:
    case PrismaTypes::WATER:
    case PrismaTypes::ICE:
    case PrismaTypes::STEEL:
        return 0.5f;
        break;
    case PrismaTypes::GRASS:
    case PrismaTypes::GROUND:
    case PrismaTypes::FLYING:
    case PrismaTypes::DRAGON:
        return 2.f;
        break;
    default:
        return 1.f;
        break;
    }
}

float Prisma::GetCoefficientTypeFighting(PrismaTypes against)
{
    switch (against)
    {
    case PrismaTypes::GHOST:
        return 0.f;
        break;
    case PrismaTypes::POISON:
    case PrismaTypes::FLYING:
    case PrismaTypes::PSYCHIC:
    case PrismaTypes::BUG:
    case PrismaTypes::FAIRY:
        return 0.5f;
        break;
    case PrismaTypes::NORMAL:
    case PrismaTypes::ICE:
    case PrismaTypes::ROCK:
    case PrismaTypes::DARK:
    case PrismaTypes::STEEL:
        return 2.f;
        break;
    default:
        return 1.f;
        break;
    }
}

float Prisma::GetCoefficientTypePoison(PrismaTypes against)
{
    switch (against)
    {
    case PrismaTypes::STEEL:
        return 0.f;
        break;
    case PrismaTypes::POISON:
    case PrismaTypes::GROUND:
    case PrismaTypes::ROCK:
    case PrismaTypes::GHOST:
        return 0.5f;
        break;
    case PrismaTypes::GRASS:
    case PrismaTypes::FAIRY:
        return 2.f;
        break;
    default:
        return 1.f;
        break;
    }
}

float Prisma::GetCoefficientTypeGround(PrismaTypes against)
{
    switch (against)
    {
    case PrismaTypes::FLYING:
        return 0.f;
        break;
    case PrismaTypes::GRASS:
    case PrismaTypes::BUG:
        return 0.5f;
        break;
    case PrismaTypes::FIRE:
    case PrismaTypes::ELECTRIC:
    case PrismaTypes::POISON:
    case PrismaTypes::ROCK:
    case PrismaTypes::STEEL:
        return 2.f;
        break;
    default:
        return 1.f;
        break;
    }
}

float Prisma::GetCoefficientTypeFlying(PrismaTypes against)
{
    switch (against)
    {
    case PrismaTypes::ELECTRIC:
    case PrismaTypes::ROCK:
    case PrismaTypes::STEEL:
        return 0.5f;
        break;
    case PrismaTypes::GRASS:
    case PrismaTypes::FIGHTING:
    case PrismaTypes::BUG:
        return 2.f;
        break;
    default:
        return 1.f;
        break;
    }
}

float Prisma::GetCoefficientTypePsychic(PrismaTypes against)
{
    switch (against)
    {
    case PrismaTypes::DARK:
        return 0.f;
        break;
    case PrismaTypes::PSYCHIC:
    case PrismaTypes::STEEL:
        return 0.5f;
        break;
    case PrismaTypes::FIGHTING:
    case PrismaTypes::POISON:
        return 2.f;
        break;
    default:
        return 1.f;
        break;
    }
}

float Prisma::GetCoefficientTypeBug(PrismaTypes against)
{
    switch (against)
    {
    case PrismaTypes::FIRE:
    case PrismaTypes::FIGHTING:
    case PrismaTypes::POISON:
    case PrismaTypes::FLYING:
    case PrismaTypes::GHOST:
    case PrismaTypes::STEEL:
    case PrismaTypes::FAIRY:
        return 0.5f;
        break;
    case PrismaTypes::GRASS:
    case PrismaTypes::PSYCHIC:
    case PrismaTypes::DARK:
        return 2.f;
        break;
    default:
        return 1.f;
        break;
    }
}

float Prisma::GetCoefficientTypeRock(PrismaTypes against)
{
    switch (against)
    {
    case PrismaTypes::FIGHTING:
    case PrismaTypes::GROUND:
    case PrismaTypes::STEEL:
        return 0.5f;
        break;
    case PrismaTypes::FIRE:
    case PrismaTypes::ICE:
    case PrismaTypes::FLYING:
    case PrismaTypes::BUG:
        return 2.f;
        break;
    default:
        return 1.f;
        break;
    }
}

float Prisma::GetCoefficientTypeGhost(PrismaTypes against)
{
    switch (against)
    {
    case PrismaTypes::NORMAL:
        return 0.f;
        break;
    case PrismaTypes::DARK:
        return 0.5f;
        break;
    case PrismaTypes::PSYCHIC:
    case PrismaTypes::GHOST:
        return 2.f;
        break;
    default:
        return 1.f;
        break;
    }
}

float Prisma::GetCoefficientTypeDragon(PrismaTypes against)
{
    switch (against)
    {
    case PrismaTypes::FAIRY:
        return 0.f;
        break;
    case PrismaTypes::STEEL:
        return 0.5f;
        break;
    case PrismaTypes::DRAGON:
        return 2.f;
        break;
    default:
        return 1.f;
        break;
    }
}

float Prisma::GetCoefficientTypeDark(PrismaTypes against)
{
    switch (against)
    {
    case PrismaTypes::FIGHTING:
    case PrismaTypes::DARK:
    case PrismaTypes::FAIRY:
        return 0.5f;
        break;
    case PrismaTypes::PSYCHIC:
    case PrismaTypes::GHOST:
        return 2.f;
        break;
    default:
        return 1.f;
        break;
    }
}

float Prisma::GetCoefficientTypeSteel(PrismaTypes against)
{
    switch (against)
    {
    case PrismaTypes::FIRE:
    case PrismaTypes::WATER:
    case PrismaTypes::ELECTRIC:
    case PrismaTypes::STEEL:
        return 0.5f;
        break;
    case PrismaTypes::ICE:
    case PrismaTypes::ROCK:
    case PrismaTypes::FAIRY:
        return 2.f;
        break;
    default:
        return 1.f;
        break;
    }
}

float Prisma::GetCoefficientTypeFairy(PrismaTypes against)
{
    switch (against)
    {
    case PrismaTypes::FIRE:
    case PrismaTypes::POISON:
    case PrismaTypes::STEEL:
        return 0.5f;
        break;
    case PrismaTypes::FIGHTING:
    case PrismaTypes::DRAGON:
    case PrismaTypes::DARK:
        return 2.f;
        break;
    default:
        return 1.f;
        break;
    }
}

uint32 Prisma::GetRequiredExperienceForNextLevel(int level, PrismaExperienceTypes type)
{
    int result = 0;

    switch (type)
    {
    case PrismaExperienceTypes::SLOW:
        result = (GetSlowExperience(level + 1) - GetSlowExperience(level));
        break;
    case PrismaExperienceTypes::MEDIUM_SLOW:
        result = (GetMediumSlowExperience(level + 1) - GetMediumSlowExperience(level));
        break;
    default:
    case PrismaExperienceTypes::MEDIUM_FAST:
        result = (GetMediumFastExperience(level + 1) - GetMediumFastExperience(level));
        break;
    case PrismaExperienceTypes::FAST:
        result = (GetFastExperience(level + 1) - GetFastExperience(level));
        break;
    };

    if (result < 0)
        return uint32(0);

    return uint32(result);
}

int Prisma::GetGainExperience(Prisma* self, Prisma* killed, bool against_trainer, bool use_multi_exp, int number_prisma_during_combat, float other_multiplicator)
{
    int level = self->GetPrismaLevel();
    int target_level = killed->GetPrismaLevel();

    int target_base_experience = 64; // default
    const PrismaTemplate* killed_template = sObjectMgr->GetPrismaTemplate(killed->GetPrismaEntry());
    if (killed_template)
        target_base_experience = killed_template->BaseExperience;

    int   A = (target_level * 2) + 10;
    float B = (float(target_base_experience * target_level) / 5.f);
    int   C = (level + target_level + 10);

    against_trainer ? B *= 1.5f : NULL;
    use_multi_exp ? B /= 2.f : NULL;
    B /= ((number_prisma_during_combat <= 0) ? 1.f : float(number_prisma_during_combat));

    int gain = floor((floor(sqrt(double(A)) * (A * A) * B) / floor(sqrt(C) * (C * C)))) + 1;

    return floor(gain * other_multiplicator);
}

int Prisma::GetSlowExperience(int level)
{
    return int((5.f * float(level * level * level)) / 4.f);
}

int Prisma::GetMediumSlowExperience(int level)
{
    return int(((6.f / 5.f) * float(level * level * level)) - (15 * (level * level)) + (100 * level) - 140);
}

int Prisma::GetMediumFastExperience(int level)
{
    return (level * level * level);
}

int Prisma::GetFastExperience(int level)
{
    return int((4.f * float(level * level * level)) / 5.f);
}
