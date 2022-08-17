#pragma once

#include "SharedDefines.h"
#include <string>
#include <unordered_map>
#include <vector>
#include <cmath>

#define PRISMA_TEMPLATE_RESERVED_MIN        45000
#define PRISMA_TEMPLATE_RESERVED_MAX        55000

struct TC_GAME_API PrismaTemplate
{
    uint32 Entry;
    std::string Name;
    std::string Description;
    uint32 DisplayID;
    float  Scale;
    uint32 EvolveLevel;
    uint32 EvolveID;
    uint32 Type;
    uint32 BaseExperience;
    uint32 Stamina;
    uint32 Attack;
    uint32 Defense;
    uint32 SpecialAttack;
    uint32 SpecialDefense;
    uint32 Speed;
    uint32 EVStamina;
    uint32 EVAttack;
    uint32 EVDefense;
    uint32 EVSpecialAttack;
    uint32 EVSpecialDefense;
    uint32 EVSpeed;
};

enum class PrismaStat
{
    STAMINA,
    ATTACK,
    DEFENSE,
    SPECIAL_ATTACK,
    SPECIAL_DEFENSE,
    SPEED
};

#define NUM_MAX_STAT        6

enum class PrismaNature
{
                // INCREASE 10%     DECREASE 10%
    HARDY,
    LONELY,     // attack           defense
    BRAVE,      // attack           speed
    ADAMANT,    // attack           sp.attack
    NAUGHTY,    // attack           sp.defense
    BOLD,       // defense          attack
    DOCILE,
    RELAXED,    // defense          speed
    IMPISH,     // defense          sp.attack
    LAX,        // defense          sp.defense
    TIMID,      // speed            attack
    HASTY,      // speed            defense
    SERIOUS,
    JOLLY,      // speed            sp.attack
    NAIVE,      // speed            sp.defense
    MODEST,     // sp.attack        attack
    MILD,       // sp.attack        defense
    QUIET,      // sp.attack        speed
    BASHFUL,
    RASH,       // sp.attack        sp.defense
    CALM,       // sp.defense       attack
    GENTLE,     // sp.defense       defense
    SASSY,      // sp.defense       speed
    CAREFUL,    // sp.defense       sp.attack
    QUIRKY
};

#define NUM_MAX_NATURE      25

struct TC_GAME_API IndividualValue
{
    uint8 stamina;
    uint8 attack;
    uint8 defense;
    uint8 special_attack;
    uint8 special_defense;
    uint8 speed;

    std::vector<PrismaStat> GetMax()
    {
        std::vector<PrismaStat> max;
        uint8 maximum = std::max({ stamina, attack, defense, special_attack, special_defense, speed });
        if (maximum == stamina)             max.push_back(PrismaStat::STAMINA);
        if (maximum == attack)              max.push_back(PrismaStat::ATTACK);
        if (maximum == defense)             max.push_back(PrismaStat::DEFENSE);
        if (maximum == special_attack)      max.push_back(PrismaStat::SPECIAL_ATTACK);
        if (maximum == special_defense)     max.push_back(PrismaStat::SPECIAL_DEFENSE);
        if (maximum == speed)               max.push_back(PrismaStat::SPEED);

        return max;
    }

    uint8 GetStat(PrismaStat _stat)
    {
        switch (_stat)
        {
        case PrismaStat::STAMINA:             return stamina;             break;
        case PrismaStat::ATTACK:              return attack;              break;
        case PrismaStat::DEFENSE:             return defense;             break;
        case PrismaStat::SPECIAL_ATTACK:      return special_attack;      break;
        case PrismaStat::SPECIAL_DEFENSE:     return special_defense;     break;
        case PrismaStat::SPEED:               return speed;               break;
        }

        return 0;
    }
};

struct TC_GAME_API Nature
{
    uint8 nature;

    PrismaNature GetNature()
    {
        switch (nature)
        {
        default:
        case 0: return PrismaNature::HARDY; break;
        case 1: return PrismaNature::LONELY; break;
        case 2: return PrismaNature::BRAVE; break;
        case 3: return PrismaNature::ADAMANT; break;
        case 4: return PrismaNature::NAUGHTY; break;
        case 5: return PrismaNature::BOLD; break;
        case 6: return PrismaNature::DOCILE; break;
        case 7: return PrismaNature::RELAXED; break;
        case 8: return PrismaNature::IMPISH; break;
        case 9: return PrismaNature::LAX; break;
        case 10: return PrismaNature::TIMID; break;
        case 11: return PrismaNature::HASTY; break;
        case 12: return PrismaNature::SERIOUS; break;
        case 13: return PrismaNature::JOLLY; break;
        case 14: return PrismaNature::NAIVE; break;
        case 15: return PrismaNature::MODEST; break;
        case 16: return PrismaNature::MILD; break;
        case 17: return PrismaNature::QUIET; break;
        case 18: return PrismaNature::BASHFUL; break;
        case 19: return PrismaNature::RASH; break;
        case 20: return PrismaNature::CALM; break;
        case 21: return PrismaNature::GENTLE; break;
        case 22: return PrismaNature::SASSY; break;
        case 23: return PrismaNature::CAREFUL; break;
        case 24: return PrismaNature::QUIRKY; break;
        }
    }

    std::string GetName()
    {
        switch (nature)
        {
        default:
        case 0: return "HARDY"; break;
        case 1: return "LONELY"; break;
        case 2: return "BRAVE"; break;
        case 3: return "ADAMANT"; break;
        case 4: return "NAUGHTY"; break;
        case 5: return "BOLD"; break;
        case 6: return "DOCILE"; break;
        case 7: return "RELAXED"; break;
        case 8: return "IMPISH"; break;
        case 9: return "LAX"; break;
        case 10: return "TIMID"; break;
        case 11: return "HASTY"; break;
        case 12: return "SERIOUS"; break;
        case 13: return "JOLLY"; break;
        case 14: return "NAIVE"; break;
        case 15: return "MODEST"; break;
        case 16: return "MILD"; break;
        case 17: return "QUIET"; break;
        case 18: return "BASHFUL"; break;
        case 19: return "RASH"; break;
        case 20: return "CALM"; break;
        case 21: return "GENTLE"; break;
        case 22: return "SASSY"; break;
        case 23: return "CAREFUL"; break;
        case 24: return "QUIRKY"; break;
        }
    }
};
