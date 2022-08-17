
#pragma once

#include "Map.h"
#include "Creature.h"
#include "PrismaData.h"
#include "ObjectMgr.h"

class TC_GAME_API Prisma : public Creature
{
public:
    explicit Prisma(bool isWorldObject = false);

    void InitializePrisma();
    void InitializePrismaFromGuid(uint32 guid);
    uint32 GetPrismaEntry() { return (GetEntry() - PRISMA_TEMPLATE_RESERVED_MIN); }

    static Prisma* Invoke(Player* owner, uint32 id);
    static uint32 GetTeamID(Player* player, uint8 num = 0);

    bool IndividualValueIsGenerated() { return _iv_generated; };
    PrismaIndividualValue const GetIV() { return _iv; };
    uint8 const GetStatIV(PrismaStats stat) { return _iv.GetStat(stat); };

    std::string const GetCharacteristic() { return _characteristic.GetCharacteristicName(); };

    bool NatureIsGenerated() { return _nature_generated; };
    PrismaNatures const GetNature() { return _nature.GetNature(); };
    uint8 const GetNatureID() { return _nature.nature; };
    std::string const GetNatureName() { return _nature.GetName(); };

    bool GenderIsGenerated() { return _gender_generated; };
    PrismaGenders const GetGender() { return _gender.GetGender(); };
    std::string const GetGenderName() { return _gender.GetGenderName(); };

private:
    PrismaIndividualValue _iv;
    PrismaNature _nature;
    PrismaGender _gender;
    PrismaCharacteristic _characteristic;

    void GenerateIndividualValue();
    void GenerateCharacteristic();
    void GenerateNature();
    void GenerateGender();

    bool _iv_generated;
    bool _nature_generated;
    bool _gender_generated;
};
