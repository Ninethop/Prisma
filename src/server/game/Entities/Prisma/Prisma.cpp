
#include "Prisma.h"

Prisma::Prisma(bool isWorldObject)
    : Creature(isWorldObject), _iv_generated(false), _nature_generated(false), _gender_generated(false)
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

void Prisma::InitializePrismaFromGuid(uint32 guid)
{

}

// this add to grid and to DB `creature`
Prisma* Prisma::Invoke(Player* owner, uint32 id)
{
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
    if (num > 5)
        return uint32(0);

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
        {
            // if id == 0, then error
            uint32 prisma_id = (*result_id)[0].GetUInt32();
            if (prisma_id == 0)
            {
                TC_LOG_INFO("prisma", "[ERROR]: Prisma with GUID:%u doesn't have ID in `prisma` table.", prisma_guid);
                return uint32(0);
            }

            return prisma_id;
        }
    }

    return uint32(0);
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
    _characteristic.data = ((uint8)(_stat) * (NUM_MAX_STAT - 1)) + (_iv.GetStat(_stat) % 5);
}

void Prisma::GenerateNature()
{
    _nature.Set(PrismaNatures(irand(0, NUM_MAX_NATURE - 1)));
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
