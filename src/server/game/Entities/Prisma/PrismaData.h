#pragma once

#include "SharedDefines.h"
#include <string>
#include <unordered_map>
#include <vector>
#include <cmath>
#include <Random.h>

#define CONFRONTATION_RANGE                 4.f
#define COMBAT_RANGE_FROM_PIVOT             10.f
#define COMBAT_RANGE_MASTER_FROM_PIVOT      12.5f
#define RAND_OFFSET_WILD_MASTER             M_PI_4
#define MAX_ITERATION_ON_ENTER_COMBAT       25

#define PRISMA_TEMPLATE_RESERVED_MIN        45000
#define PRISMA_TEMPLATE_RESERVED_MAX        55000

#define IV_MIN_VALUE        0
#define IV_MAX_VALUE        31

#define EV_MIN_VALUE        0
#define EV_MAX_VALUE        252
#define EV_TOTAL_VALUE      510

#define MIN_STAMINA_VALUE   11
#define MIN_STAT_VALUE      4

#define NATURE_FACTOR       0.1f

enum class PrismaExperienceTypes
{
    SLOW,
    MEDIUM_SLOW,
    MEDIUM_FAST,
    FAST
};

#define NUM_MAX_EXPERIENCE_TYPE                 4

enum class PrismaCombatUnits
{
    OWNER,
    PRISMA
};

#define NUM_MAX_COMBAT_UNIT                     2

enum class PrismaCombatCamps
{
    FRIEND,
    ENNEMY
};

#define NUM_MAX_COMBAT_CAMP                     2

enum class PrismaMoveCategories
{
    PHYSICAL,
    SPECIAL,
    OTHER
};

#define NUM_MAX_MOVE_CATEGORY                   3

enum class PrismaMoveSelectionTypes
{
    TARGET,
    ENEMY,
    ALL
};

#define NUM_MAX_MOVE_SELECTION_TYPE             3

enum class PrismaTypes
{
    TYPELESS = -1,
    NORMAL,
    FIRE,
    WATER,
    LIGHTNING,
    NATURE,
    ICE,
    FIGHTING,
    POISON,
    EARTH,
    AIR,
    PSIONIC,
    PHANTOM,
    DARK,
    METAL,
    LIGHT,
    SOUND
};

#define NUM_MAX_PRISMA_TYPE                     16

enum class PrismaStats
{
    STAMINA,
    ATTACK,
    DEFENSE,
    SPECIAL_ATTACK,
    SPECIAL_DEFENSE,
    SPEED
};

#define NUM_MAX_PRISMA_STAT                     6

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

#define NUM_MAX_PRISMA_NATURE                   25

enum class PrismaGenders
{
    MALE,
    FEMALE,
    NO_GENDER,
};

#define NUM_MAX_PRISMA_GENDER                   3

enum class PrismaNonVolatileStatus
{
    BURN,               //  0x1         damage 1/16 each turn
    FREEZE,             //  0x2         20% of thawed each turn, can't use move
    PARALYSIS,          //  0x4         speed * 0.5, 25% chance of being unable to use a move
    POISON,             //  0x8         damage 1/16 each turn
    BADLY_POISONED,     // 0x10         damage 1/16 + 1/16 each turn, is PrismaNonVolatileStatus::POISON at end of combat
    SLEEP,              // 0x20         1 to 3 turn
};

#define NUM_MAX_PRISMA_NON_VOLATILE_STATUS      6

enum class PrismaVolatileStatus
{
    BOUND,              //      0x1
    CAN_NOT_ESCAPE,     //      0x2
    CONFUSION,          //      0x4
    CURSE,              //      0x8
    DROWSY,             //     0x10
    EMBARGO,            //     0x20
    ENCORE,             //     0x40
    FLINCH,             //     0x80
    HEAL_BLOCK,         //    0x100
    IDENTIFIED,         //    0x200
    INFATUATION,        //    0x400
    LEECH_SEED,         //    0x800
    NIGHTMARE,          //   0x1000
    PERISH_SONG,        //   0x2000
    TAUNT,              //   0x4000
    TELEKINESIS,        //   0x8000
    TORMENT,            //  0x10000
    SPLINTERS,          //  0x20000
    POWER_BOOST,        //  0x40000
    POWER_DROP,         //  0x80000
    GUARD_BOSST,        // 0x100000
    GUARD_DROP          // 0x200000
};

#define NUM_MAX_VOLATILE_STATUS                 22

enum class PrismaVolatileCombatStatus
{
    AQUA_RING,                  //    0x1
    BRACING,                    //    0x2
    CHARGING_TURN,              //    0x4
    CENTER_OF_ATTENTION,        //    0x8
    DEFENSE_CURL,               //   0x10
    ROOTING,                    //   0x20
    MAGIC_COAT,                 //   0x40
    MAGNETIC_LEVITATION,        //   0x80
    MIMIC,                      //  0x100
    MINIMIZE,                   //  0x200
    PROTECTING,                 //  0x400
    RECHARGING,                 //  0x800
    SEMI_INVULNERABLE_TURN,     // 0x1000
    SUBSTITUTE,                 // 0x2000
    TAKING_AIM,                 // 0x4000
    THRASHING,                  // 0x8000
    TRANSFORMED,                //0x10000
    FIXATED,                    //0x20000
    PRIMED,                     //0x40000
    OBSCURED                    //0x80000
};

#define NUM_MAX_VOLATILE_COMBAT_STATUS          20

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
    int32 Type1;
    int32 Type2;
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

struct TC_GAME_API PrismaData
{
    uint32 GUID;
    uint32 ID; // refer to PrismaTemplate
    PrismaGenders Gender;
    PrismaNatures Nature;
    uint32 Level;
    uint32 Experience;
    int32 Item;
    uint32 CurrentStamina;
    uint32 StatusFlags; // only non volatile is stored
    uint32 IVStamina;
    uint32 IVAttack;
    uint32 IVDefense;
    uint32 IVSpecialAttack;
    uint32 IVSpecialDefense;
    uint32 IVSpeed;
    uint32 EVStamina;
    uint32 EVAttack;
    uint32 EVDefense;
    uint32 EVSpecialAttack;
    uint32 EVSpecialDefense;
    uint32 EVSpeed;
    int32 Move0;
    uint32 PP_Move0;
    int32 Move1;
    uint32 PP_Move1;
    int32 Move2;
    uint32 PP_Move2;
    int32 Move3;
    uint32 PP_Move3;
};

struct TC_GAME_API PrismaMoveTemplate
{
    uint32 Entry;
    uint32 SpellTrigger;
    std::string Name;
    PrismaTypes Type;
    PrismaMoveCategories Category;
    uint32 PowerPoints;
    uint32 BasePower;
    uint32 Accuracy;
    float CritRate;
    bool SpeedPriority;
    PrismaMoveSelectionTypes SelectionType;
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

struct TC_GAME_API PrismaEffortValue
{
    uint8 stamina;
    uint8 attack;
    uint8 defense;
    uint8 special_attack;
    uint8 special_defense;
    uint8 speed;

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

    uint16 Total()
    {
        return uint16(stamina + attack + defense + special_attack + special_defense + speed);
    }

    uint8 Add(uint8 param, uint8 value)
    {
        if ((param + value) > EV_MAX_VALUE)
            return EV_MAX_VALUE;

        return (param + value);
    }

    bool Add(PrismaStats _stat, uint8 value)
    {
        if ((Total() + value) > EV_TOTAL_VALUE)
            return false;

        switch (_stat)
        {
        case PrismaStats::STAMINA: stamina = Add(stamina, value); break;
        case PrismaStats::ATTACK: attack = Add(attack, value); break;
        case PrismaStats::DEFENSE: defense = Add(defense, value); break;
        case PrismaStats::SPECIAL_ATTACK: special_attack = Add(special_attack, value); break;
        case PrismaStats::SPECIAL_DEFENSE: special_defense = Add(special_defense, value); break;
        case PrismaStats::SPEED: speed = Add(speed, value); break;
        }

        return true;
    }

    void Init()
    {
        stamina = 0;
        attack = 0;
        defense = 0;
        special_attack = 0;
        special_defense = 0;
        speed = 0;
    }
};

struct TC_GAME_API PrismaMoveSet
{
    int32 move0;
    uint32 pp_move0;
    int32 move1;
    uint32 pp_move1;
    int32 move2;
    uint32 pp_move2;
    int32 move3;
    uint32 pp_move3;

    void Init()
    {
        move0 = -1;
        pp_move0 = 0;
        move1 = -1;
        pp_move1 = 0;
        move2 = -1;
        pp_move2 = 0;
        move3 = -1;
        pp_move3 = 0;
    }

    int32* GetMovesID()
    {
        static int32 moves[4];

        moves[0] = move0;
        moves[1] = move1;
        moves[2] = move2;
        moves[3] = move3;

        return moves;
    }

    uint32* GetMovesPP()
    {
        static uint32 pp_moves[4];

        pp_moves[0] = pp_move0;
        pp_moves[1] = pp_move1;
        pp_moves[2] = pp_move2;
        pp_moves[3] = pp_move3;

        return pp_moves;
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
        if (nature >= 0 && nature < NUM_MAX_PRISMA_NATURE)
            return (PrismaNatures)nature;

        return PrismaNatures::HARDY;
    }

    float GetFactor(PrismaStats _stat)
    {
        float factor = 0.f;

        if ((nature % NUM_MAX_PRISMA_STAT) == 0)
            return factor;

        switch (_stat)
        {
        case PrismaStats::STAMINA: break; // no change
        case PrismaStats::ATTACK:
            if (nature >= 0 && nature <= 4) factor += NATURE_FACTOR;
            if ((nature % (NUM_MAX_PRISMA_STAT - 1)) == 0) factor -= NATURE_FACTOR;
            break;
        case PrismaStats::DEFENSE:
            if (nature >= 5 && nature <= 9) factor += NATURE_FACTOR;
            if ((nature % (NUM_MAX_PRISMA_STAT - 1)) == 1) factor -= NATURE_FACTOR;
            break;
        case PrismaStats::SPEED:
            if (nature >= 10 && nature <= 14) factor += NATURE_FACTOR;
            if ((nature % (NUM_MAX_PRISMA_STAT - 1)) == 2) factor -= NATURE_FACTOR;
            break;
        case PrismaStats::SPECIAL_ATTACK:
            if (nature >= 15 && nature <= 19) factor += NATURE_FACTOR;
            if ((nature % (NUM_MAX_PRISMA_STAT - 1)) == 3) factor -= NATURE_FACTOR;
            break;
        case PrismaStats::SPECIAL_DEFENSE:
            if (nature >= 20 && nature <= 23) factor += NATURE_FACTOR;
            if ((nature % (NUM_MAX_PRISMA_STAT - 1)) == 4) factor -= NATURE_FACTOR;
            break;
        }

        return factor;
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
        if (gender >= 0 && gender < NUM_MAX_PRISMA_GENDER)
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

struct TC_GAME_API PrismaStatistique
{
    uint32 stamina;
    uint32 attack;
    uint32 defense;
    uint32 special_attack;
    uint32 special_defense;
    uint32 speed;
};

struct CombatCampPosition
{
    Position player;
    Position target;
};

struct CombatRandAngle
{
    float player;
    float target;

    void Initialize()
    {
        player = frand(0.f, float(M_PI * 2.0));
        target = player + M_PI;
    }
};
