#pragma once

#include <math.h>
#include <Random.h>

#define CONFRONTATION_RANGE                 4.f
#define COMBAT_RANGE_FROM_PIVOT             10.f
#define COMBAT_RANGE_MASTER_FROM_PIVOT      12.5f
#define RAND_OFFSET_WILD_MASTER             M_PI_4
#define MAX_ITERATION_ON_ENTER_COMBAT       25

namespace PrismaHandler
{
    enum class ExperienceType
    {
        SLOW,
        MEDIUM_SLOW,
        MEDIUM_FAST,
        FAST
    };

    enum class Type
    {
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

    /*enum class Stat
    {
        STAMINA,
        ATTACK,
        DEFENSE,
        SPECIAL_ATTACK,
        SPECIAL_DEFENSE,
        SPEED
    };*/

    /*enum class Nature
    {
        // INCREASE 10%     DECREASE 10%
        NATURE_NATURE_HARDY,
        NATURE_LONELY,     // attack           defense
        NATURE_BRAVE,      // attack           speed
        NATURE_ADAMANT,    // attack           sp.attack
        NATURE_NAUGHTY,    // attack           sp.defense
        NATURE_BOLD,       // defense          attack
        NATURE_DOCILE,
        NATURE_RELAXED,    // defense          speed
        NATURE_IMPISH,     // defense          sp.attack
        NATURE_LAX,        // defense          sp.defense
        NATURE_TIMID,      // speed            attack
        NATURE_HASTY,      // speed            defense
        NATURE_SERIOUS,
        NATURE_JOLLY,      // speed            sp.attack
        NATURE_NAIVE,      // speed            sp.defense
        NATURE_MODEST,     // sp.attack        attack
        NATURE_MILD,       // sp.attack        defense
        NATURE_QUIET,      // sp.attack        speed
        NATURE_BASHFUL,
        NATURE_RASH,       // sp.attack        sp.defense
        NATURE_CALM,       // sp.defense       attack
        NATURE_GENTLE,     // sp.defense       defense
        NATURE_SASSY,      // sp.defense       speed
        NATURE_CAREFUL,    // sp.defense       sp.attack
        NATURE_QUIRKY
    };*/

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

    enum class CombatUnit
    {
        OWNER,
        PRISMA
    };

    enum class CombatCamp
    {
        FRIEND,
        ENNEMY
    };

    class PrismaStats
    {
    public:
        static float GetAttackCoefficient(Type attack, Type target_type);
        static float GetAttackCoefficient(Type attack, Type target_type1, Type target_type2);

    private:
        static float GetCoefficientTypeNormal(Type against);
        static float GetCoefficientTypeFire(Type against);
        static float GetCoefficientTypeWater(Type against);
        static float GetCoefficientTypeLightning(Type against);
        static float GetCoefficientTypeNature(Type against);
        static float GetCoefficientTypeIce(Type against);
        static float GetCoefficientTypeFighting(Type against);
        static float GetCoefficientTypePoison(Type against);
        static float GetCoefficientTypeEarth(Type against);
        static float GetCoefficientTypeAir(Type against);
        static float GetCoefficientTypePsionic(Type against);
        static float GetCoefficientTypePhantom(Type against);
        static float GetCoefficientTypeDark(Type against);
        static float GetCoefficientTypeMetal(Type against);
        static float GetCoefficientTypeLight(Type against);
        static float GetCoefficientTypeSound(Type against);
    };

    class PrismaExperience
    {
    public:
        static int GetRequiredExperienceForNextLevel(int level, ExperienceType Type);
        static int GetGainExperience(int level, int target_level, int target_base_experience, bool against_trainer, bool use_multi_exp, int number_prisma_during_combat, float other_multiplicator);

    private:
        static int GetSlowExperience(int level);
        static int GetMediumSlowExperience(int level);
        static int GetMediumFastExperience(int level);
        static int GetFastExperience(int level);
    };
}
