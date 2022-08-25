
#include "ScriptMgr.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "Creature.h"
#include "Log.h"
#include "DatabaseEnv.h"
#include "Prisma.h"
#include "PrismaCombat.h"

class PrismaCombatScript : public PlayerScript
{
public:
    PrismaCombatScript()
        : PlayerScript("PrismaCombatScript")
    { }

    void OnReceivePrismaData(Player* player, std::string data) override
    {
        PrismaCombat* combat = GetPrismaCombat(player);
        if (!combat)
            return;

        auto _data = Prisma::SplitData(data, "|", true);
        if (_data.size() == 0)
            return;

        // player use a move
        if (_data[0] == "UseMove")
        {
            if (_data.size() != 2)
                return;

            uint32 move_index = uint32(std::stoul(_data[1])) - 1;
            combat->UseMove(player, move_index);
            return;
        }


        //combat->DealDamage(player, )
        //TC_LOG_INFO("prisma", "Receive data from (%s), data (%s)", player->GetName(), data);
    }

    void OnPrismaKill(Player* player, Prisma* prisma) override
    {
        PrismaCombat* combat = GetPrismaCombat(player);
        if (!combat)
            return;

        player->SendPrismaData("PRISMA", "HidePrisma");
    }

    void OnEnterCombat(Player* player, Unit* target) override
    {
        // initialize wild combat only if target is a prisma
        if (target->IsPrisma())
        {
            if (!target->ToPrisma())
                return;

            _storage[player->GetGUID()] = new PrismaCombat(player, target->ToPrisma());
        }

    }

    void OnLeaveCombat(Player* player) override
    {
        //PrismaCombat* combat = GetPrismaCombat(player);
        //if (!combat)
        //    return;

        //combat->EndCombat();
        //StopPrismaCombat(player);

        //TC_LOG_INFO("prisma", "%s leave the combat!", player->GetName());
    }

private:
    /* All prisma combat are stored here */
    std::unordered_map<ObjectGuid, PrismaCombat*> _storage;

    PrismaCombat* GetPrismaCombat(Player* player)
    {
        return _storage[player->GetGUID()];
    }

    bool StopPrismaCombat(Player* player)
    {
        ObjectGuid guid = player->GetGUID();
        if (PrismaCombat* combat = _storage[guid])
        {
            _storage.erase(guid);
        }
        else
        {
            TC_LOG_INFO("prisma", "Player with guid:%u is not in a prisma combat", guid.GetEntry());
        }
    }
};

void AddSC_prisma_combat()
{
	new PrismaCombatScript();
}
