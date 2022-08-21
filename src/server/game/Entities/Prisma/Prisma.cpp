
#include "Prisma.h"

Prisma::Prisma(bool isWorldObject)
    : Creature(isWorldObject), _iv_generated(false), _nature_generated(false), _gender_generated(false),
    m_guid(0), m_id(0), m_experience(0), m_item(-1), m_level(1), m_current_stamina(0),
    m_status_volatile_flags(0), m_status_non_volatile_flags(0), m_status_volatile_combat_flags(0)
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

    // move set is for now totally empty
    {
        _move.Init();
    }

    GenerateCalculatedStat();
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

uint32 Prisma::CalculateDamage(Prisma* attacker, Prisma* target, uint32 move_id, bool is_second_strike)
{
    if (!attacker || !target || move_id == 0)
        return 0;

    const PrismaMoveTemplate* move_template = sObjectMgr->GetPrismaMoveTemplate(move_id);
    if (!move_template)
        return 0;

    // START BASE
    uint32 damage = (2 * target->GetPrismaLevel() / 5) + 2;
    damage *= move_template->BasePower;

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

    damage *= (AttackStat / DefenseStat);
    damage /= 50;
    damage += 2;
    // END BASE

    // SINGLE OR MULTI TARGET
    damage *= (move_template->SelectionType == PrismaMoveSelectionTypes::TARGET) ? 1.f : 0.75f;

    // SECOND STRIKE
    damage *= (is_second_strike) ? 0.25f : 1.f;

    // TODO: calculate weather

    // TODO: add check is target can avoid the crit damage
    damage *= (frand(0.f, 100.f) < move_template->CritRate) ? 1.5f : 1.f;

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

    return damage;
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
    data.Move2 = _move.pp_move2;
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
    m_status_non_volatile_flags |= uint32(_status);
}

void Prisma::RemoveNonVolatileStatus(PrismaNonVolatileStatus _status)
{
    m_status_non_volatile_flags &= ~(uint32(_status));
}

bool Prisma::HasNonVolatileStatus(PrismaNonVolatileStatus _status)
{
    return (m_status_non_volatile_flags & uint32(_status)) != 0;
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

// this add to grid and to DB `creature`
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
    case PrismaTypes::NATURE:
        return GetCoefficientTypeNature(target_type);
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
    case PrismaTypes::EARTH:
        return GetCoefficientTypeEarth(target_type);
        break;
    case PrismaTypes::AIR:
        return GetCoefficientTypeAir(target_type);
        break;
    case PrismaTypes::PSIONIC:
        return GetCoefficientTypePsionic(target_type);
        break;
    case PrismaTypes::PHANTOM:
        return GetCoefficientTypePhantom(target_type);
        break;
    case PrismaTypes::DARK:
        return GetCoefficientTypeDark(target_type);
        break;
    case PrismaTypes::METAL:
        return GetCoefficientTypeMetal(target_type);
        break;
    case PrismaTypes::LIGHT:
        return GetCoefficientTypeLight(target_type);
        break;
    case PrismaTypes::SOUND:
        return GetCoefficientTypeSound(target_type);
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
    case PrismaTypes::PHANTOM:
        return 0.0f;
        break;
    case PrismaTypes::METAL:
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
        return 0.5f;
        break;
    case PrismaTypes::NATURE:
    case PrismaTypes::ICE:
    case PrismaTypes::METAL:
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
    case PrismaTypes::FIRE:
    case PrismaTypes::EARTH:
        return 2.f;
        break;
    case PrismaTypes::WATER:
    case PrismaTypes::NATURE:
    case PrismaTypes::SOUND:
        return 0.5f;
        break;
    default:
        return 1.f;
        break;
    }
}

float Prisma::GetCoefficientTypeLightning(PrismaTypes against)
{
    switch (against)
    {
    case PrismaTypes::WATER:
    case PrismaTypes::AIR:
    case PrismaTypes::SOUND:
        return 2.f;
        break;
    case PrismaTypes::LIGHTNING:
    case PrismaTypes::NATURE:
        return 0.5f;
        break;
    case PrismaTypes::EARTH:
        return 0.f;
        break;
    default:
        return 1.f;
        break;
    }
}

float Prisma::GetCoefficientTypeNature(PrismaTypes against)
{
    switch (against)
    {
    case PrismaTypes::FIRE:
    case PrismaTypes::NATURE:
    case PrismaTypes::POISON:
    case PrismaTypes::AIR:
    case PrismaTypes::METAL:
        return 0.5f;
        break;
    case PrismaTypes::WATER:
    case PrismaTypes::EARTH:
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
    case PrismaTypes::METAL:
        return 0.5f;
        break;
    case PrismaTypes::NATURE:
    case PrismaTypes::EARTH:
    case PrismaTypes::AIR:
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
    case PrismaTypes::NORMAL:
    case PrismaTypes::ICE:
    case PrismaTypes::METAL:
        return 2.f;
        break;
    case PrismaTypes::POISON:
    case PrismaTypes::AIR:
    case PrismaTypes::PSIONIC:
        return 0.5f;
        break;
    case PrismaTypes::PHANTOM:
        return 0.f;
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
    case PrismaTypes::NATURE:
    case PrismaTypes::LIGHT:
        return 2.f;
        break;
    case PrismaTypes::POISON:
    case PrismaTypes::EARTH:
    case PrismaTypes::PHANTOM:
    case PrismaTypes::DARK:
        return 0.5f;
        break;
    case PrismaTypes::METAL:
        return 0.f;
        break;
    default:
        return 1.f;
        break;
    }
}

float Prisma::GetCoefficientTypeEarth(PrismaTypes against)
{
    switch (against)
    {
    case PrismaTypes::FIRE:
    case PrismaTypes::LIGHTNING:
    case PrismaTypes::POISON:
    case PrismaTypes::METAL:
        return 2.f;
        break;
    case PrismaTypes::NATURE:
        return 0.5f;
        break;
    case PrismaTypes::AIR:
        return 0.f;
        break;
    default:
        return 1.f;
        break;
    }
}

float Prisma::GetCoefficientTypeAir(PrismaTypes against)
{
    switch (against)
    {
    case PrismaTypes::LIGHTNING:
    case PrismaTypes::METAL:
    case PrismaTypes::SOUND:
        return 0.5f;
        break;
    case PrismaTypes::NATURE:
    case PrismaTypes::FIGHTING:
        return 2.f;
        break;
    default:
        return 1.f;
        break;
    }
}

float Prisma::GetCoefficientTypePsionic(PrismaTypes against)
{
    switch (against)
    {
    case PrismaTypes::FIGHTING:
    case PrismaTypes::POISON:
        return 2.f;
        break;
    case PrismaTypes::PSIONIC:
    case PrismaTypes::METAL:
        return 0.5f;
        break;
    case PrismaTypes::DARK:
    case PrismaTypes::LIGHT:
        return 0.f;
        break;
    default:
        return 1.f;
        break;
    }
}

float Prisma::GetCoefficientTypePhantom(PrismaTypes against)
{
    switch (against)
    {
    case PrismaTypes::NORMAL:
        return 0.f;
        break;
    case PrismaTypes::PSIONIC:
    case PrismaTypes::PHANTOM:
        return 2.f;
        break;
    case PrismaTypes::DARK:
    case PrismaTypes::LIGHT:
        return 0.5f;
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
    case PrismaTypes::METAL:
        return 0.5f;
        break;
    case PrismaTypes::PSIONIC:
    case PrismaTypes::PHANTOM:
    case PrismaTypes::LIGHT:
        return 2.f;
        break;
    default:
        return 1.f;
        break;
    }
}

float Prisma::GetCoefficientTypeMetal(PrismaTypes against)
{
    switch (against)
    {
    case PrismaTypes::FIRE:
    case PrismaTypes::WATER:
    case PrismaTypes::LIGHTNING:
    case PrismaTypes::METAL:
        return 0.5f;
        break;
    case PrismaTypes::ICE:
    case PrismaTypes::DARK:
    case PrismaTypes::LIGHT:
        return 2.f;
        break;
    default:
        return 1.f;
        break;
    }
}

float Prisma::GetCoefficientTypeLight(PrismaTypes against)
{
    switch (against)
    {
    case PrismaTypes::FIRE:
    case PrismaTypes::POISON:
    case PrismaTypes::METAL:
    case PrismaTypes::LIGHT:
        return 0.5f;
        break;
    case PrismaTypes::FIGHTING:
    case PrismaTypes::DARK:
        return 2.f;
        break;
    default:
        return 1.f;
        break;
    }
}

float Prisma::GetCoefficientTypeSound(PrismaTypes against)
{
    switch (against)
    {
    case PrismaTypes::WATER:
    case PrismaTypes::AIR:
        return 2.f;
        break;
    case PrismaTypes::LIGHTNING:
        return 0.5f;
        break;
    case PrismaTypes::SOUND:
        return 0.f;
        break;
    default:
        return 1.f;
        break;
    }
}

int Prisma::GetRequiredExperienceForNextLevel(int level, PrismaExperienceTypes type)
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
        return 0;

    return result;
}

int Prisma::GetGainExperience(int level, int target_level, int target_base_experience, bool against_trainer, bool use_multi_exp, int number_prisma_during_combat, float other_multiplicator)
{
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
