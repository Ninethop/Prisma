
#include "Prisma.h"

Prisma::Prisma(bool isWorldObject)
    : Creature(isWorldObject), _iv_generated(false), _nature_generated(false), _gender_generated(false),
    m_guid(0), m_id(0), m_experience(0), m_item(-1), m_level(1)
{ }

void Prisma::InitializePrisma()
{
    // this also generate characteristic
    if (!IndividualValueIsGenerated())
        GenerateIndividualValue();

    if (!NatureIsGenerated())
        GenerateNature();

    if (!GenderIsGenerated())
        GenerateGender();
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
        uint16 numerator = ((2 * _template->Stamina) + _iv.stamina + (_ev.stamina / 4)) * GetLevel();
        uint16 result = (numerator / 100) + (GetLevel() + 10);

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

    uint16 numerator = (2 * base_value + iv_value + (ev_value / 4)) * GetLevel();
    uint16 result = ((numerator / 100) + 5) * nature_factor;

    (result < MIN_STAT_VALUE) ? result = MIN_STAT_VALUE : NULL;
    return result;
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
    prisma->InitializePrismaFromGuid(guid);
    if (!prisma->LoadFromDB(db_guid, map, true, true))
    {
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
    _iv.stamina         = irand(IV_MIN_VALUE, IV_MAX_VALUE);
    _iv.attack          = irand(IV_MIN_VALUE, IV_MAX_VALUE);
    _iv.defense         = irand(IV_MIN_VALUE, IV_MAX_VALUE);
    _iv.special_attack  = irand(IV_MIN_VALUE, IV_MAX_VALUE);
    _iv.special_defense = irand(IV_MIN_VALUE, IV_MAX_VALUE);
    _iv.speed           = irand(IV_MIN_VALUE, IV_MAX_VALUE);

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

float Prisma::GetAttackCoefficient(PrismaTypes attack, PrismaTypes target_type)
{
    switch (attack)
    {
    default:
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

float Prisma::GetAttackCoefficient(PrismaTypes attack, PrismaTypes target_type1, PrismaTypes target_type2)
{
    return (GetAttackCoefficient(attack, target_type1) * GetAttackCoefficient(attack, target_type2));
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
