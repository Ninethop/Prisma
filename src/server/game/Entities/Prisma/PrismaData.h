#pragma once

#include "SharedDefines.h"
#include <string>
#include <unordered_map>
#include <vector>
#include <cmath>

#define PRISMA_TEMPLATE_RESERVED_MIN        45000
#define PRISMA_TEMPLATE_RESERVED_MAX        55000

#define NUM_MAX_NATURE      25
#define NUM_MAX_GENDER      3
#define NUM_MAX_STAT        6

#define IV_MIN_VALUE        0
#define IV_MAX_VALUE        31

struct TC_GAME_API PrismaTemplate
{
    uint32 Entry;
    std::string Name;
    std::string Description;
    float GenderFactor;
    uint32 DisplayID;
    float  Scale;
    uint32 Faction;
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

enum class PrismaStats
{
    STAMINA,
    ATTACK,
    DEFENSE,
    SPECIAL_ATTACK,
    SPECIAL_DEFENSE,
    SPEED
};

enum class PrismaNatures
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

enum class PrismaGenders
{
    MALE,
    FEMALE,
    NO_GENDER,
};

struct TC_GAME_API PrismaIndividualValue
{
    uint8 stamina;
    uint8 attack;
    uint8 defense;
    uint8 special_attack;
    uint8 special_defense;
    uint8 speed;

    std::vector<PrismaStats> GetMax()
    {
        std::vector<PrismaStats> max;
        uint8 maximum = std::max({ stamina, attack, defense, special_attack, special_defense, speed });
        if (maximum == stamina)             max.push_back(PrismaStats::STAMINA);
        if (maximum == attack)              max.push_back(PrismaStats::ATTACK);
        if (maximum == defense)             max.push_back(PrismaStats::DEFENSE);
        if (maximum == special_attack)      max.push_back(PrismaStats::SPECIAL_ATTACK);
        if (maximum == special_defense)     max.push_back(PrismaStats::SPECIAL_DEFENSE);
        if (maximum == speed)               max.push_back(PrismaStats::SPEED);

        return max;
    }

    uint8 GetStat(PrismaStats _stat)
    {
        switch (_stat)
        {
        case PrismaStats::STAMINA:             return stamina;             break;
        case PrismaStats::ATTACK:              return attack;              break;
        case PrismaStats::DEFENSE:             return defense;             break;
        case PrismaStats::SPECIAL_ATTACK:      return special_attack;      break;
        case PrismaStats::SPECIAL_DEFENSE:     return special_defense;     break;
        case PrismaStats::SPEED:               return speed;               break;
        }

        return 0;
    }
};

struct TC_GAME_API PrismaNature
{
    uint8 nature;

    void Set(PrismaNatures val)
    {
        nature = uint8(val);
    }

    PrismaNatures GetNature()
    {
        if (nature >= 0 && nature < NUM_MAX_NATURE)
            return (PrismaNatures)nature;

        return PrismaNatures::HARDY;
    }

    std::string GetName()
    {
        switch (nature)
        {
        default:
        case 0: return "Hardy"; break;
        case 1: return "Lonely"; break;
        case 2: return "Brave"; break;
        case 3: return "Adamant"; break;
        case 4: return "Naughty"; break;
        case 5: return "Bold"; break;
        case 6: return "Docile"; break;
        case 7: return "Relaxed"; break;
        case 8: return "Impish"; break;
        case 9: return "Lax"; break;
        case 10: return "Timid"; break;
        case 11: return "Hasty"; break;
        case 12: return "Serious"; break;
        case 13: return "Jolly"; break;
        case 14: return "Naive"; break;
        case 15: return "Modest"; break;
        case 16: return "Mild"; break;
        case 17: return "Quiet"; break;
        case 18: return "Bashful"; break;
        case 19: return "Rash"; break;
        case 20: return "Calm"; break;
        case 21: return "Gentle"; break;
        case 22: return "Sassy"; break;
        case 23: return "Careful"; break;
        case 24: return "Quirky"; break;
        }
    }
};

struct TC_GAME_API PrismaGender
{
    uint8 gender;

    void Set(PrismaGenders gen)
    {
        gender = uint8(gen);
    }

    PrismaGenders GetGender()
    {
        if (gender >= 0 && gender < NUM_MAX_GENDER)
            return (PrismaGenders)gender;

        return PrismaGenders::NO_GENDER;
    }

    std::string GetGenderName()
    {
        switch (gender)
        {
        default:
        case (uint8)PrismaGenders::NO_GENDER: return ""; break; // maybe retrun "Legendary" ?
        case (uint8)PrismaGenders::MALE: return "Male"; break;
        case (uint8)PrismaGenders::FEMALE: return "Female"; break;
        }
    }
};

struct TC_GAME_API PrismaCharacteristic
{
    int8 data = -1;

    std::string GetCharacteristicName()
    {
        switch (data)
        {
            // stamina
        default:
        case 0: return "Loves to eat"; break;
        case 1: return "Often dozes off"; break;
        case 2: return "Often scatters things"; break;
        case 3: return "Scatters things often"; break;
        case 4: return "Like to relax"; break;
            // attack
        case 5: return "Proud of its power"; break;
        case 6: return "Likes to thrash about"; break;
        case 7: return "A little quick tempered"; break;
        case 8: return "Likes to fight"; break;
        case 9: return "Quick tempered"; break;
            // defense
        case 10: return "Sturdy body"; break;
        case 11: return "Capable of taking hits"; break;
        case 12: return "Highly persistent"; break;
        case 13: return "Good endurance"; break;
        case 14: return "Good perseverance"; break;
            // special attack
        case 15: return "Highly curious"; break;
        case 16: return "Mischievous"; break;
        case 17: return "Thoroughly cunning"; break;
        case 18: return "Often lost in thought"; break;
        case 19: return "Very finicky"; break;
            // special defense
        case 20: return "Strong willed"; break;
        case 21: return "Somewhat vai"; break;
        case 22: return "Strongly defiant"; break;
        case 23: return "Hates to lose"; break;
        case 24: return "Somewhat stubborn"; break;
            // speed
        case 25: return "Likes to run"; break;
        case 26: return "Alert to sounds"; break;
        case 27: return "Impetuous and silly"; break;
        case 28: return "Somewhat of a clown"; break;
        case 29: return "Quick to flee"; break;
        }
    }
};
