
#pragma once

#include "Creature.h"
#include "PrismaData.h"

class TC_GAME_API Prisma : public Creature
{
public:
    explicit Prisma(bool isWorldObject = false);

    void GenerateIV();
    bool IVIsGenerated() { return _iv_generated; };
    IndividualValue const GetIV() { return _iv; };
    uint8 const GetStatIV(PrismaStat stat) { return _iv.GetStat(stat); };

    void GenerateCharacteristic();
    std::string const GetCharacteristic() { return _characteristic; };

    void GenerateNature();
    bool NatureIsGenerated() { return _nature_generated; };
    PrismaNature const GetNature() { return _nature.GetNature(); };
    uint8 const GetNatureID() { return _nature.nature; };
    std::string const GetNatureName() { return _nature.GetName(); };

private:
    IndividualValue _iv;
    bool _iv_generated;
    void GenerateCharacteristicFromStat(PrismaStat stat);
    std::string _characteristic;

    Nature _nature;
    bool _nature_generated;
};
