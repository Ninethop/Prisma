
#include "Prisma.h"

Prisma::Prisma(bool isWorldObject)
    : Creature(isWorldObject), _iv_generated(false), _nature_generated(false)
{
    //GenerateIV();
    //GenerateNature();
}

void Prisma::GenerateIV()
{
    if (_iv_generated)
        return;

    _iv.stamina = irand(0, 31);
    _iv.attack = irand(0, 31);
    _iv.defense = irand(0, 31);
    _iv.special_attack = irand(0, 31);
    _iv.special_defense = irand(0, 31);
    _iv.speed = irand(0, 31);

    _iv_generated = true;

    GenerateCharacteristic();
}

void Prisma::GenerateCharacteristic()
{
    std::vector<PrismaStat> max = _iv.GetMax();
    if (max.size() == 0) // error
        return; 

    // only one stat at max
    if (max.size() == 1)
    {
        GenerateCharacteristicFromStat(max[0]);
    }
    else // choose one randomly
    {
        int rand = irand(0, max.size() - 1);
        GenerateCharacteristicFromStat(max[rand]);
    }
}

void Prisma::GenerateCharacteristicFromStat(PrismaStat stat)
{
    const std::string _stamina_c[5] = { "Loves to eat", "Often dozes off", "Often scatters things", "Scatters things often", "Like to relax" };
    const std::string _attack_c[5] = { "Proud of its power", "Likes to thrash about", "A little quick tempered", "Likes to fight", "Quick tempered" };
    const std::string _defense_c[5] = { "Sturdy body", "Capable of taking hits", "Highly persistent", "Good endurance", "Good perseverance" };
    const std::string _special_attack_c[5] = { "Highly curious", "Mischievous", "Thoroughly cunning", "Often lost in thought", "Very finicky" };
    const std::string _special_defense_c[5] = { "Strong willed", "Somewhat vain", "Strongly defiant", "Hates to lose", "Somewhat stubborn" };
    const std::string _speed_c[5] = { "Likes to run", "Alert to sounds", "Impetuous and silly", "Somewhat of a clown", "Quick to flee" };

    TC_LOG_INFO("prisma", "hre : %u", (uint8)stat);

    switch (stat)
    {
    case PrismaStat::STAMINA:
        _characteristic = _stamina_c[_iv.GetStat(stat) % 5];
        break;
    case PrismaStat::ATTACK:
        _characteristic = _attack_c[_iv.GetStat(stat) % 5];
        break;
    case PrismaStat::DEFENSE:
        _characteristic = _defense_c[_iv.GetStat(stat) % 5];
        break;
    case PrismaStat::SPECIAL_ATTACK:
        _characteristic = _special_attack_c[_iv.GetStat(stat) % 5];
        break;
    case PrismaStat::SPECIAL_DEFENSE:
        _characteristic = _special_defense_c[_iv.GetStat(stat) % 5];
        break;
    case PrismaStat::SPEED:
        _characteristic = _speed_c[_iv.GetStat(stat) % 5];
        break;
    }

    TC_LOG_INFO("prisma", "hhhh");
    TC_LOG_INFO("prisma", "%s", _characteristic);
}

void Prisma::GenerateNature()
{
    if (_nature_generated)
        return;

    _nature.nature = irand(0, NUM_MAX_NATURE - 1);
    _nature_generated = true;
}
